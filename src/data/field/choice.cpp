//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/field/choice.hpp>
#include <util/io/package.hpp>
#include <wx/imaglist.h>

DECLARE_TYPEOF_COLLECTION(ChoiceField::ChoiceP);
DECLARE_TYPEOF(map<String COMMA ScriptableImage>);

// ----------------------------------------------------------------------------- : ChoiceField

ChoiceField::ChoiceField()
	: choices((Choice*)new Choice)
	, default_name(_("Default"))
{}

IMPLEMENT_FIELD_TYPE(Choice, "choice");

void ChoiceField::initDependencies(Context& ctx, const Dependency& dep) const {
	Field        ::initDependencies(ctx, dep);
	script        .initDependencies(ctx, dep);
	default_script.initDependencies(ctx, dep);
}

IMPLEMENT_REFLECTION(ChoiceField) {
	REFLECT_BASE(Field);
	REFLECT_N("choices", choices->choices);
	REFLECT(script);
	REFLECT_N("default", default_script);
	REFLECT(initial);
	REFLECT(default_name);
	REFLECT_IF_READING {
		choices->initIds();
	}
	REFLECT(choice_colors);
	REFLECT(choice_colors_cardlist);
}

// ----------------------------------------------------------------------------- : ChoiceField::Choice

ChoiceField::Choice::Choice()
	: first_id(0)
	, line_below(false), enabled(true), type(CHOICE_TYPE_CHECK)
{}
ChoiceField::Choice::Choice(const String& name)
	: name(name), first_id(0)
	, line_below(false), enabled(true), type(CHOICE_TYPE_CHECK)
{}


bool ChoiceField::Choice::isGroup() const {
	return !choices.empty();
}
bool ChoiceField::Choice::hasDefault() const {
	return !isGroup() || !default_name.empty();
}


int ChoiceField::Choice::initIds() {
	int id = first_id + (hasDefault() ? 1 : 0);
	FOR_EACH(c, choices) {
		c->first_id = id;
		id = c->initIds();
	}
	return id;
}
int ChoiceField::Choice::choiceCount() const {
	return lastId() - first_id;
}
int ChoiceField::Choice::lastId() const {
	if (isGroup()) {
		// last id of last choice
		return choices.back()->lastId();
	} else {
		return first_id + 1;
	}
}


int ChoiceField::Choice::choiceId(const String& search_name) const {
	if (hasDefault() && search_name == name) {
		return first_id;
	} else if (name.empty()) { // no name for this group, forward to all children
		FOR_EACH_CONST(c, choices) {
			int sub_id = c->choiceId(search_name);
			if (sub_id != -1) return sub_id;
		}
	} else if (isGroup() && starts_with(search_name, name + _(" "))) {
		String sub_name = search_name.substr(name.size() + 1);
		FOR_EACH_CONST(c, choices) {
			int sub_id = c->choiceId(sub_name);
			if (sub_id != -1) return sub_id;
		}
	}
	return -1;
}

String ChoiceField::Choice::choiceName(int id) const {
	if (hasDefault() && id == first_id) {
		return name;
	} else {
		FOR_EACH_CONST_REVERSE(c, choices) { // take the last one that still contains id
			if (id >= c->first_id) {
				if (name.empty()) {
					return c->choiceName(id);
				} else {
					return name + _(" ") + c->choiceName(id);
				}
			}
		}
	}
	return _("");
}

String ChoiceField::Choice::choiceNameNice(int id) const {
	if (!isGroup() && id == first_id) {
		return name;
	} else if (hasDefault() && id == first_id) {
		return default_name;
	} else {
		FOR_EACH_CONST_REVERSE(c, choices) {
			if (id == c->first_id) {
				return c->name; // we don't want "<group> default"
			} else if (id > c->first_id) {
				return c->choiceNameNice(id);
			}
		}
	}
	return _("");
}


IMPLEMENT_REFLECTION_ENUM(ChoiceChoiceType) {
	VALUE_N("check", CHOICE_TYPE_CHECK);
	VALUE_N("radio", CHOICE_TYPE_RADIO);
}

IMPLEMENT_REFLECTION_NO_GET_MEMBER(ChoiceField::Choice) {
	if (isGroup() || line_below || enabled.isScripted() || (tag.reading() && tag.isComplex())) {
		// complex values are groups
		REFLECT(name);
		REFLECT_N("group_choice", default_name);
		REFLECT(choices);
		REFLECT(line_below);
		REFLECT(enabled);
		REFLECT(type);
	} else {
		REFLECT_NAMELESS(name);
	}
}

template <> void GetDefaultMember::handle(const ChoiceField::Choice& c) {
	if (!c.isGroup()) handle(c.name);
}

template <> void GetMember::handle(const ChoiceField::Choice& c) {
	handle(_("name"),         c.name);
	handle(_("group choice"), c.default_name);
	handle(_("choices"),      c.choices);
}

// ----------------------------------------------------------------------------- : ChoiceStyle

ChoiceStyle::ChoiceStyle(const ChoiceFieldP& field)
	: Style(field)
	, popup_style(POPUP_DROPDOWN)
	, render_style(RENDER_TEXT)
	, choice_images_initialized(false)
	, combine(COMBINE_NORMAL)
	, alignment(ALIGN_STRETCH)
	, thumbnails(nullptr)
{}

ChoiceStyle::~ChoiceStyle() {
	delete thumbnails;
}

void ChoiceStyle::initImage() {
	if (image.isSet() || choice_images.empty()) return;
	// for, for example:
	//     choice images:
	//          a: {uvw}
	//          b: {xyz}
	// generate the script:
	//     [a: {uvw}, b: {xyz}][input]() or else nil
	// or in bytecode
	//         PUSH_CONST [a: {uvw}, b: {xyz}]
	//         GET_VAR    input
	//        MEMBER
	//       CALL       0
	//       PUSH_CONST nil
	//      OR_ELSE
	//     RET
	intrusive_ptr<ScriptCustomCollection> lookup(new ScriptCustomCollection());
	FOR_EACH(ci, choice_images) {
		lookup->key_value[ci.first] = ci.second.getScriptP();
	}
	Script& script = image.getScript();
	script.addInstruction(I_PUSH_CONST, lookup);
	script.addInstruction(I_GET_VAR,    SCRIPT_VAR_input);
	script.addInstruction(I_BINARY,     I_MEMBER);
	script.addInstruction(I_CALL,       0);
	script.addInstruction(I_PUSH_CONST, script_nil);
	script.addInstruction(I_BINARY,     I_OR_ELSE);
	script.addInstruction(I_RET);
}

int ChoiceStyle::update(Context& ctx) {
	// Don't update the choice images, leave that to invalidate()
	int change = Style       ::update(ctx)
	           | font         .update(ctx) * CHANGE_OTHER
	           | mask_filename.update(ctx) * CHANGE_MASK;
	if (!choice_images_initialized) {
		// we only want to do this once because it is rather slow, other updates are handled by dependencies
		choice_images_initialized = true;
		FOR_EACH(ci, choice_images) {
			if (ci.second.update(ctx)) {
				change |= CHANGE_OTHER;
				// TODO : remove this thumbnail
			}
		}
	}
	return change;
}
void ChoiceStyle::initDependencies(Context& ctx, const Dependency& dep) const {
	Style::initDependencies(ctx, dep);
	FOR_EACH_CONST(ci, choice_images) {
		ci.second.initDependencies(ctx, dep);
	}
}
void ChoiceStyle::invalidate() {
	// TODO : this is also done in update(), once should be enough
	// Update choice images and thumbnails
	int end = field().choices->lastId();
	thumbnails_status.resize(end, THUMB_NOT_MADE);
	for (int i = 0 ; i < end ; ++i) {
		if (thumbnails_status[i] == THUMB_OK) thumbnails_status[i] = THUMB_CHANGED;
	}
	tellListeners(CHANGE_OTHER);
}

void ChoiceStyle::loadMask(Package& pkg) {
	if (mask.Ok() || mask_filename().empty()) return;
	// load file
	InputStreamP image_file = pkg.openIn(mask_filename);
	mask.LoadFile(*image_file);
}

IMPLEMENT_REFLECTION_ENUM(ChoicePopupStyle) {
	VALUE_N("dropdown",	POPUP_DROPDOWN);
	VALUE_N("menu",		POPUP_MENU);
	VALUE_N("in place",	POPUP_DROPDOWN_IN_PLACE);
}

IMPLEMENT_REFLECTION_ENUM(ChoiceRenderStyle) {
	VALUE_N("text",				RENDER_TEXT);
	VALUE_N("image",			RENDER_IMAGE);
	VALUE_N("both",				RENDER_BOTH);
	VALUE_N("hidden",			RENDER_HIDDEN);
	VALUE_N("image hidden",		RENDER_HIDDEN_IMAGE);
	VALUE_N("checklist",		RENDER_TEXT_CHECKLIST);
	VALUE_N("image checklist",	RENDER_IMAGE_CHECKLIST);
	VALUE_N("both checklist",	RENDER_BOTH_CHECKLIST);
	VALUE_N("text list",		RENDER_TEXT_LIST);
	VALUE_N("image list",		RENDER_IMAGE_LIST);
	VALUE_N("both list",		RENDER_BOTH_LIST);
}

template <typename T> void reflect_content(T& tag,         const ChoiceStyle& cs) {}
template <>           void reflect_content(GetMember& tag, const ChoiceStyle& cs) {
	REFLECT_N("content_width",  cs.content_width);
	REFLECT_N("content_height", cs.content_height);
}

IMPLEMENT_REFLECTION(ChoiceStyle) {
	REFLECT_ALIAS(300, "card list colors", "colors card list");
	REFLECT_BASE(Style);
	REFLECT(popup_style);
	REFLECT(render_style);
	REFLECT_N("mask",mask_filename);
	REFLECT(combine);
	REFLECT(alignment);
	REFLECT(font);
	REFLECT(image);
	REFLECT(choice_images);
	reflect_content(tag, *this);
}

// ----------------------------------------------------------------------------- : ChoiceValue

ChoiceValue::ChoiceValue(const ChoiceFieldP& field, bool initial_first_choice)
	: Value(field)
	, value( !field->initial.empty() ? field->initial
	       : initial_first_choice    ? field->choices->choiceName(0)
	       :                           _("")
	       , true)
{}

String ChoiceValue::toString() const {
	return value();
}
bool ChoiceValue::update(Context& ctx) {
	bool change = field().default_script.invokeOnDefault(ctx, value)
	            | field().        script.invokeOn(ctx, value);
	Value::update(ctx);
	return change;
}

IMPLEMENT_REFLECTION_NAMELESS(ChoiceValue) {
	if (fieldP->save_value || tag.scripting() || tag.reading()) REFLECT_NAMELESS(value);
}
