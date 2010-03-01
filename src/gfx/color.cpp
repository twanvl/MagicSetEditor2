//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gfx/color.hpp>

// ----------------------------------------------------------------------------- : Parsing etc.

template <> void Reader::handle(Color& col) {
	col = parse_color(getValue());
	if (!col.Ok()) col = *wxBLACK;
}

template <> void Reader::handle(AColor& col) {
	col = parse_acolor(getValue());
	if (!col.Ok()) col = AColor(0,0,0,0);
}
template <> void Writer::handle(const AColor& col) {
	handle(format_acolor(col));
}


Color parse_color(const String& v) {
	UInt r,g,b;
	if (wxSscanf(v.c_str(),_("rgb(%u,%u,%u)"),&r,&g,&b)) {
		return Color(r, g, b);
	} else {
		return Color(v);
	}
}

AColor parse_acolor(const String& v) {
	UInt r,g,b,a;
	if (wxSscanf(v.c_str(),_("rgb(%u,%u,%u)"),&r,&g,&b)) {
		return AColor(r, g, b);
	} else if (wxSscanf(v.c_str(),_("rgba(%u,%u,%u,%u)"),&r,&g,&b,&a)) {
		return AColor(r, g, b, a);
	} else if (v == _("transparent")) {
		return AColor(0,0,0,0);
	} else {
		return Color(v);
	}
}

String format_acolor(AColor col) {
	if (col.alpha == 255) {
		return String::Format(_("rgb(%u,%u,%u)"), col.Red(), col.Green(), col.Blue());
	} else if (col.alpha == 0) {
		return _("transparent");
	} else {
		return String::Format(_("rgba(%u,%u,%u,%u)"), col.Red(), col.Green(), col.Blue(), col.alpha);
	}
}

// ----------------------------------------------------------------------------- : Color utility functions

Color lerp(const Color& a, const Color& b, double t) {
	return Color(static_cast<int>( a.Red()   + (b.Red()   - a.Red()  ) * t ),
	             static_cast<int>( a.Green() + (b.Green() - a.Green()) * t ),
	             static_cast<int>( a.Blue()  + (b.Blue()  - a.Blue() ) * t ));
}


int hsl2rgbp(double t1, double t2, double t3) {
	// adjust t3 to [0...1)
	if      (t3 < 0.0) t3 += 1;
	else if (t3 > 1.0) t3 -= 1;
	// determine color
	if (6.0 * t3 < 1) return (int)(255 * (t1 + (t2-t1) * 6.0 * t3)             );
	if (2.0 * t3 < 1) return (int)(255 * (t2)                                  );
	if (3.0 * t3 < 2) return (int)(255 * (t1 + (t2-t1) * 6.0 * (2.0/3.0 - t3)) );
	else              return (int)(255 * (t1)                                  );
}
Color hsl2rgb(double h, double s, double l) {
	double t2 = l < 0.5 ? l * (1.0 + s) :
	                      l * (1.0 - s) + s;
	double t1 = 2.0 * l - t2;
	return Color(
		hsl2rgbp(t1, t2, h + 1.0/3.0),
		hsl2rgbp(t1, t2, h)          ,
		hsl2rgbp(t1, t2, h - 1.0/3.0)
	);
}


Color darken(const Color& c) {
	return Color(
		c.Red()   * 8 / 10,
		c.Green() * 8 / 10,
		c.Blue()  * 8 / 10
	);
}

Color saturate(const Color& c, double amount) {
	int r = c.Red(), g = c.Green(), b = c.Blue();
	double l = (r + g + b) / 3;
	return Color(
		col(static_cast<int>( (r - amount * l) / (1 - amount) )),
		col(static_cast<int>( (g - amount * l) / (1 - amount) )),
		col(static_cast<int>( (b - amount * l) / (1 - amount) ))
	);
}


void fill_image(Image& image, const Color& color) {
	Byte* pos = image.GetData();
	Byte* end = pos + image.GetWidth() * image.GetHeight() * 3;
	Byte r = color.Red(), g = color.Green(), b = color.Blue();
	if (r == g && r == b) {
		// optimization: use memset
		memset(pos, r, end-pos);
	} else {
		// fill the image
		while (pos != end) {
			*pos++ = r;
			*pos++ = g;
			*pos++ = b;
		}
	}
}
