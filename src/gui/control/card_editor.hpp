//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/card_viewer.hpp>

class ValueEditor;
class FindInfo;

// ----------------------------------------------------------------------------- : DataEditor

/// An editor for data values (usually a card)
class DataEditor : public CardViewer {
public:
  DataEditor(Window* parent, int id, long style = wxBORDER_THEME);
  
  // --------------------------------------------------- : Utility for ValueViewers/Editors
  
  DrawWhat drawWhat(const ValueViewer*) const override;
  bool viewerIsCurrent(const ValueViewer*) const override;
  
  virtual void addAction(unique_ptr<Action> action) final;
  inline SetP getSetForActions() { return set; }
  
  // --------------------------------------------------- : Selection
  
  /// Select the given viewer, sends focus events
  void select(ValueViewer* v);
  /// Select the first editable and visible editor (by tab index)
  bool selectFirst();
  /// Select the last editable and visible editor (by tab index)
  bool selectLast();
  /// Select the next editable editor, returns false if the current editor is the last one
  bool selectNext();
  /// Select the previous editable editor, returns false if the current editor is the first one
  bool selectPrevious();
  
  bool AcceptsFocus() const override;
  
  /// The next window in the tab order (optional)
  const wxWindow* next_in_tab_order;
  
  // --------------------------------------------------- : Clipboard
  
  bool canCut()   const;
  bool canCopy()  const;
  bool canPaste() const;
  void doCut();
  void doCopy();
  void doPaste();
  
  // --------------------------------------------------- : Formatting
  
  bool canFormat(int type) const;
  bool hasFormat(int type) const;
  void doFormat (int type);
  /// Get a special menu, events should be sent to onCommand
  wxMenu* getMenu(int type) const;
  /// A menu item from getMenu was selected
  void onCommand(int id);

  // --------------------------------------------------- : Text selection

  bool canSelectAll() const;
  void doSelectAll();

  // --------------------------------------------------- : Search/replace
    
  /// Do a search or replace action for the given FindInfo
  /** If from_start == false: searches only from the current selection onward (or backward)
   *  If from_start == true:  searches everything
   *
   *  Returns true if we are done and searching should be ended.
   */
  bool search(FindInfo& find, bool from_start);
  
  // --------------------------------------------------- : Selection in editor
  
  /// Insert 'text' into the current editor, using an action with the given name
  void insert(const String& text, const String& action_name);
  
  // --------------------------------------------------- : ValueViewers
  
protected:
  /// Create an editor for the given style (as opposed to a normal viewer)
  ValueViewerP makeViewer(const StyleP&) override;
  
  void onInit() override;
  
  // --------------------------------------------------- : Data
  ValueViewer* current_viewer;  ///< The currently selected viewer
  ValueEditor* current_editor;  ///< The currently selected editor, corresponding to the viewer
  ValueViewer* hovered_viewer;  ///< The editor under the mouse cursor
  vector<ValueViewer*> viewers_in_search_order;  ///< The editable viewers, sorted by tab index, for find/replace
  
private:
  // --------------------------------------------------- : Events
  DECLARE_EVENT_TABLE();
  
  
  void onLeftDown  (wxMouseEvent&);
  void onLeftUp    (wxMouseEvent&);
  void onLeftDClick(wxMouseEvent&);
  void onRightDown (wxMouseEvent&);
  void onMotion    (wxMouseEvent&);
  void onMouseWheel(wxMouseEvent&);
  void onMouseLeave(wxMouseEvent&);
  void onLoseCapture(wxMouseCaptureLostEvent&);
  
  void onChar      (wxKeyEvent&);
  
  void onContextMenu(wxContextMenuEvent&);
  void onMenu       (wxCommandEvent&);
  
  void onFocus    (wxFocusEvent&);
  void onLoseFocus(wxFocusEvent&);
  
  // --------------------------------------------------- : Functions

  /// Changes the selection to the given field, returns true if selection changed
  bool selectViewer(ValueViewer*);
  /** Sends an event to the event function of the current viewer */
  /// Changes the selection to the field at the specified coordinates
  void selectViewer(wxMouseEvent& ev, bool (ValueEditor::* event)(const RealPoint&, wxMouseEvent&));
  /// Convert mouse coordinates to internal coordinates
  RealPoint mousePoint(const wxMouseEvent&, const ValueViewer& viewer) const;
  /// Field under the mouse cursor, or nullptr if there is none
  ValueViewer* mousedOverViewer(const wxMouseEvent&, bool* over_label_out=nullptr) const;
  
  /// Select a field found by tab order, can be viewers.end()
  bool selectWithTab(vector<ValueViewerP>::iterator const&);

  template <typename Iterator>
  bool search(Iterator begin, Iterator end, FindInfo& find, bool from_start);
};

/// By default a DataEditor edits cards
typedef DataEditor CardEditor;

// ----------------------------------------------------------------------------- : Utility

#define FOR_EACH_EDITOR \
  FOR_EACH(v, viewers) \
    if (ValueEditor* e = v->getEditor())
#define FOR_EACH_EDITOR_REVERSE \
  FOR_EACH_REVERSE(v, viewers) \
    if (ValueEditor* e = v->getEditor())

