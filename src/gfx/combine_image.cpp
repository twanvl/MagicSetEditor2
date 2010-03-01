//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gfx/gfx.hpp>
#include <util/reflect.hpp>
#include <algorithm>

using namespace std;

// ----------------------------------------------------------------------------- : Reflection for combining modes

IMPLEMENT_REFLECTION_ENUM(ImageCombine) {
	VALUE_N("normal",		COMBINE_NORMAL);
	VALUE_N("add",			COMBINE_ADD);
	VALUE_N("subtract",		COMBINE_SUBTRACT);
	VALUE_N("stamp",		COMBINE_STAMP);
	VALUE_N("difference",	COMBINE_DIFFERENCE);
	VALUE_N("negation",		COMBINE_NEGATION);
	VALUE_N("multiply",		COMBINE_MULTIPLY);
	VALUE_N("darken",		COMBINE_DARKEN);
	VALUE_N("lighten",		COMBINE_LIGHTEN);
	VALUE_N("color dodge",	COMBINE_COLOR_DODGE);
	VALUE_N("color burn",	COMBINE_COLOR_BURN);
	VALUE_N("screen",		COMBINE_SCREEN);
	VALUE_N("overlay",		COMBINE_OVERLAY);
	VALUE_N("hard light",	COMBINE_HARD_LIGHT);
	VALUE_N("soft light",	COMBINE_SOFT_LIGHT);
	VALUE_N("reflect",		COMBINE_REFLECT);
	VALUE_N("glow",			COMBINE_GLOW);
	VALUE_N("freeze",		COMBINE_FREEZE);
	VALUE_N("heat",			COMBINE_HEAT);
	VALUE_N("and",			COMBINE_AND);
	VALUE_N("or",			COMBINE_OR);
	VALUE_N("xor",			COMBINE_XOR);
	VALUE_N("shadow",		COMBINE_SHADOW);
	VALUE_N("symmetric overlay",COMBINE_SYMMETRIC_OVERLAY);
}

// ----------------------------------------------------------------------------- : Combining functions

// Functor for combining functions for a given combining type
template <ImageCombine combine> struct Combine {
	static inline int f(int a, int b);
};

// Give a combining function for enum value 'combine'
#define COMBINE_FUN(combine,fun)	\
	template <> int Combine<combine>::f(int a, int b) { return fun; }

// Based on
//  http://www.pegtop.net/delphi/articles/blendmodes/

COMBINE_FUN(COMBINE_NORMAL,		b													)
COMBINE_FUN(COMBINE_ADD,		top(a + b)											)
COMBINE_FUN(COMBINE_SUBTRACT,	bot(a - b)											)
COMBINE_FUN(COMBINE_STAMP,		col(a - 2 * b + 256)								)
COMBINE_FUN(COMBINE_DIFFERENCE,	abs(a - b)											)
COMBINE_FUN(COMBINE_NEGATION,	255 - abs(255 - a - b)								)
COMBINE_FUN(COMBINE_MULTIPLY,	(a * b) / 255										)
COMBINE_FUN(COMBINE_DARKEN,		min(a, b)											)
COMBINE_FUN(COMBINE_LIGHTEN,	max(a, b)											)
COMBINE_FUN(COMBINE_COLOR_DODGE,b == 255 ? 255 : top(a * 255 / (255 - b))			)
COMBINE_FUN(COMBINE_COLOR_BURN,	b == 0   ? 0   : bot(255 - (255-a) * 255 / b)		)
COMBINE_FUN(COMBINE_SCREEN,		255 - (((255 - a) * (255 - b)) / 255)				)
COMBINE_FUN(COMBINE_OVERLAY,	a < 128
									? (a * b) >> 7
									: 255 - (((255 - a) * (255 - b)) >> 7)			)
COMBINE_FUN(COMBINE_HARD_LIGHT,	b < 128
									? (a * b) >> 7
									: 255 - (((255 - a) * (255 - b)) >> 7)			)
COMBINE_FUN(COMBINE_SOFT_LIGHT,	b)
COMBINE_FUN(COMBINE_REFLECT,	b == 255 ? 255 : top(a * a / (255 - b))				)
COMBINE_FUN(COMBINE_GLOW,		a == 255 ? 255 : top(b * b / (255 - a))				)
COMBINE_FUN(COMBINE_FREEZE,		b == 0 ? 0 : bot(255 - (255 - a) * (255 - a) / b)	)
COMBINE_FUN(COMBINE_HEAT,		a == 0 ? 0 : bot(255 - (255 - b) * (255 - b) / a)	)
COMBINE_FUN(COMBINE_AND,		a & b												)
COMBINE_FUN(COMBINE_OR,			a | b												)
COMBINE_FUN(COMBINE_XOR,		a ^ b												)
COMBINE_FUN(COMBINE_SHADOW,		(b * a * a) / (255 * 255)							)
COMBINE_FUN(COMBINE_SYMMETRIC_OVERLAY,	(Combine<COMBINE_OVERLAY>::f(a,b) + Combine<COMBINE_OVERLAY>::f(b,a)) / 2 )

// ----------------------------------------------------------------------------- : Combining

/// Combine image b onto image a using some combining mode.
/// The results are stored in the image A.
template <ImageCombine combine>
void combine_image_do(Image& a, Image b) {
	UInt size = a.GetWidth() * a.GetHeight() * 3;
	Byte *dataA = a.GetData(), *dataB = b.GetData();
	// for each pixel: apply function
	for (UInt i = 0 ; i < size ; ++i) {
		dataA[i] = Combine<combine>::f(dataA[i], dataB[i]);
	}
}

void combine_image(Image& a, const Image& b, ImageCombine combine) {
	// Images must have same size
	assert(a.GetWidth()  == b.GetWidth());
	assert(a.GetHeight() == b.GetHeight());
	// Copy alpha channel?
	if (b.HasAlpha()) {
		if (!a.HasAlpha()) a.InitAlpha();
		memcpy(a.GetAlpha(), b.GetAlpha(), a.GetWidth() * a.GetHeight());
	}
	// Combine image data, by dispatching to combineImageDo
	switch(combine) {
		#define DISPATCH(comb) case comb: combine_image_do<comb>(a,b); return
		case COMBINE_DEFAULT:
		case COMBINE_NORMAL: a = b; return; // no need to do a per pixel operation
		DISPATCH(COMBINE_ADD);
		DISPATCH(COMBINE_SUBTRACT);
		DISPATCH(COMBINE_STAMP);
		DISPATCH(COMBINE_DIFFERENCE);
		DISPATCH(COMBINE_NEGATION);
		DISPATCH(COMBINE_MULTIPLY);
		DISPATCH(COMBINE_DARKEN);
		DISPATCH(COMBINE_LIGHTEN);
		DISPATCH(COMBINE_COLOR_DODGE);
		DISPATCH(COMBINE_COLOR_BURN);
		DISPATCH(COMBINE_SCREEN);
		DISPATCH(COMBINE_OVERLAY);
		DISPATCH(COMBINE_HARD_LIGHT);
		DISPATCH(COMBINE_SOFT_LIGHT);
		DISPATCH(COMBINE_REFLECT);
		DISPATCH(COMBINE_GLOW);
		DISPATCH(COMBINE_FREEZE);
		DISPATCH(COMBINE_HEAT);
		DISPATCH(COMBINE_AND);
		DISPATCH(COMBINE_OR);
		DISPATCH(COMBINE_XOR);
		DISPATCH(COMBINE_SHADOW);
		DISPATCH(COMBINE_SYMMETRIC_OVERLAY);
	}
}

void draw_combine_image(DC& dc, UInt x, UInt y, const Image& img, ImageCombine combine) {
	if (combine <= COMBINE_NORMAL) {
		dc.DrawBitmap(img, x, y);
	} else {
		// Capture the current image in the target rectangle
		Bitmap sourceB(img.GetWidth(), img.GetHeight());
		wxMemoryDC sourceDC;
		sourceDC.SelectObject(sourceB);
		sourceDC.Blit(0, 0, img.GetWidth(), img.GetHeight(), &dc, x, y);
		sourceDC.SelectObject(wxNullBitmap);
		Image source = sourceB.ConvertToImage();
		// Combine and draw
		combine_image(source, img, combine);
		dc.DrawBitmap(source, x, y);
	}
}
