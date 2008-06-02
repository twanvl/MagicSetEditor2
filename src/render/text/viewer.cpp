//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/text/viewer.hpp>
#include <algorithm>

DECLARE_TYPEOF_COLLECTION(TextViewer::Line);
DECLARE_TYPEOF_COLLECTION(double);

// ----------------------------------------------------------------------------- : Line

struct TextViewer::Line {
	size_t         start;		///< Index of the first character in this line
	size_t         end_or_soft;	///< Index just beyond the last non-soft character
	vector<double> positions;	///< x position of each character in this line, gives the number of characters + 1, never empty
	double         top;			///< y position of (the top of) this line
	double         line_height;	///< The height of this line in pixels
	LineBreak      break_after;	///< Is there a saparator after this line?
//%	Alignment      alignment;   ///< Alignment of this line
	
	Line()
		: start(0), end_or_soft(0), top(0), line_height(0), break_after(BREAK_NO)
	{}
	
	/// The position (just beyond) the bottom of this line
	double bottom() const { return top + line_height; }
	/// The width of this line
	double width()  const { return positions[end_or_soft-start] - positions.front(); }
	/// Index just beyond the last character on this line
	size_t end() const { return start + positions.size() - 1; }
	/// Find the index of the character at the given position on this line
	/** Always returns a value in the range [start..end()] */
	size_t posToIndex(double x) const;
	
	/// Is this line visible using the given rectangle?
	bool visible(const Rotation& rot) const {
		return top + line_height > 0 && top < rot.getHeight();
	}
	
	/// Get a rectangle of the selection on this line
	/** start and end need not be in this line */
	RealRect selectionRectangle(const Rotation& rot, size_t start, size_t end);
};

size_t TextViewer::Line::posToIndex(double x) const {
	// largest index with pos <= x
	vector<double>::const_iterator it2 = lower_bound(positions.begin(), positions.end(), x);
	if (it2 == positions.begin()) return start;
	if (it2 == positions.end()) --it2; // we don't want to find the position beyond the end
	if (it2 == positions.begin()) return start;
	// first index with pos > x
	vector<double>::const_iterator it1 = it2 - 1;
	if (x - *it1 <= *it2 - x) return it1 - positions.begin() + start; // it1 is closer
	else                      return it2 - positions.begin() + start; // it2 is closer
}

// ----------------------------------------------------------------------------- : TextViewer

// can't be declared in header because we need to know sizeof(Line)
TextViewer:: TextViewer() : justifying (false) {}
TextViewer::~TextViewer() {}

// ----------------------------------------------------------------------------- : Drawing

void TextViewer::draw(RotatedDC& dc, const TextStyle& style, DrawWhat what) {
	assert(!lines.empty());
	// separator lines?
	// do this first, so pen is still set from drawing the field border
	if (what & DRAW_BORDERS) {
		drawSeparators(dc);
	}
	// Draw the text, line by line
	FOR_EACH(l, lines) {
		if (l.visible(dc)) {
			if (justifying) {
				// Draw characters separatly
				for (size_t i = 0 ; i < l.positions.size() - 1 ; ++i) {
					RealRect rect(l.positions[i], l.top, l.positions[i+1] - l.positions[i] , l.line_height);
					elements.draw(dc, scale, rect, &l.positions[i], what, l.start + i, l.start + i + 1);
				}
			} else {
				RealRect rect(l.positions.front(), l.top, l.width(), l.line_height);
				elements.draw(dc, scale, rect, &*l.positions.begin(), what, l.start, l.end());
			}
		}
	}
}

/// Intersection between two rectangles
RealRect intersect(const RealRect& a, const RealRect& b) {
	RealPoint tl = piecewise_max(a.topLeft(),     b.topLeft());
	RealPoint br = piecewise_min(a.bottomRight(), b.bottomRight());
	return RealRect(tl, RealSize(br - tl));
}

void TextViewer::drawSelection(RotatedDC& dc, const TextStyle& style, size_t sel_start, size_t sel_end) {
	if (sel_start == sel_end) return;
	if (sel_end < sel_start) swap(sel_start, sel_end);
	dc.SetBrush(*wxBLACK_BRUSH);
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetLogicalFunction(wxINVERT);
	RealRect prev_rect(0,0,0,0);
	FOR_EACH(l, lines) {
		RealRect rect = l.selectionRectangle(dc, sel_start, sel_end);
		if (rect.height > 0) dc.DrawRectangle(rect);
		// compensate for overlap between lines
		RealRect overlap = intersect(rect, prev_rect);
		if (overlap.height > 0 && overlap.width > 0) dc.DrawRectangle(overlap);
		prev_rect = rect;
	}
	dc.SetLogicalFunction(wxCOPY);
}

RealRect TextViewer::Line::selectionRectangle(const Rotation& rot, size_t sel_start, size_t sel_end) {
	if (visible(rot) && sel_start < end() && sel_end > start) {
		double x1 = positions[max(start, sel_start) - start];
		double x2 = positions[min(end(), sel_end)   - start];
		return RealRect(x1, top, x2 - x1, line_height);
	} else {
		return RealRect(0,0,0,0);
	}
}

void TextViewer::drawSeparators(RotatedDC& dc) {
	// separator lines
	bool separator = false;
	double y = 0;
	FOR_EACH(l, lines) {
		double y2 = l.top + l.line_height;
		if (separator && l.visible(dc)) {
			// between the two lines
			y = (y + l.top) / 2;
			dc.DrawLine(RealPoint(0, y), RealPoint(dc.getInternalRect().width, y));
		}
		separator = l.break_after == BREAK_LINE;
		y = y2;
	}
	// separator at the end?
	if (separator) {
		dc.DrawLine(RealPoint(0, y), RealPoint(dc.getInternalRect().width, y));
	}
}

bool TextViewer::prepare(RotatedDC& dc, const String& text, TextStyle& style, Context& ctx) {
	if (!prepared()) {
		// not prepared yet
		prepareElements(text, style, ctx);
		prepareLines(dc, text, style, ctx);
		return true;
	} else {
		return false;
	}
}
void TextViewer::reset(bool related) {
	elements.elements.clear();
	lines.clear();
	if (!related) scale = 1.0;
}
bool TextViewer::prepared() const {
	return !lines.empty();
}

// ----------------------------------------------------------------------------- : Positions

const TextViewer::Line& TextViewer::findLine(size_t index) const {
	assert(!lines.empty());
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
	inline bool operator () (double                  a, double                  b) const { return a     < b;     }
	inline bool operator () (const TextViewer::Line& a, double                  b) const { return a.top < b;     }
	inline bool operator () (double                  a, const TextViewer::Line& b) const { return a     < b.top; }
	inline bool operator () (const TextViewer::Line& a, const TextViewer::Line& b) const { return a.top < b.top; }
};
size_t TextViewer::indexAt(const RealPoint& pos) const {
	// 1. find the line
	if (lines.empty()) return 0;
	vector<Line>::const_iterator l = lower_bound(lines.begin(), lines.end(), pos.y, CompareTop());
	if (l != lines.begin()) l--;
	assert(l != lines.end());
	// 2. find char on line
	return l->posToIndex(pos.x);
}

RealRect TextViewer::charRect(size_t index, bool first) const {
	if (lines.empty()) return RealRect(0,0,0,0);
	const Line& l = findLine(index);
	size_t pos = index - l.start;
	if (pos + 1 >= l.positions.size()) {
		if (!first && &l < &lines.back()) {
			// try the start of the next line
			const Line& l2 = *(&l+1);
			if (index == l2.start) {
				return RealRect(l2.positions.front(), l2.top, 0, l2.line_height);
			}
		}
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

// ----------------------------------------------------------------------------- : Scrolling

size_t TextViewer::lineCount() const {
	return lines.size();
}
size_t TextViewer::visibleLineCount(double height) const {
	size_t count = 0;
	FOR_EACH_CONST(l, lines) {
		if (l.top + l.line_height > height) return count;
		if (l.top >= 0) ++count;
	}
	return count;
}
size_t TextViewer::firstVisibleLine() const {
	size_t i = 0;
	FOR_EACH_CONST(l, lines) {
		if (l.top >= 0) return i;
		i++;
	}
	return 0; //no visible lines
}

void TextViewer::scrollTo(size_t line_id) {
	scrollBy(-lines.at(line_id).top);
}
void TextViewer::scrollBy(double delta) {
	if (delta == 0) return;
	FOR_EACH(l, lines) {
		l.top += delta;
	}
}

bool TextViewer::ensureVisible(double height, size_t char_id) {
	if (lines.empty()) return true;
	const Line& line = findLine(char_id);
	if (line.top < 0) {
		// scroll up
		scrollBy(-line.top);
		return true;
	} else if (line.bottom() > height) {
		// scroll down
		FOR_EACH(l, lines) {
			if (l.top > 0) scrollBy(-l.line_height); // scroll down a single line ...
			if (line.bottom() <= height) break;      // ... until we can see the current line
		}
		return true;
	} else {
		return false; // line was already visible
	}
}

double TextViewer::getExactScrollPosition() const {
	if (lines.empty()) return 0;
	return -lines.front().top;
}
void TextViewer::setExactScrollPosition(double pos) {
	if (lines.empty()) return; // no scrolling is needed
	pos += lines.front().top;
	scrollBy(-pos);
}

// ----------------------------------------------------------------------------- : Elements

void TextViewer::prepareElements(const String& text, const TextStyle& style, Context& ctx) {
	elements.fromString(text, 0, text.size(), style, ctx);
}


// ----------------------------------------------------------------------------- : Layout

void TextViewer::prepareLines(RotatedDC& dc, const String& text, TextStyle& style, Context& ctx) {
	vector<CharInfo> chars;
	prepareLinesTryScales(dc, text, style, chars);
	
	// store information about the content/layout, allow this to change alignment
	style.content_width  = 0;
	FOR_EACH(l, lines) {
		style.content_width = max(style.content_width, l.width());
	}
	style.content_height = 0;
	FOR_EACH_REVERSE(l, lines) {
		style.content_height = l.top + l.line_height;
		if (l.line_height) break; // not an empty line
	}
	style.content_lines = (int)lines.size();
	style.alignment.update(ctx); // allow this to affect the alignment
	
	// no text, find a dummy height for the single line we have
	if (lines.size() == 1 && lines[0].width() < 0.0001) {
		if (style.always_symbol && style.symbol_font.valid()) {
			lines[0].line_height = style.symbol_font.font->defaultSymbolSize(style.symbol_font.size).height;
		} else {
			dc.SetFont(style.font, scale);
			lines[0].line_height = dc.GetCharHeight();
		}
	}
	
	// align
	alignLines(dc, chars, style);
	
	// HACK : fix empty first line before <line>, do this after align, so layout is not affected
	if (lines.size() > 1 && lines[0].line_height == 0) {
		dc.SetFont(style.font, scale);
		double h = dc.GetCharHeight();
		lines[0].line_height =  h;
		lines[0].top         -= h;
	}
}

// bound on max_scale, given that scale fits and produces the given lines
inline double bound_on_max_scale(RotatedDC& dc, const TextStyle& style, const vector<TextViewer::Line>& lines, double scale) {
	if (lines.empty()) return 1.0;
	double tot_height = dc.getInternalSize().height + 1;
	double height = min(tot_height, lines.back().bottom() + style.padding_bottom);
	if (height < 1) return 1.0;
	return scale * tot_height / height;
}
// bound on min_scale, given that scale doesn't fit and produces the given lines
inline double bound_on_min_scale(RotatedDC& dc, const TextStyle& style, const vector<TextViewer::Line>& lines, double scale) {
	if (lines.empty()) return 0.0;
	double tot_height = dc.getInternalSize().height;
	double height = lines.back().bottom() + style.padding_bottom;
	if (height < 1) return 0.0;
	return scale * tot_height / height;
}

void TextViewer::prepareLinesTryScales(RotatedDC& dc, const String& text, const TextStyle& style, vector<CharInfo>& chars) {
	// Bounds
	double min_scale = elements.minScale();
	double scale_step = max(0.01,elements.scaleStep());
	// Is there any scaling (common case is: no)
	if (min_scale >= 1.0) {
		scale = 1.0;
		elements.getCharInfo(dc, scale, 0, text.size(), chars);
		prepareLinesScale(dc, chars, style, false, lines);
		return;
	}
	
	// More complicated fitting
	double max_scale = 1.0 + scale_step;
	double best_scale;
	
	// Assumption:
	//    It is likely that the text should have the same scale as the previous render attempt
	//    So:
	//       - try that scale first
	//       - if it fits
	//           - change min_scale
	//           - then try the scale just before it
	//               - if that doesn't fit, we are done
	//               - if it doesn't, we have just (almost) wasted 1 cycle, start binary search
	//       - if it doesn't
	//           - change max_scale	
	
	// Try the layout at the previous scale, this could give a quick upper bound
	elements.getCharInfo(dc, scale, 0, text.size(), chars);
	bool fits = prepareLinesScale(dc, chars, style, false, lines);
	if (fits) {
		min_scale = scale;
		max_scale = min(max_scale, bound_on_max_scale(dc,style,lines,scale));
		// is there a before?
		if (scale + scale_step >= max_scale) return;
		// try just before
		scale += scale_step;
		vector<Line> lines_before;
		vector<CharInfo> chars_before;
		elements.getCharInfo(dc, scale, 0, text.size(), chars_before);
		fits = prepareLinesScale(dc, chars_before, style, false, lines_before);
		if (fits) {
			// too bad
			swap(lines, lines_before);
			swap(chars, chars_before);
			best_scale = min_scale = scale;
			max_scale = min(max_scale, bound_on_max_scale(dc,style,lines,scale));
		} else {
			// yay
			scale = min_scale;
			return;
		}
	} else {
		max_scale = scale;
		min_scale = max(min_scale, bound_on_min_scale(dc,style,lines,scale));
		// ensure invariant d (below)
		best_scale = scale = min_scale;
		chars.clear();
		elements.getCharInfo(dc, scale, 0, text.size(), chars);
		prepareLinesScale(dc, chars, style, false, lines);
		max_scale = min(max_scale, bound_on_max_scale(dc,style,lines,scale));
	}
	
	// The common case optimization fialed, try a binary search
	// Invariant:
	//    a. The text fits at min_scale (or we force it anyway)
	//    b. but not at max_scale
	//    c. 0 < min_scale <= real_scale < max_scale <= 1.0+epsilon
	//    d. lines and chars give the best fitting positioning, at best_scale
	//    try: e. min_scale <= best_scale
		
	// go binary search!
	while(min_scale + scale_step < max_scale) {
		scale = (min_scale + max_scale) / 2;
		vector<Line> lines_try;
		vector<CharInfo> chars_try;
		elements.getCharInfo(dc, scale, 0, text.size(), chars_try);
		fits = prepareLinesScale(dc, chars_try, style, false, lines_try);
		if (fits) {
			min_scale = scale;
			max_scale = min(max_scale, bound_on_max_scale(dc,style,lines_try,scale));
			best_scale = scale; // invariant d
			swap(lines,lines_try); 
			swap(chars,chars_try);
		} else {
			max_scale = scale;
			min_scale = max(min_scale, bound_on_min_scale(dc,style,lines_try,scale));
			// the above can break pseudo invariant e
		}
	}
	if (best_scale != min_scale) {
		// we'd better update lines, e doesn't hold
		scale = min_scale;
		chars.clear();
		elements.getCharInfo(dc, scale, 0, text.size(), chars);
		fits = prepareLinesScale(dc, chars, style, false, lines);
	}
	scale = min_scale;
}


bool TextViewer::prepareLinesScale(RotatedDC& dc, const vector<CharInfo>& chars, const TextStyle& style, bool stop_if_too_long, vector<Line>& lines) const {
	// Try to layout the text at the current scale
	// first line
	lines.clear();
	Line line;
	line.top = style.padding_top;
	// size of the line so far
	RealSize line_size(lineLeft(dc, style, 0), 0);
	while (line.top < style.height && line_size.width + 1 >= style.width - style.padding_right) {
		// nothing fits on this line, move down one pixel
		line.top += 1;
		line_size.width = lineLeft(dc, style, line.top);
	}
	line.positions.push_back(line_size.width);
	// The word we are currently reading
	RealSize       word_size;
	vector<double> positions_word; // positios for this word
	size_t         word_end_or_soft = 0;
	size_t         word_start = 0;
	// For each character ...
	for(size_t i = 0 ; i < chars.size() ; ++i) {
		const CharInfo& c = chars[i];
		// Should we break?
		bool word_too_long = false;
		bool break_now     = false;
		bool accept_word   = false; // the current word should be added to the line
		bool hide_breaker  = true;  // hide the \n or _(' ') that caused a line break
		if (c.break_after == BREAK_SOFT || c.break_after == BREAK_HARD || c.break_after == BREAK_LINE) {
			break_now   = true;
			accept_word = true;
			line.break_after = c.break_after;
		} else if (c.break_after == BREAK_SPACE && style.field().multi_line) {
			// Soft break == end of word
			accept_word = true;
		} else if (c.break_after == BREAK_MAYBE && style.direction == TOP_TO_BOTTOM) {
			break_now   = true;
			accept_word = true;
			hide_breaker = false;
			line.break_after = BREAK_SOFT;
		}
		// Add size of the character
		if (c.break_after != BREAK_LINE) {
			// ^^ HACK: don't count the line height of <line> tags, if they are the only thing on a line
			//          then the linebreak is 'ignored'.
			word_size = add_horizontal(word_size, c.size);
		}
		positions_word.push_back(word_size.width);
		if (!c.soft) word_end_or_soft = i + 1;
		// Did the word become too long?
		if (style.field().multi_line && !break_now) {
			double max_width = lineRight(dc, style, line.top);
			if (line_size.width + word_size.width > max_width) {
				if (word_start == line.start) {
					// single word on this line; the word is too long
					if (stop_if_too_long) {
						return false; // just give up
					} else {
						// force a word break
						break_now = true;
						accept_word = true;
						hide_breaker = false;
						word_too_long = true;
						line.break_after = BREAK_SOFT;
					}
				} else {
					// line would become too long, break before the current word
					break_now = true;
					line.break_after = BREAK_SOFT;
				}
			}
		}
		// Ending the current word
		if (accept_word) {
			// move word pos to line
			FOR_EACH(p, positions_word) {
				line.positions.push_back(line_size.width + p);
			}
			if (word_end_or_soft != 0) line.end_or_soft = word_end_or_soft;
			line_size = add_horizontal(line_size, word_size);
			// next word
			word_size = RealSize(0, 0);
			word_start = i + 1;
			positions_word.clear();
			word_end_or_soft = 0;
			// move character that goes outside the box to the next line
			if (word_too_long && line.positions.size() > 2) {
				line.positions.pop_back();
				word_start = i;
				word_size = add_horizontal(word_size, c.size);
				positions_word.push_back(word_size.width);
				if (!c.soft) word_end_or_soft = i + 1;
			}
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
			line.end_or_soft = max(line.start, min(line.end_or_soft, line.end()));
			// push
			lines.push_back(line);
			// reset line object for next line
			double line_height_multiplier = line.break_after == BREAK_HARD ? style.line_height_hard
			                              : line.break_after == BREAK_LINE ? style.line_height_line
			                              :                                  style.line_height_soft;
			line.top += line.line_height * line_height_multiplier;
			line.start = word_start;
			line.positions.clear();
			if (line.break_after == BREAK_LINE) line.line_height = 0;
			line.break_after = BREAK_NO;
			// reset line_size
			line_size = RealSize(lineLeft(dc, style, line.top), 0);
			while (line.top < style.height && line_size.width + 1 >= style.width - style.padding_right) {
				// nothing fits on this line, move down one pixel
				line.top += 1;
				line_size.width = lineLeft(dc, style, line.top);
			}
			line.positions.push_back(line_size.width); // start position
		}
	}
	// the last word
	FOR_EACH(p, positions_word) {
		line.positions.push_back(line_size.width + p);
	}
	if (word_end_or_soft != 0) line.end_or_soft = word_end_or_soft;
	line_size = add_horizontal(line_size, word_size);
	// the last line
	if (line_size.height < 0.01 && !lines.empty()) {
		// if a line has 0 height, use the height of the line above it, but at most once
	} else {
		line.line_height = line_size.height;
	}
	line.end_or_soft = max(line.start, min(line.end_or_soft, line.end()));
	lines.push_back(line);
	// does it fit vertically?
	if (style.paragraph_height > 0) {
		// height = max(paragraph_height) * paragraph_count
		double max_height = 0;
		// per paragraph alignment
		size_t start = 0;
		for (size_t last = 0 ; last < lines.size() ; ++last) {
			if (lines[last].break_after != BREAK_SOFT || last == lines.size()) {
				max_height = max(max_height, lines[last].bottom() - lines[start].top);
				start = last + 1;
			}
		}
		// how many paragraphs would fit?
		int n = int(floor(0.5 + (dc.getInternalSize().height - style.padding_bottom) / style.paragraph_height));
		lines.back().top = max_height * n - lines.back().line_height;
	}
	return lines.back().bottom() <= dc.getInternalSize().height - style.padding_bottom;
}

double TextViewer::lineLeft(RotatedDC& dc, const TextStyle& style, double y) const {
	return style.mask.rowLeft(y, dc.getInternalSize()) + style.padding_left;
}
double TextViewer::lineRight(RotatedDC& dc, const TextStyle& style, double y) const {
	return style.mask.rowRight(y, dc.getInternalSize()) - style.padding_right;
}


void TextViewer::alignLines(RotatedDC& dc, const vector<CharInfo>& chars, const TextStyle& style) {
	// Size of the box
	RealSize s = add_diagonal(
					dc.getInternalSize(),
					-RealSize(style.padding_left+style.padding_right, style.padding_top + style.padding_bottom));
	if (style.paragraph_height <= 0) {
		// whole text box alignment
		alignParagraph(0, lines.size(), chars, style, RealRect(RealPoint(0,style.padding_top),s));
	} else {
		// per paragraph alignment
		size_t start = 0;
		int n = 0;
		for (size_t last = 0 ; last < lines.size() ; ++last) {
			if (lines[last].break_after != BREAK_SOFT || last == lines.size()) {
				alignParagraph(start, last + 1, chars, style, RealRect(0, style.padding_top+n*style.paragraph_height, s.width, style.paragraph_height));
				start = last + 1;
				++n;
			}
		}
	}
}

void TextViewer::alignParagraph(size_t start_line, size_t end_line, const vector<CharInfo>& chars, const TextStyle& style, const RealRect& s) {
	if (start_line >= end_line) return;
	// Find height of the text, don't count the last lines if they are empty
	double height = 0;
	for (size_t li = end_line - 1 ; li + 1 > start_line ; --li) {
		Line& l = lines[li];
		height = l.top + l.line_height;
		if (l.line_height) break; // not an empty line
	}
	height -= lines[start_line].top;
	// stretch lines by increasing the space between them
	if (height < s.height) {
		double d_soft = max(0.0, style.line_height_soft_max - style.line_height_soft);
		double d_hard = max(0.0, style.line_height_hard_max - style.line_height_hard);
		double d_line = max(0.0, style.line_height_line_max - style.line_height_line);
		double stops[] = {0.0, d_soft, d_hard, d_line};
		sort(stops + 1, stops + 4);
		for (int i = 1 ; i < 4 && height < s.height ; ++i) {
			double stop = stops[i] - stops[i-1];
			if (stop <= 0) continue;
			// which types can use this stop?
			bool soft = d_soft >= stop;
			bool hard = d_hard >= stop;
			bool line = d_line >= stop;
			// sum of the line height we can apply this to?
			double sum = 0;
			for (size_t li = start_line ; li < end_line ; ++li) {
				const Line& l = lines[li];
				if ((soft && l.break_after == BREAK_SOFT)
				 || (hard && l.break_after == BREAK_HARD)
				 || (line && l.break_after == BREAK_LINE)) sum += l.line_height;
			}
			if (sum == 0) break;
			// how much do we need to add?
			double to_add = min(stop, (s.height - height) / sum);
			// apply
			double add = 0;
			for (size_t li = start_line ; li < end_line ; ++li) {
				Line& l = lines[li];
				l.top  += add;
				// adjust next line by..
				if ((soft && l.break_after == BREAK_SOFT)
				 || (hard && l.break_after == BREAK_HARD)
				 || (line && l.break_after == BREAK_LINE)) add += to_add * l.line_height;
			}
			height += add;
		}
	}
	if (style.alignment == ALIGN_TOP_LEFT) return;
	// align
	double vdelta = align_delta_y(style.alignment, s.height, height)
	              + s.y - lines[start_line].top;
	// align all lines
	for (size_t li = start_line ; li < end_line ; ++li) {
		Line& l = lines[li];
		l.top += vdelta;
		// amount to shift all characters horizontally
		double width = l.positions[l.end_or_soft - l.start];
		if ((style.alignment & ALIGN_JUSTIFY) ||
			(style.alignment & ALIGN_JUSTIFY_OVERFLOW && width > s.width)) {
			// justify text
			justifying = true;
			double hdelta = s.width - width;            // amount of space to distribute
			int count = (int)(l.end_or_soft - l.start); // distribute it among this many characters
			if (count <= 0) count = 1;                  // prevent div by 0
			int i = 0;
			FOR_EACH(c, l.positions) {
				c += s.x + hdelta * i++ / count;
			}
		} else if (style.alignment & ALIGN_JUSTIFY_WORDS) {
			// justify text, by words
			justifying = true;
			double hdelta = s.width - width; // amount of space to distribute
			int count = 0;                   // distribute it among this many words
			for (size_t k = l.start + 1 ; k < l.end_or_soft - 1 ; ++k) {
				if (chars[k].break_after == BREAK_SPACE) ++count;
			}
			if (count == 0) count = 1;       // prevent div by 0
			int i = 0; size_t j = l.start;
			FOR_EACH(c, l.positions) {
				c += s.x + hdelta * i / count;
				if (j < l.end_or_soft && chars[j++].break_after == BREAK_SPACE) i++;
			}
		} else if (style.alignment & ALIGN_STRETCH_OVERFLOW && width >= s.width) {
			// stretching, don't center or align right
			justifying = false;
		} else {
			// simple alignment
			justifying = false;
			double hdelta = s.x + align_delta_x(style.alignment, s.width, width);
			FOR_EACH(c, l.positions) {
				c += hdelta;
			}
		}
	}
	// TODO : work well with mask
}
