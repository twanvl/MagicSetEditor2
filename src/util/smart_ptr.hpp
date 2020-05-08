//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

/** @file util/smart_ptr.hpp
 *
 *  @brief Smart pointers and related utility functions
 */

// ----------------------------------------------------------------------------- : Includes

#include <memory>

using std::shared_ptr;
using std::unique_ptr;
using std::static_pointer_cast;
using std::dynamic_pointer_cast;
using std::make_shared;
using std::make_unique;

#ifndef USE_INTRUSIVE_PTR
  #define USE_INTRUSIVE_PTR 1
#endif

// ----------------------------------------------------------------------------- : Intrusive pointers

#if USE_INTRUSIVE_PTR

#include <atomic>
#include <boost/smart_ptr/intrusive_ptr.hpp>
using boost::intrusive_ptr;

/// Base class for types that can be pointed to
template <typename T> class IntrusivePtrBase {
public:
  IntrusivePtrBase() {}
  // don't copy or assign ref count
  IntrusivePtrBase(IntrusivePtrBase const&) {}
  void operator = (IntrusivePtrBase const&) {}
protected:
  inline void destroy() const {
    delete static_cast<const T*>(this);
  }
private:
  mutable std::atomic<unsigned int> ref_count = 0;
  template <typename U> friend void intrusive_ptr_add_ref(const IntrusivePtrBase<U>* ptr);
  template <typename U> friend void intrusive_ptr_release(const IntrusivePtrBase<U>* ptr);
};

template <typename T> void intrusive_ptr_add_ref(const IntrusivePtrBase<T>* ptr) {
  ++(ptr->ref_count);
}

template <typename T> void intrusive_ptr_release(const IntrusivePtrBase<T>* ptr) {
  if (--(ptr->ref_count) == 0) {
    static_cast<const T*>(ptr)->destroy();
  }
}

template <typename T>
class IntrusiveFromThis {
public:
  inline intrusive_ptr<T> intrusive_from_this() noexcept {
    return intrusive_ptr<T>(static_cast<T*>(this));
  }
};

/// Allocate an object of type T and store it in a new intrusive_ptr, similar to std::make_shared
template <typename T, class... Args>
inline intrusive_ptr<T> make_intrusive(Args&&... args) {
  return intrusive_ptr<T>(new T(std::forward<Args>(args)...));
}

// ----------------------------------------------------------------------------- : Shared pointers
#else

template <typename T> using intrusive_ptr = shared_ptr<T>;

/// Base class for types that can be pointed to
template <typename T> class IntrusivePtrBase {};

template <typename T>
class IntrusiveFromThis : public std::enable_shared_from_this<T> {
public:
  inline intrusive_ptr<T> intrusive_from_this() {
    return this->shared_from_this();
  }
};

/// Allocate an object of type T and store it in a new intrusive_ptr, similar to std::make_shared
template <typename T, class... Args>
inline intrusive_ptr<T> make_intrusive(Args&&... args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

#endif

// ----------------------------------------------------------------------------- : Declaring

/// Declares the type TypeP as a intrusive_ptr<Type>
#define DECLARE_POINTER_TYPE(Type) \
  class Type; \
  typedef intrusive_ptr<Type> Type##P;

/// Declares the type TypeP as a shared_ptr<Type>
#define DECLARE_SHARED_POINTER_TYPE(Type) \
  class Type; \
  typedef shared_ptr<Type> Type##P;

// ----------------------------------------------------------------------------- : Utility

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
  virtual void destroy() const {
    delete this;
  }
  template <typename T> friend void intrusive_ptr_release(const IntrusivePtrBase<T>* ptr);
};

/// Pointer to 'anything'
typedef intrusive_ptr<IntrusivePtrVirtualBase> VoidP;


