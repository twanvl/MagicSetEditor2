#+----------------------------------------------------------------------------+
#| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
#| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
#| License:      GNU General Public License 2 or later (see file COPYING)     |
#+----------------------------------------------------------------------------+

# Process this file with autoconf to produce a configure script.

AC_INIT(magicseteditor, 0.3.7b, twanvl@users.sourceforge.net)
AC_CONFIG_SRCDIR([src/main.cpp])
# TODO: Actually use the config header
AC_CONFIG_HEADER([src/config.h])

AM_INIT_AUTOMAKE([-Wall foreign])

# Checks for programs.
AC_PROG_CXX
# Do not pass -O2, because that causes failures for now.
CXXFLAGS=-g
# Checks for libraries.
AC_CHECK_LIB([hunspell],[Hunspell_create])

# Check for Boost
AX_BOOST_BASE([1.37.0])
AX_BOOST_REGEX
BOOST_REGEX_LIB="-Wl,-Bstatic $BOOST_REGEX_LIB -Wl,-Bdynamic"

# Check for wxWidgets
AM_OPTIONS_WXCONFIG
AM_PATH_WXCONFIG(2.8.0,wxWin=1)
if test "$wxWin" != 1; then
    AC_MSG_ERROR([
       wxWindows must be installed on your system
       but wx-config script couldn't be found.

       Please check that wx-config is in path, the directory
       where wxWindows libraries are installed (returned by
       'wx-config --libs' command) is in LD_LIBRARY_PATH or
        equivalent variable and wxWindows version is 2.6.0 or above.
    ])
fi

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_HEADER_STDC
AC_FUNC_MALLOC
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([floor memset pow select sqrt])

AC_CACHE_CHECK([whether the compiler provides atomic builtins], [ap_cv_atomic_builtins],
[AC_TRY_RUN([
int main()
{
    unsigned long val = 1010, tmp, *mem = &val;

    if (__sync_fetch_and_add(&val, 1010) != 1010 || val != 2020)
        return 1;

    tmp = val;

    if (__sync_fetch_and_sub(mem, 1010) != tmp || val != 1010)
        return 1;

    if (__sync_sub_and_fetch(&val, 1010) != 0 || val != 0)
        return 1;

    tmp = 3030;

    if (__sync_val_compare_and_swap(mem, 0, tmp) != 0 || val != tmp)
        return 1;

    if (__sync_lock_test_and_set(&val, 4040) != 3030)
        return 1;

    mem = &tmp;

    if (__sync_val_compare_and_swap(&mem, &tmp, &val) != &tmp)
        return 1;

    __sync_synchronize();

    if (mem != &val)
        return 1;

    return 0;
}], [ap_cv_atomic_builtins=yes], [ap_cv_atomic_builtins=no], [ap_cv_atomic_builtins=no])])

if test "$ap_cv_atomic_builtins" = "yes"; then
    AC_DEFINE(HAVE_GCC_ATOMIC_BUILTINS, 1, [Define if compiler provides atomic builtins])
fi

AC_OUTPUT([
	Makefile
])
