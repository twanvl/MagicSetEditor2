//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/symbol/viewer.hpp>
#include <util/error.hpp> // clearDC_black
#include <gui/util.hpp> // clearDC_black

DECLARE_TYPEOF_COLLECTION(SymbolPartP);

// ----------------------------------------------------------------------------- : Simple rendering

Image render_symbol(const SymbolP& symbol, double border_radius, int width, int height, bool editing_hints, bool allow_smaller) {
	SymbolViewer viewer(symbol, editing_hints, width, border_radius);
	// limit width/height ratio to aspect ratio of symbol
	double ar  = symbol->aspectRatio();
	double par = (double)width/height;
	if (par > ar && (ar > 1 || (allow_smaller && height < width))) {
		width  = int(height * ar);
	} else if (par < ar && (ar < 1 || (allow_smaller && width < height))) {
		height = int(width / ar);
	}
	if (width > height) {
		viewer.setZoom(width);
		viewer.setOrigin(Vector2D(0,-(width-height) * 0.5));
		viewer.border_radius *= (double)height / width;
	} else {
		viewer.setZoom(height);
		viewer.setOrigin(Vector2D(-(height-width) * 0.5,0));
		viewer.border_radius *= (double)width / height;
	}
	Bitmap bmp(width, height);
	wxMemoryDC dc;
	dc.SelectObject(bmp);
	clearDC(dc, Color(0,128,0));
	viewer.draw(dc);
	dc.SelectObject(wxNullBitmap);
	return bmp.ConvertToImage();
}

// ----------------------------------------------------------------------------- : Constructor

SymbolViewer::SymbolViewer(const SymbolP& symbol, bool editing_hints, double size, double border_radius)
	: border_radius(border_radius), editing_hints(editing_hints)
	, rotation(0, RealRect(0,0,size,size), size)
	, multiply(size,0,0,size)
	, origin(0,0)
	, in_symmetry(0)
{
	setSymbol(symbol);
}

void SymbolViewer::setZoom(double zoom) {
	rotation.setZoom(zoom);
	rotation.setOrigin(zoom * origin);
	multiply = Matrix2D(zoom,0, 0,zoom);
}
void SymbolViewer::setOrigin(const Vector2D& origin) {
	this->origin = origin;
	rotation.setOrigin(origin);
}

// ----------------------------------------------------------------------------- : Drawing : Combining

typedef shared_ptr<wxMemoryDC> MemoryDCP;

// Return a temporary DC with the same size as the parameter
MemoryDCP getTempDC(DC& dc) {
	wxSize s = dc.GetSize();
	#ifdef __WXMSW__
		Bitmap buffer(s.GetWidth(), s.GetHeight(), 1);
	#else
		Bitmap buffer(s.GetWidth(), s.GetHeight(), 24);
	#endif
	MemoryDCP newDC(new wxMemoryDC);
	newDC->SelectObject(buffer);
	clearDC(*newDC, *wxBLACK_BRUSH);
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
	in_symmetry = 0;
	// Temporary dcs
	MemoryDCP borderDC;
	MemoryDCP interiorDC;
	// Check if we can paint directly to the dc
	// This will fail if there are parts with combine == intersection
	FOR_EACH(p, symbol->parts) {
		if (SymbolShape* s = p->isSymbolShape()) {
			if (s->combine == SYMBOL_COMBINE_INTERSECTION) {
				paintedSomething = true;
				break;
			}
		}
	}
	// Draw all parts
	combineSymbolPart(dc, *symbol, paintedSomething, buffersFilled, true, borderDC, interiorDC);
	// Output the final parts from the buffer
	if (buffersFilled) {
		combineBuffers(dc, borderDC.get(), interiorDC.get());
	}
	// Editing hints?
	if (editing_hints) {
		drawEditingHints(dc);
	}
}
void SymbolViewer::combineSymbolPart(DC& dc, const SymbolPart& part, bool& paintedSomething, bool& buffersFilled, bool allow_overlap, MemoryDCP& borderDC, MemoryDCP& interiorDC) {
	if (const SymbolShape* s = part.isSymbolShape()) {
		if (s->combine == SYMBOL_COMBINE_OVERLAP && buffersFilled && allow_overlap) {
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
		
		// Paint the part itself
		if (!paintedSomething) {
			// No need to buffer
			if (!interiorDC) interiorDC = getTempDC(dc);
			combineSymbolShape(*s, dc, *interiorDC, true, false);
			buffersFilled = true;
		} else {
			if (!borderDC)   borderDC   = getTempDC(dc);
			if (!interiorDC) interiorDC = getTempDC(dc);
			// Draw this shape to the buffer
			combineSymbolShape(*s, *borderDC, *interiorDC, false, false);
			buffersFilled = true;
		}
	} else if (const SymbolSymmetry* s = part.isSymbolSymmetry()) {
		// Draw all parts, in reverse order (bottom to top), also draw rotated copies
		Radians b = 2 * s->handle.angle();
		Matrix2D old_m = multiply;
		Vector2D old_o = origin;
		int copies = s->kind == SYMMETRY_REFLECTION ? s->copies / 2 * 2 : s->copies;
		FOR_EACH_CONST_REVERSE(p, s->parts) {
			if (copies > 1) ++in_symmetry;
			for (int i = copies - 1 ; i >= 0 ; --i) {
				if (i == 0) --in_symmetry;
				if (s->clip) {
					// todo: clip
				}
				double a = i * 2 * M_PI / copies;
				if (s->kind == SYMMETRY_ROTATION || i % 2 == 0) {
					// set matrix
					// Calling:
					//  - p  the input point
					//  - p' the output point
					//  - rot our rotation matrix
					//  - d   out origin
					//  - o   the current origin (old_o)
					//  - m   the current matrix (old_m)
					// We want:
					//   p' = ((p - d) * rot + d) * m + o
					//      =  (p * rot - d * rot + d) * m + o
					//      =  p * rot * m + (d - d * rot) * m + o
					Matrix2D rot(cos(a),-sin(a), sin(a),cos(a));
					multiply = rot * old_m;
					origin = old_o + (s->center - s->center * rot) * old_m;
				} else {
					// reflection
					//  Calling angle = b
					// Matrix2D ref(cos(b),sin(b), sin(b),-cos(b));
					// Matrix2D rot(cos(a),-sin(a), sin(a),cos(a));
					// 
					//  ref * rot
					//    [ cos b   sin b !  [ cos a  -sin a !
					//  = ! sin b  -cos b ]  ! sin a   cos a ]
					//  = [ cos(a+b)  sin(a+b) !
					//    ! sin(a+b) -cos(a+b) ]
					Matrix2D rot(cos(a+b),sin(a+b), sin(a+b),-cos(a+b));
					multiply = rot * old_m;
					origin = old_o + (s->center - s->center * rot) * old_m;
				}
				// draw rotated copy
				combineSymbolPart(dc, *p, paintedSomething, buffersFilled, allow_overlap && i == copies - 1, borderDC, interiorDC);
			}
		}
		multiply = old_m;
		origin   = old_o;
		if (editing_hints) {
			highlightPart(dc, *s, HIGHLIGHT_LESS);
		}
	} else if (const SymbolGroup* g = part.isSymbolGroup()) {
		// Draw all parts, in reverse order (bottom to top)
		FOR_EACH_CONST_REVERSE(p, g->parts) {
			combineSymbolPart(dc, *p, paintedSomething, buffersFilled, allow_overlap, borderDC, interiorDC);
		}
	}
}


void SymbolViewer::combineSymbolShape(const SymbolShape& shape, DC& border, DC& interior, bool directB, bool directI) {
	// what color should the interior be?
	// use black when drawing to the screen
	Byte interiorCol = directI ? 0 : 255;
	if (editing_hints && in_symmetry) {
		interiorCol = directI ? 16 : 240;
	}
	// how to draw depends on combining mode
	switch(shape.combine) {
		case SYMBOL_COMBINE_OVERLAP:
		case SYMBOL_COMBINE_MERGE: {
			drawSymbolShape(shape, &border, &interior, 255, interiorCol, directB, false);
			break;
		} case SYMBOL_COMBINE_SUBTRACT: {
			border.SetLogicalFunction(wxAND);
			drawSymbolShape(shape, &border, &interior, 0, ~interiorCol, directB, false);
			border.SetLogicalFunction(wxCOPY);
			break;
		} case SYMBOL_COMBINE_INTERSECTION: {
			MemoryDCP keepBorder   = getTempDC(border);
			MemoryDCP keepInterior = getTempDC(interior);
			drawSymbolShape(shape, keepBorder.get(), keepInterior.get(), 255, 255, false, false);
			// combine the temporary dcs with the result using the AND operator
			wxSize s = border.GetSize();
			border  .Blit(0, 0, s.GetWidth(), s.GetHeight(), &*keepBorder  , 0, 0, wxAND);
			interior.Blit(0, 0, s.GetWidth(), s.GetHeight(), &*keepInterior, 0, 0, wxAND);
			break;
		} case SYMBOL_COMBINE_DIFFERENCE: {
			interior.SetLogicalFunction(wxXOR);
			drawSymbolShape(shape, &border, &interior, 0, interiorCol, directB, true);
			interior.SetLogicalFunction(wxCOPY);
			break;
		} case SYMBOL_COMBINE_BORDER: {
			// draw border as interior
			drawSymbolShape(shape, nullptr, &border, 0, 255, false, false);
			break;
		}
	}
}


// ----------------------------------------------------------------------------- : Drawing : Basic


void SymbolViewer::drawSymbolShape(const SymbolShape& shape, DC* border, DC* interior, Byte borderCol, Byte interiorCol, bool directB, bool clear) {
	// create point list
	vector<wxPoint> points;
	size_t size = shape.points.size();
	for(size_t i = 0 ; i < size ; ++i) {
		segment_subdivide(*shape.getPoint((int)i), *shape.getPoint((int)i+1), origin, multiply, points);
	}
	// draw border
	if (border && border_radius > 0) {
		// white/black or, if directB white/green
		border->SetBrush(Color(borderCol, (directB && borderCol == 0 ? 128 : borderCol), borderCol));
		border->SetPen(wxPen(*wxWHITE, (int) rotation.trS(border_radius)));
		border->DrawPolygon((int)points.size(), &points[0]);

		if (clear) {
			border->SetPen(*wxTRANSPARENT_PEN);
			border->SetBrush(Color(0, (directB ? 128 : 0), 0));

			int func = border->GetLogicalFunction();
			border->SetLogicalFunction(wxCOPY);
			border->DrawPolygon((int)points.size(), &points[0]);
			border->SetLogicalFunction(func);
		}
	}
	// draw interior
	if (interior) {
		interior->SetBrush(Color(interiorCol,interiorCol,interiorCol));
		interior->SetPen(*wxTRANSPARENT_PEN);
		interior->DrawPolygon((int)points.size(), &points[0]);
	}
}

// ----------------------------------------------------------------------------- : Drawing : Highlighting

void SymbolViewer::highlightPart(DC& dc, const SymbolPart& part, HighlightStyle style) {
	if (const SymbolShape* s = part.isSymbolShape()) {
		highlightPart(dc, *s, style);
	} else if (const SymbolSymmetry* s = part.isSymbolSymmetry()) {
		highlightPart(dc, *s, style);
	} else if (const SymbolGroup* g = part.isSymbolGroup()) {
		highlightPart(dc, *g, style);
	} else {
		throw InternalError(_("Invalid symbol part type"));
	}
}

void SymbolViewer::highlightPart(DC& dc, const SymbolShape& shape, HighlightStyle style) {
	if (style == HIGHLIGHT_LESS) return;
	// create point list
	vector<wxPoint> points;
	size_t size = shape.points.size();
	for(size_t i = 0 ; i < size ; ++i) {
		segment_subdivide(*shape.getPoint((int)i), *shape.getPoint((int)i+1), origin, multiply, points);
	}
	// draw
	if (style == HIGHLIGHT_BORDER) {
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.SetPen  (wxPen(Color(255,0,0), 2));
		dc.DrawPolygon((int)points.size(), &points[0]);
	} else if (style == HIGHLIGHT_BORDER_DOT) {
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.SetPen  (wxPen(Color(255,0,0), 1, wxDOT));
		dc.DrawPolygon((int)points.size(), &points[0]);
	} else {
		dc.SetLogicalFunction(wxOR);
		dc.SetBrush(Color(0,0,64));
		dc.SetPen  (*wxTRANSPARENT_PEN);
		dc.DrawPolygon((int)points.size(), &points[0]);
		if (shape.combine == SYMBOL_COMBINE_SUBTRACT || shape.combine == SYMBOL_COMBINE_BORDER) {
			dc.SetLogicalFunction(wxAND);
			dc.SetBrush(Color(191,191,255));
			dc.DrawPolygon((int)points.size(), &points[0]);
		}
		dc.SetLogicalFunction(wxCOPY);
	}
}

void SymbolViewer::highlightPart(DC& dc, const SymbolSymmetry& sym, HighlightStyle style) {
	// highlight parts?
	FOR_EACH_CONST(part, sym.parts) {
		highlightPart(dc, *part, (HighlightStyle)(style | HIGHLIGHT_LESS));
	}
	// Color?
	Color color = style & HIGHLIGHT_BORDER   ? Color(255,100,0)
	            : style & HIGHLIGHT_INTERIOR ? Color(255,200,0)
	            : Color(200,170,0);
	// center
	RealPoint center = rotation.tr(sym.center);
	// draw 'spokes'
	Radians angle = atan2(sym.handle.y, sym.handle.x);
	dc.SetPen(wxPen(color, sym.kind == SYMMETRY_ROTATION ? 1 : 3));
	int copies = sym.kind == SYMMETRY_REFLECTION ? sym.copies / 2 * 2 : sym.copies;
	for (int i = 0; i < copies ; ++i) {
		Radians a = angle + (i + 0.5) * 2 * M_PI / copies;
		Vector2D dir(cos(a), sin(a));
		Vector2D dir2 = rotation.tr(sym.center + 2 * dir);
		dc.DrawLine(int(center.x), int(center.y), int(dir2.x), int(dir2.y));
	}
	// draw center
	dc.SetPen(*wxBLACK_PEN);
	dc.SetBrush(color);
	dc.DrawCircle(int(center.x), int(center.y), sym.kind == SYMMETRY_ROTATION ? 7 : 5);
}

void SymbolViewer::highlightPart(DC& dc, const SymbolGroup& group, HighlightStyle style) {
	if (style == HIGHLIGHT_BORDER) {
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.SetPen  (wxPen(Color(255,0,0), 2));
		dc.DrawRectangle(rotation.trRectToBB(RealRect(group.bounds)));
	}
	FOR_EACH_CONST(part, group.parts) {
		highlightPart(dc, *part, (HighlightStyle)(style | HIGHLIGHT_LESS));
	}
}


void SymbolViewer::drawEditingHints(DC& dc) {
	// TODO?
}
