//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GFX_GENERATED_IMAGE
#define HEADER_GFX_GENERATED_IMAGE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/age.hpp>
#include <gfx/gfx.hpp>
#include <script/value.hpp>

DECLARE_POINTER_TYPE(GeneratedImage);
DECLARE_POINTER_TYPE(SymbolVariation);
class Package;

// ----------------------------------------------------------------------------- : GeneratedImage

/// An image that is generated from a script.
/** The actual generation is independend of the script execution
 */
class GeneratedImage : public ScriptValue {
  public:
	/// Options for generating the image
	struct Options {
		Options(int width = 0, int height = 0, Package* package = nullptr, Package* local_package = nullptr, PreserveAspect preserve_aspect = ASPECT_STRETCH, bool saturate = false)
			: width(width), height(height), preserve_aspect(preserve_aspect), saturate(saturate), package(package), local_package(local_package)
		{}
		
		int            width, height;	///< Width to force the image to, or 0 to keep the width of the input
		PreserveAspect preserve_aspect;
		bool           saturate;
		Package* package;       ///< Package to load images from
		Package* local_package; ///< Package to load symbols and ImageValue images from
	};
	
	/// Generate the image, and conform to the options
	Image generateConform(const Options&) const;
	/// Generate the image
	virtual Image generate(const Options&) const = 0;
	/// How must the image be combined with the background?
	virtual ImageCombine combine() const { return COMBINE_NORMAL; }
	/// Equality should mean that every pixel in the generated images is the same if the same options are used
	virtual bool operator == (const GeneratedImage& that) const = 0;
	inline  bool operator != (const GeneratedImage& that) const { return !(*this == that); }
	
	/// Can this image be generated safely from another thread?
	virtual bool threadSafe() const { return true; };
	
	virtual ScriptType type() const;
	virtual String typeName() const;
};

/// Resize an image to conform to the options
Image conform_image(const Image&, const GeneratedImage::Options&);

// ----------------------------------------------------------------------------- : BlankImage

/// An image generator that returns a blank image
class BlankImage : public GeneratedImage {
  public:
	virtual Image generate(const Options&) const;
	virtual bool operator == (const GeneratedImage& that) const;
};

// ----------------------------------------------------------------------------- : LinearBlendImage

/// An image generator that linearly blends two other images
class LinearBlendImage : public GeneratedImage {
  public:
	inline LinearBlendImage(const GeneratedImageP& image1, const GeneratedImageP& image2, double x1, double y1, double x2, double y2)
		: image1(image1), image2(image2), x1(x1), y1(y1), x2(x2), y2(y2)
	{}
	virtual Image generate(const Options& opt) const;
	virtual ImageCombine combine() const;
	virtual bool operator == (const GeneratedImage& that) const;
  private:
	GeneratedImageP image1, image2;
	double x1, y1, x2, y2;
};

// ----------------------------------------------------------------------------- : MaskedBlendImage

/// An image generator that blends two other images using a third as a mask
class MaskedBlendImage : public GeneratedImage {
  public:
	inline MaskedBlendImage(const GeneratedImageP& light, const GeneratedImageP& dark, const GeneratedImageP& mask)
		: light(light), dark(dark), mask(mask)
	{}
	virtual Image generate(const Options& opt) const;
	virtual ImageCombine combine() const;
	virtual bool operator == (const GeneratedImage& that) const;
  private:
	GeneratedImageP light, dark, mask;
};

// ----------------------------------------------------------------------------- : CombineBlendImage

/// An image generator that blends two other images using an ImageCombine function
class CombineBlendImage : public GeneratedImage {
  public:
	inline CombineBlendImage(const GeneratedImageP& image1, const GeneratedImageP& image2, ImageCombine image_combine)
		: image1(image1), image2(image2), image_combine(image_combine)
	{}
	virtual Image generate(const Options& opt) const;
	virtual ImageCombine combine() const;
	virtual bool operator == (const GeneratedImage& that) const;
  private:
	GeneratedImageP image1, image2;
	ImageCombine image_combine;
};

// ----------------------------------------------------------------------------- : SetMaskImage

/// Change the alpha channel of an image
class SetMaskImage : public GeneratedImage {
  public:
	inline SetMaskImage(const GeneratedImageP& image, const GeneratedImageP& mask)
		: image(image), mask(mask)
	{}
	virtual Image generate(const Options& opt) const;
	virtual ImageCombine combine() const;
	virtual bool operator == (const GeneratedImage& that) const;
  private:
	GeneratedImageP image, mask;
};

/// Change the alpha channel of an image
class SetAlphaImage : public GeneratedImage {
  public:
	inline SetAlphaImage(const GeneratedImageP& image, double alpha)
		: image(image), alpha(alpha)
	{}
	virtual Image generate(const Options& opt) const;
	virtual ImageCombine combine() const;
	virtual bool operator == (const GeneratedImage& that) const;
  private:
	GeneratedImageP image;
	double alpha;
};

// ----------------------------------------------------------------------------- : SetCombineImage

/// Change the combine mode
class SetCombineImage : public GeneratedImage {
  public:
	inline SetCombineImage(const GeneratedImageP& image, ImageCombine image_combine)
		: image(image), image_combine(image_combine)
	{}
	virtual Image generate(const Options& opt) const;
	virtual ImageCombine combine() const;
	virtual bool operator == (const GeneratedImage& that) const;
  private:
	GeneratedImageP image;
	ImageCombine image_combine;
};

// ----------------------------------------------------------------------------- : EnlargeImage

/// Enlarge an image by adding a border around it
class EnlargeImage : public GeneratedImage {
  public:
	inline EnlargeImage(const GeneratedImageP& image, double border_size)
		: image(image), border_size(fabs(border_size))
	{}
	virtual Image generate(const Options& opt) const;
	virtual ImageCombine combine() const;
	virtual bool operator == (const GeneratedImage& that) const;
  private:
	GeneratedImageP image;
	double border_size;
};

// ----------------------------------------------------------------------------- : DropShadowImage

/// Add a drop shadow to an image
class DropShadowImage : public GeneratedImage {
  public:
	inline DropShadowImage(const GeneratedImageP& image, double offset_x, double offset_y, double shadow_alpha, double shadow_blur_radius, Color shadow_color)
		: image(image), offset_x(offset_x), offset_y(offset_y)
		, shadow_alpha(shadow_alpha), shadow_blur_radius(shadow_blur_radius), shadow_color(shadow_color)
	{}
	virtual Image generate(const Options& opt) const;
	virtual ImageCombine combine() const;
	virtual bool operator == (const GeneratedImage& that) const;
  private:
	GeneratedImageP image;
	double offset_x, offset_y;
	double shadow_alpha;
	double shadow_blur_radius;
	Color shadow_color;
};

// ----------------------------------------------------------------------------- : PackagedImage

/// Load an image from a file in a package
class PackagedImage : public GeneratedImage {
  public:
	inline PackagedImage(const String& filename)
		: filename(filename)
	{}
	virtual Image generate(const Options& opt) const;
	virtual bool operator == (const GeneratedImage& that) const;
  private:
	String filename;
};

// ----------------------------------------------------------------------------- : BuiltInImage

/// Return a built in image
class BuiltInImage : public GeneratedImage {
  public:
	inline BuiltInImage(const String& name)
		: name(name)
	{}
	virtual Image generate(const Options& opt) const;
	virtual bool operator == (const GeneratedImage& that) const;
  private:
	String name;
};

// ----------------------------------------------------------------------------- : SymbolToImage

/// Use a symbol as an image
class SymbolToImage : public GeneratedImage {
  public:
	SymbolToImage(const String& filename, Age age, const SymbolVariationP& variation);
	~SymbolToImage();
	virtual Image generate(const Options& opt) const;
	virtual bool operator == (const GeneratedImage& that) const;
	
	#ifdef __WXGTK__
		virtual bool threadSafe() const { return false; }
	#endif
  private:
	SymbolToImage(const SymbolToImage&); // copy ctor
	String           filename;
	Age              age; ///< Age the symbol was last updated
	SymbolVariationP variation;
};

// ----------------------------------------------------------------------------- : ImageValueToImage

/// Use an image from an ImageValue as an image
class ImageValueToImage : public GeneratedImage {
  public:
	ImageValueToImage(const String& filename, Age age);
	~ImageValueToImage();
	virtual Image generate(const Options& opt) const;
	virtual bool operator == (const GeneratedImage& that) const;
  private:
	ImageValueToImage(const ImageValueToImage&); // copy ctor
	String filename;
	Age    age; ///< Age the symbol was last updated
};

// ----------------------------------------------------------------------------- : EOF
#endif
