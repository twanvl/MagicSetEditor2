//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <render/text/viewer.hpp>
#include <algorithm>

DECLARE_TYPEOF_COLLECTION(TextViewer::Line);
DECLARE_TYPEOF_COLLECTION(double);

// ----------------------------------------------------------------------------- : Line

struct TextViewer::Line {
	size_t         start;		///< Index of the first character in this line
	vector<double> positions;	///< x position of each character in this line, gives the number of characters + 1, never empty
	double         top;			///< y position of (the top of) this line
	double         line_height;	///< The height of this line in pixels
	bool           separator_after;	///< Is there a saparator after this line?
	
	Line()
		: start(0), top(0), line_height(0), separator_after(false)
	{}
	
	/// The position (just beyond) the bottom of this line
	double bottom() const { return top + line_height; }
	/// The width of this line
	double width()  const { return positions.back() - positions.front(); }
	/// Index just beyond the last character on this line
	size_t end() const { return start + positions.size() - 1; }
	/// Find the index of the character at the given position on this line
	/** Always returns a value in the range [start..end()] */
	size_t posToIndex(double x) const;
	
	/// Is this line visible using the given rectangle?
	bool visible(const Rotation& rot) const {
		return top + line_height > 0 && top < rot.getInternalSize().height;
	}
	
	/// Draws a selection indicator on this line from start to end
	/** start and end need not be in this line */
	void drawSelection(RotatedDC& dc, size_t start, size_t end);
};

size_t TextViewer::Line::posToIndex(double x) const {
	// largest index with pos <= x
	vector<double>::const_iterator it2 = lower_bound(positions.begin(), positions.end(), x);
	if (it2 == positions.begin()) return start;
	if (it2 == positions.end()) --it2; // we don't want to find the position beyond the end
	// first index with pos > x
	vector<double>::const_iterator it1 = it2 - 1;
	if (x - *it1 <= *it2 - x) return it1 - positions.begin() + start; // it1 is closer
	else                      return it2 - positions.begin() + start; // it2 is closer
}

// ----------------------------------------------------------------------------- : TextViewer

// can't be declared in header because we need to know sizeof(Line)
TextViewer:: TextViewer() {}
TextViewer::~TextViewer() {}

// ----------------------------------------------------------------------------- : Drawing

void TextViewer::draw(RotatedDC& dc, const TextStyle& style, DrawWhat what) {
	assert(!lines.empty());
	Rotater r(dc, style.getRotation());
	// Draw the text, line by line
	FOR_EACH(l, lines) {
		if (l.visible(dc)) {
			RealRect rect(l.positions.front(), l.top, l.width(), l.line_height);
			elements.draw(dc, scale, rect, &*l.positions.begin(), what, l.start, l.end());
		}
	}
}

void TextViewer::drawSelection(RotatedDC& dc, const TextStyle& style, size_t sel_start, size_t sel_end) {
	Rotater r(dc, style.getRotation());
	if (sel_start == sel_end) return;
	if (sel_end < sel_start) swap(sel_start, sel_end);
	dc.SetBrush(*wxBLACK_BRUSH);
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetLogicalFunction(wxINVERT);
	FOR_EACH(l, lines) {
		l.drawSelection(dc, sel_start, sel_end);
	}
	dc.SetLogicalFunction(wxCOPY);
}

void TextViewer::Line::drawSelection(RotatedDC& dc, size_t sel_start, size_t sel_end) {
	if (!visible(dc)) return;
	if (sel_start < end() && sel_end > start) {
		double x1 = positions[max(start, sel_start) - start];
		double x2 = positions[min(end(), sel_end)   - start];
		dc.DrawRectangle(RealRect(x1, top, x2 - x1, line_height));
	}
}

void TextViewer::prepare(RotatedDC& dc, const String& text, const TextStyle& style, Context& ctx) {
	if (lines.empty()) {
		// not prepared yet
		Rotater r(dc, style.getRotation());
		prepareElements(text, style, ctx);
		prepareLines(dc, text, style);
	}
}
void TextViewer::reset() {
	elements.clear();
	lines.clear();
}

// ----------------------------------------------------------------------------- : Positions

const TextViewer::Line& TextViewer::findLine(size_t index) const {
	FOR_EACH_CONST(l, lines) {
		if (l.end() >= index) return l;
	}
	return lines.front();
}

size_t TextViewer::moveLine(size_t index, int delta) const {
	if (lines.empty()) return index;
	const Line* line1 = &findLine(index);
	const Line* line2 = line1 + delta;
	if (line2 >= &lines.front() && line2 <= &lines.back()) {
		size_t idx = index - line1->start;
		if (idx < 0 || idx >= line1->positions.size()) return index; // can't move
		return line2->posToIndex(line1->positions[idx]); // character at the same position
	} else {
		return index; // can't move
	}
}

size_t TextViewer::lineStart(size_t index) const {
	if (lines.empty()) return 0;
	return findLine(index).start;
}

size_t TextViewer::lineEnd(size_t index) const {
	if (lines.empty()) return 0;
	return findLine(index).end();
}

struct CompareTop {
	inline bool operator () (const TextViewer::Line& l, double y) const { return l.top < y; }
	inline bool operator () (double y, const TextViewer::Line& l) const { return y < l.top; }
};
size_t TextViewer::indexAt(const RealPoint& pos) const {
	// 1. find the line
	vector<Line>::const_iterator l = lower_bound(lines.begin(), lines.end(), pos.y, CompareTop());
	if (l != lines.begin()) l--;
	assert(l != lines.end());
	// 2. find char on line
	return l->posToIndex(pos.x);
}

RealRect TextViewer::charRect(size_t index) const {
	if (lines.empty()) return RealRect(0,0,0,0);
	const Line& l = findLine(index);
	size_t pos = index - l.start;
	if (pos >= l.positions.size()) {
		return RealRect(l.positions.back(), l.top, 0, l.line_height);
	} else {
		return RealRect(l.positions[pos], l.top, l.positions[pos + 1] - l.positions[pos], l.line_height);
	}
}

bool TextViewer::isVisible(size_t index) const {
	if (lines.empty()) return false;
	const Line& l = findLine(index);
	size_t pos = index - l.start;
	if (pos >= l.positions.size()) {
		return false;
	} else if (pos + 1 == l.positions.size()) {
		return true; // last char of the line
	} else {
		return l.positions[pos + 1] - l.positions[pos] > 0.0001;
	}
}
size_t TextViewer::firstVisibleChar(size_t index, int delta) const {
	if (lines.empty()) return index;
	const Line* l = &findLine(index);
	while (true) {
		int pos = (int)(index - l->start);
		while (index == l->end() || (pos + delta >= 0 && (size_t)pos + delta < l->positions.size())) {
			if (index == l->end() || l->positions[pos + 1] - l->positions[pos] > 0.0001) {
				return index;
			}
			pos   += delta;
			index += delta;
		}
		// move to another line, if not at start/end
		if (l + delta < &lines.front()) return 0;
		if (l + delta > &lines.back())  return l->end();
		index += delta;
		l     += delta;
	}
}

double TextViewer::heightOfLastLine() const {
	if (lines.empty()) return 0;
	return lines.back().line_height;
}

// ----------------------------------------------------------------------------- : Elements

void TextViewer::prepareElements(const String& text, const TextStyle& style, Context& ctx) {
	if (style.always_symbol) {
		elements.elements.clear();
		elements.elements.push_back(new_shared5<SymbolTextElement>(text, 0, text.size(), style.symbol_font, &ctx));
	} else {
		elements.fromString(text, 0, text.size(), style, ctx);
	}
}


// ----------------------------------------------------------------------------- : Layout

void TextViewer::prepareLines(RotatedDC& dc, const String& text, const TextStyle& style) {
	scale = 1;
	// find character sizes
	vector<CharInfo> chars;
	elements.getCharInfo(dc, scale, 0, text.size(), chars);
	// try to layout
	prepareLinesScale(dc, chars, style, false);
	// align
	alignLines(dc, chars, style);
}

bool TextViewer::prepareLinesScale(RotatedDC& dc, const vector<CharInfo>& chars, const TextStyle& style, bool stop_if_too_long) {
	// Try to layout the text at the current scale
	// first line
	lines.clear();
	Line line;
	// size of the line so far
	RealSize line_size(lineLeft(dc, style, 0), 0);
	line.positions.push_back(line_size.width);
	// The word we are currently reading
	RealSize       word_size;
	vector<double> positions_word; // positios for this word
	size_t         word_start = 0;
	// For each character ...
	for(size_t i = 0 ; i < chars.size() ; ++i) {
		const CharInfo& c = chars[i];
		// Should we break?
		bool   break_now    = false;
		bool   accept_word  = false; // the current word should be added to the line
		bool   hide_breaker = true;  // hide the \n or _(' ') that caused a line break
		double line_height_multiplier = 1; // multiplier for line height for next line top
		if (c.break_after == BREAK_HARD) {
			break_now   = true;
			accept_word = true;
			line_height_multiplier = style.line_height_hard;
		} else if (c.break_after == BREAK_LINE) {
			line.separator_after = true;
			break_now   = true;
			accept_word = true;
			line_height_multiplier = style.line_height_line;
		} else if (c.break_after == BREAK_SOFT && style.field().multi_line) {
			// Soft break == end of word
			accept_word = true;
		}
		// Add size of the character
		word_size = addHorizontal(word_size, c.size);
		positions_word.push_back(word_size.width);
		// Did the word become too long?
		if (style.field().multi_line && !break_now) {
			double max_width = lineRight(dc, style, line.top);
			if (word_start == line.start && word_size.width > max_width) {
				// single word on this line; the word is too long
				if (stop_if_too_long) {
					return false; // just give up
				} else {
					// force a word break
					break_now = true;
					accept_word = true;
					hide_breaker = false;
					line_height_multiplier = style.line_height_soft;
				}
			} else if (line_size.width + word_size.width > max_width) {
				// line would become too long, break before the current word
				break_now = true;
				line_height_multiplier = style.line_height_soft;
			}
		}
		// Ending the current word
		if (accept_word) {
			// move word pos to line
			FOR_EACH(p, positions_word) {
				line.positions.push_back(line_size.width + p);
			}
			// add size; next word
			line_size = addHorizontal(line_size, word_size);
			word_size = RealSize(0, 0);
			word_start = i + 1;
			positions_word.clear();
		}
		// Breaking (ending the current line)
		if (break_now) {
			// remove the _('\n') or _(' ') that caused the break
			if (hide_breaker && line.positions.size() > 1) {
				line.positions.pop_back();
			}
			// height of the line
			if (line_size.height < 0.01 && !lines.empty()) {
				// if a line has 0 height, use the height of the line above it, but at most once
			} else {
				line.line_height = line_size.height;
			}
			// push
			lines.push_back(line);
			// reset line object for next line
			line.top += line.line_height * line_height_multiplier;
			line.start = word_start;
			line.positions.clear();
			if (line.separator_after) line.line_height = 0;
			line.separator_after = false;
			// reset line_size
			line_size = RealSize(lineLeft(dc, style, line.top), 0);
			line.positions.push_back(line_size.width); // start position
		}
	}
	// the last word
	FOR_EACH(p, positions_word) {
		line.positions.push_back(line_size.width + p);
	}
	line_size = addHorizontal(line_size, word_size);
	// the last line
	if (line_size.height < 0.01 && !lines.empty()) {
		// if a line has 0 height, use the height of the line above it, but at most once
	} else {
		line.line_height = line_size.height;
	}
	lines.push_back(line);
	return true;
}

double TextViewer::lineLeft(RotatedDC& dc, const TextStyle& style, double y) {
	return 0 + style.padding_left;
//	return style.mask.rowLeft(y, dc.getInternalSize()) + style.padding_left;
}
double TextViewer::lineRight(RotatedDC& dc, const TextStyle& style, double y) {
	return style.width - style.padding_right;
//	return style.mask.rowRight(y, dc.getInternalSize()) - style.padding_right;
}

ContourMask::ContourMask() {} // MOVEME //@@
ContourMask::~ContourMask() {}

void TextViewer::alignLines(RotatedDC& dc, const vector<CharInfo>& chars, const TextStyle& style) {
	if (style.alignment == ALIGN_TOP_LEFT) return;
	// Find height of the text, don't count the last lines if they are empty
	double height = 0;
	FOR_EACH_REVERSE(l, lines) {
		height = l.top + l.line_height;
		if (l.line_height) break; // not an empty line
	}
	// amount to shift all lines vertically
	RealSize s = dc.getInternalSize();
	double vdelta = align_delta_y(style.alignment, s.height, height);
	// align all lines
	FOR_EACH(l, lines) {
		l.top += vdelta;
		// amount to shift all characters horizontally
		double width = l.positions.back();
		if ((style.alignment & ALIGN_JUSTIFY) ||
			(style.alignment & ALIGN_JUSTIFY_OVERFLOW && width > s.width)) {
			// justify text
//			justifying = true;
			double hdelta = s.width - width;         // amount of space to distribute
			int count = (int)l.positions.size() - 1; // distribute it among this many characters
			if (count == 0) count = 1;               // prevent div by 0
			int i = 0;
			FOR_EACH(c, l.positions) {
				c += hdelta * i++ / count;
			}
		} else if (style.alignment & ALIGN_JUSTIFY) {
			// justify text, by words
//			justifying = true;
			double hdelta = s.width - width;         // amount of space to distribute
			int count = 0;                           // distribute it among this many words
			for (size_t k = l.start + 1 ; k < l.end() - 1 ; ++k) {
				if (chars[k].break_after == BREAK_SOFT) ++count;
			}
			if (count == 0) count = 1;               // prevent div by 0
			int i = 0; size_t j = l.start;
			FOR_EACH(c, l.positions) {
				c += hdelta * i / count;
				if (j < l.end() && chars[j++].break_after == BREAK_SOFT) i++;
			}
		} else {
			// simple alignment
//			justifying = false;
			double hdelta = align_delta_x(style.alignment, s.width, width);
			FOR_EACH(c, l.positions) {
				c += hdelta;
			}
		}
	}
	// TODO : work well with mask
}
