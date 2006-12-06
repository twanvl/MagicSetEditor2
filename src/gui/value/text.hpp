//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_VALUE_TEXT
#define HEADER_GUI_VALUE_TEXT

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/editor.hpp>
#include <render/value/text.hpp>

class TextValueEditorScrollBar;

// ----------------------------------------------------------------------------- : TextValueEditor

/// Directions of cursor movement
enum Movement
{	MOVE_LEFT	///< Always move the cursor to the left
,	MOVE_MID	///< Move in whichever direction the distance to move is shorter (TODO: define shorter)
,	MOVE_RIGHT	///< Always move the cursor to the right
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
	
	virtual void onLeftDown  (const RealPoint& pos, wxMouseEvent&);
	virtual void onLeftUp    (const RealPoint& pos, wxMouseEvent&);
	virtual void onLeftDClick(const RealPoint& pos, wxMouseEvent&);
	virtual void onRightDown (const RealPoint& pos, wxMouseEvent&);
	virtual void onMotion    (const RealPoint& pos, wxMouseEvent&);
	virtual void onMouseWheel(const RealPoint& pos, wxMouseEvent& ev);
	
	virtual bool onContextMenu(wxMenu& m, wxContextMenuEvent&);
	virtual void onMenu(wxCommandEvent&);
	
	virtual void onChar(wxKeyEvent&);
	
	// --------------------------------------------------- : Actions
	
	virtual void onValueChange();
	virtual void onAction(const ValueAction&, bool undone);
	
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
	
	// --------------------------------------------------- : Other
	
	virtual wxCursor cursor() const;
	virtual void determineSize();
	virtual void onShow(bool);
	virtual void draw(RotatedDC&);
	
	// --------------------------------------------------- : Data
  private:
	size_t selection_start, selection_end; ///< Cursor position/selection (if any)
	TextValueEditorScrollBar* scrollbar;   ///< Scrollbar for multiline fields in native look
	
	// --------------------------------------------------- : Selection / movement
	
	/// Move the selection to a new location, clears the previously drawn selection
	void moveSelection(size_t new_end, bool also_move_start=true, Movement dir = MOVE_MID);
	/// Move the selection to a new location, but does not redraw
	void moveSelectionNoRedraw(size_t new_end, bool also_move_start=true, Movement dir = MOVE_MID);
	
	/// Replace the current selection with 'replacement', name the action
	void replaceSelection(const String& replacement, const String& name);
	
	/// Make sure the selection satisfies its constraints
	/** - selection_start and selection_end are inside the text
	 *  - not inside tags
	 *  - the selection does not contain a <sep> or </sep> tag
	 *
	 *  When correcting the selection, move in the given direction
	 */
	void fixSelection(Movement dir = MOVE_MID);
	
	/// Return a position resulting from moving pos outside the range [start...end), in the direction dir
	static size_t move(size_t pos, size_t start, size_t end, Movement dir);
	
	/// Move the caret to the selection_end position and show it
	void showCaret();
	
	/// Position of previous visible & selectable character
	size_t prevCharBoundry(size_t pos) const;
	size_t nextCharBoundry(size_t pos) const;
	/// Front of previous word, used witch Ctrl+Left/right
	size_t prevWordBoundry(size_t pos) const;
	size_t nextWordBoundry(size_t pos) const;
	
	// --------------------------------------------------- : Scrolling
	
	friend class TextValueEditorScrollBar;
	
	/// Scroll to the given position, called by scrollbar
	void scrollTo(int pos);
};

// ----------------------------------------------------------------------------- : EOF
#endif
