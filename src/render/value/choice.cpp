//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/value/choice.hpp>
#include <render/card/viewer.hpp>

DECLARE_TYPEOF_COLLECTION(wxPoint);

// ----------------------------------------------------------------------------- : ChoiceValueViewer

IMPLEMENT_VALUE_VIEWER(Choice);

void get_options(Rotation& rot, ValueViewer& viewer, const ChoiceStyle& style, GeneratedImage::Options& opts);

bool ChoiceValueViewer::prepare(RotatedDC& dc) {
	return prepare_choice_viewer(dc, *this, style(), value().value());
}
void ChoiceValueViewer::draw(RotatedDC& dc) {
	int w = max(0,(int)dc.trX(style().width)), h = max(0,(int)dc.trY(style().height));
	const AlphaMask& alpha_mask = getMask(w,h);
	drawFieldBorder(dc, alpha_mask);
	if (style().render_style & RENDER_HIDDEN) return;
	draw_choice_viewer(dc, *this, style(), value().value());
}

void ChoiceValueViewer::drawFieldBorder(RotatedDC& dc, const AlphaMask& alpha_mask) {
	if (!alpha_mask.isLoaded()) {
		ValueViewer::drawFieldBorder(dc);
	} else if (setFieldBorderPen(dc)) {
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		vector<wxPoint> points;
		alpha_mask.convexHull(points);
		if (points.size() < 3) return;
		FOR_EACH(p, points) p = dc.trPixelNoZoom(RealPoint(p.x,p.y));
		dc.getDC().DrawPolygon((int)points.size(), &points[0]);
	}
}

bool ChoiceValueViewer::containsPoint(const RealPoint& p) const {
	// check against mask
	return getMask(0,0).isOpaque(p, style().getSize());
}

void ChoiceValueViewer::onStyleChange(int changes) {
	if (changes & CHANGE_MASK) style().image.clearCache();
	ValueViewer::onStyleChange(changes);
}

const AlphaMask& ChoiceValueViewer::getMask(int w, int h) const {
	GeneratedImage::Options opts;
	opts.package       = &viewer.getStylePackage();
	opts.local_package = &viewer.getLocalPackage();
	opts.angle         = 0;
	opts.width         = w;
	opts.height        = h;
	return style().mask.get(opts);
}

// ----------------------------------------------------------------------------- : Generic draw/prepare

bool prepare_choice_viewer(RotatedDC& dc, ValueViewer& viewer, ChoiceStyle& style, const String& value) {
	if (style.render_style & RENDER_IMAGE) {
		style.initImage();
		CachedScriptableImage& img = style.image;
		Context& ctx = viewer.viewer.getContext();
		ctx.setVariable(SCRIPT_VAR_input, to_script(value));
		// generate to determine the size
		if (img.update(ctx) && img.isReady()) {
			GeneratedImage::Options img_options;
			get_options(dc, viewer, style, img_options);
			// Generate image/bitmap (whichever is available)
			// don't worry, we cache the image
			ImageCombine combine = style.combine;
			Bitmap bitmap; Image image;
			RealSize size;
			img.generateCached(img_options, &style.mask, &combine, &bitmap, &image, &size);
			// store content properties
			if (style.content_width != size.width || style.content_height != size.height) {
				style.content_width  = size.width;
				style.content_height = size.height;
				return true;
			}
		}
	}
	return false;
}

void draw_choice_viewer(RotatedDC& dc, ValueViewer& viewer, ChoiceStyle& style, const String& value) {
	if (value.empty()) return;
	double margin = 0;
	if (style.render_style & RENDER_IMAGE) {
		// draw image
		CachedScriptableImage& img = style.image;
		if (style.content_dependent) {
			// re run script
			Context& ctx = viewer.viewer.getContext();
			ctx.setVariable(SCRIPT_VAR_input, to_script(value));
			img.update(ctx);
		}
		if (img.isReady()) {
			GeneratedImage::Options img_options;
			get_options(dc, viewer, style, img_options);
			// Generate image/bitmap
			ImageCombine combine = style.combine;
			Bitmap bitmap; Image image;
			RealSize size;
			img.generateCached(img_options, &style.mask, &combine, &bitmap, &image, &size);
			size = dc.trInvS(size);
			RealRect rect(align_in_rect(style.alignment, size, dc.getInternalRect()), size);
			if (bitmap.Ok()) {
				// just draw it
				dc.DrawPreRotatedBitmap(bitmap,rect);
			} else {
				// use combine mode
				dc.DrawPreRotatedImage(image,rect,combine);
			}
			margin = size.width + 2;
		} else if (viewer.nativeLook()) {
			// always have the margin
			margin = 18;
		}
	}
	if (style.render_style & RENDER_TEXT) {
		String text = tr(viewer.getStylePackage(), value, capitalize_sentence);
		dc.SetFont(style.font, 1.0);
		RealPoint pos = align_in_rect(ALIGN_MIDDLE_LEFT, RealSize(0, dc.GetCharHeight()), dc.getInternalRect()) + RealSize(margin, 0);
		dc.DrawTextWithShadow(text, style.font, pos);
	}
}

void get_options(Rotation& rot, ValueViewer& viewer, const ChoiceStyle& style, GeneratedImage::Options& opts) {
	opts.package       = &viewer.getStylePackage();
	opts.local_package = &viewer.getLocalPackage();
	opts.angle         = rot.trAngle(0);
	if (viewer.nativeLook()) {
		opts.width = opts.height = 16;
		opts.preserve_aspect = ASPECT_BORDER;
	} else if(style.render_style & RENDER_TEXT) {
		// also drawing text, use original size
	} else {
		opts.width  = (int) rot.trX(style.width);
		opts.height = (int) rot.trY(style.height);
		opts.preserve_aspect = (style.alignment & ALIGN_STRETCH) ? ASPECT_STRETCH : ASPECT_FIT;
	}
}
