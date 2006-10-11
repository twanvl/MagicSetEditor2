/** @mainpage

This is the documentation of the Magic Set Editor (MSE) source code, automatically generated using doxygen.

Before starting with the source code, you should take a look at the following:
 - \subpage structure          "Structure of the MSE source code"
 - \subpage dependencies       "Libraries and dependencies"
 - \subpage coding_conventions "Coding conventions"
 - \subpage tricks             "Tricks used by MSE"
 

@page structure Structure of the MSE source code

The MSE source code is subdivided into several directories, with the following meaning:
 - <tt>util</tt>: Utility functions and classes, stuff that would work equally well in another project
     - <tt>util/io</tt>: Classes related to input and output
 - <tt>gfx</tt>:  Graphics related functions, again mostly independent of MSE.
                  This directory contains algorithms for image blending, scaling, and bezier curve functions.
 - <tt>data</tt>: Data structures, like sets, cards, symbols, etc.
                  These data structures are documented in the <a href="http://magicseteditor.sourceforge.net/extending">'Extending MSE'</a> section of the official documentation.
     - <tt>data/action</tt>: Actions that can be applied to those data structures.
     - <tt>data/field</tt>:  Data types for fields, values and styles. One source file per type.
     - <tt>data/format</tt>: File formats and import/export stuff.
 - <tt>script</tt>: The scripting system.
 - <tt>gui</tt>:  Graphical User Interface
     - <tt>set</tt>:    SetWindow related
     - <tt>symbol</tt>: SymbolWindow related
 - <tt>resource</tt>: Resource files used (icons, cursors, etc.)

See <a href="dirs.html">the directory list</a> for details.

@page dependencies Libraries and dependencies

MSE depends on the following libraries:
 - <a href="http://wxwidgets.org">wxWidgets</a> for the GUI.
 - <a href="http://boost.org">boost</a>, just for shared_ptr and some preprocessor tricks.

Additional tools (not needed for building MSE) also depend on:
 - <a href="http://doxygen.org">doxygen</a> for generating the documentation.
 - Perl for small utility scripts

*/