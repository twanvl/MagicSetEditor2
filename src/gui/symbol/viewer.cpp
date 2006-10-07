//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/symbol/viewer.hpp>

DECLARE_TYPEOF_COLLECTION(SymbolPartP);

// ----------------------------------------------------------------------------- : Constructor

SymbolViewer::SymbolViewer(const SymbolP& symbol, double borderRadius)
	: borderRadius(borderRadius)
	, rotation(0, RealRect(0,0,500,500))
{
	setSymbol(symbol);
}

// ----------------------------------------------------------------------------- : Drawing

typedef shared_ptr<wxMemoryDC> MemoryDCP;

// Return a temporary DC with the same size as the parameter
MemoryDCP getTempDC(DC& dc) {
	wxSize s = dc.GetSize();
	Bitmap buffer(s.GetWidth(), s.GetHeight(), 1);
	MemoryDCP newDC(new wxMemoryDC);
	newDC->SelectObject(buffer);
	// On windows 9x it seems that bitmaps are not black by default
	#if !BITMAPS_DEFAULT_BLACK
		newDC->SetPen(*wxTRANSPARENT_PEN);
		newDC->SetBrush(*wxBLACK_BRUSH);
		newDC->DrawRectangle(0, 0, s.GetWidth(), s.GetHeight());
	#endif
	return newDC;
}

// Combine the temporary DCs used in the drawing with the main dc
void combineBuffers(DC& dc, DC* borders, DC* interior) {
	wxSize s = dc.GetSize();
	if (borders)  dc.Blit(0, 0, s.GetWidth(), s.GetHeight(), borders,  0, 0, wxOR);
	if (interior) dc.Blit(0, 0, s.GetWidth(), s.GetHeight(), interior, 0, 0, wxAND_INVERT);
}

void SymbolViewer::draw(DC& dc) {
	bool paintedSomething = false;
	bool buffersFilled    = false;
	// Temporary dcs
	MemoryDCP borderDC;
	MemoryDCP interiorDC;
	// Check if we can paint directly to the dc
	// This will fail if there are parts with combine == intersection
	FOR_EACH(p, symbol->parts) {
		if (p->combine == PART_INTERSECTION) {
			paintedSomething = true;
			break;
		}
	}
	// Draw all parts, in reverse order (bottom to top)
	FOR_EACH_REVERSE(p, symbol->parts) {
		const SymbolPart& part = *p;
		if (part.combine == PART_OVERLAP && buffersFilled) {
			// We will be overlapping some previous parts, write them to the screen
			combineBuffers(dc, borderDC.get(), interiorDC.get());
			// Clear the buffers
			buffersFilled = false;
			paintedSomething = true;
			wxSize s = dc.GetSize();
			if (borderDC) {
				borderDC->SetBrush(*wxBLACK_BRUSH);
				borderDC->SetPen(  *wxTRANSPARENT_PEN);
				borderDC->DrawRectangle(0, 0, s.GetWidth(), s.GetHeight());
			}
			interiorDC->SetBrush(*wxBLACK_BRUSH);
			interiorDC->DrawRectangle(0, 0, s.GetWidth(), s.GetHeight());
		}
		
		if (!paintedSomething) {
			// No need to buffer
			if (!interiorDC) interiorDC = getTempDC(dc);
			combineSymbolPart(part, dc, *interiorDC, true, false);
			buffersFilled = true;
		} else {
			if (!borderDC)    borderDC   = getTempDC(dc);
			if (!interiorDC)  interiorDC = getTempDC(dc);
			// Draw this part to the buffer
			combineSymbolPart(part, *borderDC, *interiorDC, false, false);
			buffersFilled = true;
		}
	}
	
	// Output the final parts from the buffer
	if (buffersFilled) {
		combineBuffers(dc, borderDC.get(), interiorDC.get());
	}
}

void SymbolViewer::highlightPart(DC& dc, const SymbolPart& part, HighlightStyle style) {
	// create point list
	vector<wxPoint> points;
	size_t size = part.points.size();
	for(size_t i = 0 ; i < size ; ++i) {
		segmentSubdivide(*part.getPoint((int)i), *part.getPoint((int)i+1), rotation, points);
	}
	// draw
	if (style == HIGHLIGHT_BORDER) {
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.SetPen  (wxPen(Color(255,0,0), 2));
		dc.DrawPolygon((int)points.size(), &points[0]);
	} else {
		dc.SetLogicalFunction(wxOR);
		dc.SetBrush(Color(0,0,64));
		dc.SetPen  (*wxTRANSPARENT_PEN);
		dc.DrawPolygon((int)points.size(), &points[0]);
		if (part.combine == PART_SUBTRACT || part.combine == PART_BORDER) {
			dc.SetLogicalFunction(wxAND);
			dc.SetBrush(Color(192,192,255));
			dc.DrawPolygon((int)points.size(), &points[0]);
		}
		dc.SetLogicalFunction(wxCOPY);
	}
}


void SymbolViewer::combineSymbolPart(const SymbolPart& part, DC& border, DC& interior, bool directB, bool directI) {
	// what color should the interior be?
	// use black when drawing to the screen
	int interiorCol = directI ? 0 : 255;
	// how to draw depends on combining mode
	switch(part.combine) {
		case PART_OVERLAP:
		case PART_MERGE: {
			drawSymbolPart(part, &border, &interior, 255, interiorCol, directB);
			break;
		} case PART_SUBTRACT: {
			border.SetLogicalFunction(wxAND);
			drawSymbolPart(part, &border, &interior, 0, interiorCol ^ 255, directB);
			border.SetLogicalFunction(wxCOPY);
			break;
		} case PART_INTERSECTION: {
			MemoryDCP keepBorder   = getTempDC(border);
			MemoryDCP keepInterior = getTempDC(interior);
			drawSymbolPart(part, keepBorder.get(), keepInterior.get(), 255, 255, false);
			// combine the temporary dcs with the result using the AND operator
			wxSize s = border.GetSize();
			border  .Blit(0, 0, s.GetWidth(), s.GetHeight(), &*keepBorder,   0, 0, wxAND);
			interior.Blit(0, 0, s.GetWidth(), s.GetHeight(), &*keepInterior, 0, 0, wxAND);
			break;
		} case PART_DIFFERENCE: {
			// TODO
			break;
		} case PART_BORDER: {
			// draw border as interior
			drawSymbolPart(part, nullptr, &border, 0, 255, false);
			break;
		}
	}
}

void SymbolViewer::drawSymbolPart(const SymbolPart& part, DC* border, DC* interior, int borderCol, int interiorCol, bool directB) {
	// create point list
	vector<wxPoint> points;
	size_t size = part.points.size();
	for(size_t i = 0 ; i < size ; ++i) {
		segmentSubdivide(*part.getPoint((int)i), *part.getPoint((int)i+1), rotation, points);
	}
	// draw border
	if (border) {
		if (directB) {
			// white/green
			border->SetBrush(Color(borderCol, min(255,borderCol + 128), borderCol));
		} else {
			// white/black
			border->SetBrush(Color(borderCol, borderCol, borderCol));
		}
		border->SetPen(wxPen(*wxWHITE, rotation.trS(borderRadius)));
		border->DrawPolygon((int)points.size(), &points[0]);
	}
	// draw interior
	if (interior) {
		interior->SetBrush(Color(interiorCol,interiorCol,interiorCol));
		interior->SetPen(*wxTRANSPARENT_PEN);
		interior->DrawPolygon((int)points.size(), &points[0]);
	}
}


/*
void SymbolViewer::calcBezierPoint(const ControlPointP& p0, const ControlPointP& p1, Point*& p_out, UInt count) {
	BezierCurve c(*p0, *p1);
	// add start point
	*p_out = toDisplay(*p0);
	++p_out;
	// recursively calculate rest of curve
	calcBezierOpt(c, *p0, *p1, 0.0f, 1.0f, p_out, count-1);
}


void SymbolViewer::calcBezierOpt(const BezierCurve& c, const Vector2D& p0, const Vector2D& p1, double t0, double t1, Point*& p_out, mutable UInt count) {
	if (count <= 0)  return;
	double midtime = (t0+t1) * 0.5f;
	Vector2D midpoint = c.pointAt(midtime);
	Vector2D d0 = p0 - midpoint;
	Vector2D d1 = midpoint - p1;
	// Determine treshold for subdivision, greater angle -> subdivide
	// greater size -> subdivide
	double treshold = fabs(  atan2(d0.x,d0.y) - atan2(d1.x,d1.y))   * (p0-p1).lengthSqr();
	bool subdivide = treshold >= .0001;
	// subdivide left
	calcBezierOpt(c, p0, midpoint, t0, midtime, p_out, count/2);
	// add midpoint
	if (subdivide) {
		*p_out = toDisplay(midpoint);
		++p_out;
		count -= 1;
	}
	// subdivide right
	calcBezierOpt(c, midpoint, p1, midtime, t1, p_out, count/2);
}
*/