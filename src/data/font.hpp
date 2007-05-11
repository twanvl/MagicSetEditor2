//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_FONT
#define HEADER_DATA_FONT

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/real_point.hpp>
#include <script/scriptable.hpp>

DECLARE_POINTER_TYPE(Font);

// ----------------------------------------------------------------------------- : Font

/// A font for rendering text
/** Contains additional information about scaling, color and shadow */
class Font : public IntrusivePtrBase<Font> {
  public:
	Scriptable<String> name;				///< Name of the font
	Scriptable<String> italic_name;			///< Font name for italic text (optional)
	Scriptable<double> size;				///< Size of the font
	Scriptable<String> weight, style;		///< Weight and style of the font (bold/italic)
	Scriptable<bool>   underline;			///< Underlined?
	int                weight_i, style_i;	///< wx constants for weight and style
	double             scale_down_to;		///< Smallest size to scale down to
	Scriptable<Color>  color;				///< Color to use
	Scriptable<Color>  shadow_color;		///< Color for shadow
	RealSize           shadow_displacement;	///< Position of the shadow
	Color              separator_color;		///< Color for <sep> text
	enum {
		NORMAL,
		TYPEWRITER,	///< Use a typewriter font
	} type;
	
	Font();
	
	/// Update the scritables, returns true if there is a change
	bool update(Context& ctx);
	/// Add the given dependency to the dependent_scripts list for the variables this font depends on
	void initDependencies(Context&, const Dependency&) const;
	
	/// Does this font have a shadow?
	inline bool hasShadow() { return shadow_displacement.width != 0 || shadow_displacement.height != 0; }
	
	/// Make a bold/italic/placeholder version of this font
	FontP make(bool bold, bool italic, bool placeholder_color, bool code_color, Color* other_color) const;
	
	/// Convert this font to a wxFont
	wxFont toWxFont(double scale) const;
	
  private:
	DECLARE_REFLECTION();
};


// ----------------------------------------------------------------------------- : EOF
#endif
