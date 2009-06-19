//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2009 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/value/multiple_choice.hpp>
#include <render/value/choice.hpp>
#include <render/card/viewer.hpp>
#include <gui/util.hpp>

DECLARE_TYPEOF_COLLECTION(String);

// ----------------------------------------------------------------------------- : MultipleChoiceValueViewer

IMPLEMENT_VALUE_VIEWER(MultipleChoice);

bool MultipleChoiceValueViewer::prepare(RotatedDC& dc) {
	if (style().render_style & (RENDER_CHECKLIST | RENDER_LIST)) return false;
	return prepare_choice_viewer(dc, *this, style(), value().value());
}

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
		draw_choice_viewer(dc, *this, style(), value().value());
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
			GeneratedImage::Options options(0,0, &getStylePackage(), &getLocalPackage());
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
		String text = tr(getStylePackage(), choice, capitalize_sentence);
		dc.SetFont(style().font,1);
		RealSize text_size = dc.GetTextExtent(text);
		RealPoint text_pos = align_in_rect(ALIGN_MIDDLE_LEFT, text_size, RealRect(pos.x + size.width + 1, pos.y, 0,size.height));
		dc.DrawTextWithShadow(text, style().font, text_pos);
		size = add_horizontal(size, text_size);
	}
	// next position
	pos = move_in_direction(style().direction, pos, size, style().spacing);
}

void MultipleChoiceValueViewer::onStyleChange(int changes) {
	if (changes & CHANGE_MASK) style().image.clearCache();
	ValueViewer::onStyleChange(changes);
}
