//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/value/text.hpp>
#include <data/action/value.hpp>
#include <util/tagged_string.hpp>
#include <util/window_id.hpp>
#include <wx/clipbrd.h>
#include <wx/caret.h>

// ----------------------------------------------------------------------------- : TextValueEditorScrollBar

/// A scrollbar to scroll a TextValueEditor
/** implemented as the scrollbar of a Window because that functions better */
class TextValueEditorScrollBar : public wxWindow {
  public:
	TextValueEditorScrollBar(TextValueEditor& tve);
  private:
	DECLARE_EVENT_TABLE();
	TextValueEditor& tve;
	
	void onScroll(wxScrollWinEvent&);
	void onMotion(wxMouseEvent&);
};


TextValueEditorScrollBar::TextValueEditorScrollBar(TextValueEditor& tve)
	: wxWindow(&tve.editor(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxVSCROLL | wxALWAYS_SHOW_SB)
	, tve(tve)
{}

void TextValueEditorScrollBar::onScroll(wxScrollWinEvent& ev) {
	if (ev.GetOrientation() == wxVERTICAL) {
		tve.scrollTo(ev.GetPosition());
	}
}
void TextValueEditorScrollBar::onMotion(wxMouseEvent& ev) {
	tve.editor().SetCursor(*wxSTANDARD_CURSOR);
	ev.Skip();
}

BEGIN_EVENT_TABLE(TextValueEditorScrollBar, wxEvtHandler)
	EVT_SCROLLWIN    (TextValueEditorScrollBar::onScroll)
	EVT_MOTION       (TextValueEditorScrollBar::onMotion)
END_EVENT_TABLE  ()



// ----------------------------------------------------------------------------- : TextValueEditor

IMPLEMENT_VALUE_EDITOR(Text)
	, selection_start  (0), selection_end  (0)
	, selection_start_i(0), selection_end_i(0)
	, select_words(false)
	, scrollbar(nullptr), scroll_with_cursor(false)
{
	if (viewer.nativeLook() && field().multi_line) {
		scrollbar = new TextValueEditorScrollBar(*this);
	}
}

TextValueEditor::~TextValueEditor() {
	delete scrollbar;
}

// ----------------------------------------------------------------------------- : Mouse

void TextValueEditor::onLeftDown(const RealPoint& pos, wxMouseEvent& ev) {
	select_words = false;
	moveSelection(TYPE_INDEX, v.indexAt(style().getRotation().trInv(pos)), !ev.ShiftDown(), MOVE_MID);
}
void TextValueEditor::onLeftUp(const RealPoint& pos, wxMouseEvent&) {
	// TODO: lookup position of click?
}

void TextValueEditor::onMotion(const RealPoint& pos, wxMouseEvent& ev) {
	if (ev.LeftIsDown()) {
		size_t index = v.indexAt(style().getRotation().trInv(pos));
		if (select_words) {
			// on the left, swap start and end
			bool left = selection_end_i < selection_start_i;
			size_t next = nextWordBoundry(index);
			size_t prev = prevWordBoundry(index);
			if (( left && next > max(selection_start_i, selection_end_i)) ||
			    (!left && prev < min(selection_start_i, selection_end_i))) {
				left = !left;
				swap(selection_start_i, selection_end_i);
			}
			// TODO : still not quite right, requires a moveSelection function that moves start & end simultaniously
			moveSelection(TYPE_INDEX, left ? prev : next, false, MOVE_MID);
		} else {
			moveSelection(TYPE_INDEX, index, false, MOVE_MID);
		}
	}
}

void TextValueEditor::onLeftDClick(const RealPoint& pos, wxMouseEvent& ev) {
	select_words = true;
	size_t index = v.indexAt(style().getRotation().trInv(pos));
	moveSelection(TYPE_INDEX, prevWordBoundry(index), true, MOVE_MID);
	moveSelection(TYPE_INDEX, nextWordBoundry(index), false, MOVE_MID);
}

void TextValueEditor::onRightDown(const RealPoint& pos, wxMouseEvent& ev) {
	size_t index = v.indexAt(style().getRotation().trInv(pos));
	if (index < min(selection_start_i, selection_end_i) ||
		index > max(selection_start_i, selection_end_i)) {
		// only move cursor when outside selection
		moveSelection(TYPE_INDEX, index, !ev.ShiftDown(), MOVE_MID);
	}
}

// ----------------------------------------------------------------------------- : Keyboard

void TextValueEditor::onChar(wxKeyEvent& ev) {
	fixSelection();
	switch (ev.GetKeyCode()) {
		case WXK_LEFT:
			// move left (selection?)
			if (ev.ControlDown()) {
				moveSelection(TYPE_INDEX,  prevWordBoundry(selection_end_i),!ev.ShiftDown(), MOVE_LEFT);
			} else {
				moveSelection(TYPE_CURSOR, prevCharBoundry(selection_end),  !ev.ShiftDown(), MOVE_LEFT);
			}
			break;
		case WXK_RIGHT:
			// move left (selection?)
			if (ev.ControlDown()) {
				moveSelection(TYPE_INDEX,  nextWordBoundry(selection_end_i),!ev.ShiftDown(), MOVE_RIGHT);
			} else {
				moveSelection(TYPE_CURSOR, nextCharBoundry(selection_end),  !ev.ShiftDown(), MOVE_RIGHT);
			}
			break;
		case WXK_UP:
			moveSelection(TYPE_INDEX, v.moveLine(selection_end_i, -1), !ev.ShiftDown(), MOVE_LEFT);
			break;
		case WXK_DOWN:
			moveSelection(TYPE_INDEX, v.moveLine(selection_end_i, +1), !ev.ShiftDown(), MOVE_RIGHT);
			break;
		case WXK_HOME:
			// move to begining of line / all (if control)
			if (ev.ControlDown()) {
				moveSelection(TYPE_INDEX, 0,                            !ev.ShiftDown(), MOVE_LEFT);
			} else {
				moveSelection(TYPE_INDEX, v.lineStart(selection_end_i), !ev.ShiftDown(), MOVE_LEFT);
			}
			break;
		case WXK_END:
			// move to end of line / all (if control)
			if (ev.ControlDown()) {
				moveSelection(TYPE_INDEX, value().value().size(),   !ev.ShiftDown(), MOVE_RIGHT);
			} else {
				moveSelection(TYPE_INDEX, v.lineEnd(selection_end_i), !ev.ShiftDown(), MOVE_RIGHT);
			}
			break;
		case WXK_BACK:
			if (selection_start == selection_end) {
				// if no selection, select previous character
				moveSelectionNoRedraw(TYPE_CURSOR, prevCharBoundry(selection_end), false);
				if (selection_start == selection_end) {
					// Walk over a <sep> as if we are the LEFT key
					moveSelection(TYPE_CURSOR, prevCharBoundry(selection_end), true, MOVE_LEFT);
					return;
				}
			}
			replaceSelection(wxEmptyString, _("Backspace"));
			break;
		case WXK_DELETE:
			if (selection_start == selection_end) {
				// if no selection select next
				moveSelectionNoRedraw(TYPE_CURSOR, nextCharBoundry(selection_end), false);
				if (selection_start == selection_end) {
					// Walk over a <sep> as if we are the RIGHT key
					moveSelection(TYPE_CURSOR, nextCharBoundry(selection_end), true, MOVE_RIGHT);
				}
			}
			replaceSelection(wxEmptyString, _("Delete"));
			break;
		case WXK_RETURN:
			if (field().multi_line) {
				replaceSelection(_("\n"), _("Enter"));
			}
			break;
		default:
			if (ev.GetKeyCode() >= _(' ') && ev.GetKeyCode() == (int)ev.GetRawKeyCode()) {
				// TODO: Find a more correct way to determine normal characters,
				//       this might not work for internationalized input.
				//       It might also not be portable!
				replaceSelection(String(ev.GetUnicodeKey(), 1), _("Typing"));
			}
	}
}

// ----------------------------------------------------------------------------- : Other events

void TextValueEditor::onFocus() {
	showCaret();
}
void TextValueEditor::onLoseFocus() {
	// hide caret
	wxCaret* caret = editor().GetCaret();
	assert(caret);
	if (caret->IsVisible()) caret->Hide();
	// hide selection
	selection_start   = selection_end   = 0;
	selection_start_i = selection_end_i = 0;
}

bool TextValueEditor::onContextMenu(wxMenu& m, wxContextMenuEvent& ev) {
	// in a keword? => "reminder text" option
	size_t kwpos = in_tag(value().value(), _("<kw-"), selection_start_i, selection_start_i);
	if (kwpos != String::npos) {
		Char c = String(value().value()).GetChar(kwpos + 4);
		m.AppendSeparator();
		m.AppendCheckItem(ID_FORMAT_REMINDER, _("&Reminder text"), _("Show or hide reminder text for this keyword"));
		m.Check(ID_FORMAT_REMINDER, c == _('1') || c == _('A')); // reminder text currently shown
	}
	// always show the menu
	return true;
}
void TextValueEditor::onMenu(wxCommandEvent& ev) {
	if (ev.GetId() == ID_FORMAT_REMINDER) {
		// toggle reminder text
		size_t kwpos = in_tag(value().value(), _("<kw-"), selection_start_i, selection_start_i);
		if (kwpos != String::npos) {
//			getSet().actions.add(new TextToggleReminderAction(value, kwpos));
		}
	} else {
		ev.Skip();
	}
}

// ----------------------------------------------------------------------------- : Other overrides

void TextValueEditor::draw(RotatedDC& dc) {
	// update scrollbar
	prepareDrawScrollbar(dc);
	// draw text
	TextValueViewer::draw(dc);
	// draw selection
	if (isCurrent()) {
		v.drawSelection(dc, style(), selection_start_i, selection_end_i);
		// show caret, onAction() would be a better place
		// but it has to be done after the viewer has updated the TextViewer
		// we could do that ourselfs, but we need a dc for that
		fixSelection();
		showCaret();
	}
}

wxCursor rotated_ibeam;

wxCursor TextValueEditor::cursor() const {
	if (viewer.getRotation().sideways() ^ style().getRotation().sideways()) { // 90 or 270 degrees
		if (!rotated_ibeam.Ok()) {
			rotated_ibeam = wxCursor(_("CUR_ROT_IBEAM"));
		}
		return rotated_ibeam;
	} else {
		return wxCURSOR_IBEAM;
	}
}

void TextValueEditor::onValueChange() {
	TextValueViewer::onValueChange();
	selection_start   = selection_end   = 0;
	selection_start_i = selection_end_i = 0;
}

void TextValueEditor::onAction(const Action& action, bool undone) {
	TextValueViewer::onAction(action, undone);
	TYPE_CASE(action, TextValueAction) {
		selection_start_i = action.selection_start;
		selection_end_i   = action.selection_end;
		fixSelection(TYPE_INDEX);
	}
}

// ----------------------------------------------------------------------------- : Clipboard

bool TextValueEditor::canPaste() const {
	return wxTheClipboard->IsSupported(wxDF_TEXT);
}

bool TextValueEditor::canCopy() const {
	return selection_start != selection_end; // text is selected
}

bool TextValueEditor::doPaste() {
	// get data
	if (!wxTheClipboard->Open()) return false;
	wxTextDataObject data;
	bool ok = wxTheClipboard->GetData(data);
	wxTheClipboard->Close();
	if (!ok) return false;
	// paste
	replaceSelection(escape(data.GetText()), _("Paste"));
	return true;
}

bool TextValueEditor::doCopy() {
	// determine string to store
	if (selection_start_i > value().value().size()) selection_start_i = value().value().size();
	if (selection_end_i   > value().value().size()) selection_end_i   = value().value().size();
	size_t start = min(selection_start_i, selection_end_i);
	size_t end   = max(selection_start_i, selection_end_i);
	String str = untag(value().value().substr(start, end - start));
	if (str.empty()) return false; // no data to copy
	// set data
	if (!wxTheClipboard->Open()) return false;
	bool ok = wxTheClipboard->SetData(new wxTextDataObject(str));
	wxTheClipboard->Close();
	return ok;
}

bool TextValueEditor::doDelete() {
	replaceSelection(wxEmptyString, _("Cut"));
	return true;
}

// ----------------------------------------------------------------------------- : Formatting

bool TextValueEditor::canFormat(int type) const {
	switch (type) {
		case ID_FORMAT_BOLD: case ID_FORMAT_ITALIC:
			return !style().always_symbol && style().allow_formating;
		case ID_FORMAT_SYMBOL:
			return !style().always_symbol && style().allow_formating && style().symbol_font.valid();
		case ID_FORMAT_REMINDER:
			return false; // TODO
		default:
			return false;
	}
}

bool TextValueEditor::hasFormat(int type) const {
	switch (type) {
		case ID_FORMAT_BOLD:
			return in_tag(value().value(), _("<b"),   selection_start_i, selection_end_i) != String::npos;
		case ID_FORMAT_ITALIC:
			return in_tag(value().value(), _("<i"),   selection_start_i, selection_end_i) != String::npos;
		case ID_FORMAT_SYMBOL:
			return in_tag(value().value(), _("<sym"), selection_start_i, selection_end_i) != String::npos;
		case ID_FORMAT_REMINDER:
			return false; // TODO
		default:
			return false;
	}
}

void TextValueEditor::doFormat(int type) {
	size_t ss = selection_start, se = selection_end;
	switch (type) {
		case ID_FORMAT_BOLD: {
			getSet().actions.add(toggle_format_action(valueP(), _("b"),   selection_start_i, selection_end_i, _("Bold")));
			break;
		}
		case ID_FORMAT_ITALIC: {
			getSet().actions.add(toggle_format_action(valueP(), _("i"),   selection_start_i, selection_end_i, _("Italic")));
			break;
		}
		case ID_FORMAT_SYMBOL: {
			getSet().actions.add(toggle_format_action(valueP(), _("sym"), selection_start_i, selection_end_i, _("Symbols")));
			break;
		}
	}
	selection_start = ss;
	selection_end   = se;
	fixSelection();
}

// ----------------------------------------------------------------------------- : Selection

void TextValueEditor::showCaret() {
	// Rotation
	Rotation rot(viewer.getRotation());
	Rotater rot2(rot, style().getRotation());
	// The caret
	wxCaret* caret = editor().GetCaret();
	// cursor rectangle
	RealRect cursor = v.charRect(selection_end_i);
	cursor.width = 0;
	// height may be 0 near a <line>
	// it is not 0 for empty text, because TextRenderer handles that case
	if (cursor.height == 0) {
		if (style().always_symbol && style().symbol_font.valid()) {
			RealSize s = style().symbol_font.font->defaultSymbolSize(viewer.getContext(), rot.trS(style().symbol_font.size));
			cursor.height = s.height;
		} else {
			cursor.height = v.heightOfLastLine();
			if (cursor.height == 0) {
				wxClientDC dc(&editor());
				// TODO : high quality?
				dc.SetFont(style().font.font);
				int hi;
				dc.GetTextExtent(_(" "), 0, &hi);
				cursor.height = rot.trS(hi);
			}
		}
	}
	// clip caret pos and size; show caret
	if (nativeLook()) {
		if (cursor.y + cursor.height <= 0 || cursor.y >= style().height) {
			// caret should be hidden
			if (caret->IsVisible()) caret->Hide();
			return;
		} else if (cursor.y < 0) {
			// caret partially hidden, clip
			cursor.height -= -cursor.y;
			cursor.y = 0;
		} else if (cursor.y + cursor.height >= style().height) {
			// caret partially hidden, clip
			cursor.height = style().height - cursor.y;
		}
	}
	// rotate
	cursor = rot.tr(cursor);
	// set size
	wxSize size = cursor.size();
	if (size.GetWidth()  == 0) size.SetWidth (1);
	if (size.GetHeight() == 0) size.SetHeight(1);
	// resize, move, show
	if (size != caret->GetSize()) {
		caret->SetSize(size);
	}
	caret->Move(cursor.position());
	if (!caret->IsVisible()) caret->Show();
}

void TextValueEditor::replaceSelection(const String& replacement, const String& name) {
	if (replacement.empty() && selection_start == selection_end) {
		// no text selected, nothing to delete
		return;
	}
	// fix the selection, it may be changed by undo/redo
	if (selection_end < selection_start) swap(selection_end, selection_start);
	fixSelection();
	// execute the action before adding it to the stack,
	// because we want to run scripts before action listeners see the action
	ValueAction* action = typing_action(valueP(), selection_start_i, selection_end_i, replacement, name);
	if (!action) {
		// nothing changes, but move the selection anyway
		moveSelection(TYPE_CURSOR, selection_start);
		return;
	}
	// perform the action
	// NOTE: this calls our onAction, invalidating the text viewer and moving the selection around the new text
	getSet().actions.add(action);
	// move cursor
	if (field().move_cursor_with_sort && replacement.size() == 1) {
		String val = value().value();
		Char typed  = replacement.GetChar(0);
		Char typedU = toUpper(typed);
		Char cur    = val.GetChar(selection_start_i);
		// the cursor may have moved because of sorting...
		// is 'replacement' just after the current cursor?
		if (selection_start_i >= 0 && selection_start_i < val.size() && (cur == typed || cur == typedU)) {
			// no need to move cursor in a special way
			selection_end_i = selection_start_i = min(selection_end_i, selection_start_i) + 1;
		} else {
			// find the last occurence of 'replacement' in the value
			size_t pos = val.find_last_of(typed);
			if (pos == String::npos) {
				// try upper case
				pos = val.find_last_of(typedU);
			}
			if (pos != String::npos) {
				selection_end_i = selection_start_i = pos + 1;
			} else {
				selection_end_i = selection_start_i;
			}
		}
	} else {
		selection_end_i = selection_start_i = min(selection_end_i, selection_start_i) + replacement.size();
	}
	fixSelection(TYPE_INDEX, MOVE_MID);
	// scroll with next update
	scroll_with_cursor = true;
}

void TextValueEditor::moveSelection(IndexType t, size_t new_end, bool also_move_start, Movement dir) {
	if (!isCurrent()) {
		// selection is only visible for curent editor, we can do a move the simple way
		moveSelectionNoRedraw(t, new_end, also_move_start, dir);
		return;
	}
	// Hide caret
	wxCaret* caret = editor().GetCaret();
	if (caret->IsVisible()) caret->Hide();
	// Destroy the clientDC before reshowing the caret, prevent flicker on MSW
	{
		// Move selection
		shared_ptr<DC> dc = editor().overdrawDC();
		RotatedDC rdc(*dc, viewer.getRotation(), false);
		if (nativeLook()) {
			// clip the dc to the region of this control
			rdc.SetClippingRegion(style().getRect());
		}
		// clear old selection by drawing it again
		v.drawSelection(rdc, style(), selection_start_i, selection_end_i);
		// move
		moveSelectionNoRedraw(t, new_end, also_move_start, dir);
		// scroll?
		scroll_with_cursor = true;
		if (ensureCaretVisible()) {
			// we can't redraw just the selection because we must scroll
			updateScrollbar();
			redraw();
		} else {
			// draw new selection
			v.drawSelection(rdc, style(), selection_start_i, selection_end_i);
		}
	}
	showCaret();
}

void TextValueEditor::moveSelectionNoRedraw(IndexType t, size_t new_end, bool also_move_start, Movement dir) {
	if (t == TYPE_INDEX) {
		selection_end_i = new_end;
		if (also_move_start) selection_start_i = selection_end_i;
	} else {
		selection_end = new_end;
		if (also_move_start) selection_start   = selection_end;
	}
	fixSelection(t, dir);
}

void TextValueEditor::fixSelection(IndexType t, Movement dir) {
	const String& val = value().value();
	// Which type takes precedent?
	if (t == TYPE_INDEX) {
		selection_start = index_to_cursor(value().value(), selection_start_i, dir);
		selection_end   = index_to_cursor(value().value(), selection_end_i,   dir);
	}
	// make sure the selection is at a valid position inside the text
	selection_start_i = cursor_to_index(val, selection_start, selection_start == selection_end ? MOVE_MID : MOVE_RIGHT);
	selection_end_i   = cursor_to_index(val, selection_end,   selection_start == selection_end ? MOVE_MID : MOVE_LEFT);
	// start and end must be on the same side of separators
	size_t seppos = val.find(_("<sep"));
	while (seppos != String::npos) {
		size_t sepend = skip_tag(val,match_close_tag(val, seppos));
		if (selection_start_i <= seppos && selection_end_i > seppos) {
		    // not on same side, move selection end before sep
			selection_end   = index_to_cursor(val, seppos, dir);
			selection_end_i = cursor_to_index(val, selection_end, selection_start == selection_end ? MOVE_MID : MOVE_LEFT);
		} else if (selection_start_i >= sepend && selection_end_i < sepend) {
		    // not on same side, move selection end after sep
			selection_end   = index_to_cursor(val, sepend, dir);
			selection_end_i = cursor_to_index(val, selection_end, selection_start == selection_end ? MOVE_MID : MOVE_LEFT);
		}
		// find next separator
		seppos = val.find(_("<sep"), seppos + 1);
	}
}


size_t TextValueEditor::prevCharBoundry(size_t pos) const {
	return max(0, (int)pos - 1);
}
size_t TextValueEditor::nextCharBoundry(size_t pos) const {
	return pos + 1;
}
size_t TextValueEditor::prevWordBoundry(size_t pos_i) const {
	const String& val = value().value();
	size_t p = val.find_last_not_of(_(" ,.:;()\n"), max(0, (int)pos_i - 1));
	if (p == String::npos) return 0;
	p = val.find_last_of(_(" ,.:;()\n"), p);
	if (p == String::npos) return 0;
	return p + 1;
}
size_t TextValueEditor::nextWordBoundry(size_t pos_i) const {
	const String& val = value().value();
	size_t p = val.find_first_of(_(" ,.:;()\n"), pos_i);
	if (p == String::npos) return val.size();
	p = val.find_first_not_of(_(" ,.:;()\n"), p);
	if (p == String::npos) return val.size();
	return p;
}

void TextValueEditor::select(size_t start, size_t end) {
	selection_start = start;
	selection_end   = end;
	// TODO : redraw?
}

size_t TextValueEditor::move(size_t pos, size_t start, size_t end, Movement dir) {
	if (dir == MOVE_LEFT)      return start;
	if (dir == MOVE_RIGHT)     return end;
	if (pos * 2 > start + end) return end; // past the middle
	else                       return start;
}

// ----------------------------------------------------------------------------- : Native look / scrollbar

void TextValueEditor::determineSize(bool force_fit) {
	if (!nativeLook()) return;
	style().angle = 0; // no rotation in nativeLook
	if (scrollbar) {
		// muliline, determine scrollbar size
		if (!force_fit) style().height = 100;
		int sbw = wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
		scrollbar->SetSize(
			style().left + style().width - sbw + 1,
			style().top - 1,
			sbw,
			style().height + 2);
		v.reset();
	} else {
		// Height depends on font
		wxMemoryDC dc;
		Bitmap bmp(1,1);
		dc.SelectObject(bmp);
		dc.SetFont(style().font.font);
		style().height = dc.GetCharHeight() + 2;
	}
}

void TextValueEditor::onShow(bool showing) {
	if (scrollbar) {
		// show/hide our scrollbar
		scrollbar->Show(showing);
	}
}

void TextValueEditor::onMouseWheel(const RealPoint& pos, wxMouseEvent& ev) {
	if (scrollbar) {
		int toScroll = ev.GetWheelRotation() * ev.GetLinesPerAction() / ev.GetWheelDelta(); // note: up is positive
		int target = min(max(scrollbar->GetScrollPos(wxVERTICAL) - toScroll, 0),
			             scrollbar->GetScrollRange(wxVERTICAL) - scrollbar->GetScrollThumb(wxVERTICAL));
		scrollTo(target);
	}
}

void TextValueEditor::scrollTo(int pos) {
	// scroll
	v.scrollTo(pos);
	// move the cursor if needed
	// refresh
	redraw();
}

bool TextValueEditor::ensureCaretVisible() {
	if (scrollbar && scroll_with_cursor) {
		scroll_with_cursor = false;
		return v.ensureVisible(style().height - style().padding_top - style().padding_bottom, selection_end_i);
	}
	return false;
}

void TextValueEditor::updateScrollbar() {
	assert(scrollbar);
	int position  = (int)v.firstVisibleLine();
	int page_size = (int)v.visibleLineCount(style().height - style().padding_top - style().padding_bottom);
	int range     = (int)v.lineCount();
	scrollbar->SetScrollbar(
		wxVERTICAL,
		position,
		page_size,
		range,
		page_size > 1 ? page_size - 1 : 0
	);
}

void TextValueEditor::prepareDrawScrollbar(RotatedDC& dc) {
	if (scrollbar) {
		// don't draw under the scrollbar
		int scrollbar_width = wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
		style().width.mutate() -= scrollbar_width;
		// prepare text, and remember scroll position
		double scroll_pos = v.getExactScrollPosition();
		v.prepare(dc, value().value(), style(), viewer.getContext());
		v.setExactScrollPosition(scroll_pos);
		// scroll to the same place, but always show the caret
		ensureCaretVisible();
		// update after scrolling
		updateScrollbar();
		style().width.mutate() += scrollbar_width;
	}
}