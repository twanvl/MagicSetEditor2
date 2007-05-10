//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <script/functions/functions.hpp>
#include <script/functions/util.hpp>
#include <script/image.hpp>
// used by the functions
#include <data/set.hpp>
#include <data/stylesheet.hpp>
#include <data/symbol.hpp>
#include <data/field/symbol.hpp>
#include <gfx/generated_image.hpp>

DECLARE_TYPEOF_COLLECTION(SymbolVariationP);

bool parse_enum(const String&, ImageCombine& out);

// ----------------------------------------------------------------------------- : Utility

template <> inline GeneratedImageP from_script<GeneratedImageP>(const ScriptValueP& value) {
	return image_from_script(value);
}

// ----------------------------------------------------------------------------- : Image functions

SCRIPT_FUNCTION(linear_blend) {
	SCRIPT_PARAM(GeneratedImageP, image1);
	SCRIPT_PARAM(GeneratedImageP, image2);
	SCRIPT_PARAM(double, x1); SCRIPT_PARAM(double, y1);
	SCRIPT_PARAM(double, x2); SCRIPT_PARAM(double, y2);
	return new_intrusive6<LinearBlendImage>(image1, image2, x1,y1, x2,y2);
}

SCRIPT_FUNCTION(masked_blend) {
	SCRIPT_PARAM(GeneratedImageP, light);
	SCRIPT_PARAM(GeneratedImageP, dark);
	SCRIPT_PARAM(GeneratedImageP, mask);
	return new_intrusive3<MaskedBlendImage>(light, dark, mask);
}

SCRIPT_FUNCTION(combine_blend) {
	SCRIPT_PARAM(String, combine);
	SCRIPT_PARAM(GeneratedImageP, image1);
	SCRIPT_PARAM(GeneratedImageP, image2);
	ImageCombine image_combine;
	if (!parse_enum(combine, image_combine)) {
		throw ScriptError(_("Not a valid combine mode: '") + combine + _("'"));
	}
	return new_intrusive3<CombineBlendImage>(image1, image2, image_combine);
}

SCRIPT_FUNCTION(set_mask) {
	SCRIPT_PARAM(GeneratedImageP, image);
	SCRIPT_PARAM(GeneratedImageP, mask);
	return new_intrusive2<SetMaskImage>(image, mask);
}

SCRIPT_FUNCTION(set_combine) {
	SCRIPT_PARAM(String, combine);
	SCRIPT_PARAM(GeneratedImageP, input);
	ImageCombine image_combine;
	if (!parse_enum(combine, image_combine)) {
		throw ScriptError(_("Not a valid combine mode: '") + combine + _("'"));
	}
	return new_intrusive2<SetCombineImage>(input, image_combine);
}

SCRIPT_FUNCTION(symbol_variation) {
	// find symbol
	SCRIPT_PARAM(ValueP, symbol);
	SymbolValueP value = dynamic_pointer_cast<SymbolValue>(symbol);
	SCRIPT_PARAM(String, variation);
	// find style
	SCRIPT_PARAM(Set*, set);
	SCRIPT_OPTIONAL_PARAM_(CardP, card);
	SymbolStyleP style = dynamic_pointer_cast<SymbolStyle>(set->stylesheetFor(card)->styleFor(value->fieldP));
	if (!style) throw InternalError(_("Symbol value has a style of the wrong type"));
	// find variation
	FOR_EACH(v, style->variations) {
		if (v->name == variation) {
			// found it
			return new_intrusive3<SymbolToImage>(value->filename, value->last_update, v);
		}
	}
	throw ScriptError(_("Variation of symbol not found ('") + variation + _("')"));
}

SCRIPT_FUNCTION(built_in_image) {
	SCRIPT_PARAM(String, input);
	return new_intrusive1<BuiltInImage>(input);
}

// ----------------------------------------------------------------------------- : Init

void init_script_image_functions(Context& ctx) {
	ctx.setVariable(_("linear blend"),     script_linear_blend);
	ctx.setVariable(_("masked blend"),     script_masked_blend);
	ctx.setVariable(_("combine blend"),    script_combine_blend);
	ctx.setVariable(_("set mask"),         script_set_mask);
	ctx.setVariable(_("set combine"),      script_set_combine);
	ctx.setVariable(_("symbol variation"), script_symbol_variation);
	ctx.setVariable(_("built in image"),   script_built_in_image);
}
