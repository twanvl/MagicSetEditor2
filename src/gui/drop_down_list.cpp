//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/drop_down_list.hpp>
#include <gui/util.hpp>
#include <render/value/viewer.hpp>
#include <render/card/viewer.hpp>
#include <gui/value/editor.hpp>
#include <util/rotation.hpp>
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
		int t = ev.GetEventType();
		if ( t == wxEVT_LEFT_DOWN      || t == wxEVT_RIGHT_DOWN
		  || t == wxEVT_MOVE           || t == wxEVT_SIZE
		  || t == wxEVT_MENU_HIGHLIGHT || t == wxEVT_MENU_OPEN    || t == wxEVT_MENU_OPEN
		  || t == wxEVT_ACTIVATE       || t == wxEVT_CLOSE_WINDOW || t == wxEVT_KILL_FOCUS
		  || t == wxEVT_COMMAND_TOOL_CLICKED)
		{
			// close the list, and pass on the event
			// don't just use ev.Skip(), because this event handler will be removed by hiding,
			// so there will be no next handler to skip to
			wxEvtHandler* nh = GetNextHandler();
			list.hide(false);
			if (nh) nh->ProcessEvent(ev);
			return false;
		} else {
//			if (t !=10093 && t !=10098 && t !=10097 && t !=10099 && t !=10004 && t !=10062
//			 && t !=10025 && t !=10035 && t !=10034 && t !=10036 && t !=10042 && t !=10119)
//			{
//				t=t;//DEBUG
//			}
			return wxEvtHandler::ProcessEvent(ev);
		}
	}
};


// ----------------------------------------------------------------------------- : DropDownList : Show/Hide

DropDownList::DropDownList(Window* parent, bool is_submenu, ValueViewer* viewer)
	: wxPopupWindow(parent)
	, text_offset(1)
	, item_size(100,1)
	, icon_size(0,0)
	, selected_item(NO_SELECTION)
	, mouse_down(false)
	, open_sub_menu(nullptr)
	, parent_menu(nullptr)
	, viewer(viewer)
	, hider(is_submenu ? nullptr : new DropDownHider(*this))
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
	delete hider;
}

void DropDownList::show(bool in_place, wxPoint pos) {
	if (IsShown()) return;
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
	RealSize size(
		item_size.width + marginW * 2,
		item_size.height * count + marginH * 2 + line_count
	);
	int parent_height = 0;
	if (!in_place && viewer) {
		// Position the drop down list below the editor control (based on the style)
		RealRect r = viewer->viewer.getRotation().trNoNeg(viewer->getStyle()->getRect());
		if (viewer->viewer.nativeLook()) {
			pos           = RealPoint(r.x - 3, r.y - 3);
			size.width    = max(size.width, r.width + 6);
			parent_height = (int)r.height + 6;
		} else {
			pos           = RealPoint(r.x - 1, r.y - 1);
			size.width    = max(size.width, r.width + 2);
			parent_height = (int)r.height;
		}
	} else if (parent_menu) {
		parent_height = -(int)item_size.height - 1;
	}
	pos = GetParent()->ClientToScreen(pos);
	// move & resize
	item_size.width = size.width - marginW * 2;
	SetSize(size);
	Position(pos, wxSize(0, parent_height));
	// set event handler
	if (hider) {
		Window* parent = wxGetTopLevelParent(GetParent());
		parent->PushEventHandler(hider);
	}
	// show
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
		parent_menu->open_sub_menu = nullptr;
	} else {
		redrawArrowOnParent();
		// disconnect event handler
		Window* parent = wxGetTopLevelParent(GetParent());
		parent->RemoveEventHandler(hider);
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

int DropDownList::itemPosition(size_t item) const {
	int y = marginH;
	size_t count = itemCount();
	for (size_t i = 0 ; i < count ; ++i) {
		if (i == item) return y;
		y += (int)item_size.height + lineBelow(item);
	}
	// not found
	assert(false);
	return 0;
}

void DropDownList::redrawArrowOnParent() {
	if (viewer) {
		ValueEditor* e = viewer->getEditor();
		if (e) e->redraw();
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
		y += (int)item_size.height + lineBelow(i);
	}
}

void DropDownList::drawItem(DC& dc, int y, size_t item) {
	// draw background
	dc.SetPen(*wxTRANSPARENT_PEN);
	if (item == selected_item) {
		dc.SetBrush         (wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
		dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
		dc.DrawRectangle(marginW, y, (int)item_size.width, (int)item_size.height);
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
	if (lineBelow(item)) {
		dc.SetPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW));
		dc.DrawLine(marginW, y + (int)item_size.height, marginW + (int)item_size.width, y + (int)item_size.height);
	}
}

// ----------------------------------------------------------------------------- : DropDownList : Events

void DropDownList::onLeftDown(wxMouseEvent&) {
	mouse_down = true; // prevent closing on mouseup of the click that opened the window
}

void DropDownList::onLeftUp(wxMouseEvent&) {
	if (mouse_down) {
		// don't hide if there is a child menu
		if (selected_item != NO_SELECTION && submenu(selected_item)) return;
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
		int endY = startY + (int)item_size.height;
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

bool DropDownList::onMouseInParent(wxMouseEvent& ev, bool open_in_place) {
	if (IsShown()) hide(false);
	else           show(open_in_place, wxPoint(ev.GetX(), ev.GetY()));
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
					if (selected_item > 0) {
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
					for (size_t i = 0 ; i < count ; ++i) {
						size_t index = (si + i) % count;
						String c = itemText(index);
#ifdef UNICODE
						if (!c.empty() && toUpper(c.GetChar(0)) == toUpper(ev.GetUnicodeKey())) {
#else
						if (!c.empty() && toUpper(c.GetChar(0)) == toUpper(ev.GetKeyCode())) {
#endif
							// first character matches
							selected_item = index;
							showSubMenu();
							Refresh(false);
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

// ----------------------------------------------------------------------------- : DropDownList : Event table

// Note: some DropDownList events get sent to the parent which in turn should send them to the DropDownList
BEGIN_EVENT_TABLE(DropDownList,wxPopupWindow)
	EVT_PAINT        (DropDownList::onPaint)
	EVT_LEFT_DOWN    (DropDownList::onLeftDown)
	EVT_LEFT_UP      (DropDownList::onLeftUp)
	EVT_MOTION       (DropDownList::onMotion)
END_EVENT_TABLE  ()
