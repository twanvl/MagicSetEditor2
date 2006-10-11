/** @page tricks Macro and template tricks

The source code uses several macro/preprocessor and template tricks to make the code more readable.

<h2>Smart pointers</h2>

MSE makes extensive use of boost::shared_ptr. To make the code more readable there are typedefs for these pointer types, using a suffix P.
These are defined using a macro:
@code
 DECLARE_POINTER_TYPE(MyClass);
 MyClassP someObject; // the same as boost::shared_ptr<MyClass> someObject
@endcode

To create new shared_ptrs the function new_shared# can be used (where # is the number of arguments):
@code
 MyClassP someObject;
 someObject = new_shared2<MyClass>(arg1, arg2);
@endcode

Implemented in: util/smart_ptr.hpp


<h2>Iterating</h2>

To iterate over containers the FOR_EACH macro is used:
@code
 vector<CardP> cards;
 FOR_EACH(card, cards) {
     doSomething(card);
 }
@endcode
Is equivalent to:
@code
 vector<CardP> cards;
 for(vector<CardP>::iterator it = cards.begin() ; it != cards.end() ; ++it) {
     CardP& card = *it;
     doSomething(card);
 }
@endcode
The iterators are completely hidden!
There are several veriations to this macro, for using const iterators (FOR_EACH_CONST), iterating in reverse (FOR_EACH_REVERSE),
for iterating over two collections in parallel (FOR_EACH_2), and for getting access to the iterator (FOR_EACH_IT).

Each of these macros require that the collection type has been declared using:
@code
 DECLARE_COLLECTION_TYPE(CardP);
@endcode
This allows the calling of TYPEOF(cards) to evaluate to vector<CardP>.

Implemented in: util/for_each.hpp


<h2>Reflection</h2>

The io (input/output) system is based on reflection.
For a class to support reflection the following must be declared:
@code
 class SomeClass {
	int member1, member2;
	DECLARE_REFLECTION();
 };
@endcode
Then in a source file the members of the class have to be specified:
@code
 IMPLEMENT_REFLECTION(SomeClass) {
	REFLECT(member1);
	REFLECT_N("another_name", member2);
 }
@endcode

Simlairly for enumerations (a declaration is not necessary):
@code
 IMPLEMENT_REFLECTION_ENUM(MyEnum) {
     VALUE_N("value1", MY_VALUE1); // the first is the default value
     VALUE_N("value2", MY_VALUE2);
 }
@endcode

Reflection is used by the following classes:
 - Reader
 - Writer
 - GetMember

Implemented in: util/reflect.hpp

*/