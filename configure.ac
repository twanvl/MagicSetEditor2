#+----------------------------------------------------------------------------+
#| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
#| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
#| License:      GNU General Public License 2 or later (see file COPYING)     |
#+----------------------------------------------------------------------------+

# Process this file with autoconf to produce a configure script.

AC_INIT(magicseteditor, 0.3.0, twanvl@users.sourceforge.net)
AC_CONFIG_SRCDIR([src/main.cpp])
# TODO: Actually use the config header
AC_CONFIG_HEADER([src/config.h])

AM_INIT_AUTOMAKE([-Wall foreign])

# Checks for programs.
AC_PROG_CXX

# Checks for libraries.
# TODO: Check for boost

# Check for wxWidgets
AM_OPTIONS_WXCONFIG
AM_PATH_WXCONFIG(2.6.0,wxWin=1)
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

AC_OUTPUT([
	Makefile
])
