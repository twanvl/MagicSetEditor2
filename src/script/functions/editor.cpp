//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <script/functions/functions.hpp>
#include <script/functions/util.hpp>
#include <util/tagged_string.hpp>
#include <data/set.hpp>
#include <data/game.hpp>
#include <data/field/text.hpp>

DECLARE_TYPEOF_COLLECTION(FieldP);
DECLARE_TYPEOF_COLLECTION(TextValue*);
DECLARE_TYPEOF_COLLECTION(String);

// ----------------------------------------------------------------------------- : Combined editor

// Combining multiple (text) values into a single one
// The combined value is  value1 <sep>something</sep> value2 <sep>something</sep> value3
// 

SCRIPT_FUNCTION_WITH_DEP(combined_editor) {
	// read 'field#' arguments
	vector<TextValue*> values;
	for (int i = 0 ; ; ++i) {
		String name = _("field"); if (i > 0) name = name << i;
		SCRIPT_OPTIONAL_PARAM_N(ValueP, name, value) {
			TextValue* text_value = dynamic_cast<TextValue*>(value.get());
			if (!text_value) throw ScriptError(_("Argument '")+name+_("' should be a text field")); 
			values.push_back(text_value);
		} else if (i > 0) break;
	}
	if (values.empty()) {
		throw ScriptError(_("No fields specified for combined_editor"));
	}
	// read 'separator#' arguments
	vector<String> separators;
	for (int i = 0 ; ; ++i) {
		String name = _("separator"); if (i > 0) name = name << i;
		SCRIPT_OPTIONAL_PARAM_N(String, name, separator) {
			separators.push_back(separator);
		} else if (i > 0) break;
	}
	if (separators.size() < values.size() - 1) {
		throw ScriptError(String::Format(_("Not enough separators for combine_editor, expected %d"), values.size()-1));
	}
	// split the value
	SCRIPT_PARAM(String, value);
	vector<String> value_parts;
	size_t pos = value.find(_("<sep"));
	while (pos != String::npos) {
		value_parts.push_back(value.substr(0, pos));
		value = value.substr(min(match_close_tag_end(value,pos), value.size()));
		pos = value.find(_("<sep"));
	}
	value_parts.push_back(value);
	value_parts.resize(values.size()); // TODO: what if there are more value_parts than values?
	// update the values if our input value is newer?
	Age new_value_update = last_update_age();
	FOR_EACH_2(v, values, nv, value_parts) {
		if (v->value() != nv && v->last_update < new_value_update) {
			// TODO : type over
			v->value.assign(nv);
			v->update(ctx);
		}
		nv = v->value();
	}
	// options
	SCRIPT_PARAM_DEFAULT_N(bool, _("hide when empty"),   hide_when_empty,   false);
	SCRIPT_PARAM_DEFAULT_N(bool, _("soft before empty"), soft_before_empty, false);
	// recombine the parts
	String new_value = value_parts.front();
	for (size_t i = 1 ; i < value_parts.size() ; ++i) {
		if (value_parts[i].empty() && new_value.empty() && hide_when_empty) {
			// no separator
		} else if (value_parts[i].empty() && soft_before_empty) {
			// soft separator
			new_value += _("<sep-soft>") + separators[i - 1] + _("</sep-soft>");
		} else {
			// normal separator
			new_value += _("<sep>")      + separators[i - 1] + _("</sep>");
			new_value += value_parts[i];
		}
	}
	SCRIPT_RETURN(new_value);
}

SCRIPT_FUNCTION_DEPENDENCIES(combined_editor) {
	// read 'field#' arguments
	vector<FieldP> fields;
	for (int i = 0 ; ; ++i) {
		String name = _("field"); if (i > 0) name = name << i;
		SCRIPT_OPTIONAL_PARAM_N(ValueP, name, value) {
			fields.push_back(value->fieldP);
		} else if (i > 0) break;
	}
	// Find the target field
	SCRIPT_PARAM(Set*, set);
	GameP game = set->game;
	FieldP target_field;
	if      (dep.type == DEP_CARD_FIELD) target_field = game->card_fields[dep.index];
	else if (dep.type == DEP_SET_FIELD)  target_field = game->set_fields[dep.index];
	else                                 throw InternalError(_("Finding dependencies of combined error for non card/set field"));
	// Add dependencies, from target_field on field#
	// For card fields
	size_t j = 0;
	FOR_EACH(f, game->card_fields) {
		Dependency dep(DEP_CARD_COPY_DEP, j++);
		FOR_EACH(fn, fields) {
			if (f == fn) {
				target_field->dependent_scripts.add(dep);
				break;
			}
		}
	}
	// For set fields
	j = 0;
	FOR_EACH(f, game->set_fields) {
		Dependency dep(DEP_SET_COPY_DEP, j++);
		FOR_EACH(fn, fields) {
			if (f == fn) {
				target_field->dependent_scripts.add(dep);
				break;
			}
		}
	}
	return dependency_dummy;
}

// ----------------------------------------------------------------------------- : Init

void init_script_editor_functions(Context& ctx) {
	ctx.setVariable(_("forward editor"),  script_combined_editor); // combatability
	ctx.setVariable(_("combined editor"), script_combined_editor);
}
