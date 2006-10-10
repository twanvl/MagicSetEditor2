//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/io/get_member.hpp>
#include <util/vector2d.hpp>
#include <script/value.hpp>

// ----------------------------------------------------------------------------- : GetMember

GetMember::GetMember(const String& name)
	: targetName(name)
{}

void GetMember::store(const String&       v) { value = toScript(v); }
void GetMember::store(const int           v) { value = toScript(v); }
void GetMember::store(const unsigned int  v) { value = toScript((int)v); }
void GetMember::store(const double        v) { value = toScript(v); }
void GetMember::store(const bool          v) { value = toScript(v); }

void GetMember::store(const Vector2D&     v) {
	value = toScript(String::Format(_("(%.10lf,%.10lf)"), v.x, v.y));
}
