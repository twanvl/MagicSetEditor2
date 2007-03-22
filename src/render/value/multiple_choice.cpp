//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
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
	RealPoint pos = style().getPos();
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
	} else {
		// render only selected choices
		FOR_EACH(choice, selected) {
			drawChoice(dc, pos, choice);
		}
	}
}

void MultipleChoiceValueViewer::drawChoice(RotatedDC& dc, RealPoint& pos, const String& choice, bool active) {
	RealSize size; size.height = item_height;
	if (nativeLook() && (style().render_style & RENDER_CHECKLIST)) {
		wxRect rect = dc.tr(RealRect(pos + RealSize(1,1), RealSize(12,12)));
		draw_checkbox(nullptr, dc.getDC(), rect, active); // TODO
		size = add_horizontal(size, RealSize(14,16));
	}
	if (style().render_style & RENDER_IMAGE) {
		map<String,ScriptableImage>::iterator it = style().choice_images.find(cannocial_name_form(choice));
		if (it != style().choice_images.end()) {
			ScriptImageP i = it->second.update(viewer.getContext(), *viewer.stylesheet, 0, 0);
			if (i) {
				// TODO : alignment?
				dc.DrawImage(i->image, pos + RealSize(size.width, 0), i->combine == COMBINE_NORMAL ? style().combine : i->combine);
				size = add_horizontal(size, dc.trInv(RealSize(i->image.GetWidth() + 1, i->image.GetHeight())));
			}
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
	if (style().direction == HORIZONTAL) {
		pos.x += size.width  + style().spacing;
	} else {
		pos.y += size.height + style().spacing;
	}
}
