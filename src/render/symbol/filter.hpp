//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_RENDER_SYMBOL_FILTER
#define HEADER_RENDER_SYMBOL_FILTER

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/reflect.hpp>

DECLARE_POINTER_TYPE(Symbol);
class SymbolFilter;

// ----------------------------------------------------------------------------- : Color

/// Color with alpha channel
class AColor : public Color {
  public:
	Byte alpha;	///< The alpha value, in the range [0..255]
	inline AColor()                                     :               alpha(0) {}
	inline AColor(Byte r, Byte g, Byte b, Byte a = 255) : Color(r,g,b), alpha(a) {}
	inline AColor(const Color& color,     Byte a = 255) : Color(color), alpha(a) {}
};

// ----------------------------------------------------------------------------- : Symbol filtering

/// Filter a symbol-image.
/** Filtering means that each pixel will be determined by the specified function.
 *  The result is stored in the symbol parameter.
 */
void filter_symbol(Image& symbol, const SymbolFilter& filter);

/// Render a Symbol to an Image and filter it
Image render_symbol(const SymbolP& symbol, const SymbolFilter& filter, double border_radius = 0.05, int width = 100, int height = 100, bool edit_hints = false, bool allow_smaller = false);

/// Is a point inside a symbol?
enum SymbolSet
{	SYMBOL_INSIDE
,	SYMBOL_BORDER
,	SYMBOL_OUTSIDE
};

// ----------------------------------------------------------------------------- : SymbolFilter

/// Base class for symbol filters
class SymbolFilter : public IntrusivePtrVirtualBase {
  public:
	virtual ~SymbolFilter() {}
	/// What color should the symbol have at location (x, y)?
	/** x,y are in the range [0...1) */
	virtual AColor color(double x, double y, SymbolSet point) const = 0;
	/// Name of this fill type
	virtual String fillType() const = 0;
	/// Comparision
	virtual bool operator == (const SymbolFilter& that) const = 0;
	
	DECLARE_REFLECTION_VIRTUAL();
};

template <>
intrusive_ptr<SymbolFilter> read_new<SymbolFilter>(Reader& reader);

// ----------------------------------------------------------------------------- : SymbolFilter types

/// Symbol filter that returns solid colors
class SolidFillSymbolFilter : public SymbolFilter {
  public:
	inline SolidFillSymbolFilter() {}
	inline SolidFillSymbolFilter(const AColor& fill_color, const AColor& border_color)
		: fill_color(fill_color), border_color(border_color)
	{}
	virtual AColor color(double x, double y, SymbolSet point) const;
	virtual String fillType() const;
	virtual bool operator == (const SymbolFilter& that) const;
  private:
	AColor fill_color, border_color;
	DECLARE_REFLECTION();
};

/// Symbol filter that returns some gradient
class GradientSymbolFilter : public SymbolFilter {
  public:
	inline GradientSymbolFilter() {}
	inline GradientSymbolFilter(const Color& fill_color_1, const Color& border_color_1, const Color& fill_color_2, const Color& border_color_2)
		: fill_color_1(fill_color_1), border_color_1(border_color_1)
		, fill_color_2(fill_color_2), border_color_2(border_color_2)
	{}
  protected:
	Color fill_color_1, border_color_1;
	Color fill_color_2, border_color_2;
	template <typename T>
	AColor color(double x, double y, SymbolSet point, const T* t) const;
	bool equal(const GradientSymbolFilter& that) const;
	
	DECLARE_REFLECTION();
};

/// Symbol filter that returns a linear gradient
class LinearGradientSymbolFilter : public GradientSymbolFilter {
  public:
	LinearGradientSymbolFilter();
	LinearGradientSymbolFilter(const Color& fill_color_1, const Color& border_color_1, const Color& fill_color_2, const Color& border_color_2
	                          ,double center_x, double center_y, double end_x, double end_y);
	
	virtual AColor color(double x, double y, SymbolSet point) const;
	virtual String fillType() const;
	virtual bool operator == (const SymbolFilter& that) const;
	
	/// return time on the gradient, used by GradientSymbolFilter::color
	inline double t(double x, double y) const;
	
  private:
	double center_x, center_y;
	double end_x,    end_y;
	mutable double len;
	DECLARE_REFLECTION();
};

/// Symbol filter that returns a radial gradient
class RadialGradientSymbolFilter : public GradientSymbolFilter {
  public:
	inline RadialGradientSymbolFilter() {}
	inline RadialGradientSymbolFilter(const Color& fill_color_1, const Color& border_color_1, const Color& fill_color_2, const Color& border_color_2)
		: GradientSymbolFilter(fill_color_1, border_color_1, fill_color_2, border_color_2)
	{}
	
	virtual AColor color(double x, double y, SymbolSet point) const;
	virtual String fillType() const;
	virtual bool operator == (const SymbolFilter& that) const;
	
	/// return time on the gradient, used by GradientSymbolFilter::color
	inline double t(double x, double y) const;
};

// ----------------------------------------------------------------------------- : EOF
#endif
