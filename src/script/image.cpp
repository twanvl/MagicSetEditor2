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


// ----------------------------------------------------------------------------- : CachedScriptableImage

void CachedScriptableImage::generateCached(const GeneratedImage::Options& options,
	                                       Image* mask,
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
		if (cached_i.Ok() && (options.angle - cached_angle + 360) % 90 == 0) {
			if ((w_ok && h_ok) || (options.preserve_aspect == ASPECT_FIT && (w_ok || h_ok))) { // only one dimension has to fit when fitting
				if (options.angle != cached_angle) {
					// rotate cached image
					cached_i = rotate_image(cached_i, options.angle - cached_angle + 360);
					cached_angle = options.angle;
				}
				*image = cached_i;
				return;
			}
		}
	}
	// generate
	cached_i = generate(options);
	cached_angle = options.angle;
	*size = cached_size = RealSize(options.width, options.height);
	if (mask && mask->Ok()) {
		// apply mask
		if (mask->GetWidth() == cached_i.GetWidth() && mask->GetHeight() == cached_i.GetHeight()) {
			set_alpha(cached_i, *mask);
		} else {
			Image mask_scaled(cached_i.GetWidth(),cached_i.GetHeight(), false);
			resample(*mask,mask_scaled);
			set_alpha(cached_i, mask_scaled);
		}
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
