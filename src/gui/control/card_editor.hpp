//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_CONTROL_CARD_EDITOR
#define HEADER_GUI_CONTROL_CARD_EDITOR

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/card_viewer.hpp>

// ----------------------------------------------------------------------------- : DataEditor

/// An editor for data values (usually a card)
class DataEditor : public CardViewer {
  public:
	DataEditor(Window* parent, int id, long style = 0);
  private:
	DECLARE_EVENT_TABLE();
};

/// By default a DataEditor edits cards
typedef DataEditor CardEditor;

// ----------------------------------------------------------------------------- : EOF
#endif
