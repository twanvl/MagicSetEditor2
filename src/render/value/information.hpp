//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

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


