//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
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
	inline Defaultable()           :           is_default(true)  {}
	inline Defaultable(const T& v) : value(v), is_default(false) {}
	
	/// Assigning a value takes this object out of the default state
	inline void assign(const T& new_value) {
		value      = new_value;
		is_default = false;
	}
	
	/// Get access to the value
	inline const T& operator () () const { return value; }
	
	/// Is this value in the default state?
	inline bool isDefault() const { return is_default; }
	
  private:
	/// Is this value in the default state?
	bool is_default;
	/// The value
	T value;
	
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
