//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
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
,	ID_FILE_RELOAD			= 9
,	ID_FILE_RECENT			= wxID_FILE1
,	ID_FILE_RECENT_MAX		= wxID_FILE9
,	ID_FILE_CHECK_UPDATES	= 10
,	ID_FILE_PROFILER        = 11

	// Edit menu
,	ID_EDIT_UNDO			= wxID_UNDO
,	ID_EDIT_REDO			= wxID_REDO
,	ID_EDIT_CUT				= wxID_CUT
,	ID_EDIT_COPY			= wxID_COPY
,	ID_EDIT_PASTE			= wxID_PASTE
,	ID_EDIT_DELETE			= 101
,	ID_EDIT_FIND			= wxID_FIND
,	ID_EDIT_FIND_NEXT		= 103
,	ID_EDIT_REPLACE			= wxID_REPLACE
,	ID_EDIT_AUTO_REPLACE	= 104
,	ID_EDIT_PREFERENCES		= 105

	// Window menu (MainWindow)
,	ID_WINDOW_NEW			= 201
,	ID_WINDOW_MIN			= 202
,	ID_WINDOW_CARDS			= ID_WINDOW_MIN + 0
,	ID_WINDOW_SET			= ID_WINDOW_MIN + 1
,	ID_WINDOW_STYLE			= ID_WINDOW_MIN + 2
,	ID_WINDOW_KEYWORDS		= ID_WINDOW_MIN + 3
,	ID_WINDOW_STATS			= ID_WINDOW_MIN + 4
,	ID_WINDOW_RANDOM_PACK	= ID_WINDOW_MIN + 5
,	ID_WINDOW_MAX			= 220

	// Help menu (MainWindow)
,	ID_HELP_INDEX			= wxID_HELP_CONTENTS
,	ID_HELP_WEBSITE			= 301
,	ID_HELP_DOCUMENTATION
,	ID_HELP_ABOUT			= wxID_ABOUT

	// Mode menu (SymbolWindow)
,	ID_MODE_MIN				= 401
,	ID_MODE_SELECT			= ID_MODE_MIN
,	ID_MODE_ROTATE
,	ID_MODE_POINTS
,	ID_MODE_SHAPES
,	ID_MODE_SYMMETRY
,	ID_MODE_PAINT
,	ID_MODE_MAX
};


// ----------------------------------------------------------------------------- : Child ids

/// Ids for menus on child panels (MainWindowPanel / SymbolEditorBase)
enum ChildMenuID {

	ID_CHILD_MIN			= 6000
,	ID_CHILD_MAX			= 16999

	// Cards menu
,	ID_CARD_ADD				= 6001
,	ID_CARD_ADD_MULT
,	ID_CARD_REMOVE
,	ID_CARD_PREV
,	ID_CARD_NEXT
,	ID_CARD_ROTATE
,	ID_CARD_ROTATE_0
,	ID_CARD_ROTATE_90
,	ID_CARD_ROTATE_180
,	ID_CARD_ROTATE_270
	// CardList
,	ID_SELECT_COLUMNS

	// Keyword menu
,	ID_KEYWORD_ADD			= 6101
,	ID_KEYWORD_REMOVE
,	ID_KEYWORD_PREV
,	ID_KEYWORD_NEXT

	// Format menu
,	ID_FORMAT_BOLD			= 6201
,	ID_FORMAT_ITALIC
,	ID_FORMAT_SYMBOL
,	ID_FORMAT_REMINDER
,	ID_INSERT_SYMBOL

	// Spelling errors
,	ID_SPELLING_ADD_TO_DICT = 6301
,	ID_SPELLING_NO_SUGGEST
,	ID_SPELLING_SUGGEST
,	ID_SPELLING_SUGGEST_MAX = 6399

	// Graph menu
,	ID_GRAPH_PIE			= 6401 // corresponds to GraphType
,	ID_GRAPH_BAR
,	ID_GRAPH_STACK
,	ID_GRAPH_SCATTER
,	ID_GRAPH_SCATTER_PIE

	// SymbolSelectEditor toolbar/menu
,	ID_SYMBOL_COMBINE				= 7001
,	ID_SYMBOL_COMBINE_MERGE			= ID_SYMBOL_COMBINE + 0 //SYMBOL_COMBINE_MERGE
,	ID_SYMBOL_COMBINE_SUBTRACT		= ID_SYMBOL_COMBINE + 1 //SYMBOL_COMBINE_SUBTRACT
,	ID_SYMBOL_COMBINE_INTERSECTION	= ID_SYMBOL_COMBINE + 2 //SYMBOL_COMBINE_INTERSECTION
,	ID_SYMBOL_COMBINE_DIFFERENCE	= ID_SYMBOL_COMBINE + 3 //SYMBOL_COMBINE_DIFFERENCE
,	ID_SYMBOL_COMBINE_OVERLAP		= ID_SYMBOL_COMBINE + 4 //SYMBOL_COMBINE_OVERLAP
,	ID_SYMBOL_COMBINE_BORDER		= ID_SYMBOL_COMBINE + 5 //SYMBOL_COMBINE_BORDER
,	ID_SYMBOL_COMBINE_MAX
,	ID_EDIT_DUPLICATE					// duplicating symbol parts
,	ID_EDIT_GROUP
,	ID_EDIT_UNGROUP
,	ID_VIEW_GRID
,	ID_VIEW_GRID_SNAP

	// SymbolPointEditor toolbar/menu
,	ID_SEGMENT				= 7101
,	ID_SEGMENT_LINE			= ID_SEGMENT + 0//SEGMENT_LINE
,	ID_SEGMENT_CURVE		= ID_SEGMENT + 1//SEGMENT_CURVE
,	ID_SEGMENT_MAX
,	ID_LOCK					= 7151
,	ID_LOCK_FREE			= ID_LOCK + 0//LOCK_FREE
,	ID_LOCK_DIR				= ID_LOCK + 1//LOCK_DIR
,	ID_LOCK_SIZE			= ID_LOCK + 2//LOCK_SIZE
,	ID_LOCK_MAX

	// SymbolBasicShapeEditor toolbar/menu
,	ID_SHAPE				= 7201
,	ID_SHAPE_CIRCLE			= ID_SHAPE
,	ID_SHAPE_RECTANGLE
,	ID_SHAPE_POLYGON
,	ID_SHAPE_STAR
,	ID_SHAPE_MAX
,	ID_SIDES
	
	// SymbolSymmetryEditor toolbar/menu
,	ID_SYMMETRY				= 7301
,	ID_SYMMETRY_ROTATION	= ID_SYMMETRY
,	ID_SYMMETRY_REFLECTION
,	ID_SYMMETRY_MAX
,	ID_ADD_SYMMETRY
,	ID_REMOVE_SYMMETRY
,	ID_COPIES
	
	// On cards panel
,	ID_COLLAPSE_NOTES		= 8001
,	ID_CARD_FILTER
	
	// Style panel
,	ID_STYLE_USE_FOR_ALL	= 8011
,	ID_STYLE_USE_CUSTOM

	// Keywords panel
,	ID_KEYWORD_ADD_PARAM	= 8021
,	ID_KEYWORD_REF_PARAM
,	ID_KEYWORD_MODE
,	ID_KEYWORD_FILTER
,	ID_PARAM_TYPE_MIN		= 8101
,	ID_PARAM_TYPE_MAX		= 8200
,	ID_PARAM_REF_MIN		= 8201
,	ID_PARAM_REF_MAX		= 8300
	
	// Statistics panel
,	ID_FIELD_LIST			= 8301
	
	// Random pack panel
,	ID_PACK_AMOUNT			= 8111
,	ID_PACK_TYPE
,	ID_SEED_RANDOM
,	ID_SEED_FIXED
,	ID_GENERATE_PACK
,	ID_CUSTOM_PACK
	
	// Console panel
,	ID_EVALUATE
	
	// SymbolFont (Format menu)
,	ID_INSERT_SYMBOL_MENU_MIN =  9001
,	ID_INSERT_SYMBOL_MENU_MAX = 10000
	
	// AddCardsScript (Card menu)
,	ID_ADD_CARDS_MENU_MIN = 10001
,	ID_ADD_CARDS_MENU_MAX = 11000
};


// ----------------------------------------------------------------------------- : Control/window ids

/// Window ids for controls
enum ControlID {

	ID_CONTROL_MIN			= 1000
,	ID_CONTROL_MAX			= 1999

	// Controls
,	ID_VIEWER				= 1001
,	ID_EDITOR
,	ID_CONTROL
,	ID_TAB_BAR
,	ID_CARD_LIST
,	ID_PART_LIST
,	ID_GAME_LIST
,	ID_STYLESHEET_LIST
,	ID_KEYWORD_LIST
,	ID_EXPORT_LIST
,	ID_NOTES
,	ID_KEYWORD
,	ID_MATCH
,	ID_REMINDER
,	ID_RULES
,	ID_MESSAGE_LIST
	// Card list column select
,	ID_MOVE_UP
,	ID_MOVE_DOWN
,	ID_SHOW
,	ID_HIDE
	// Card select
,	ID_SELECT_CARDS
,	ID_SELECTION_CHOICE
,	ID_SELECTION_CHOICE_MAX = ID_SELECTION_CHOICE + 100
,	ID_SELECT_ALL
,	ID_SELECT_NONE
	// Settings
,	ID_NOTEBOOK
,	ID_APPRENTICE_BROWSE
,	ID_CHECK_UPDATES_NOW
	// Image slicer
,	ID_PREVIEW
,	ID_SELECTOR
,	ID_SIZE
,	ID_LEFT
,	ID_TOP
,	ID_WIDTH
,	ID_HEIGHT
,	ID_FIX_ASPECT
,	ID_ZOOM
,	ID_ZOOM_X
,	ID_ZOOM_Y
,	ID_SHARPEN
,	ID_SHARPEN_AMOUNT
	// Updates window
,	ID_PACKAGE_LIST
,	ID_KEEP
,	ID_INSTALL
,	ID_UPGRADE
,	ID_REMOVE
	// Auto replace window
,	ID_USE_AUTO_REPLACE
,	ID_ITEM_VALUE
,	ID_ADD_ITEM
,	ID_REMOVE_ITEM
,	ID_DEFAULTS
};

// ----------------------------------------------------------------------------- : EOF
#endif
