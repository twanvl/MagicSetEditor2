//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_IO_GET_MEMBER
#define HEADER_UTIL_IO_GET_MEMBER

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

class ScriptValue;
typedef boost::intrusive_ptr<ScriptValue> ScriptValueP;
inline void intrusive_ptr_add_ref(ScriptValue* p);
inline void intrusive_ptr_release(ScriptValue* p);

// ----------------------------------------------------------------------------- : GetMember

/// Find a member with a specific name using reflection
/** The member is wrapped in a ScriptValue */
class GetMember {
  public:
	/// Construct a member getter that looks for the given name
	GetMember(const String& name);
	
	/// Tell the reflection code we are not reading
	inline bool reading() const { return false; }
	
	/// The result, or scriptNil if the member was not found
	inline ScriptValueP result() { return value; } 
	
	// --------------------------------------------------- : Handling objects
	
	/// Handle an object: we are done if the name matches
	template <typename T>
	void handle(const Char* name, const T& object) {
		if (!value && name == targetName) store(object);
	}
	template <typename T>
	void handle(const T&);
	
	/// Store something in the return value
	void store(const String&       v);
	void store(const int           v);
	void store(const unsigned int  v);
	void store(const double        v);
	void store(const bool          v);
	/// Store a vector in the return value
	template <typename T> void store(const vector<T>& vector) {
		value = toScript(&vector);
	}
	/// Store a shared_ptr in the return value
	template <typename T> void store(const shared_ptr<T>& pointer) {
		value = toScript(pointer);
	}
	
  private:
	const String& targetName;	///< The name we are looking for
	ScriptValueP  value;		///< The value we found (if any)
};

// ----------------------------------------------------------------------------- : Reflection

/// Implement reflection as used by GetMember
#define REFLECT_OBJECT_GET_MEMBER(Cls)							\
	template<> void GetMember::handle<Cls>(const Cls& object) {	\
		const_cast<Cls&>(object).reflect(*this);				\
	}

// ----------------------------------------------------------------------------- : Reflection for enumerations

/// Implement enum reflection as used by Writer
#define REFLECT_ENUM_WRITER(Enum)								\
	template<> void Writer::handle<Enum>(const Enum& enum_) {	\
		EnumGetMember gm(*this);								\
		reflect_ ## Enum(const_cast<Enum&>(enum_), gm);			\
	}

/// 'Tag' to be used when reflecting enumerations for GetMember
class EnumGetMember {
  public:
	inline EnumGetMember(GetMember& getMember)
		: getMember(getMember) {}
	
	/// Handle a possible value for the enum, if the name matches the name in the input
	template <typename Enum>
	inline void handle(const Char* name, Enum value, Enum enum_) {
		if (enum_ == value) {
			writer.store(name);
		}
	}
	
  private:
	GetMember& getMember;  ///< The writer to write output to
};

// ----------------------------------------------------------------------------- : EOF
#endif
