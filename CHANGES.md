Magic Set Editor changelog, for the details see `git log`
==============================================================================

HEAD: new items added as changes are made
------------------------------------------------------------------------------

Bug fixes:
 * Fixed: crash in expand_keywords when given empty tags (#90)
 * Fixed: tab traversal in native look editors (style and set info tabs) (#98)
 * Fixed: Mana Symbol Menu Items were notlonger using names from locale (#84)
 * Auto replaces that match "" are now disabled

------------------------------------------------------------------------------
version 2.1.1, 2020-06-14
------------------------------------------------------------------------------

Features:
 * Sorting of the card list can now be changed per window

Bug fixes:
 * Fixed: keywords after atoms were not showing up (#67)
 * Fixed: multiple keywords that matched in the same place both showed reminder text. (#70)
   Now, when there are overlapping matches the longest one is used.
 * Fixed: Slice Image window defaulting to Force to Fit (#69)
 * Fixed: Wide set symbols being shrunk down (#68)

------------------------------------------------------------------------------
version 2.1.0, 2020-06-01
------------------------------------------------------------------------------

Features:
 * In the quick search box you can specify which field to search in,
   for example `type:Wizard` searches for cards with Wizard in the type.
 * Added "Select All" to menu (#19)
 * Added "Save as Directory" to menu
 * Added a keyboard shortcut for the search box (Ctrl+K)

Bug fixes:
 * Keywords that appear multiple times don't mess up reminder text (#20)
 * card variable in console panel now refers to the selected card
 * length function now gives correct results for maps
 * substr("foo",begin:3) now returns "" instead of true

Template features:
 * Added `<font:...>` tag to change the font inside a text field.
 * Added `<margin:...>` tag to change the margins of a block of text.
 * Added `<align:...>` tag to change the horizontal alignment of a block of text.
 * Added `<li>` tag for list bullet points. (Experimental!)
 * Colors can now be written using hex notation, `#rrggbb` / `#rrggbbaa`, and short hex notation (`#rgb` / `#rgba`)
 * Added card_style.field.layout, with information on the position of each line and paragraph in a text box.
 * It is now possible to set the width and height of set info and style fields. This is especially useful for set specific images like watermarks and symbols.

Scripting:
 * Added type_name function
 * `nil != ""`, so missing values are no longer equal to the empty string
 * The `=` operator is now deprecated, use `==` for comparisons, `:=` for assignment.
 * if statements without an else will now produce a warning if their result is used.
 * Added case-of control structure, for comparing a value against multiple alternatives

Internal:
 * Switch build system to to CMake
 * Update code to work with wxWidgets 3.1 and C++ 17
 * Lots of code cleanup

------------------------------------------------------------------------------
version 2.0.1
------------------------------------------------------------------------------

 * Some bugfixes

Other
 * Changes to build system

------------------------------------------------------------------------------
version 2.0.0, 2011-02-05
------------------------------------------------------------------------------

Program:
 * Added operators ("" and -) to quick search
 * Added quick search for keywords (#58)
 * Added "Console" panel
 * Error message handling moved to console
 * fixed #56: The quick search bar doesn't look in card.notes.
 * fixed #59: Keywords with special characters don't work.
 * fixed: Selection in package lists (new set window and style tab) is not highlighted.

Templates:
 * no changes

Other:
 * Actually started maintaining changelog, older entries are reconstructed.

------------------------------------------------------------------------------
version 0.3.9, 2011-01-07
------------------------------------------------------------------------------

Program:
 * Added quick search box for filtering the card list
 * Win32: themed selection rectangles in GallaryList (broken)
 * Win32: themed selection rectangles in Card/KeywordList
 * bug fixes: #18,#19, #16,#24,#25,28, #13,#14,#51

Templates:
 * Magic: new keywords and various minor updates

Other:
 * This is the first new release in nearly two years

------------------------------------------------------------------------------
version 0.3.8, 2009-01-15
------------------------------------------------------------------------------

New in this release:
 * A spelling checker.
 * Improved random booster pack generator.
 * A bit of support for different languages in templates.
 * Improvements in text alignment.
 * Many other small improvements.
 * And of course lots of bug fixes.

------------------------------------------------------------------------------
version 0.3.7b, 2008-08-11
------------------------------------------------------------------------------

 * This release fixes some semi-critical bugs in 0.3.7b.

------------------------------------------------------------------------------
version 0.3.7, 2008-08-11
------------------------------------------------------------------------------

New in this release:
 * A random booster pack generator.
 * A command line interface for connecting MSE with other programs.
 * Many other minor improvements.
 * And of course lots of bug fixes.

------------------------------------------------------------------------------
version 0.3.6b, 2008-06-02
------------------------------------------------------------------------------

 * This release fixes some critical bugs in 0.3.6.

------------------------------------------------------------------------------
version 0.3.6, 2008-06-01
------------------------------------------------------------------------------

New in this release:
 * 2/R mana symbols and , the reverse tap symbol.
 * Customizable set statistics.
 * Improved printing quality.
 * User friendly system for installing templates.
 * Support for arbitrarily rotated text in templates.
 * Support for changing font size and color in templates.
 * Many, many bug fixes and minor improvements.

------------------------------------------------------------------------------
version 0.3.5b, 2007-09-21
------------------------------------------------------------------------------

 * This release fixes some critical bugs in 0.3.5.

------------------------------------------------------------------------------
version 0.3.5, 2007-09-20
------------------------------------------------------------------------------

New in this release:
 * Future sight templates.
 * Planeswalker template.
 * HTML export for VS-System.
 * New style VS-System templates.
 * Many, many bug fixes and minor improvements.

------------------------------------------------------------------------------
version 0.3.4, 2007-07-05
------------------------------------------------------------------------------

New in this release:
 * Export to HTML.
 * New color combination dialog for magic.
 * Modern style magic tokens.
 * VS-System hellboy templates.
 * and of course lots of minor improvements and bugfixes.

------------------------------------------------------------------------------
version 0.3.3, 2007-05-14
------------------------------------------------------------------------------

 * The crashes are finally fixed.
 * All keyword problems are fixed.
 * Two dimensional statistics (e.g. color vs. rarity)

------------------------------------------------------------------------------
version 0.3.2, 2007-05-11
------------------------------------------------------------------------------

 * The templates have recieved a large update.
 * The large bugs from the previous version are fixed.

------------------------------------------------------------------------------
version 0.3.1, 2007-04-21
------------------------------------------------------------------------------

 * The most important new thing are the keywords. You can now make keywords with multiple parameters.

------------------------------------------------------------------------------
version 0.3.0(NQMSE), 2006-12-25
------------------------------------------------------------------------------

Version 0.3.0, prerelease.
 * no keywords
 * no printing
 * no export
 * no search/replace
 * no help
 * no windows 9x build
 * lots of new bugs

------------------------------------------------------------------------------
version 0.2.7, 2006-08-04
------------------------------------------------------------------------------

New features:
 * Buttons for making text bold/italic
 * Good looking guild watermarks
 * Yu-Gi-Oh templates (by artfreakwiu)

Bug fixes / template tweaks:
 * Correct snow mana symbol
 * Fixed crash when creating new set
 * Settings of styles are finally saved correctly
 * The latest VS System templates

------------------------------------------------------------------------------
version 0.2.6, 2006-07-18
------------------------------------------------------------------------------

Bug fixes:
 * Duplicate text in keyword reminder text
 * Reminder text of Scry
 * Not remembering zoom, and border display settings
 * Crash when deleting cards
 * Drawing glitches under windows 9x
 * Support for symbols with transparency

------------------------------------------------------------------------------
version 0.2.5, 2006-06-25
------------------------------------------------------------------------------

New features:
 * Improved high quality rendering
 * Improved statistics/graphs
 * Smart keywords, numbers are written with words, for example Graft 2 (This creature comes into play with two +1/+1 counters on it...). To use this with your own keywords all you need to do is say "counter(s)".

Bug fixes:
 * Card sizes are now changed correctly when using different styles
 * The infamous 'n' keyword bug
 * Probably more Some template tweaks
