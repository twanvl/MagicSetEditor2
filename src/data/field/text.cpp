//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/field/text.hpp>
#include <util/tagged_string.hpp>
#include <script/script.hpp>

// ----------------------------------------------------------------------------- : TextField

TextField::TextField()
	: multi_line(false), move_cursor_with_sort(false)
	, default_name(_("Default"))
{}

IMPLEMENT_FIELD_TYPE(Text)

String TextField::typeName() const {
	return _("text");
}

void TextField::initDependencies(Context& ctx, const Dependency& dep) const {
	Field        ::initDependencies(ctx, dep);
	script        .initDependencies(ctx, dep);
	default_script.initDependencies(ctx, dep);
}


IMPLEMENT_REFLECTION(TextField) {
	REFLECT_BASE(Field);
	REFLECT(multi_line);
	REFLECT(script);
	REFLECT_N("default", default_script);
	REFLECT(move_cursor_with_sort);
	REFLECT(default_name);
}

// ----------------------------------------------------------------------------- : TextStyle

TextStyle::TextStyle(const TextFieldP& field)
	: Style(field)
	, always_symbol(false), allow_formating(true)
	, alignment(ALIGN_TOP_LEFT)
	, angle(0)
	, padding_left  (0), padding_left_min  (10000)
	, padding_right (0), padding_right_min (10000)
	, padding_top   (0), padding_top_min   (10000)
	, padding_bottom(0), padding_bottom_min(10000)
	, line_height_soft(1.0)
	, line_height_hard(1.0)
	, line_height_line(1.0)
	, direction(LEFT_TO_RIGHT)
{}

bool TextStyle::update(Context& ctx) {
	return Style     ::update(ctx)
	     | font       .update(ctx)
	     | symbol_font.update(ctx);
}
void TextStyle::initDependencies(Context& ctx, const Dependency& dep) const {
	Style     ::initDependencies(ctx, dep);
//	font       .initDependencies(ctx, dep);
//	symbol_font.initDependencies(ctx, dep);
}

IMPLEMENT_REFLECTION(TextBackground) {
	REFLECT(image);
	REFLECT_N("displacement_x", displacement.width);
	REFLECT_N("displacement_y", displacement.height);
}

IMPLEMENT_REFLECTION(TextStyle) {
	REFLECT_BASE(Style);
	REFLECT(font);
	REFLECT(symbol_font);
	REFLECT(always_symbol);
	REFLECT(allow_formating);
	REFLECT(alignment);
	REFLECT(angle);
	REFLECT(padding_left);
	REFLECT(padding_right);
	REFLECT(padding_top);
	REFLECT(padding_bottom);
	REFLECT(padding_left_min);
	REFLECT(padding_right_min);
	REFLECT(padding_top_min);
	REFLECT(padding_bottom_min);
	REFLECT(line_height_soft);
	REFLECT(line_height_hard);
	REFLECT(line_height_line);
	REFLECT_N("mask", mask_filename);
	REFLECT(direction);
	REFLECT(text_background);
	REFLECT(text_background_left);
	REFLECT(text_background_right);
}

// ----------------------------------------------------------------------------- : TextValue

String TextValue::toString() const {
	return untag_hide_sep(value());
}
bool TextValue::update(Context& ctx) {
	Value::update(ctx);
	WITH_DYNAMIC_ARG(last_update_age, value.isDefault() ? 0 : last_update.get());
	bool change = field().default_script.invokeOnDefault(ctx, value)
	            | field().        script.invokeOn(ctx, value);
	if (change) last_update.update();
	return change;
}

IMPLEMENT_REFLECTION_NAMELESS(TextValue) {
	if (fieldP->save_value || tag.scripting()) REFLECT_NAMELESS(value);
}

// ----------------------------------------------------------------------------- : FakeTextValue

FakeTextValue::FakeTextValue(const TextFieldP& field, String* underlying, bool editable, bool untagged)
	: TextValue(field), underlying(underlying)
	, editable(editable), untagged(untagged)
{}

void FakeTextValue::store() {
	if (underlying) {
		if (editable) {
			*underlying = untagged ? untag(value) : value();
		} else {
			retrieve();
		}
	}
}
void FakeTextValue::retrieve() {
	if (underlying) {
		value.assign(untagged ? escape(*underlying) : *underlying);
	} else {
		value.assign(wxEmptyString);
	}
}

void FakeTextValue::onAction(Action& a, bool undone) {
	store();
}

bool FakeTextValue::equals(const Value* that) {
	if (this == that) return true;
	if (!underlying)  return false;
	const FakeTextValue* thatT = dynamic_cast<const FakeTextValue*>(that);
	if (!thatT || underlying != thatT->underlying) return false;
	// update the value
	retrieve();
	return true;
}
