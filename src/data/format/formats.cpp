//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/format/formats.hpp>
#include <data/set.hpp>

DECLARE_POINTER_TYPE(FileFormat);
DECLARE_TYPEOF_COLLECTION(FileFormatP);

// ----------------------------------------------------------------------------- : Formats

// All supported file formats
vector<FileFormatP> fileFormats;

void initFileFormats() {
	//fileFormats.push_back(new_shared<MSE2FileFilter>());
	//fileFormats.push_back(new_shared<MSE1FileFilter>());
	//fileFormats.push_back(new_shared<MtgEditorFileFilter>());
}

String importFormats() {
	String allExtensions; // type1;type2
	String typeStrings;   // |name1|type1|name2|type2
	FOR_EACH(f, fileFormats) {
		if (f->canImport()) {
			if (!allExtensions.empty()) allExtensions += _(";");
			allExtensions += _("*.") + f->extension();
			typeStrings   += _("|") + f->name() + _("|*.") + f->extension();
		}
	}
	return _("Set files|") + allExtensions + typeStrings + _("|All files (*.*)|*.*");
}

String exportFormats(const Game& game) {
	String typeStrings; // name1|type1|name2|type2
	FOR_EACH(f, fileFormats) {
		if (f->canExport(game)) {
			if (!typeStrings.empty())  typeStrings += _("|");
			typeStrings += f->name() + _("|*.") + f->extension();
		}
	}
	return typeStrings;
}

void exportSet(const Set& set, const String& filename, size_t formatType) {
	FileFormatP format = fileFormats.at(formatType);
	if (!format->canExport(*set.game)) {
		throw InternalError(_("File format doesn't apply to set"));
	}
	format->exportSet(set, filename);
}

SetP importSet(String name) {
	size_t pos = name.find_last_of(_('.'));
	String extension = pos==String::npos ? _("") : name.substr(pos + 1);
	// determine format
	FOR_EACH(f, fileFormats) {
		if (f->extension() == extension) {
			return f->importSet(name);
		}
	}
	// default : use first format = MSE2 format
	assert(!fileFormats.empty() && fileFormats[0]->canImport());
	return fileFormats[0]->importSet(name);
}