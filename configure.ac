#+----------------------------------------------------------------------------+
#| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
#| Copyright:    (C) 2001 - 2007 Twan van Laarhoven and Sean "coppro" Hunt    |
#| License:      GNU General Public License 2 or later (see file COPYING)     |
#+----------------------------------------------------------------------------+

# Process this file with autoconf to produce a configure script.

AC_INIT(magicseteditor, 0.3.9, twanvl@users.sourceforge.net)
AC_CANONICAL_SYSTEM

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
DEFAULT_CXXFLAGS="$DEFAULT_CXXFLAGS -std=gnu++98"

if test X$user_set_cxxflags != Xyes; then
  CXXFLAGS=${DEFAULT_CXXFLAGS}
fi

# Checks for libraries.
AC_CHECK_LIB([hunspell],[Hunspell_create])

# Check for Boost
AX_BOOST_BASE([1.37.0])
AX_BOOST_REGEX
AX_BOOST_SYSTEM

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

AC_OUTPUT([
	Makefile
])
