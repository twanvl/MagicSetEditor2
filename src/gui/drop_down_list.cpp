//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/drop_down_list.hpp>
#include <gui/util.hpp>
#include <render/value/viewer.hpp>
#include <gfx/gfx.hpp>
#include <wx/dcbuffer.h>

// ----------------------------------------------------------------------------- : DropDownHider

// Class that intercepts all events not directed to a DropDownList, and closes the list
class DropDownHider : public wxEvtHandler {
  public:
	DropDownHider(DropDownList& list) : list(list) {}
	
  private:
	DropDownList& list;
	
	virtual bool ProcessEvent(wxEvent& ev) {
		// close the list, and pass on the event
		// don't just use ev.Skip(), because this event handler will be removed by hiding,
		// so there will be no next handler to skip to
		wxEvtHandler* nh = GetNextHandler();
		list.hide(false);
		if (nh) nh->ProcessEvent(ev);
	}
};

/*BEGIN_EVENT_TABLE(DropDownHider, wxEvtHandler)
	EVT_CUSTOM(wxEVT_LEFT_DOWN,      wxID_ANY, DropDownHider::onEvent)
	EVT_CUSTOM(wxEVT_RIGHT_DOWN,     wxID_ANY, DropDownHider::onEvent)
	EVT_CUSTOM(wxEVT_MOVE,           wxID_ANY, DropDownHider::onEvent)
	EVT_CUSTOM(wxEVT_SIZE,           wxID_ANY, DropDownHider::onEvent)
	EVT_CUSTOM(wxEVT_MENU_HIGHLIGHT, wxID_ANY, DropDownHider::onEvent)
	EVT_CUSTOM(wxEVT_MENU,           wxID_ANY, DropDownHider::onEvent)
	EVT_CUSTOM(wxEVT_MENU_OPEN,      wxID_ANY, DropDownHider::onEvent)
	EVT_CUSTOM(wxEVT_ACTIVATE,       wxID_ANY, DropDownHider::onEvent)
//	EVT_CUSTOM(wxEVT_CLOSE,          wxID_ANY, DropDownHider::onEvent)
	EVT_CUSTOM(wxEVT_KILL_FOCUS,     wxID_ANY, DropDownHider::onEvent)
/*	EVT_LEFT_DOWN      (          (wxMouseEventFunction)&DropDownHider::onEvent)
	EVT_RIGHT_DOWN     (          DropDownHider::onMouseEvent)
	EVT_MOVE           (          DropDownHider::onMoveEvent)
	EVT_SIZE           (          DropDownHider::onSizeEvent)
	EVT_MENU_HIGHLIGHT (wxID_ANY, DropDownHider::onMenuEvent)
	EVT_MENU           (wxID_ANY, DropDownHider::onCommandEvent)
	EVT_MENU_OPEN      (          DropDownHider::onMenuEvent)
	EVT_ACTIVATE       (          DropDownHider::onActivateEvent)
	EVT_CLOSE          (          DropDownHider::onCloseEvent)
	EVT_KILL_FOCUS     (          DropDownHider::onFocusEvent)
END_EVENT_TABLE  ()
*/

// ----------------------------------------------------------------------------- : DropDownList : Show/Hide

DropDownList::DropDownList(Window* parent, bool is_submenu, ValueViewer* viewer)
	: wxPopupWindow(parent)
	, mouse_down(false)
	, selected_item(NO_SELECTION)
	, open_sub_menu(nullptr)
	, parent_menu(is_submenu ? static_cast<DropDownList*>(GetParent()) : nullptr)
	, viewer(viewer)
	, item_size(100,1)
{
	// determine item height
	wxClientDC dc(this);
	dc.SetFont(*wxNORMAL_FONT);
	int h;
	dc.GetTextExtent(_("X"), 0, &h);
	item_size.height = h;
}

void DropDownList::show(bool in_place, wxPoint pos) {
	if (IsShown()) return;
	// find selection
	selected_item = selection();
	// fix size & position
	int line_count = 0;
	size_t count = itemCount();
	for (size_t i = 0 ; i < count ; ++i) if (lineBelow(i)) line_count += 1;
	wxSize size(
		item_size.width + marginW * 2,
		item_size.height * count + marginH * 2 + line_count
	);
	int parent_height = 0;
	/*if (!in_place) {
		// Position the drop down list below the editor control (based on the style)
		RotatedObject rot(editor.rotation);
		Rect r = rot.trNoNeg(style->rect);
		if (editor.nativeLook()) {
			pos          = Point(r.left - 3, r.top - 3);
			size.width   = max(size.width, r.width + 6);
			editorHeight = r.height + 6;
		} else {
			pos          = Point(r.left - 1, r.top - 1);
			size.width   = max(size.width, r.width + 2);
			editorHeight = r.height;
		}
	} else if (parent_menu) {
		parent_height = -item_height - 1;
	}
	*/
	// move & resize
	item_size.width = size.GetWidth() - marginW * 2;
	SetSize(size);
	Position(pos, wxSize(0, parent_height));
	// set event handler
	if (!parent_menu) {
//		Window* parent = wxGetTopLevelParent(this);
//		parent->PushEventHandler(&hider);
	}
	// show
//	oldSelectedItem = selectedItem;

	if (selected_item == NO_SELECTION && itemCount() > 0) selected_item = 0; // select first item by default
	mouse_down = false;
	Window::Show();
	// fix drop down arrow
	redrawArrowOnParent();
}

void DropDownList::hide(bool event) {
	// hide root
	DropDownList* root = this;
	while (root->parent_menu) {
		root = root->parent_menu;
	}
	root->realHide();
	// send event
	if (event && selected_item != NO_SELECTION) select(selected_item);
}

void DropDownList::realHide() {
	if (!IsShown()) return;
	Window::Hide();
//	onHide();
	hideSubMenu();
	if (parent_menu) {
		parent_menu->open_sub_menu = 0;
	} else {
//		redrawDropDownArrowOnParent();
		// disconnect event handler
//		Window* parent = getTopLevelParent(&editor);
//		parent->RemoveEventHandler(&hider);
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
	DropDownList* sub_menu = item == NO_SELECTION ? nullptr : popup(item);
	if (sub_menu == open_sub_menu) return sub_menu; // no change
	hideSubMenu();
	open_sub_menu = sub_menu;
	if (!sub_menu) return false;
	// open new menu
	wxSize size = GetSize();
	sub_menu->show(true,
		sub_menu->GetParent()->ScreenToClient(ClientToScreen(
			wxPoint(size.GetWidth() - 1, y + item_size.height)
		)));
	return true;
}

int DropDownList::itemPosition(size_t item) const {
	int y = marginH;
	size_t count = itemCount();
	for (size_t i = 0 ; i < count ; ++i) {
		if (i == item) return y;
		y += item_size.height + lineBelow(item);
	}
	// not found
	assert(false);
	return 0;
}

void DropDownList::redrawArrowOnParent() {
	if (viewer) {
		// TODO
	}
}

// ----------------------------------------------------------------------------- : DropDownList : Drawing

void DropDownList::onPaint(wxPaintEvent&) {
	wxBufferedPaintDC dc(this);
	dc.BeginDrawing();
	draw(dc);
	dc.EndDrawing();
}

void DropDownList::draw(DC& dc) {
	// Size
	wxSize cs = GetClientSize();
	// Draw background & frame
	dc.SetPen  (wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWFRAME));
	dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	dc.DrawRectangle(0, 0, cs.GetWidth(), cs.GetHeight());
	dc.SetFont(*wxNORMAL_FONT);
	// Draw items
	int y = marginH;
	size_t count = itemCount();
	for (size_t i = 0 ; i < count ; ++i) {
		drawItem(dc, y, i);
		y += item_size.height + lineBelow(i);
	}
}

void DropDownList::drawItem(DC& dc, int y, size_t item) {
	// draw background
	dc.SetPen(*wxTRANSPARENT_PEN);
	if (item == selected_item) {
		dc.SetBrush         (wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
		dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
		dc.DrawRectangle(marginW, y, item_size.width, item_size.height);
	} else if (highlightItem(item)) {
		// mix a color between selection and window
		dc.SetBrush         (lerp(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT),
		                          wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW), 0.75));
		dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
		dc.DrawRectangle(marginW, y, item_size.width, item_size.height);
	} else {
		dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
	}
	// draw text and icon
	drawIcon(dc, marginW, y, item, item == selected_item);	
	dc.DrawText(capitalize(itemText(item)), marginW + icon_size.width, y + text_offset);
	// draw popup icon
	if (popup(item)) {
		draw_menu_arrow(this, dc, wxRect(marginW, y, item_size.width, item_size.height), item == selected_item);
	}
	// draw line below
	if (lineBelow(item)) {
		dc.SetPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW));
		dc.DrawLine(marginW, y + item_size.height, marginW + item_size.width, y + item_size.height);
	}
}

// ----------------------------------------------------------------------------- : DropDownList : Events

void DropDownList::onLeftDown(wxMouseEvent&) {
	mouse_down = true; // prevent closing on mouseup of the click that opened the window
}

void DropDownList::onLeftUp(wxMouseEvent&) {
	if (mouse_down) {
		// don't hide if there is a child menu
		if (selected_item != NO_SELECTION && popup(selected_item)) return;
		hide(true);
	}
}

void DropDownList::onMotion(wxMouseEvent& ev) {
	// size
	wxSize cs = GetClientSize();
	// find selected item
	if (ev.GetX() < marginW || ev.GetX() + marginW >= cs.GetWidth() || ev.GetY() < marginH || ev.GetY() + marginH >= cs.GetHeight()) return;
	int startY = marginH;
	size_t count = itemCount();
	for (size_t i = 0 ; i < count ; ++i) {
		int endY = startY + item_size.height;
		if (ev.GetY() >= startY && ev.GetY() < endY) {
			selected_item = i;
			showSubMenu(i, startY);
			Refresh(false);
			return;
		}
		startY = endY + lineBelow(i);
	}
	hideSubMenu();
}

// ----------------------------------------------------------------------------- : DropDownList : Parent events

void DropDownList::onMouseInParent(wxMouseEvent& ev, bool open_in_place) {
	if (IsShown()) hide(false);
	else           show(open_in_place, wxPoint(ev.GetX(), ev.GetY()));
}

void DropDownList::onCharInParent(wxKeyEvent& ev) {
	// keyboard codes
	int k = ev.GetKeyCode();
	if (IsShown()) {
		if (open_sub_menu) {
			// sub menu always takes keys
			open_sub_menu->onCharInParent(ev);
		} else {
			switch (k) {
				case WXK_UP:
					if (selected_item - 1 >= 0) {
						selected_item -= 1;
						Refresh(false);
					}
					break;
				case WXK_DOWN:
					if (selected_item + 1 < itemCount()) {
						selected_item += 1;
						Refresh(false);
					}
					break;
				case WXK_RETURN:
					if (!showSubMenu()) {
						hide(true);
					}
					break;
				case WXK_ESCAPE:
					hide(false);
					break;
				case WXK_LEFT:
					if (parent_menu) hide(false);
					break;
				default:
					// match first character of an item, start searching just after the current selection
					size_t si = selected_item != NO_SELECTION ? selected_item + 1 : 0;
					size_t count = itemCount();
					for (size_t i = si ; i < count + si ; ++i) {
						String c = itemText(i);
						if (!c.empty() && toUpper(c.GetChar(0)) == toUpper(ev.GetUnicodeKey())) {
							// first character matches
							selected_item = i;
							showSubMenu();
							Refresh(false);
							return;
						}
					}
			}
		}
	} else if (k==WXK_SPACE || k==WXK_RETURN || k==WXK_DOWN) {
		// drop down list is not shown yet, show it now
		show(false, wxPoint(0,0));
	}
}

// ----------------------------------------------------------------------------- : DropDownList : Event table

// Note: some DropDownList events get sent to the parent which in turn should send them to the DropDownList
BEGIN_EVENT_TABLE(DropDownList,wxPopupWindow)
	EVT_PAINT        (DropDownList::onPaint)
	EVT_LEFT_DOWN    (DropDownList::onLeftDown)
	EVT_LEFT_UP      (DropDownList::onLeftUp)
	EVT_MOTION       (DropDownList::onMotion)
END_EVENT_TABLE  ()
