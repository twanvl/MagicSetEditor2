//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/control/card_editor.hpp>

// ----------------------------------------------------------------------------- : DataEditor

DataEditor::DataEditor(Window* parent, int id, long style)
	: CardViewer(parent, id, style)
{}


// ----------------------------------------------------------------------------- : Event table

BEGIN_EVENT_TABLE(DataEditor, CardViewer)
END_EVENT_TABLE  ()
