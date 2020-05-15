//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/editor.hpp>
#include <render/value/information.hpp>

// ----------------------------------------------------------------------------- : InfoValueEditor

/// An editor 'control' for editing InfoValues
class InfoValueEditor : public InfoValueViewer, public ValueEditor {
public:
  DECLARE_VALUE_EDITOR(Info);
  
  void determineSize(bool) override;
  bool drawLabel() const override { return false; }
};

