//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
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
	
	virtual void draw(RotatedDC& dc);
};

// ----------------------------------------------------------------------------- : EOF
#endif
