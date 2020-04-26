//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gfx/color.hpp>

// ----------------------------------------------------------------------------- : Parsing etc.

template <> void Reader::handle(Color& out) {
  String const& str = getValue();
  auto col = parse_color(str);
  if (!col.has_value()) {
    out = Color();
    warning(_("Not a valid color value: ") + str);
  } else {
    out = *col;
  }
}

template <> void Writer::handle(const Color& col) {
  handle(format_color(col));
}


optional<Color> parse_color(const String& v) {
  UInt r,g,b,a;
  if (wxSscanf(v.c_str(),_("rgb(%u,%u,%u)"),&r,&g,&b)) {
    return Color(r, g, b);
  } else if (wxSscanf(v.c_str(),_("rgba(%u,%u,%u,%u)"),&r,&g,&b,&a)) {
    return Color(r, g, b, a);
  } else if (v == _("transparent")) {
    return Color(0,0,0,0);
  } else {
    // Try to find a named color
    wxColour c = wxTheColourDatabase->Find(v);
    if (c.Ok()) {
      return Color(c);
    } else {
      return optional<Color>();
    }
  }
}

String format_color(Color col) {
  if (col.Alpha() == 255) {
    return String::Format(_("rgb(%u,%u,%u)"), col.Red(), col.Green(), col.Blue());
  } else if (col.Alpha() == 0) {
    return _("transparent");
  } else {
    return String::Format(_("rgba(%u,%u,%u,%u)"), col.Red(), col.Green(), col.Blue(), col.Alpha());
  }
}

// ----------------------------------------------------------------------------- : Color utility functions

Color lerp(Color a, Color b, double t) {
  return Color(static_cast<int>( a.Red()   + (b.Red()   - a.Red()  ) * t ),
               static_cast<int>( a.Green() + (b.Green() - a.Green()) * t ),
               static_cast<int>( a.Blue()  + (b.Blue()  - a.Blue() ) * t ),
               static_cast<int>( a.Alpha() + (b.Alpha() - a.Alpha()) * t ));
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


Color darken(Color c) {
  return Color(
    c.r * 8 / 10,
    c.g * 8 / 10,
    c.b * 8 / 10,
    c.a
  );
}

Color saturate(Color c, double amount) {
  double l = (c.r + c.g + c.b) / 3;
  return Color(
    col(static_cast<int>( (c.r - amount * l) / (1 - amount) )),
    col(static_cast<int>( (c.g - amount * l) / (1 - amount) )),
    col(static_cast<int>( (c.b - amount * l) / (1 - amount) )),
    c.a
  );
}


void fill_image(Image& image, RGB x) {
  RGB* pos = (RGB*)image.GetData();
  RGB* end = pos + image.GetWidth() * image.GetHeight();
  if (x.r == x.g && x.r == x.b) {
    // optimization: use memset
    memset(pos, x.r, (end-pos) * sizeof(*pos));
  } else {
    // fill the image
    while (pos != end) {
      *pos++ = x;
    }
  }
}
