//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/value/editor.hpp>
#include <gui/value/text.hpp>
#include <gui/value/choice.hpp>
#include <gui/value/multiple_choice.hpp>
#include <gui/value/image.hpp>
#include <gui/value/symbol.hpp>
#include <gui/value/color.hpp>
#include <gui/value/information.hpp>

// ----------------------------------------------------------------------------- : ValueEditor

// ----------------------------------------------------------------------------- : Type dispatch

#define IMPLEMENT_MAKE_EDITOR(Type)														\
	ValueViewerP Type##Style::makeEditor(DataEditor& parent, const StyleP& thisP) {		\
		assert(thisP.get() == this);													\
		return ValueViewerP(new Type##ValueEditor(parent, static_pointer_cast<Type##Style>(thisP)));	\
	}

IMPLEMENT_MAKE_EDITOR(Text);
IMPLEMENT_MAKE_EDITOR(Choice);
IMPLEMENT_MAKE_EDITOR(MultipleChoice);
IMPLEMENT_MAKE_EDITOR(Color);
IMPLEMENT_MAKE_EDITOR(Image);
IMPLEMENT_MAKE_EDITOR(Symbol);
IMPLEMENT_MAKE_EDITOR(Info);
