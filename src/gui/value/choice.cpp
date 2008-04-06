//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/choice.hpp>
#include <gui/util.hpp>
#include <gui/thumbnail_thread.hpp>
#include <data/action/value.hpp>
#include <data/stylesheet.hpp>
#include <script/image.hpp>
#include <wx/imaglist.h>

DECLARE_TYPEOF_COLLECTION(ChoiceField::ChoiceP);

// ----------------------------------------------------------------------------- : ChoiceThumbnailRequest

class ChoiceThumbnailRequest : public ThumbnailRequest {
  public:
	ChoiceThumbnailRequest(ValueViewer* cve, int id, bool from_disk, bool thread_safe);
	virtual Image generate();
	virtual void store(const Image&);

	bool isThreadSafe;
	virtual bool threadSafe() const {return isThreadSafe;}
  private:
	StyleSheetP stylesheet;
	int id;
	
	inline ChoiceStyle& style()  { return *static_cast<ChoiceStyle*>(viewer().getStyle().get()); }
	inline ValueViewer& viewer() { return *static_cast<ValueViewer*>(owner); }
};

ChoiceThumbnailRequest::ChoiceThumbnailRequest(ValueViewer* viewer, int id, bool from_disk, bool thread_safe)
	: ThumbnailRequest(
		static_cast<void*>(viewer),
		viewer->viewer.stylesheet->name() + _("/") + viewer->getField()->name + _("/") << id,
		from_disk ? viewer->viewer.stylesheet->lastModified()
		          : wxDateTime::Now()
	)
	, isThreadSafe(thread_safe)
	, stylesheet(viewer->viewer.stylesheet)
	, id(id)
{}

Image ChoiceThumbnailRequest::generate() {
	ChoiceStyle& s = style();
	String name = cannocial_name_form(s.field().choices->choiceName(id));
	ScriptableImage& img = s.choice_images[name];
	return img.isReady()
		? img.generate(GeneratedImage::Options(16,16, stylesheet.get(), viewer().viewer.getSet().get(), ASPECT_BORDER, true))
		: wxImage();
}

void ChoiceThumbnailRequest::store(const Image& img) {
	ChoiceStyle& s = style();
	wxImageList* il = s.thumbnails;
	while (id > il->GetImageCount()) {
		il->Add(wxBitmap(16,16),*wxBLACK);
	}
	if (img.Ok()) {
		#ifdef __WXMSW__
			// for some reason windows doesn't like completely transparent images if they do not have a mask
			// HACK:
			if (img.HasAlpha() && img.GetWidth() == 16 && img.GetHeight() == 16) {
				// is the image empty?
				bool empty = true;
				int* b = (int*)img.GetAlpha();
				int* e = b + 16*16/sizeof(int);
				while (b != e) {
					if (*b++) {
						empty = false;
						break;
					}
				}
				// if so, use a mask instead
				if (empty) {
					const_cast<Image&>(img).ConvertAlphaToMask();
				}
			}
			// Hack ends here
		#endif
		if (id == il->GetImageCount()) {
			il->Add(img);
		} else {
			il->Replace(id, img);
		}
		s.thumbnails_status[id] = THUMB_OK;
	}
}

// ----------------------------------------------------------------------------- : DropDownChoiceListBase

DropDownChoiceListBase::DropDownChoiceListBase
		(Window* parent, bool is_submenu, ValueViewer& cve, ChoiceField::ChoiceP group)
	: DropDownList(parent, is_submenu, is_submenu ? nullptr : &cve)
	, cve(cve)
	, group(group)
{
	icon_size.width  = 16;
	icon_size.height = 16;
	item_size.height = max(16., item_size.height);
}

void DropDownChoiceListBase::onShow() {
	// update 'enabled'
	Context& ctx = cve.viewer.getContext();
	FOR_EACH(c, group->choices) {
		c->enabled.update(ctx);
	}
}

size_t DropDownChoiceListBase::itemCount() const {
	return group->choices.size() + hasDefault();
}

ChoiceField::ChoiceP DropDownChoiceListBase::getChoice(size_t item) const {
	if (isGroupDefault(item)) {
		return group;
	} else {
		return group->choices[item - hasDefault()];
	}
}

String DropDownChoiceListBase::itemText(size_t item) const {
	if (isFieldDefault(item)) {
		return field().default_name;
	} else if (isGroupDefault(item)) {
		return group->default_name;
	} else {
		ChoiceField::ChoiceP choice = getChoice(item);
		return choice->name;
	}
}
bool DropDownChoiceListBase::lineBelow(size_t item) const {
	return isDefault(item) || getChoice(item)->line_below;
}
bool DropDownChoiceListBase::itemEnabled(size_t item) const {
	return isDefault(item) || getChoice(item)->enabled;
}
DropDownList* DropDownChoiceListBase::submenu(size_t item) const {
	if (isDefault(item)) return nullptr;
	item -= hasDefault();
	if (item >= submenus.size()) submenus.resize(item + 1);
	if (submenus[item]) return submenus[item].get();
	ChoiceField::ChoiceP choice = group->choices[item];
	if (choice->isGroup()) {
		// create submenu
		submenus[item].reset(createSubMenu(choice));
	}
	return submenus[item].get();
}

void DropDownChoiceListBase::drawIcon(DC& dc, int x, int y, size_t item, bool selected) const {
	// imagelist to use
	wxImageList* il = style().thumbnails;
	assert(il);
	// find the image for the item
	int image_id;
	if (isFieldDefault(item)) {
		image_id = default_id;
	} else {
		image_id = getChoice(item)->first_id;
	}
	// draw image
	if (image_id < il->GetImageCount()) {
		il->Draw(image_id, dc, x, y, itemEnabled(item) ? wxIMAGELIST_DRAW_NORMAL : wxIMAGELIST_DRAW_TRANSPARENT);
	}
}

void DropDownChoiceListBase::generateThumbnailImages() {
	if (!isRoot()) return;
	if (!style().thumbnails) {
		style().thumbnails = new wxImageList(16,16);
	}
	int image_count = style().thumbnails->GetImageCount();
	int end = group->lastId();
	// init choice images
	Context& ctx = cve.viewer.getContext();
	if (style().choice_images.empty() && style().image.isScripted()) {
		for (int i = 0 ; i < end ; ++i) {
			try {
				String name = cannocial_name_form(field().choices->choiceName(i));
				ctx.setVariable(_("input"), to_script(name));
				GeneratedImageP img = image_from_script(style().image.getScript().eval(ctx));
				style().choice_images.insert(make_pair(name, ScriptableImage(img)));
			} catch (const Error& e) {
				handle_error(Error(e.what() + _("\n  while generating choice images for drop down list")),true,false);
			}
		}
	}
	// request thumbnails
	style().thumbnails_status.resize(end, THUMB_NOT_MADE);
	for (int i = 0 ; i < end ; ++i) {
		ThumbnailStatus& status = style().thumbnails_status[i];
		if (i >= image_count || status != THUMB_OK) {
			// update image
			ChoiceStyle& s = style();
			String name = cannocial_name_form(s.field().choices->choiceName(i));
			ScriptableImage& img = s.choice_images[name];
			if (!img.update(ctx) && status == THUMB_CHANGED) {
				status = THUMB_OK; // no need to rebuild
			} else if (img.isReady()) {
				// request this thumbnail
				thumbnail_thread.request( new_intrusive4<ChoiceThumbnailRequest>(
						&cve, i, status == THUMB_NOT_MADE && !img.local(), img.threadSafe()
					));
			}
		}
	}
}

void DropDownChoiceListBase::onIdle(wxIdleEvent& ev) {
	if (!isRoot()) return;
	if (thumbnail_thread.done(&cve)) {
		Refresh(false);
	}
}

BEGIN_EVENT_TABLE(DropDownChoiceListBase, DropDownList)
	EVT_IDLE(DropDownChoiceListBase::onIdle)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------- : DropDownChoiceList

DropDownChoiceList::DropDownChoiceList(Window* parent, bool is_submenu, ValueViewer& cve, ChoiceField::ChoiceP group)
	: DropDownChoiceListBase(parent, is_submenu, cve, group)
{}

void DropDownChoiceList::onShow() {
	DropDownChoiceListBase::onShow();
	// we need thumbnail images soon
	generateThumbnailImages();
}

void DropDownChoiceList::select(size_t item) {
	if (isFieldDefault(item)) {
		dynamic_cast<ChoiceValueEditor&>(cve).change( Defaultable<String>() );
	} else {
		ChoiceField::ChoiceP choice = getChoice(item);
		dynamic_cast<ChoiceValueEditor&>(cve).change( field().choices->choiceName(choice->first_id) );
	}
}

size_t DropDownChoiceList::selection() const {
	// selected item
	const Defaultable<String>& value = dynamic_cast<ChoiceValueEditor&>(cve).value().value();
	int id = field().choices->choiceId(value);
	// id of default item
	if (hasFieldDefault()) {
		if (value.isDefault()) {
			// default is selected
			default_id = id;
			return 0;
		} else {
			// run default script to find out what the default choice would be
			String default_choice = *field().default_script.invoke( cve.viewer.getContext() );
			default_id = group->choiceId(default_choice);
		}
	}
	// item corresponding to id
	size_t i = hasDefault();
	FOR_EACH(c, group->choices) {
		if (id >= c->first_id && id < c->lastId()) {
			return i;
		}
		i++;
	}
	return NO_SELECTION;
}

DropDownList* DropDownChoiceList::createSubMenu(ChoiceField::ChoiceP group) const {
	return new DropDownChoiceList(static_cast<Window*>(const_cast<DropDownChoiceList*>(this)), true, cve, group);
}

// ----------------------------------------------------------------------------- : ChoiceValueEditor

IMPLEMENT_VALUE_EDITOR(Choice)
	, drop_down(new DropDownChoiceList(&editor(), false, *this, field().choices))
{}

ChoiceValueEditor::~ChoiceValueEditor() {
	thumbnail_thread.abort(this);
}

bool ChoiceValueEditor::onLeftDown(const RealPoint& pos, wxMouseEvent& ev) {
	return drop_down->onMouseInParent(ev, style().popup_style == POPUP_DROPDOWN_IN_PLACE && !nativeLook());
}
bool ChoiceValueEditor::onChar(wxKeyEvent& ev) {
	return drop_down->onCharInParent(ev);
}
void ChoiceValueEditor::onLoseFocus() {
	drop_down->hide(false);
}

void ChoiceValueEditor::draw(RotatedDC& dc) {
	ChoiceValueViewer::draw(dc);
	if (nativeLook()) {
		draw_drop_down_arrow(&editor(), dc.getDC(), dc.trRectStraight(style().getInternalRect().grow(1)), drop_down->IsShown());
	}
}
void ChoiceValueEditor::determineSize(bool) {
	style().height = max(style().height(), 16.);
}

void ChoiceValueEditor::change(const Defaultable<String>& c) {
	perform(value_action(card(), valueP(), c));
}
