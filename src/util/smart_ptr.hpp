//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2017 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_SMART_PTR
#define HEADER_UTIL_SMART_PTR

/** @file util/smart_ptr.hpp
 *
 *  @brief Utilities related to boost smart pointers
 */

// ----------------------------------------------------------------------------- : Includes

#include <memory>

using std::shared_ptr;
using std::unique_ptr;
using std::static_pointer_cast;
using std::dynamic_pointer_cast;
using std::make_shared;
using std::make_unique;

// ----------------------------------------------------------------------------- : Declaring

/// Declares the type TypeP as a shared_ptr<Type>
#define DECLARE_SHARED_POINTER_TYPE(Type) \
  class Type; \
  typedef shared_ptr<Type> Type##P;

// ----------------------------------------------------------------------------- : Intrusive pointers

#define DECLARE_POINTER_TYPE DECLARE_SHARED_POINTER_TYPE

template <typename T> class IntrusivePtrBase {};
template <typename T> using intrusive_ptr = shared_ptr<T>;

/// Allocate an object of type T and store it in a new intrusive_ptr, similar to std::make_shared
template <typename T, class... Args>
inline intrusive_ptr<T> make_intrusive(Args&&... args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

/// IntrusivePtrBase with a virtual destructor
class IntrusivePtrVirtualBase : public IntrusivePtrBase<IntrusivePtrVirtualBase> {
  public:
  virtual ~IntrusivePtrVirtualBase() {}
};

class IntrusivePtrBaseWithDelete : public IntrusivePtrBase<IntrusivePtrBaseWithDelete> {
  public:
  virtual ~IntrusivePtrBaseWithDelete() {}
  protected:
  /// Delete this object
  virtual void destroy() {
    delete this;
  }
};

/// Pointer to 'anything'
typedef intrusive_ptr<IntrusivePtrVirtualBase> VoidP;

// ----------------------------------------------------------------------------- : EOF
#endif
