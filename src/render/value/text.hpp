//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_RENDER_VALUE_TEXT
#define HEADER_RENDER_VALUE_TEXT

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/value/viewer.hpp>
#include <render/text/viewer.hpp>
#include <data/field/text.hpp>

// ----------------------------------------------------------------------------- : TextValueViewer

/// Viewer that displays a text value
class TextValueViewer : public ValueViewer {
  public:
	DECLARE_VALUE_VIEWER(Text) : ValueViewer(parent,style) {}
	
	virtual bool prepare(RotatedDC& dc);
	virtual void draw(RotatedDC& dc);
	virtual void onValueChange();
	virtual void onStyleChange();
	
  protected:
	TextViewer v;
};


// ----------------------------------------------------------------------------- : EOF
#endif
