//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_DELAYED_INDEX_MAPS
#define HEADER_UTIL_DELAYED_INDEX_MAPS

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/smart_ptr.hpp>
#include <util/index_map.hpp>
#include <util/reflect.hpp>
#include <wx/sstream.h>

// ----------------------------------------------------------------------------- : DelayedIndexMaps

template <typename Key, typename Value>
IndexMap<Key,Value>& DelayedIndexMaps<Key,Value>::get(const String& name, const vector<Key>& init_with) {
	intrusive_ptr<DelayedIndexMapsData<Key,Value> >& item = data[name];
	if (!item) { // no item, make a new one
		item = intrusive(new DelayedIndexMapsData<Key,Value>);
		item->read_data.init(init_with);
	} else if (!item->unread_data.empty()) { // not read, read now
		item->read_data.init(init_with);
		Reader reader(shared(new wxStringInputStream(item->unread_data)), nullptr, _("delayed data for ") + name);
		reader.handle_greedy(item->read_data);
		item->unread_data.clear();
	}
	return item->read_data;
}

template <typename Key, typename Value>
void DelayedIndexMaps<Key,Value>::clear() {
	data.clear();
}

// ----------------------------------------------------------------------------- : Reflection

// custom reflection : it's a template class
template <typename Key, typename Value> void Reader::handle(DelayedIndexMaps<Key,Value>& dim) {
	handle(dim.data);
}
template <typename Key, typename Value> void Writer::handle(const DelayedIndexMaps<Key,Value>& dim) {
	handle(dim.data);
}
template <typename Key, typename Value> void GetMember::handle(const DelayedIndexMaps<Key,Value>& dim) {
	handle(dim.data);
}

// custom reflection : read into unread_data
template <typename Key, typename Value>
void Reader::handle(DelayedIndexMapsData<Key,Value>& d) {
	handle(d.unread_data);
	if (d.unread_data.empty()) d.unread_data = _("\n"); // never empty (invariant)
}
template <typename Key, typename Value>
void Writer::handle(const DelayedIndexMapsData<Key,Value>& d) {
	if (!d.unread_data.empty()) {
		if (d.unread_data == _("\n")) {
			// this is not interesting, it is only used to make unread_data nonempty (see above)
			// we don't need to write it
		} else {
			handle(d.unread_data); // TODO: how to handle filenames
		}
	} else {
		handle(d.read_data);
	}
}
template <typename Key, typename Value>
void GetMember::handle(const DelayedIndexMapsData<Key,Value>& d) {
	handle(d.read_data);
}
template <typename Key, typename Value>
void GetDefaultMember::handle(const DelayedIndexMapsData<Key,Value>& d) {
	handle(d.read_data);
}

// ----------------------------------------------------------------------------- : EOF
#endif
