//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/vcs.hpp>

// ----------------------------------------------------------------------------- : SubversionVCS

class GitVCS : public VCS {
public:
	void addFile(const wxFileName& filename) final;
	void moveFile(const wxFileName& source, const wxFileName& destination) final;
	void removeFile(const wxFileName& filename) final;
	void updateFile(const wxFileName& filename) final;
	void commit(const String& directory) final;
	void pull(const String& directory) final;

	DECLARE_REFLECTION();
};

