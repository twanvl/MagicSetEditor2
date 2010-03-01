//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/field/text.hpp>
#include <util/tagged_string.hpp>
#include <script/script.hpp>

// ----------------------------------------------------------------------------- : TextField

TextField::TextField()
	: multi_line(false)
	, default_name(_("Default"))
{}

IMPLEMENT_FIELD_TYPE(Text, "text");

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
	REFLECT(default_name);
}

// ----------------------------------------------------------------------------- : TextStyle

TextStyle::TextStyle(const TextFieldP& field)
	: Style(field)
	, always_symbol(false), allow_formating(true)
	, alignment(ALIGN_TOP_LEFT)
	, padding_left  (0), padding_left_min  (10000)
	, padding_right (0), padding_right_min (10000)
	, padding_top   (0), padding_top_min   (10000)
	, padding_bottom(0), padding_bottom_min(10000)
	, line_height_soft(1.0)
	, line_height_hard(1.0)
	, line_height_line(1.0)
	, line_height_soft_max(0.0)
	, line_height_hard_max(0.0)
	, line_height_line_max(0.0)
	, paragraph_height(-1)
	, direction(LEFT_TO_RIGHT)
	, content_width(0), content_height(0), content_lines(0)
{}

double TextStyle::getStretch() const {
	if (content_width > 0 && (alignment() & ALIGN_STRETCH)) {
		double factor = (width - padding_left - padding_right) / content_width;
		if (!(alignment() & ALIGN_IF_OVERFLOW) || factor < 1.0) {
			return factor;
		}
	}
	return 1.0;
}

int TextStyle::update(Context& ctx) {
	return Style     ::update(ctx)
	     | font       .update(ctx) * CHANGE_OTHER
	     | symbol_font.update(ctx) * CHANGE_OTHER
	     | alignment  .update(ctx) * CHANGE_OTHER
	     | ( padding_left        .update(ctx)
	       | padding_left_min    .update(ctx)
	       | padding_right       .update(ctx)
	       | padding_right_min   .update(ctx)
	       | padding_top         .update(ctx)
	       | padding_top_min     .update(ctx)
	       | padding_bottom      .update(ctx)
	       | padding_bottom_min  .update(ctx)
	       | line_height_soft    .update(ctx)
	       | line_height_hard    .update(ctx)
	       | line_height_line    .update(ctx)
	       | line_height_soft_max.update(ctx)
	       | line_height_hard_max.update(ctx)
	       | line_height_line_max.update(ctx)
	       ) * CHANGE_OTHER;
}
void TextStyle::initDependencies(Context& ctx, const Dependency& dep) const {
	Style     ::initDependencies(ctx, dep);
//	font       .initDependencies(ctx, dep);
//	symbol_font.initDependencies(ctx, dep);
}
void TextStyle::checkContentDependencies(Context& ctx, const Dependency& dep) const {
	Style   ::checkContentDependencies(ctx, dep);
	alignment.initDependencies(ctx, dep);
}

template <typename T> void reflect_content(T& tag,         const TextStyle& ts) {}
template <>           void reflect_content(GetMember& tag, const TextStyle& ts) {
	REFLECT_N("content_width",  ts.content_width);
	REFLECT_N("content_height", ts.content_height);
	REFLECT_N("content_lines",  ts.content_lines);
}

IMPLEMENT_REFLECTION(TextStyle) {
	REFLECT_BASE(Style);
	REFLECT(font);
	REFLECT(symbol_font);
	REFLECT(always_symbol);
	REFLECT(allow_formating);
	REFLECT(alignment);
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
	REFLECT(line_height_soft_max);
	REFLECT(line_height_hard_max);
	REFLECT(line_height_line_max);
	REFLECT(paragraph_height);
	REFLECT(direction);
	reflect_content(tag, *this);
}

// ----------------------------------------------------------------------------- : TextValue

String TextValue::toString() const {
	return untag_hide_sep(value());
}
bool TextValue::update(Context& ctx) {
	updateAge();
	WITH_DYNAMIC_ARG(last_update_age,     last_update.get());
	WITH_DYNAMIC_ARG(value_being_updated, this);
	bool change = field().default_script.invokeOnDefault(ctx, value)
	            | field().        script.invokeOn(ctx, value);
	if (change) last_update.update();
	updateSortValue(ctx);
	return change;
}

IMPLEMENT_REFLECTION_NAMELESS(TextValue) {
	if (fieldP->save_value || tag.scripting() || tag.reading()) REFLECT_NAMELESS(value);
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
