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
#include <gfx/generated_image.hpp>

// ----------------------------------------------------------------------------- : ScriptableImage

/// An image that can also be scripted
/** Differs from Scriptable<Image> in that:
 *   - A script is always used
 *   - Age is checked, chached images are used if possible
 *   - The image can be scaled
 */
class ScriptableImage {
  public:
	inline ScriptableImage() {}
	inline ScriptableImage(const String& script) : script(script) {}
	inline ScriptableImage(const GeneratedImageP& gen) : value(gen) {}
	
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
inline ScriptValueP to_script(const ScriptableImage&) { return script_nil; }

/// Convert a script value to a GeneratedImageP
GeneratedImageP image_from_script(const ScriptValueP& value);

// ----------------------------------------------------------------------------- : EOF
#endif
