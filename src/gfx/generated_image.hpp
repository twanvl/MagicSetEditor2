//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/age.hpp>
#include <util/io/package.hpp>
#include <gfx/gfx.hpp>
#include <script/value.hpp>

DECLARE_POINTER_TYPE(GeneratedImage);
DECLARE_POINTER_TYPE(SymbolVariation);
class Package;

// ----------------------------------------------------------------------------- : GeneratedImage

/// An image that is generated from a script.
/** The actual generation is independend of the script execution
 */
class GeneratedImage : public ScriptValue, public IntrusiveFromThis<GeneratedImage> {
public:
  /// Options for generating the image
  struct Options {
    Options(int width = 0, int height = 0, Package* package = nullptr, Package* local_package = nullptr, PreserveAspect preserve_aspect = ASPECT_STRETCH, bool saturate = false)
      : width(width), height(height), zoom(1.0), angle(0)
      , preserve_aspect(preserve_aspect), saturate(saturate)
      , package(package), local_package(local_package)
    {}
    
    mutable int    width, height;  ///< Width to force the image to, or 0 to keep the width of the input
                    ///< In that case, width and height will be later set to the actual size
    double         zoom;            ///< Zoom factor to use, when width=height=0
    Radians        angle;           ///< Angle to rotate image by afterwards
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
  virtual ImageCombine combine() const { return COMBINE_DEFAULT; }
  /// Equality should mean that every pixel in the generated images is the same if the same options are used
  virtual bool operator == (const GeneratedImage& that) const = 0;
  inline  bool operator != (const GeneratedImage& that) const { return !(*this == that); }
  
  /// Can this image be generated safely from another thread?
  virtual bool threadSafe() const { return true; }
  /// Is this image specific to the set (the local_package)?
  virtual bool local() const { return false; }
  /// Is this image blank?
  virtual bool isBlank() const { return false; }
  
  ScriptType type() const override;
  String typeName() const override;
  GeneratedImageP toImage() const override;
};

/// Resize an image to conform to the options
Image conform_image(const Image&, const GeneratedImage::Options&);

// ----------------------------------------------------------------------------- : SimpleFilterImage

/// Apply some filter to a single image
class SimpleFilterImage : public GeneratedImage {
public:
  inline SimpleFilterImage(const GeneratedImageP& image)
    : image(image)
  {}
  ImageCombine combine() const override { return image->combine(); }
  bool local() const override { return image->local(); }
protected:
  GeneratedImageP image;
};

// ----------------------------------------------------------------------------- : BlankImage

/// An image generator that returns a blank image
class BlankImage : public GeneratedImage {
public:
  Image generate(const Options&) const override;
  bool operator == (const GeneratedImage& that) const override;
  bool isBlank() const override { return true; }
  
  // Why is this not thread safe? What is GTK smoking?
  #ifdef __WXGTK__
    bool threadSafe() const override { return false; }
  #endif
};

// ----------------------------------------------------------------------------- : LinearBlendImage

/// An image generator that linearly blends two other images
class LinearBlendImage : public GeneratedImage {
public:
  inline LinearBlendImage(const GeneratedImageP& image1, const GeneratedImageP& image2, double x1, double y1, double x2, double y2)
    : image1(image1), image2(image2), x1(x1), y1(y1), x2(x2), y2(y2)
  {}
  Image generate(const Options& opt) const override;
  ImageCombine combine() const override;
  bool operator == (const GeneratedImage& that) const override;
  bool local() const override { return image1->local() && image2->local(); }
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
  Image generate(const Options& opt) const override;
  ImageCombine combine() const override;
  bool operator == (const GeneratedImage& that) const override;
  bool local() const override { return light->local() && dark->local() && mask->local(); }
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
  Image generate(const Options& opt) const override;
  ImageCombine combine() const override;
  bool operator == (const GeneratedImage& that) const override;
  bool local() const override { return image1->local() && image2->local(); }
private:
  GeneratedImageP image1, image2;
  ImageCombine image_combine;
};

// ----------------------------------------------------------------------------- : OverlayImage

/// Overlay an image over another
class OverlayImage : public GeneratedImage {
public:
	inline OverlayImage(const GeneratedImageP& image1, const GeneratedImageP& image2, double offset_x, double offset_y)
		: image1(image1), image2(image2), offset_x(offset_x), offset_y(offset_y)
	{}
	Image generate(const Options& opt) const override;
	ImageCombine combine() const override;
	bool operator == (const GeneratedImage& that) const override;
	bool local() const override { return image1->local() && image2->local(); }
private:
	GeneratedImageP image1, image2;
	double offset_x, offset_y;
};

// ----------------------------------------------------------------------------- : SetMaskImage

/// Change the alpha channel of an image
class SetMaskImage : public SimpleFilterImage {
public:
  inline SetMaskImage(const GeneratedImageP& image, const GeneratedImageP& mask)
    : SimpleFilterImage(image), mask(mask)
  {}
  Image generate(const Options& opt) const override;
  bool operator == (const GeneratedImage& that) const override;
private:
  GeneratedImageP mask;
};

/// Change the alpha channel of an image
class SetAlphaImage : public SimpleFilterImage {
public:
  inline SetAlphaImage(const GeneratedImageP& image, double alpha)
    : SimpleFilterImage(image), alpha(alpha)
  {}
  Image generate(const Options& opt) const override;
  bool operator == (const GeneratedImage& that) const override;
private:
  double alpha;
};

// ----------------------------------------------------------------------------- : SetCombineImage

/// Change the combine mode
class SetCombineImage : public SimpleFilterImage {
public:
  inline SetCombineImage(const GeneratedImageP& image, ImageCombine image_combine)
    : SimpleFilterImage(image), image_combine(image_combine)
  {}
  Image generate(const Options& opt) const override;
  ImageCombine combine() const override;
  bool operator == (const GeneratedImage& that) const override;
private:
  ImageCombine image_combine;
};

// ----------------------------------------------------------------------------- : SaturateImage

/// Saturate/desaturate an image
class SaturateImage : public SimpleFilterImage {
public:
  inline SaturateImage(const GeneratedImageP& image, double amount)
    : SimpleFilterImage(image), amount(amount)
  {}
  Image generate(const Options& opt) const override;
  bool operator == (const GeneratedImage& that) const override;
private:
  double amount;
};

// ----------------------------------------------------------------------------- : InvertImage

/// Invert an image
class InvertImage : public SimpleFilterImage {
public:
  inline InvertImage(const GeneratedImageP& image)
    : SimpleFilterImage(image)
  {}
  Image generate(const Options& opt) const override;
  bool operator == (const GeneratedImage& that) const override;
};

// ----------------------------------------------------------------------------- : RecolorImage

/// Recolor an image
class RecolorImage : public SimpleFilterImage {
public:
  inline RecolorImage(const GeneratedImageP& image, Color color)
    : SimpleFilterImage(image), color(color)
  {}
  Image generate(const Options& opt) const override;
  bool operator == (const GeneratedImage& that) const override;
private:
  Color color;
};
/// Recolor an image, with custom colors
class RecolorImage2 : public SimpleFilterImage {
public:
  inline RecolorImage2(const GeneratedImageP& image, Color red, Color green, Color blue, Color white)
    : SimpleFilterImage(image), red(red), green(green), blue(blue), white(white)
  {}
  Image generate(const Options& opt) const override;
  bool operator == (const GeneratedImage& that) const override;
private:
  Color red,green,blue,white;
};

// ----------------------------------------------------------------------------- : FlipImage

/// Flip an image horizontally
class FlipImageHorizontal : public SimpleFilterImage {
public:
  inline FlipImageHorizontal(const GeneratedImageP& image)
    : SimpleFilterImage(image)
  {}
  Image generate(const Options& opt) const override;
  bool operator == (const GeneratedImage& that) const override;
};

/// Flip an image vertically
class FlipImageVertical : public SimpleFilterImage {
public:
  inline FlipImageVertical(const GeneratedImageP& image)
    : SimpleFilterImage(image)
  {}
  Image generate(const Options& opt) const override;
  bool operator == (const GeneratedImage& that) const override;
};

/// Rotate an image
class RotateImage : public SimpleFilterImage {
public:
  inline RotateImage(const GeneratedImageP& image, Radians angle)
    : SimpleFilterImage(image), angle(angle)
  {}
  Image generate(const Options& opt) const override;
  bool operator == (const GeneratedImage& that) const override;
private:
  Radians angle;
};

// ----------------------------------------------------------------------------- : EnlargeImage

/// Enlarge an image by adding a border around it
class EnlargeImage : public SimpleFilterImage {
public:
  inline EnlargeImage(const GeneratedImageP& image, double border_size)
    : SimpleFilterImage(image), border_size(fabs(border_size))
  {}
  Image generate(const Options& opt) const override;
  bool operator == (const GeneratedImage& that) const override;
private:
  double border_size;
};

// ----------------------------------------------------------------------------- : CropImage

/// Crop an image at a certain point, to a certain size
class CropImage : public SimpleFilterImage {
public:
  inline CropImage(const GeneratedImageP& image, double width, double height, double offset_x, double offset_y)
    : SimpleFilterImage(image), width(width), height(height), offset_x(offset_x), offset_y(offset_y)
  {}
  Image generate(const Options& opt) const override;
  bool operator == (const GeneratedImage& that) const override;
private:
  double width, height;
  double offset_x, offset_y;
};

// ----------------------------------------------------------------------------- : ResizeImage

/// Resize an image
class ResizeImage : public SimpleFilterImage {
public:
  inline ResizeImage(const GeneratedImageP& image, double width, double height, wxImageResizeQuality resize_quality)
    : SimpleFilterImage(image), width(width), height(height), resize_quality(resize_quality)
  {}
  Image generate(const Options& opt) const override;
  bool operator == (const GeneratedImage& that) const override;
private:
  double width, height;
  wxImageResizeQuality resize_quality;
};

// ----------------------------------------------------------------------------- : DropShadowImage

/// Add a drop shadow to an image
class DropShadowImage : public SimpleFilterImage {
public:
  inline DropShadowImage(const GeneratedImageP& image, double offset_x, double offset_y, double shadow_alpha, double shadow_blur_radius, Color shadow_color)
    : SimpleFilterImage(image), offset_x(offset_x), offset_y(offset_y)
    , shadow_alpha(shadow_alpha), shadow_blur_radius(shadow_blur_radius), shadow_color(shadow_color)
  {}
  Image generate(const Options& opt) const override;
  bool operator == (const GeneratedImage& that) const override;
private:
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
  Image generate(const Options& opt) const override;
  bool operator == (const GeneratedImage& that) const override;
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
  Image generate(const Options& opt) const override;
  bool operator == (const GeneratedImage& that) const override;
private:
  String name;
};

// ----------------------------------------------------------------------------- : SymbolToImage

/// Use a symbol as an image
class SymbolToImage : public GeneratedImage {
public:
  SymbolToImage(bool is_local, const LocalFileName& filename, Age age, const SymbolVariationP& variation);
  ~SymbolToImage();
  Image generate(const Options& opt) const override;
  bool operator == (const GeneratedImage& that) const override;
  bool local() const override { return is_local; }
  
  #ifdef __WXGTK__
    bool threadSafe() const override { return false; }
  #endif
private:
  SymbolToImage(const SymbolToImage&); // copy ctor
  bool             is_local; ///< Use local package?
  LocalFileName    filename;
  Age              age;      ///< Age the symbol was last updated
  SymbolVariationP variation;
};

// ----------------------------------------------------------------------------- : ImageValueToImage

/// Use an image from an ImageValue as an image
class ImageValueToImage : public GeneratedImage {
public:
  ImageValueToImage(const LocalFileName& filename, Age age);
  ~ImageValueToImage();
  Image generate(const Options& opt) const override;
  bool operator == (const GeneratedImage& that) const override;
  bool local() const override { return true; }
private:
  ImageValueToImage(const ImageValueToImage&); // copy ctor
  LocalFileName filename;
  Age age; ///< Age the image was last updated
};

