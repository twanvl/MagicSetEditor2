//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <script/image.hpp>
#include <script/context.hpp>
#include <script/to_value.hpp>
#include <util/dynamic_arg.hpp>
#include <util/io/package.hpp>
#include <gfx/generated_image.hpp>

// ----------------------------------------------------------------------------- : Utility

// convert any script value to a GeneratedImageP
GeneratedImageP image_from_script(const ScriptValueP& value) {
	if (value->type() == SCRIPT_STRING) {
		return new_intrusive1<PackagedImage>(value->toString());
	} else {
		GeneratedImageP img = dynamic_pointer_cast<GeneratedImage>(value);
		if (!img) throw ScriptError(_ERROR_2_("can't convert", value->typeName(), _TYPE_("image")));
		return img;
	}
}

// ----------------------------------------------------------------------------- : ScriptableImage

Image ScriptableImage::generate(const GeneratedImage::Options& options, bool cache) const {
	if (cached.Ok() && cached.GetWidth() == options.width && cached.GetHeight() == options.height) {
		// cached, so we are done
		return cached;
	}
	// generate blank image
	Image image(1,1);
	image.InitAlpha();
	image.SetAlpha(0,0,0);
	if (isReady()) {
		try {
			image = value->generate(options);
		}
		catch (FileNotFoundError e) {
			handle_error (e);
			return image;
		}
	}
	else {
		return image;
	}
	// resize?
	int iw = image.GetWidth(), ih = image.GetHeight();
	if ((iw == options.width && ih == options.height) || options.width == 0 || options.height == 0) {
		// already the right size
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
		Image resampled_image(w, h, false);
		resample(image, resampled_image);
		image = resampled_image;
	} else {
		Image resampled_image(options.width, options.height, false);
		if (options.preserve_aspect == ASPECT_BORDER && (options.width < options.height * 3) && (options.height < options.width * 3)) {
			// preserve the aspect ratio if there is not too much difference
			resample_preserve_aspect(image, resampled_image);
		} else {
			resample(image, resampled_image);
		}
		image = resampled_image;
	}
	if (options.saturate) {
		saturate(image, 40);
	}
	// cache? and return
	if (cache) cached = image;
	return image;
}

ImageCombine ScriptableImage::combine() const {
	if (!isReady()) return COMBINE_NORMAL;
	return value->combine();
}

bool ScriptableImage::update(Context& ctx) {
	if (!isScripted()) return false;
	GeneratedImageP new_value = image_from_script(script.invoke(ctx));
	if (!new_value || !value || *new_value != *value) {
		value = new_value;
		cached = Image();
		return true;
	} else {
		return false;
	}
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
		s.value = new_intrusive1<PackagedImage>(s.script.unparsed);
	}
}
template <> void Writer::handle(const ScriptableImage& s) {
	handle(s.script.unparsed);
}
template <> void GetDefaultMember::handle(const ScriptableImage& s) {
	handle(s.script.unparsed);
}
