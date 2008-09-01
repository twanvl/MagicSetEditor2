//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/set/panel.hpp>
#include <data/card.hpp>

// ----------------------------------------------------------------------------- : SetWindowPanel

SetWindowPanel::SetWindowPanel(Window* parent, int id, bool autoTabbing)
	: wxPanel(parent, id, wxDefaultPosition, wxDefaultSize, autoTabbing ? wxTAB_TRAVERSAL : 0)
{}

CardP SetWindowPanel::selectedCard() const {
	return CardP();
}

bool SetWindowPanel::isInitialized() const {
	return !GetChildren().IsEmpty();
}
