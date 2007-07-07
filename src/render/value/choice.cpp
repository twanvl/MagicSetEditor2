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

void ChoiceValueViewer::draw(RotatedDC& dc) {
	drawFieldBorder(dc);
	if (style().render_style & RENDER_HIDDEN) return;
	if (value().value().empty()) return;
	double margin = 0;
	if (style().render_style & RENDER_IMAGE) {
		// draw image
//		map<String,ScriptableImage>::iterator it = style().choice_images.find(cannocial_name_form(value().value()));
//		if (it != style().choice_images.end() && it->second.isReady()) {
//			ScriptableImage& img = it->second;
		style().initImage();
		ScriptableImage& img = style().image;
		Context& ctx = viewer.getContext();
		ctx.setVariable(_("input"), to_script(value().value()));
		img.update(ctx);
		if (img.isReady()) {
			GeneratedImage::Options img_options(0,0, viewer.stylesheet.get(), &getSet());
			if (nativeLook()) {
				img_options.width = img_options.height = 16;
				img_options.preserve_aspect = ASPECT_BORDER;
			} else if(style().render_style & RENDER_TEXT) {
				// also drawing text, use original size
			} else {
				img_options.width  = (int) dc.trX(style().width);
				img_options.height = (int) dc.trY(style().height);
				img_options.preserve_aspect = style().alignment == ALIGN_STRETCH ? ASPECT_STRETCH : ASPECT_FIT;
			}
			Image image = img.generate(img_options, true);
			ImageCombine combine = img.combine();
			// apply mask?
			style().loadMask(*viewer.stylesheet);
			if (style().mask.Ok()) {
				set_alpha(image, style().mask);
			}
			// draw
			dc.DrawImage(image,
				align_in_rect(style().alignment, RealSize(image.GetWidth(), image.GetHeight()), style().getRect()),
				combine == COMBINE_NORMAL ? style().combine : combine,
				style().angle
			);
			margin = dc.trInvS(image.GetWidth()) + 1;
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

void ChoiceValueViewer::onStyleChange(bool already_prepared) {
	if (!already_prepared) viewer.redraw(*this);
}
