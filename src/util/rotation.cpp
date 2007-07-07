//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/rotation.hpp>
#include <gfx/gfx.hpp>
#include <data/font.hpp>

// ----------------------------------------------------------------------------- : Rotation

// constrain an angle to {0,90,180,270}
int constrain_angle(int angle) {
	int a = ((angle + 45) % 360 + 360) % 360; // [0..360)
	return (a / 90) * 90; // multiple of 90
}

Rotation::Rotation(int angle, const RealRect& rect, double zoom, double strectch, bool is_internal)
	: angle(constrain_angle(angle))
	, size(rect.size())
	, origin(rect.position())
	, zoomX(zoom * strectch)
	, zoomY(zoom)
{
	if (is_internal) {
		size = trNoNeg(size);
	}
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
		return RealSize(negX(s.height) * zoomY, negY(s.width) * zoomX);
	} else {
		return RealSize(negX(s.width) * zoomX, negY(s.height) * zoomY);
	}
}
RealRect Rotation::tr(const RealRect& r) const {
	return RealRect(tr(r.position()), tr(r.size()));
}

RealSize Rotation::trNoNeg(const RealSize& s) const {
	if (sideways()) {
		return RealSize(s.height * zoomY, s.width * zoomX);
	} else {
		return RealSize(s.width * zoomX, s.height * zoomY);
	}
}
RealRect Rotation::trNoNeg(const RealRect& r) const {
	RealSize s = trNoNeg(r.size());
	return RealRect(tr(r.position()) - RealSize(revX()?s.width:0, revY()?s.height:0), s);
}
RealRect Rotation::trNoNegNoZoom(const RealRect& r) const {
	RealSize s = sideways() ? RealSize(r.height, r.width) : r.size();
	return RealRect(tr(r.position()) - RealSize(revX()?s.width:0, revY()?s.height:0), s);
}


RealSize Rotation::trInv(const RealSize& s) const {
	if (sideways()) {
		return RealSize(negY(s.height) / zoomY, negX(s.width) / zoomX);
	} else {
		return RealSize(negX(s.width) / zoomX, negY(s.height) / zoomY);
	}
}

RealPoint Rotation::trInv(const RealPoint& p) const {
	RealPoint p2((p.x - origin.x) / zoomX, (p.y - origin.y) / zoomY);
	if (sideways()) {
		return RealPoint(negY(p2.y), negX(p2.x));
	} else {
		return RealPoint(negX(p2.x), negY(p2.y));
	}
}

RealSize Rotation::trInvNoNeg(const RealSize& s) const {
	if (sideways()) {
		return RealSize(s.height / zoomY, s.width / zoomX);
	} else {
		return RealSize(s.width / zoomX, s.height / zoomY);
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
	if (by.sideways()) {
		rot.zoomX *= by.zoomY;
		rot.zoomY *= by.zoomX;
	} else {
		rot.zoomX *= by.zoomX;
		rot.zoomY *= by.zoomY;
	}
	rot.size   = new_ext.size();
	rot.origin = new_ext.position() + RealSize(rot.revX() ? rot.size.width : 0, rot.revY() ? rot.size.height : 0);
}

Rotater::~Rotater() {
	rot = old; // restore
}

// ----------------------------------------------------------------------------- : RotatedDC

RotatedDC::RotatedDC(DC& dc, int angle, const RealRect& rect, double zoom, RenderQuality quality, bool is_internal)
	: Rotation(angle, rect, zoom, 1.0, is_internal)
	, dc(dc), quality(quality)
{}

RotatedDC::RotatedDC(DC& dc, const Rotation& rotation, RenderQuality quality)
	: Rotation(rotation)
	, dc(dc), quality(quality)
{}

// ----------------------------------------------------------------------------- : RotatedDC : Drawing

void RotatedDC::DrawText  (const String& text, const RealPoint& pos, int blur_radius, int boldness) {
	if (text.empty()) return;
	if (quality == QUALITY_AA) {
		RealRect r(pos, GetTextExtent(text));
		RealRect r_ext = trNoNeg(r);
		draw_resampled_text(dc, r_ext, stretch(), revX(), revY(), angle, text, blur_radius, boldness);
	} else if (quality == QUALITY_SUB_PIXEL) {
		RealPoint p_ext = tr(pos)*text_scaling;
		double usx,usy;
		dc.GetUserScale(&usx, &usy);
		dc.SetUserScale(usx/text_scaling, usy/text_scaling);
		dc.DrawRotatedText(text, (int) p_ext.x, (int) p_ext.y, angle);
		dc.SetUserScale(usx, usy);
	} else {
		RealPoint p_ext = tr(pos);
		dc.DrawRotatedText(text, (int) p_ext.x, (int) p_ext.y, angle);
	}
}

void RotatedDC::DrawBitmap(const Bitmap& bitmap, const RealPoint& pos) {
	if (angle == 0) {
		RealPoint p_ext = tr(pos);
		dc.DrawBitmap(bitmap, (int) p_ext.x, (int) p_ext.y, true);
	} else {
		DrawImage(bitmap.ConvertToImage(), pos);
	}
}
void RotatedDC::DrawImage (const Image& image, const RealPoint& pos, ImageCombine combine, int angle) {
	Image rotated = rotate_image(image, angle + this->angle);
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

void RotatedDC::DrawCircle(const RealPoint& center, double radius) {
	wxPoint p = tr(center);
	dc.DrawCircle(p.x + 1, p.y + 1, trS(radius));
}

/// Convert radians to degrees
double rad_to_deg(double rad) { return  rad * (180.0 / M_PI); }
/// Convert degrees to radians
double deg_to_rad(double deg) { return  deg * (M_PI / 180.0); }

void RotatedDC::DrawEllipticArc(const RealPoint& center, const RealSize& size, double start, double end) {
	wxPoint c_ext = tr(center - size/2);
	wxSize  s_ext = trNoNeg(size);
	dc.DrawEllipticArc(c_ext.x, c_ext.y, s_ext.x, s_ext.y, rad_to_deg(start) + angle, rad_to_deg(end) + angle);
}
void RotatedDC::DrawEllipticSpoke(const RealPoint& center, const RealSize& size, double angle) {
	wxPoint c_ext = tr(center - size/2);
	wxSize  s_ext = trNoNeg(size);
	double rot_angle = angle + deg_to_rad(this->angle);
	double sin_angle = sin(rot_angle), cos_angle = cos(rot_angle);
	// position of center and of point on the boundary can vary because of rounding errors,
	// this code matches DrawEllipticArc (at least on windows xp).
	dc.DrawLine(
		c_ext.x + int(       0.5 * (s_ext.x + cos_angle) ), // center
		c_ext.y + int(       0.5 * (s_ext.y - sin_angle) ),
		c_ext.x + int( 0.5 + 0.5 * (s_ext.x-1) * (1 + cos_angle) ), // boundary
		c_ext.y + int( 0.5 + 0.5 * (s_ext.y-1) * (1 - sin_angle) )
	);
}

// ----------------------------------------------------------------------------- : Forwarded properties

void RotatedDC::SetPen(const wxPen& pen)              { dc.SetPen(pen); }
void RotatedDC::SetBrush(const wxBrush& brush)        { dc.SetBrush(brush); }
void RotatedDC::SetTextForeground(const Color& color) { dc.SetTextForeground(color); }
void RotatedDC::SetLogicalFunction(int function)      { dc.SetLogicalFunction(function); }

void RotatedDC::SetFont(const wxFont& font) {
	if (quality == QUALITY_LOW && zoomX == 1 && zoomY == 1) {
		dc.SetFont(font);
	} else {
		wxFont scaled = font;
		if (quality == QUALITY_LOW) {
			scaled.SetPointSize((int)  trY(font.GetPointSize()));
		} else {
			scaled.SetPointSize((int) (trY(font.GetPointSize()) * text_scaling));
		}
		dc.SetFont(scaled);
	}
}
void RotatedDC::SetFont(const Font& font, double scale) {
	dc.SetFont(font.toWxFont(trS(scale) * (quality == QUALITY_LOW ? 1 : text_scaling)));
}

double RotatedDC::getFontSizeStep() const {
	if (quality == QUALITY_LOW) {
		return 1;
	} else {
		return 1. / text_scaling;
	}
}

RealSize RotatedDC::GetTextExtent(const String& text) const {
	int w, h;
	dc.GetTextExtent(text, &w, &h);
	#ifdef __WXGTK__
		// HACK: Some fonts don't get the descender height set correctly.
		int charHeight = dc.GetCharHeight();
		if (charHeight != h)
			h += h - charHeight;
	#endif
	if (quality == QUALITY_LOW) {
		return RealSize(w / zoomX, h / zoomY);
	} else {
		return RealSize(w / (zoomX * text_scaling), h / (zoomY * text_scaling));
	}
}
double RotatedDC::GetCharHeight() const {
	int h = dc.GetCharHeight();
	#ifdef __WXGTK__
		// See above HACK
		int extent;
		dc.GetTextExtent(_("H"), 0, &extent);
		if (h != extent)
			h = 2 * extent - h;
	#endif
	if (quality == QUALITY_LOW) {
		return h / zoomY;
	} else {
		return h / (zoomY * text_scaling);
	}
}

void RotatedDC::SetClippingRegion(const RealRect& rect) {
	dc.SetClippingRegion(trNoNeg(rect));
}
void RotatedDC::DestroyClippingRegion() {
	dc.DestroyClippingRegion();
}
