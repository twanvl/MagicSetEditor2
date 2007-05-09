//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_SCRIPT_IMAGE
#define HEADER_SCRIPT_IMAGE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/age.hpp>
#include <util/dynamic_arg.hpp>
#include <script/scriptable.hpp>
//%%#include <gfx/gfx.hpp>
#include <gfx/generated_image.hpp>

class Package;
DECLARE_INTRUSIVE_POINTER_TYPE(ScriptImage); //%% OLD
DECLARE_INTRUSIVE_POINTER_TYPE(GeneratedImage); //%% OLD

// ----------------------------------------------------------------------------- : ScriptableImage

/// An image that can also be scripted
/** Differs from Scriptable<Image> in that:
 *   - A script is always used
 *   - Age is checked, chached images are used if possible
 *   - The image can be scaled
 */
class ScriptableImage2 {
  public:
	inline ScriptableImage2() {}
	inline ScriptableImage2(const String& script) : script(script) {}
	
	/// Is there an image set?
	inline bool isScripted() const { return script; }
	/// Is there an image generator available?
	inline bool isReady()    const { return value; }
	
	/// Generate an image.
	Image generate(const GeneratedImage::Options& options, bool cache = false) const;
	/// How should images be combined with the background?
	ImageCombine combine() const;
	
	/// Update the script, returns true if the value has changed
	bool update(Context& ctx);
	
	inline void initDependencies(Context& ctx, const Dependency& dep) const {
		script.initDependencies(ctx, dep);
	}
	
  private:
	OptionalScript  script;		///< The script, not really optional
	GeneratedImageP value;		///< The image generator
	mutable Image   cached;		///< The cached actual image
	
	DECLARE_REFLECTION();
};

/// Missing for now
inline ScriptValueP to_script(const ScriptableImage2&) { return script_nil; }

/// Convert a script value to a GeneratedImageP
GeneratedImageP image_from_script(const ScriptValueP& value);

// ----------------------------------------------------------------------------- : ScriptableImage
//%% OLD

DECLARE_DYNAMIC_ARG(Package*, load_images_from);

/// An image, returned by a script function
class ScriptImage : public ScriptValue {
  public:
	inline ScriptImage() : combine(COMBINE_NORMAL) {}
	inline ScriptImage(const Image& image, ImageCombine combine = COMBINE_NORMAL)
		: image(image), combine(combine)
	{}
	
	Image        image;   ///< The image
	ImageCombine combine; ///< How to combine the image with the background
	
	virtual ScriptType type() const;
	virtual String typeName() const;
};


/// An image that can also be scripted
/** Differs from Scriptable<Image> in that:
 *   - A script is always used
 *   - Age is checked, chached images are used if possible
 *   - The image can be scaled
 */
class ScriptableImage {
  public:
	inline ScriptableImage() {}
	ScriptableImage(const String& script);
	
	/// Is there an image set?
	inline operator bool() const { return script; }
	
	/// Generate an image, doesn't cache, and doesn't scale
	/** Image files are loaded from the given package.
	 *  The result is always valid. */
	ScriptImageP generate(Context& ctx, Package&) const;
	/// Generate an image, scaling it and optionally saturating it
	ScriptImageP generate(Context& ctx, Package&, UInt width, UInt height, PreserveAspect preserve_aspect = ASPECT_STRETCH, bool saturate = false) const;
	
	/// Update and return the cached image
	/** Only recomputes the image if it is out of date, or the size doesn't match.
	 *  If width==height==0 then doesn't resample.
	 */
	ScriptImageP update(Context& ctx, Package&, UInt width = 0, UInt height = 0, PreserveAspect preserve_aspect = ASPECT_STRETCH, bool saturate = false);
	
	/// Is the cached image up to date?
	bool upToDate(Context& ctx, Age age) const;
	
	inline void initDependencies(Context& ctx, const Dependency& dep) const {
		script.initDependencies(ctx, dep);
	}
	/// Invalidate the cached image
	inline void invalidate() {
		cache = ScriptImageP();
	}
	
  private:
	OptionalScript script;		///< The script, not really optional
	ScriptImageP   cache;		///< The cached image
	Age            last_update;	///< Age of last image update of the cached image
	
	DECLARE_REFLECTION();
};

/// Missing for now
inline ScriptValueP to_script(const ScriptableImage&) { return script_nil; }

/// Convert a script value to an image
ScriptImageP to_script_image(const ScriptValueP& value);

/// Is the given image up to date?
bool script_image_up_to_date(const ScriptValueP& value);

// ----------------------------------------------------------------------------- : EOF
#endif
