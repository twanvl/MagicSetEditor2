//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <script/context.hpp>
#include <script/to_value.hpp>
#include <util/error.hpp>
#include <queue>

DECLARE_TYPEOF_COLLECTION(ScriptValueP);
DECLARE_TYPEOF_COLLECTION(Context::Binding);

// NOTE: dependency.cpp has nothing to do with dependency.hpp, the latter defines the dependency
// type, which is used here as an abstract type. The header for this source file is context.hpp

// ----------------------------------------------------------------------------- : Dummy values

// A dummy type used during dependency analysis,
// it simply supresses all error messages.
class DependencyDummy : public ScriptIterator {
  public:
	virtual ScriptType type() const { return SCRIPT_DUMMY; }
	virtual String typeName() const { return _("dummy"); }
	virtual ScriptValueP next() { return ScriptValueP(); }
};

ScriptValueP dependency_dummy(new DependencyDummy);

ScriptValueP unified(const ScriptValueP& a, const ScriptValueP& b);

// A script value that is a 'union' of two values.
/* During actual execution the value could be either a *or* b,
 * So it has the dependency characteristics of both.
 */
class DependencyUnion : public ScriptValue {
  public:
	DependencyUnion(const ScriptValueP& a, const ScriptValueP& b)
		: a(a), b(b)
	{}
	
	virtual ScriptType type() const { return SCRIPT_DUMMY; }
	virtual String typeName() const { return _("union of ") + a->typeName() + _(" and ") + b->typeName(); }
	
	virtual ScriptValueP dependencies(Context& ctx, const Dependency& dep) const {
		return unified( a->dependencies(ctx,dep), b->dependencies(ctx,dep));
	}
	virtual ScriptValueP makeIterator() const {
		return unified(a->makeIterator(), b->makeIterator());
	}
	virtual ScriptValueP dependencyMember(const String& name, const Dependency& dep) const {
		return unified(a->dependencyMember(name,dep), b->dependencyMember(name,dep));
	}
  private:
	ScriptValueP a, b;
};

// Unify two values from different execution paths
void unify(ScriptValueP& a, const ScriptValueP& b) {
	if (a != b) a = new_intrusive2<DependencyUnion>(a,b);
}
// Unify two values from different execution paths
ScriptValueP unified(const ScriptValueP& a, const ScriptValueP& b) {
	if (a == b) return a;
	else        return new_intrusive2<DependencyUnion>(a,b);
}

/// Behaves like script_nil, but with a name
class ScriptMissingVariable : public ScriptValue {
  public:
	ScriptMissingVariable(const String& name) : name(name) {}
	virtual ScriptType type() const { return SCRIPT_NIL; }
	virtual String typeName() const { return _("missing variable '") + name + _("'"); }
	virtual operator String() const { return wxEmptyString; }
	virtual operator double() const { return 0.0; }
	virtual operator int()    const { return 0; }
	virtual ScriptValueP eval(Context&) const { return script_nil; } // nil() == nil
  private:
	String name; ///< Name of the variable
};

// ----------------------------------------------------------------------------- : Jump record

// Utility class: a jump that has been postponed
struct Context::Jump {
	const Instruction*   target;		///< Target of the jump
	vector<ScriptValueP> stack_top;		///< The top part of the stack, everything local to the current call
	vector<Binding>      bindings;		///< The bindings made up to this point in the current scope
};
// an ordering on jumps by their target, lowest target = highest priority
struct Context::JumpOrder {
	inline bool operator () (Jump* a, Jump* b) {
		return a->target > b->target;
	}
};

// ----------------------------------------------------------------------------- : Dependency analysis

ScriptValueP Context::dependencies(const Dependency& dep, const Script& script) {
	// Dependency analysis proceeds in the same way as normal evaluation.
	// Operator calls will be replaced by "push dummy", we don't care about values.
	// Only the operators left are:
	//   - member operator; and it signals a dependency.
	//   - looper construction
	//   - + for function composition
	// Variable assignments are performed as normall.
	// Jumps are tricky:
	//   - I_LOOP:        We want to prevent infinite loops, the solution is that after the first
	//                    iteration we set the looper to a dummy value, so the loop is only executed once.
	//                    TODO: This could result in false negatives when iterating over things like fields.
	//                          We ignore this, because loops are usually only used for exporting, where dependency
	//                          analysis is not used anyway.
	//   - I_JUMP_IF_NOT: We don't know the value of the condition, so we must evaluate both branches.
	//                    The simple solution would be to use recursion to fork off one of the cases.
	//                    This could result in an exponential increase in execution time,
	//                    because the analysis after an if statement is duplicated.
	//                    A better solution is to evalutate branches 'in parallel'.
	//                    We create a jump record for taking the branch, and evaluate the fall through case.
	//                    When later a jump record points to the current instruction the stack and variables of that
	//                    record are unify with the current execution path.
	//   - I_JUMP:        We must can not follow all jumps, because they may lead to a point beyond a jump record,
	//                    we can then no longer hope to unify with that jump record.
	//                    Instead we create a new jump record, and follow the jump record with the lowest target address.
	//                    This story doesn't hold for backwards jumps, we can safely follow those (see I_LOOP above)
	
	// Scope for evaluating this script.
	size_t stack_size = stack.size();
	size_t scope      = openScope();
	
	// Forward jumps waiting to be performed, by order of target (descending)
	priority_queue<Jump*,vector<Jump*>,JumpOrder> jumps;
	
	try {
		// Instruction pointer
		const Instruction* instr = &script.instructions[0];
		
		// Loop until we are done
		while (true) {
			assert(instr < &*script.instructions.end());
			// Is there a jump going here?
			// If so, unify with current execution path
			while (!jumps.empty() && jumps.top()->target == instr) {
				// unify with current execution path
				Jump* j = jumps.top(); jumps.pop();
				// unify stack
				assert(stack_size + j->stack_top.size()  ==  stack.size());
				for (size_t i = 0; i < j->stack_top.size() ; ++i) {
					unify(stack[stack_size + i], j->stack_top[i]);
				}
				// unify bindings
				FOR_EACH(v, j->bindings) {
					unify(variables[v.variable].value, v.value.value);
				}
				delete j;
			}
			
			// Analyze the current instruction
			Instruction i = *instr++;
			switch (i.instr) {
				case I_NOP: break;
				// Push a constant (as normal)
				case I_PUSH_CONST: {
					stack.push_back(script.constants[i.data]);
					break;
				}
				// Pop top value (as normal)
				case I_POP: {
					stack.pop_back();
					break;
				}
				// Jump
				case I_JUMP: {
					if (&script.instructions[i.data] >= instr) {
						// forward jump
						// create jump record
						Jump* jump = new Jump;
						jump->target = &script.instructions[i.data];
						jump->stack_top.assign(stack.begin() + stack_size, stack.end());
						getBindings(scope, jump->bindings);
						jumps.push(jump);
						// clear scope
						stack.resize(stack_size);
						resetBindings(scope);
						// we don't follow this jump just yet, there may be jumps that point to earlier positions
						Jump* jumpTo = jumps.top(); jumps.pop();
						instr = jumpTo->target;
						FOR_EACH(s, jumpTo->stack_top) stack.push_back(s);
						FOR_EACH(b, jumpTo->bindings)  setVariable(b.variable, b.value.value);
						delete jumpTo;
					} else {
						// backward jump: just follow it, someone else (I_LOOP) will make sure
						// we don't go into an infinite loop
						instr = &script.instructions[i.data];
					}
					break;
				}
				// Conditional jump
				case I_JUMP_IF_NOT: {
					stack.pop_back(); // condition
					// create jump record
					Jump* jump = new Jump;
					jump->target = &script.instructions[i.data];
					assert(jump->target >= instr); // jumps must be forward
					jump->stack_top.assign(stack.begin() + stack_size, stack.end());
					getBindings(scope, jump->bindings);
					jumps.push(jump);
					// just fall through for the case that the condition holds
					break;
				}
				
				// Get an object member (almost as normal)
				case I_MEMBER_C: {
					String name = *script.constants[i.data];
					stack.back() = stack.back()->dependencyMember(name, dep); // dependency on member
					break;
				}
				// Loop over a container, push next value or jump (almost as normal)
				case I_LOOP: {
					ScriptValueP& it = stack[stack.size() - 2]; // second element of stack
					assert(dynamic_pointer_cast<ScriptIterator>(it)); // top of stack must be an iterator
					ScriptValueP val = static_pointer_cast<ScriptIterator>(it)->next();
					if (val) {
						it = dependency_dummy; // invalidate iterator, so we loop only once
						stack.push_back(val);
					} else {
						stack.erase(stack.end() - 2); // remove iterator
						instr = &script.instructions[i.data];
					}
					break;
				}
				
				// Function call (as normal)
				case I_CALL: {
					// new scope
					size_t scope = openScope();
					// prepare arguments
					for (unsigned int j = 0 ; j < i.data ; ++j) {
						setVariable(instr[i.data - j - 1].data, stack.back());
						stack.pop_back();
					}
					instr += i.data; // skip arguments, there had better not be any jumps into the argument list
					// get function and call
					stack.back() = stack.back()->dependencies(*this, dep);
					// restore scope
					closeScope(scope);
					break;
				}
				// Function return (as normal)
				case I_RET: {
					closeScope(scope);
					// return top of stack
					ScriptValueP result = stack.back();
					stack.pop_back();
					assert(stack.size() == stack_size); // we end up with the same stack
					assert(jumps.empty());              // no open jump records
					return result;
				}
				
				// Get a variable (almost as normal)
				case I_GET_VAR: {
					ScriptValueP value = variables[i.data].value;
					if (!value) {
						value = new_intrusive1<ScriptMissingVariable>(variable_to_string(i.data)); // no errors here
					}
					stack.push_back(value);
					break;
				}
				// Set a variable (as normal)
				case I_SET_VAR: {
					setVariable(i.data, stack.back());
					break;
				}
				
				// Simple instruction: unary
				case I_UNARY: {
					ScriptValueP& a = stack.back();
					switch (i.instr1) {
						case I_ITERATOR_C:
							a = a->makeIterator(); // as normal
							break;
						default:
							a = dependency_dummy;
					}
					break;
				}
				// Simple instruction: binary
				case I_BINARY: {
					ScriptValueP  b = stack.back(); stack.pop_back();
					ScriptValueP& a = stack.back();
					switch (i.instr2) {
						case I_ITERATOR_R:
							a = rangeIterator(0,0); // values don't matter
							break;
						case I_MEMBER: {
							String name = *b;
							a = a->dependencyMember(name, dep); // dependency on member
							break;
						} case I_ADD:
							unify(a, b); // may be function composition
							break;
						default:
							a = dependency_dummy;
					}
					break;
				}
				// Simple instruction: ternary
				case I_TERNARY: {
					ScriptValueP  c = stack.back(); stack.pop_back();
					ScriptValueP  b = stack.back(); stack.pop_back();
					ScriptValueP& a = stack.back();
					a = dependency_dummy;
					break;
				}
			}
		}
	} catch (...) {
		// cleanup after an exception
		// the only place where exceptions should be possible is in someValue->getMember
		if (scope) closeScope(scope); // restore scope
		stack.resize(stack_size);     // restore stack
		// delete jump records
		while (!jumps.empty()) {
			delete jumps.top();
			jumps.pop();
		}
		throw; // rethrow
	}
}

void Context::getBindings(size_t scope, vector<Binding>& bindings) {
	for (size_t i = scope + 1 ; i < shadowed.size() ; ++i) {
		Binding b = {shadowed[i].variable, variables[shadowed[i].variable]};
		bindings.push_back(b);
	}
}

void Context::resetBindings(size_t scope) {
	// same as closeScope()
	while (shadowed.size() > scope) {
		variables[shadowed.back().variable] = shadowed.back().value;
		shadowed.pop_back();
	}
}
