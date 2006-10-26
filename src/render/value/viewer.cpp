//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <render/value/viewer.hpp>

// ----------------------------------------------------------------------------- : ValueViewer


// ----------------------------------------------------------------------------- : Development/debug

#ifdef _DEBUG

// REMOVEME

#include <data/field.hpp>
#include <data/field/choice.hpp>
#include <data/field/boolean.hpp>
#include <data/field/multiple_choice.hpp>
#include <data/field/color.hpp>
#include <data/field/image.hpp>
#include <data/field/symbol.hpp>
#include <data/field/text.hpp>

ValueViewerP ChoiceStyle        ::makeViewer(DataViewer& parent, const StyleP& thisP) { return ValueViewerP(); }
ValueViewerP BooleanStyle       ::makeViewer(DataViewer& parent, const StyleP& thisP) { return ValueViewerP(); }
ValueViewerP MultipleChoiceStyle::makeViewer(DataViewer& parent, const StyleP& thisP) { return ValueViewerP(); }
ValueViewerP ColorStyle         ::makeViewer(DataViewer& parent, const StyleP& thisP) { return ValueViewerP(); }
ValueViewerP ImageStyle         ::makeViewer(DataViewer& parent, const StyleP& thisP) { return ValueViewerP(); }
ValueViewerP SymbolStyle        ::makeViewer(DataViewer& parent, const StyleP& thisP) { return ValueViewerP(); }
ValueViewerP TextStyle          ::makeViewer(DataViewer& parent, const StyleP& thisP) { return ValueViewerP(); }

ValueEditorP ChoiceStyle        ::makeEditor(DataEditor& parent, const StyleP& thisP) { return ValueEditorP(); }
ValueEditorP BooleanStyle       ::makeEditor(DataEditor& parent, const StyleP& thisP) { return ValueEditorP(); }
ValueEditorP MultipleChoiceStyle::makeEditor(DataEditor& parent, const StyleP& thisP) { return ValueEditorP(); }
ValueEditorP ColorStyle         ::makeEditor(DataEditor& parent, const StyleP& thisP) { return ValueEditorP(); }
ValueEditorP ImageStyle         ::makeEditor(DataEditor& parent, const StyleP& thisP) { return ValueEditorP(); }
ValueEditorP SymbolStyle        ::makeEditor(DataEditor& parent, const StyleP& thisP) { return ValueEditorP(); }
ValueEditorP TextStyle          ::makeEditor(DataEditor& parent, const StyleP& thisP) { return ValueEditorP(); }

#endif