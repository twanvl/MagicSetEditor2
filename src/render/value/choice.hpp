//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2009 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_RENDER_VALUE_CHOICE
#define HEADER_RENDER_VALUE_CHOICE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/value/viewer.hpp>
#include <data/field/choice.hpp>

// ----------------------------------------------------------------------------- : ChoiceValueViewer

/// Viewer that displays a choice value
class ChoiceValueViewer : public ValueViewer {
  public:
	DECLARE_VALUE_VIEWER(Choice) : ValueViewer(parent,style) {}
	
	virtual bool prepare(RotatedDC& dc);
	virtual void draw(RotatedDC& dc);
	virtual void onStyleChange(int);
};

bool prepare_choice_viewer(RotatedDC& dc, ValueViewer& viewer, ChoiceStyle& style, const String& value);
void draw_choice_viewer(RotatedDC& dc, ValueViewer& viewer, ChoiceStyle& style, const String& value);

// ----------------------------------------------------------------------------- : EOF
#endif
