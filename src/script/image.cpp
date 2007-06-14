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
#include <data/field/image.hpp>

// ----------------------------------------------------------------------------- : Utility

// convert any script value to a GeneratedImageP
GeneratedImageP image_from_script(const ScriptValueP& value) {
	ScriptType t = value->type();
	if (t == SCRIPT_IMAGE) {
		GeneratedImageP img = dynamic_pointer_cast<GeneratedImage>(value);
		if (img) return img;
	} else if (t == SCRIPT_STRING) {
		return new_intrusive1<PackagedImage>(value->toString());
	} else if (t == SCRIPT_NIL) {
		return new_intrusive<BlankImage>();
	} else if (t == SCRIPT_OBJECT) {
		// maybe it's an image value?
		intrusive_ptr<ScriptObject<ValueP> > v = dynamic_pointer_cast<ScriptObject<ValueP> >(value);
		if (v) {
			ImageValueP iv = dynamic_pointer_cast<ImageValue>(v->getValue());
			if (iv) {
				return new_intrusive2<ImageValueToImage>(iv->filename, iv->last_update);
			}
		}
	}
	throw ScriptError(_ERROR_2_("can't convert", value->typeName(), _TYPE_("image")));
}

// ----------------------------------------------------------------------------- : ScriptableImage

Image ScriptableImage::generate(const GeneratedImage::Options& options, bool cache) const {
	if (cached.Ok() && cached.GetWidth() == options.width && cached.GetHeight() == options.height) {
		// cached, so we are done
		return cached;
	}
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

ScriptP ScriptableImage::getScriptP() {
	if (script) return script.getScriptP();
	// return value or a blank image
	ScriptP s(new Script);
	s->addInstruction(I_PUSH_CONST, value ? static_pointer_cast<ScriptValue>(value) : script_nil);
	s->addInstruction(I_RET);
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
		s.value = new_intrusive1<PackagedImage>(s.script.unparsed);
	}
}
template <> void Writer::handle(const ScriptableImage& s) {
	handle(s.script.unparsed);
}
template <> void GetDefaultMember::handle(const ScriptableImage& s) {
	handle(s.script.unparsed);
}
