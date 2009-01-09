//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/io/get_member.hpp>
#include <util/vector2d.hpp>
#include <script/script.hpp>
#include <script/to_value.hpp>
#include <boost/logic/tribool.hpp>

// ---------------------------------------------------------------------------- : GetDefaultMember

            void GetDefaultMember::handle(const Char*         v) { value = to_script(v); }
template <> void GetDefaultMember::handle(const String&       v) { value = to_script(v); }
template <> void GetDefaultMember::handle(const FileName&     v) { value = to_script(v); }
template <> void GetDefaultMember::handle(const int&          v) { value = to_script(v); }
template <> void GetDefaultMember::handle(const unsigned int& v) { value = to_script((int)v); }
template <> void GetDefaultMember::handle(const double&       v) { value = to_script(v); }
template <> void GetDefaultMember::handle(const bool&         v) { value = to_script(v); }
template <> void GetDefaultMember::handle(const tribool&      v) { value = to_script((bool)v); }
template <> void GetDefaultMember::handle(const Vector2D&     v) { value = to_script(String::Format(_("(%.10lf,%.10lf)"), v.x, v.y)); }
template <> void GetDefaultMember::handle(const Color&        v) { value = to_script(v); }
            void GetDefaultMember::handle(const ScriptValueP& v) { value = v; }
            void GetDefaultMember::handle(const ScriptP&      v) { value = v; }

// ----------------------------------------------------------------------------- : GetMember

GetMember::GetMember(const String& name)
	: target_name(name)
{}

// caused by the pattern: if (!tag.isComplex()) { REFLECT_NAMELESS(stuff) }
template <> void GetMember::handle(const String& v) {
	throw InternalError(_("GetDefaultMember::handle"));
}
