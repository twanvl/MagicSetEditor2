//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/value/multiple_choice.hpp>
#include <gui/thumbnail_thread.hpp>
#include <gui/util.hpp>
#include <data/action/value.hpp>
#include <data/stylesheet.hpp>
#include <wx/imaglist.h>

// ----------------------------------------------------------------------------- : ChoiceThumbnailRequest

class MultipleChoiceThumbnailRequest : public ThumbnailRequest {
  public:
	MultipleChoiceThumbnailRequest(ValueViewer* cve, int id, bool from_disk);
	virtual Image generate();
	virtual void store(const Image&);

	bool isThreadSafe;
	virtual bool threadSafe() const {return isThreadSafe;}
  private:
	StyleSheetP stylesheet;
	int id;
};

MultipleChoiceThumbnailRequest::MultipleChoiceThumbnailRequest(ValueViewer* cve, int id, bool from_disk)
	: ThumbnailRequest(
		reinterpret_cast<void *> (cve),
		cve->viewer.stylesheet->name() + _("/") + cve->getField()->name + _("/") << id,
		from_disk ? cve->viewer.stylesheet->lastModified()
		          : wxDateTime::Now()
	)
	, stylesheet(cve->viewer.stylesheet)
	, id(id)
{
	MultipleChoiceValueEditor* e = dynamic_cast<MultipleChoiceValueEditor*> (cve);
	if (!e)
		throw InternalError(_("Non-editor passed to MultipleChoiceThumbnailRequest"));
	String name = cannocial_name_form(e->field().choices->choiceName(id));
	ScriptableImage img = e->style().choice_images[name];
	isThreadSafe = img.threadSafe();
}

Image MultipleChoiceThumbnailRequest::generate() {
	MultipleChoiceValueEditor* cve = reinterpret_cast<MultipleChoiceValueEditor*> (owner);
	String name = cannocial_name_form(cve->field().choices->choiceName(id));
	ScriptableImage& img = cve->style().choice_images[name];
	return img.isReady()
		? img.generate(GeneratedImage::Options(16,16, stylesheet.get(), &cve->getSet(), ASPECT_BORDER, true), false)
		: wxImage();
}

void MultipleChoiceThumbnailRequest::store(const Image& img) {
	MultipleChoiceValueEditor* cve = reinterpret_cast<MultipleChoiceValueEditor*> (owner);
	wxImageList* il = cve->style().thumbnails;
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
		cve->style().thumbnails_status[id] = THUMB_OK;
	}
}

// ----------------------------------------------------------------------------- : DropDownMultipleChoiceList

/// A drop down list of color choices
class DropDownMultipleChoiceList : public DropDownChoiceListBase {
  public:
	DropDownMultipleChoiceList(Window* parent, bool is_submenu, ValueViewer& cve, ChoiceField::ChoiceP group);
	
  protected:
	virtual void   onShow();
	virtual bool   select(size_t item);
	virtual size_t selection() const;
	virtual DropDownList* createSubMenu(ChoiceField::ChoiceP group) const;
	virtual ThumbnailRequestP createThumbnailRequest(ValueViewer * e, int index, bool from_disk) const;
	virtual void drawIcon(DC& dc, int x, int y, size_t item, bool selected) const;
	
	virtual void onMotion(wxMouseEvent&);
  private:
	bool kept_open; ///< Was the list kept open after selecting a choice, if so, be eager to close it
};

DropDownMultipleChoiceList::DropDownMultipleChoiceList
		(Window* parent, bool is_submenu, ValueViewer& cve, ChoiceField::ChoiceP group)
	: DropDownChoiceListBase(parent, is_submenu, cve, group)
	, kept_open(false)
{
	icon_size.width += 16;
}

bool DropDownMultipleChoiceList::select(size_t item) {
	MultipleChoiceValueEditor& mcve = dynamic_cast<MultipleChoiceValueEditor&>(cve);
	if (isFieldDefault(item)) {
		mcve.toggleDefault();
	} else {
		ChoiceField::ChoiceP choice = getChoice(item);
		mcve.toggle(choice->first_id);
	}
	// keep the box open
	DropDownChoiceListBase::onShow(); // update 'enabled'
	kept_open = true;
	return false;
}

void DropDownMultipleChoiceList::drawIcon(DC& dc, int x, int y, size_t item, bool selected) const {
	// is this item active/checked?
	bool active = false;
	bool radio  = false;
	if (!isFieldDefault(item)) {
		ChoiceField::ChoiceP choice = getChoice(item);
		active = dynamic_cast<MultipleChoiceValueEditor&>(cve).active[choice->first_id];
		radio  = choice->type == CHOICE_TYPE_RADIO;
	} else {
		active = dynamic_cast<MultipleChoiceValueEditor&>(cve).value().value.isDefault();
	}
	// draw checkbox
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	dc.DrawRectangle(x,y,16,16);
	wxRect rect = RealRect(x+2,y+2,12,12);
	if (radio) {
		draw_radiobox(nullptr, dc, rect, active, itemEnabled(item));
	} else {
		draw_checkbox(nullptr, dc, rect, active, itemEnabled(item));
	}
	// draw icon
	DropDownChoiceListBase::drawIcon(dc, x + 16, y, item, selected);
}

void DropDownMultipleChoiceList::onShow() {
	DropDownChoiceListBase::onShow();
	// we need thumbnail images soon
	const_cast<DropDownMultipleChoiceList*>(this)->generateThumbnailImages();
	kept_open = false;
}

size_t DropDownMultipleChoiceList::selection() const {
	return NO_SELECTION; // we don't know the selection
}

DropDownList* DropDownMultipleChoiceList::createSubMenu(ChoiceField::ChoiceP group) const {
	return new DropDownMultipleChoiceList(const_cast<DropDownMultipleChoiceList*>(this), true, cve, group);
}

ThumbnailRequestP DropDownMultipleChoiceList::createThumbnailRequest(ValueViewer * e, int index, bool from_disk) const {
	return new_intrusive3<MultipleChoiceThumbnailRequest>(e, index, from_disk);
}

void DropDownMultipleChoiceList::onMotion(wxMouseEvent& ev) {
	if (kept_open) {
		wxSize cs = GetClientSize();
		if (ev.GetX() < 0 || ev.GetY() < 0 || ev.GetX() >= cs.x || ev.GetY() >= cs.y) {
			hide(false); // outside box; hide it
		}
	}
	DropDownChoiceListBase::onMotion(ev);
}

// ----------------------------------------------------------------------------- : MultipleChoiceValueEditor

IMPLEMENT_VALUE_EDITOR(MultipleChoice) {}

MultipleChoiceValueEditor::~MultipleChoiceValueEditor() {
	thumbnail_thread.abort(this);
}

DropDownList& MultipleChoiceValueEditor::initDropDown() {
	if (!drop_down) {
		drop_down.reset(new DropDownMultipleChoiceList(&editor(), false, *this, field().choices));
	}
	return *drop_down;
}

void MultipleChoiceValueEditor::determineSize(bool force_fit) {
	if (!nativeLook()) return;
	// item height
	item_height = 16;
	// height depends on number of items and item height
	int item_count = field().choices->lastId();
	style().height = item_count * item_height;
}

bool MultipleChoiceValueEditor::onLeftDown(const RealPoint& pos, wxMouseEvent& ev) {
	// find item under cursor
	if (style().render_style & RENDER_CHECKLIST) {
		int id = (int)((pos.y - style().top) / item_height);
		int end = field().choices->lastId();
		if (id >= 0 && id < end) {
			toggle(id);
			return true;
		}
	} else {
		// open a drop down menu
		return initDropDown().onMouseInParent(ev, style().popup_style == POPUP_DROPDOWN_IN_PLACE && !nativeLook());
	}
	return false;
}
bool MultipleChoiceValueEditor::onChar(wxKeyEvent& ev) {
	if (style().render_style & RENDER_CHECKLIST) {
		// todo;
		return false;
	} else {
		return initDropDown().onCharInParent(ev);
	}
}
void MultipleChoiceValueEditor::onLoseFocus() {
	if (drop_down) drop_down->hide(false);
}

void MultipleChoiceValueEditor::onValueChange() {
	MultipleChoiceValueViewer::onValueChange();
	// determine active values
	active.clear();
	vector<String> selected;
	value().get(selected);
	vector<String>::iterator select_it = selected.begin();
	// for each choice...
	int end = field().choices->lastId();
	for (int i = 0 ; i < end ; ++i) {
		String choice = field().choices->choiceName(i);
		bool is_active = select_it != selected.end() && *select_it == choice;
		if (is_active) select_it++;
		active.push_back(is_active);
	}
}

void MultipleChoiceValueEditor::toggle(int id) {
	String new_value;
	String toggled_choice;
	// old selection
	vector<String> selected;
	value().get(selected);
	vector<String>::iterator select_it = selected.begin();
	// copy selected choices to new value
	int end = field().choices->lastId();
	for (int i = 0 ; i < end ; ++i) {
		String choice = field().choices->choiceName(i);
		bool active = select_it != selected.end() && *select_it == choice;
		if (active) select_it++;
		if (active != (i == id)) {
			if (!new_value.empty()) new_value += _(", ");
			new_value += choice;
		}
		if (i == id) toggled_choice = choice;
	}
	// store value
	getSet().actions.add(value_action(valueP(), new_value, toggled_choice));
}

void MultipleChoiceValueEditor::toggleDefault() {
	getSet().actions.add(value_action(valueP(), Defaultable<String>(value().value(), !value().value.isDefault()), _("")));
}
