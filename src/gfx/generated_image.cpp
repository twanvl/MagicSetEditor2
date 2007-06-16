//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

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

// ----------------------------------------------------------------------------- : BlankImage

Image BlankImage::generate(const Options& opt) const {
	Image img(opt.width, opt.height);
	img.InitAlpha();
	memset(img.GetAlpha(), 0, opt.width * opt.height);
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

// ----------------------------------------------------------------------------- : SetMaskImage

Image SetMaskImage::generate(const Options& opt) const {
	Image img = image->generate(opt);
	set_alpha(img, mask->generate(opt));
	return img;
}
ImageCombine SetMaskImage::combine() const {
	return image->combine();
}
bool SetMaskImage::operator == (const GeneratedImage& that) const {
	const SetMaskImage* that2 = dynamic_cast<const SetMaskImage*>(&that);
	return that2 && *image == *that2->image
	             && *mask  == *that2->mask;
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

// ----------------------------------------------------------------------------- : PackagedImage

Image PackagedImage::generate(const Options& opt) const {
	// TODO : use opt.width and opt.height?
	// open file from package
	if (!opt.package) throw ScriptError(_("Can only load images in a context where an image is expected"));
	InputStreamP file = opt.package->openIn(filename);
	Image img;
	if (img.LoadFile(*file)) {
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
	Image img = load_resource_image(name);
	if (!img.Ok()) {
		throw ScriptError(_("There is no build in image '") + name + _("'"));
	}
	return img;
}
bool BuiltInImage::operator == (const GeneratedImage& that) const {
	const BuiltInImage* that2 = dynamic_cast<const BuiltInImage*>(&that);
	return that2 && name == that2->name;
}

// ----------------------------------------------------------------------------- : SymbolToImage

SymbolToImage::SymbolToImage(const String& filename, Age age, const SymbolVariationP& variation)
	: filename(filename), age(age), variation(variation)
{}
SymbolToImage::~SymbolToImage() {}

Image SymbolToImage::generate(const Options& opt) const {
	// TODO : use opt.width and opt.height?
	if (!opt.local_package) throw ScriptError(_("Can only load images in a context where an image is expected"));
	SymbolP the_symbol;
	if (filename.empty()) {
		the_symbol = default_symbol();
	} else {
		the_symbol = opt.local_package->readFile<SymbolP>(filename);
	}
	return render_symbol(the_symbol, *variation->filter, variation->border_radius);
}
bool SymbolToImage::operator == (const GeneratedImage& that) const {
	const SymbolToImage* that2 = dynamic_cast<const SymbolToImage*>(&that);
	return that2 && filename  == that2->filename
	             && age       == that2->age
	             && variation == that2->variation;
}

// ----------------------------------------------------------------------------- : ImageValueToImage

ImageValueToImage::ImageValueToImage(const String& filename, Age age)
	: filename(filename), age(age)
{}
ImageValueToImage::~ImageValueToImage() {}

Image ImageValueToImage::generate(const Options& opt) const {
	// TODO : use opt.width and opt.height?
	if (!opt.local_package) throw ScriptError(_("Can only load images in a context where an image is expected"));
	Image image;
	if (!filename.empty()) {
		InputStreamP image_file = opt.local_package->openIn(filename);
		image.LoadFile(*image_file);
	}
	if (!image.Ok()) {
		image = Image(opt.width, opt.height);
	}
	return image;
}
bool ImageValueToImage::operator == (const GeneratedImage& that) const {
	const ImageValueToImage* that2 = dynamic_cast<const ImageValueToImage*>(&that);
	return that2 && filename == that2->filename
	             && age      == that2->age;
}
