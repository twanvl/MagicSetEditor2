//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <script/scriptable.hpp>
#include <script/context.hpp>
#include <script/parser.hpp>
#include <script/script.hpp>
#include <script/value.hpp>

// ----------------------------------------------------------------------------- : Store

void store(const ScriptValueP& val, String& var)              { var = static_cast<String>(*val); }
void store(const ScriptValueP& val, int&    var)              { var = *val; }
void store(const ScriptValueP& val, double& var)              { var = *val; }
void store(const ScriptValueP& val, bool&   var)              { var = static_cast<int>(*val); }
void store(const ScriptValueP& val, Color&  var)              { var = *val; }
void store(const ScriptValueP& val, Defaultable<String>& var) { var.assign(*val); }

// ----------------------------------------------------------------------------- : OptionalScript

OptionalScript::OptionalScript(const String& script_)
	: unparsed(script_)
	, script(::parse(script_))
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
	try {
		script = ::parse(unparsed, string_mode);
	} catch (const ParseError& e) {
		reader.warning(e.what());
	}
}

void OptionalScript::initDependencies(Context& ctx, const Dependency& dep) const {
	if (script) {
		ctx.dependencies(dep, *script);
	}
}


// custom reflection, different for each type

template <> void Reader::handle(OptionalScript& os) {
	handle(os.unparsed);
	os.parse(*this);
}

template <> void Writer::handle(const OptionalScript& os) {
	handle(os.unparsed);
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
	script = ::parse(unparsed, true);
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
