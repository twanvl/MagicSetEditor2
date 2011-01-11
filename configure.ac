#+----------------------------------------------------------------------------+
#| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
#| Copyright:    (C) 2001 - 2007 Twan van Laarhoven and Sean "coppro" Hunt    |
#| License:      GNU General Public License 2 or later (see file COPYING)     |
#+----------------------------------------------------------------------------+

# Process this file with autoconf to produce a configure script.

AC_INIT(magicseteditor, 0.3.9, twanvl@users.sourceforge.net)
AC_CONFIG_SRCDIR([src/main.cpp])
# TODO: Actually use the config header
AC_CONFIG_HEADER([src/config.h])

AM_INIT_AUTOMAKE([foreign])

# Checks for programs.
if test -n "${CXXFLAGS}"; then
  user_set_cxxflags=yes
fi
AC_PROG_CXX
AC_ARG_ENABLE(debug, [--enable-debug  Enable debug build (requires debug
 versions of wxWidgets and libstdc++.])
if test "x${enable_debug}" = "xyes"; then
  DEFAULT_CXXFLAGS="-ggdb -O0"
  WXCONFIG_FLAGS="--debug"
else
  DEFAULT_CXXFLAGS="-O2"
  WXCONFIG_FLAGS=""
fi

if test X$user_set_cxxflags != Xyes; then
  CXXFLAGS=${DEFAULT_CXXFLAGS}
fi

# Checks for libraries.
AC_CHECK_LIB([hunspell],[Hunspell_create])

# Check for Boost
AX_BOOST_BASE([1.37.0])
AX_BOOST_REGEX

# Check for wxWidgets
AM_OPTIONS_WXCONFIG
AM_PATH_WXCONFIG([2.8.0],[HAVE_WX=1],[HAVE_WX=0],,${WXCONFIG_FLAGS})
if test "${HAVE_WX}" != 1; then
    AC_MSG_ERROR([
       wxWindows must be installed on your system
       but could not be configured.

       Please check that wx-config is in path, the directory
       where wxWindows libraries are installed (returned by
       'wx-config --libs' command) is in LD_LIBRARY_PATH or
       equivalent variable and wxWindows version is 2.6.0 or
       above. If --enable-debug was passed, please ensure
       debugging libraries are installed.
    ])
fi

AC_ARG_ENABLE(pch, [--enable-pch  Enable precompiled headers (Requires
 GCC of sufficiently high version). Speeds up compile times
 significantly... if it works.])

# Check for precompiled headers
# TODO: Deal with braindead GCC and actually check
AM_CONDITIONAL(GLIBCPP_BUILD_PCH, test "x$enable_pch" = "xyes" -a "x$CXX" = "xg++")

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

if test "x$ap_cv_atomic_builtins" = "xyes"; then
    AC_DEFINE(HAVE_GCC_ATOMIC_BUILTINS, 1, [Define if compiler provides atomic builtins])
fi

AC_OUTPUT([
	Makefile
])
