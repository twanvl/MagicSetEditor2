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

TextCtrl::TextCtrl(Window* parent, int id, long style)
	: DataEditor(parent, id, style)
	, value(nullptr)
{}

Rotation TextCtrl::getRotation() const {
	return Rotation(0, RealRect(RealPoint(0,0),GetClientSize()));
}

void TextCtrl::draw(DC& dc) {
	RotatedDC rdc(dc, getRotation(), false);
	DataViewer::draw(rdc, wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
}


void TextCtrl::setValue(String* value) {
	this->value = value;
	if (viewers.empty() && value) {
		// create a field, style and value
		TextFieldP field(new TextField);
		TextStyleP style(new TextStyle(field));
		TextValueP value(new TextValue(field));
		// set stuff
		field->index = 0;
		field->multi_line = true;
		style->width = 100;
		style->height = 20;
		style->left = style->top = 1;
		value->value.assign(*this->value);
		// assign to this control
		IndexMap<FieldP,StyleP> styles; styles.add(field, style);
		IndexMap<FieldP,ValueP> values; values.add(field, value);
		setStyles(set->stylesheet, styles);
		setData(values);
		// determine size
		wxSize cs = GetClientSize();
		style->width  = cs.GetWidth()  - 2;
		style->height = cs.GetHeight() - 2;
		viewers.front()->getEditor()->determineSize(true);
		// We don't wan to change the window size
		//SetMinSize(RealSize(style->width + 6, style->height + 6));
	}
	valueChanged();
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
	TYPE_CASE(action, TextValueAction) {
		TextValue& tv = static_cast<TextValue&>(*viewers.front()->getValue());
		if (&tv == action.valueP.get()) {
			// the value has changed
			if (value) *value = tv.value();
		}
	}
}
void TextCtrl::onChangeSet() {
	DataEditor::onChangeSet();
	setValue(nullptr);
}

void TextCtrl::onInit() {
	// Give viewers a chance to show/hide controls (scrollbar) when selecting other editors
	FOR_EACH_EDITOR {
		e->onShow(true);
	}
}

void TextCtrl::onSize(wxSizeEvent&) {
	if (!viewers.empty()) {
		wxSize cs = GetClientSize();
		Style& style = *viewers.front()->getStyle();
		style.width  = cs.GetWidth()  - 2;
		style.height = cs.GetHeight() - 2;
		viewers.front()->getEditor()->determineSize(true);
	}
	onChange();
}
wxSize TextCtrl::DoGetBestSize() const {
	return wxSize(1,1);
}

BEGIN_EVENT_TABLE(TextCtrl, DataEditor)
	EVT_SIZE        (TextCtrl::onSize)
END_EVENT_TABLE()
