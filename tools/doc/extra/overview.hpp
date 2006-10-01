/** \mainpage

This is the documentation of the Magic Set Editor (MSE) source code, automatically generated using doxygen.

<h2>Structure</h2>

The MSE source code is subdivided into several directories, with the following meaning:
 - util: Utility functions and classes, stuff that would work equally well in another project
 - gfx:  Graphics related functions, again mostly independent of MSE.
         This directory contains algorithms for image blending, scaling, and bezier curve functions.
 - data: Data structures, like sets, cards, symbols, etc.
         These data structures are documented in the <a href="http://magicseteditor.sourceforge.net/extending">'Extending MSE'</a> section of the official documentation.
     - data/action: Actions that can be applied to those data structures.
 - gui:  Graphical User Interface
 - resource: Resource files used (icons, cursors, etc.)


<h2>Libraries/dependencies</h2>

MSE depends on the following libraries:
 - <a href="http://wxwidgets.org">wxWidgets</a> for the GUI.
 - <a href="http://boost.org">boost</a>, just for shared_ptr and some preprocessor tricks.

Additional tools (not needed for building MSE) also depend on:
 - <a href="http://doxygen.org">doxygen</a> for generating the documentation.
 - Perl for small utility scripts


<h2>Coding style</h2>

MSE uses the following coding style:
@code
class ClassName {
  public:
    void someFunction();
  private:
    int someMember;
};
@endcode

The exception to this rule are wxWidgets functions, which LookLikeThis; and classes, which look like wxSomeClass.
As well as standard library functions.


<h2>Macro and template tricks</h2>

The source code uses several macro/preprocessor and template tricks to make the code more readable.

<h3>Smart pointers</h3>

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

<h3>Iterating</h3>

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

<h3>Reflection</h3>

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
     VALUE(value_of_enum_1); // the default value
     VALUE(value_of_enum_2);
 }
@endcode

Implemented in: util/reflect.hpp

*/