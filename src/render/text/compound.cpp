//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/text/element.hpp>

// ----------------------------------------------------------------------------- : CompoundTextElement

void CompoundTextElement::draw(RotatedDC& dc, double scale, const RealRect& rect, const double* xs, DrawWhat what, size_t start, size_t end) const {
  for (auto const& e : children) {
    size_t start_ = max(start, e->start);
    size_t end_ = min(end, e->end);
    if (start_ < end_) {
      e->draw(dc, scale,
        RealRect(rect.x + xs[start_ - start] - xs[0], rect.y,
          xs[end_ - start] - xs[start_ - start], rect.height),
        xs + start_ - start, what, start_, end_);
    }
    if (end <= e->end) return; // nothing can be after this
  }
}

void CompoundTextElement::getCharInfo(RotatedDC& dc, double scale, vector<CharInfo>& out) const {
  for (auto const& e : children) {
    // characters before this element, after the previous
    assert(e->start >= out.size());
    out.resize(e->start);
    e->getCharInfo(dc, scale, out);
  }
  assert(end >= out.size());
  out.resize(end);
}

double CompoundTextElement::minScale() const {
  double m = 0.0001;
  for (auto const& e : children) {
    m = max(m, e->minScale());
  }
  return m;
}

double CompoundTextElement::scaleStep() const {
  double m = 1;
  for (auto const& e : children) {
    m = min(m, e->scaleStep());
  }
  return m;
}

// ----------------------------------------------------------------------------- : AtomTextElement

void AtomTextElement::draw(RotatedDC& dc, double scale, const RealRect& rect, const double* xs, DrawWhat what, size_t start, size_t end) const {
  if (what & DRAW_ACTIVE) {
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(background_color);
    dc.DrawRectangle(rect);
  }
  CompoundTextElement::draw(dc, scale, rect, xs, what, start, end);
}

// ----------------------------------------------------------------------------- : ErrorTextElement

void ErrorTextElement::draw(RotatedDC& dc, double scale, const RealRect& rect, const double* xs, DrawWhat what, size_t start, size_t end) const {
  // Draw wavy underline
  if (what & DRAW_ERRORS) {
    dc.SetPen(*wxRED_PEN);
    RealPoint pos = rect.bottomLeft() - dc.trInvS(RealSize(0,2));
    RealSize  dx(dc.trInvS(2), 0), dy(0, dc.trInvS(1));
    while (pos.x + 1 < rect.right()) {
      dc.DrawLine(pos - dy, pos + dx + dy);
      pos += dx;
      dy  = -dy;
    }
    if (pos.x < rect.right()) {
      // final piece
      dc.DrawLine(pos - dy, pos + dx * 0.5);
    }
  }
  // Draw the contents
  CompoundTextElement::draw(dc, scale, rect, xs, what, start, end);
}
