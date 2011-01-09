//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <script/image.hpp>
#include <script/context.hpp>
#include <script/to_value.hpp>
#include <util/dynamic_arg.hpp>
#include <util/io/package.hpp>
#include <gfx/generated_image.hpp>
#include <data/field/image.hpp>

// ----------------------------------------------------------------------------- : Utility

// convert any script value to a GeneratedImageP
GeneratedImageP image_from_script(const ScriptValueP& value) {
	return value->toImage(value);
}

// ----------------------------------------------------------------------------- : ScriptableImage

Image ScriptableImage::generate(const GeneratedImage::Options& options) const {
	// generate
	Image image;
	if (isReady()) {
		// note: Don't catch exceptions here, we don't want to return an invalid image.
		//       We could return a blank one, but the thumbnail code does want an invalid
		//       image in case of errors.
		//       This allows the caller to catch errors.
		image = value->generate(options);
	} else {
		// error, return blank image
		Image i(1,1);
		i.InitAlpha();
		i.SetAlpha(0,0,0);
		image = i;
	}
	return conform_image(image, options);
}

ImageCombine ScriptableImage::combine() const {
	if (!isReady()) return COMBINE_DEFAULT;
	return value->combine();
}

bool ScriptableImage::update(Context& ctx) {
	if (!isScripted()) return false;
	GeneratedImageP new_value = image_from_script(script.invoke(ctx));
	if (!new_value || !value || *new_value != *value) {
		value = new_value;
		return true;
	} else {
		return false;
	}
}

ScriptP ScriptableImage::getValidScriptP() {
	if (script) return script.getScriptP();
	// return value or a blank image
	ScriptP s(new Script);
	s->addInstruction(I_PUSH_CONST, value ? static_pointer_cast<ScriptValue>(value) : script_nil);
	return s;
}

// ----------------------------------------------------------------------------- : Reflection

// we need some custom io, because the behaviour is different for each of Reader/Writer/GetMember

template <> void Reader::handle(ScriptableImage& s) {
	handle(s.script.unparsed);
	if (starts_with(s.script.unparsed, _("script:"))) {
		s.script.unparsed = s.script.unparsed.substr(7);
		s.script.parse(*this);
	} else if (s.script.unparsed.find_first_of('{') != String::npos) {
		s.script.parse(*this, true);
	} else {
		// a filename
		s.value = intrusive(new PackagedImage(s.script.unparsed));
	}
}
template <> void Writer::handle(const ScriptableImage& s) {
	handle(s.script.unparsed);
}
template <> void GetDefaultMember::handle(const ScriptableImage& s) {
	handle(s.script.unparsed);
}


// ----------------------------------------------------------------------------- : CachedScriptableImage

void CachedScriptableImage::generateCached(const GeneratedImage::Options& options,
	                                       CachedScriptableMask* mask,
	                                       ImageCombine* combine, wxBitmap* bitmap, wxImage* image, RealSize* size) {
	// ready?
	if (!isReady()) {
		// error, return blank image
		Image i(1,1);
		i.InitAlpha();
		i.SetAlpha(0,0,0);
		*image = i;
		*size = RealSize(0,0);
		return;
	}
	// find combine mode
	ImageCombine combine_i = value->combine();
	if (combine_i != COMBINE_DEFAULT) *combine = combine_i;
	*size = cached_size;
	// does the size match?
	bool w_ok = cached_size.width  == options.width,
	     h_ok = cached_size.height == options.height;
	// image or bitmap?
	if (*combine <= COMBINE_NORMAL) {
		// bitmap
		if (cached_b.Ok() && options.angle == cached_angle) {
			if ((w_ok && h_ok) || (options.preserve_aspect == ASPECT_FIT && (w_ok || h_ok))) { // only one dimension has to fit when fitting
				// cached, we are done
				*bitmap = cached_b;
				return;
			}
		}
	} else {
		// image
		Radians relative_rotation = options.angle + rad360 - cached_angle;
		if (cached_i.Ok() && is_straight(relative_rotation)) {
			// we need only an {0,90,180,270} degree rotation compared to the cached one, this doesn't reduce image quality
			if ((w_ok && h_ok) || (options.preserve_aspect == ASPECT_FIT && (w_ok || h_ok))) { // only one dimension has to fit when fitting
				if (options.angle != cached_angle) {
					// rotate cached image
					cached_i = rotate_image(cached_i, relative_rotation);
					cached_angle = options.angle;
				}
				*image = cached_i;
				return;
			}
		}
	}
	// hack(part1): temporarily set angle to 0, do actual rotation after applying mask
	Radians a = options.angle;
	const_cast<GeneratedImage::Options&>(options).angle = 0;
	// generate
	cached_i = generate(options);
	const_cast<GeneratedImage::Options&>(options).angle = cached_angle = a;
	*size = cached_size = RealSize(options.width, options.height);
	if (mask) {
		// apply mask
		GeneratedImage::Options mask_opts(options);
		mask_opts.width  = cached_i.GetWidth();
		mask_opts.height = cached_i.GetHeight();
		mask_opts.angle  = 0;
		mask->get(mask_opts).setAlpha(cached_i);
	}
	if (options.angle != 0) {
		// hack(part2) do the actual rotation now
		cached_i = rotate_image(cached_i, options.angle);
	}
	if (*combine <= COMBINE_NORMAL) {
		*bitmap = cached_b = Bitmap(cached_i);
		cached_i = Image();
	} else {
		*image = cached_i;
	}
}

bool CachedScriptableImage::update(Context& ctx) {
	bool change = ScriptableImage::update(ctx);
	if (change) {
		clearCache();
	}
	return change;
}

void CachedScriptableImage::clearCache() {
	cached_i = Image();
	cached_b = Bitmap();
}


template <> void Reader::handle(CachedScriptableImage& s) {
	handle((ScriptableImage&)s);
}
template <> void Writer::handle(const CachedScriptableImage& s) {
	handle((const ScriptableImage&)s);
}
template <> void GetDefaultMember::handle(const CachedScriptableImage& s) {
	handle((const ScriptableImage&)s);
}


// ----------------------------------------------------------------------------- : CachedScriptableMask


bool CachedScriptableMask::update(Context& ctx) {
	if (script.update(ctx)) {
		mask.clear();
		return true;
	} else {
		return false;
	}
}

const AlphaMask& CachedScriptableMask::get(const GeneratedImage::Options& img_options) {
	if (mask.isLoaded()) {
		// already loaded?
		if (img_options.width == 0 && img_options.height == 0) return mask;
		if (mask.hasSize(wxSize(img_options.width,img_options.height))) return mask;
	}
	// load?
	getNoCache(img_options,mask);
	return mask;
}
void CachedScriptableMask::getNoCache(const GeneratedImage::Options& img_options, AlphaMask& other_mask) const {
	if (script.isBlank()) {
		other_mask.clear();
	} else {
		Image image = script.generate(img_options);
		other_mask.load(image);
	}
}

template <> void Reader::handle(CachedScriptableMask& i) {
	handle(i.script);
}
template <> void Writer::handle(const CachedScriptableMask& i) {
	handle(i.script);
}
template <> void GetDefaultMember::handle(const CachedScriptableMask& i) {
	handle(i.script);
}
