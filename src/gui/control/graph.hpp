//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_CONTROL_GRAPH
#define HEADER_GUI_CONTROL_GRAPH

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/rotation.hpp>

DECLARE_POINTER_TYPE(GraphAxis);
DECLARE_POINTER_TYPE(GraphElement);
DECLARE_POINTER_TYPE(GraphData);
DECLARE_POINTER_TYPE(Graph);

// ----------------------------------------------------------------------------- : Events

/// Event that indicates the a subset is selected/deselected in a graph
DECLARE_EVENT_TYPE(EVENT_GRAPH_SELECT, <not used>)
/// Handle EVENT_GRAPH_SELECT events
#define EVT_GRAPH_SELECT(id, handler) EVT_COMMAND(id, EVENT_GRAPH_SELECT, handler)

// ----------------------------------------------------------------------------- : Graph data

/// A group in a table or graph
/** A group is rendered as a single bar or pie slice */
class GraphGroup {
  public:
	GraphGroup(const String& name, UInt size, const Color& color = *wxBLACK)
		: name(name), color(color), size(size)
	{}
	
	String name;	///< Name of this position
	Color  color;	///< Associated color
	UInt   size;	///< Number of elements in this group
};

/// An axis in a graph, consists of a list of groups
/** The sum of groups.sum = sum of all elements in the data */
class GraphAxis {
  public:
	GraphAxis(const String& name, bool auto_color = true, bool numeric = false)
		: name(name)
		, auto_color(auto_color)
		, numeric(numeric)
		, max(0)
	{}
	
	String             name;		///< Name/label of this axis
	bool               auto_color;	///< Automatically assign colors to the groups on this axis
	vector<GraphGroup> groups;		///< Groups along this axis
	bool               numeric;		///< Numeric axis?
	UInt               max;			///< Maximum size of the groups
};

/// A single data point of a graph
class GraphElement {
  public:
	GraphElement() {}
	GraphElement(const String& v1);
	GraphElement(const String& v1, const String& v2);
	
	vector<String> values; ///< Group name for each axis
};

/// Data to be displayed in a graph, not processed yet
class GraphDataPre {
  public:
	vector<GraphAxisP>    axes;
	vector<GraphElementP> elements;
};

/// Data to be displayed in a graph
class GraphData {
  public:
	GraphData(const GraphDataPre&);
	
	vector<GraphAxisP> axes;	///< The axes in the data
	vector<UInt>       values;	///< Multi dimensional (dim = axes.size()) array of values
	UInt               size;	///< Total number of elements
};


// ----------------------------------------------------------------------------- : Graph

/// A type of graph
/** It is rendered into a sub-rectangle of the screen */
class Graph {
  public:
	virtual ~Graph() {}
	/// Draw this graph, filling the internalRect() of the dc.
	virtual void draw(RotatedDC& dc, const vector<int>& current) const = 0;
	/// Find the item at the given position, the rectangle gives the screen size
	virtual bool findItem(const RealPoint& pos, const RealRect& rect, vector<int>& out) const { return false; }
	/// Change the data
	virtual void setData(const GraphDataP& d) { data = d; }
	/// Get the data
	inline const GraphData& getData() const { return *data; }
	
  protected:
	/// Data of the graph
	GraphDataP data;
};

/// Base class for 1 dimensional graph components
class Graph1D : public Graph {
  public:
	inline Graph1D(size_t axis) : axis(axis) {}
	virtual void draw(RotatedDC& dc, const vector<int>& current) const;
	virtual bool findItem(const RealPoint& pos, const RealRect& rect, vector<int>& out) const;
  protected:
	size_t axis;
	/// Find an item, return the position along the axis, or -1 if not found
	virtual int findItem(const RealPoint& pos, const RealRect& rect) const = 0;
	virtual void draw(RotatedDC& dc, int current) const = 0;
	inline GraphAxis& axis_data() const { return *data->axes.at(axis); }
};

/// A bar graph
class BarGraph : public Graph1D {
  public:
	inline BarGraph(size_t axis) : Graph1D(axis) {}
	virtual void draw(RotatedDC& dc, int current) const;
	virtual int findItem(const RealPoint& pos, const RealRect& rect) const;
};

// TODO
//class BarGraph2D {
//};

/// A pie graph
class PieGraph : public Graph1D {
  public:
	inline PieGraph(size_t axis) : Graph1D(axis) {}
	virtual void draw(RotatedDC& dc, int current) const;
	virtual int findItem(const RealPoint& pos, const RealRect& rect) const;
};

/// The legend, used for pie graphs
class GraphLegend : public Graph1D {
  public:
	inline GraphLegend(size_t axis) : Graph1D(axis) {}
	virtual void draw(RotatedDC& dc, int current) const;
	virtual int findItem(const RealPoint& pos, const RealRect& rect) const;
};

//class GraphTable {
//};

//class GraphAxis : public Graph1D {
//	virtual void draw(RotatedDC& dc) const;
//};

//class GraphValueAxis {
//	virtual void draw(RotatedDC& dc) const;
//};

// ----------------------------------------------------------------------------- : Graph control

/// A control showing statistics in a graphical form
class GraphControl : public wxControl {
  public:
	/// Create a graph control
	GraphControl(Window* parent, int id);
	
	/// Update the data in the graph
	void setData(const GraphDataPre& data);
	/// Update the data in the graph
	void setData(const GraphDataP& data);
	
	/// Is there a selection on the given axis?
	bool hasSelection(size_t axis) const;
	/// Get the current item along the given axis
	String getSelection(size_t axis) const;
	
  private:
	/// Graph object
	GraphP graph;
	/// The selected item per axis, or an empty vector if there is no selection
	/** If the value for an axis is -1, then all groups on that axis are selected */
	vector<int> current_item;
	
	DECLARE_EVENT_TABLE();
	
	void onPaint(wxPaintEvent&);
	void onSize (wxSizeEvent&);
	void onMouseDown(wxMouseEvent& ev);
};

// ----------------------------------------------------------------------------- : EOF
#endif
