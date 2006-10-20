//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_WINDOW_ID
#define HEADER_UTIL_WINDOW_ID

/** @file util/window_id.hpp
 *
 *  @brief Enumerations of all window ids used.
 */

// ----------------------------------------------------------------------------- : Includes

// ----------------------------------------------------------------------------- : Menu ids

/// Window ids for menus and toolbars
enum MenuID {
	
	ID_MENU_MIN				= 0
,	ID_MENU_MAX				= 999

	//	File menu
,	ID_FILE_NEW				= wxID_NEW
,	ID_FILE_OPEN			= wxID_OPEN
,	ID_FILE_SAVE			= wxID_SAVE
,	ID_FILE_SAVE_AS			= wxID_SAVEAS
,	ID_FILE_STORE			= 1
,	ID_FILE_EXIT			= wxID_EXIT
,	ID_FILE_EXPORT			= 2
,	ID_FILE_EXPORT_HTML		= 3
,	ID_FILE_EXPORT_IMAGE	= 4
,	ID_FILE_EXPORT_IMAGES	= 5
,	ID_FILE_EXPORT_APPR		= 6
,	ID_FILE_EXPORT_MWS		= 7
,	ID_FILE_PRINT			= wxID_PRINT
,	ID_FILE_PRINT_PREVIEW	= wxID_PREVIEW
,	ID_FILE_INSPECT			= 8
,	ID_FILE_RECENT			= wxID_FILE1
,	ID_FILE_RECENT_MAX		= wxID_FILE9

	// Edit menu
,	ID_EDIT_UNDO			= wxID_UNDO
,	ID_EDIT_REDO			= wxID_REDO
,	ID_EDIT_CUT				= wxID_CUT
,	ID_EDIT_COPY			= wxID_COPY
,	ID_EDIT_PASTE			= wxID_PASTE
,	ID_EDIT_DELETE			= 101
,	ID_EDIT_DUPLICATE		= 102
,	ID_EDIT_FIND			= wxID_FIND
,	ID_EDIT_FIND_NEXT		= 103
,	ID_EDIT_REPLACE			= wxID_REPLACE
,	ID_EDIT_PREFERENCES		= 104

	// Window menu (MainWindow)
,	ID_WINDOW_NEW			= 201
,	ID_WINDOW_MIN			= 202
,	ID_WINDOW_MAX			= 220

	// Help menu (MainWindow)
,	ID_HELP_INDEX			= 301
,	ID_HELP_ABOUT

	// Mode menu (SymbolWindow)
,	ID_MODE_MIN				= 401
,	ID_MODE_SELECT			= ID_MODE_MIN
,	ID_MODE_ROTATE
,	ID_MODE_POINTS
,	ID_MODE_SHAPES
,	ID_MODE_PAINT
,	ID_MODE_MAX
};


// ----------------------------------------------------------------------------- : Child ids

/// Ids for menus on child panels (MainWindowPanel / SymbolEditorBase)
enum ChildMenuID {

	ID_CHILD_MIN			= 1000
,	ID_CHILD_MAX			= 2999

	// Cards menu
,	ID_CARD_ADD				= 1001
,	ID_CARD_ADD_MULT
,	ID_CARD_REMOVE
,	ID_CARD_PREV
,	ID_CARD_NEXT
,	ID_CARD_ROTATE
,	ID_CARD_ROTATE_0
,	ID_CARD_ROTATE_90
,	ID_CARD_ROTATE_180
,	ID_CARD_ROTATE_270

	// Keyword menu
,	ID_KEYWORD_ADD			= 1101
,	ID_KEYWORD_REMOVE
,	ID_KEYWORD_PREV
,	ID_KEYWORD_NEXT

	// Format menu
,	ID_FORMAT_BOLD			= 1201
,	ID_FORMAT_ITALIC
,	ID_FORMAT_SYMBOL

	// SymbolSelectEditor toolbar/menu
,	ID_PART					= 2001
,	ID_PART_MERGE			= ID_PART + 0//PART_MERGE
,	ID_PART_SUBTRACT		= ID_PART + 1//PART_SUBTRACT
,	ID_PART_INTERSECTION	= ID_PART + 2//PART_INTERSECTION
,	ID_PART_DIFFERENCE		= ID_PART + 3//PART_DIFFERENCE
,	ID_PART_OVERLAP			= ID_PART + 4//PART_OVERLAP
,	ID_PART_BORDER			= ID_PART + 5//PART_BORDER
,	ID_PART_MAX

	// SymbolPointEditor toolbar/menu
,	ID_SEGMENT				= 2101
,	ID_SEGMENT_LINE			= ID_SEGMENT + 0//SEGMENT_LINE
,	ID_SEGMENT_CURVE		= ID_SEGMENT + 1//SEGMENT_CURVE
,	ID_SEGMENT_MAX
,	ID_LOCK					= 2151
,	ID_LOCK_FREE			= ID_LOCK + 0//LOCK_FREE
,	ID_LOCK_DIR				= ID_LOCK + 1//LOCK_DIR
,	ID_LOCK_SIZE			= ID_LOCK + 2//LOCK_SIZE
,	ID_LOCK_MAX

	// SymbolBasicShapeEditor toolbar/menu
,	ID_SHAPE				= 2201
,	ID_SHAPE_CIRCLE			= ID_SHAPE
,	ID_SHAPE_RECTANGLE
,	ID_SHAPE_POLYGON
,	ID_SHAPE_STAR
,	ID_SHAPE_MAX
,	ID_SIDES

	// CardList
,	ID_SELECT_COLUMNS		= 3001
};


// ----------------------------------------------------------------------------- : Control/window ids

/// Window ids for controls
enum ControlID {

	ID_CONTROL_MIN			= 6000
,	ID_CONTROL_MAX			= 6999

	// Controls
,	ID_VIEWER				= 6001
,	ID_EDITOR
,	ID_CONTROL
,	ID_TAB_BAR
,	ID_CARD_LIST
,	ID_PART_LIST
,	ID_NOTES
,	ID_KEYWORD
,	ID_PARAMETER
,	ID_SEPARATOR
,	ID_REMINDER
,	ID_RULES
};

// ----------------------------------------------------------------------------- : EOF
#endif
