//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/control/text_ctrl.hpp>
#include <gui/value/editor.hpp>
#include <gui/util.hpp>
#include <data/field/text.hpp>
#include <data/action/value.hpp>

DECLARE_TYPEOF_COLLECTION(ValueViewerP);

// ----------------------------------------------------------------------------- : TextCtrl

TextCtrl::TextCtrl(Window* parent, int id, bool multi_line, long style)
	: DataEditor(parent, id, style)
	, value(nullptr)
	, multi_line(multi_line)
{}

Rotation TextCtrl::getRotation() const {
	return Rotation(0, RealRect(RealPoint(0,0),GetClientSize()));
}

void TextCtrl::draw(DC& dc) {
	RotatedDC rdc(dc, getRotation(), QUALITY_LOW);
	DataViewer::draw(rdc, wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
}


TextStyle& TextCtrl::getStyle() {
	assert(!viewers.empty());
	return static_cast<TextStyle&>(*viewers.front()->getStyle());
}
TextField& TextCtrl::getField() {
	assert(!viewers.empty());
	return static_cast<TextField&>(*viewers.front()->getField());
}
void TextCtrl::updateSize() {
	wxSize cs = GetClientSize();
	Style& style = getStyle();
	style.width  = cs.GetWidth()  - 2;
	style.height = cs.GetHeight() - 2;
	viewers.front()->getEditor()->determineSize(true);
}

void TextCtrl::setValue(String* value, bool untagged) {
	if (value != this->value) {
		this->value = value;
		// create a new value, for a different underlying actual value
		ValueViewer& viewer = *viewers.front();
		TextValueP new_value(new FakeTextValue(static_pointer_cast<TextField>(viewer.getField()), this->value, untagged));
		viewer.setValue(new_value);
		updateSize();
		valueChanged();
	}
}
void TextCtrl::valueChanged() {
	if (!viewers.empty()) {
		TextValue& tv = static_cast<TextValue&>(*viewers.front()->getValue());
		tv.value.assign(value ? String(*value) : String(wxEmptyString));
		viewers.front()->onValueChange();
	}
	onChange();
}
void TextCtrl::onAction(const Action& action, bool undone) {
	DataEditor::onAction(action, undone);
	/*
	TYPE_CASE(action, TextValueAction) {
		FakeTextValue& tv = static_cast<FakeTextValue&>(*viewers.front()->getValue());
		if (tv.equals(action.valueP.get())) {
			// the value has changed
			if (value) *value = tv.value();
		}
	}
	*/
}
void TextCtrl::onChangeSet() {
	DataEditor::onChangeSet();
	// initialize
	if (viewers.empty()) {
		// create a field, style and value
		TextFieldP field(new TextField);
		TextStyleP style(new TextStyle(field));
		TextValueP value(new FakeTextValue(field, nullptr, false));
		// set stuff
		field->index = 0;
		field->multi_line = multi_line;
		style->width = 100;
		style->height = 20;
		style->left = style->top = 1;
		style->font.color = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
		// assign to this control
		IndexMap<FieldP,StyleP> styles; styles.add(field, style);
		IndexMap<FieldP,ValueP> values; values.add(field, value);
		setStyles(set->stylesheet, styles);
		setData(values);
		updateSize();
		onChange();
	} else {
		setValue(nullptr);
	}
}

void TextCtrl::onInit() {
	// Give viewers a chance to show/hide controls (scrollbar) when selecting other editors
	FOR_EACH_EDITOR {
		e->onShow(true);
	}
}

void TextCtrl::onSize(wxSizeEvent&) {
	if (!viewers.empty()) {
		updateSize();
		onChange();
	}
}
wxSize TextCtrl::DoGetBestSize() const {
	if (multi_line || viewers.empty()) {
		// flexible size
		return wxSize(1,1);
	} else {
		wxSize ws = GetSize(), cs = GetClientSize();
		Style& style = *viewers.front()->getStyle();
		return wxSize(style.width, style.height) + ws - cs;
	}
}

BEGIN_EVENT_TABLE(TextCtrl, DataEditor)
	EVT_SIZE        (TextCtrl::onSize)
END_EVENT_TABLE()
