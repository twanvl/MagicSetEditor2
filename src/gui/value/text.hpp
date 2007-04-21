//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_VALUE_TEXT
#define HEADER_GUI_VALUE_TEXT

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/tagged_string.hpp> // for Movement
#include <gui/value/editor.hpp>
#include <render/value/text.hpp>

class TextValueEditorScrollBar;
class wxFindReplaceData;
DECLARE_POINTER_TYPE(Card);

// ----------------------------------------------------------------------------- : Search/replace

/// Information for search/replace
class FindInfo {
  public:
	FindInfo(wxFindReplaceData& what) : what(what) {}
	virtual ~FindInfo() {}
	
	/// Handle that a match was found.
	/** Should return whether more searching is needed.
		*/
	virtual bool handle(const CardP& card, const TextValueP& value, size_t start, size_t end) = 0;
	
	wxFindReplaceData& what; ///< What to search for, the direction to search in
};

// ----------------------------------------------------------------------------- : TextValueEditor

enum IndexType
{	TYPE_CURSOR	///< Positions are cursor positions
,	TYPE_INDEX	///< Positions are character indices
};

/// An editor 'control' for editing TextValues
/** Okay, this class responds to pretty much every event available... :)
 */
class TextValueEditor : public TextValueViewer, public ValueEditor {
  public:
	DECLARE_VALUE_EDITOR(Text);
	~TextValueEditor();
	
	// --------------------------------------------------- : Events
	
	virtual void onFocus();
	virtual void onLoseFocus();
	
	virtual bool onLeftDown  (const RealPoint& pos, wxMouseEvent&);
	virtual bool onLeftUp    (const RealPoint& pos, wxMouseEvent&);
	virtual bool onLeftDClick(const RealPoint& pos, wxMouseEvent&);
	virtual bool onRightDown (const RealPoint& pos, wxMouseEvent&);
	virtual bool onMotion    (const RealPoint& pos, wxMouseEvent&);
	virtual bool onMouseWheel(const RealPoint& pos, wxMouseEvent& ev);
	
	virtual bool onContextMenu(IconMenu& m, wxContextMenuEvent&);
	virtual wxMenu* getMenu(int type) const;
	virtual bool onCommand(int);
	
	virtual bool onChar(wxKeyEvent&);
	
	// --------------------------------------------------- : Actions
	
	virtual void onValueChange();
	virtual void onAction(const Action&, bool undone);
	
	// --------------------------------------------------- : Clipboard
	
	virtual bool canCopy()  const;
	virtual bool canPaste() const;
	virtual bool doCopy();
	virtual bool doPaste();
	virtual bool doDelete();
	
	// --------------------------------------------------- : Formating
	
	virtual bool canFormat(int type) const;
	virtual bool hasFormat(int type) const;
	virtual void doFormat(int type);
	
	// --------------------------------------------------- : Selection
	
	virtual void select(size_t start, size_t end);
	virtual size_t selectionStart() const { return selection_start; }
	virtual size_t selectionEnd()   const { return selection_end; }
	
	virtual void insert(const String& text, const String& action_name);
	
	// --------------------------------------------------- : Other
	
	virtual wxCursor cursor() const;
	virtual void determineSize(bool force_fit = false);
	virtual void onShow(bool);
	virtual void draw(RotatedDC&);
	
	// --------------------------------------------------- : Data
  private:
	size_t selection_start,   selection_end;   ///< Cursor position/selection (if any), cursor positions
	size_t selection_start_i, selection_end_i; ///< Cursor position/selection, character indices
	bool select_words;                         ///< Select whole words when dragging the mouse?
	TextValueEditorScrollBar* scrollbar;       ///< Scrollbar for multiline fields in native look
	bool scroll_with_cursor;                   ///< When the cursor moves, should the scrollposition change?
	
	// --------------------------------------------------- : Selection / movement
	
	/// Move the selection to a new location, clears the previously drawn selection.
	/** t specifies what kind of position new_end is */
	void moveSelection(IndexType t, size_t new_end, bool also_move_start=true, Movement dir = MOVE_MID);
	/// Move the selection to a new location, but does not redraw.
	/** t specifies what kind of position new_end is */
	void moveSelectionNoRedraw(IndexType t, size_t new_end, bool also_move_start=true, Movement dir = MOVE_MID);
	
	/// Replace the current selection with 'replacement', name the action
	/** replacement should be a tagged string (i.e. already escaped) */
	void replaceSelection(const String& replacement, const String& name);
	
	/// Make sure the selection satisfies its constraints
	/** - selection_start and selection_end are inside the text
	 *  - not inside tags
	 *  - the selection does not contain a <sep> or </sep> tag
	 *
	 *  When correcting the selection, move in the given direction
	 */
	void fixSelection(IndexType t = TYPE_CURSOR, Movement dir = MOVE_MID);
	
	/// Return a position resulting from moving pos outside the range [start...end), in the direction dir
	static size_t move(size_t pos, size_t start, size_t end, Movement dir);
	
	/// Move the caret to the selection_end position and show it
	void showCaret();
	
	/// Position of previous visible & selectable character
	/** Uses cursor positions */
	size_t prevCharBoundry(size_t pos) const;
	size_t nextCharBoundry(size_t pos) const;
	/// Front of previous word, used witch Ctrl+Left/right
	/** Uses character indices */
	size_t prevWordBoundry(size_t pos_i) const;
	size_t nextWordBoundry(size_t pos_i) const;
	
	// --------------------------------------------------- : Scrolling
	
	friend class TextValueEditorScrollBar;
		
	/// Scroll to the given position, called by scrollbar
	void scrollTo(int pos);
	/// Update the scrollbar to show the current scroll position
	void updateScrollbar();
	/// Scrolls to ensure the caret stays visible, return true if the control is scrolled
	bool ensureCaretVisible();
	/// Prepare for drawing if there is a scrollbar
	void prepareDrawScrollbar(RotatedDC& dc);
};

// ----------------------------------------------------------------------------- : EOF
#endif
