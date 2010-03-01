//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <script/functions/functions.hpp>
#include <script/functions/util.hpp>
#include <script/image.hpp>
// used by the functions
#include <data/set.hpp>
#include <data/card.hpp>
#include <data/stylesheet.hpp>
#include <data/symbol.hpp>
#include <data/field/symbol.hpp>
#include <gfx/generated_image.hpp>
#include <render/symbol/filter.hpp>

DECLARE_TYPEOF_COLLECTION(SymbolVariationP);

void parse_enum(const String&, ImageCombine& out);

// ----------------------------------------------------------------------------- : Utility

template <> inline GeneratedImageP from_script<GeneratedImageP>(const ScriptValueP& value) {
	return image_from_script(value);
}

SCRIPT_FUNCTION(to_image) {
	SCRIPT_PARAM_C(GeneratedImageP, input);
	return input;
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
	parse_enum(combine, image_combine);
	return new_intrusive3<CombineBlendImage>(image1, image2, image_combine);
}

SCRIPT_FUNCTION(set_mask) {
	SCRIPT_PARAM(GeneratedImageP, image);
	SCRIPT_PARAM(GeneratedImageP, mask);
	return new_intrusive2<SetMaskImage>(image, mask);
}

SCRIPT_FUNCTION(set_alpha) {
	SCRIPT_PARAM_C(GeneratedImageP, input);
	SCRIPT_PARAM(double, alpha);
	return new_intrusive2<SetAlphaImage>(input, alpha);
}

SCRIPT_FUNCTION(set_combine) {
	SCRIPT_PARAM(String, combine);
	SCRIPT_PARAM_C(GeneratedImageP, input);
	ImageCombine image_combine;
	parse_enum(combine, image_combine);
	return new_intrusive2<SetCombineImage>(input, image_combine);
}

SCRIPT_FUNCTION(saturate) {
	SCRIPT_PARAM_C(GeneratedImageP, input);
	SCRIPT_PARAM(double, amount);
	return new_intrusive2<SaturateImage>(input, amount);
}

SCRIPT_FUNCTION(enlarge) {
	SCRIPT_PARAM_C(GeneratedImageP, input);
	SCRIPT_PARAM_N(double, _("border size"), border_size);
	return new_intrusive2<EnlargeImage>(input, border_size);
}

SCRIPT_FUNCTION(crop) {
	SCRIPT_PARAM_C(GeneratedImageP, input);
	SCRIPT_PARAM_N(int, _("width"),       width);
	SCRIPT_PARAM_N(int, _("height"),      height);
	SCRIPT_PARAM_N(double, _("offset x"), offset_x);
	SCRIPT_PARAM_N(double, _("offset y"), offset_y);
	return new_intrusive5<CropImage>(input, width, height, offset_x, offset_y);
}

SCRIPT_FUNCTION(drop_shadow) {
	SCRIPT_PARAM_C(GeneratedImageP, input);
	SCRIPT_OPTIONAL_PARAM_N_(double, _("offset x"),    offset_x);
	SCRIPT_OPTIONAL_PARAM_N_(double, _("offset y"),    offset_y);
	SCRIPT_OPTIONAL_PARAM_N_(double, _("alpha"),       alpha);
	SCRIPT_OPTIONAL_PARAM_N_(double, _("blur radius"), blur_radius);
	SCRIPT_OPTIONAL_PARAM_N_(Color,  _("color"),       color);
	return new_intrusive6<DropShadowImage>(input, offset_x, offset_y, alpha, blur_radius, color);
}

SCRIPT_FUNCTION(symbol_variation) {
	// find symbol
	SCRIPT_PARAM(ScriptValueP, symbol); // TODO: change to input?
	ScriptObject<ValueP>* valueO = dynamic_cast<ScriptObject<ValueP>*>(symbol.get());
	SymbolValue* value = valueO ? dynamic_cast<SymbolValue*>(valueO->getValue().get()) : nullptr;
	String filename;
	if (value) {
		filename = value->filename;
	} else if (valueO) {
		throw ScriptErrorConversion(valueO->typeName(), _TYPE_("symbol" ));
	} else {
		filename = from_script<String>(symbol);
	}
	// known variation?
	SCRIPT_OPTIONAL_PARAM_(String, variation)
	if (value && variation_) {
		// find style
		SCRIPT_PARAM(Set*, set);
		SCRIPT_OPTIONAL_PARAM_(CardP, card);
		SymbolStyleP style = dynamic_pointer_cast<SymbolStyle>(set->stylesheetForP(card)->styleFor(value->fieldP));
		if (!style) throw InternalError(_("Symbol value has a style of the wrong type"));
		// find variation
		FOR_EACH(v, style->variations) {
			if (v->name == variation) {
				// found it
				return new_intrusive4<SymbolToImage>(value, filename, value->last_update, v);
			}
		}
		throw ScriptError(_("Variation of symbol not found ('") + variation + _("')"));
	} else {
		// custom variation
		SCRIPT_PARAM_N(double, _("border radius"), border_radius);
		SCRIPT_OPTIONAL_PARAM_N_(String, _("fill type"), fill_type);
		SymbolVariationP var(new SymbolVariation);
		var->border_radius = border_radius;
		if (fill_type == _("solid") || fill_type.empty()) {
			SCRIPT_PARAM_N(Color, _("fill color"),   fill_color);
			SCRIPT_PARAM_N(Color, _("border color"), border_color);
			var->filter = new_intrusive2<SolidFillSymbolFilter>(fill_color, border_color);
		} else if (fill_type == _("linear gradient")) {
			SCRIPT_PARAM_N(Color, _("fill color 1"),   fill_color_1);
			SCRIPT_PARAM_N(Color, _("border color 1"), border_color_1);
			SCRIPT_PARAM_N(Color, _("fill color 2"),   fill_color_2);
			SCRIPT_PARAM_N(Color, _("border color 2"), border_color_2);
			SCRIPT_PARAM_N(double, _("center x"), center_x);
			SCRIPT_PARAM_N(double, _("center y"), center_y);
			SCRIPT_PARAM_N(double, _("end x"), end_x);
			SCRIPT_PARAM_N(double, _("end y"), end_y);
			var->filter = new_intrusive8<LinearGradientSymbolFilter>(fill_color_1, border_color_1, fill_color_2, border_color_2
			                                                        ,center_x, center_y, end_x, end_y);
		} else if (fill_type == _("radial gradient")) {
			SCRIPT_PARAM_N(Color, _("fill color 1"),   fill_color_1);
			SCRIPT_PARAM_N(Color, _("border color 1"), border_color_1);
			SCRIPT_PARAM_N(Color, _("fill color 2"),   fill_color_2);
			SCRIPT_PARAM_N(Color, _("border color 2"), border_color_2);
			var->filter = new_intrusive4<RadialGradientSymbolFilter>(fill_color_1, border_color_1, fill_color_2, border_color_2);
		} else {
			throw ScriptError(_("Unknown fill type for symbol_variation: ") + fill_type);
		}
		return new_intrusive4<SymbolToImage>(value, filename, value ? value->last_update : Age(0), var);
	}
}

SCRIPT_FUNCTION(built_in_image) {
	SCRIPT_PARAM_C(String, input);
	return new_intrusive1<BuiltInImage>(input);
}

// ----------------------------------------------------------------------------- : Init

void init_script_image_functions(Context& ctx) {
	ctx.setVariable(_("to image"),         script_to_image);
	ctx.setVariable(_("linear blend"),     script_linear_blend);
	ctx.setVariable(_("masked blend"),     script_masked_blend);
	ctx.setVariable(_("combine blend"),    script_combine_blend);
	ctx.setVariable(_("set mask"),         script_set_mask);
	ctx.setVariable(_("set alpha"),        script_set_alpha);
	ctx.setVariable(_("set combine"),      script_set_combine);
	ctx.setVariable(_("saturate"),         script_saturate);
	ctx.setVariable(_("enlarge"),          script_enlarge);
	ctx.setVariable(_("crop"),             script_crop);
	ctx.setVariable(_("drop shadow"),      script_drop_shadow);
	ctx.setVariable(_("symbol variation"), script_symbol_variation);
	ctx.setVariable(_("built in image"),   script_built_in_image);
}
