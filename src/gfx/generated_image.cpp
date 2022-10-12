//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gfx/generated_image.hpp>
#include <util/io/package.hpp>
#include <util/error.hpp>
#include <data/symbol.hpp>
#include <data/field/symbol.hpp>
#include <render/symbol/filter.hpp>
#include <gui/util.hpp> // load_resource_image

// ----------------------------------------------------------------------------- : GeneratedImage

ScriptType GeneratedImage::type() const { return SCRIPT_IMAGE; }
String GeneratedImage::typeName() const { return _TYPE_("image"); }
GeneratedImageP GeneratedImage::toImage() const {
  return const_cast<GeneratedImage*>(this)->intrusive_from_this();
}

Image GeneratedImage::generateConform(const Options& options) const {
  return conform_image(generate(options),options);
}

Image conform_image(const Image& img, const GeneratedImage::Options& options) {
  Image image = img;
  // resize?
  int iw = image.GetWidth(), ih = image.GetHeight();
  if ((iw == options.width && ih == options.height) || (options.width == 0 && options.height == 0)) {
    // zoom?
    if (options.zoom != 1.0) {
      image = resample(image, int(iw * options.zoom), int(ih * options.zoom));
    } else {
      // already the right size
    }
  } else if (options.height == 0) {
    // width is given, determine height
    int h = options.width * ih / iw;
    image = resample(image, options.width, h);
  } else if (options.width == 0) {
    // height is given, determine width
    int w = options.height * iw / ih;
    image = resample(image, w, options.height);
  } else if (options.preserve_aspect == ASPECT_FIT) {
    // determine actual size of resulting image
    int w, h;
    if (iw * options.height > ih * options.width) { // too much height requested
      w = options.width;
      h = options.width * ih / iw;
    } else {
      w = options.height * iw / ih;
      h = options.height;
    }
    image = resample(image, w, h);
  } else {
    if (options.preserve_aspect == ASPECT_BORDER && (options.width < options.height * 3) && (options.height < options.width * 3)) {
      // preserve the aspect ratio if there is not too much difference
      image = resample_preserve_aspect(image, options.width, options.height);
    } else {
      image = resample(image, options.width, options.height);
    }
  }
  // saturate?
  if (options.saturate) {
    saturate(image, .1);
  }
  options.width  = image.GetWidth();
  options.height = image.GetHeight();
  // rotate?
  if (options.angle != 0) {
    image = rotate_image(image, options.angle);
  }
  return image;
}

// ----------------------------------------------------------------------------- : BlankImage

Image BlankImage::generate(const Options& opt) const {
  int w = max(1, opt.width >= 0  ? opt.width  : opt.height);
  int h = max(1, opt.height >= 0 ? opt.height : opt.width);
  Image img(w, h);
  assert(img.Ok());
  img.InitAlpha();
  memset(img.GetAlpha(), 0, w * h);
  return img;
}
bool BlankImage::operator == (const GeneratedImage& that) const {
  const BlankImage* that2 = dynamic_cast<const BlankImage*>(&that);
  return that2;
}

// ----------------------------------------------------------------------------- : LinearBlendImage

Image LinearBlendImage::generate(const Options& opt) const {
  Image img = image1->generate(opt);
  linear_blend(img, image2->generate(opt), x1, y1, x2, y2);
  return img;
}
ImageCombine LinearBlendImage::combine() const {
  return image1->combine();
}
bool LinearBlendImage::operator == (const GeneratedImage& that) const {
  const LinearBlendImage* that2 = dynamic_cast<const LinearBlendImage*>(&that);
  return that2 && *image1 == *that2->image1
               && *image2 == *that2->image2
               && x1 == that2->x1 && y1 == that2->y1
               && x2 == that2->x2 && y2 == that2->y2;
}

// ----------------------------------------------------------------------------- : MaskedBlendImage

Image MaskedBlendImage::generate(const Options& opt) const {
  Image img = light->generate(opt);
  mask_blend(img, dark->generate(opt), mask->generate(opt));
  return img;
}
ImageCombine MaskedBlendImage::combine() const {
  return light->combine();
}
bool MaskedBlendImage::operator == (const GeneratedImage& that) const {
  const MaskedBlendImage* that2 = dynamic_cast<const MaskedBlendImage*>(&that);
  return that2 && *light == *that2->light
               && *dark  == *that2->dark
               && *mask  == *that2->mask;
}

// ----------------------------------------------------------------------------- : CombineBlendImage

Image CombineBlendImage::generate(const Options& opt) const {
  Image img = image1->generate(opt);
  combine_image(img, image2->generate(opt), image_combine);
  return img;
}
ImageCombine CombineBlendImage::combine() const {
  return image1->combine();
}
bool CombineBlendImage::operator == (const GeneratedImage& that) const {
  const CombineBlendImage* that2 = dynamic_cast<const CombineBlendImage*>(&that);
  return that2 && *image1 == *that2->image1
               && *image2 == *that2->image2
               && image_combine == that2->image_combine;
}

// ----------------------------------------------------------------------------- : OverlayImage

Image OverlayImage::generate(const Options& opt) const {
  Image img = image1->generate(opt);
  Image img2 = image2->generate(opt);
  if (img.GetWidth() < img2.GetWidth() + offset_x || img.GetHeight() < img2.GetHeight() + offset_y)
    throw ScriptError(_("Overlayed image is out of bounds"));

  int off_x = offset_x,  off_y = offset_y;
  int w1 = img.GetWidth(), h1 = img.GetHeight();  // original image size
  int w2 = img2.GetWidth(), h2 = img2.GetHeight();  // overlay image size
  Byte* data1 = img.GetData(), *data2 = img2.GetData();
  Byte* alpha1 = img.GetAlpha(), *alpha2 = img2.GetAlpha();

  // if image2 has alpha, take it into account
  if (alpha2) {
    for (int x = 0; x < w2; ++x) {
      for (int y = 0; y < h2; ++y) {
        Byte a = 255 - alpha2[x + y * w2];
        for (int b = 0; b < 3; ++b) { // each pixel is 3 bytes
          Byte& d1 = data1[((y + off_y) * w1 + x + off_x) * 3 + b];
          Byte d2 = data2[(y * w2 + x) * 3 + b];
          d1 = (d1 * a + d2 * (255 - a)) / 255; // copied from mask_blend()
        }
      }
    }

    if (alpha1) {
      for (int x = 0; x < w2; ++x) {
        for (int y = 0; y < h2; ++y) {
          int idx1 = (y + off_y) * w1 + x + off_x;
		  alpha1[idx1] = max(alpha1[idx1], alpha2[x + y * w2]);
        }
      }
    }
  // otherwise, we can copy the data directly
  } else {
    for (int y = 0; y < h2; ++y) {
      memcpy(data1 + 3 * ((y + off_y) * w1 + off_x), data2 + 3 * y * w2, 3 * w2); // copy a line
    }
    if (alpha1) {
      for (int y = 0; y < h2; ++y) {
        memset(alpha1 + (y + off_y) * w1 + off_x, w2, 255);
      }
    }
  }

  return img;
}
ImageCombine OverlayImage::combine() const {
	return image1->combine();
}
bool OverlayImage::operator == (const GeneratedImage& that) const {
  const OverlayImage* that2 = dynamic_cast<const OverlayImage*>(&that);
  return that2 && image1 == that2->image1 && image2 == that2->image2
               && offset_x == that2->offset_x && offset_y == that2->offset_y;
}

// ----------------------------------------------------------------------------- : SetMaskImage

Image SetMaskImage::generate(const Options& opt) const {
  Image img = image->generate(opt);
  set_alpha(img, mask->generate(opt));
  return img;
}
bool SetMaskImage::operator == (const GeneratedImage& that) const {
  const SetMaskImage* that2 = dynamic_cast<const SetMaskImage*>(&that);
  return that2 && *image == *that2->image
               && *mask  == *that2->mask;
}

Image SetAlphaImage::generate(const Options& opt) const {
  Image img = image->generate(opt);
  set_alpha(img, alpha);
  return img;
}
bool SetAlphaImage::operator == (const GeneratedImage& that) const {
  const SetAlphaImage* that2 = dynamic_cast<const SetAlphaImage*>(&that);
  return that2 && *image == *that2->image
               && alpha  == that2->alpha;
}

// ----------------------------------------------------------------------------- : SetCombineImage

Image SetCombineImage::generate(const Options& opt) const {
  return image->generate(opt);
}
ImageCombine SetCombineImage::combine() const {
  return image_combine;
}
bool SetCombineImage::operator == (const GeneratedImage& that) const {
  const SetCombineImage* that2 = dynamic_cast<const SetCombineImage*>(&that);
  return that2 && *image == *that2->image
               && image_combine == that2->image_combine;
}

// ----------------------------------------------------------------------------- : SaturateImage

Image SaturateImage::generate(const Options& opt) const {
  Image img = image->generate(opt);
  saturate(img, amount);
  return img;
}
bool SaturateImage::operator == (const GeneratedImage& that) const {
  const SaturateImage* that2 = dynamic_cast<const SaturateImage*>(&that);
  return that2 && *image == *that2->image
               && amount == that2->amount;
}

// ----------------------------------------------------------------------------- : InvertImage

Image InvertImage::generate(const Options& opt) const {
  Image img = image->generate(opt);
  invert(img);
  return img;
}
bool InvertImage::operator == (const GeneratedImage& that) const {
  const InvertImage* that2 = dynamic_cast<const InvertImage*>(&that);
  return that2 && *image == *that2->image;
}

// ----------------------------------------------------------------------------- : RecolorImage

Image RecolorImage::generate(const Options& opt) const {
  Image img = image->generate(opt);
  recolor(img, color);
  return img;
}
bool RecolorImage::operator == (const GeneratedImage& that) const {
  const RecolorImage* that2 = dynamic_cast<const RecolorImage*>(&that);
  return that2 && *image == *that2->image
               && color == that2->color;
}

Image RecolorImage2::generate(const Options& opt) const {
  Image img = image->generate(opt);
  recolor(img, red,green,blue,white);
  return img;
}
bool RecolorImage2::operator == (const GeneratedImage& that) const {
  const RecolorImage2* that2 = dynamic_cast<const RecolorImage2*>(&that);
  return that2 && *image == *that2->image
               && red == that2->red
               && green == that2->green
               && blue == that2->blue
               && white == that2->white;
}

// ----------------------------------------------------------------------------- : FlipImage

Image FlipImageHorizontal::generate(const Options& opt) const {
  Image img = image->generate(opt);
  return flip_image_horizontal(img);
}
bool FlipImageHorizontal::operator == (const GeneratedImage& that) const {
  const FlipImageHorizontal* that2 = dynamic_cast<const FlipImageHorizontal*>(&that);
  return that2 && *image == *that2->image;
}

Image FlipImageVertical::generate(const Options& opt) const {
  Image img = image->generate(opt);
  return flip_image_vertical(img);
}
bool FlipImageVertical::operator == (const GeneratedImage& that) const {
  const FlipImageVertical* that2 = dynamic_cast<const FlipImageVertical*>(&that);
  return that2 && *image == *that2->image;
}

Image RotateImage::generate(const Options& opt) const {
  Image img = image->generate(opt);
  return rotate_image(img,angle);
}
bool RotateImage::operator == (const GeneratedImage& that) const {
  const RotateImage* that2 = dynamic_cast<const RotateImage*>(&that);
  return that2 && *image == *that2->image
               && angle == that2->angle;
}

// ----------------------------------------------------------------------------- : EnlargeImage

Image EnlargeImage::generate(const Options& opt) const {
  // generate 'sub' image
  Options sub_opt
    ( int(opt.width  * (border_size < 0.5 ? 1 - 2 * border_size : 0))
    , int(opt.height * (border_size < 0.5 ? 1 - 2 * border_size : 0))
    , opt.package
    , opt.local_package
    , opt.preserve_aspect);
  Image img = image->generate(sub_opt);
  // size of generated image
  int w  = img.GetWidth(),  h = img.GetHeight();  // original image size
  int dw = int(w * border_size), dh = int(h * border_size); // delta
  int w2 = w + dw + dw,     h2 = h + dh + dh;     // new image size
  Image larger(w2,h2);
  larger.InitAlpha();
  memset(larger.GetAlpha(),0,w2*h2); // blank
  // copy to sub-part of larger image
  Byte* data1 = img.GetData(), *data2 = larger.GetData();
  for (int y = 0 ; y < h ; ++y) {
    memcpy(data2 + 3*(dw + (y+dh)*w2), data1 + 3*y*w, 3*w); // copy a line
  }
  if (img.HasAlpha()) {
    data1 = img.GetAlpha(), data2 = larger.GetAlpha();
    for (int y = 0 ; y < h ; ++y) {
      memcpy(data2 + dw + (y+dh)*w2, data1 + y*w, w); // copy a line
    }
  }
  // done
  return larger;
}
bool EnlargeImage::operator == (const GeneratedImage& that) const {
  const EnlargeImage* that2 = dynamic_cast<const EnlargeImage*>(&that);
  return that2 && *image      == *that2->image
               && border_size == that2->border_size;
}

// ----------------------------------------------------------------------------- : CropImage

Image CropImage::generate(const Options& opt) const {
  return image->generate(opt).Size(wxSize((int)width, (int)height), wxPoint(-(int)offset_x, -(int)offset_y));
}
bool CropImage::operator == (const GeneratedImage& that) const {
  const CropImage* that2 = dynamic_cast<const CropImage*>(&that);
  return that2 && *image      == *that2->image
               && width    == that2->width    && height   == that2->height
               && offset_x == that2->offset_x && offset_y == that2->offset_y;
}

// ----------------------------------------------------------------------------- : ResizeImage

IMPLEMENT_REFLECTION_ENUM(wxImageResizeQuality) {
  VALUE_N("nearest", wxIMAGE_QUALITY_NEAREST);
  VALUE_N("bilinear", wxIMAGE_QUALITY_BILINEAR);
  VALUE_N("bicubic", wxIMAGE_QUALITY_BICUBIC);
  VALUE_N("box average", wxIMAGE_QUALITY_BOX_AVERAGE);
}

Image ResizeImage::generate(const Options& opt) const {
  return image->generate(opt).Rescale((int)width, (int)height, resize_quality);
}
bool ResizeImage::operator == (const GeneratedImage& that) const {
  const ResizeImage* that2 = dynamic_cast<const ResizeImage*>(&that);
  return that2 && *image == *that2->image
               && width == that2->width
               && height == that2->height
               && resize_quality == that2->resize_quality;
}

// ----------------------------------------------------------------------------- : DropShadowImage

/// Preform a gaussian blur, from the image in of w*h bytes to out
/** out is scaled some scaling, this is the return value */
UInt gaussian_blur(Byte* in, UInt* out, int w, int h, double radius) {
  // blur horizontally
  auto blur_x = make_unique<UInt[]>(w*h); // scaled by total_x, so in [0..255*total_x]
  memset(blur_x.get(), 0, w*h*sizeof(UInt));
  UInt total_x = 0;
  {
    double sigma = radius * w;
    double mult = (1 << 8) / (sqrt(2 * M_PI) * sigma);
    double sigsqr2 = 1 / (2 * sigma * sigma);
    int range = min(w, (int)(3*sigma));
    for (int d = -range ; d <= range ; ++d) {
      UInt factor = (int)( mult * exp(-d * d * sigsqr2) );
      total_x += factor;
      if (factor > 0) {
        int x_start = max(0, -d), x_end = min(w, w-d);
        for (int y = 0 ; y < h ; ++y) {
          for (int x = x_start ; x < x_end ; ++x) {
            blur_x[x + y*w] += in[x + d + y*w] * factor;
          }
        }
      }
    }
  }
  // blur vertically
  memset(out, 0, w*h*sizeof(UInt));
  UInt total_y = 0;
  {
    double sigma = radius * h;
    double mult = (1 << 8) / (sqrt(2 * M_PI) * sigma);
    double sigsqr2 = 1 / (2 * sigma * sigma);
    int range = min(h, (int)(3*sigma));
    for (int d = -range ; d <= range ; ++d) {
      UInt factor = (UInt)( mult * exp(-d * d * sigsqr2) );
      total_y += factor;
      if (factor > 0) {
        int y_start = max(0, -d), y_end = min(h, h-d);
        for (int y = y_start ; y < y_end ; ++y) {
          for (int x = 0 ; x < w ; ++x) {
            out[x + y*w] += blur_x[x + (d + y)*w] * factor;
          }
        }
      }
    }
  }
  return total_x * total_y;
}

Image DropShadowImage::generate(const Options& opt) const {
  // sub image
  Image img = image->generate(opt);
  if (!img.HasAlpha()) {
    // no alpha, there is nothing we can do
    return img;
  }
  int w = img.GetWidth(), h = img.GetHeight();
  Byte* alpha = img.GetAlpha();
  // blur
  auto shadow = make_unique<UInt[]>(w*h);
  UInt total = 255 * gaussian_blur(alpha, shadow.get(), w, h, shadow_blur_radius);
  // combine
  Byte* data = img.GetData();
  int dw = int(w * offset_x), dh = int(h * offset_y);
  int x_start = max(0,   dw), y_start = max(0,   dh);
  int x_end   = min(w, w+dw), y_end   = min(h, h+dh);
  int delta = dw + w * dh;
  int sa = (int)(shadow_alpha * (1 << 16));
  for (int y = y_start ; y < y_end ; ++y) {
    for (int x = x_start ; x < x_end ; ++x) {
      int p  = x + y * w; // pixel we are working on
      int a = alpha[p];
      int shad = ((((255 - a)*sa)>>16) * shadow[p - delta]) / total; // amount of shadow to add
      int factor = max(1, a + shad); // divide by this
      data[3 * p    ] = (a * data[3 * p    ] + shad * shadow_color.Red()  ) / factor;
      data[3 * p + 1] = (a * data[3 * p + 1] + shad * shadow_color.Green()) / factor;
      data[3 * p + 2] = (a * data[3 * p + 2] + shad * shadow_color.Blue() ) / factor;
      alpha[p] = a + shad;
    }
  }
  return img;
}
bool DropShadowImage::operator == (const GeneratedImage& that) const {
  const DropShadowImage* that2 = dynamic_cast<const DropShadowImage*>(&that);
  return that2 && *image   == *that2->image
               && offset_x == that2->offset_x && offset_y == that2->offset_y
               && shadow_alpha == that2->shadow_alpha && shadow_blur_radius == that2->shadow_blur_radius
               && shadow_color == that2->shadow_color;
}

// ----------------------------------------------------------------------------- : PackagedImage

Image PackagedImage::generate(const Options& opt) const {
  // TODO : use opt.width and opt.height?
  // open file from package
  if (!opt.package) throw ScriptError(_("Can only load images in a context where an image is expected"));
  auto file_stream = opt.package->openIn(filename);
  Image img;
  if (image_load_file(img, *file_stream)) {
    if (img.HasMask()) img.InitAlpha(); // we can't handle masks
    return img;
  } else {
    throw ScriptError(_("Unable to load image '") + filename + _("' from '" + opt.package->name() + _("'")));
  }
}
bool PackagedImage::operator == (const GeneratedImage& that) const {
  const PackagedImage* that2 = dynamic_cast<const PackagedImage*>(&that);
  return that2 && filename == that2->filename;
}

// ----------------------------------------------------------------------------- : BuiltInImage

Image BuiltInImage::generate(const Options& opt) const {
  // TODO : use opt.width and opt.height?
  try {
    Image img = load_resource_image(name);
    if (img.Ok()) return img;
  } catch (...) {}
  throw ScriptError(_("There is no built in image '") + name + _("'"));
}
bool BuiltInImage::operator == (const GeneratedImage& that) const {
  const BuiltInImage* that2 = dynamic_cast<const BuiltInImage*>(&that);
  return that2 && name == that2->name;
}

// ----------------------------------------------------------------------------- : SymbolToImage

SymbolToImage::SymbolToImage(bool is_local, const LocalFileName& filename, Age age, const SymbolVariationP& variation)
  : is_local(is_local), filename(filename), age(age), variation(variation)
{}
SymbolToImage::~SymbolToImage() {}

Image SymbolToImage::generate(const Options& opt) const {
  // TODO : use opt.width and opt.height?
  Package* package = is_local ? opt.local_package : opt.package;
  if (!package) throw ScriptError(_("Can only load images in a context where an image is expected"));
  SymbolP the_symbol;
  if (filename.empty()) {
    the_symbol = default_symbol();
  } else {
    the_symbol = package->readFile<SymbolP>(filename);
  }
  int size = max(100, 3*max(opt.width,opt.height));
  if (opt.width <= 1 || opt.height <= 1) {
    return render_symbol(the_symbol, *variation->filter, variation->border_radius, size, size);
  } else {
    int width  = size * opt.width  / max(opt.width,opt.height);
    int height = size * opt.height / max(opt.width,opt.height);
    return render_symbol(the_symbol, *variation->filter, variation->border_radius, width, height, false, true);
  }
}
bool SymbolToImage::operator == (const GeneratedImage& that) const {
  const SymbolToImage* that2 = dynamic_cast<const SymbolToImage*>(&that);
  return that2 && is_local  == that2->is_local
               && filename  == that2->filename
               && age       == that2->age
               && (variation == that2->variation ||
                   *variation == *that2->variation // custom variation
                  );
}

// ----------------------------------------------------------------------------- : ImageValueToImage

ImageValueToImage::ImageValueToImage(const LocalFileName& filename, Age age)
  : filename(filename), age(age)
{}
ImageValueToImage::~ImageValueToImage() {}

Image ImageValueToImage::generate(const Options& opt) const {
  // TODO : use opt.width and opt.height?
  if (!opt.local_package) throw ScriptError(_("Can only load images in a context where an image is expected"));
  Image image;
  if (!filename.empty()) {
    auto image_file_stream = opt.local_package->openIn(filename);
    image_load_file(image, *image_file_stream);
  }
  if (!image.Ok()) {
    image = Image(max(1,opt.width), max(1,opt.height));
  }
  return image;
}
bool ImageValueToImage::operator == (const GeneratedImage& that) const {
  const ImageValueToImage* that2 = dynamic_cast<const ImageValueToImage*>(&that);
  return that2 && filename == that2->filename
               && age      == that2->age;
}
