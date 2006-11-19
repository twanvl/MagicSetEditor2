//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/control/card_editor.hpp>
#include <gui/value/editor.hpp>
#include <data/field.hpp>

// ----------------------------------------------------------------------------- : DataEditor

#define FOR_EACH_EDITOR			\
	FOR_EACH(v, viewers)		\
		if (ValueEditorP = static_pointer_cast<ValueEditor>(v))

DataEditor::DataEditor(Window* parent, int id, long style)
	: CardViewer(parent, id, style)
{}

ValueViewerP DataEditor::makeViewer(const StyleP& style) {
	return style->makeEditor(*this, style);
}

// ----------------------------------------------------------------------------- : Event table

BEGIN_EVENT_TABLE(DataEditor, CardViewer)
END_EVENT_TABLE  ()
