//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <script/image.hpp>
#include <script/context.hpp>
#include <util/dynamic_arg.hpp>
#include <util/io/package.hpp>

// image generating functions have two modes
// if last_update_age >  0 they return whether the image is still up to date
// if last_update_age == 0 they generate the image
DECLARE_DYNAMIC_ARG  (long, last_update_age);
IMPLEMENT_DYNAMIC_ARG(long, last_update_age, 0);
IMPLEMENT_DYNAMIC_ARG(Package*, load_images_from, nullptr);

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
	} else {
		throw ScriptError(_("Can not convert from '") + value->typeName() + _("' to image"));
	}
}

/// Is the given image up to date?
bool script_image_up_to_date(const ScriptValueP& value) {
	if (value->type() == SCRIPT_INT) {
		return (int)*value; // boolean up-to-dateness from parameter
	} else {
		return true;
	}
}


// ----------------------------------------------------------------------------- : ScriptableImage

ScriptImageP ScriptableImage::generate(Context& ctx) const {
	try {
		ScriptImageP img = to_script_image(script.invoke(ctx));
		return img;
	} catch (Error e) {
		// loading images can fail
		// it is likely we are inside a paint function or outside the main thread, handle error later
		handle_error(e, false, false);
		return new_intrusive1<ScriptImage>(Image(1,1));
	}
}

ScriptImageP ScriptableImage::generate(Context& ctx, UInt width, UInt height, PreserveAspect preserve_aspect, bool saturate) const {
	ScriptImageP image = generate(ctx);
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

ScriptImageP ScriptableImage::update(Context& ctx, UInt width, UInt height, PreserveAspect preserve_aspect, bool saturate) {
	// up to date?
	if (!cache || (UInt)cache->image.GetWidth() != width || (UInt)cache->image.GetHeight() != height || !upToDate(ctx, last_update)) {
		// cache must be updated
		cache = generate(ctx, width, height, preserve_aspect, saturate);
		last_update.update();
	}
	return cache;
}

bool ScriptableImage::upToDate(Context& ctx, Age age) const {
	try {
		WITH_DYNAMIC_ARG(last_update_age, age.get());
		return (int)*script.invoke(ctx);
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
	} else {
		// script is a constant function
		s.script.script = new_intrusive<Script>();
		s.script.script->addInstruction(I_PUSH_CONST, s.script.unparsed);
	}
}
template <> void Writer::handle(const ScriptableImage& s) {
	handle(s.script.unparsed);
}
template <> void GetDefaultMember::handle(const ScriptableImage& s) {
	handle(s.script.unparsed);
}


// ----------------------------------------------------------------------------- : Functions

SCRIPT_FUNCTION(linear_blend) {
	if (last_update_age() == 0) {
		ScriptImageP image1 = to_script_image(ctx.getVariable(_("image1")));
		ScriptImageP image2 = to_script_image(ctx.getVariable(_("image2")));
		SCRIPT_PARAM(double, x1); SCRIPT_PARAM(double, y1);
		SCRIPT_PARAM(double, x2); SCRIPT_PARAM(double, y2);
		linear_blend(image1->image, image2->image, x1, y1, x2, y2);
		return image1;
	} else {
		SCRIPT_RETURN(
			script_image_up_to_date(ctx.getVariable(_("image1"))) &&
			script_image_up_to_date(ctx.getVariable(_("image2")))
		);
	}
}

SCRIPT_FUNCTION(masked_blend) {
	if (last_update_age() == 0) {
		ScriptImageP light = to_script_image(ctx.getVariable(_("light")));
		ScriptImageP dark  = to_script_image(ctx.getVariable(_("dark")));
		ScriptImageP mask  = to_script_image(ctx.getVariable(_("mask")));
		mask_blend(light->image, dark->image, mask->image);
		return light;
	} else {
		SCRIPT_RETURN(
			script_image_up_to_date(ctx.getVariable(_("light"))) &&
			script_image_up_to_date(ctx.getVariable(_("dark" ))) &&
			script_image_up_to_date(ctx.getVariable(_("mask" )))
		);
	}
}

SCRIPT_FUNCTION(set_mask) {
	if (last_update_age() == 0) {
		ScriptImageP image = to_script_image(ctx.getVariable(_("image")));
		ScriptImageP mask  = to_script_image(ctx.getVariable(_("mask")));
		set_alpha(image->image, mask->image);
		return image;
	} else {
		SCRIPT_RETURN(
			script_image_up_to_date(ctx.getVariable(_("image"))) &&
			script_image_up_to_date(ctx.getVariable(_("mask")))
		);
	}
}
