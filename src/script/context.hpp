//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2009 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_SCRIPT_CONTEXT
#define HEADER_SCRIPT_CONTEXT

// ----------------------------------------------------------------------------- : Includes

#include <script/script.hpp>

class Dependency;

// ----------------------------------------------------------------------------- : VectorIntMap

/// A map like data structure that stores the elements in a vector.
/** K should be an integer type, the keys should be dense. */
template <typename K, typename V>
class VectorIntMap {
  public:
	inline V& operator [] (K key) {
		if (values.size() <= key) {
			values.resize(key + 1);
		}
		return values[key];
	}
	/// Get access to the vector
	inline const vector<V>& get() const { return values; }
  private:
	vector<V> values;
};

// ----------------------------------------------------------------------------- : Context

/// Context for script evaluation
class Context {
  public:
	Context();
	
	/// Evaluate a script inside this context.
	/** This function is safely reentrant.
	 *  @param openScope if false, variables set in this eval call will leak out.
	 */
	ScriptValueP eval(const Script& script, bool openScope = true);
	
	/// Analyze the dependencies of a script
	/** All things the script depends on are marked with signalDependent(dep).
	 *  The return value of this function should be ignored
	 */
	ScriptValueP dependencies(const Dependency& dep, const Script& script);
		
	/// Set a variable to a new value (in the current scope)
	void setVariable(const String& name, const ScriptValueP& value);
	/// Set a variable to a new value (in the current scope)
	void setVariable(Variable name, const ScriptValueP& value);
	
	/// Get the value of a variable, throws if it not set
	ScriptValueP getVariable(const String& name);
	/// Get the value of a variable, returns ScriptValue() if it is not set
	ScriptValueP getVariableOpt(const String& name);
	/// Get the value of a variable, throws if it not set
	ScriptValueP getVariable(Variable var);
	/// Get the value of a variable, returns ScriptValue() if it is not set
	inline ScriptValueP getVariableOpt(Variable var) { return variables[var].value; }
	/// Get the value of a variable only if it was set in the current scope, returns ScriptValue() if it is not set
	ScriptValueP getVariableInScopeOpt(Variable var);
	/// In what scope was the variable set?
	/** Returns 0 for the current scope and >0 for outer scopes.
	 *  Returns -1 if the varible is not set
	 */
	int getVariableScope(Variable var);
	
	/// Make a closure of the function with the direct parameters of the current call
	ScriptValueP makeClosure(const ScriptValueP& fun);
	
  public:
	
	/// Open a new scope
	/** returns the number of shadowed binding before that scope */
	size_t openScope();
	/// Close a scope, must be passed a value from openScope
	void closeScope(size_t scope);
	friend class LocalScope;
	
  public:// public for FOR_EACH
	/// Record of a variable
	struct VariableValue {
		VariableValue() : level(0) {}
		unsigned int level; ///< Scope level on which this variable was set
		ScriptValueP value; ///< Value of this variable
	};
	/// Record of a variable binding that is being shadowed (overwritten) by another binding
	struct Binding {
		Variable      variable; ///< Name of the overwritten variable.
		VariableValue value;    ///< Old value of that variable.
	};
  private:
	/// Variables, indexed by integer name (using string_to_variable)
	VectorIntMap<unsigned int, VariableValue> variables;
	/// Shadowed variable bindings
	vector<Binding> shadowed;
	/// Number of scopes opened
	unsigned int level;
	/// Stack of values
	vector<ScriptValueP> stack;
	#ifdef _DEBUG
		/// The opened scopes, for sanity checking
		vector<size_t> scopes;
	#endif
	
	// utility types for dependency analysis
	struct Jump;
	struct JumpOrder;
	
	/// Return the bindings in the current scope
	void getBindings(size_t scope, vector<Binding>&);
	/// Remove all bindings made in the current scope
	void resetBindings(size_t scope);
	/// Make an object with n elements, popping 2n values from the stack, and push it onto the stack
	void makeObject(size_t n);
	/// Make a closure with n arguments
	void makeClosure(size_t n, const Instruction*& instr);
	
	/// Get a variable name givin its value, returns (Variable)-1 if not found (slow!)
	Variable lookupVariableValue(const ScriptValueP& value);
	friend class ScriptCompose;
};

/// A class that creates a local scope
class LocalScope {
  public:
	inline LocalScope(Context& ctx) : ctx(ctx), scope(ctx.openScope()) {}
	inline ~LocalScope() { ctx.closeScope(scope); }
  private:
	Context& ctx;
	size_t scope;
};

// ----------------------------------------------------------------------------- : EOF
#endif
