//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/io/get_member.hpp>
#include <util/vector2d.hpp>
#include <script/value.hpp>
#include <script/script.hpp>

// ----------------------------------------------------------------------------- : GetDefaultMember

template <> void GetDefaultMember::handle(const Char* const&  v) { value = toScript(v); }
template <> void GetDefaultMember::handle(const String&       v) { value = toScript(v); }
template <> void GetDefaultMember::handle(const int&          v) { value = toScript(v); }
template <> void GetDefaultMember::handle(const unsigned int& v) { value = toScript((int)v); }
template <> void GetDefaultMember::handle(const double&       v) { value = toScript(v); }
template <> void GetDefaultMember::handle(const bool&         v) { value = toScript(v); }
template <> void GetDefaultMember::handle(const Vector2D&     v) { value = toScript(String::Format(_("(%.10lf,%.10lf)"), v.x, v.y)); }
template <> void GetDefaultMember::handle(const Color&        v) { value = toScript(v); }
            void GetDefaultMember::handle(const ScriptValueP& v) { value = v; }
            void GetDefaultMember::handle(const ScriptP&      v) { value = v; }

// ----------------------------------------------------------------------------- : GetMember

GetMember::GetMember(const String& name)
	: target_name(name)
{}
