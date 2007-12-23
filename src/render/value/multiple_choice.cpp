//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <render/value/multiple_choice.hpp>
#include <render/card/viewer.hpp>
#include <data/stylesheet.hpp>
#include <gui/util.hpp>

DECLARE_TYPEOF_COLLECTION(String);

// ----------------------------------------------------------------------------- : MultipleChoiceValueViewer

void MultipleChoiceValueViewer::draw(RotatedDC& dc) {
	drawFieldBorder(dc);
	if (style().render_style & RENDER_HIDDEN) return;
	RealPoint pos = align_in_rect(style().alignment, RealSize(0,0), style().getInternalRect());
	// selected choices
	vector<String> selected;
	value().get(selected);
	if (style().render_style & RENDER_CHECKLIST) {
		// render all choices
		int end = field().choices->lastId();
		vector<String>::iterator select_it = selected.begin();
		for (int i = 0 ; i < end ; ++i) {
			String choice = field().choices->choiceName(i);
			bool active = select_it != selected.end() && *select_it == choice;
			if (active) select_it++;
			drawChoice(dc, pos, choice, active);
		}
	} else if (style().render_style & RENDER_LIST) {
		// render only selected choices
		FOR_EACH(choice, selected) {
			drawChoice(dc, pos, choice);
		}
	} else {
		// COPY FROM ChoiceValueViewer
		if (value().value().empty()) return;
		double margin = 0;
		if (style().render_style & RENDER_IMAGE) {
			// draw image
			style().initImage();
			CachedScriptableImage& img = style().image;
			Context& ctx = viewer.getContext();
			ctx.setVariable(_("input"), to_script(value().value()));
			img.update(ctx);
			if (img.isReady()) {
				GeneratedImage::Options img_options;
				getOptions(dc, img_options);
				// Generate image/bitmap
				ImageCombine combine = style().combine;
				style().loadMask(*viewer.stylesheet);
				Bitmap bitmap; Image image;
				RealSize size;
				img.generateCached(img_options, &style().mask, &combine, &bitmap, &image, &size);
				size = dc.trInvS(size);
				RealRect rect(align_in_rect(style().alignment, size, dc.getInternalRect()), size);
				if (bitmap.Ok()) {
					// just draw it
					dc.DrawPreRotatedBitmap(bitmap,rect);
				} else {
					// use combine mode
					dc.DrawPreRotatedImage(image,rect,combine);
				}
				margin = size.width + 1;
			}
		}
		if (style().render_style & RENDER_TEXT) {
			// draw text
			dc.DrawText(tr(*viewer.stylesheet, value().value(), capitalize(value().value())),
				align_in_rect(ALIGN_MIDDLE_LEFT, RealSize(0, dc.GetCharHeight()), dc.getInternalRect()) + RealSize(margin, 0)
			);
		}
		// COPY ENDS HERE
	}
}

void MultipleChoiceValueViewer::drawChoice(RotatedDC& dc, RealPoint& pos, const String& choice, bool active) {
	RealSize size; size.height = item_height;
	if (nativeLook() && (style().render_style & RENDER_CHECKLIST)) {
		wxRect rect = dc.trRectStraight(RealRect(pos + RealSize(1,1), RealSize(12,12)));
		draw_checkbox(nullptr, dc.getDC(), rect, active); // TODO
		size = add_horizontal(size, RealSize(14,16));
	}
	if (style().render_style & RENDER_IMAGE) {
		map<String,ScriptableImage>::iterator it = style().choice_images.find(cannocial_name_form(choice));
		if (it != style().choice_images.end() && it->second.isReady()) {
			// TODO: caching
			GeneratedImage::Options options(0,0, viewer.stylesheet.get(),&getSet());
			options.zoom = dc.getZoom();
			options.angle = dc.trAngle(style().angle);
			Image image = it->second.generate(options);
			ImageCombine combine = it->second.combine();
			// TODO : alignment?
			dc.DrawPreRotatedImage(image, RealRect(pos.x + size.width, pos.y, options.width, options.height), combine == COMBINE_DEFAULT ? style().combine : combine);
			size.width += options.width;
		}
	}
	if (style().render_style & RENDER_TEXT) {
		// draw text
		String text = tr(*viewer.stylesheet, choice, capitalize_sentence(choice));
		RealSize text_size = dc.GetTextExtent(text);
		dc.DrawText(text, align_in_rect(ALIGN_MIDDLE_LEFT, text_size,
		                                RealRect(pos + RealSize(size.width + 1, 0), RealSize(0,size.height))));
		size = add_horizontal(size, text_size);
	}
	// next position
	pos = move_in_direction(style().direction, pos, size, style().spacing);
}

// COPY from ChoiceValueViewer
void MultipleChoiceValueViewer::getOptions(Rotation& rot, GeneratedImage::Options& opts) {
	opts.package       = viewer.stylesheet.get();
	opts.local_package = &getSet();
	opts.angle         = rot.trAngle(0); //%%
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
