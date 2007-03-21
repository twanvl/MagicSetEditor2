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
	double margin = 0, height = 0;
	if (nativeLook() && (style().render_style & RENDER_CHECKLIST)) {
		height = 10;
		margin = 11;
		wxRect rect = dc.tr(RealRect(pos,RealSize(10,10)));
		draw_checkbox(nullptr, dc.getDC(), rect, active); // TODO
	}
	if (style().render_style & RENDER_IMAGE) {
		map<String,ScriptableImage>::iterator it = style().choice_images.find(cannocial_name_form(choice));
		if (it != style().choice_images.end()) {
			ScriptImageP i = it->second.update(viewer.getContext(), *viewer.stylesheet, 0, 0);
			if (i) {
				// TODO : alignment?
				dc.DrawImage(i->image, pos, i->combine == COMBINE_NORMAL ? style().combine : i->combine);
				margin += dc.trInvS(i->image.GetWidth()) + 1;
				height = max(height, dc.trInvS(i->image.GetHeight()));
			}
		}
	}
	if (style().render_style & RENDER_TEXT) {
		// draw text
		// TODO: alignment
		dc.DrawText(tr(*viewer.stylesheet, choice, capitalize(choice)), pos + RealSize(margin, 0));
		// TODO: determine size
	}
	if (style().direction == HORIZONTAL) {
		pos.x += margin + style().spacing;
	} else {
		pos.y += height + style().spacing;
	}
}
