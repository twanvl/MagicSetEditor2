//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/tagged_string.hpp> // for Movement
#include <gui/value/editor.hpp>
#include <render/value/text.hpp>

class TextValueEditorScrollBar;
DECLARE_POINTER_TYPE(WordListPos);
DECLARE_SHARED_POINTER_TYPE(DropDownWordList);

// ----------------------------------------------------------------------------- : TextValueEditor

enum IndexType
{  TYPE_CURSOR  ///< Positions are cursor positions
,  TYPE_INDEX  ///< Positions are character indices
};

/// An editor 'control' for editing TextValues
/** Okay, this class responds to pretty much every event available... :)
 */
class TextValueEditor : public TextValueViewer, public ValueEditor {
public:
  DECLARE_VALUE_EDITOR(Text);
  ~TextValueEditor();
  
  // --------------------------------------------------- : Events
  
  void onFocus() override;
  void onLoseFocus() override;
  
  bool onLeftDown  (const RealPoint& pos, wxMouseEvent&) override;
  bool onLeftUp    (const RealPoint& pos, wxMouseEvent&) override;
  bool onLeftDClick(const RealPoint& pos, wxMouseEvent&) override;
  bool onRightDown (const RealPoint& pos, wxMouseEvent&) override;
  bool onMotion    (const RealPoint& pos, wxMouseEvent&) override;
  void onMouseLeave(const RealPoint& pos, wxMouseEvent&) override;
  bool onMouseWheel(const RealPoint& pos, wxMouseEvent&) override;
  
  bool onContextMenu(wxMenu& m, wxContextMenuEvent&) override;
  wxMenu* getMenu(int type) const override;
  bool onCommand(int) override;
  
  bool onChar(wxKeyEvent&) override;
  
  // --------------------------------------------------- : Actions
  
  void onValueChange() override;
  void onAction(const Action&, bool undone) override;
  
  // --------------------------------------------------- : Clipboard
  
  bool canCopy() const override;
  bool canPaste() const override;
  bool doCopy() override;
  bool doPaste() override;
  bool doDelete() override;
  
  // --------------------------------------------------- : Formating
  
  bool canFormat(int type) const override;
  bool hasFormat(int type) const override;
  void doFormat(int type) override;
  
  // --------------------------------------------------- : Selection

  bool canSelectAll() const override { return true; }
  void doSelectAll() override;
  void select(size_t start, size_t end) override;
  size_t selectionStart() const override { return selection_start; }
  size_t selectionEnd()   const override { return selection_end; }
  
  void insert(const String& text, const String& action_name) override;
  
  // --------------------------------------------------- : Search/replace
  
  bool search(FindInfo& find, bool from_start) override;
private:
  bool matchSubstr(const String& s, size_t pos, FindInfo& find);
public:
  
  // --------------------------------------------------- : Other
  
  wxCursor cursor(const RealPoint& pos) const override;
  void determineSize(bool force_fit = false) override;
  bool containsPoint(const RealPoint& p) const override;
  RealRect boundingBoxBorder() const override;
  void onShow(bool) override;
  void draw(RotatedDC&) override;
  
  // --------------------------------------------------- : Data
private:
  size_t selection_start,   selection_end;   ///< Cursor position/selection (if any), cursor positions
  size_t selection_start_i, selection_end_i; ///< Cursor position/selection, character indices
  bool selecting;                            ///< Selecting text?
  bool select_words;                         ///< Select whole words when dragging the mouse?
  TextValueEditorScrollBar* scrollbar;       ///< Scrollbar for multiline fields in native look
  bool scroll_with_cursor;                   ///< When the cursor moves, should the scrollposition change?
  vector<WordListPosP> word_lists;           ///< Word lists in the text
  
  // --------------------------------------------------- : Selection / movement
  
  /// Move the selection to a new location, clears the previously drawn selection.
  /** t specifies what kind of position new_end is */
  void moveSelection(IndexType t, size_t new_end, bool also_move_start=true, Movement dir = MOVE_MID);
  /// Move the selection to a new location, but does not redraw.
  /** t specifies what kind of position new_end is */
  void moveSelectionNoRedraw(IndexType t, size_t new_end, bool also_move_start=true, Movement dir = MOVE_MID);
  
  /// Redraw the selection
  void redrawSelection(size_t old_selection_start_i, size_t old_selection_end_i, bool old_drop_down_shown);
  
  /// Replace the current selection with 'replacement', name the action
  /** replacement should be a tagged string (i.e. already escaped) */
  void replaceSelection(const String& replacement, const String& name, bool allow_auto_replace = false, bool select_on_undo = true);
  /// Try to autoreplace at the position before the cursor
  void tryAutoReplace();
  
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
  size_t prevCharBoundary(size_t pos) const;
  size_t nextCharBoundary(size_t pos) const;
  /// Front of previous word, used witch Ctrl+Left/right
  /** Uses character indices */
  size_t prevWordBoundary(size_t pos_i) const;
  size_t nextWordBoundary(size_t pos_i) const;
  bool   isWordBoundary(size_t pos_i) const;
  
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
  
  // --------------------------------------------------- : Word lists
  
  friend class DropDownWordList;
  DropDownWordListP drop_down;
  bool dropDownShown() const;
  mutable WordListPos* hovered_words;
  
  /// Find all word lists in the current value
  void findWordLists();
  /// Draw word list indicators
  void drawWordListIndicators(RotatedDC& dc, bool redrawing = false);
  /// Remove word list indicators
  void clearWordListIndicators(RotatedDC& dc);
  /// Re-draw word list indicators
  void redrawWordListIndicators(bool toggling_dropdown = false);
  /// Find a WordListPos under the mouse cursor (if any), pos is in internal coordinates
  WordListPosP findWordList(const RealPoint& pos) const;
  /// Find a WordListPos rectangle under the mouse cursor (if any), pos is in internal coordinates
  WordListPosP findWordListBody(const RealPoint& pos) const;
  /// Find a WordListPos for a index position
  WordListPosP findWordList(size_t index) const;
  /// Show a word list drop down menu, if wl. Returns true if a list is shown
  bool wordListDropDown(const WordListPosP& wl);
  
};

