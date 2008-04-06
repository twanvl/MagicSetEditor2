//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/tree_list.hpp>
#include <gfx/gfx.hpp>
#include <wx/renderer.h>
#include <wx/dcbuffer.h>

DECLARE_TYPEOF_COLLECTION(TreeList::ItemP);

// ----------------------------------------------------------------------------- : TreeList : item managment

void TreeList::rebuild(bool full) {
	if (full) initItems();
	calcItemCount();
	UpdateScrollbar();
	Refresh(false);
}

bool TreeList::hasChildren(size_t item) const {
	return item + 1 < items.size() && items[item]->level < items[item+1]->level;
}

void TreeList::expand(size_t item, bool expand) {
	if (hasChildren(item) && items[item]->expanded != expand) {
		items[item]->expanded = expand;
		rebuild(false);
	}
}

void TreeList::select(size_t item, bool event) {
	if (item >= items.size() || selection == item) return;
	// select
	size_t oldpos = selection < items.size() ? items[selection]->position : 0;
	selection = item;
	size_t pos = items[selection]->position;
	// event
	if (event) {
		wxCommandEvent ev(wxEVT_COMMAND_LISTBOX_SELECTED, GetId());
		ProcessEvent(ev);
	}
	// redraw
	if (pos < first_line) {
		ScrollToLine(pos);
	} else if (pos >= first_line + visible_lines_t) {
		ScrollToLine(pos - visible_lines_t + 1);
	} else {
		if (oldpos != NOTHING) RefreshLine(oldpos);
		RefreshLine(pos);
	}
}

void TreeList::calcItemCount() {
	// item count
	total_lines = 0;
	int visible_level = 0;
	FOR_EACH(i,items) {
		if (i->level <= visible_level) {
			i->position = total_lines;
			++total_lines;
			if (i->expanded) visible_level = i->level + 1;
			else             visible_level = i->level;
		} else {
			i->position = NOTHING;
		}
	}
	// update lines
	UInt lines = 0;
	FOR_EACH_REVERSE(i,items) {
		if (i->visible()) {
			i->lines = lines;
			lines &= (1 << i->level) - 1;
			lines |= 1 << i->level;
		}
	}
	// selection hidden? move to first visible item before it
	if (selection < items.size()) {
		for ( ; selection + 1 > 0 ; --selection) {
			if (items[selection]->visible()) break; // visible
		}
		if (selection >= items.size()) selection = 0;
	}
}

size_t TreeList::findItemByPos(int y) const {
	if (y < header_height) return false;
	return findItem(first_line + (y - header_height) / item_height);
}
size_t TreeList::findItem(size_t line, size_t start) const {
	for (size_t i = start ; i < items.size() ; ++i) {
		if (items[i]->visible() && items[i]->position >= line) return i;
	}
	return items.size();
}
size_t TreeList::findLastItem(size_t start) const {
	for (size_t i = min(items.size(), start) - 1 ; i + 1 > 0 ; --i) {
		if (items[i]->visible()) return i;
	}
	return items.size();
}
size_t TreeList::findParent(size_t start) const {
	int level = items[start]->level;
	for (size_t i = start - 1 ; i + 1 > 0 ; --i) {
		if (items[i]->visible() && items[i]->level < level) return i;
	}
	return items.size();
}

// ----------------------------------------------------------------------------- : TreeList : UI

TreeList::TreeList(Window* parent, int id, long style)
	: wxPanel(parent, id, wxDefaultPosition, wxDefaultSize, style | wxWANTS_CHARS | wxVSCROLL)
	, selection(NOTHING)
	, total_lines(0)
	, first_line(0)
{
	// determine item size
	wxClientDC dc(this);
	dc.SetFont(*wxNORMAL_FONT);
	int h;
	dc.GetTextExtent(_("X"), 0, &h);
	item_height = h + 2;
}

void TreeList::onPaint(wxPaintEvent& ev) {
	wxBufferedPaintDC dc(this);
	size_t cols = columnCount();
	wxRendererNative& rn = wxRendererNative::GetDefault();
	// clear background
	wxSize cs = GetClientSize();
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	dc.DrawRectangle(0,0,cs.x,header_height);
	dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	dc.DrawRectangle(0,header_height,cs.x,cs.y-header_height);
	// draw header
	dc.SetFont(*wxNORMAL_FONT);
	dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
	int x = 0, y = 0;
	for (size_t j = 0 ; j < cols ; ++j) {
		int w = columnWidth(j);
		wxRect rect(x,0,w-1,header_height-1);
		rn.DrawHeaderButton(this, dc, rect);
		dc.DrawText(columnText(j),x+3,2);
		x += w;
	}
	y += header_height;
	// draw items
	size_t start = findItem(first_line);
	size_t end   = findItem(first_line + visible_lines);
	for (size_t i = start ; i < end ; ++i) {
		const Item& item = *items[i];
		if (!item.visible()) continue; // invisible
		x = level_width * (item.level + 1);
		// line below
		dc.SetPen(lerp(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW),
		               wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT),0.2));
		dc.DrawLine(x,y+item_height-1,cs.x,y+item_height-1);
		// draw lines
		dc.SetPen(lerp(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW),
		               wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT),0.4));
		for (int j = 0 ; j < 32 && j <= item.level ; ++j) {
			if (item.lines & (1 << j)) {
				// draw line
				dc.DrawLine(8 + level_width*j,y,8 + level_width*j,y+item_height);
			}
		}
		dc.DrawLine(x-1, y + item_height/2, x - 9, y + item_height/2);
		if (!(item.lines & (1 << item.level))) {
			dc.DrawLine(8 + level_width*item.level,y,8 + level_width*item.level,y+item_height/2+1);
		}
		// draw expand button
		if (hasChildren(i)) {
			wxRect rect(x - 13, y + (item_height - 9)/2, 9, 9);
			rn.DrawTreeItemButton(this, dc, rect, item.expanded ? wxCONTROL_EXPANDED : 0);
		}
		if (selection == i) {
			dc.SetPen(*wxTRANSPARENT_PEN);
			dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
			dc.DrawRectangle(x, y, cs.x-x, item_height-1);
		}
		// draw text(s)
		for (size_t j = 0 ; j < cols ; ++j) {
			int w = columnWidth(j);
			drawItem(dc, i, j, x+1, y, selection == i);
			if (j == 0) x = 0;
			x += w;
		}
		y += item_height;
	}
}

void TreeList::onChar(wxKeyEvent& ev) {
	switch (ev.GetKeyCode()) {
		case WXK_UP: {
			select(findLastItem(selection));
			break;
		} case WXK_DOWN: {
			select(findItem(0, selection + 1));
			break;
		} case WXK_LEFT: {
			if (selection < items.size()) {
				if (hasChildren(selection) && items[selection]->expanded) {
					expand(selection, false);
				} else {
					// select parent
					select(findParent(selection));
				}
			}
			break;
		} case WXK_RIGHT: {
			if (selection < items.size() && hasChildren(selection)) {
				if (items[selection]->expanded) {
					// select first child
					select(selection+1);
					Refresh(false);
				} else {
					expand(selection, true);
				}
			}
			break;
		} case WXK_PAGEUP: {
			ScrollToLine(first_line > visible_lines_t ? first_line - visible_lines_t : 0);
			break;
		} case WXK_PAGEDOWN: {
			ScrollToLine(first_line + visible_lines_t);
			break;
		} case WXK_HOME: {
			select(findItem(0));
			break;
		} case WXK_END: {
			select(findLastItem(items.size()));
			break;
		} case WXK_TAB: {
			// send a navigation event to our parent, to select another control
			// we need this because of wxWANTS_CHARS
			wxNavigationKeyEvent nev;
			nev.SetDirection(!ev.ShiftDown());
			GetParent()->ProcessEvent(nev);
			break;
		}
	}
}

void TreeList::onLeftDown(wxMouseEvent& ev) {
	size_t i = findItemByPos(ev.GetY());
	if (i >= items.size()) return;
	int left = items[i]->level * level_width;
	if (hasChildren(i) && ev.GetX() >= left && ev.GetX() < left + level_width) {
		expand(i, !items[i]->expanded);
	} else {
		select(i);
	}
	ev.Skip();
}

void TreeList::onLeftDClick(wxMouseEvent& ev) {
	size_t i = findItemByPos(ev.GetY());
	if (i >= items.size()) return;
	if (hasChildren(i)) {
		expand(i, !items[i]->expanded);
	}
	ev.Skip();
}

// ----------------------------------------------------------------------------- : TreeList : Copy of VScrolledWindow

void TreeList::ScrollToLine(size_t line) {
    // determine the real first line to scroll to: we shouldn't scroll beyond the end
    line = (size_t)max((int)line, 0);
    line = (size_t)min((int)line, max(0, (int)(total_lines - visible_lines_t)));
	// nothing to do?
    if (line == first_line) return;
    first_line = line;
    UpdateScrollbar();
	Refresh(false);
}

void TreeList::UpdateScrollbar() {
    // how many lines fit on the screen?
    int h = GetClientSize().y - header_height;
	visible_lines   = (h + item_height - 1) / item_height;
	visible_lines_t = h / item_height;
	// set the scrollbar parameters to reflect this
    SetScrollbar(wxVERTICAL, (int)first_line, (int)visible_lines_t, (int)total_lines);
}

void TreeList::RefreshLine(size_t line) {
    if (line < first_line || line >= first_line + visible_lines) return;
    // calculate the rect occupied by this line on screen
    wxRect rect;
    rect.x      = 0;
    rect.y      = header_height + (int)(line - first_line) * item_height;
    rect.width  = GetClientSize().x;
    rect.height = item_height;
    // do refresh it
    RefreshRect(rect, false);
}

void TreeList::onScroll(wxScrollWinEvent& ev) {
    wxEventType type = ev.GetEventType();
    if (type == wxEVT_SCROLLWIN_TOP) {
		ScrollToLine(0);
	} else if (type == wxEVT_SCROLLWIN_BOTTOM) {
		ScrollToLine(total_lines);
	} else if (type == wxEVT_SCROLLWIN_LINEUP) {
		ScrollToLine(first_line > 0 ? first_line - 1 : 0);
	} else if (type == wxEVT_SCROLLWIN_LINEDOWN) {
		ScrollToLine(first_line + 1);
	} else if (type == wxEVT_SCROLLWIN_PAGEUP) {
		ScrollToLine(first_line > visible_lines_t ? first_line - visible_lines_t : 0);
	} else if (type == wxEVT_SCROLLWIN_PAGEDOWN) {
		ScrollToLine(first_line + visible_lines_t);
	} else {
		ScrollToLine(ev.GetPosition());
    }
}

void TreeList::onSize(wxSizeEvent& ev) {
	UpdateScrollbar();
	Refresh(false);
	ev.Skip();
}

void TreeList::onMouseWheel(wxMouseEvent& ev) {
	int delta = -ev.GetWheelRotation() * ev.GetLinesPerAction() / ev.GetWheelDelta();
	ScrollToLine(first_line + delta);
}


// ----------------------------------------------------------------------------- : TreeList : Event table


BEGIN_EVENT_TABLE(TreeList, wxPanel)
	EVT_PAINT      (TreeList::onPaint)
	EVT_SIZE       (TreeList::onSize)
	EVT_SCROLLWIN  (TreeList::onScroll)
	EVT_CHAR       (TreeList::onChar)
	EVT_LEFT_DOWN  (TreeList::onLeftDown)
	EVT_LEFT_DCLICK(TreeList::onLeftDClick)
	EVT_MOUSEWHEEL (TreeList::onMouseWheel)
END_EVENT_TABLE()

