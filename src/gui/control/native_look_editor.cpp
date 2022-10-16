//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/native_look_editor.hpp>
#include <gui/value/editor.hpp>
#include <gui/util.hpp>
#include <data/stylesheet.hpp>
#include <data/export_template.hpp>
#include <data/settings.hpp>

// ----------------------------------------------------------------------------- : NativeLookEditor

NativeLookEditor::NativeLookEditor(Window* parent, int id, long style)
  : DataEditor(parent, id, style)
{}

Rotation NativeLookEditor::getRotation() const {
  int dx = CanScroll(wxHORIZONTAL) ? GetScrollPos(wxHORIZONTAL) : 0;
  int dy = CanScroll(wxVERTICAL) ? GetScrollPos(wxVERTICAL) : 0;
  return Rotation(0, RealRect(RealPoint(-dx,-dy),GetClientSize()));
}

void NativeLookEditor::draw(DC& dc) {
  RotatedDC rdc(dc, getRotation(), QUALITY_LOW);
  DataViewer::draw(rdc, wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
}
void NativeLookEditor::drawViewer(RotatedDC& dc, ValueViewer& v) {
  if (!shouldDraw(v)) return;
  ValueEditor* e = v.getEditor();
  if (!e || e->drawLabel()) {
    // draw control border and box
    draw_control_box(this, dc.getDC(), dc.getExternalRect().grow(1), current_editor == e, e != nullptr);
    // draw label
    dc.SetFont(*wxNORMAL_FONT);
    dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
    dc.DrawText(v.getField()->caption.get(), RealPoint(margin_left - v.bounding_box.x, 1));
  }
  // draw viewer
  v.draw(dc);
}

void NativeLookEditor::resizeViewers() {
  // size stuff
  double y = margin;
  int w, h;
  GetClientSize(&w, &h);
  const double default_height = 17;
  // Determine label width
  {
    label_width = 0;
    wxClientDC dc(this);
    dc.SetFont(*wxNORMAL_FONT);
    FOR_EACH(v, viewers) {
      ValueEditor* e = v->getEditor();
      if (!e || e->drawLabel()) {
        // width of the label string
        int w;
        Style& s = *v->getStyle();
        dc.GetTextExtent(s.fieldP->caption.get(), &w, nullptr);
        label_width = max(label_width, w + label_margin);
      }
    }
  }
  // Set editor sizes
  FOR_EACH(v, viewers) {
    StyleP s = v->getStyle();
    ValueEditor* e = v->getEditor();
    if (!e || e->drawLabel()) {
      v->bounding_box.x = margin + label_width;
    } else {
      v->bounding_box.x = margin;
    }
    v->bounding_box.y  = y;
    v->bounding_box.width  = w - v->bounding_box.x - margin;
    v->bounding_box.height = s->height() == 0 ? default_height : s->height();
    if (e) e->determineSize();
    y += v->bounding_box.height + vspace;
  }
  y = y - vspace + margin;
  SetVirtualSize(w, (int)y);
  if (CanScroll(wxVERTICAL)) {
    SetScrollbar(wxVERTICAL, 0, h, (int)y);
  }
  if (y >= h) {
    // Doesn't fit vertically, add scrollbar and resize
    // create scrollbar
  }
}

void NativeLookEditor::onInit() {
  DataEditor::onInit();
  // Give viewers a chance to show/hide controls (scrollbar) when selecting other editors
  FOR_EACH_EDITOR {
    e->onShow(true);
  }
  resizeViewers();
}

wxSize NativeLookEditor::DoGetBestSize() const {
  return wxSize(200, 200);
}
void NativeLookEditor::onSize(wxSizeEvent& ev) {
  resizeViewers();
  Refresh(false);
}
void NativeLookEditor::onScroll(wxScrollWinEvent& ev) {
  if (ev.GetOrientation() == wxVERTICAL) {
    int y      = GetScrollPos(wxVERTICAL);
    int page   = GetClientSize().y;  // view size
    // determine new y offset
    // NOTE: can't use case, these are not constants
    if        (ev.GetEventType() == wxEVT_SCROLLWIN_TOP) {
      y = 0;
    } else if (ev.GetEventType() == wxEVT_SCROLLWIN_BOTTOM) {
      y = numeric_limits<int>::max();
    } else if (ev.GetEventType() == wxEVT_SCROLLWIN_LINEUP) {
      y = y - 10;
    } else if (ev.GetEventType() == wxEVT_SCROLLWIN_LINEDOWN) {
      y = y + 10;
    } else if (ev.GetEventType() == wxEVT_SCROLLWIN_PAGEUP) {
      y = y - page;
    } else if (ev.GetEventType() == wxEVT_SCROLLWIN_PAGEDOWN) {
      y = y + page;
    } else if (ev.GetEventType() == wxEVT_SCROLLWIN_THUMBTRACK ||
        ev.GetEventType() == wxEVT_SCROLLWIN_THUMBRELEASE) {
      y = ev.GetPosition();
    }
    scrollTo(wxVERTICAL, y);
  }
}
void NativeLookEditor::onMouseWheel(wxMouseEvent& ev) {
  // send scroll event to field under cursor
  FOR_EACH_EDITOR_REVERSE { // find high z index fields first
    RealPoint pos = mousePoint(ev, *v);
    if (v->containsPoint(pos) && v->getField()->editable) {
      bool scrolled = e->onMouseWheel(pos, ev);
      if (scrolled) return;
      break;
    }
  }
  // scroll entire window
  int toScroll = 10 * ev.GetWheelRotation() * ev.GetLinesPerAction() / ev.GetWheelDelta(); // note: up is positive
  int y = GetScrollPos(wxVERTICAL);
  scrollTo(wxVERTICAL, y - toScroll);
}

void NativeLookEditor::scrollTo(int direction, int pos) {
  if (direction == wxVERTICAL) {
    int y      = GetScrollPos(wxVERTICAL);
    int height = GetVirtualSize().y; // height
    int page   = GetClientSize().y;  // view size
    int bottom = max(0, height - page);
    pos = max(0, min(bottom, pos));
    if (pos != y) {
      SetScrollPos(wxVERTICAL, pos);

      // move child controls
      FOR_EACH(v, viewers) {
        ValueEditor* e = v->getEditor();
        if (e) e->determineSize();
      }
    }
    // redraw
    onChange();
  }
}

BEGIN_EVENT_TABLE(NativeLookEditor, DataEditor)
  EVT_SIZE        (NativeLookEditor::onSize)
  EVT_SCROLLWIN   (NativeLookEditor::onScroll)
  EVT_MOUSEWHEEL  (NativeLookEditor::onMouseWheel)
END_EVENT_TABLE()


// ----------------------------------------------------------------------------- : SetInfoEditor

SetInfoEditor::SetInfoEditor(Window* parent, int id, long style)
  : NativeLookEditor(parent, id, style | wxVSCROLL)
{}

void SetInfoEditor::onChangeSet() {
  setStyles(set->stylesheet, set->stylesheet->set_info_style);
  setData(set->data);
}

Package& SetInfoEditor::getStylePackage() const {
  return DataEditor::getStylePackage();
  // TODO: Use the game
  //return getGame();
}

// ----------------------------------------------------------------------------- : StylingEditor

StylingEditor::StylingEditor(Window* parent, int id, long style)
  : NativeLookEditor(parent, id, style | wxVSCROLL)
{}

void StylingEditor::showStylesheet(const StyleSheetP& stylesheet) {
  setStyles(stylesheet, stylesheet->styling_style);
  setData(set->stylingDataFor(*stylesheet));
}
void StylingEditor::showCard(const CardP& card) {
  StyleSheetP stylesheet = set->stylesheetForP(card);
  setStyles(stylesheet, stylesheet->styling_style);
  setData(set->stylingDataFor(card));
}

void StylingEditor::onChangeSet() {
  showStylesheet(set->stylesheet);
}

// ----------------------------------------------------------------------------- : ExportOptionsEditor

ExportOptionsEditor::ExportOptionsEditor(Window* parent, int id, long style)
  : NativeLookEditor(parent, id, style | wxVSCROLL)
{}

void ExportOptionsEditor::showExport(const ExportTemplateP& export_template) {
  this->export_template = export_template;
  setStyles(set->stylesheet, export_template->option_style);
  setData(settings.exportOptionsFor(*export_template));
}

Package& ExportOptionsEditor::getStylePackage() const {
  return *export_template;
}
