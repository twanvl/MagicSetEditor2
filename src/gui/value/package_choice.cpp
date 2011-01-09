//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/package_choice.hpp>
#include <gui/drop_down_list.hpp>
#include <gui/util.hpp>
#include <data/action/value.hpp>

DECLARE_TYPEOF_COLLECTION(PackageChoiceValueViewer::Item);

// ----------------------------------------------------------------------------- : DropDownPackageChoiceList

/// A drop down list of color choices
class DropDownPackageChoiceList : public DropDownList {
  public:
	DropDownPackageChoiceList(Window* parent, PackageChoiceValueEditor* editor);
	
  protected:
	virtual size_t itemCount() const;
	virtual String itemText(size_t item) const;
	virtual bool   lineBelow(size_t item) const;
	virtual void   drawIcon(DC& dc, int x, int y, size_t item, bool selected) const;
	virtual void   select(size_t selection);
	virtual size_t selection() const;
	
  private:
	PackageChoiceValueEditor& editor;
};

DropDownPackageChoiceList::DropDownPackageChoiceList(Window* parent, PackageChoiceValueEditor* editor)
	: DropDownList(parent, false, editor)
	, editor(*editor)
{
	icon_size.width  = 16;
	icon_size.height = 16;
	item_size.height = max(16., item_size.height);
}

size_t DropDownPackageChoiceList::itemCount() const {
	return editor.items.size() + (editor.field().required ? 0 : 1);
}
String DropDownPackageChoiceList::itemText(size_t item) const {
	if (item == 0 && !editor.field().required) return editor.field().empty_name;
	else {
		size_t i = item - !editor.field().required;
		return editor.items[i].name;
	}
}
bool DropDownPackageChoiceList::lineBelow(size_t item) const {
	return item == 0 && !editor.field().required;
}

void DropDownPackageChoiceList::drawIcon(DC& dc, int x, int y, size_t item, bool selected) const {
	if (item == 0 && !editor.field().required) return;
	size_t i = item - !editor.field().required;
	const Bitmap& bmp = editor.items[i].image;
	if (bmp.Ok()) dc.DrawBitmap(bmp, x, y);
}


void DropDownPackageChoiceList::select(size_t item) {
	String new_value;
	if (item != 0 || editor.field().required) {
		size_t i = item - !editor.field().required;
		new_value = editor.items[i].package_name;
	}
	editor.change(new_value);
}

size_t DropDownPackageChoiceList::selection() const {
	size_t n = 0;
	FOR_EACH(i, editor.items) {
		if (editor.value().package_name == i.package_name) {
			return n + !editor.field().required;
		}
		++n;
	}
	return editor.field().required ? NO_SELECTION : 0;
}

// ----------------------------------------------------------------------------- : PackageChoiceValueEditor

IMPLEMENT_VALUE_EDITOR(PackageChoice) {}

bool PackageChoiceValueEditor::onLeftDown(const RealPoint& pos, wxMouseEvent& ev) {
	if (!drop_down) initDropDown();
	return drop_down->onMouseInParent(ev, false);
}
bool PackageChoiceValueEditor::onChar(wxKeyEvent& ev) {
	if (!drop_down) initDropDown();
	return drop_down->onCharInParent(ev);
}
void PackageChoiceValueEditor::onLoseFocus() {
	if (drop_down) drop_down->hide(false);
}

void PackageChoiceValueEditor::draw(RotatedDC& dc) {
	PackageChoiceValueViewer::draw(dc);
	if (nativeLook()) {
		draw_drop_down_arrow(&editor(), dc.getDC(), dc.trRectToBB(style().getInternalRect().grow(1)), drop_down && drop_down->IsShown());
	}
}
void PackageChoiceValueEditor::determineSize(bool) {
	style().height = max(style().height(), 16.);
}

void PackageChoiceValueEditor::change(const String& c) {
	addAction(value_action(valueP(), c));
}

void PackageChoiceValueEditor::initDropDown() {
	if (drop_down) return;
	drop_down = shared(new DropDownPackageChoiceList(&editor(), this));
}
