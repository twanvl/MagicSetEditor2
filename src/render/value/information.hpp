//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_RENDER_VALUE_INFORMATION
#define HEADER_RENDER_VALUE_INFORMATION

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/value/viewer.hpp>
#include <data/field/information.hpp>

// ----------------------------------------------------------------------------- : InfoValueViewer

/// Viewer that displays a text value
class InfoValueViewer : public ValueViewer {
  public:
	DECLARE_VALUE_VIEWER(Info) : ValueViewer(parent,style) {}
	
	virtual void draw(RotatedDC& dc);
};


// ----------------------------------------------------------------------------- : EOF
#endif
