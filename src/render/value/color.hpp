//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_RENDER_VALUE_COLOR
#define HEADER_RENDER_VALUE_COLOR

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/value/viewer.hpp>
#include <data/field/color.hpp>

DECLARE_POINTER_TYPE(AlphaMask);

// ----------------------------------------------------------------------------- : ColorValueViewer

/// Viewer that displays a color value
class ColorValueViewer : public ValueViewer {
  public:
	DECLARE_VALUE_VIEWER(Color) : ValueViewer(parent,style) {}
	
	virtual void draw(RotatedDC& dc);
	virtual bool containsPoint(const RealPoint& p) const;
	
  private:
	/// Draws a border around the field
	void drawFieldBorder(RotatedDC& dc, const AlphaMask& alpha_mask);
	/// Load the AlphaMask for this field
	const AlphaMask& getMask(int w, int h) const;
};

// ----------------------------------------------------------------------------- : EOF
#endif
