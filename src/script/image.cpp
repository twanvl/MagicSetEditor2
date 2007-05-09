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

IMPLEMENT_DYNAMIC_ARG(Package*, load_images_from, nullptr);

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

// ----------------------------------------------------------------------------- : ScriptableImage2

Image ScriptableImage2::generate(const GeneratedImage::Options& options, bool cache) const {
	if (!isReady()) {
		// error, return blank image
		Image i(1,1);
		i.InitAlpha();
		i.SetAlpha(0,0,0);
		return i;
	}
	if (cached.Ok() && cached.GetWidth() == options.width && cached.GetHeight() == options.height) {
		return cached;
	}
	Image img = value->generate(options);
	if (cache) cached = img;
	return img;
}

ImageCombine ScriptableImage2::combine() const {
	if (!isReady()) return COMBINE_NORMAL;
	return value->combine();
}

bool ScriptableImage2::update(Context& ctx) {
	if (!isScripted()) return false;
	GeneratedImageP new_value = image_from_script(script.invoke(ctx));
	if (!new_value || !value || *new_value != *value) {
		value = new_value;
		return true;
	} else {
		return false;
	}
}






// ----------------------------------------------------------------------------- : ScriptImage

ScriptType ScriptImage::type() const { return SCRIPT_IMAGE; }
String ScriptImage::typeName() const { return _("image"); }

// ----------------------------------------------------------------------------- : Utility

/// Convert a script value to an image
ScriptImageP to_script_image(const ScriptValueP& value) {
	if (ScriptImageP img = dynamic_pointer_cast<ScriptImage>(value)) {
		return img; // already an image
	} else if (value->type() == SCRIPT_STRING) {
		// open a file
		String filename = *value;
		Package* pkg = load_images_from();
		if (!pkg) throw ScriptError(_("Can only load images in a context where an image is expected"));
		InputStreamP file = pkg->openIn(filename);
		ScriptImageP img = new_intrusive<ScriptImage>();
		if (img->image.LoadFile(*file)) {
			if (img->image.HasMask()) img->image.InitAlpha(); // we can't handle masks
			return img;
		} else {
			throw ScriptError(_("Unable to load image '") + filename + _("' from '" + pkg->name() + _("'")));
		}
	} else if (value->type() == SCRIPT_NIL) {
		// error, return blank image
		Image i(1,1);
		i.InitAlpha();
		i.SetAlpha(0,0,0);
		return new_intrusive1<ScriptImage>(i);
	} else {
		throw ScriptError(_("Can not convert from '") + value->typeName() + _("' to image"));
	}
}

/// Is the given image up to date?
bool script_image_up_to_date(const ScriptValueP& value) {
	if (value->type() == SCRIPT_INT) {
		return (bool)*value; // boolean up-to-dateness from parameter
	} else {
		return true;
	}
}


// ----------------------------------------------------------------------------- : ScriptableImage

ScriptableImage::ScriptableImage(const String& script_)
	: script(script_)
{}

ScriptImageP ScriptableImage::generate(Context& ctx, Package& pkg) const {
	try {
		WITH_DYNAMIC_ARG(load_images_from, &pkg);
		ScriptImageP img = to_script_image(script.invoke(ctx));
		return img;
	} catch (Error e) {
		// loading images can fail
		// it is likely we are inside a paint function or outside the main thread, handle error later
		handle_error(e, false, false);
		return new_intrusive1<ScriptImage>(Image(1,1));
	}
}

ScriptImageP ScriptableImage::generate(Context& ctx, Package& pkg, UInt width, UInt height, PreserveAspect preserve_aspect, bool saturate) const {
	ScriptImageP image = generate(ctx, pkg);
	if (!image->image.Ok()) {
		// return an image so we don't fail
		image->image = Image(1,1);
	}
	UInt iw = image->image.GetWidth(), ih = image->image.GetHeight();
	if ((iw == width && ih == height) || width == 0) {
		// already the right size
	} else if (preserve_aspect == ASPECT_FIT) {
		// determine actual size of resulting image
		UInt w, h;
		if (iw * height > ih * width) { // too much height requested
			w = width;
			h = width * ih / iw;
		} else {
			w = height * iw / ih;
			h = height;
		}
		Image resampled_image(w, h, false);
		resample(image->image, resampled_image);
		image->image = resampled_image;
	} else {
		Image resampled_image(width, height, false);
		if (preserve_aspect == ASPECT_BORDER && (width < height * 3) && (height < width * 3)) {
			// preserve the aspect ratio if there is not too much difference
			resample_preserve_aspect(image->image, resampled_image);
		} else {
			resample(image->image, resampled_image);
		}
		image->image = resampled_image;
	}
	if (saturate) {
		::saturate(image->image, 40);
	}
	return image;
}

ScriptImageP ScriptableImage::update(Context& ctx, Package& pkg, UInt width, UInt height, PreserveAspect preserve_aspect, bool saturate) {
	// up to date?
	if (!cache || (UInt)cache->image.GetWidth() != width || (UInt)cache->image.GetHeight() != height || !upToDate(ctx, last_update)) {
		// cache must be updated
		cache = generate(ctx, pkg, width, height, preserve_aspect, saturate);
		last_update.update();
	}
	return cache;
}

bool ScriptableImage::upToDate(Context& ctx, Age age) const {
	if (!script) return true;
	try {
		WITH_DYNAMIC_ARG(last_update_age, age.get());
		return script_image_up_to_date(script.invoke(ctx));
	} catch (Error e) {
		return true; // script gives errors, don't update
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
		// script is a constant function
		s.script.script = new_intrusive<Script>();
		s.script.script->addInstruction(I_PUSH_CONST, s.script.unparsed);
		s.script.script->addInstruction(I_RET);
	}
}
template <> void Writer::handle(const ScriptableImage& s) {
	handle(s.script.unparsed);
}
template <> void GetDefaultMember::handle(const ScriptableImage& s) {
	handle(s.script.unparsed);
}




// ----------------------------------------------------------------------------- : Reflection

// we need some custom io, because the behaviour is different for each of Reader/Writer/GetMember

template <> void Reader::handle(ScriptableImage2& s) {
	handle(s.script.unparsed);
	if (starts_with(s.script.unparsed, _("script:"))) {
		s.script.unparsed = s.script.unparsed.substr(7);
		s.script.parse(*this);
	} else if (s.script.unparsed.find_first_of('{') != String::npos) {
		s.script.parse(*this, true);
	} else {
		// script is a constant function
		s.script.script = new_intrusive<Script>();
		s.script.script->addInstruction(I_PUSH_CONST, s.script.unparsed);
		s.script.script->addInstruction(I_RET);
	}
}
template <> void Writer::handle(const ScriptableImage2& s) {
	handle(s.script.unparsed);
}
template <> void GetDefaultMember::handle(const ScriptableImage2& s) {
	handle(s.script.unparsed);
}
