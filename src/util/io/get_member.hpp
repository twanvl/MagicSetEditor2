//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_IO_GET_MEMBER
#define HEADER_UTIL_IO_GET_MEMBER

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <script/value.hpp>

class Script;
DECLARE_POINTER_TYPE(Script);

template <typename T> class Defaultable;
template <typename T> class Scriptable;

// ----------------------------------------------------------------------------- : GetDefaultMember

/// Find a member without a name using reflection
/** The member is wrapped in a ScriptValue */
class GetDefaultMember {
  public:
	/// Tell the reflection code we are not reading
	inline bool reading()   const { return false; }
	inline bool scripting() const { return true; }
	inline bool isComplex() const { return false; }
	inline void addAlias(int, const Char*, const Char*) {}
	inline void handleAppVersion() {} // no effect
	
	/// The result, or script_nil if the member was not found
	inline ScriptValueP result() { return value; } 
	
	// --------------------------------------------------- : Handling objects
	
	/// Handle an object: we don't match things with a name
	template <typename T>
	void handle(const Char* name, const T& object) {}
	/// Don't handle a value
	template <typename T>
	inline void handleNoScript(const Char* name, T& value) {}

	/// Handle an object: investigate children, or store it if we know how
	                                  void handle(const Char *);
	template <typename T>             void handle(const T&);
	
	/// Handle a Defaultable: investigate children
	template <typename T>             void handle(const Defaultable<T>&);
	template <typename T>             void handle(const Scriptable<T>& );
	template <typename T>             void handle(const vector<T>&     c) { value = to_script(&c); }
	template <typename K, typename V> void handle(const map<K,V>&      c) { value = to_script(&c); }
	template <typename K, typename V> void handle(const IndexMap<K,V>& c) { value = to_script(&c); }
	template <typename K, typename V> void handle(const DelayedIndexMaps<K,V>&) {}
	template <typename K, typename V> void handle(const DelayedIndexMapsData<K,V>& c);
	template <typename T>             void handle(const intrusive_ptr<T>& p) { value = to_script(p); }
	void handle(const ScriptValueP&);
	void handle(const ScriptP&);
  private:
	ScriptValueP  value;		///< The value we found (if any)
};

// ----------------------------------------------------------------------------- : GetMember

/// Find a member with a specific name using reflection
/** The member is wrapped in a ScriptValue */
class GetMember : private GetDefaultMember {
  public:
	/// Construct a member getter that looks for the given name
	GetMember(const String& name);
	
	/// Tell the reflection code we are not reading
	inline bool reading()   const { return false; }
	inline bool scripting() const { return true; }
	inline bool isComplex() const { return false; }
	inline void addAlias(int, const Char*, const Char*) {}
	inline void handleAppVersion() {} // no effect
	
	/// The result, or script_nil if the member was not found
	inline ScriptValueP result() { return gdm.result(); } 
	
	// --------------------------------------------------- : Handling objects
	
	/// Handle an object: we are done if the name matches
	template <typename T>
	void handle(const Char* name, const T& object) {
		if (!gdm.result() && cannocial_name_compare(target_name, name)) {
			gdm.handle(object);
		}
	}
	/// Don't handle a value
	template <typename T>
	inline void handleNoScript(const Char* name, T& value) {}
	/// Handle an object: investigate children
	template <typename T> void handle(const T&);
	/// Handle an index map: invistigate keys
	template <typename K, typename V> void handle(const IndexMap<K,V>& m) {
		if (gdm.result()) return;
		for (typename IndexMap<K,V>::const_iterator it = m.begin() ; it != m.end() ; ++it) {
			if (get_key_name(*it) == target_name) {
				gdm.handle(*it);
				return;
			}
		}
	}
	template <typename K, typename V> void handle(const DelayedIndexMaps<K,V>&);
	template <typename K, typename V> void handle(const DelayedIndexMapsData<K,V>&);
		
  private:
	const String& target_name;	///< The name we are looking for
	GetDefaultMember gdm;		///< Object to store and retrieve the value
};

// ----------------------------------------------------------------------------- : Reflection

/// Implement reflection as used by GetDefaultMember
#define REFLECT_OBJECT_GET_DEFAULT_MEMBER(Cls)		REFLECT_WRITE_YES(Cls,GetDefaultMember)
#define REFLECT_OBJECT_GET_MEMBER(Cls)				REFLECT_WRITE_YES(Cls,GetMember)
#define REFLECT_OBJECT_GET_DEFAULT_MEMBER_NOT(Cls)	REFLECT_WRITE_NO(Cls,GetDefaultMember)
#define REFLECT_OBJECT_GET_MEMBER_NOT(Cls)			REFLECT_WRITE_NO(Cls,GetMember)

#define REFLECT_WRITE_YES(Cls, Tag)										\
	template<> void Tag::handle<Cls>(const Cls& object) {				\
		const_cast<Cls&>(object).reflect(*this);						\
	}																	\
	void Cls::reflect(Tag& tag) {										\
		reflect_impl(tag);												\
	}

#define REFLECT_WRITE_NO(Cls, Tag)										\
	template<> void Tag::handle<Cls>(const Cls& object) {}				\
	void Cls::reflect(Tag& tag) {}

// ----------------------------------------------------------------------------- : Reflection for enumerations

/// Implement enum reflection as used by GetMember
#define REFLECT_ENUM_GET_MEMBER(Enum)									\
	template<> void GetDefaultMember::handle<Enum>(const Enum& enum_) {	\
		EnumGetMember egm(*this);										\
		reflect_ ## Enum(const_cast<Enum&>(enum_), egm);				\
	}

/// 'Tag' to be used when reflecting enumerations for GetMember
class EnumGetMember {
  public:
	inline EnumGetMember(GetDefaultMember& gdm)
		: gdm(gdm) {}
	
	/// Handle a possible value for the enum, if the name matches the name in the input
	template <typename Enum>
	inline void handle(const Char* name, Enum value, Enum enum_) {
		if (enum_ == value) {
			gdm.handle(String(name));
		}
	}
	
  private:
	GetDefaultMember& gdm;  ///< The object to store output in
};

// ----------------------------------------------------------------------------- : EOF
#endif
