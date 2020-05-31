//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/choice.hpp>
#include <gui/util.hpp>
#include <gui/thumbnail_thread.hpp>
#include <data/action/value.hpp>
#include <script/image.hpp>
#include <wx/imaglist.h>

const int thumbnail_size = 18;
const double min_item_size = thumbnail_size;

// ----------------------------------------------------------------------------- : ChoiceThumbnailRequest

class ChoiceThumbnailRequest : public ThumbnailRequest {
public:
  ChoiceThumbnailRequest(ValueViewer* cve, int id, bool from_disk, bool thread_safe);
  Image generate() override;
  void store(const Image&) override;

  bool isThreadSafe;
  bool threadSafe() const override {return isThreadSafe;}
private:
  int id;
  
  inline ChoiceStyle& style()  { return *static_cast<ChoiceStyle*>(viewer().getStyle().get()); }
  inline ValueViewer& viewer() { return *static_cast<ValueViewer*>(owner); }
};

ChoiceThumbnailRequest::ChoiceThumbnailRequest(ValueViewer* viewer, int id, bool from_disk, bool thread_safe)
  : ThumbnailRequest(
    static_cast<void*>(viewer),
    viewer->getStylePackage().name() + _("/") + viewer->getField()->name + _("/") << id,
    from_disk ? viewer->getStylePackage().lastModified()
              : wxDateTime::Now()
  )
  , isThreadSafe(thread_safe)
  , id(id)
{}

Image ChoiceThumbnailRequest::generate() {
  ChoiceStyle& s = style();
  String name = canonical_name_form(s.field().choices->choiceName(id));
  ScriptableImage& img = s.choice_images[name];
  return img.isReady()
    ? img.generate(GeneratedImage::Options(thumbnail_size, thumbnail_size, &viewer().getStylePackage(), &viewer().getLocalPackage(), ASPECT_BORDER, true))
    : wxImage();
}

void ChoiceThumbnailRequest::store(const Image& img) {
  if (img.Ok()) {
    ChoiceThumbnail& thumbnail = style().thumbnails[id];
    ChoiceThumbnailLock lock(thumbnail.mutex);
    thumbnail.bitmap = img;
    thumbnail.status = THUMB_OK;
  }
}

// ----------------------------------------------------------------------------- : DropDownChoiceListBase

DropDownChoiceListBase::DropDownChoiceListBase
    (Window* parent, bool is_submenu, ValueViewer& cve, ChoiceField::ChoiceP group)
  : DropDownList(parent, is_submenu, is_submenu ? nullptr : &cve)
  , cve(cve)
  , group(group)
{
  icon_size.width  = min_item_size;
  icon_size.height = min_item_size;
  item_size.height = max(min_item_size, item_size.height);
}

void DropDownChoiceListBase::onShow() {
  // update 'enabled'
  Context& ctx = cve.getContext();
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
  // find the image for the item
  int image_id;
  if (isFieldDefault(item)) {
    image_id = default_id;
  } else {
    image_id = getChoice(item)->first_id;
  }
  // draw image
  if (image_id < style().thumbnails.size()) {
    auto const& thumbnail = style().thumbnails[image_id];
    if (thumbnail.status == THUMB_OK)
    dc.DrawBitmap(thumbnail.bitmap, x, y);
    //il->Draw(image_id, dc, x, y, itemEnabled(item) ? wxIMAGELIST_DRAW_NORMAL : wxIMAGELIST_DRAW_TRANSPARENT);
  }
}

void DropDownChoiceListBase::generateThumbnailImages() {
  if (!isRoot()) return;
  // init choice images
  Context& ctx = cve.getContext();
  if (style().choice_images.empty() && style().image.isScripted()) {
    int n = field().choices->lastId();
    for (int i = 0 ; i < n; ++i) {
      try {
        String name = field().choices->choiceName(i);
        ctx.setVariable(_("input"), to_script(name));
        GeneratedImageP img = style().image.getValidScriptP()->eval(ctx)->toImage();
        style().choice_images.try_emplace(canonical_name_form(name), ScriptableImage(img));
      } catch (const Error& e) {
        handle_error(Error(e.what() + _("\n  while generating choice images for drop down list")));
      }
    }
  }
  // init thumbnail vector
  if (style().thumbnails.empty()) {
    style().thumbnails.resize(field().choices->lastId());
  }
  assert(style().thumbnails.size() == field().choices->lastId());
  // request thumbnails
  int end = group->lastId();
  for (int i = group->first_id ; i < end ; ++i) {
    auto& thumbnail = style().thumbnails[i];
    ChoiceThumbnailLock lock(thumbnail.mutex);
    if (thumbnail.status != THUMB_OK) {
      // update image
      String name = canonical_name_form(field().choices->choiceName(i));
      ScriptableImage& img = style().choice_images[name];
      if (!img.update(ctx) && thumbnail.status == THUMB_CHANGED) {
        thumbnail.status = THUMB_OK; // no need to rebuild
      } else if (img.isReady()) {
        // request this thumbnail
        thumbnail_thread.request(make_intrusive<ChoiceThumbnailRequest>(
            &cve, i, thumbnail.status == THUMB_NOT_MADE && !img.local(), img.threadSafe()
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
      try {
        String default_choice = field().default_script.invoke(cve.getContext())->toString();
        default_id = group->choiceId(default_choice);
      } catch (ScriptError const& e) {
        handle_error(ScriptError(e.what() + _("\n  in default script for '") + field().name + _("'")));
      }
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
    draw_drop_down_arrow(&editor(), dc.getDC(), dc.getExternalRect().grow(1), drop_down->IsShown());
  }
}
void ChoiceValueEditor::determineSize(bool) {
  bounding_box.height = max(bounding_box.height, 16.);
}

void ChoiceValueEditor::change(const Defaultable<String>& c) {
  addAction(value_action(valueP(), c));
}
