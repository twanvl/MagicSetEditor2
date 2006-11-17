//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/rotation.hpp>
#include <gfx/gfx.hpp>

// ----------------------------------------------------------------------------- : Rotation

// constrain an angle to {0,90,180,270}
int constrain_angle(int angle) {
	int a = ((angle + 45) % 360 + 360) % 360; // [0..360)
	return (a / 90) * 90; // multiple of 90
}

Rotation::Rotation(int angle, const RealRect& rect, double zoom)
	: angle(constrain_angle(angle))
	, size(rect.size)
	, origin(rect.position)
	, zoom(zoom)
{
	// set origin
	if (revX()) origin.x += size.width;
	if (revY()) origin.y += size.height;
}

RealRect Rotation::getExternalRect() const {
	return RealRect(origin - RealSize(revX() ? size.width : 0, revY() ? size.height : 0), size);
}

RealPoint Rotation::tr(const RealPoint& p) const {
	return tr(RealSize(p.x, p.y)) + origin; // TODO : optimize?
}
RealSize Rotation::tr(const RealSize& s) const {
	if (sideways()) {
		return RealSize(negX(s.height), negY(s.width)) * zoom;
	} else {
		return RealSize(negX(s.width), negY(s.height)) * zoom;
	}
}
RealRect Rotation::tr(const RealRect& r) const {
	return RealRect(tr(r.position), tr(r.size));
}

RealSize Rotation::trNoNeg(const RealSize& s) const {
	if (sideways()) {
		return RealSize(s.height, s.width) * zoom;
	} else {
		return RealSize(s.width, s.height) * zoom;
	}
}
RealRect Rotation::trNoNeg(const RealRect& r) const {
	RealSize s = (sideways() ? RealSize(r.size.height, r.size.width) : r.size) * zoom;
	return RealRect(tr(r.position) - RealSize(revX()?s.width:0, revY()?s.height:0), s);
}
RealRect Rotation::trNoNegNoZoom(const RealRect& r) const {
	RealSize s = sideways() ? RealSize(r.size.height, r.size.width) : r.size;
	return RealRect(tr(r.position) - RealSize(revX()?s.width:0, revY()?s.height:0), s);
}


RealSize Rotation::trInv(const RealSize& s) const {
	if (sideways()) {
		return RealSize(negY(s.height), negX(s.width)) / zoom;
	} else {
		return RealSize(negX(s.width), negY(s.height)) / zoom;
	}
}

RealPoint Rotation::trInv(const RealPoint& p) const {
	RealPoint p2 = (p - origin) / zoom;
	if (sideways()) {
		return RealPoint(negY(p2.y), negX(p2.x));
	} else {
		return RealPoint(negX(p2.x), negY(p2.y));
	}
}

// ----------------------------------------------------------------------------- : Rotater

Rotater::Rotater(Rotation& rot, const Rotation& by)
	: old(rot)
	, rot(rot)
{
	// apply rotation
	RealRect new_ext = rot.trNoNeg(by.getExternalRect());
	rot.angle = constrain_angle(rot.angle + by.angle);
	rot.zoom *= by.zoom;
	rot.size   = new_ext.size;
	rot.origin = new_ext.position + RealSize(rot.revX() ? rot.size.width : 0, rot.revY() ? rot.size.height : 0);
}

Rotater::~Rotater() {
	rot = old; // restore
}

// ----------------------------------------------------------------------------- : RotatedDC

RotatedDC::RotatedDC(DC& dc, int angle, const RealRect& rect, double zoom, bool high_quality)
	: Rotation(angle, rect, zoom)
	, dc(dc), high_quality(high_quality)
{}

RotatedDC::RotatedDC(DC& dc, const Rotation& rotation, bool high_quality = false)
	: Rotation(rotation)
	, dc(dc), high_quality(high_quality)
{}

// ----------------------------------------------------------------------------- : RotatedDC : Drawing

void RotatedDC::DrawText  (const String& text, const RealPoint& pos) {
	if (text.empty()) return;
//	if (high_quality) {
/*		RealRect r(p, getTextExtent(text));
		//dc.getTextExtent(text, &r.width, &r.height)
		RealRect r_ext = trNoNeg(r);
		drawResampledText( {
			dc, r_ext.x, r_ext.y, r_ext.width, r_ext.height,
			revX(), revY(),
			angle, text);
		}
	} else {
*/		RealPoint p_ext = tr(pos);
		dc.DrawRotatedText(text, p_ext.x, p_ext.y, angle);
//	}
}

void RotatedDC::DrawBitmap(const Bitmap& bitmap, const RealPoint& pos) {
	if (angle == 0) {
		RealPoint p_ext = tr(pos);
		dc.DrawBitmap(bitmap, p_ext.x, p_ext.y, true);
	} else {
		DrawImage(bitmap.ConvertToImage(), pos);
	}
}
void RotatedDC::DrawImage (const Image& image, const RealPoint& pos, ImageCombine combine) {
	Image rotated = rotate_image(image, angle);
	wxRect r = trNoNegNoZoom(RealRect(pos, RealSize(image.GetWidth(), image.GetHeight())));
	draw_combine_image(dc, r.x, r.y, rotated, combine);
}

void RotatedDC::DrawLine  (const RealPoint& p1,  const RealPoint& p2) {
	wxPoint p1_ext = tr(p1), p2_ext = tr(p2);
	dc.DrawLine(p1_ext.x, p1_ext.y, p2_ext.x, p2_ext.y);
}

void RotatedDC::DrawRectangle(const RealRect& r) {
	wxRect r_ext = trNoNeg(r);
	dc.DrawRectangle(r_ext.x, r_ext.y, r_ext.width, r_ext.height);
}

void RotatedDC::DrawRoundedRectangle(const RealRect& r, double radius) {
	wxRect r_ext = trNoNeg(r);
	dc.DrawRoundedRectangle(r_ext.x, r_ext.y, r_ext.width, r_ext.height, trS(radius));
}

// ----------------------------------------------------------------------------- : Forwarded properties

void RotatedDC::SetPen(const wxPen& pen)              { dc.SetPen(pen); }
void RotatedDC::SetBrush(const wxBrush& brush)        { dc.SetBrush(brush); }
void RotatedDC::SetTextForeground(const Color& color) { dc.SetTextForeground(color); }
void RotatedDC::SetLogicalFunction(int function)      { dc.SetLogicalFunction(function); }

void RotatedDC::SetFont(const wxFont& font)           { SetFont(font, font.GetPointSize()); }
void RotatedDC::SetFont(wxFont font, double size) {
	font.SetPointSize(trS(size));
	dc.SetFont(font);
}

RealSize RotatedDC::GetTextExtent(const String& text) const {
	int w, h;
	dc.GetTextExtent(text, &w, &h);
	return RealSize(w,h) / zoom;
}
double RotatedDC::GetCharHeight() const {
	return dc.GetCharHeight() / zoom;
}
