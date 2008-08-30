//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_SCRIPT_IMAGE
#define HEADER_SCRIPT_IMAGE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/age.hpp>
#include <util/dynamic_arg.hpp>
#include <script/scriptable.hpp>
#include <gfx/generated_image.hpp>

class CachedScriptableMask;

// ----------------------------------------------------------------------------- : ScriptableImage

/// An image that can also be scripted
/** Differs from Scriptable<Image> in that:
 *   - A script is always used
 *   - The image can be scaled
 */
class ScriptableImage {
  public:
	inline ScriptableImage() {}
	inline ScriptableImage(const String& script) : script(script) {}
	inline ScriptableImage(const GeneratedImageP& gen) : value(gen) {}
	
	/// Is there a scripted image set?
	inline bool isScripted() const { return script; }
	/// Is there an image generator available?
	inline bool isReady()    const { return value; }
	/// Is there an image set?
	inline bool isSet()      const { return script || value; }
	
	/// Generate an image.
	Image generate(const GeneratedImage::Options& options) const;
	/// How should images be combined with the background?
	ImageCombine combine() const;
	
	/// Update the script, returns true if the value has changed
	bool update(Context& ctx);
	
	inline void initDependencies(Context& ctx, const Dependency& dep) const {
		script.initDependencies(ctx, dep);
	}
	
	/// Can this be safely generated from another thread?
	inline bool threadSafe() const { return !value || value->threadSafe(); }
	/// Is this image specific to the set (the local_package)?
	inline bool local() const { return value && value->local(); }
	/// Is this image blank?
	inline bool isBlank() const { return !value || value->isBlank(); }
	
	/// Get access to the script, be careful
	inline Script& getMutableScript() { return script.getMutableScript(); }
	/// Get access to the script, always returns a valid script
	ScriptP getValidScriptP();
	
  protected:
	OptionalScript  script;		///< The script, not really optional
	GeneratedImageP value;		///< The image generator
	
	DECLARE_REFLECTION();
};

/// Missing for now
inline ScriptValueP to_script(const ScriptableImage&) { return script_nil; }

/// Convert a script value to a GeneratedImageP
GeneratedImageP image_from_script(const ScriptValueP& value);

// ----------------------------------------------------------------------------- : CachedScriptableImage

/// A version of ScriptableImage that does caching
class CachedScriptableImage : public ScriptableImage {
  public:
	inline CachedScriptableImage() {}
	inline CachedScriptableImage(const String& script) : ScriptableImage(script) {}
	inline CachedScriptableImage(const GeneratedImageP& gen) : ScriptableImage(gen) {}
	
	/// Generate an image, using caching if possible.
	/** *combine should be set to the combine value of the style.
	 *  It will be overwritten if the image specifies a non-default combine.
	 *  After this call, either:
	 *     -    combine <= COMBINE_NORMAL && bitmap->Ok()
	 *     - or combine >  COMBINE_NORMAL && image->Ok()
	 *  Optionally, an alpha mask is applied to the image.
	 */
	void generateCached(const GeneratedImage::Options& img_options,
	                    CachedScriptableMask* mask,
	                    ImageCombine* combine, wxBitmap* bitmap, wxImage* image, RealSize* size);
	
	/// Update the script, returns true if the value has changed
	bool update(Context& ctx);
	
	/// Clears the cache
	void clearCache();
	
  private:
	Image  cached_i; ///< The cached image
	Bitmap cached_b; ///< *or* the cached bitmap
	RealSize cached_size; ///< The size of the image before rotating
	int    cached_angle;
};

// ----------------------------------------------------------------------------- : CachedScriptableMask

/// A version of ScriptableImage that caches an AlphaMask
class CachedScriptableMask {
  public:
	
	/// Update the script, returns true if the value has changed
	bool update(Context& ctx);
	
	/// Get the alpha mask; with the given options
	/** if img_options.width == 0 and the mask is already loaded, just returns it. */
	const AlphaMask& get(const GeneratedImage::Options& img_options);
	
	/// Get a mask that is not cached
	void getNoCache(const GeneratedImage::Options& img_options, AlphaMask& mask);
	
  private:
	ScriptableImage script;
	AlphaMask       mask;
	friend class Reader;
	friend class Writer;
	friend class GetDefaultMember;
};

// ----------------------------------------------------------------------------- : EOF
#endif
