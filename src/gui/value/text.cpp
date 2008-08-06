//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/text.hpp>
#include <gui/icon_menu.hpp>
#include <gui/util.hpp>
#include <gui/drop_down_list.hpp>
#include <data/word_list.hpp>
#include <data/game.hpp>
#include <data/settings.hpp>
#include <data/action/value.hpp>
#include <util/tagged_string.hpp>
#include <util/find_replace.hpp>
#include <util/window_id.hpp>
#include <wx/clipbrd.h>
#include <wx/caret.h>

#undef small // some evil windows header defines this

DECLARE_SHARED_POINTER_TYPE(DropDownList);
DECLARE_TYPEOF_COLLECTION(WordListP);
DECLARE_TYPEOF_COLLECTION(WordListWordP);
DECLARE_TYPEOF_COLLECTION(WordListPosP);
DECLARE_TYPEOF_COLLECTION(AutoReplaceP);
struct DropDownWordListItem;
DECLARE_TYPEOF_COLLECTION(DropDownWordListItem);
DECLARE_TYPEOF_COLLECTION(String);

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


// ----------------------------------------------------------------------------- : WordListPos

class WordListPos : public IntrusivePtrBase<WordListPos> {
  public:
	WordListPos(size_t start, size_t end, WordListP word_list)
		: start(start), end(end)
		, rect(-1,-1,-1,-1)
		, word_list(word_list)
	{}
	
	const size_t start, end; ///< Start and ending indices
	RealRect  rect;          ///< Rectangle around word list text
	WordListP word_list;     ///< Word list to use
	Bitmap    behind;        ///< Bitmap behind the button
};

enum WordListItemFlags
{	FLAG_ACTIVE  = 0x01
,	FLAG_SUBMENU = 0x02
,	FLAG_LINE_BELOW = 0x04
};
struct DropDownWordListItem {
	DropDownWordListItem() : flags(0) {}
	DropDownWordListItem(WordListWordP word, int flags = 0)
		: word(word)
		, name(word->name)
		, flags(flags | (word->isGroup() * FLAG_SUBMENU)
		              | (word->line_below * FLAG_LINE_BELOW))
	{}
	DropDownWordListItem(WordListWordP word, const String& name, int flags = 0)
		: word(word)
		, name(name)
		, flags(flags)
	{}
	
	WordListWordP word;
	String        name;
	int           flags;
	DropDownListP submenu;
	
	inline bool active() const { return flags & FLAG_ACTIVE; }
	inline void setActive(bool a) { flags = flags & ~FLAG_ACTIVE | a * FLAG_ACTIVE; }
};

class DropDownWordList : public DropDownList {
  public:
	DropDownWordList(Window* parent, bool is_submenu, TextValueEditor& tve, const WordListPosP& pos, const WordListWordP& list);
	
	void setWords(const WordListWordP& words2);
	void setWords(const WordListPosP& pos2);
	
	inline WordListPosP getPos() const { return pos; }
	
  protected:
	virtual void          redrawArrowOnParent();
	virtual size_t        itemCount() const             { return items.size(); }
	virtual bool          lineBelow(size_t item) const  { return items[item].flags & FLAG_LINE_BELOW; }
	virtual String        itemText(size_t item) const   { return items[item].name; }
	virtual void          drawIcon(DC& dc, int x, int y, size_t item, bool selected) const;
	virtual DropDownList* submenu(size_t item) const;
	virtual size_t        selection() const;
	virtual void          select(size_t item);
	virtual bool          stayOpen(size_t selection) const;
  private:
	TextValueEditor& tve;
	WordListPosP pos;
	WordListWordP words; ///< The words we are listing
	bool has_checkboxes; ///< Do we need checkboxes?
	mutable vector <DropDownWordListItem> items;
	
	void addWordsFromScript(const WordListWordP& w);
};


DropDownWordList::DropDownWordList(Window* parent, bool is_submenu, TextValueEditor& tve, const WordListPosP& pos, const WordListWordP& words)
	: DropDownList(parent, is_submenu, is_submenu ? nullptr : &tve)
	, tve(tve), pos(pos)
	, has_checkboxes(false)
{
	setWords(words);
}

void DropDownWordList::setWords(const WordListPosP& pos2) {
	setWords((WordListWordP)pos2->word_list);
	pos = pos2;
}
void DropDownWordList::setWords(const WordListWordP& words2) {
	if (words == words2) return;
	// switch to different list
	items.clear();
	words = words2;
	// init items; do we need checkboxes?
	// do we need checkboxes?
	has_checkboxes = false;
	FOR_EACH(w, words->words) {
		if (w->is_prefix) has_checkboxes = true;
		if (w->script) {
			addWordsFromScript(w);
		} else {
			// only if not duplicating
			bool already_added = false;
			FOR_EACH(i,items) {
				if (i.name == w->name) {
					already_added = true;
					break;
				}
			}
			if (!already_added || w->isGroup()) {
				items.push_back(DropDownWordListItem(w));
			}
		}
	}
	// size of items
	icon_size.width = has_checkboxes ? 16 : 0;
	item_size.height = max(16., item_size.height);
}
void DropDownWordList::addWordsFromScript(const WordListWordP& w) {
	assert(w->script);
	// run script
	Context& ctx = tve.viewer.getContext();
	String str = w->script.invoke(ctx)->toString();
	// collect items
	vector<String> strings;
	{
		size_t prev  = 0;
		size_t comma = str.find_first_of(_(','));
		while (comma != String::npos) {
			strings.push_back(str.substr(prev, comma - prev));
			prev = comma + 1;
			if (prev + 1 < str.size() && str.GetChar(prev + 1) == _(' ')) ++prev; // skip space after comma
			comma = str.find_first_of(_(','), prev);
		}
		strings.push_back(str.substr(prev));
		sort(strings.begin(), strings.end());
	}
	// add to menu
	size_t prev = items.size();
	FOR_EACH(s, strings) {
		if (prev >= items.size() || s != items[prev].name) {
			// no line below prev
			if (prev < items.size() && !items[prev].name.empty()) {
				items[prev].flags = 0;
			}
			// not a duplicate
			prev = items.size();
			items.push_back(DropDownWordListItem(w, s, w->line_below * FLAG_LINE_BELOW));
		}
	}
}

void DropDownWordList::drawIcon(DC& dc, int x, int y, size_t item, bool selected) const {
	if (has_checkboxes) {
		bool radio = !words->words[item]->is_prefix;
		// draw checkbox
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
		dc.DrawRectangle(x,y,16,16);
		wxRect rect = RealRect(x+2,y+2,12,12);
		if (radio) {
			draw_radiobox(nullptr, dc, rect, items[item].active(), itemEnabled(item));
		} else {
			draw_checkbox(nullptr, dc, rect, items[item].active(), itemEnabled(item));
		}
	}
}

DropDownList* DropDownWordList::submenu(size_t item) const {
	DropDownWordListItem& i = items[item];
	if (i.submenu) return i.submenu.get();
	if (i.flags & FLAG_SUBMENU) {
		// create submenu?
		if (!i.submenu) {
			i.submenu.reset(new DropDownWordList(const_cast<DropDownWordList*>(this), true, tve, pos, i.word));
		}
		return i.submenu.get();
	} else {
		return nullptr;
	}
}


size_t DropDownWordList::selection() const {
	// current selection
	String current = untag(tve.value().value().substr(pos->start, pos->end - pos->start));
	// find selection
	size_t selected = NO_SELECTION;
	bool prefix_selected = true;
	size_t n = 0;
	FOR_EACH(item, items) {
		if (item.word->is_prefix) {
			if (starts_with(current, item.name)) {
				item.setActive(true);
				current = current.substr(item.name.size());
			} else {
				item.setActive(false);
			}
		} else {
			item.setActive((current == item.name));
		}
		if (item.active() && (selected == NO_SELECTION || (prefix_selected && !item.word->is_prefix))) {
			selected = n;
			prefix_selected = item.word->is_prefix;
		}
		++n;
	}
	return selected;
}

void DropDownWordList::select(size_t item) {
	// determine new value
	String new_value;
	bool toggling_prefix = items[item].word->is_prefix;
	for (size_t i = 0 ; i < items.size() ; ++i) {
		const DropDownWordListItem& it = items[i];
		if (it.word->is_prefix) {
			if (it.active() != (i == item)) {
				new_value += it.name;
			}
		} else {
			if (i == item || (toggling_prefix && items[i].active())) {
				new_value += it.name;
			}
		}
	}
	// set value
	tve.selection_start_i = pos->start;
	tve.selection_end_i   = pos->end;
	tve.fixSelection(TYPE_INDEX);
	tve.replaceSelection(escape(new_value), _ACTION_1_("change", tve.field().name));
	// stay open?
	if (IsShown()) selection(); // update 'enabled'
}

bool DropDownWordList::stayOpen(size_t selection) const {
	if (selection == NO_SELECTION) return false;
	return items[selection].word->is_prefix;
}

void DropDownWordList::redrawArrowOnParent() {
	tve.redrawWordListIndicators(true);
}

// ----------------------------------------------------------------------------- : TextValueEditor

IMPLEMENT_VALUE_EDITOR(Text)
	, selection_start  (0), selection_end  (0)
	, selection_start_i(0), selection_end_i(0)
	, selecting(false), select_words(false)
	, scrollbar(nullptr), scroll_with_cursor(false)
	, hovered_words(nullptr)
{
	if (viewer.nativeLook() && field().multi_line) {
		scrollbar = new TextValueEditorScrollBar(*this);
	}
}

TextValueEditor::~TextValueEditor() {
	delete scrollbar;
}

// ----------------------------------------------------------------------------- : Mouse

bool TextValueEditor::onLeftDown(const RealPoint& pos, wxMouseEvent& ev) {
	select_words = false;
	// on word list dropdown button?
	WordListPosP wl_pos = findWordList(pos);
	if (wl_pos) {
		wordListDropDown(wl_pos);
	} else {
		// no, select text
		selecting = true;
		moveSelection(TYPE_INDEX, v.indexAt(pos), !ev.ShiftDown(), MOVE_MID);
	}
	return true;
}
bool TextValueEditor::onLeftUp(const RealPoint& pos, wxMouseEvent&) {
	// TODO: lookup position of click?
	selecting = false;
	return false;
}

bool TextValueEditor::onMotion(const RealPoint& pos, wxMouseEvent& ev) {
	if (dropDownShown()) return false;
	if (ev.LeftIsDown() && selecting) {
		size_t index = v.indexAt(pos);
		if (select_words) {
			// on the left, swap start and end
			bool left = selection_end_i < selection_start_i;
			size_t next = nextWordBoundary(index);
			size_t prev = prevWordBoundary(index);
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
	return true;
}

void TextValueEditor::onMouseLeave(const RealPoint& pos, wxMouseEvent& ev) {
	if (hovered_words) {
		hovered_words = nullptr;
		redrawWordListIndicators();
	}
}

bool TextValueEditor::onLeftDClick(const RealPoint& pos, wxMouseEvent& ev) {
	if (dropDownShown()) return false;
	select_words = true;
	selecting = true;
	size_t index = v.indexAt(pos);
	moveSelection(TYPE_INDEX, prevWordBoundary(index), true, MOVE_MID);
	moveSelection(TYPE_INDEX, nextWordBoundary(index), false, MOVE_MID);
	return true;
}

bool TextValueEditor::onRightDown(const RealPoint& pos, wxMouseEvent& ev) {
	if (dropDownShown()) return false;
	size_t index = v.indexAt(pos);
	if (index < min(selection_start_i, selection_end_i) ||
		index > max(selection_start_i, selection_end_i)) {
		// only move cursor when outside selection
		moveSelection(TYPE_INDEX, index, !ev.ShiftDown(), MOVE_MID);
	}
	return true;
}

// ----------------------------------------------------------------------------- : Keyboard

bool TextValueEditor::onChar(wxKeyEvent& ev) {
	if (dropDownShown()) {
		// forward to drop down list
		return drop_down->onCharInParent(ev);
	}
	if (ev.AltDown()) return false;
	fixSelection();
	switch (ev.GetKeyCode()) {
		case WXK_LEFT:
			// move left (selection?)
			if (ev.ControlDown()) {
				moveSelection(TYPE_INDEX,  prevWordBoundary(selection_end_i),!ev.ShiftDown(), MOVE_LEFT);
			} else {
				moveSelection(TYPE_CURSOR, prevCharBoundary(selection_end),  !ev.ShiftDown(), MOVE_LEFT);
			}
			break;
		case WXK_RIGHT:
			// move left (selection?)
			if (ev.ControlDown()) {
				moveSelection(TYPE_INDEX,  nextWordBoundary(selection_end_i),!ev.ShiftDown(), MOVE_RIGHT);
			} else {
				moveSelection(TYPE_CURSOR, nextCharBoundary(selection_end),  !ev.ShiftDown(), MOVE_RIGHT);
			}
			break;
		case WXK_UP:
			if ( wordListDropDown(findWordList(selection_end_i)) ) break;
			moveSelection(TYPE_INDEX, v.moveLine(selection_end_i, -1), !ev.ShiftDown(), MOVE_LEFT_OPT);
			break;
		case WXK_DOWN:
			if ( wordListDropDown(findWordList(selection_end_i)) ) break;
			moveSelection(TYPE_INDEX, v.moveLine(selection_end_i, +1), !ev.ShiftDown(), MOVE_RIGHT_OPT);
			break;
		case WXK_HOME:
			// move to begining of line / all (if control)
			if (ev.ControlDown()) {
				moveSelection(TYPE_INDEX, 0,                            !ev.ShiftDown(), MOVE_LEFT_OPT);
			} else {
				moveSelection(TYPE_INDEX, v.lineStart(selection_end_i), !ev.ShiftDown(), MOVE_LEFT_OPT);
			}
			break;
		case WXK_END:
			// move to end of line / all (if control)
			if (ev.ControlDown()) {
				moveSelection(TYPE_INDEX, value().value().size(),     !ev.ShiftDown(), MOVE_RIGHT_OPT);
			} else {
				moveSelection(TYPE_INDEX, v.lineEnd(selection_end_i), !ev.ShiftDown(), MOVE_RIGHT_OPT);
			}
			break;
		case WXK_BACK:
			if (selection_start == selection_end) {
				// if no selection, select previous character
				moveSelectionNoRedraw(TYPE_CURSOR, prevCharBoundary(selection_end), false);
				if (selection_start == selection_end) {
					// Walk over a <sep> as if we are the LEFT key
					moveSelection(TYPE_CURSOR, prevCharBoundary(selection_end), true, MOVE_LEFT);
					return true;
				}
			}
			replaceSelection(wxEmptyString, _ACTION_("backspace"));
			break;
		case WXK_DELETE:
			if (selection_start == selection_end) {
				// if no selection select next
				moveSelectionNoRedraw(TYPE_CURSOR, nextCharBoundary(selection_end), false);
				if (selection_start == selection_end) {
					// Walk over a <sep> as if we are the RIGHT key
					moveSelection(TYPE_CURSOR, nextCharBoundary(selection_end), true, MOVE_RIGHT);
				}
			}
			replaceSelection(wxEmptyString, _ACTION_("delete"));
			break;
		case WXK_RETURN:
			if (field().multi_line) {
				if (ev.ShiftDown()) {
					// soft line break
					replaceSelection(_("<soft-line>\n</soft-line>"), _ACTION_("soft line break"));
				} else {
					replaceSelection(_("\n"), _ACTION_("enter"));
				}
			}
			break;
		default:
		  #ifdef __WXMSW__
			if (ev.GetKeyCode() >= _(' ') && ev.GetKeyCode() == (int)ev.GetRawKeyCode()) {
				// This check is need, otherwise pressing a key, say "0" on the numpad produces "a0"
				// (don't ask me why)
		  #else
			if (ev.GetKeyCode() >= _(' ') /*&& ev.GetKeyCode() == (int)ev.GetRawKeyCode()*/) {
		  #endif
				// TODO: Find a more correct way to determine normal characters,
				//       this might not work for internationalized input.
				//       It might also not be portable!
				#ifdef UNICODE
					replaceSelection(escape(String(ev.GetUnicodeKey(),    1)), _ACTION_("typing"), true);
				#else
					replaceSelection(escape(String((Char)ev.GetKeyCode(), 1)), _ACTION_("typing"), true);
				#endif
			} else {
				return false;
			}
	}
	return true;
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
	//selection_start   = selection_end   = 0;
	//selection_start_i = selection_end_i = 0;
}

bool TextValueEditor::onContextMenu(IconMenu& m, wxContextMenuEvent& ev) {
	// in a keword? => "reminder text" option
	size_t kwpos = in_tag(value().value(), _("<kw-"), selection_start_i, selection_start_i);
	if (kwpos != String::npos) {
		m.AppendSeparator();
		m.Append(ID_FORMAT_REMINDER,	_("reminder"),		_MENU_("reminder text"),	_HELP_("reminder text"),	wxITEM_CHECK);
	}
	// always show the menu
	return true;
}
bool TextValueEditor::onCommand(int id) {
	if (id >= ID_INSERT_SYMBOL_MENU_MIN && id <= ID_INSERT_SYMBOL_MENU_MAX) {
		// Insert a symbol
		if ((style().always_symbol || style().allow_formating) && style().symbol_font.valid()) {
			String code = style().symbol_font.font->insertSymbolCode(id);
			if (!style().always_symbol) {
				code = _("<sym>") + code + _("</sym>");
			}
			replaceSelection(code, _ACTION_("insert symbol"));
			return true;
		}
	}
	return false;
}
wxMenu* TextValueEditor::getMenu(int type) const {
	if (type == ID_INSERT_SYMBOL && (style().always_symbol || style().allow_formating)
	                             && style().symbol_font.valid()) {
		return style().symbol_font.font->insertSymbolMenu(viewer.getContext());
	} else {
		return nullptr;
	}
}

// ----------------------------------------------------------------------------- : Drawing

void TextValueEditor::draw(RotatedDC& dc) {
	if (nativeLook()) {
		// clip the dc to the region of this control
		dc.SetClippingRegion(style().getInternalRect());
	}
	// update scrollbar
	prepareDrawScrollbar(dc);
	// draw text
	TextValueViewer::draw(dc);
	// draw word list thingamajigies
	drawWordListIndicators(dc);
	// draw selection
	if (isCurrent()) {
		v.drawSelection(dc, style(), selection_start_i, selection_end_i);
		// show caret, onAction() would be a better place
		// but it has to be done after the viewer has updated the TextViewer
		// we could do that ourselfs, but we need a dc for that
		fixSelection();
		showCaret();
	}
	if (nativeLook()) {
		dc.DestroyClippingRegion();
	}
}

void TextValueEditor::redrawSelection(size_t old_selection_start_i, size_t old_selection_end_i, bool old_drop_down_shown) {
	// Hide caret
	if (isCurrent()) {
		wxCaret* caret = editor().GetCaret();
		if (caret->IsVisible()) caret->Hide();
	}
	// Destroy the clientDC before reshowing the caret, prevent flicker on MSW
	{
		// Move selection
		shared_ptr<RotatedDC> dcP = editor().overdrawDC();
		RotatedDC& dc = *dcP;
		Rotater r(dc, getRotation());
		if (nativeLook()) {
			// clip the dc to the region of this control
			dc.SetClippingRegion(style().getInternalRect());
		}
		// clear old selection by drawing it again
		if (!old_drop_down_shown) {
			v.drawSelection(dc, style(), old_selection_start_i, old_selection_end_i);
		}
		// scroll?
		scroll_with_cursor = true;
		if (ensureCaretVisible()) {
			// we can't redraw just the selection because we must scroll
			updateScrollbar();
			redraw();
		} else {
			// remove indicators
			clearWordListIndicators(dc);
			// draw new selection
			if (!dropDownShown()) {
				v.drawSelection(dc, style(), selection_start_i, selection_end_i);
			}
			// redraw drop down indicators
			drawWordListIndicators(dc, true);
		}
	}
	if (isCurrent()) {
		showCaret();
	}
}

// ----------------------------------------------------------------------------- : Other overrides

wxCursor rotated_ibeam;

wxCursor TextValueEditor::cursor(const RealPoint& pos) const {
	WordListPosP p = findWordList(pos);
	if (p) {
		if (hovered_words != p.get()) {
			hovered_words = p.get();
			const_cast<TextValueEditor*>(this)->redrawWordListIndicators();
		}
		return wxCursor();
	} else {
		p = findWordListBody(pos);
		if (hovered_words != p.get()) {
			hovered_words = p.get();
			const_cast<TextValueEditor*>(this)->redrawWordListIndicators();
		}
		int angle = viewer.getRotation().getAngle() + style().angle;
		if (sideways(angle)) { // 90 or 270 degrees
			if (!rotated_ibeam.Ok()) {
				rotated_ibeam = wxCursor(load_resource_cursor(_("rot_text")));
			}
			return rotated_ibeam;
		} else {
			return wxCURSOR_IBEAM;
		}
	}
}

bool TextValueEditor::containsPoint(const RealPoint& pos) const {
	RealPoint pos2(pos.x * style().getStretch(), pos.y);
	if (TextValueViewer::containsPoint(pos2)) return true;
	if (word_lists.empty()) return false;
	return findWordList(pos);
}
RealRect TextValueEditor::boundingBox() const {
	if (word_lists.empty()) return ValueViewer::boundingBox();
	RealRect r = style().getInternalRect().grow(1);
	FOR_EACH_CONST(wl, word_lists) {
		r.width = max(r.width, wl->rect.right() + 9);
	}
	return r;
}

void TextValueEditor::onValueChange() {
	TextValueViewer::onValueChange();
	selection_start   = selection_end   = 0;
	selection_start_i = selection_end_i = 0;
	findWordLists();
}

void TextValueEditor::onAction(const Action& action, bool undone) {
	TextValueViewer::onAction(action, undone);
	findWordLists();
	TYPE_CASE(action, TextValueAction) {
		selection_start = action.selection_start;
		selection_end   = action.selection_end;
		fixSelection(TYPE_CURSOR);
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
	replaceSelection(escape(data.GetText()), _ACTION_("paste"));
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
	replaceSelection(wxEmptyString, _ACTION_("cut"));
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
			return !style().always_symbol && style().allow_formating &&
			       in_tag(value().value(), _("<kw"), selection_start_i, selection_start_i) != String::npos;
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
		case ID_FORMAT_REMINDER: {
			const String& v = value().value();
			size_t tag = in_tag(v, _("<kw"),  selection_start_i, selection_start_i);
			if (tag != String::npos && tag + 4 < v.size()) {
				Char c = v.GetChar(tag + 4);
				return c == _('1') || c == _('A');
			}
			return false;
		} default:
			return false;
	}
}

void TextValueEditor::doFormat(int type) {
	size_t ss = selection_start, se = selection_end;
	switch (type) {
		case ID_FORMAT_BOLD: {
			addAction(toggle_format_action(valueP(), _("b"),   selection_start_i, selection_end_i, selection_start, selection_end, _("Bold")));
			break;
		}
		case ID_FORMAT_ITALIC: {
			addAction(toggle_format_action(valueP(), _("i"),   selection_start_i, selection_end_i, selection_start, selection_end, _("Italic")));
			break;
		}
		case ID_FORMAT_SYMBOL: {
			addAction(toggle_format_action(valueP(), _("sym"), selection_start_i, selection_end_i, selection_start, selection_end, _("Symbols")));
			break;
		}
		case ID_FORMAT_REMINDER: {
			addAction(new TextToggleReminderAction(valueP(), selection_start_i));
			break;
		}
	}
	selection_start = ss;
	selection_end   = se;
	fixSelection();
}

// ----------------------------------------------------------------------------- : Selection

void TextValueEditor::showCaret() {
	if (dropDownShown()) {
		wxCaret* caret = editor().GetCaret();
		if (caret && caret->IsVisible()) caret->Hide();
		return;
	}
	// Rotation
	Rotation rot(viewer.getRotation());
	Rotater rot2(rot, getRotation());
	// The caret
	wxCaret* caret = editor().GetCaret();
	// cursor rectangle
	RealRect cursor = v.charRect(selection_end_i, selection_start_i <= selection_end_i);
	cursor.width = 0;
	// height may be 0 near a <line>
	// it is not 0 for empty text, because TextRenderer handles that case
	if (cursor.height == 0) {
		if (style().always_symbol && style().symbol_font.valid()) {
			style().symbol_font.font->update(viewer.getContext());
			RealSize s = style().symbol_font.font->defaultSymbolSize(rot.trS(style().symbol_font.size));
			cursor.height = s.height;
		} else {
			cursor.height = v.heightOfLastLine();
			if (cursor.height == 0) {
				wxClientDC dc(&editor());
				// TODO : high quality?
				dc.SetFont(style().font.toWxFont(1.0));
				int hi;
				dc.GetTextExtent(_(" "), 0, &hi);
				#ifdef __WXGTK__
					// HACK: Some fonts don't get the descender height set correctly.
					int charHeight = dc.GetCharHeight();
					if (charHeight != hi)
						hi += hi - charHeight;
				#endif
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
	// TODO: handle rotated cursor
	cursor = rot.trRectToBB(cursor);
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

void TextValueEditor::insert(const String& text, const String& action_name) {
	replaceSelection(text, action_name);
}

/// compare two cursor positions, determine how much the text matches before and after
size_t match_cursor_position(size_t pos1, const String& text1, size_t pos2, const String& text2) {
	size_t score = 0; // total score
	// Match part before cursor
	size_t before1, before2;
	for (before1 = before2 = 0 ; before1 < pos1 && before2 < pos2 ; ++before1, ++before2) {
		Char c1 = text1.GetChar(pos1-before1-1), c2 = text2.GetChar(pos2-before2-1);
		if      (c1 == c2)                   score += 10000;
		else if (toLower(c1) == toLower(c2)) score +=  9999;
		else if (isPunct(c1) && isPunct(c2)) score +=     1;
		else if (c1 == UNTAG_ATOM_KWPPH)    {score +=     1; --before2;}
		else                                 break;
	}
	// bonus points for matching start of string
	if (pos1 == before1 && pos2 == before2) score += 10000;
	// Match part after cursor
	size_t after1, after2;
	for (after1 = after2 = 0 ; pos1 + after1 < text1.size() && pos2 + after2 < text2.size() ; ++after1, ++after2) {
		Char c1 = text1.GetChar(pos1+after1), c2 = text2.GetChar(pos2+after2);
		if      (c1 == c2)                   score += 10;
		else if (toLower(c1) == toLower(c2)) score +=  9;
		else if (isPunct(c1) && isPunct(c2)) score +=  1;
		else if (c2 == UNTAG_ATOM_KWPPH)    {score +=  1; --after1;}
		else                                 break;
	}
	// bonus points for matching end of string
	if (pos1+after1 == text1.size() && pos2+after2 == text2.size()) score += 2;
	// final score: matching 'before' is more important
	return score;
}

void TextValueEditor::replaceSelection(const String& replacement, const String& name, bool allow_auto_replace, bool select_on_undo) {
	if (replacement.empty() && selection_start == selection_end) {
		// no text selected, nothing to delete
		return;
	}
	// fix the selection, it may be changed by undo/redo
	if (selection_end < selection_start) swap(selection_end, selection_start);
	fixSelection();
	// execute the action before adding it to the stack,
	// because we want to run scripts before action listeners see the action
	TextValueAction* action = typing_action(valueP(), selection_start_i, selection_end_i, select_on_undo ? selection_start : selection_end, selection_end, replacement, name);
	if (!action) {
		// nothing changes, but move the selection anyway
		moveSelection(TYPE_CURSOR, selection_end);
		return;
	}
	// what we would expect if no scripts take place
	String expected_value  = untag_for_cursor(action->newValue());
	size_t expected_cursor = min(selection_start, selection_end) + untag(replacement).size();
	// perform the action
	// NOTE: this calls our onAction, invalidating the text viewer and moving the selection around the new text
	addAction(action);
	// move cursor
	{
		String real_value = untag_for_cursor(value().value());
		// where real and expected value are the same, nothing has happend, so don't look there
		size_t start, end_min;
		for (start = 0 ; start < min(real_value.size(), expected_value.size()) ; ++start) {
			if (real_value.GetChar(start) != expected_value.GetChar(start)) break;
		}
		for (end_min = 0 ; end_min < min(real_value.size(), expected_value.size()) ; ++end_min) {
			if (real_value.GetChar(real_value.size() - end_min - 1) !=
				expected_value.GetChar(expected_value.size() - end_min - 1)) break;
		}
		// what is the best cursor position?
		size_t best_cursor = expected_cursor;
		if (real_value.size() < expected_value.size()
			&& expected_cursor < expected_value.size()
			&& expected_value.GetChar(expected_cursor) == UNTAG_SEP
			&& real_value.GetChar(start)               == UNTAG_SEP
			&& real_value.size() - end_min == start) {
			// exception for type-over separators
			best_cursor = start + 1;
		} else {
			// try to find the best match to what text we expected to be around the cursor
			size_t best_match  = 0;
			size_t begin = min(start, expected_cursor);
			size_t end   = min(real_value.size(), max(real_value.size() - end_min, expected_cursor) + 1);
			for (size_t i = begin ; i < end ; ++i) {
				size_t match = match_cursor_position(expected_cursor, expected_value, i, real_value);
				if (match > best_match || (match == best_match && abs((int)expected_cursor - (int)i) < abs((int)expected_cursor - (int)best_cursor))) {
					best_match = match;
					best_cursor = i;
				}
			}
		}
		selection_end = selection_start = best_cursor;
		fixSelection(TYPE_CURSOR, MOVE_RIGHT);
	}
	// auto replace after typing?
	if (allow_auto_replace) tryAutoReplace();
	// scroll with next update
	scroll_with_cursor = true;
}

void TextValueEditor::tryAutoReplace() {
	size_t end = selection_start_i;
	GameSettings& gs = settings.gameSettingsFor(viewer.getGame());
	if (!gs.use_auto_replace) return;
	FOR_EACH(ar, gs.auto_replaces) {
		if (ar->enabled && ar->match.size() <= end) {
			size_t start = end - ar->match.size();
			if (is_substr(value().value(), start, ar->match) &&
			    (!ar->whole_word || (isWordBoundary(start) && isWordBoundary(end)))
			   ) {
				// replace
				selection_start_i = start;
				selection_end_i   = end;
				fixSelection(TYPE_INDEX);
				replaceSelection(ar->replace, _ACTION_("auto replace"), false, false);
			}
		}
	}
}

void TextValueEditor::moveSelection(IndexType t, size_t new_end, bool also_move_start, Movement dir) {
	size_t old_start = selection_start_i;
	size_t old_end   = selection_end_i;
	moveSelectionNoRedraw(t, new_end, also_move_start, dir);
	redrawSelection(old_start, old_end, dropDownShown());
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

// direction of a with respect to b
Movement direction_of(size_t a, size_t b) {
	if (a < b) return MOVE_LEFT_OPT;
	if (a > b) return MOVE_RIGHT_OPT;
	else       return MOVE_MID;
}

void TextValueEditor::fixSelection(IndexType t, Movement dir) {
	const String& val = value().value();
	// Which type takes precedent?
	if (t == TYPE_INDEX) {
		selection_start = index_to_cursor(value().value(), selection_start_i, dir);
		selection_end   = index_to_cursor(value().value(), selection_end_i,   dir);
	}
	// make sure the selection is at a valid position inside the text
	// prepare to move 'inward' (i.e. from start in the direction of end and vice versa)
	selection_start_i = cursor_to_index(val, selection_start, direction_of(selection_end, selection_start));
	selection_end_i   = cursor_to_index(val, selection_end,   direction_of(selection_start, selection_end));
	// start and end must be on the same side of separators
	size_t seppos = val.find(_("<sep"));
	while (seppos != String::npos) {
		size_t sepend = match_close_tag_end(val, seppos);
		if (selection_start_i <= seppos && selection_end_i > seppos) {
		    // not on same side, move selection end before sep
			selection_end   = index_to_cursor(val, seppos, dir);
			selection_end_i = cursor_to_index(val, selection_end, direction_of(selection_start, selection_end));
		} else if (selection_start_i >= sepend && selection_end_i < sepend) {
		    // not on same side, move selection end after sep
			selection_end   = index_to_cursor(val, sepend, dir);
			selection_end_i = cursor_to_index(val, selection_end, direction_of(selection_start, selection_end));
		}
		// find next separator
		seppos = val.find(_("<sep"), seppos + 1);
	}
}


size_t TextValueEditor::prevCharBoundary(size_t pos) const {
	return max(0, (int)pos - 1);
}
size_t TextValueEditor::nextCharBoundary(size_t pos) const {
	return min(index_to_cursor(value().value(), String::npos), pos + 1);
}

static const Char word_bound_chars[] = _(" ,.:;()\n");
bool isWordBoundaryChar(Char c) {
	return c == _(' ') || c == _(',') || c == _('.') || c == _(':') || c == _(';') ||
	       c == _('(') || c == _(')') || c == _('\n') || isPunct(c);
}

size_t TextValueEditor::prevWordBoundary(size_t pos_i) const {
	const String& val = value().value();
	size_t p = val.find_last_not_of(word_bound_chars, max(0, (int)pos_i - 1));
	if (p == String::npos) return 0;
	p = val.find_last_of(word_bound_chars, p);
	if (p == String::npos) return 0;
	return p + 1;
}
size_t TextValueEditor::nextWordBoundary(size_t pos_i) const {
	const String& val = value().value();
	size_t p = val.find_first_of(word_bound_chars, pos_i);
	if (p == String::npos) return val.size();
	p = val.find_first_not_of(word_bound_chars, p);
	if (p == String::npos) return val.size();
	return p;
}
bool TextValueEditor::isWordBoundary(size_t pos_i) const {
	const String& val = value().value();
	// boundary after?
	size_t pos = pos_i;
	while (true) {
		if (pos >= val.size()) return true;
		Char c = val.GetChar(pos);
		if (c == _('<')) pos = skip_tag(val,pos); // skip tags
		else if (isWordBoundaryChar(c)) return true;
		else break;
	}
	// boundary before?
	pos = pos_i;
	while (true) {
		if (pos == 0) return true;
		Char c = val.GetChar(pos - 1);
		if (c == _('>')) {
			// (try to) skip tags in reverse
			while (true) {
				if (pos == 0) return false; // not a tag
				--pos;
				c = val.GetChar(pos - 1);
				if      (c == _('<')) { --pos; break; } // was a tag
				else if (c == _('>')) return false; // was not a tag
			}
		}
		else return isWordBoundaryChar(c);
	}
}

void TextValueEditor::select(size_t start, size_t end) {
	selection_start = start;
	selection_end   = end;
	// TODO : redraw?
}

size_t TextValueEditor::move(size_t pos, size_t start, size_t end, Movement dir) {
	if (dir < 0 /*MOVE_LEFT*/)  return start;
	if (dir > 0 /*MOVE_RIGHT*/) return end;
	if (pos * 2 > start + end)  return end; // past the middle
	else                        return start;
}

// ----------------------------------------------------------------------------- : Search / replace

bool is_word_end(const String& s, size_t pos) {
	if (pos == 0 || pos >= s.size()) return true;
	Char c = s.GetChar(pos);
	return isSpace(c) || isPunct(c);
}

// is find.findString() at postion pos of s
bool TextValueEditor::matchSubstr(const String& s, size_t pos, FindInfo& find) {
	if (find.wholeWord()) {
		if (!is_word_end(s, pos - 1) || !is_word_end(s, pos + find.findString().size())) return false;
	}
	if (find.caseSensitive()) {
		if (!is_substr(s, pos, find.findString())) return false;
	} else {
		if (!is_substr(s, pos, find.findString().Lower())) return false;
	}
	// handle
	bool was_selection = false;
	if (find.select()) {
		editor().select(this);
		editor().SetFocus();
		size_t old_sel_start = selection_start, old_sel_end = selection_end;
		selection_start_i = untagged_to_index(value().value(), pos,                            true);
		selection_end_i   = untagged_to_index(value().value(), pos + find.findString().size(), true);
		fixSelection(TYPE_INDEX);
		was_selection = old_sel_start == selection_start && old_sel_end == selection_end;
	}
	if (find.handle(viewer.getCard(), valueP(), pos, was_selection)) {
		return true;
	} else {
		// TODO: string might have changed when doing replace all
		return false;
	}
}

bool TextValueEditor::search(FindInfo& find, bool from_start) {
	String v = untag(value().value());
	if (!find.caseSensitive()) v.LowerCase();
	size_t selection_min = index_to_untagged(value().value(), min(selection_start_i, selection_end_i));
	size_t selection_max = index_to_untagged(value().value(), max(selection_start_i, selection_end_i));
	if (find.forward()) {
		size_t start = min(v.size(), find.searchSelection() ? selection_min : selection_max);
		for (size_t i = start ; i + find.findString().size() <= v.size() ; ++i) {
			if (matchSubstr(v, i, find)) return true;
		}
	} else {
		size_t start = 0;
		int end      = (int)(find.searchSelection() ? selection_max : selection_min) - (int)find.findString().size();
		if (end < 0) return false;
		for (size_t i = end ; i >= start ; --i) {
			if (matchSubstr(v, i, find)) return true;
		}
	}
	return false;
}

// ----------------------------------------------------------------------------- : Native look / scrollbar

void TextValueEditor::determineSize(bool force_fit) {
	if (!nativeLook()) return;
	style().angle = 0; // no rotation in nativeLook
	if (scrollbar) {
		// muliline, determine scrollbar size
		Rotation rot = viewer.getRotation();
		Rotater r(rot, getRotation());
		if (!force_fit) style().height = 100;
		int sbw = wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
		RealPoint pos = rot.tr(RealPoint(0,0));
		scrollbar->SetSize(
			(int)(pos.x + rot.trX(style().width) + 1 - sbw),
			(int)pos.y - 1,
			(int)sbw,
			(int)rot.trY(style().height) + 2);
		v.reset(true);
	} else {
		// Height depends on font
		wxMemoryDC dc;
		Bitmap bmp(1,1);
		dc.SelectObject(bmp);
		dc.SetFont(style().font.toWxFont(1.0));
		style().height = dc.GetCharHeight() + 2 + style().padding_top + style().padding_bottom;
	}
}

void TextValueEditor::onShow(bool showing) {
	if (scrollbar) {
		// show/hide our scrollbar
		scrollbar->Show(showing);
	}
}

bool TextValueEditor::onMouseWheel(const RealPoint& pos, wxMouseEvent& ev) {
	if (scrollbar) {
		int toScroll = ev.GetWheelRotation() * ev.GetLinesPerAction() / ev.GetWheelDelta(); // note: up is positive
		int target = min(max(scrollbar->GetScrollPos(wxVERTICAL) - toScroll, 0),
			             scrollbar->GetScrollRange(wxVERTICAL) - scrollbar->GetScrollThumb(wxVERTICAL));
		scrollTo(target);
		return true;
	}
	return false;
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

// ----------------------------------------------------------------------------- : Word lists

bool TextValueEditor::dropDownShown() const {
	return drop_down && drop_down->IsShown();
}

void TextValueEditor::findWordLists() {
	word_lists.clear();
	hovered_words = nullptr;
	// for each word list...
	const String& str = value().value();
	size_t pos = str.find(_("<word-list-"));
	while (pos != String::npos) {
		size_t type_end = str.find_first_of(_('>'), pos);
		size_t end = match_close_tag_end(str, pos);
		if (type_end == String::npos || end == String::npos) return;
		String name = str.substr(pos + 11, type_end - pos - 11);
		WordListP word_list;
		// find word list type
		FOR_EACH(wl, viewer.getGame().word_lists) {
			if (wl->name == name) {
				word_list = wl;
				break;
			}
		}
		if (!word_list) {
			throw Error(_ERROR_1_("word list type not found", name));
		}
		// add to word_lists
		word_lists.push_back(new_intrusive3<WordListPos>(pos, end, word_list));
		// next
		pos = str.find(_("<word-list-"), end);
	}
}

void TextValueEditor::clearWordListIndicators(RotatedDC& dc) {
	if (word_lists.empty()) return;
	bool current = isCurrent();
	FOR_EACH(wl, word_lists) {
		if (current && drop_down && drop_down->IsShown() && drop_down->getPos() == wl) {
			continue;
		} else if (current && selection_end_i >= wl->start && selection_end_i <= wl->end && !dropDownShown()) {
			continue;
		} else if (wl.get() == hovered_words) {
			continue;
		}
		// restore background
		if (wl->behind.Ok()) {
			dc.DrawPreRotatedBitmap(wl->behind, RealRect(wl->rect.right(), wl->rect.y - 1, 10, wl->rect.height+3));
		}
	}
}

void TextValueEditor::redrawWordListIndicators(bool toggling_dropdown) {
	redrawSelection(selection_start_i, selection_end_i, dropDownShown() != toggling_dropdown);
}

void TextValueEditor::drawWordListIndicators(RotatedDC& dc, bool redrawing) {
	if (word_lists.empty()) return;
	DrawWhat what = viewer.drawWhat(this);
	bool current = what & DRAW_ACTIVE;
	// Draw lines around fields
	FOR_EACH(wl, word_lists) {
		RealRect& r = wl->rect;
		if (r.height < 0) {
			// find the rectangle for this indicator
			RealRect start = v.charRect(wl->start, true);
			RealRect end   = v.charRect(wl->end > wl->start ? wl->end - 1 : wl->start, false);
			r.x = start.x;
			r.y = start.y;
			r.width  = end.right() - start.left() + 0.5;
			r.height = end.bottom() - start.top();
		}
		// color?
		bool small = false;
		if (current && drop_down && drop_down->IsShown() && drop_down->getPos() == wl) {
			dc.SetPen  (Color(0,  128,255));
		} else if (current && selection_end_i >= wl->start && selection_end_i <= wl->end && !dropDownShown()) {
			dc.SetPen  (Color(64, 160,255));
		} else {
			dc.SetPen  (Color(128,128,128));
			small = (wl.get() != hovered_words);
		}
		// capture background?
		if (!redrawing) {
			wl->behind = dc.GetBackground(RealRect(r.right(), r.top() - 1, 10, r.height + 3));
		}
		if (what & (DRAW_ACTIVE | DRAW_BOXES)) {
			// draw rectangle around value
			dc.SetBrush(*wxTRANSPARENT_BRUSH);
			dc.DrawRectangle(r.move(-1,-1,2,2));
		}
	}
	// Draw drop down arrows
	FOR_EACH_REVERSE(wl, word_lists) {
		RealRect& r = wl->rect;
		// color
		bool small = false;
		if (current && drop_down && drop_down->IsShown() && drop_down->getPos() == wl) {
			dc.SetPen  (Color(0,  128,255));
			dc.SetBrush(Color(64, 160,255));
		} else if (current && selection_end_i >= wl->start && selection_end_i <= wl->end && !dropDownShown()) {
			dc.SetPen  (Color(64, 160,255));
			dc.SetBrush(Color(160,208,255));
		} else {
			dc.SetPen  (Color(128,128,128));
			dc.SetBrush(Color(192,192,192));
			small = (wl.get() != hovered_words);
		}
		if (small) {
			if (what & DRAW_BOXES) {
				dc.DrawRectangle(RealRect(r.right(), r.top() - 1, 2, r.height + 2));
			}
		} else {
			// draw background of drop down button
			dc.DrawRectangle(RealRect(r.right(), r.top() - 1, 9, r.height + 2));
			// draw foreground
			/*
			dc.SetPen  (*wxTRANSPARENT_PEN);
			dc.SetBrush(*wxBLACK_BRUSH);
			wxPoint poly[] = {dc.tr(RealPoint(0,0)), dc.tr(RealPoint(5,0)), dc.tr(RealPoint(3,2))};
			dc.getDC().DrawPolygon(3, poly, r.right() + 2, r.bottom() - 5);
			*/
			dc.SetPen  (*wxBLACK_PEN);
			double x = r.right(), y = r.bottom() - 1;
			dc.DrawLine(RealPoint(x + 4, y - 3), RealPoint(x + 5, y - 3));
			dc.DrawLine(RealPoint(x + 3, y - 4), RealPoint(x + 6, y - 4));
			dc.DrawLine(RealPoint(x + 2, y - 5), RealPoint(x + 7, y - 5));
		}
	}
}

WordListPosP TextValueEditor::findWordList(const RealPoint& pos) const {
	FOR_EACH_CONST(wl, word_lists) {
		const RealRect& r = wl->rect;
		if (pos.x >= r.right() - 0.5 && pos.x < r.right() + 9 &&
		    pos.y >= r.top()         && pos.y < r.bottom()) {
			return wl;
		}
	}
	return WordListPosP();
}
WordListPosP TextValueEditor::findWordListBody(const RealPoint& pos) const {
	FOR_EACH_CONST(wl, word_lists) {
		const RealRect& r = wl->rect;
		if (pos.x >= r.left() && pos.x < r.right() &&
		    pos.y >= r.top()  && pos.y < r.bottom()) {
			return wl;
		}
	}
	return WordListPosP();
}

WordListPosP TextValueEditor::findWordList(size_t index) const {
	FOR_EACH_CONST(wl, word_lists) {
		if (index >= wl->start && index <= wl->end) {
			return wl;
		}
	}
	return WordListPosP();
}

bool TextValueEditor::wordListDropDown(const WordListPosP& wl) {
	if (!wl) return false;
	if (dropDownShown()) return false;
	// show dropdown
	if (drop_down) {
		drop_down->setWords(wl);
	} else {
		drop_down.reset(new DropDownWordList(&editor(), false, *this, wl, wl->word_list));
	}
	RealRect rect = wl->rect.grow(1);
	drop_down->show(false, wxPoint(0,0), &rect);
	return true;
}
