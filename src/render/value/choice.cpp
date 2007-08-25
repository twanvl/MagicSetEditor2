//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <render/value/choice.hpp>
#include <render/card/viewer.hpp>
#include <data/stylesheet.hpp>

// ----------------------------------------------------------------------------- : ChoiceValueViewer

bool ChoiceValueViewer::prepare(RotatedDC& dc) {
	if (style().render_style & RENDER_IMAGE) {
		style().initImage();
		CachedScriptableImage& img = style().image;
		Context& ctx = viewer.getContext();
		ctx.setVariable(_("input"), to_script(value().value()));
		// generate to determine the size
		if (img.update(ctx) && img.isReady()) {
			GeneratedImage::Options img_options;
			getOptions(dc, img_options);
			// Generate image/bitmap (whichever is available)
			// don't worry, we cache the image
			ImageCombine combine = style().combine;
			style().loadMask(*viewer.stylesheet);
			Bitmap bitmap; Image image;
			img.generateCached(img_options, &style().mask, &combine, &bitmap, &image);
			int w, h;
			if (bitmap.Ok()) {
				w = bitmap.GetWidth();
				h = bitmap.GetHeight();
			} else {
				assert(image.Ok());
				w = image.GetWidth();
				h = image.GetHeight();
			}
			if (sideways(img_options.angle)) swap(w,h);
			// store content properties
			if (style().content_width  != w || style().content_height != h) {
				style().content_width  = w;
				style().content_height = h;
				return true;
			}
		}
	}
	return false;
}

void ChoiceValueViewer::draw(RotatedDC& dc) {
	drawFieldBorder(dc);
	if (style().render_style & RENDER_HIDDEN) return;
	if (value().value().empty()) return;
	double margin = 0;
	if (style().render_style & RENDER_IMAGE) {
		// draw image
		CachedScriptableImage& img = style().image;
		if (img.isReady()) {
			GeneratedImage::Options img_options;
			getOptions(dc, img_options);
			// Generate image/bitmap
			ImageCombine combine = style().combine;
			style().loadMask(*viewer.stylesheet);
			Bitmap bitmap; Image image;
			img.generateCached(img_options, &style().mask, &combine, &bitmap, &image);
			if (bitmap.Ok()) {
				// just draw it
				dc.DrawPreRotatedBitmap(bitmap,
					align_in_rect(style().alignment, dc.trInvNoNeg(RealSize(bitmap)), style().getRect())
				);
				margin = dc.trInv(RealSize(bitmap)).width + 1;
			} else {
				// use combine mode
				dc.DrawPreRotatedImage(image,
					align_in_rect(style().alignment, dc.trInvNoNeg(RealSize(image)), style().getRect()),
					combine
				);
				margin = dc.trInv(RealSize(image)).width + 1;
			}
		} else if (nativeLook()) {
			// always have the margin
			margin = 17;
		}
	}
	if (style().render_style & RENDER_TEXT) {
		// draw text
		dc.SetFont(style().font, 1.0);
		String text = tr(*viewer.stylesheet, value().value(), capitalize(value().value()));
		RealPoint pos = align_in_rect(ALIGN_MIDDLE_LEFT, RealSize(0, dc.GetCharHeight()), style().getRect()) + RealSize(margin, 0);
		if (style().font.hasShadow()) {
			dc.SetTextForeground(style().font.shadow_color);
			dc.DrawText(text, pos + style().font.shadow_displacement);
		}
		dc.SetTextForeground(style().font.color());
		dc.DrawText(text, pos);
	}
}

void ChoiceValueViewer::onStyleChange(int changes) {
	if (changes & CHANGE_MASK) style().image.clearCache();
	ValueViewer::onStyleChange(changes);
}

void ChoiceValueViewer::getOptions(Rotation& rot, GeneratedImage::Options& opts) {
	opts.package       = viewer.stylesheet.get();
	opts.local_package = &getSet();
	opts.angle         = rot.trAngle(style().angle);
	if (nativeLook()) {
		opts.width = opts.height = 16;
		opts.preserve_aspect = ASPECT_BORDER;
	} else if(style().render_style & RENDER_TEXT) {
		// also drawing text, use original size
	} else {
		opts.width  = (int) rot.trX(style().width);
		opts.height = (int) rot.trY(style().height);
		opts.preserve_aspect = (style().alignment & ALIGN_STRETCH) ? ASPECT_STRETCH : ASPECT_FIT;
	}
}
