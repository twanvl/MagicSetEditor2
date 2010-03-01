//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_DEFAULTABLE
#define HEADER_UTIL_DEFAULTABLE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/reflect.hpp>

// ----------------------------------------------------------------------------- : Defaultable

/// A value that can also be in a 'default' state.
/** TODO: Defaultable is automatically also Aged
 */
template <typename T>
class Defaultable {
  public:
	inline Defaultable()                             :           is_default(true)  {}
	inline Defaultable(const T& v, bool def = false) : value(v), is_default(def) {}
	
	/// Assigning a value takes this object out of the default state
	inline void assign(const T& new_value) {
		value      = new_value;
		is_default = false;
	}
	/// Assigning a value keep this object in the default state
	inline void assignDefault(const T& new_value) {
		assert(is_default);
		value      = new_value;
	}
	/// Assigning a value, don't change the defaultness
	inline void assignDontChangeDefault(const T& new_value) {
		value = new_value;
	}
	
	/// Get access to the value
	inline operator const T& () const { return value; }
	inline const T& operator () () const { return value; }
	/// Get access to the value, for changing it
	inline       T& mutate      ()       {
		is_default = false;
		return value;
	}
	/// Get access to the value, for changing it, don't change the defaultness
	inline T& mutateDontChangeDefault() { return value; }
	
	/// Is this value in the default state?
	inline bool isDefault() const { return is_default; }
	/// Set the defaultness to d
	inline void makeDefault(bool d = true) { is_default = d; }
	
	/// Compare the values, ignore defaultness
	/** used by scriptable to check for changes */
	inline bool operator != (const Defaultable& that) const { return value != that.value; }
	
  private:
	/// The value
	T value;
	/// Is this value in the default state?
	bool is_default;
	
	friend class Reader;
	friend class Writer;
};

// we need some custom io, because the behaviour is different for each of Reader/Writer/GetMember

template <typename T>
void Reader::handle(Defaultable<T>& def) {
	def.is_default = false;
	handle(def.value);
}
template <typename T>
void Writer::handle(const Defaultable<T>& def) {
	if (!def.isDefault()) {
		handle(def());
	}
}
template <typename T>
void GetDefaultMember::handle(const Defaultable<T>& def) {
	handle(def());
}

// ----------------------------------------------------------------------------- : EOF
#endif
