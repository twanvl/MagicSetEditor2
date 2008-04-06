//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
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

enum FontFlags
{	FONT_NORMAL      = 0
,	FONT_BOLD        = 0x01
,	FONT_ITALIC      = 0x02
,	FONT_SOFT        = 0x04
,	FONT_CODE        = 0x08
,	FONT_CODE_KW     = 0x10 // syntax highlighting
,	FONT_CODE_STRING = 0x20 // syntax highlighting
,	FONT_CODE_NUMBER = 0x40 // syntax highlighting
,	FONT_CODE_OPER   = 0x80 // syntax highlighting
};

/// A font for rendering text
/** Contains additional information about scaling, color and shadow */
class Font : public IntrusivePtrBase<Font> {
  public:
	Scriptable<String> name;				///< Name of the font
	Scriptable<String> italic_name;			///< Font name for italic text (optional)
	Scriptable<double> size;				///< Size of the font
	Scriptable<String> weight, style;		///< Weight and style of the font (bold/italic)
	Scriptable<bool>   underline;			///< Underlined?
	double             scale_down_to;		///< Smallest size to scale down to
	double             max_stretch;			///< How much should the font be stretched before scaling down?
	Scriptable<Color>  color;				///< Color to use
	Scriptable<Color>  shadow_color;		///< Color for shadow
	RealSize           shadow_displacement;	///< Position of the shadow
	Color              separator_color;		///< Color for <sep> text
	int                flags;				///< FontFlags for this font
	
	Font();
	
	/// Update the scritables, returns true if there is a change
	bool update(Context& ctx);
	/// Add the given dependency to the dependent_scripts list for the variables this font depends on
	void initDependencies(Context&, const Dependency&) const;
	
	/// Does this font have a shadow?
	inline bool hasShadow() const {
		return shadow_displacement.width != 0 || shadow_displacement.height != 0;
	}
	
	/// Add style to a font, and optionally change the color and size
	FontP make(int add_flags, Color* other_color, double* other_size) const;
	
	/// Convert this font to a wxFont
	wxFont toWxFont(double scale) const;
	
  private:
	DECLARE_REFLECTION();
};


// ----------------------------------------------------------------------------- : EOF
#endif
