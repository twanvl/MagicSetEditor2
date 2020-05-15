//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/card/viewer.hpp>

// ----------------------------------------------------------------------------- : Events

/// Event that indicates the size of a CardViewer has changed
DECLARE_LOCAL_EVENT_TYPE(EVENT_SIZE_CHANGE, <not used>)
/// Handle EVENT_SIZE_CHANGE events
#define EVT_SIZE_CHANGE(id, handler) EVT_COMMAND(id, EVENT_SIZE_CHANGE, handler)

// ----------------------------------------------------------------------------- : CardViewer

/// A control to view a single card
class CardViewer : public wxControl, public DataViewer {
public:
  CardViewer(Window* parent, int id, long style = wxBORDER_THEME);
  
  /// Get a dc to draw on the card outside onPaint  
  /** May NOT be called while in onPaint/draw */
  shared_ptr<RotatedDC> overdrawDC();
  
  /// Invalidate and redraw the entire viewer
  void redraw();
  /// Invalidate and redraw (the area of) a single value viewer
  void redraw(const ValueViewer&) override;
  
  /// The rotation to use
  Rotation getRotation() const override;
  
  bool AcceptsFocus() const override { return false; }
  
protected:
  /// Return the desired size of control
  wxSize DoGetBestSize() const override;
  
  void onChange() override;
  void onChangeSize() override;
  
  /// Should the given viewer be drawn?
  bool shouldDraw(const ValueViewer&) const;
  
  void drawViewer(RotatedDC& dc, ValueViewer& v) override;
  
private:
  DECLARE_EVENT_TABLE();
  
  void onPaint(wxPaintEvent&);
  
  Bitmap buffer;     ///< Off-screen buffer we draw to
  bool   up_to_date; ///< Is the buffer up to date?
  
  class OverdrawDC;
  class OverdrawDC_aux;
};

