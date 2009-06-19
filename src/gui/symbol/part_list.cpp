//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2009 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/symbol/part_list.hpp>
#include <gui/symbol/selection.hpp>
#include <gui/util.hpp>
#include <data/action/symbol.hpp>
#include <data/action/symbol_part.hpp>
#include <gfx/gfx.hpp>
#include <render/symbol/filter.hpp>
#include <util/error.hpp>
#include <wx/dcbuffer.h>
#include <wx/caret.h>

DECLARE_TYPEOF_COLLECTION(SymbolPartP);

// ----------------------------------------------------------------------------- : Events

DEFINE_EVENT_TYPE(EVENT_PART_SELECT);
DEFINE_EVENT_TYPE(EVENT_PART_ACTIVATE);

// ----------------------------------------------------------------------------- : SymbolPartList


SymbolPartList::SymbolPartList(Window* parent, int id, SymbolPartsSelection& selection, SymbolP symbol)
	: wxScrolledWindow(parent, id, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER | wxVSCROLL)
	, selection(selection)
	, state_icons(9,8)
{
	SetScrollRate(0, ITEM_HEIGHT+1);
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
		updateParts(selection.get());
		return;
	}
	TYPE_CASE_(action, SymmetryTypeAction) {
		if (typing_in) cursor = typing_in->name.size(); // can change the name
		Refresh(false);
		return;
	}
	TYPE_CASE_(action, SymmetryCopiesAction) {
		if (typing_in) cursor = typing_in->name.size();
		Refresh(false);
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
	SetVirtualSize(110, number_of_items * (ITEM_HEIGHT + 1));
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
	int top; GetViewStart(0, &top);
	// find item under cursor
	if (ev.GetX() < 0 || ev.GetX() >= GetClientSize().x) return;
	int pos = top + ev.GetY() / (ITEM_HEIGHT + 1);
	SymbolPartP part = findItem(pos, ev.GetX());
	if (part) {
		// toggle/select
		selection.select(part, ev.ShiftDown() ? SELECT_TOGGLE : SELECT_OVERRIDE);
		if (!ev.ShiftDown() && selection.selected(part)) {
			// drag item
			drag = part;
			findParent(*part, drag_parent, drag_position);
			drop_parent = SymbolGroupP();
			CaptureMouse();
		}
		sendEvent(EVENT_PART_SELECT);
		Refresh(false);
	}
	ev.Skip(); // for focus
}
void SymbolPartList::onLeftUp(wxMouseEvent& ev) {
	if (HasCapture()) ReleaseMouse();
	if (drag_parent && drop_parent) {
		// move part
		if (drag_parent == drop_parent && drag_position < drop_position) {
			drop_position -= 1; // adjust for removal of the dragged part
		}
		// move part
		SymbolGroupP par = drag_parent; drag_parent = SymbolGroupP();
		if (par != drop_parent || drag_position != drop_position) {
			if (par != drop_parent && par->parts.size() == 1 && !par->isSymbolSymmetry()) {
				// this leaves a group without elements, remove it
				findParent(*par, par, drag_position); // parent of the group
				symbol->actions.addAction(new UngroupReorderSymbolPartsAction(*par, drag_position, *drop_parent, drop_position));
			} else {
				symbol->actions.addAction(new ReorderSymbolPartsAction(*par, drag_position, *drop_parent, drop_position));
			}
		} else {
			Refresh(false);
		}
	} else {
		drag_parent = SymbolGroupP();
	}
}
void SymbolPartList::onMotion(wxMouseEvent& ev) {
	int top; GetViewStart(0, &top);
	if (drag_parent) {
		int pos = top + (ev.GetY() + ITEM_HEIGHT/2) / (ITEM_HEIGHT + 1);
		if (pos < 0) pos = 0;
		if (pos >= number_of_items) pos = number_of_items;
		bool before = ev.GetY() < (pos - top) * (ITEM_HEIGHT + 1);
		// old stuff
		SymbolGroupP old_drop_parent   = drop_parent;
		size_t       old_drop_position = drop_position;
		bool         old_drop_inside   = drop_inside;
		// find drop target
		drop_parent = SymbolGroupP();
		findDropTarget(symbol, pos, before);
		// the drop parent must be an ancestor or sibling of ancestor of the drag_parent
		// i.e. the drop parent's parent must be an ancestor of drag_parent
		if (drop_parent) {
			drop_inside = !drop_parent->isAncestor(*drag_parent);
			while(drop_parent != symbol) {
				// is drop_parent a sibling of an ancestor of drag_parent?
				SymbolGroupP drop_parent_parent;
				size_t       drop_parent_position;
				findParent(*drop_parent, drop_parent_parent, drop_parent_position);
				if (!drop_parent_parent->isAncestor(*drag_parent)) {
					// move up one level
					drop_parent   = drop_parent_parent;
					drop_position = drop_parent_position;
				} else {
					break;
				}
			}
			if (drop_parent == symbol) {
				drop_inside = false;
			}
		}
		// refresh?
		if (drop_parent != old_drop_parent || drop_position != old_drop_position || drop_inside != old_drop_inside) {
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
					symbol->actions.addAction(new SymbolPartNameAction(typing_in, new_name, cursor, cursor - 1));
				}
				break;
			case WXK_DELETE:
				if (cursor < typing_in->name.size()) {
					String new_name = typing_in->name;
					new_name.erase(cursor, 1);
					symbol->actions.addAction(new SymbolPartNameAction(typing_in, new_name, cursor, cursor));
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
					symbol->actions.addAction(new SymbolPartNameAction(typing_in, new_name, cursor, cursor + 1));
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

SymbolPartP SymbolPartList::findItem(int i, int x) const {
	FOR_EACH(p, symbol->parts) {
		SymbolPartP f = findItem(i, x, p);
		if (f) return f;
	}
	return SymbolPartP();
}
SymbolPartP SymbolPartList::findItem(int& i, int x, const SymbolPartP& part) {
	if (i < 0 ) return SymbolPartP();
	if (i == 0) return part;
	i -= 1;
	// sub item?
	if (SymbolGroup* g = part->isSymbolGroup()) {
		FOR_EACH(p, g->parts) {
			SymbolPartP f = findItem(i, x - 5, p);
			if (f) return x < 5 ? part : f; // clicked on bar at the left of group?
		}
	}
	return SymbolPartP();
}

void SymbolPartList::findParent(const SymbolPart& of, SymbolGroupP& parent_out, size_t& pos_out) {
	if (!findParent(of, symbol, parent_out, pos_out)) {
		throw InternalError(_("Symbol part without a parent"));
	}
}
bool SymbolPartList::findParent(const SymbolPart& of, const SymbolGroupP& g, SymbolGroupP& parent_out, size_t& pos_out) {
	if (!g) return false;
	for (size_t i = 0 ; i < g->parts.size() ; ++i) {
		if (g->parts[i].get() == &of) {
			parent_out = g;
			pos_out    = i;
			return true;
		}
		if (findParent(of, dynamic_pointer_cast<SymbolGroup>(g->parts[i]), parent_out, pos_out)) return true;
	}
	return false;
}

bool SymbolPartList::findDropTarget(const SymbolPartP& parent, int& i, bool before) {
	if (parent != symbol) --i;
	if (SymbolGroup* g = parent->isSymbolGroup()) {
		size_t pos = 0;
		FOR_EACH(p, g->parts) {
			if (i <= 0) {
				// drop before this part
				drop_parent   = static_pointer_cast<SymbolGroup>(parent);
				drop_position = pos;
				return true;
			}
			if (p == drag) {
				i -= childCount(p) + 1; // don't drop inside
			} else {
				if (findDropTarget(p, i, before)) {
					return true;
				}
			}
			++pos;
		}
		if (i <= 0 && (parent == symbol || before)) {
			// drop at the end
			drop_parent   = static_pointer_cast<SymbolGroup>(parent);
			drop_position = g->parts.size();
			return true;
		}
	}
	return false;
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
	wxSize size = piecewise_max(GetVirtualSize() + RealSize(0,ITEM_HEIGHT+1), GetClientSize());
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	dc.DrawRectangle(0,0,size.x,size.y);
	// items
	int i = 0;
	drawItem(dc, 0, i, false, symbol);
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
	bool active = selection.selected(part);
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
	wxPen line_pen = lerp(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW),
						wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT),0.5);
	if (part != symbol) {
		// draw item : name
		int h = dc.GetCharHeight();
		dc.DrawText(part->name, ITEM_HEIGHT + x + 3, y + (ITEM_HEIGHT - h) / 2);
		// draw item : icon
		dc.SetBrush(lerp(background,wxColour(0,128,0),0.7));
		dc.DrawRectangle(x,y,ITEM_HEIGHT,ITEM_HEIGHT);
		dc.DrawBitmap(symbolPreview(),     x, y);
		dc.DrawBitmap(itemPreview(i,part), x, y);
		// draw item : border
		dc.SetPen(line_pen);
		dc.DrawLine(x+ITEM_HEIGHT, y, x+ITEM_HEIGHT, y + ITEM_HEIGHT + 1); // line after image
		dc.DrawLine(x, y + ITEM_HEIGHT, size.x, y + ITEM_HEIGHT); // line below
		// update caret
		if (selection.size() == 1 && active) {
			updateCaret(dc, x + ITEM_HEIGHT + 3, y + (ITEM_HEIGHT - h) / 2, h, part);
		}
		// draw icon
		state_icons.Draw(part->icon(), dc, size.x - 10, y + 1);
		// move down
		i += 1;
	}
	// Draw children
	int child_x = part == symbol ? x : x + 5;
	if (SymbolGroup* g = part->isSymbolGroup()) {
		FOR_EACH(p, g->parts) drawItem(dc, child_x, i, active || parent_active, p);
	}
	// draw bar on the left?
	int old_y = y + ITEM_HEIGHT+1; // after part itself
	int new_y = i * (ITEM_HEIGHT + 1); // after children
	if (old_y != new_y && part != symbol) {
		dc.SetPen(line_pen);
		dc.SetBrush(background);
		dc.DrawRectangle(x-1,old_y-1,5+1,new_y-old_y+1);
	}
	// Drop indicator?
	if (drag_parent && drop_parent) {
		dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT), 3));
		if (drop_inside) {
			if (part == drop_parent) {
				// drop inside part
				dc.SetBrush(*wxTRANSPARENT_BRUSH);
				dc.DrawRectangle(x, y, w, new_y - y);
			}
		} else if (drop_position < drop_parent->parts.size()) {
			if (part == drop_parent->parts[drop_position]) {
				// drop before part
				dc.DrawLine(x,y,size.x,y);
				dc.DrawLine(x,y-3,x,y+3);
				dc.DrawLine(size.x-1,y-3,size.x-1,y+3);
			}
		} else {
			if (part == drop_parent) {
				// drop after part
				dc.DrawLine(child_x,new_y,size.x,new_y);
				dc.DrawLine(child_x,new_y-3,child_x,new_y+3);
				dc.DrawLine(size.x-1,new_y-3,size.x-1,new_y+3);
			}
		}
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
				img = render_symbol(sym, filter, 0.08, ITEM_HEIGHT * 4, ITEM_HEIGHT * 4, true);
				s->combine = SYMBOL_COMBINE_SUBTRACT;
			} else {
				img = render_symbol(sym, filter, 0.08, ITEM_HEIGHT * 4, ITEM_HEIGHT * 4, true);
			}
		} else {
			img = render_symbol(sym, filter, 0.08, ITEM_HEIGHT * 4, ITEM_HEIGHT * 4, true);
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
	int top; GetViewStart(0, &top);
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
	caret->Move(x+w, y - top*(ITEM_HEIGHT+1));
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
