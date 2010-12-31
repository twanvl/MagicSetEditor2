//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/drop_down_list.hpp>
#include <gui/util.hpp>
#include <render/value/viewer.hpp>
#include <render/card/viewer.hpp>
#include <gui/value/editor.hpp>
#include <util/rotation.hpp>
#include <gfx/gfx.hpp>
#include <wx/dcbuffer.h>

// ----------------------------------------------------------------------------- : DropDownList : Show/Hide

DropDownList::DropDownList(Window* parent, bool is_submenu, ValueViewer* viewer)
	: wxPopupTransientWindow(parent, wxSIMPLE_BORDER)//wxPopupWindow(parent, wxSIMPLE_BORDER)
	, text_offset(1)
	, item_size(100,1)
	, icon_size(0,0)
	, selected_item(NO_SELECTION)
	, mouse_down(false)
	, open_sub_menu(nullptr)
	, parent_menu(nullptr)
	, viewer(viewer)
	, close_on_mouse_out(false)
{
	if (is_submenu) {
		parent_menu = &dynamic_cast<DropDownList&>(*GetParent());
	}
	// determine item height
	wxClientDC dc(this);
	dc.SetFont(*wxNORMAL_FONT);
	int h;
	dc.GetTextExtent(_("X"), 0, &h);
	item_size.height = h + 2;
}

DropDownList::~DropDownList() {
	realHide(); // restore event handler before deleting it
}

void DropDownList::show(bool in_place, wxPoint pos, RealRect* rect) {
	if (IsShown()) return;
	onShow();
	// find selection
	selected_item = selection();
	// width
	size_t count = itemCount();
	if (item_size.width == 100) { // not initialized
		wxClientDC dc(this);
		dc.SetFont(*wxNORMAL_FONT);
		for (size_t i = 0 ; i < count ; ++i) {
			int text_width;
			dc.GetTextExtent(capitalize(itemText(i)), &text_width, 0);
			item_size.width = max(item_size.width, text_width + icon_size.width + 14); // 14 = room for popup arrow + padding
		}
	}
	// height
	int line_count = 0;
	for (size_t i = 0 ; i < count ; ++i) if (lineBelow(i)) line_count += 1;
	// size
	RealSize border_size(2,2); // GetClientSize() - GetSize(), assume 1px borders
	RealSize size(
		item_size.width + marginW * 2,
		item_size.height * count + marginH * 2 + line_count
	);
	// placement
	int parent_height = 0;
	if (!in_place && viewer) {
		// Position the drop down list below the editor control (based on the style)
		Rotation rot = viewer->viewer.getRotation();
		Rotater rr(rot, viewer->getRotation());
		RealRect r = rot.trRectToBB(rect ? *rect : rot.getInternalRect());
		if (viewer->viewer.nativeLook()) {
			pos           = RealPoint(r.x - 3, r.y - 3);
			size.width    = max(size.width, r.width + 6);
			parent_height = (int)r.height + 6;
		} else {
			pos           = RealPoint(r.x - 1, r.y - 1);
			parent_height = (int)r.height;
		}
	} else if (parent_menu) {
		parent_height = -(int)item_size.height - 1;
	}
	pos = GetParent()->ClientToScreen(pos);
	// virtual size = item size
	RealSize virtual_size = size;
	SetVirtualSize(virtual_size);
	item_size.width = virtual_size.width - marginW * 2;
	// is there enough room for all items, or do we need a scrollbar?
	int room_below = wxGetDisplaySize().y - border_size.height - pos.y - parent_height - 50;
	int max_height = max(200, room_below);
	if (size.height > max_height) {
		size.height = max_height;
		size.width += wxSystemSettings::GetMetric(wxSYS_VSCROLL_ARROW_X); // width of scrollbar
		SetScrollbar(wxVERTICAL, 0, size.height, virtual_size.height, false);
	} else {
		SetScrollbar(wxVERTICAL,0,0,0,false);
	}
	// move & resize
	SetSize(add_diagonal(size, border_size));
	Position(pos, wxSize(0, parent_height));
	// visible item
	visible_start = 0;
	ensureSelectedItemVisible();
	// show
	if (GetParent()->HasCapture()) {
		// release capture on parent
		// do this before showing the popup, because that might change who has the capture
		GetParent()->ReleaseMouse();
	}
	if (selected_item == NO_SELECTION && itemCount() > 0) selected_item = 0; // select first item by default
	mouse_down = false;
	close_on_mouse_out = false;
	Popup();
	// fix drop down arrow
	redrawArrowOnParent();
}

void DropDownList::hide(bool event, bool allow_veto) {
	// hide?
	bool keep_open = event && allow_veto && stayOpen(selected_item);
	if (keep_open) {
		close_on_mouse_out = true;
	} else {
		// hide root
		DropDownList* root = this;
		while (root->parent_menu) {
			root = root->parent_menu;
		}
		root->realHide();
	}
	// send event
	if (event && selected_item != NO_SELECTION && itemEnabled(selected_item)) {
		select(selected_item);
		if (IsShown()) Refresh(false);
	}
}

void DropDownList::realHide() {
	if (IsShown()) Dismiss();
}

void DropDownList::OnDismiss() {
	hideSubMenu();
	if (parent_menu) {
		parent_menu->open_sub_menu = nullptr;
	} else {
		redrawArrowOnParent();
	}
}

void DropDownList::hideSubMenu() {
	if (open_sub_menu) {
		open_sub_menu->realHide();
		open_sub_menu = nullptr;
	}
}

bool DropDownList::showSubMenu() {
	if (selected_item == NO_SELECTION) {
		hideSubMenu();
		return false;
	} else {
		// find position to show item at
		return showSubMenu(selected_item, itemPosition(selected_item));
	}
}
bool DropDownList::showSubMenu(size_t item, int y) {
	DropDownList* sub_menu = item == NO_SELECTION ? nullptr : submenu(item);
	if (sub_menu == open_sub_menu) return sub_menu; // no change
	hideSubMenu();
	open_sub_menu = sub_menu;
	if (!sub_menu) return false;
	// open new menu
	wxSize size = GetSize();
	sub_menu->show(true,
		sub_menu->GetParent()->ScreenToClient(ClientToScreen(
			wxPoint(size.GetWidth() - 1, y + (int)item_size.height)
		)));
	return true;
}

bool DropDownList::selectItem(size_t item) {
	if ((int)item >= 0 && item < itemCount()) {
		selected_item = item;
		ensureSelectedItemVisible();
		Refresh(false);
		return true;
	} else {
		return false;
	}
}

void DropDownList::ensureSelectedItemVisible() {
	if (selected_item == NO_SELECTION) return;
	// ensure that this item is visible
	int item_top = itemPosition(selected_item);
	wxSize cs = GetClientSize();
	if (item_top < marginH) {
		scrollTo(item_top + visible_start);
	} else if (item_top + item_size.height - 1 > cs.y - marginH) {
		scrollTo(item_top + visible_start + item_size.height - 1 - (cs.y - marginH));
	}
}
void DropDownList::scrollTo(int pos) {
	visible_start = max(0, min(GetVirtualSize().y - GetSize().y, pos));
	SetScrollPos(wxVERTICAL, visible_start);
	Refresh(false);
}

int DropDownList::itemPosition(size_t item) const {
	int y = marginH - visible_start;
	size_t count = itemCount();
	for (size_t i = 0 ; i < count ; ++i) {
		if (i == item) return y;
		y += (int)item_size.height + lineBelow(i);
	}
	// not found
	assert(false);
	return 0;
}

void DropDownList::redrawArrowOnParent() {
	if (viewer) {
		ValueEditor* e = viewer->getEditor();
		if (e && viewer->viewer.nativeLook()) {
			CardEditor& editor = static_cast<CardEditor&>(viewer->viewer);
			shared_ptr<RotatedDC> dcP = editor.overdrawDC();
			RotatedDC& dc = *dcP;
			Rotater r(dc, viewer->getRotation());
			draw_drop_down_arrow(&editor, dc.getDC(), dc.trRectToBB(dc.getInternalRect().grow(1)), IsShown());
		}
	}
}

// ----------------------------------------------------------------------------- : DropDownList : Drawing

void DropDownList::onPaint(wxPaintEvent&) {
	wxBufferedPaintDC dc(this);
	draw(dc);
}

void DropDownList::draw(DC& dc) {
	// Size
	wxSize cs = dc.GetSize();
	// Draw background & frame
	dc.SetPen  (*wxTRANSPARENT_PEN);
	dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	dc.DrawRectangle(0, 0, cs.x, cs.y);
	dc.SetFont(*wxNORMAL_FONT);
	// Draw items
	int y = marginH - visible_start;
	size_t count = itemCount();
	for (size_t i = 0 ; i < count ; ++i) {
		drawItem(dc, y, i);
		y += (int)item_size.height + lineBelow(i);
	}
}

void DropDownList::drawItem(DC& dc, int y, size_t item) {
	if (y + item_size.height <= 0 || y >= dc.GetSize().y) return; // not visible
	// draw background
	dc.SetPen(*wxTRANSPARENT_PEN);
	if (item == selected_item) {
		if (itemEnabled(item)) {
			dc.SetBrush         (wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
			dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
		} else {
			dc.SetBrush         (wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
			dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
		}
		dc.DrawRectangle(marginW, y, (int)item_size.width, (int)item_size.height);
	} else if (!itemEnabled(item)) {
		// mix between foreground and background
		dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
	} else if (highlightItem(item)) {
		// mix a color between selection and window
		dc.SetBrush         (lerp(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT),
		                          wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW), 0.75));
		dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
		dc.DrawRectangle(marginW, y, (int)item_size.width, (int)item_size.height);
	} else {
		dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
	}
	// draw text and icon
	drawIcon(dc, marginW, y, item, item == selected_item);	
	dc.DrawText(capitalize(itemText(item)), marginW + (int)icon_size.width + 1, y + text_offset);
	// draw popup icon
	if (submenu(item)) {
		draw_menu_arrow(this, dc, RealRect(marginW, y, item_size.width, item_size.height), item == selected_item);
	}
	// draw line below
	if (lineBelow(item) && item != itemCount()) {
		dc.SetPen(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
		dc.DrawLine(marginW, y + (int)item_size.height, marginW + (int)item_size.width, y + (int)item_size.height);
	}
}

// ----------------------------------------------------------------------------- : DropDownList : Events

void DropDownList::onLeftDown(wxMouseEvent& ev) {
	wxSize cs = GetClientSize();
	if (ev.GetX() < 0 || ev.GetX() >= cs.x || ev.GetY() < marginH || ev.GetY() >= cs.y) {
		hide(false);
		ev.Skip();
		return;
	}
	mouse_down = true; // prevent closing on mouseup of the click that opened the window
}

void DropDownList::onLeftUp(wxMouseEvent&) {
	if (mouse_down) {
		if (selected_item != NO_SELECTION && !itemEnabled(selected_item)) return; // disabled item
		// don't hide if there is a child menu
		if (selected_item != NO_SELECTION && submenu(selected_item)) return;
		hide(true);
	}
}

void DropDownList::onMotion(wxMouseEvent& ev) {
	// size
	wxSize cs = GetClientSize();
	// inside?
	if (ev.GetX() < marginW || ev.GetX() + marginW >= cs.GetWidth() || ev.GetY() < marginH || ev.GetY() + marginH >= cs.GetHeight()) {
		ev.Skip();
		return;
	}
	// find selected item
	int startY = marginH - visible_start;
	size_t count = itemCount();
	for (size_t i = 0 ; i < count ; ++i) {
		int endY = startY + (int)item_size.height;
		if (ev.GetY() >= startY && ev.GetY() < endY) {
			if (itemEnabled(i)) {
				showSubMenu(i, startY);
			}
			selectItem(i);
			return;
		}
		startY = endY + lineBelow(i);
	}
	hideSubMenu();
}

void DropDownList::onMouseLeave(wxMouseEvent& ev) {
	if (close_on_mouse_out) {
		wxSize cs = GetClientSize();
		if (ev.GetX() < 0 || ev.GetY() < 0 || ev.GetX() >= cs.x || ev.GetY() >= cs.y) {
			hide(false); // outside box; hide it
			ev.Skip();
			return;
		}
	}
}

void DropDownList::onMouseWheel(wxMouseEvent& ev) {
	scrollTo(visible_start - item_size.height * ev.GetWheelRotation() / ev.GetWheelDelta());
}

void DropDownList::onScroll(wxScrollWinEvent& ev) {
	wxEventType type = ev.GetEventType();
	if (type == wxEVT_SCROLLWIN_TOP) {
		scrollTo(0);
	} else if (type == wxEVT_SCROLLWIN_BOTTOM) {
		scrollTo(INT_MAX);
	} else if (type == wxEVT_SCROLLWIN_LINEUP) {
		scrollTo(visible_start - item_size.height);
	} else if (type == wxEVT_SCROLLWIN_LINEDOWN) {
		scrollTo(visible_start + item_size.height);
	} else if (type == wxEVT_SCROLLWIN_PAGEUP) {
		scrollTo(visible_start - (GetClientSize().y - item_size.height));
	} else if (type == wxEVT_SCROLLWIN_PAGEDOWN) {
		scrollTo(visible_start + (GetClientSize().y - item_size.height));
	} else {
		scrollTo(ev.GetPosition());
	}
}

// ----------------------------------------------------------------------------- : DropDownList : Parent events

bool DropDownList::onMouseInParent(wxMouseEvent& ev, bool open_in_place) {
	if (IsShown()) {
		hide(false);
	} else {
		show(open_in_place, wxPoint(ev.GetX(), ev.GetY()));
		ev.Skip(false); // Don't set the focus to the parent afterwards
	}
	return true;
}

bool DropDownList::onCharInParent(wxKeyEvent& ev) {
	// keyboard codes
	int k = ev.GetKeyCode();
	if (IsShown()) {
		if (open_sub_menu) {
			// sub menu always takes keys
			return open_sub_menu->onCharInParent(ev);
		} else {
			switch (k) {
				case WXK_UP:
					return selectItem(selected_item - 1);
				case WXK_DOWN:
					return selectItem(selected_item + 1);
				case WXK_RETURN:
					if (!showSubMenu() && (selected_item == NO_SELECTION || itemEnabled(selected_item))) {
						hide(true, false); // don't veto; always close
					}
					break;
				case WXK_SPACE:
					if (!showSubMenu() && (selected_item == NO_SELECTION || itemEnabled(selected_item))) {
						hide(true);
					}
					break;
				case WXK_ESCAPE:
					hide(false);
					break;
				case WXK_LEFT:
					if (parent_menu) hide(false);
					break;
				case WXK_RIGHT:
					return showSubMenu();
				default:
					// match first character of an item, start searching just after the current selection
					size_t si = selected_item != NO_SELECTION ? selected_item + 1 : 0;
					size_t count = itemCount();
					for (size_t i = 0 ; i < count ; ++i) {
						size_t index = (si + i) % count;
						if (!itemEnabled(index)) continue;
						String c = itemText(index);
#ifdef UNICODE
						if (!c.empty() && toUpper(c.GetChar(0)) == toUpper(ev.GetUnicodeKey())) {
#else
						if (!c.empty() && toUpper(c.GetChar(0)) == toUpper(ev.GetKeyCode())) {
#endif
							// first character matches
							selected_item = index;
							showSubMenu();
							selectItem(index);
							return true;
						}
					}
			}
		}
		return true;
	} else if (k==WXK_SPACE || k==WXK_RETURN || k==WXK_DOWN) {
		// drop down list is not shown yet, show it now
		show(false, wxPoint(0,0));
		return true;
	}
	return false;
}

void DropDownList::onKeyDown(wxKeyEvent& ev) {
	// If we don't handle this event, then wxPopupTransientWindow desides to dismiss itself
	// we can't wait for onChar
	onCharInParent(ev);
}

// ----------------------------------------------------------------------------- : DropDownList : Event table

// Note: some DropDownList events get sent to the parent which in turn should send them to the DropDownList
BEGIN_EVENT_TABLE(DropDownList,wxPopupWindow)
	EVT_PAINT        (DropDownList::onPaint)
	EVT_LEFT_DOWN    (DropDownList::onLeftDown)
	EVT_LEFT_UP      (DropDownList::onLeftUp)
	EVT_MOTION       (DropDownList::onMotion)
	EVT_LEAVE_WINDOW (DropDownList::onMouseLeave)
	EVT_MOUSEWHEEL   (DropDownList::onMouseWheel)
	EVT_SCROLLWIN    (DropDownList::onScroll)
	EVT_KEY_DOWN     (DropDownList::onKeyDown)
END_EVENT_TABLE  ()
