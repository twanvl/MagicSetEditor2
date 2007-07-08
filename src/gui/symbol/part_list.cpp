//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/symbol/part_list.hpp>
#include <gui/util.hpp>
#include <data/action/symbol.hpp>
#include <gfx/gfx.hpp>
#include <render/symbol/filter.hpp>
#include <wx/dcbuffer.h>
#include <wx/caret.h>

DECLARE_TYPEOF_COLLECTION(SymbolPartP);

// ----------------------------------------------------------------------------- : Events

DEFINE_EVENT_TYPE(EVENT_PART_SELECT);
DEFINE_EVENT_TYPE(EVENT_PART_ACTIVATE);

// ----------------------------------------------------------------------------- : SymbolPartList


SymbolPartList::SymbolPartList(Window* parent, int id, set<SymbolPartP>& selection, SymbolP symbol)
	: wxScrolledWindow(parent, id, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER | wxVSCROLL)
	, selection(selection)
	, state_icons(9,8)
{
	// NOTE: this is based on the order of the SymbolShapeCombine and SymbolSymmetryType enums!
	state_icons.Add(load_resource_image(_("icon_combine_merge")));
	state_icons.Add(load_resource_image(_("icon_combine_subtract")));
	state_icons.Add(load_resource_image(_("icon_combine_intersection")));
	state_icons.Add(load_resource_image(_("icon_combine_difference")));
	state_icons.Add(load_resource_image(_("icon_combine_overlap")));
	state_icons.Add(load_resource_image(_("icon_combine_border")));
	state_icons.Add(load_resource_image(_("icon_symmetry_rotation")));
	state_icons.Add(load_resource_image(_("icon_symmetry_reflection")));
	state_icons.Add(load_resource_image(_("icon_symbol_group")));
	// view symbol
	setSymbol(symbol);
}

void SymbolPartList::onChangeSymbol() {
	typing_in = SymbolPartP();
	cursor = 0;
	update();
}

void SymbolPartList::onAction(const Action& action, bool undone) {
	TYPE_CASE(action, SymbolPartNameAction) {
		if (typing_in == action.part) {
			cursor = action.new_cursor;
		}
		Refresh(false);
		if (true) return;
	}
	TYPE_CASE_(action, SymbolPartListAction) {
		update();
		return;
	}
	TYPE_CASE(action, SymbolPartsAction) {
		symbol_preview.up_to_date = false;
		updateParts(action.parts);
		if (true) return;
	}
	TYPE_CASE_(action, SymbolPartAction) {
		symbol_preview.up_to_date = false;
		// some part changed, but we don't know which one, assume it is the selection
		updateParts(selection);
		return;
	}
}

wxSize SymbolPartList::DoGetBestSize() const {
	wxSize ws = GetSize(), cs = GetClientSize();
	const int w = 110;
	const int h = ITEM_HEIGHT;
	return wxSize(w, h) + ws - cs;
}

void SymbolPartList::update() {
	// count items
	number_of_items = childCount(symbol);
	SetVirtualSize(110, number_of_items * (ITEM_HEIGHT+1));
	// invalidate previews
	symbol_preview.up_to_date = false;
	for (size_t i = 0 ; i < part_previews.size() ; ++i) {
		part_previews[i].up_to_date = false;
	}
	// refresh
	Refresh(false);
}
void SymbolPartList::updateParts(const set<SymbolPartP>& parts) {
	symbol_preview.up_to_date = false;
	int i = 0;
	FOR_EACH(p, symbol->parts) {
		updatePart(parts, i, false, p);
	}
	// refresh
	Refresh(false);
}
void SymbolPartList::updatePart(const set<SymbolPartP>& parts, int& i, bool parent_updated, const SymbolPartP& part) {
	bool update = parent_updated || parts.find(part) != parts.end();
	if (update) {
		part_previews[i].up_to_date = false;
	}
	++i;
	if (SymbolGroup* g = part->isSymbolGroup()) {
		FOR_EACH(p, g->parts) {
			updatePart(parts, i, update, p);
		}
	}
}

// ----------------------------------------------------------------------------- : Events

void SymbolPartList::onLeftDown(wxMouseEvent& ev) {
	// find item under cursor
	if (ev.GetX() < 0 || ev.GetX() >= GetClientSize().x) return;
	SymbolPartP part = findItem(ev.GetY() / (ITEM_HEIGHT + 1));
	if (part) {
		// toggle/select
		if (!ev.ShiftDown()) {
			selection.clear();
		}
		if (selection.find(part) != selection.end()) {
			selection.erase(part);
		} else {
			selection.insert(part);
		}
		if (!ev.ShiftDown()) {
			// drag item
			mouse_down_on = part;
			drop_position = -1;
			CaptureMouse();
		}
		sendEvent(EVENT_PART_SELECT);
		Refresh(false);
	}
	ev.Skip(); // for focus
}
void SymbolPartList::onLeftUp(wxMouseEvent& ev) {
	if (HasCapture()) ReleaseMouse();
	if (mouse_down_on && drop_position != -1) {
		// move part
		// find old position
		vector<SymbolPartP>::const_iterator it = find(symbol->parts.begin(), symbol->parts.end(), mouse_down_on);
		mouse_down_on = SymbolPartP();
		if (it == symbol->parts.end()) {
			Refresh(false);
			return;
		}
		size_t old_position = it - symbol->parts.begin();
		// find new position
		size_t new_position;
		SymbolPartP drop_before = findItem(drop_position);
		it = find(symbol->parts.begin(), symbol->parts.end(), drop_before);
		if (it == symbol->parts.end()) {
			new_position = number_of_items - 1;
		} else {
			new_position = it - symbol->parts.begin();
			if (old_position < new_position) new_position -= 1;
		}
		// move part
		if (old_position != new_position) {
			symbol->actions.add(new ReorderSymbolPartsAction(*symbol, old_position, new_position));
		} else {
			Refresh(false);
		}
	} else {
		mouse_down_on = SymbolPartP();
	}
}
void SymbolPartList::onMotion(wxMouseEvent& ev) {
	if (mouse_down_on) {
		int new_drop_position = (ev.GetY() + ITEM_HEIGHT/2) / (ITEM_HEIGHT + 1);
		if (new_drop_position < 0 || new_drop_position > number_of_items) new_drop_position = -1;
		// TODO: make sure it is not in a group
		if (drop_position != new_drop_position) {
			drop_position = new_drop_position;
			Refresh(false);
		}
	}
}

void SymbolPartList::onLeftDClick(wxMouseEvent& ev) {
	// double click = activate
	sendEvent(EVENT_PART_ACTIVATE);
}

void SymbolPartList::onChar(wxKeyEvent& ev) {
	if (typing_in) {
		// Typing in name
		String new_name = typing_in->name;
		switch (ev.GetKeyCode()) {
			case WXK_HOME:
				if (cursor != 0) {
					cursor = 0; Refresh(false); // TODO: without refresh
				}
				break;
			case WXK_END:
				if (cursor != typing_in->name.size()) {
					cursor = typing_in->name.size(); Refresh(false);
				}
				break;
			case WXK_LEFT:
				if (cursor > 0) {
					--cursor; Refresh(false);
				}
				break;
			case WXK_RIGHT:
				if (cursor < typing_in->name.size()) {
					++cursor; Refresh(false);
				}
				break;
			case WXK_BACK:
				if (cursor > 0 && cursor <= typing_in->name.size()) {
					String new_name = typing_in->name;
					new_name.erase(cursor - 1, 1);
					symbol->actions.add(new SymbolPartNameAction(typing_in, new_name, cursor, cursor - 1));
				}
				break;
			case WXK_DELETE:
				if (cursor < typing_in->name.size()) {
					String new_name = typing_in->name;
					new_name.erase(cursor, 1);
					symbol->actions.add(new SymbolPartNameAction(typing_in, new_name, cursor, cursor));
				}
				break;
			default:
			  // See gui/value/text.cpp
			  #ifdef __WXMSW__
				if (ev.GetKeyCode() >= _(' ') && ev.GetKeyCode() == (int)ev.GetRawKeyCode()) {
			  #else
				if (ev.GetKeyCode() >= _(' ') /*&& ev.GetKeyCode() == (int)ev.GetRawKeyCode()*/) {
			  #endif
					#ifdef UNICODE
						Char key = ev.GetUnicodeKey();
					#else
						Char key = (Char)ev.GetKeyCode();
					#endif
					String new_name = typing_in->name;
					new_name.insert(cursor, 1, key);
					symbol->actions.add(new SymbolPartNameAction(typing_in, new_name, cursor, cursor + 1));
				}
		}
	}
	// Move up/down?
}

void SymbolPartList::onSize(wxSizeEvent&) {
	Refresh(false);
}

void SymbolPartList::sendEvent(int type) {
	wxCommandEvent ev(type, GetId());
	ProcessEvent(ev);
}

// ----------------------------------------------------------------------------- : Items

SymbolPartP SymbolPartList::findItem(int i) const {
	FOR_EACH(p, symbol->parts) {
		SymbolPartP f = findItem(i, p);
		if (f) return f;
	}
	return SymbolPartP();
}
SymbolPartP SymbolPartList::findItem(int& i, const SymbolPartP& part) {
	if (i < 0 ) return SymbolPartP();
	if (i == 0) return part;
	i -= 1;
	// sub item?
	if (SymbolGroup* g = part->isSymbolGroup()) {
		FOR_EACH(p, g->parts) {
			if (findItem(i, p)) return part;
		}
	}
	return SymbolPartP();
}

int SymbolPartList::childCount(const SymbolPartP& part) {
	if (SymbolGroup* g = part->isSymbolGroup()) {
		int count = 0;
		FOR_EACH(p, g->parts) count += 1 + childCount(p);
		return count;
	} else {
		return 0;
	}
}

// ----------------------------------------------------------------------------- : Text editor


// ----------------------------------------------------------------------------- : Drawing


void SymbolPartList::onPaint(wxPaintEvent&) {
	wxBufferedPaintDC dc(this);
	DoPrepareDC(dc);
	OnDraw(dc);
}
void SymbolPartList::OnDraw(DC& dc) {
	// init
	dc.SetFont(*wxNORMAL_FONT);
	// clear background
	wxSize size = GetClientSize();
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	dc.DrawRectangle(0,0,size.x,size.y);
	// items
	int i = 0;
	FOR_EACH(p, symbol->parts) {
		drawItem(dc, 0, i, false, p);
	}
	// drag/drop indicator
	if (mouse_down_on && drop_position != -1) {
		dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT), 3));
		int y = drop_position * (ITEM_HEIGHT + 1) - 1;
		dc.DrawLine(0,y,size.x,y);
		dc.DrawLine(0,y-3,0,y+3);
		dc.DrawLine(size.x-1,y-3,size.x-1,y+3);
	}
	// hide caret
	if (selection.size() != 1) {
		typing_in = SymbolPartP();
		wxCaret* caret = GetCaret();
		if (caret) caret->Hide();
	}
}

void SymbolPartList::drawItem(DC& dc, int x, int& i, bool parent_active, const SymbolPartP& part) {
	wxSize size = GetClientSize();
	int w = size.x - x;
	int y = i * (ITEM_HEIGHT + 1);
	// draw item : highlight
	Color background;
	dc.SetPen(*wxTRANSPARENT_PEN);
	bool active = selection.find(part) != selection.end();
	if (active) {
		background = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
		dc.SetBrush(background);
		dc.DrawRectangle(x,y,w,ITEM_HEIGHT);
		dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
	} else if (parent_active) {
		background = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
		dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
	} else {
		background = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
		dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
	}
	// draw item : name
	int h = dc.GetCharHeight();
	dc.DrawText(part->name, ITEM_HEIGHT + x + 3, y + (ITEM_HEIGHT - h) / 2);
	// draw item : icon
	dc.SetBrush(lerp(background,wxColour(0,128,0),0.7));
	dc.DrawRectangle(x,y,ITEM_HEIGHT,ITEM_HEIGHT);
	dc.DrawBitmap(symbolPreview(),     x, y);
	dc.DrawBitmap(itemPreview(i,part), x, y);
	// draw item : border
	wxPen line_pen = lerp(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW),
	                      wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT),0.5);
	dc.SetPen(line_pen);
	dc.DrawLine(x+ITEM_HEIGHT, y, x+ITEM_HEIGHT, y + ITEM_HEIGHT + 1); // line after image
	dc.DrawLine(x, y + ITEM_HEIGHT, size.x, y + ITEM_HEIGHT); // line below
	// update caret
	if (selection.size() == 1 && active) {
		updateCaret(dc, x + ITEM_HEIGHT + 3, y + (ITEM_HEIGHT - h) / 2, h, part);
	}
	// move down
	i += 1;
	// draw more?
	if (SymbolShape* s = part->isSymbolShape()) {
		// combine state
		state_icons.Draw(s->combine, dc, size.x - 10, y + 1);
	} else if (SymbolSymmetry* s = part->isSymbolSymmetry()) {
		// kind of symmetry
		state_icons.Draw(s->kind, dc, size.x - 10, y + 1);
		// TODO: show clip mode?
	} else if (SymbolGroup* g = part->isSymbolGroup()) {
		state_icons.Draw(SYMMETRY_REFLECTION + 1, dc, size.x - 10, y + 1);
		FOR_EACH(p, g->parts) drawItem(dc, x + 5, i, active || parent_active, p);
		// draw bar on the left
		int new_y = i * (ITEM_HEIGHT + 1);
		y += ITEM_HEIGHT+1;
		if (y != new_y) {
			dc.SetPen(line_pen);
			dc.SetBrush(background);
			dc.DrawRectangle(x-1,y-1,5+1,new_y-y+1);
		}
	} else {
		throw "Unknown symbol part type";
	}
}

const Image& SymbolPartList::itemPreview(int i, const SymbolPartP& part) {
	if ((size_t)i >= part_previews.size()) part_previews.resize(i + 1);
	Preview& p = part_previews[i];
	if (!p.up_to_date) {
		SolidFillSymbolFilter filter(*wxBLACK, Color(255,128,128));
		// temporary symbol
		SymbolP sym(new Symbol); sym->parts.push_back(part);
		Image img;
		if (SymbolShape* s = part->isSymbolShape()) {
			if (s->combine == SYMBOL_COMBINE_SUBTRACT) {
				// temporarily render using subtract instead, otherwise we don't see anything
				s->combine = SYMBOL_COMBINE_BORDER;
				img = render_symbol(sym, filter, 0.08, ITEM_HEIGHT * 4);
				s->combine = SYMBOL_COMBINE_SUBTRACT;
			} else {
				img = render_symbol(sym, filter, 0.08, ITEM_HEIGHT * 4);
			}
		} else {
			img = render_symbol(sym, filter, 0.08, ITEM_HEIGHT * 4);
		}
		resample(img, p.image);
		p.up_to_date = true;
	}
	return p.image;
}
const Image& SymbolPartList::symbolPreview() {
	if (!symbol_preview.up_to_date) {
		SolidFillSymbolFilter filter(AColor(0,0,0,40), AColor(255,255,255,40));
		Image img = render_symbol(symbol, filter, 0.06, ITEM_HEIGHT * 4);
		resample(img, symbol_preview.image);
		symbol_preview.up_to_date = true;
	}
	return symbol_preview.image;
}

void SymbolPartList::updateCaret(DC& dc, int x, int y, int h, const SymbolPartP& part) {
	// make caret
	wxCaret* caret = GetCaret();
	if (!caret) {
		caret = new wxCaret(this, 1, h);
		SetCaret(caret);
	}
	// move caret
	if (typing_in != part) {
		typing_in = part;
		cursor = typing_in->name.size();
	}
	cursor = min(cursor, typing_in->name.size());
	int w;
	dc.GetTextExtent(typing_in->name.substr(0,cursor), &w, nullptr);
	caret->Move(x+w,y);
	if (!caret->IsVisible()) caret->Show();
}

// ----------------------------------------------------------------------------- : Event table

BEGIN_EVENT_TABLE(SymbolPartList, wxScrolledWindow)
	EVT_LEFT_DOWN      (SymbolPartList::onLeftDown)
	EVT_LEFT_DCLICK    (SymbolPartList::onLeftDClick)
	EVT_LEFT_UP        (SymbolPartList::onLeftUp)
	EVT_MOTION         (SymbolPartList::onMotion)
	EVT_CHAR           (SymbolPartList::onChar)
	EVT_PAINT          (SymbolPartList::onPaint)
	EVT_SIZE           (SymbolPartList::onSize)
END_EVENT_TABLE  ()
