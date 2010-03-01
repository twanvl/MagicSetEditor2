//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <script/scriptable.hpp>
#include <script/context.hpp>
#include <script/parser.hpp>
#include <script/script.hpp>
#include <script/value.hpp>
#include <gfx/color.hpp>

Alignment from_string(const String&);
void parse_enum(const String&,Direction&);

DECLARE_TYPEOF_COLLECTION(ScriptParseError);

// ----------------------------------------------------------------------------- : Store

void store(const ScriptValueP& val, String& var)              { var = val->toString(); }
void store(const ScriptValueP& val, int&    var)              { var = *val; }
void store(const ScriptValueP& val, double& var)              { var = *val; }
void store(const ScriptValueP& val, bool&   var)              { var = *val; }
void store(const ScriptValueP& val, Color&  var)              { var = (AColor)*val; }
void store(const ScriptValueP& val, AColor& var)              { var = *val; }
void store(const ScriptValueP& val, Defaultable<String>& var) { var.assign(*val); }
void store(const ScriptValueP& val, Defaultable<Color>&  var) { var.assign((AColor)*val); }
void store(const ScriptValueP& val, Defaultable<AColor>& var) { var.assign(*val); }
void store(const ScriptValueP& val, Alignment& var)           { var = from_string(val->toString()); }
void store(const ScriptValueP& val, Direction& var)           { parse_enum(val->toString(),var); }

// ----------------------------------------------------------------------------- : OptionalScript

OptionalScript::OptionalScript(const String& script_)
	: script(::parse(script_))
	, unparsed(script_)
{}

OptionalScript::~OptionalScript() {}

ScriptValueP OptionalScript::invoke(Context& ctx, bool open_scope) const {
	if (script) {
		return ctx.eval(*script, open_scope);
	} else {
		return script_nil;
	}
}

void OptionalScript::parse(Reader& reader, bool string_mode) {
	vector<ScriptParseError> errors;
	script = ::parse(unparsed, reader.getPackage(), string_mode, errors);
	// show parse errors as warnings
	String include_warnings;
	for (size_t i = 0 ; i < errors.size() ; ++i) {
		const ScriptParseError& e = errors[i];
		if (!e.filename.empty()) {
			// error in an include file
			include_warnings += String::Format(_("\n  On line %d:\t  "), e.line) + e.ParseError::what();
			if (i + 1 >= errors.size() || errors[i+1].filename != e.filename) {
				reader.warning(_("In include file '") + e.filename + _("'") + include_warnings);
				include_warnings.clear();
			}
		} else {
			// use ParseError::what because we don't want e.start in the error message
			reader.warning(e.ParseError::what(), e.line - 1);
		}
	}
}

void OptionalScript::initDependencies(Context& ctx, const Dependency& dep) const {
	if (script) {
		ctx.dependencies(dep, *script);
	}
}

Script& OptionalScript::getMutableScript() {
	if (!script) script = new_intrusive<Script>();
	return *script;
}

// custom reflection, different for each type

template <> void Reader::handle(OptionalScript& os) {
	handle(os.unparsed);
	os.parse(*this);
}

template <> void Writer::handle(const OptionalScript& os) {
	if (os.script) {
		handle(os.unparsed);
	}
}

template <> void GetDefaultMember::handle(const OptionalScript& os) {
	// reflect as the script itself
	if (os.script) {
		handle(os.script);
	} else {
		handle(script_nil);
	}
}

// ----------------------------------------------------------------------------- : StringScript

const String& StringScript::get() const {
	return unparsed;
}

void StringScript::set(const String& s) {
	unparsed = s;
	script = ::parse(unparsed, nullptr, true);
}

template <> void Reader::handle(StringScript& os) {
	handle(os.unparsed);
	os.parse(*this, true);
}

// same as OptionalScript

template <> void Writer::handle(const StringScript& os) {
	handle(os.unparsed);
}

template <> void GetDefaultMember::handle(const StringScript& os) {
	// reflect as the script itself
	if (os.script) {
		handle(os.script);
	} else {
		handle(script_nil);
	}
}
