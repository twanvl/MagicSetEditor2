/** @page coding_conventions Coding conventions

MSE uses the following coding style:
@code
/// Doxygen documentation
class ClassName {
  public:
    void someMemberFunction();
  private:
    int some_member;		///< postfix doxygen documentation
};

void a_global_function();

enum MyEnumeration
{	MY_SOMETHING
,	MY_SOMETHING_ELSE
};
@endcode

The rules are:
 - Classes use CaptializationForEachWord
 - Member functions use camelCase
 - Data members and globals use lower_case_with_underscores
 - Constants (enumeration values) and macros are UPPER_CASE_WITH_UNDERSCORES

The exceptions to this are:
 - wxWidgets functions, which LookLikeThis
 - wxWidget classes, which look like wxSomeClass
 - C++ standard library and boost, lower_case for everything
 - Person names, in particular deCasteljau
 - Class names in function names, in particular clearDC


*/
