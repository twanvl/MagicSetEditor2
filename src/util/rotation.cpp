//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/rotation.hpp>
#include <gfx/gfx.hpp>
#include <data/font.hpp>

// ----------------------------------------------------------------------------- : Rotation

// constrain an angle to [0..360)
int constrain_angle(int angle) {
	return (angle + 3600) % 360;
}

Rotation::Rotation(int angle, const RealRect& rect, double zoom, double stretch, RotationFlags flags)
	: angle(constrain_angle(angle))
	, size(rect.size())
	, origin(rect.position())
	, zoomX(zoom * stretch)
	, zoomY(zoom)
{
	if (stretch != 1.0) {
		size.width /= stretch;
	}
	// set origin
	if (flags & ROTATION_ATTACH_TOP_LEFT) {
		if (revX()) origin.x += zoom * (sideways() ? size.height : size.width);
		if (revY()) origin.y += zoom * (sideways() ? size.width : size.height);
	}
}

void Rotation::setStretch(double s) {
	size.width *= s * getStretch();
	zoomX = zoomY * s;
}


RealPoint Rotation::tr(const RealPoint& p) const {
	double a = deg_to_rad(angle), s = sin(a), c = cos(a);
	double x = p.x * zoomX, y = p.y * zoomY;
	return RealPoint(c * x + s * y + origin.x,
	                -s * x + c * y + origin.y);
}
RealPoint Rotation::trPixel(const RealPoint& p) const {
	double a = deg_to_rad(angle), s = sin(a), c = cos(a);
	double x = p.x * zoomX + 0.5, y = p.y * zoomY + 0.5;
	return RealPoint(c * x + s * y + origin.x - 0.5,
	                -s * x + c * y + origin.y - 0.5);
}
RealPoint Rotation::trNoZoom(const RealPoint& p) const {
	double a = deg_to_rad(angle), s = sin(a), c = cos(a);
	double x = p.x, y = p.y;
	return RealPoint(c * x + s * y + origin.x,
	                -s * x + c * y + origin.y);
}
RealPoint Rotation::trPixelNoZoom(const RealPoint& p) const {
	double a = deg_to_rad(angle), s = sin(a), c = cos(a);
	double x = p.x + 0.5, y = p.y + 0.5;
	return RealPoint(c * x + s * y + origin.x - 0.5,
	                -s * x + c * y + origin.y - 0.5);
}
/*
RealSize Rotation::trSize(const RealSize& size) const {
	double a = deg_to_rad(angle), s = sin(a), c = cos(a);
	double x = size.width * zoomX, y = size.height * zoomY;
	return RealSize(c * x + s * y, s * x + c * y);
}
*/
RealSize Rotation::trSizeToBB(const RealSize& size) const {
	if (straight()) {
		if (sideways()) {
			return RealSize(size.height * zoomY, size.width * zoomX);
		} else {
			return RealSize(size.width * zoomX, size.height * zoomY);
		}
	} else {
		double a = deg_to_rad(angle), s = sin(a), c = cos(a);
		double x = size.width * zoomX, y = size.height * zoomY;
		return RealSize(fabs(c * x) + fabs(s * y), fabs(s * x) + fabs(c * y));
	}
}
RealRect Rotation::trRectToBB(const RealRect& r) const {
	if (straight()) {
		RealSize s = trSizeToBB(r.size());
		return RealRect(tr(r.position()) - RealSize(revX()?s.width:0, revY()?s.height:0), s);
	} else {
		double a = deg_to_rad(angle), s = sin(a), c = cos(a);
		double x = r.x     * zoomX, y = r.y      * zoomY;
		double w = r.width * zoomX, h = r.height * zoomY;
		RealRect result(c * x + s * y + origin.x,
		               -s * x + c * y + origin.y,
		               0,0);
		if (c > 0) {
			result.width  += c * w;
			result.height += c * h;
		} else {
			result.x      += c * w;
			result.width  -= c * w;
			result.y      += c * h;
			result.height -= c * h;
		}
		if (s > 0) {
			result.width  += s * h;
			result.y      -= s * w;
			result.height += s * w;
		} else {
			result.x      += s * h;
			result.width  -= s * h;
			result.height -= s * w;
		}
		return result;
	}
}
RealRect Rotation::trRectStraight(const RealRect& r) const {
	assert(angle == 0);
	return RealRect(r.position() + origin, r.size());
}
wxRegion Rotation::trRectToRegion(const RealRect& r) const {
	if (straight()) {
		return trRectToBB(r).toRect();
	} else {
		wxPoint points[4] = {trPixel(RealPoint(r.left(),  r.top()   ))
		                    ,trPixel(RealPoint(r.left(),  r.bottom()))
		                    ,trPixel(RealPoint(r.right(), r.bottom()))
		                    ,trPixel(RealPoint(r.right(), r.top()   ))};
		return wxRegion(4,points);
	}
}

RealPoint Rotation::trInv(const RealPoint& p) const {
	double a = deg_to_rad(angle), s = sin(a), c = cos(a);
	double x = p.x - origin.x, y = p.y - origin.y;
	return RealPoint((c * x - s * y) / zoomX,
	                 (s * x + c * y) / zoomY);
}

RealPoint Rotation::boundingBoxCorner(const RealSize& size) const {
	// This function is a bit tricky,
	// I derived it by drawing the four cases.
	// Two succeeding cases must agree where they overlap (0,90,180,270 degrees)
	double a = deg_to_rad(angle), s = sin(a), c = cos(a);
	double w = size.width * zoomX, h = size.height * zoomY;
	if (angle <= 90)  return RealPoint(0,            -w * s);
	if (angle <= 180) return RealPoint(w * c,         h * c - w * s);
	if (angle <= 270) return RealPoint(w * c + h * s, h * c);
	else              return RealPoint(h * s,         0);
}
/*
RealPoint Rotation::boundingBoxCorner(const RealSize& size) const {
	//if(true)return RealPoint(0,0);
	// This function is a bit tricky,
	// I derived it by drawing the four cases.
	// Two succeeding cases must agree where they overlap (0,90,180,270 degrees)
	double a = deg_to_rad(angle), s = sin(a), c = cos(a);
	if (angle <= 90)  return RealPoint(           + size.width  * s * s,             - size.width  * s * c);
	if (angle <= 180) return RealPoint(size.width - size.height * s * c,               size.height * c * c);
	if (angle <= 270) return RealPoint(size.width - size.width  * s * s, size.height + size.width  * s * c);
	else              return RealPoint(             size.height * s * c, size.height - size.height * c * c);
}
*/

// ----------------------------------------------------------------------------- : Rotater

Rotater::Rotater(Rotation& rot, const Rotation& by)
	: old(rot)
	, rot(rot)
{
	// apply rotation
	rot.origin = rot.tr(by.origin);
	rot.size   = by.size;
	rot.angle  = constrain_angle(rot.angle + by.angle);
	// zooming is not really correct if rot.zoomX != rot.zoomY
	rot.zoomX *= by.zoomX;
	rot.zoomY *= by.zoomY;
}

Rotater::~Rotater() {
	rot = old; // restore
}

// ----------------------------------------------------------------------------- : RotatedDC

RotatedDC::RotatedDC(DC& dc, int angle, const RealRect& rect, double zoom, RenderQuality quality, RotationFlags flags)
	: Rotation(angle, rect, zoom, 1.0, flags)
	, dc(dc), quality(quality)
{}

RotatedDC::RotatedDC(DC& dc, const Rotation& rotation, RenderQuality quality)
	: Rotation(rotation)
	, dc(dc), quality(quality)
{}

// ----------------------------------------------------------------------------- : RotatedDC : Drawing

void RotatedDC::DrawText  (const String& text, const RealPoint& pos, int blur_radius, int boldness, double stretch_) {
	DrawText(text, pos, dc.GetTextForeground(), blur_radius, boldness, stretch_);
}

void RotatedDC::DrawText  (const String& text, const RealPoint& pos, AColor color, int blur_radius, int boldness, double stretch_) {
	if (text.empty()) return;
	if (color.alpha == 0) return;
	if (quality >= QUALITY_AA) {
		RealRect r(pos, GetTextExtent(text));
		RealRect r_ext = trRectToBB(r);
		RealPoint pos2 = tr(pos);
		if (zoomX != zoomY) {
			r.width *= zoomX / zoomY;
			RealRect r_ext2 = trRectToBB(r);
			pos2.x += r_ext2.x - r_ext.x;
			pos2.y += r_ext2.y - r_ext.y;
			r_ext.x = r_ext2.x;
			r_ext.y = r_ext2.y;
		}
		draw_resampled_text(dc, pos2, r_ext, stretch_ * getStretch(), angle, color, text, blur_radius, boldness);
	} else if (quality >= QUALITY_SUB_PIXEL) {
		RealPoint p_ext = tr(pos)*text_scaling;
		double usx,usy;
		dc.GetUserScale(&usx, &usy);
		dc.SetUserScale(usx/text_scaling, usy/text_scaling);
		dc.SetTextForeground(color);
		dc.DrawRotatedText(text, (int) p_ext.x, (int) p_ext.y, angle);
		dc.SetUserScale(usx, usy);
	} else {
		RealPoint p_ext = tr(pos);
		dc.SetTextForeground(color);
		dc.DrawRotatedText(text, (int) p_ext.x, (int) p_ext.y, angle);
	}
}

void RotatedDC::DrawTextWithShadow(const String& text, const Font& font, const RealPoint& pos, double scale, double stretch) {
	DrawText(text, pos + font.shadow_displacement * scale, font.shadow_color, font.shadow_blur * scale, 1, stretch);
	DrawText(text, pos, font.color, 0, 1, stretch);
}

void RotatedDC::DrawBitmap(const Bitmap& bitmap, const RealPoint& pos) {
	if (angle == 0) {
		RealPoint p_ext = tr(pos);
		dc.DrawBitmap(bitmap, to_int(p_ext.x), to_int(p_ext.y), true);
	} else {
		DrawImage(bitmap.ConvertToImage(), pos);
	}
}
void RotatedDC::DrawImage(const Image& image, const RealPoint& pos, ImageCombine combine) {
	Image rotated = rotate_image(image, angle);
	DrawPreRotatedImage(rotated, RealRect(pos,trInvS(RealSize(image))), combine);
}
void RotatedDC::DrawPreRotatedBitmap(const Bitmap& bitmap, const RealRect& rect) {
	RealPoint p_ext = tr(rect.position()) + boundingBoxCorner(rect.size());
	dc.DrawBitmap(bitmap, to_int(p_ext.x), to_int(p_ext.y), true);
}
void RotatedDC::DrawPreRotatedImage (const Image& image, const RealRect& rect, ImageCombine combine) {
	RealPoint p_ext = tr(rect.position()) + boundingBoxCorner(rect.size());
	draw_combine_image(dc, to_int(p_ext.x), to_int(p_ext.y), image, combine);
}

void RotatedDC::DrawLine  (const RealPoint& p1,  const RealPoint& p2) {
	wxPoint p1_ext = tr(p1), p2_ext = tr(p2);
	dc.DrawLine(p1_ext.x, p1_ext.y, p2_ext.x, p2_ext.y);
}

void RotatedDC::DrawRectangle(const RealRect& r) {
	if (straight()) {
		wxRect r_ext = trRectToBB(r);
		dc.DrawRectangle(r_ext.x, r_ext.y, r_ext.width, r_ext.height);
	} else {
		wxPoint points[4] = {trPixel(RealPoint(r.left(),  r.top()   ))
		                    ,trPixel(RealPoint(r.left(),  r.bottom()))
		                    ,trPixel(RealPoint(r.right(), r.bottom()))
		                    ,trPixel(RealPoint(r.right(), r.top()   ))};
		dc.DrawPolygon(4,points);
	}
}

void RotatedDC::DrawRoundedRectangle(const RealRect& r, double radius) {
	if (straight()) {
		wxRect r_ext = trRectToBB(r);
		dc.DrawRoundedRectangle(r_ext.x, r_ext.y, r_ext.width, r_ext.height, trS(radius));
	} else {
		// TODO
		DrawRectangle(r);
	}
}

void RotatedDC::DrawCircle(const RealPoint& center, double radius) {
	wxPoint p = tr(center);
	dc.DrawCircle(p.x + 1, p.y + 1, int(trS(radius)));
}

void RotatedDC::DrawEllipticArc(const RealPoint& center, const RealSize& size, double start, double end) {
	wxPoint c_ext = tr(center - size/2);
	wxSize  s_ext = trSizeToBB(size);
	dc.DrawEllipticArc(c_ext.x, c_ext.y, s_ext.x, s_ext.y, rad_to_deg(start) + angle, rad_to_deg(end) + angle);
}
void RotatedDC::DrawEllipticSpoke(const RealPoint& center, const RealSize& size, double angle) {
	wxPoint c_ext = tr(center - size/2);
	wxSize  s_ext = trSizeToBB(size);
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
	dc.SetClippingRegion(trRectToRegion(rect));
}
void RotatedDC::DestroyClippingRegion() {
	dc.DestroyClippingRegion();
}

// ----------------------------------------------------------------------------- : Other

Bitmap RotatedDC::GetBackground(const RealRect& r) {
	wxRect wr = trRectToBB(r);
	Bitmap background(wr.width, wr.height);
	wxMemoryDC mdc;
	mdc.SelectObject(background);
	mdc.Blit(0, 0, wr.width, wr.height, &dc, wr.x, wr.y);
	mdc.SelectObject(wxNullBitmap);
	return background;
}
