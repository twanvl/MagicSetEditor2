//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <render/symbol/filter.hpp>
#include <render/symbol/viewer.hpp>
#include <gfx/gfx.hpp>
#include <util/error.hpp>
#include <script/value.hpp> // for some strange reason the profile build needs this :(

// ----------------------------------------------------------------------------- : Symbol filtering

void filter_symbol(Image& symbol, const SymbolFilter& filter) {
	if (!symbol.HasAlpha()) symbol.InitAlpha();
	Byte* data  = symbol.GetData();
	Byte* alpha = symbol.GetAlpha();
	UInt width = symbol.GetWidth(), height = symbol.GetHeight();
	for (UInt y = 0 ; y < width ; ++y) {
		for (UInt x = 0 ; x < height ; ++x) {
			// Determine set
			//  green           -> border or outside
			//  green+red=white -> border
			SymbolSet point = data[1] ? (data[0] ? SYMBOL_BORDER : SYMBOL_OUTSIDE) : SYMBOL_INSIDE;
			// Call filter
			AColor result = filter.color((double)x / width, (double)y / height, point);
			// Store color
			data[0]  = result.Red();
			data[1]  = result.Green();
			data[2]  = result.Blue();
			alpha[0] = result.alpha;
			// next
			data  += 3;
			alpha += 1;
		}
	}
}

Image render_symbol(const SymbolP& symbol, const SymbolFilter& filter, double border_radius, int size) {
	Image i = render_symbol(symbol, border_radius, size);
	filter_symbol(i, filter);
	return i;
}

// ----------------------------------------------------------------------------- : SymbolFilter

IMPLEMENT_REFLECTION(SymbolFilter) {
	if (!tag.reading()) {
		String fill_type = fillType();
		REFLECT(fill_type);
	}
}
template <> void GetMember::handle(const shared_ptr<SymbolFilter>& f) {
	handle(*f);
}

template <>
shared_ptr<SymbolFilter> read_new<SymbolFilter>(Reader& reader) {
	// there must be a fill type specified
	String fill_type;
	reader.handle(_("fill type"), fill_type);
	if      (fill_type == _("solid"))			return new_shared<SolidFillSymbolFilter>();
	else if (fill_type == _("linear gradient"))	return new_shared<LinearGradientSymbolFilter>();
	else if (fill_type == _("radial gradient"))	return new_shared<RadialGradientSymbolFilter>();
	else {
		throw ParseError(_("Unsupported fill type: '") + fill_type + _("'"));
	}
}

// ----------------------------------------------------------------------------- : SolidFillSymbolFilter

String SolidFillSymbolFilter::fillType() const { return _("solid"); }

AColor SolidFillSymbolFilter::color(double x, double y, SymbolSet point) const {
	if      (point == SYMBOL_INSIDE) return fill_color;
	else if (point == SYMBOL_BORDER) return border_color;
	else                             return AColor(0,0,0,0);
}

IMPLEMENT_REFLECTION(SolidFillSymbolFilter) {
	REFLECT_BASE(SymbolFilter);
	REFLECT(fill_color);
	REFLECT(border_color);
}

// ----------------------------------------------------------------------------- : GradientSymbolFilter

template <typename T>
AColor GradientSymbolFilter::color(double x, double y, SymbolSet point, const T* t) const {
	if      (point == SYMBOL_INSIDE) return lerp(fill_color_1,   fill_color_2,   t->t(x,y));
	else if (point == SYMBOL_BORDER) return lerp(border_color_1, border_color_2, t->t(x,y));
	else                             return AColor(0,0,0,0);
}

IMPLEMENT_REFLECTION(GradientSymbolFilter) {
	REFLECT_BASE(SymbolFilter);
	REFLECT(fill_color_1);
	REFLECT(fill_color_2);
	REFLECT(border_color_1);
	REFLECT(border_color_2);
}

// ----------------------------------------------------------------------------- : LinearGradientSymbolFilter

// TODO: move to some general util header
inline double sqr(double x) { return x * x; }

String LinearGradientSymbolFilter::fillType() const { return _("linear gradient"); }

LinearGradientSymbolFilter::LinearGradientSymbolFilter()
	: center_x(0.5), center_y(0.5)
	, end_x(1), end_y(1)
{}

AColor LinearGradientSymbolFilter::color(double x, double y, SymbolSet point) const {
	len = sqr(end_x - center_x) + sqr(end_y - center_y);
	if (len == 0) len = 1; // prevent div by 0
	return GradientSymbolFilter::color(x,y,point,this);
}

double LinearGradientSymbolFilter::t(double x, double y) const {
	double t= abs( (x - center_x) * (end_x - center_x) + (y - center_y) * (end_y - center_y)) / len;
	return min(1.,max(0.,t));
}

IMPLEMENT_REFLECTION(LinearGradientSymbolFilter) {
	REFLECT_BASE(GradientSymbolFilter);
	REFLECT(center_x); REFLECT(center_y);
	REFLECT(end_x);    REFLECT(end_y);
}

// ----------------------------------------------------------------------------- : RadialGradientSymbolFilter

String RadialGradientSymbolFilter::fillType() const { return _("radial gradient"); }

AColor RadialGradientSymbolFilter::color(double x, double y, SymbolSet point) const {
	return GradientSymbolFilter::color(x,y,point,this);
}

double RadialGradientSymbolFilter::t(double x, double y) const {
	return sqrt( (sqr(x - 0.5) + sqr(y - 0.5)) * 2); 
}
