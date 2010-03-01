//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_CONTROL_GRAPH
#define HEADER_GUI_CONTROL_GRAPH

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/alignment.hpp>
#include <util/rotation.hpp>
#include <data/graph_type.hpp>

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
class GraphGroup : public IntrusivePtrBase<GraphGroup> {
  public:
	GraphGroup(const String& name, UInt size, const Color& color = *wxBLACK)
		: name(name), color(color), size(size)
	{}
	
	String name;	///< Name of this position
	Color  color;	///< Associated color
	UInt   size;	///< Number of elements in this group
};

/// Automatic coloring mode
enum AutoColor
{	AUTO_COLOR_NO
,	AUTO_COLOR_EVEN
,	AUTO_COLOR_WEIGHTED
};

/// An axis in a graph, consists of a list of groups
/** The sum of groups.sum = sum of all elements in the data */
class GraphAxis : public IntrusivePtrBase<GraphAxis> {
  public:
	GraphAxis(const String& name, AutoColor auto_color = AUTO_COLOR_EVEN, bool numeric = false, double bin_size = 0, const map<String,Color>* colors = nullptr, const vector<String>* order = nullptr)
		: name(name)
		, auto_color(auto_color)
		, numeric(numeric), bin_size(bin_size)
		, max(0)
		, total(0)
		, mean_value(0), max_value(-numeric_limits<double>::infinity())
		, colors(colors)
		, order(order)
	{}
	
	String             name;		///< Name/label of this axis
	AutoColor          auto_color;	///< Automatically assign colors to the groups on this axis
	vector<GraphGroup> groups;		///< Groups along this axis
	bool               numeric;		///< Numeric axis?
	double             bin_size;	///< Group numeric values into bins of this size
	UInt               max;			///< Maximum size of the groups
	UInt               total;		///< Sum of the size of all groups
	double             mean_value;		///< Mean value, only for numeric axes
	double             max_value;		///< Maximal value, only for numeric axes
	const map<String,Color>* colors;	///< Colors for each choice (optional)
	const vector<String>*    order;		///< Order of the items (optional)
	
	/// Add a graph group
	void addGroup(const String& name, UInt size);
};

/// A single data point of a graph
class GraphElement : public IntrusivePtrBase<GraphElement> {
  public:
	GraphElement(size_t original_index) : original_index(original_index) {}
	
	size_t         original_index; ///< Corresponding index in the original input
	vector<String> values;         ///< Group name for each axis
};

/// Data to be displayed in a graph, not processed yet
class GraphDataPre {
  public:
	vector<GraphAxisP>    axes;
	vector<GraphElementP> elements;
	/// Split compound elements, "a,b,c" -> "a" and "b" and "c"
	void splitList(size_t axis);
};

/// A single data point of a graph
struct GraphDataElement {
	size_t original_index;
	int    group_nrs[1];   ///< Group number for each axis
};

/// Data to be displayed in a graph
class GraphData : public IntrusivePtrBase<GraphData> {
  public:
	GraphData(const GraphDataPre&);
	~GraphData();
	
	vector<GraphAxisP>        axes;		///< The axes in the data
	vector<GraphDataElement*> values;	///< All elements, with the group number for each axis, or -1
	UInt                      size;		///< Total number of elements
	
	/// Create a cross table for two axes
	void crossAxis(size_t axis1, size_t axis2, vector<UInt>& out) const;
	/// Create a cross table for three axes
	void crossAxis(size_t axis1, size_t axis2, size_t axis3, vector<UInt>& out) const;
	/// Count the number of elements with the given values, -1 is a wildcard
	UInt count(const vector<int>& match) const;
	/// Get the original_indices of elements matching the selection
	void indices(const vector<int>& match, vector<size_t>& out) const;
};


// ----------------------------------------------------------------------------- : Graph

enum DrawLayer
{	LAYER_BOTTOM    = 0
,	LAYER_SELECTION = 0
,	LAYER_AXES
,	LAYER_VALUES
,	LAYER_COUNT
};

/// A type of graph
/** It is rendered into a sub-rectangle of the screen */
class Graph : public IntrusivePtrVirtualBase {
  public:
	/// Determine the size of this graph viewer, return -1 if the viewer stretches
	virtual RealSize determineSize(RotatedDC& dc) const { return RealSize(-1,-1); }
	/// Draw this graph, filling the internalRect() of the dc.
	virtual void draw(RotatedDC& dc, const vector<int>& current, DrawLayer layer) const = 0;
	/// Find the item at the given position, the rectangle gives the screen size
	virtual bool findItem(const RealPoint& pos, const RealRect& rect, bool tight, vector<int>& out) const { return false; }
	/// Change the data
	virtual void setData(const GraphDataP& d) { data = d; }
	/// Get the data
	inline const GraphDataP& getData() const { return data; }
	
  protected:
	/// Data of the graph
	GraphDataP data;
};

/// Base class for 1 dimensional graph components
class Graph1D : public Graph {
  public:
	inline Graph1D(size_t axis) : axis(axis) {}
	virtual void draw(RotatedDC& dc, const vector<int>& current, DrawLayer layer) const;
	virtual bool findItem(const RealPoint& pos, const RealRect& rect, bool tight, vector<int>& out) const;
  protected:
	size_t axis;
	/// Find an item, return the position along the axis, or -1 if not found
	virtual int findItem(const RealPoint& pos, const RealRect& rect, bool tight) const { return -1; }
	virtual void draw(RotatedDC& dc, int current, DrawLayer layer) const = 0;
	inline GraphAxis& axis_data() const { return *data->axes.at(axis); }
};

/// Base class for 2 dimensional graph components
class Graph2D : public Graph {
  public:
	inline Graph2D(size_t axis1, size_t axis2) : axis1(axis1), axis2(axis2) {}
	virtual void setData(const GraphDataP& d);
  protected:
	size_t axis1, axis2;
	vector<UInt> values; // axis1.size * axis2.size array
	inline GraphAxis& axis1_data() const { return *data->axes.at(axis1); }
	inline GraphAxis& axis2_data() const { return *data->axes.at(axis2); }
};

/// A bar graph
class BarGraph : public Graph1D {
  public:
	inline BarGraph(size_t axis) : Graph1D(axis) {}
	virtual void draw(RotatedDC& dc, int current, DrawLayer layer) const;
	virtual int findItem(const RealPoint& pos, const RealRect& rect, bool tight) const;
};

// A bar graph with stacked bars
class BarGraph2D : public Graph2D {
  public:
	inline BarGraph2D(size_t axis_h, size_t axis_v) : Graph2D(axis_h, axis_v) {}
	virtual void draw(RotatedDC& dc, const vector<int>& current, DrawLayer layer) const;
	virtual bool findItem(const RealPoint& pos, const RealRect& rect, bool tight, vector<int>& out) const;
};

/// A pie graph
class PieGraph : public Graph1D {
  public:
	inline PieGraph(size_t axis) : Graph1D(axis) {}
	virtual void draw(RotatedDC& dc, int current, DrawLayer layer) const;
	virtual int findItem(const RealPoint& pos, const RealRect& rect, bool tight) const;
};

/// A scatter plot
class ScatterGraph : public Graph2D {
  public:
	inline ScatterGraph(size_t axis1, size_t axis2) : Graph2D(axis1, axis2) {}
	virtual void draw(RotatedDC& dc, const vector<int>& current, DrawLayer layer) const;
	virtual bool findItem(const RealPoint& pos, const RealRect& rect, bool tight, vector<int>& out) const;
	virtual void setData(const GraphDataP& d);
  protected:
	UInt max_value;
	double max_value_x, max_value_y; ///< highest sum of two adjacent scaled values (radii)
	static double scale(double x); ///< nonlinear scaling
};

/// A scatter plot with an extra dimension
class ScatterGraphPlus : public ScatterGraph {
  public:
	inline ScatterGraphPlus(size_t axis1, size_t axis2, size_t axis3) : ScatterGraph(axis1, axis2), axis3(axis3) {}
	virtual void setData(const GraphDataP& d);
  protected:
	size_t axis3;
	vector<UInt> values3D; // axis1.size * axis2.size * axis3.size array
	inline GraphAxis& axis3_data() const { return *data->axes.at(axis3); }
};

/// A scatter plot with a pie graph for the third dimension
class ScatterPieGraph : public ScatterGraphPlus {
  public:
	inline ScatterPieGraph(size_t axis1, size_t axis2, size_t axis3) : ScatterGraphPlus(axis1, axis2, axis3) {}
	virtual void draw(RotatedDC& dc, const vector<int>& current, DrawLayer layer) const;
};

/// The legend, used for pie graphs
class GraphLegend : public Graph1D {
  public:
	inline GraphLegend(size_t axis, Alignment alignment, bool reverse = false)
		: Graph1D(axis), alignment(alignment), reverse(reverse)
	{}
	virtual RealSize determineSize(RotatedDC& dc) const;
	virtual void draw(RotatedDC& dc, int current, DrawLayer layer) const;
	virtual int findItem(const RealPoint& pos, const RealRect& rect, bool tight) const;
  private:
	mutable RealSize size, item_size;
	Alignment alignment;
	bool reverse;
};

/// Simple statistics like the mean
class GraphStats : public Graph1D {
  public:
	inline GraphStats(size_t axis, Alignment alignment)
		: Graph1D(axis), alignment(alignment)
	{}
	virtual RealSize determineSize(RotatedDC& dc) const;
	virtual void draw(RotatedDC& dc, int current, DrawLayer layer) const;
	virtual void setData(const GraphDataP& d);
  private:
	mutable RealSize size, item_size;
	mutable double label_width;
	Alignment alignment;
	vector<pair<String,String> > values;
};

//class GraphTable {
//};

enum DrawLines
{	DRAW_LINES_NO
,	DRAW_LINES_BETWEEN
,	DRAW_LINES_MID
};

/// Draws a horizontal/vertical axis for group labels
class GraphLabelAxis : public Graph1D {
  public:
	inline GraphLabelAxis(size_t axis, Direction direction, bool rotate = false, DrawLines draw_lines = DRAW_LINES_NO, bool label = false)
		: Graph1D(axis), direction(direction), rotate(rotate), draw_lines(draw_lines), label(label)
	{}
	virtual void draw(RotatedDC& dc, int current, DrawLayer layer) const;
	virtual int findItem(const RealPoint& pos, const RealRect& rect, bool tight) const;
  private:
	Direction direction;
	int levels;
	bool rotate;
	DrawLines draw_lines;
	bool label;
};

/// Draws an a vertical axis for counts
class GraphValueAxis : public Graph1D {
  public:
	inline GraphValueAxis(size_t axis, bool highlight_value) : Graph1D(axis), highlight_value(highlight_value) {}
	virtual void draw(RotatedDC& dc, int current, DrawLayer layer) const;
  private:
	bool highlight_value;
};

/// A graph with margins
class GraphWithMargins : public Graph {
  public:
	inline GraphWithMargins(const GraphP& graph,
	                        double margin_left, double margin_top, double margin_right, double margin_bottom,
	                        bool upside_down = false)
		: graph(graph)
		, margin_left(margin_left), margin_top(margin_top), margin_right(margin_right), margin_bottom(margin_bottom)
		, upside_down(upside_down)
	{}
	virtual void draw(RotatedDC& dc, const vector<int>& current, DrawLayer layer) const;
	virtual bool findItem(const RealPoint& pos, const RealRect& rect, bool tight, vector<int>& out) const;
	virtual void setData(const GraphDataP& d);
  private:
	const GraphP graph;
	double margin_left, margin_top, margin_right, margin_bottom;
	bool upside_down; // put the coordinate system upside down, since graphs are usually bottom-to-top
};

/// A display containing multiple graphs
class GraphContainer : public Graph {
  public:
	virtual void draw(RotatedDC& dc, const vector<int>& current, DrawLayer layer) const;
	virtual bool findItem(const RealPoint& pos, const RealRect& rect, bool tight, vector<int>& out) const;
	virtual void setData(const GraphDataP& d);
	
	void add(const GraphP& graph);
  private:
	vector<GraphP> items;
};

// ----------------------------------------------------------------------------- : Graph control

/// A control showing statistics in a graphical form
class GraphControl : public wxControl {
  public:
	/// Create a graph control
	GraphControl(Window* parent, int id);
	
	/// Set the type of graph used, from a number of predefined choices
	void setLayout(GraphType type, bool force_refresh = false);
	/// Update the data in the graph
	void setData(const GraphDataPre& data);
	/// Update the data in the graph
	void setData(const GraphDataP& data);
	/// Retrieve the data in the graph
	GraphDataP getData() const;
	
	/// Is there a selection on the given axis?
	bool hasSelection(size_t axis) const;
	/// Get the current item along the given axis
	String getSelection(size_t axis) const;
	/// Get the current item along each axis
	vector<int> getSelectionIndices() const;
	
	/// Get the current layout
	GraphType getLayout() const;
	/// Get the current dimensionality
	size_t getDimensionality() const;
	
  private:
	/// Graph object
	GraphP graph;
	GraphType layout; /// < The current layout
	/// The selected item per axis, or an empty vector if there is no selection
	/** If the value for an axis is -1, then all groups on that axis are selected */
	vector<int> current_item;
	
	DECLARE_EVENT_TABLE();
	
	void onPaint(wxPaintEvent&);
	void onSize (wxSizeEvent&);
	void onMouseDown(wxMouseEvent& ev);
	void onMotion(wxMouseEvent& ev);
	void onChar(wxKeyEvent& ev);
	
	void onSelectionChange();
};

// ----------------------------------------------------------------------------- : EOF
#endif
