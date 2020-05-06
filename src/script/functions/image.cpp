//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
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

void parse_enum(const String&, ImageCombine& out);

// ----------------------------------------------------------------------------- : Utility

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
  return make_intrusive<LinearBlendImage>(image1, image2, x1,y1, x2,y2);
}

SCRIPT_FUNCTION(masked_blend) {
  SCRIPT_PARAM(GeneratedImageP, light);
  SCRIPT_PARAM(GeneratedImageP, dark);
  SCRIPT_PARAM(GeneratedImageP, mask);
  return make_intrusive<MaskedBlendImage>(light, dark, mask);
}

SCRIPT_FUNCTION(combine_blend) {
  SCRIPT_PARAM(String, combine);
  SCRIPT_PARAM(GeneratedImageP, image1);
  SCRIPT_PARAM(GeneratedImageP, image2);
  ImageCombine image_combine;
  parse_enum(combine, image_combine);
  return make_intrusive<CombineBlendImage>(image1, image2, image_combine);
}

SCRIPT_FUNCTION(set_mask) {
  SCRIPT_PARAM(GeneratedImageP, image);
  SCRIPT_PARAM(GeneratedImageP, mask);
  return make_intrusive<SetMaskImage>(image, mask);
}

SCRIPT_FUNCTION(set_alpha) {
  SCRIPT_PARAM_C(GeneratedImageP, input);
  SCRIPT_PARAM(double, alpha);
  return make_intrusive<SetAlphaImage>(input, alpha);
}

SCRIPT_FUNCTION(set_combine) {
  SCRIPT_PARAM(String, combine);
  SCRIPT_PARAM_C(GeneratedImageP, input);
  ImageCombine image_combine;
  parse_enum(combine, image_combine);
  return make_intrusive<SetCombineImage>(input, image_combine);
}

SCRIPT_FUNCTION(saturate) {
  SCRIPT_PARAM_C(GeneratedImageP, input);
  SCRIPT_PARAM(double, amount);
  return make_intrusive<SaturateImage>(input, amount);
}

SCRIPT_FUNCTION(invert_image) {
  SCRIPT_PARAM_C(GeneratedImageP, input);
  return make_intrusive<InvertImage>(input);
}

SCRIPT_FUNCTION(recolor_image) {
  SCRIPT_PARAM_C(GeneratedImageP, input);
  SCRIPT_OPTIONAL_PARAM(Color, red) {
    SCRIPT_PARAM(Color, green);
    SCRIPT_PARAM(Color, blue);
    SCRIPT_PARAM_DEFAULT(Color, white, Color(255,255,255));
    return make_intrusive<RecolorImage2>(input,red,green,blue,white);
  } else {
    SCRIPT_PARAM(Color, color);
    return make_intrusive<RecolorImage>(input,color);
  }
}

SCRIPT_FUNCTION(enlarge) {
  SCRIPT_PARAM_C(GeneratedImageP, input);
  SCRIPT_PARAM(double, border_size);
  return make_intrusive<EnlargeImage>(input, border_size);
}

SCRIPT_FUNCTION(crop) {
  SCRIPT_PARAM_C(GeneratedImageP, input);
  SCRIPT_PARAM(int, width);
  SCRIPT_PARAM(int, height);
  SCRIPT_PARAM(double, offset_x);
  SCRIPT_PARAM(double, offset_y);
  return make_intrusive<CropImage>(input, width, height, offset_x, offset_y);
}

SCRIPT_FUNCTION(flip_horizontal) {
  SCRIPT_PARAM_C(GeneratedImageP, input);
  return make_intrusive<FlipImageHorizontal>(input);
}

SCRIPT_FUNCTION(flip_vertical) {
  SCRIPT_PARAM_C(GeneratedImageP, input);
  return make_intrusive<FlipImageVertical>(input);
}

SCRIPT_FUNCTION(rotate) {
  SCRIPT_PARAM_C(GeneratedImageP, input);
  SCRIPT_PARAM(Degrees, angle);
  return make_intrusive<RotateImage>(input,deg_to_rad(angle));
}

SCRIPT_FUNCTION(drop_shadow) {
  SCRIPT_PARAM_C(GeneratedImageP, input);
  SCRIPT_OPTIONAL_PARAM_(double, offset_x);
  SCRIPT_OPTIONAL_PARAM_(double, offset_y);
  SCRIPT_OPTIONAL_PARAM_(double, alpha);
  SCRIPT_OPTIONAL_PARAM_(double, blur_radius);
  SCRIPT_OPTIONAL_PARAM_(Color,  color);
  return make_intrusive<DropShadowImage>(input, offset_x, offset_y, alpha, blur_radius, color);
}

SCRIPT_FUNCTION(symbol_variation) {
  // find symbol
  SCRIPT_PARAM(ScriptValueP, symbol); // TODO: change to input?
  ScriptObject<ValueP>* valueO = dynamic_cast<ScriptObject<ValueP>*>(symbol.get());
  SymbolValue* value = valueO ? dynamic_cast<SymbolValue*>(valueO->getValue().get()) : nullptr;
  LocalFileName filename;
  if (value) {
    filename = value->filename;
  } else if (valueO) {
    throw ScriptErrorConversion(valueO->typeName(), _TYPE_("symbol" ));
  } else {
    filename = LocalFileName::fromReadString(symbol->toString());
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
        return make_intrusive<SymbolToImage>(value, filename, value->last_update, v);
      }
    }
    throw ScriptError(_("Variation of symbol not found ('") + variation + _("')"));
  } else {
    // custom variation
    SCRIPT_PARAM(double, border_radius);
    SCRIPT_OPTIONAL_PARAM_(String, fill_type);
    SymbolVariationP var(new SymbolVariation);
    var->border_radius = border_radius;
    if (fill_type == _("solid") || fill_type.empty()) {
      SCRIPT_PARAM(Color, fill_color);
      SCRIPT_PARAM(Color, border_color);
      var->filter = make_intrusive<SolidFillSymbolFilter>(fill_color, border_color);
    } else if (fill_type == _("linear gradient")) {
      SCRIPT_PARAM(Color, fill_color_1);
      SCRIPT_PARAM(Color, border_color_1);
      SCRIPT_PARAM(Color, fill_color_2);
      SCRIPT_PARAM(Color, border_color_2);
      SCRIPT_PARAM(double, center_x);
      SCRIPT_PARAM(double, center_y);
      SCRIPT_PARAM(double, end_x);
      SCRIPT_PARAM(double, end_y);
      var->filter = make_intrusive<LinearGradientSymbolFilter>(fill_color_1, border_color_1, fill_color_2, border_color_2, center_x, center_y, end_x, end_y);
    } else if (fill_type == _("radial gradient")) {
      SCRIPT_PARAM(Color, fill_color_1);
      SCRIPT_PARAM(Color, border_color_1);
      SCRIPT_PARAM(Color, fill_color_2);
      SCRIPT_PARAM(Color, border_color_2);
      var->filter = make_intrusive<RadialGradientSymbolFilter>(fill_color_1, border_color_1, fill_color_2, border_color_2);
    } else {
      throw ScriptError(_("Unknown fill type for symbol_variation: ") + fill_type);
    }
    return make_intrusive<SymbolToImage>(value, filename, value ? value->last_update : Age(0), var);
  }
}

SCRIPT_FUNCTION(built_in_image) {
  SCRIPT_PARAM_C(String, input);
  return make_intrusive<BuiltInImage>(input);
}

// ----------------------------------------------------------------------------- : Init

void init_script_image_functions(Context& ctx) {
  ctx.setVariable(_("to_image"),         script_to_image);
  ctx.setVariable(_("linear_blend"),     script_linear_blend);
  ctx.setVariable(_("masked_blend"),     script_masked_blend);
  ctx.setVariable(_("combine_blend"),    script_combine_blend);
  ctx.setVariable(_("set_mask"),         script_set_mask);
  ctx.setVariable(_("set_alpha"),        script_set_alpha);
  ctx.setVariable(_("set_combine"),      script_set_combine);
  ctx.setVariable(_("saturate"),         script_saturate);
  ctx.setVariable(_("invert_image"),     script_invert_image);
  ctx.setVariable(_("recolor_image"),    script_recolor_image);
  ctx.setVariable(_("enlarge"),          script_enlarge);
  ctx.setVariable(_("crop"),             script_crop);
  ctx.setVariable(_("flip_horizontal"),  script_flip_horizontal);
  ctx.setVariable(_("flip_vertical"),    script_flip_vertical);
  ctx.setVariable(_("rotate"),           script_rotate);
  ctx.setVariable(_("rotate_image"),     script_rotate);
  ctx.setVariable(_("drop_shadow"),      script_drop_shadow);
  ctx.setVariable(_("symbol_variation"), script_symbol_variation);
  ctx.setVariable(_("built_in_image"),   script_built_in_image);
}
