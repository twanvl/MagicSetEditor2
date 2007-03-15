//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_FOR_EACH
#define HEADER_UTIL_FOR_EACH

/** @file util/for_each.hpp
 *
 *  @brief Macros to simplify looping over collections.
 * 
 *  This header contains some evil template and macro hackery.
 */

// ----------------------------------------------------------------------------- : Includes

// ----------------------------------------------------------------------------- : Typeof magic

#ifdef __GNUC__
	// GCC has a buildin typeof function, so it doesn't need (as much) hacks
	#define DECLARE_TYPEOF(T)
	#define DECLARE_TYPEOF_NO_REV(T)
	#define DECLARE_TYPEOF_CONST(T)
	#define DECLARE_TYPEOF_COLLECTION(T)
	
	#define TYPEOF(Value)      __typeof(Value)
	#define TYPEOF_IT(Value)   __typeof(Value.begin())
	#define TYPEOF_CIT(Value)  __typeof(Value.begin())
	#define TYPEOF_RIT(Value)  __typeof(Value.rbegin())
	#define TYPEOF_CRIT(Value) __typeof(Value.rbegin())
	#define TYPEOF_REF(Value)  __typeof(*Value.begin())&
	#define TYPEOF_CREF(Value) __typeof(*Value.begin())&
	
#else
	/// Helper for typeof tricks
	template<const type_info &ref_type_info> struct TypeOf {};

	/// The type of a value
	#define TYPEOF(Value)      TypeOf<typeid(Value)>::type
	/// The type of an iterator
	#define TYPEOF_IT(Value)   TypeOf<typeid(Value)>::iterator
	/// The type of a const iterator
	#define TYPEOF_CIT(Value)  TypeOf<typeid(Value)>::const_iterator
	/// The type of a reverse iterator
	#define TYPEOF_RIT(Value)  TypeOf<typeid(Value)>::reverse_iterator
	/// The type of a const reverse iterator
	#define TYPEOF_CRIT(Value) TypeOf<typeid(Value)>::const_reverse_iterator
	/// The type of a reference
	#define TYPEOF_REF(Value)  TypeOf<typeid(Value)>::reference
	/// The type of a const reference
	#define TYPEOF_CREF(Value) TypeOf<typeid(Value)>::const_reference

	/// Declare typeof magic for a specific type
	#define DECLARE_TYPEOF(T)											\
		template<> struct TypeOf<typeid(T)> {							\
			typedef T							type;					\
			typedef T::iterator					iterator;				\
			typedef T::const_iterator			const_iterator;			\
			typedef T::reverse_iterator			reverse_iterator;		\
			typedef T::const_reverse_iterator	const_reverse_iterator;	\
			typedef T::reference				reference;				\
			typedef T::const_reference			const_reference;		\
		}
	/// Declare typeof magic for a specific type that doesn't support reverse iterators
	#define DECLARE_TYPEOF_NO_REV(T)									\
		template<> struct TypeOf<typeid(T)> {							\
			typedef T							type;					\
			typedef T::iterator					iterator;				\
			typedef T::const_iterator			const_iterator;			\
			typedef T::reference				reference;				\
			typedef T::const_reference			const_reference;		\
		}
	/// Declare typeof magic for a specific type, using const iterators
	#define DECLARE_TYPEOF_CONST(T)										\
		template<> struct TypeOf<typeid(T)> {							\
			typedef T							type;					\
			typedef T::const_iterator			iterator;				\
			typedef T::const_iterator			const_iterator;			\
			typedef T::const_reverse_iterator	reverse_iterator;		\
			typedef T::const_reverse_iterator	const_reverse_iterator;	\
			typedef T::const_reference			reference;				\
			typedef T::const_reference			const_reference;		\
		}
	
	/// Declare typeof magic for a specific std::vector type
	#define DECLARE_TYPEOF_COLLECTION(T)  DECLARE_TYPEOF(vector<T>); \
// 	                                      DECLARE_TYPEOF_CONST(set<T>)
	
#endif

	
// ----------------------------------------------------------------------------- : Looping macros with iterators

/// Iterate over a collection, using an iterator it of type Type
/** Usage: FOR_EACH_IT_T(Type,it,collect) { body-of-loop }
 */
#define FOR_EACH_IT_T(Type,Iterator,Collection)						\
		for(Type Iterator = Collection.begin() ;					\
		    Iterator != Collection.end() ;							\
		    ++Iterator)

/// Iterate over a collection whos type must be declared with DECLARE_TYPEOF
/** Usage: FOR_EACH_IT(it,collect) { body-of-loop }
 */
#define FOR_EACH_IT(Iterator,Collection)							\
		FOR_EACH_IT_T(TYPEOF_IT(Collection), Iterator, Collection)

/// Iterate over a collection whos type must be declared with DECLARE_TYPEOF
/** Uses a const_iterator
 *  Usage: FOR_EACH_IT(it,collect) { body-of-loop }
 */
#define FOR_EACH_CONST_IT(Iterator,Collection)						\
		FOR_EACH_IT_T(TYPEOF_CIT(Collection), Iterator, Collection)

/// Iterate over a collection in whos type must be declared with DECLARE_TYPEOF
/** Iterates using a reverse_iterator
 *  Usage: FOR_EACH_REVERSE_IT(it,collect) { body-of-loop }
 */
#define FOR_EACH_REVERSE_IT(Iterator,Collection)					\
		for(TYPEOF_RIT(Collection)									\
		                        Iterator = Collection.rbegin() ;	\
		    Iterator != Collection.rend() ;							\
		    ++Iterator)

// ----------------------------------------------------------------------------- : Looping macros

/// Iterate over a collection, with an iterator of type TypeIt, and elements of type TypeElem
/** Usage: FOR_EACH_T(TypeIt,TypeElem,e,collect,begin,end) { body-of-loop }
 *
 *  We need a hack to be able to declare a local variable without needing braces.
 *  To do this we use a nested for loop that is only executed once, and which is optimized away.
 *  To terminate this loop we need an extra bool, which we set to false after the first iteration.
 */
#define FOR_EACH_T(TypeIt,TypeElem,Elem,Collection, begin, end)				\
		for(std::pair<TypeIt,bool> Elem##_IT(Collection.begin(), true) ;	\
		    Elem##_IT.second && Elem##_IT.first != Collection.end() ;		\
		    ++Elem##_IT.first, Elem##_IT.second = !Elem##_IT.second)		\
		    for(TypeElem Elem = *Elem##_IT.first ;							\
				Elem##_IT.second ;											\
				Elem##_IT.second = false)

/// Iterate over a collection whos type must be declared with DECLARE_TYPEOF
/** Usage: FOR_EACH(e,collect) { body-of-loop }
 */
#define FOR_EACH(Elem,Collection)											\
		FOR_EACH_T(TYPEOF_IT(Collection), TYPEOF_REF(Collection), Elem, Collection, begin, end)

/// Iterate over a collection whos type must be declared with DECLARE_TYPEOF
/** Uses a const iterator
 *  Usage: FOR_EACH_CONST(e,collect) { body-of-loop }
 */
#define FOR_EACH_CONST(Elem,Collection)										\
		FOR_EACH_T(TYPEOF_CIT(Collection), TYPEOF_CREF(Collection), Elem, Collection, begin, end)

/// Iterate over a collection whos type must be declared with DECLARE_TYPEOF
/** Iterates using a reverse_iterator
 *  Usage: FOR_EACH_REVERSE(e,collect) { body-of-loop }
 */
#define FOR_EACH_REVERSE(Elem,Collection)									\
		FOR_EACH_T(TYPEOF_RIT(Collection), TYPEOF_REF(Collection), Elem, Collection, rbegin, rend)

/// Iterate over a collection whos type must be declared with DECLARE_TYPEOF
/** Iterates using a const_reverse_iterator
 *  Usage: FOR_EACH_CONST_REVERSE(e,collect) { body-of-loop }
 */
#define FOR_EACH_CONST_REVERSE(Elem,Collection)								\
		FOR_EACH_T(TYPEOF_CRIT(Collection), TYPEOF_CREF(Collection), Elem, Collection, rbegin, rend)

/// Iterate over two collection in parallel
/** Usage: FOR_EACH_2_T(TypeIt1,TypeElem1,e1,collect1,TypeIt2,TypeElem2,e2,collect2) { body-of-loop }
 *
 *  Note: This has got to be one of the craziest pieces of code I have ever written :)
 *  It is just an extension of the idea of FOR_EACH_T.
 */
#define FOR_EACH_2_T(TypeIt1,TypeElem1,Elem1,Coll1,TypeIt2,TypeElem2,Elem2,Coll2)	\
		for(std::pair<std::pair<TypeIt1,TypeIt2>, bool>						\
				Elem1##_IT(make_pair(Coll1.begin(), Coll2.begin()), true) ;	\
			Elem1##_IT.first.first  != Coll1.end() &&						\
			Elem1##_IT.first.second != Coll2.end() ;						\
			++Elem1##_IT.first.first, ++Elem1##_IT.first.second,			\
			Elem1##_IT.second = true)										\
			for(TypeElem1 Elem1 = *Elem1##_IT.first.first ;					\
				Elem1##_IT.second ;											\
				Elem1##_IT.second = false)									\
				for(TypeElem2 Elem2 = *Elem1##_IT.first.second ;			\
					Elem1##_IT.second ;										\
					Elem1##_IT.second = false)

/// Iterate over two collections in parallel, their type must be declared with DECLARE_TYPEOF.
/** Usage: FOR_EACH_2(e1,collect1, e2,collect2) { body-of-loop }
 */
#define FOR_EACH_2(Elem1,Collection1, Elem2,Collection2)									\
		FOR_EACH_2_T(TYPEOF_IT(Collection1), TYPEOF_REF(Collection1), Elem1, Collection1,	\
		             TYPEOF_IT(Collection2), TYPEOF_REF(Collection2), Elem2, Collection2)

/// Iterate over two constants collections in parallel, their type must be declared with DECLARE_TYPEOF.
/** Usage: FOR_EACH_2_CONST(e1,collect1, e2,collect2) { body-of-loop }
 */
#define FOR_EACH_2_CONST(Elem1,Collection1, Elem2,Collection2)								\
		FOR_EACH_2_T(TYPEOF_CIT(Collection1), TYPEOF_CREF(Collection1), Elem1, Collection1,	\
		             TYPEOF_CIT(Collection2), TYPEOF_CREF(Collection2), Elem2, Collection2)


// ----------------------------------------------------------------------------- : EOF
#endif
