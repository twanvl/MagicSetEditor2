//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_RENDER_SYMBOL_FILTER
#define HEADER_RENDER_SYMBOL_FILTER

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/reflect.hpp>

class SymbolFilter;

// ----------------------------------------------------------------------------- : Color

/// Color with alpha channel
class AColor : public Color {
  public:
	int alpha;	///< The alpha value, in the range [0..255]
	inline AColor(int r, int g, int b, int a = 255) : Color(r,g,b), alpha(a) {}
	inline AColor(const Color& color,  int a = 255) : Color(color), alpha(a) {}
};

// ----------------------------------------------------------------------------- : Symbol filtering

/// Filter a symbol-image.
/** Filtering means that each pixel will be determined by the specified function.
 *  The result is stored in the symbol parameter.
 */
void filter_symbol(Image& symbol, const SymbolFilter& filter);

/// Is a point inside a symbol?
enum SymbolSet
{	SYMBOL_INSIDE
,	SYMBOL_BORDER
,	SYMBOL_OUTSIDE
};

// ----------------------------------------------------------------------------- : SymbolFilter

/// Base class for symbol filters
class SymbolFilter {
  public:
	virtual ~SymbolFilter() {}
	/// What color should the symbol have at location (x, y)?
	/** x,y are in the range [0...1) */
	virtual AColor color(double x, double y, SymbolSet point) const = 0;
	/// Name of this fill type
	virtual String fillType() const = 0;
	
	DECLARE_REFLECTION_VIRTUAL();
};

template <>
shared_ptr<SymbolFilter> read_new<SymbolFilter>(Reader& reader);

// ----------------------------------------------------------------------------- : SymbolFilter types

/// Symbol filter that returns solid colors
class SolidFillSymbolFilter : public SymbolFilter {
  public:
	virtual AColor color(double x, double y, SymbolSet point) const;
	virtual String fillType() const;
  private:
	Color fill_color, border_color;
	DECLARE_REFLECTION();
};

/// Symbol filter that returns some gradient
class GradientSymbolFilter : public SymbolFilter {
  protected:
	Color fill_color_1, border_color_1;
	Color fill_color_2, border_color_2;
	template <typename T>
	AColor color(double x, double y, SymbolSet point, const T* t) const;
	
	DECLARE_REFLECTION();
};

/// Symbol filter that returns a linear gradient
class LinearGradientSymbolFilter : public GradientSymbolFilter {
  public:
	LinearGradientSymbolFilter();
	
	virtual AColor color(double x, double y, SymbolSet point) const;
	virtual String fillType() const;
	
	/// return time on the gradient, used by GradientSymbolFilter::color
	inline double t(double x, double y) const;
	
  private:
	double center_x, center_y;
	double end_x,    end_y;
	DECLARE_REFLECTION();
};

/// Symbol filter that returns a radial gradient
class RadialGradientSymbolFilter : public GradientSymbolFilter {
  public:
	virtual AColor color(double x, double y, SymbolSet point) const;
	virtual String fillType() const;
	
	/// return time on the gradient, used by GradientSymbolFilter::color
	inline double t(double x, double y) const;
};

// ----------------------------------------------------------------------------- : EOF
#endif
