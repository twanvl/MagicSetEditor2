//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_SCRIPT_SCRIPTABLE
#define HEADER_SCRIPT_SCRIPTABLE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/reflect.hpp>
#include <util/defaultable.hpp>
#include <script/script.hpp>
#include <script/context.hpp>
#include <script/parser.hpp>

DECLARE_INTRUSIVE_POINTER_TYPE(Script);

// ----------------------------------------------------------------------------- : Store

/// Store a ScriptValue in a variable
void store(const ScriptValueP& val, String& var);
void store(const ScriptValueP& val, int&    var);
void store(const ScriptValueP& val, double& var);
void store(const ScriptValueP& val, bool&   var);
void store(const ScriptValueP& val, Defaultable<String>& var);

// ----------------------------------------------------------------------------- : OptionalScript

/// An optional script, 
class OptionalScript {
  public:
	~OptionalScript();
	/// Is the script set?
	inline operator bool() const { return !!script; }
	
	/// Invoke the script, return the result, or script_nil if there is no script
	ScriptValueP invoke(Context& ctx, bool open_scope = true) const;
	
	/// Invoke the script on a value
	/** Assigns the result to value if it has changed.
	 *  Returns true if the value has changed.
	 */
	template <typename T>
	bool invokeOn(Context& ctx, T& value) const {
		if (script) {
			T new_value;
			ctx.setVariable(_("value"), toScript(value));
			store(ctx.eval(*script), new_value);
			if (value != new_value) {
				value = new_value;
				return true;
			}
		}
		return false;
	}
	/// Invoke the script on a value if it is in the default state
	template <typename T>
	bool invokeOnDefault(Context& ctx, Defaultable<T>& value) const {
		if (value.isDefault() && invokeOn(ctx, value)) {
			value.setDefault(); // restore defaultness
			return true;
		} else {
			return false;
		}
	}
	
	/// Initialize things this script depends on by adding dep to their list of dependent scripts
	void initDependencies(Context&, const Dependency& dep) const;
	
  private:
	ScriptP script;		///< The script, may be null if there is no script
	String unparsed;	///< Unparsed script, for writing back to a file
	// parse the unparsed string, while reading
	void parse(Reader&);
	DECLARE_REFLECTION();
	template <typename T> friend class Scriptable;
};

template <typename T>
inline ScriptValueP toScript(const Defaultable<T>& v) { return toScript(v.get()); }

// ----------------------------------------------------------------------------- : Scriptable

/// A script that defines a calculation to find a value
/** NOTE: reading MUST happen inside a block */
template <typename T>
class Scriptable {
  public:
	Scriptable()               : value()      {}
	Scriptable(const T& value) : value(value) {}
	
	inline operator const T& () const { return value; }
	inline bool isScripted() const { return script; }
	
	/// Updates the value by executing the script, returns true if the value has changed
	inline bool update(Context& ctx) {
		return script.invokeOn(ctx, value);
	}
	
	inline void initDependencies(Context& ctx, const Dependency& dep) const {
		script.initDependencies(ctx, dep);
	}
	
  private:
	T value;				///< The actual value
	OptionalScript script;	///< The optional script
	
	DECLARE_REFLECTION();
};


// we need some custom io, because the behaviour is different for each of Reader/Writer/GetMember

template <typename T>
void Reader::handle(Scriptable<T>& s) {
	handle(s.script.unparsed);
	if (starts_with(s.script.unparsed, _("script:"))) {
		s.script.unparsed = s.script.unparsed.substr(7);
		s.script.parse(*this);
	} else {
		handle(s.value);
	}
}
template <typename T>
void Writer::handle(const Scriptable<T>& s) {
	if (s.script) {
		handle(s.script);
	} else {
		handle(s.value);
	}
}
template <typename T>
void GetDefaultMember::handle(const Scriptable<T>& s) {
	// just handle as the value
	handle(s.value);
}

// ----------------------------------------------------------------------------- : EOF
#endif
