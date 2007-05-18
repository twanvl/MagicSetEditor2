rm Makefile.am;
echo "
#+----------------------------------------------------------------------------+
#| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
#| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
#| License:      GNU General Public License 2 or later (see file COPYING)     |
#+----------------------------------------------------------------------------+

# This flag allows us to use subdirectories:
AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = magicseteditor
AM_CXXFLAGS = @WX_CXXFLAGS@ -DUNICODE -I . -Wall
AM_LDFLAGS  = @WX_LIBS@

magicseteditor_SOURCES = 

# The script used to generate is MakeAM.sh " > Makefile.am;

find . -name *.cpp | sed "s/\./magicseteditor_SOURCES += ./" >> Makefile.am;