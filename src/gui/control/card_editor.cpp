//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/card_editor.hpp>
#include <gui/value/editor.hpp>
#include <gui/util.hpp>
#include <data/field.hpp>
#include <data/stylesheet.hpp>
#include <data/settings.hpp>
#include <util/find_replace.hpp>
#include <util/window_id.hpp>
#include <wx/caret.h>
#include <boost/iterator/filter_iterator.hpp>

const bool draw_hover_borders = false;

// ----------------------------------------------------------------------------- : DataEditor

DataEditor::DataEditor(Window* parent, int id, long style)
  : CardViewer(parent, id, style | wxWANTS_CHARS)
  , next_in_tab_order(nullptr)
  , current_viewer(nullptr)
  , current_editor(nullptr)
  , hovered_viewer(nullptr)
{
  // Create a caret
  SetCaret(new wxCaret(this,1,1));
}

ValueViewerP DataEditor::makeViewer(const StyleP& style) {
  return style->makeEditor(*this);
}

// ----------------------------------------------------------------------------- : Utility for ValueViewers

DrawWhat DataEditor::drawWhat(const ValueViewer* viewer) const {
  int what = DRAW_NORMAL
           | DRAW_ACTIVE * viewerIsCurrent(viewer)
           | DRAW_HOVER * (draw_hover_borders && viewer == hovered_viewer);
  if (nativeLook()) {
    what |= DRAW_BOXES | DRAW_EDITING | DRAW_NATIVELOOK | DRAW_ERRORS;
  } else {
    StyleSheetSettings& ss = settings.stylesheetSettingsFor(set->stylesheetFor(card));
    what |= DRAW_BORDERS * ss.card_borders()
         |  (DRAW_BOXES | DRAW_EDITING) * ss.card_draw_editing()
         |  DRAW_ERRORS;
  }
  return (DrawWhat)what;
}

bool DataEditor::viewerIsCurrent(const ValueViewer* viewer) const {
  return viewer == current_viewer && FindFocus() == this;
}

void DataEditor::addAction(unique_ptr<Action> action) {
  set->actions.addAction(move(action));
}

// ----------------------------------------------------------------------------- : Algorithms

/// Swap the order of comparison, i.e. greater-than instead of less-than
template <typename Comp>
struct SwapCompare {
  Comp comp;
  SwapCompare(Comp comp) : comp(comp) {}
  template <typename T, typename U> inline bool operator () (T x, U y) {
    return comp(y,x);
  }
};

// Return the next element in a collection after x, when using comp as ordering
template <typename It, typename V, typename Comp>
It next_element(It first, It last, V const& x, Comp comp) {
  It best = last;
  for (It it = first ; it != last ; ++it) {
    if (comp(x, *it)) {
      // this is a candidate
      if (best == last || comp(*it, *best)) {
        best = it;
      }
    }
  }
  return best;
}

template <typename It, typename V, typename Comp>
It prev_element(It first, It last, V const& x, Comp comp) {
  return next_element(first, last, x, SwapCompare<Comp>(comp));
}

// ----------------------------------------------------------------------------- : Selection

bool DataEditor::AcceptsFocus() const {
  return wxWindow::AcceptsFocus();
}

void DataEditor::select(ValueViewer* new_viewer) {
  ValueEditor* old_editor = current_editor;
  if (new_viewer) {
    current_viewer = new_viewer;
    current_editor = new_viewer->getEditor();
  } else {
    current_viewer = nullptr;
    current_editor = nullptr;
  }
  if (current_editor != old_editor) {
    // selection has changed
    if (old_editor)     old_editor->onLoseFocus();
    if (current_editor) current_editor->onFocus();
    onChange();
  }
}

struct CompareTabOrder {
  bool operator() (ValueViewer* a, ValueViewer* b) {
    assert(a && b);
    Style& as = *a->getStyle(), &bs = *b->getStyle();
    // if tab_index differs, use that
    if (as.tab_index < bs.tab_index) return true;
    if (as.tab_index > bs.tab_index) return false;
    // otherwise look at the positions
    // To get a total order, we look at the viewer center.
    // Not completely (y,x), because for viewers that are almost at the same y we prefer to sort by x
    double ax = 2*as.left + as.right; // bias a bit to the left
    double bx = 2*bs.left + bs.right;
    double ay = as.top + as.bottom + 0.1*ax; // a bit of x, so that dominates when y is approximately equal
    double by = bs.top + bs.bottom + 0.1*bx;
    if (ay < by) return true;
    if (ay > by) return false;
    if (ax < bx) return true;
    if (ax > bx) return false;
    // arbitrary order otherwise
    return a < b;
  }
  bool operator() (ValueViewerP const& a, ValueViewer* b) {
    return operator () (a.get(), b);
  }
  bool operator() (ValueViewer* a, ValueViewerP const& b) {
    return operator () (a, b.get());
  }
  bool operator() (ValueViewerP const& a, ValueViewerP const& b) {
    return operator () (a.get(), b.get());
  }
};

bool is_enabled(ValueViewerP const& v) {
  return v->getField()->editable && v->isVisible();
}

bool DataEditor::selectWithTab(vector<ValueViewerP>::iterator const& it) {
  if (it != viewers.end()) {
    select(it->get());
    return true;
  } else {
    select(nullptr);
    return false;
  }
}

bool DataEditor::selectFirst() {
  // This would be nicer with boost::range, but filtered adaptor was only introduced in boost 1.42(?)
  return selectWithTab(std::min_element(
    boost::make_filter_iterator(is_enabled, viewers.begin(), viewers.end()),
    boost::make_filter_iterator(is_enabled, viewers.end(), viewers.end()),
    CompareTabOrder()).base());
}
bool DataEditor::selectLast() {
  return selectWithTab(std::max_element(
    boost::make_filter_iterator(is_enabled, viewers.begin(), viewers.end()),
    boost::make_filter_iterator(is_enabled, viewers.end(), viewers.end()),
    CompareTabOrder()).base());
}
bool DataEditor::selectNext() {
  if (!current_viewer) return selectFirst();
  return selectWithTab(next_element(
    boost::make_filter_iterator(is_enabled, viewers.begin(), viewers.end()),
    boost::make_filter_iterator(is_enabled, viewers.end(), viewers.end()),
    current_viewer,
    CompareTabOrder()).base());
}
bool DataEditor::selectPrevious() {
  if (!current_viewer) return selectLast();
  return selectWithTab(prev_element(
    boost::make_filter_iterator(is_enabled, viewers.begin(), viewers.end()),
    boost::make_filter_iterator(is_enabled, viewers.end(), viewers.end()),
    current_viewer,
    CompareTabOrder()).base());
}

void DataEditor::onInit() {
  current_viewer = nullptr;
  current_editor = nullptr;
  hovered_viewer = nullptr;
  viewers_in_search_order.clear();
  // hide caret if it is shown
  wxCaret* caret = GetCaret();
  if (caret->IsVisible()) caret->Hide();
}
// ----------------------------------------------------------------------------- : Search / replace

template <typename Iterator>
bool DataEditor::search(Iterator it, Iterator end, FindInfo& find, bool from_start) {
  bool include = from_start || current_viewer == nullptr;
  for (;it != end; ++it) {
    ValueViewer* viewer = *it;
    if (viewer == current_viewer) include = true;
    if (include && viewer->getField()->editable && viewer->isVisible()) {
      ValueEditor* editor = viewer->getEditor();
      if (editor) {
        if (editor && editor->search(find, from_start || viewer != current_viewer)) {
          selectViewer(viewer);
          return true; // done
        }
      }
    }
  }
  return false;
}

vector<ValueViewer*> init_search_order(vector<ValueViewerP> const& viewers) {
  vector<ValueViewer*> in_order;
  for (auto const& v : viewers) {
    if (v->getEditor()) in_order.push_back(v.get());
  }
  stable_sort(in_order.begin(), in_order.end(), CompareTabOrder());
  return in_order;
}

bool DataEditor::search(FindInfo& find, bool from_start) {
  if (viewers_in_search_order.empty()) {
    viewers_in_search_order = init_search_order(viewers);
  }
  if (find.forward()) {
    return search(viewers_in_search_order.begin(), viewers_in_search_order.end(), find, from_start);
  } else {
    return search(viewers_in_search_order.rbegin(), viewers_in_search_order.rend(), find, from_start);
  }
}

// ----------------------------------------------------------------------------- : Clipboard & Formatting

bool DataEditor::canCut()            const { return current_editor && current_editor->canCut();        }
bool DataEditor::canCopy()           const { return current_editor && current_editor->canCopy();       }
bool DataEditor::canPaste()          const { return current_editor && current_editor->canPaste();      }
bool DataEditor::canFormat(int type) const { return current_editor && current_editor->canFormat(type); }
bool DataEditor::hasFormat(int type) const { return current_editor && current_editor->hasFormat(type); }
bool DataEditor::canSelectAll()      const { return current_editor && current_editor->canSelectAll();  }

void DataEditor::doCut()                   { if    (current_editor)   current_editor->doCut();         }
void DataEditor::doCopy()                  { if    (current_editor)   current_editor->doCopy();        }
void DataEditor::doPaste()                 { if    (current_editor)   current_editor->doPaste();       }
void DataEditor::doFormat(int type)        { if    (current_editor)   current_editor->doFormat(type);  }
void DataEditor::doSelectAll()             { if    (current_editor)   current_editor->doSelectAll();   }


wxMenu* DataEditor::getMenu(int type) const {
  if (current_editor) {
    return current_editor->getMenu(type);
  } else {
    return nullptr;
  }
}
void DataEditor::onCommand(int id) {
  if (current_editor) {
    current_editor->onCommand(id);
  }
}

void DataEditor::insert(const String& text, const String& action_name) {
  if (current_editor) current_editor->insert(text, action_name);
}

// ----------------------------------------------------------------------------- : Mouse events

void DataEditor::onLeftDown(wxMouseEvent& ev) {
  ev.Skip(); // for focus
  CaptureMouse();
  // change selection?
  selectViewer(ev, &ValueEditor::onLeftDown);
}
void DataEditor::onLeftUp(wxMouseEvent& ev) {
  if (HasCapture()) ReleaseMouse();
  if (current_editor && current_viewer) {
    RealPoint pos = mousePoint(ev, *current_viewer);
    if (current_viewer->containsPoint(pos)) {
      current_editor->onLeftUp(pos, ev);
    }
  }
}
void DataEditor::onLeftDClick(wxMouseEvent& ev) {
  if (current_editor && current_viewer) {
    RealPoint pos = mousePoint(ev, *current_viewer);
    if (current_viewer->containsPoint(pos)) {
      current_editor->onLeftDClick(pos, ev);
    }
  }
}
void DataEditor::onRightDown(wxMouseEvent& ev) {
  ev.Skip(); // for context menu
  // change selection?
  selectViewer(ev, &ValueEditor::onRightDown);
}
void DataEditor::onMouseWheel(wxMouseEvent& ev) {
  if (current_editor && current_viewer) {
    RealPoint pos = mousePoint(ev, *current_viewer);
    if (current_viewer->containsPoint(pos)) {
      if (current_editor->onMouseWheel(pos, ev)) return;
    }
  }
  ev.Skip();
}

void DataEditor::onMotion(wxMouseEvent& ev) {
  if (current_editor && current_viewer) {
    RealPoint pos = mousePoint(ev, *current_viewer);
    current_editor->onMotion(pos, ev);
  }
  if (!HasCapture()) {
    // find editor under mouse
    ValueViewer* old_hovered_viewer = hovered_viewer;
    bool hovered_label = false;
    hovered_viewer = mousedOverViewer(ev, &hovered_label);
    if (old_hovered_viewer && hovered_viewer != old_hovered_viewer) {
      ValueEditor* e = old_hovered_viewer->getEditor();
      RealPoint pos = mousePoint(ev, *old_hovered_viewer);
      if (e) e->onMouseLeave(pos, ev);
      if (draw_hover_borders) redraw(*old_hovered_viewer);
    }
    if (hovered_viewer && hovered_viewer != old_hovered_viewer) {
      if (draw_hover_borders) redraw(*hovered_viewer);
    }
    // change cursor and set status text
    if (hovered_viewer && !hovered_label) {
      ValueEditor* e = hovered_viewer->getEditor();
      RealPoint pos = mousePoint(ev, *hovered_viewer);
      wxCursor c;
      if (e) c = e->cursor(pos);
      if (c.Ok()) {
        SetCursor(c);
      } else {
        SetCursor(wxCURSOR_ARROW);
      }
    } else {
      SetCursor(wxCURSOR_ARROW);
    }
    // set status text
    wxFrame* frame = dynamic_cast<wxFrame*>( wxGetTopLevelParent(this) );
    if (frame) {
      frame->SetStatusText(hovered_viewer ? hovered_viewer->getField()->description : String());
    }
  }
}

void DataEditor::onMouseLeave(wxMouseEvent& ev) {
  // on mouse leave for editor
  if (hovered_viewer) {
    ValueEditor* e = hovered_viewer->getEditor();
    if (e) e->onMouseLeave(mousePoint(ev,*hovered_viewer), ev);
    if (draw_hover_borders && hovered_viewer) redraw(*hovered_viewer);
    hovered_viewer = nullptr;
  }
  // clear status text
  wxFrame* frame = dynamic_cast<wxFrame*>( wxGetTopLevelParent(this) );
  if (frame) frame->SetStatusText(wxEmptyString);
}

bool DataEditor::selectViewer(ValueViewer* v) {
  if (!v) return false;
  ValueEditor* e = v->getEditor();
  if (!e) return false;
  ValueEditor* old_editor = current_editor;
  current_editor = e;
  current_viewer = v;
  if (old_editor != current_editor) {
    // selection has changed, send focus events
    if (old_editor)     old_editor->onLoseFocus();
    if (current_editor) current_editor->onFocus();
    return true;
  }
  return false;
}

void DataEditor::selectViewer(wxMouseEvent& ev, bool (ValueEditor::*event)(const RealPoint&, wxMouseEvent&)) {
  // change viewer/editor
  ValueViewer* viewer = mousedOverViewer(ev);
  bool changed = viewer && selectViewer(viewer);
  // pass event
  if (current_editor && current_viewer) {
    RealPoint pos = mousePoint(ev, *current_viewer);
    if (current_viewer->containsPoint(pos)) {
      (current_editor->*event)(pos, ev);
    }
  }
  // refresh?
  if (changed) {
    // selection has changed, refresh viewers
    // NOTE: after passing mouse down event, otherwise opening combo box produces flicker
    onChange();
  }
}

RealPoint DataEditor::mousePoint(const wxMouseEvent& ev, const ValueViewer& viewer) const {
  Rotation rot = getRotation();
  Rotater r(rot,viewer.getRotation());
  return rot.trInv(RealPoint(ev.GetX(), ev.GetY()));
}

ValueViewer* DataEditor::mousedOverViewer(const wxMouseEvent& ev, bool* over_label_out) const {
  FOR_EACH_EDITOR_REVERSE{ // find high z index fields first
    if (v->getField()->editable) {
      if (v->containsPoint(mousePoint(ev,*v))) {
        if (over_label_out) *over_label_out = false;
        return v.get();
      } else if (nativeLook()) {
        int y = ev.GetY() + GetScrollPos(wxVERTICAL);
        if (y >= v->getStyle()->top && y < v->getStyle()->bottom) {
          if (over_label_out) *over_label_out = true;
          return v.get();
        }
      }
    }
  }
  return nullptr;
}

void DataEditor::onLoseCapture(wxMouseCaptureLostEvent&) {
  // We already test for wrong release with HasCapture()
  // but stupid wxwidget people decided to throw assertion failures
}

// ----------------------------------------------------------------------------- : Keyboard events

void DataEditor::onChar(wxKeyEvent& ev) {
  if (ev.GetKeyCode() == WXK_TAB) {
    if (!ev.ShiftDown()) {
      // try to select the next editor
      if (selectNext()) return;
      // send a navigation event to our parent, to select another control
      wxNavigationKeyEvent evt;
      GetParent()->HandleWindowEvent(evt);
    } else {
      // try to select the previos editor
      if (selectPrevious()) return;
      // send a navigation event to our parent, to select another control
      wxNavigationKeyEvent evt;
      evt.SetDirection(false);
      GetParent()->HandleWindowEvent(evt);
    }
  } else if (current_editor) {
    if (!current_editor->onChar(ev)) {
      ev.Skip();
    }
  } else {
    ev.Skip();
  }
}

// ----------------------------------------------------------------------------- : Menu events

void DataEditor::onContextMenu(wxContextMenuEvent& ev) {
  if (current_editor) {
    wxMenu m;
    add_menu_item_tr(&m, ID_EDIT_CUT, "cut", "cut");
    add_menu_item_tr(&m, ID_EDIT_COPY, "copy", "copy");
    add_menu_item_tr(&m, ID_EDIT_PASTE, "paste", "paste");
    m.Enable(ID_EDIT_CUT,   canCut());
    m.Enable(ID_EDIT_COPY,  canCopy());
    m.Enable(ID_EDIT_PASTE, canPaste());
    if (current_editor->onContextMenu(m, ev)) {
      PopupMenu(&m);
    }
  }
}
void DataEditor::onMenu(wxCommandEvent& ev) {
  if (current_editor) {
    if (!current_editor->onCommand(ev.GetId())) {
      ev.Skip();
    }
  } else {
    ev.Skip();
  }
}

// ----------------------------------------------------------------------------- : Focus events

void DataEditor::onFocus(wxFocusEvent& ev) {
  if (current_editor) {
    current_editor->onFocus();
    onChange();
  } else {
    if (ev.GetWindow() && ev.GetWindow() == next_in_tab_order) {
      selectLast();
    } else {
      selectFirst();
    }
  }
}
void DataEditor::onLoseFocus(wxFocusEvent& ev) {
  if (current_editor) {
    current_editor->onLoseFocus();
    onChange();
  }
}

// ----------------------------------------------------------------------------- : Event table

BEGIN_EVENT_TABLE(DataEditor, CardViewer)
  EVT_LEFT_DOWN      (DataEditor::onLeftDown)
  EVT_LEFT_UP        (DataEditor::onLeftUp)
  EVT_LEFT_DCLICK    (DataEditor::onLeftDClick)
  EVT_RIGHT_DOWN     (DataEditor::onRightDown)
  EVT_MOTION         (DataEditor::onMotion)
  EVT_MOUSEWHEEL     (DataEditor::onMouseWheel)
  EVT_LEAVE_WINDOW   (DataEditor::onMouseLeave)
  EVT_CONTEXT_MENU   (DataEditor::onContextMenu)
  EVT_MENU           (wxID_ANY, DataEditor::onMenu)
  EVT_CHAR           (DataEditor::onChar)
  EVT_SET_FOCUS      (DataEditor::onFocus)
  EVT_KILL_FOCUS     (DataEditor::onLoseFocus)
  EVT_MOUSE_CAPTURE_LOST(DataEditor::onLoseCapture)
END_EVENT_TABLE  ()
