//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_STATISTICS
#define HEADER_DATA_STATISTICS

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/reflect.hpp>
#include <data/graph_type.hpp>
#include <script/scriptable.hpp>

class Field;
DECLARE_POINTER_TYPE(StatsDimension);
DECLARE_POINTER_TYPE(StatsCategory);

// ----------------------------------------------------------------------------- : Statistics dimension

/// A dimension that can be plotted as an axis in a graph
/** Dimensions can be generated automatically based on card fields */
class StatsDimension : public IntrusivePtrBase<StatsDimension> {
  public:
	StatsDimension();
	StatsDimension(const Field&);
	
	const bool        automatic;		///< Based on a card field?
	String            name;				///< Name of this dimension
	String            description;		///< Description, used in status bar
	int               position_hint;	///< Hint for the ordering
	String            icon_filename;	///< Icon for lists
	Bitmap            icon;				///< The loaded icon (optional of course)
	OptionalScript    script;			///< Script that determines the value(s)
	bool              numeric;			///< Are the values numeric? If so, they require special sorting
	bool              show_empty;		///< Should "" be shown?
	bool              split_list;		///< Split values into multiple ones separated by commas
	map<String,Color> colors;			///< Colors for the categories
	vector<String>    groups;			///< Order of the items
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : Statistics category

/// A category for statistics
/** Can be generated automatically based on a dimension */
class StatsCategory : public IntrusivePtrBase<StatsCategory> {
  public:
	StatsCategory();
	StatsCategory(const StatsDimensionP&);
	
	const bool              automatic;		///< Automatically generated?
	String                  name;			///< Name/label
	String                  description;	///< Description, used in status bar
	int                     position_hint;	///< Hint for the ordering
	String                  icon_filename;	///< Icon for lists
	Bitmap                  icon;			///< The loaded icon (optional of course)
	vector<String>          dimension_names;///< Names of the dimensions to use
	vector<StatsDimensionP> dimensions;		///< Actual dimensions
	GraphType               type;			///< Type of graph to use
	
	/// Initialize dimensions from dimension_names
	void find_dimensions(const vector<StatsDimensionP>& available);
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : EOF
#endif
