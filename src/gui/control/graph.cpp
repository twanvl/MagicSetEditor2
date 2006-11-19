//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/control/graph.hpp>
#include <gfx/gfx.hpp>
#include <wx/dcbuffer.h>

DECLARE_TYPEOF_COLLECTION(GraphAxisP);
DECLARE_TYPEOF_COLLECTION(GraphElementP);
DECLARE_TYPEOF_COLLECTION(GraphGroup);
typedef map<String,UInt> map_String_UInt;
DECLARE_TYPEOF(map_String_UInt);

// ----------------------------------------------------------------------------- : GraphData

GraphElement::GraphElement(const String& v1) {
	values.push_back(v1);
}
GraphElement::GraphElement(const String& v1, const String& v2) {
	values.push_back(v1);
	values.push_back(v2);
}


GraphData::GraphData(const GraphDataPre& d)
	: axes(d.axes)
{
	// total size
	size = (UInt)d.elements.size();
	// find groups on each axis
	size_t value_count = 1;
	size_t i = 0;
	FOR_EACH(a, axes) {
		map<String,UInt> counts; // note: default constructor for UInt() does initialize to 0
		FOR_EACH_CONST(e, d.elements) {
			counts[e->values[i]] += 1;
		}
		// TODO: allow some ordering in the groups, and allow colors to be passed
		FOR_EACH(c, counts) {
			a->groups.push_back(GraphGroup(c.first, c.second));
			a->max = max(a->max, c.second);
		}
		// find some nice colors for the groups
		if (a->auto_color) {
			double hue = 0.6; // start hue
			bool first = true;
			FOR_EACH(g, a->groups) {
				double amount = double(g.size) / size; // amount this group takes
				if (!first) hue += amount/2;
				g.color = hsl2rgb(hue, 1.0, 0.5);
				hue += amount / 2;
				first = false;
			}
		}
		value_count *= a->groups.size();
		++i;
	}
	// count elements in each position
	values.clear();
	values.resize(value_count, 0);
	FOR_EACH_CONST(e, d.elements) {
		// find index j in elements
		size_t i = 0, j = 0;
		FOR_EACH(a, axes) {
			String v = e->values[i];
			size_t k = 0, l = 0;
			FOR_EACH(g, a->groups) {
				if (v == g.name) {
					k = l;
					break;
				}
				l += 1;
			}
			j = j * a->groups.size() + k;
			++i;
		}
		values[j] += 1;
	}
}


// ----------------------------------------------------------------------------- : Graph1D

bool Graph1D::findItem(const RealPoint& pos, vector<int>& out) const {
	int i = findItem(pos);
	if (i == -1) return false;
	else {
		out.clear();
		out.insert(out.begin(), data->axes.size(), -1);
		out.at(axis) = i;
		return true;
	}
}

// ----------------------------------------------------------------------------- : Bar Graph

void BarGraph::draw(RotatedDC& dc) const {
	// TODO
}
int BarGraph::findItem(const RealPoint& pos) const {
	return -1; // TODO
}

// ----------------------------------------------------------------------------- : Pie Graph

// ----------------------------------------------------------------------------- : Graph Legend

// ----------------------------------------------------------------------------- : GraphControl

GraphControl::GraphControl(Window* parent, int id)
	: wxControl(parent, id)
{
	graph = new_shared1<BarGraph>(0);
}

void GraphControl::setData(const GraphDataPre& data) {
	setData(new_shared1<GraphData>(data));
}
void GraphControl::setData(const GraphDataP& data) {
	if (graph) {
		graph->setData(data);
		current_item.clear(); // TODO : preserver selection
		Refresh(false);
	}
}

void GraphControl::onPaint(wxPaintEvent&) {
	wxBufferedPaintDC dc(this);
	wxSize cs = GetClientSize();
	RotatedDC rdc(dc, 0, RealRect(RealPoint(0,0),cs), 1, false);
	rdc.SetPen(*wxTRANSPARENT_PEN);
	rdc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	rdc.DrawRectangle(rdc.getInternalRect());
	if (graph) graph->draw(rdc);
}

void GraphControl::onSize(wxSizeEvent&) {
	Refresh(false);
}

void GraphControl::onMouseDown(wxMouseEvent& ev) {
	if (!graph) return;
	wxSize cs = GetClientSize();
	graph->findItem(RealPoint((double)ev.GetX()/cs.GetWidth(), (double)ev.GetY()/cs.GetHeight()), current_item);
}

BEGIN_EVENT_TABLE(GraphControl, wxControl)
	EVT_PAINT		(GraphControl::onPaint)
	EVT_SIZE		(GraphControl::onSize)
	EVT_LEFT_DOWN	(GraphControl::onMouseDown)
END_EVENT_TABLE  ()
