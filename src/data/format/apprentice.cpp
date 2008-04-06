//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/format/formats.hpp>
#include <data/settings.hpp>
#include <data/set.hpp>
#include <data/game.hpp>
#include <data/card.hpp>
#include <data/field/text.hpp>
#include <data/field/choice.hpp>
#include <util/tagged_string.hpp>
#include <util/window_id.hpp>
#include <wx/progdlg.h>
#include <wx/wfstream.h>
#include <wx/datstrm.h>
#include <wx/filename.h>

DECLARE_TYPEOF_COLLECTION(String);
DECLARE_TYPEOF_COLLECTION(CardP);
DECLARE_TYPEOF(map<String COMMA String>);

String card_rarity_code(const String& rarity);

// ----------------------------------------------------------------------------- : Generic export dialog

/// Callback for updating a progress bar
class WithProgress {
  public:
	virtual void onProgress(float progress, const String& message) = 0;
	virtual ~WithProgress () {}
};

/// Exception thrown to indicate exporting should be aborted
class AbortException {};

/// A dialog to show the progress of exporting
class ExportProgressDialog : public wxProgressDialog, public WithProgress {
  public:
	ExportProgressDialog(Window* parent, const String& title, const String& message);
	
	/// Update the progress bar
	/** if the operation should be aborted, throws an AbortException
	 */
	virtual void onProgress(float progress, const String& message);
};

ExportProgressDialog::ExportProgressDialog(Window* parent, const String& title, const String& message)
	: wxProgressDialog(title, message, 1000, parent, wxPD_APP_MODAL | wxPD_SMOOTH | wxPD_CAN_ABORT)
{}

void ExportProgressDialog::onProgress(float progress, const String& message) {
	if (!Update(int(progress * 1000), message)) {
		throw AbortException();
	}
}

// ----------------------------------------------------------------------------- : Generic apprentice database

/// An Apprentice database file, has read() and write() functions
class ApprDatabase {
  public:
	ApprDatabase(WithProgress* progress_target, const String& name);
	virtual ~ApprDatabase();
	
	/// Read the database
	void read();
	/// Write to a temporary file
	void write();
	/// Finalize the writing, swap the actual and the temporary file
	void commit();
	
  protected:
	virtual void doRead(wxInputStream&)   = 0;
	virtual void doWrite(wxOutputStream&) = 0;
	
	WithProgress* progress_target; ///< Write progress information to here
	
  private:
	bool in_progress; ///< Is writing in progress?
	String filename; ///< Filename of database file
};


ApprDatabase::ApprDatabase(WithProgress* progress_target, const String& name)
	: progress_target(progress_target)
	, in_progress(false)
	, filename(settings.apprentice_location + _("\\") + name)
{}
ApprDatabase::~ApprDatabase() {
	// An exception is thrown while we were writing, clean up the temporary files
	if (in_progress) {
		// abort 'transaction'
		wxRemoveFile(filename + _(".new"));
	}
}

void ApprDatabase::read() {
	wxFileInputStream in(filename);
	if (!in.Ok()) {
		throw Error(_("Can not open apprentice file for input\n'") + filename + _("'"));
	}
	doRead(in);
}
void ApprDatabase::write() {
	// write to a .new file, doesn't commit yet
	if (wxFileExists(filename + _(".new"))) {
		wxRemoveFile(filename + _(".new"));
	}
	wxFileOutputStream out(filename + _(".new"));
	in_progress = true;
	if (!out.Ok()) {
		throw Error(_("Can not open apprentice file for output\n'") + filename + _(".new'"));
	}
	doWrite(out);
}
void ApprDatabase::commit() {
	// commit : rename .new to real filename
	if (in_progress) {
		wxRenameFile(filename, filename + _(".bak"));
		wxRenameFile(filename + _(".new"), filename);
		in_progress = false;
	}
}

// ----------------------------------------------------------------------------- : Expansion database

/// An Apprentice expansion database (Expan.dat)
class ApprExpansionDatabase : public ApprDatabase {
  public:
	inline ApprExpansionDatabase(WithProgress* progress_target)
		: ApprDatabase(progress_target, _("Expan.dat"))
	{}
  protected:
	virtual void doRead(wxInputStream& in);
	virtual void doWrite(wxOutputStream& out);
	
  public:
	map<String,String> expansions; ///< code -> name
	vector<String> order;          ///< order of codes
};

void ApprExpansionDatabase::doRead(wxInputStream& in) {
	wxTextInputStream tin(in);
	while (!in.Eof()) {
		String l = tin.ReadLine();
		if (l.size() < 3) continue;
		order.push_back(l.substr(0,2));
		expansions[l.substr(0,2)] = l.substr(3);
	}
}

void ApprExpansionDatabase::doWrite(wxOutputStream& out) {
	wxTextOutputStream tout(out, wxEOL_DOS);
	// write in order first
	FOR_EACH(c, order) {
		String code = c;
		if (code.GetChar(0) != _('-')) {
			// but not the rarities
			tout << code << _("-") << expansions[c] << _("\n");
			expansions.erase(c);
		}
	}
	// the remaing expansions (our new set)
	FOR_EACH(e, expansions) {
		String code = e.first;
		if (code.GetChar(0) != _('-')) {
			tout << code << _("-") << e.second << _("\n");
		}
	}
	// and at last the rarities
	FOR_EACH(c, order) {
		String code = c;
		if (code.GetChar(0) == _('-')) {
			tout << c << _("-") << expansions[c] << _("\n");
		}
	}
}

// ----------------------------------------------------------------------------- : Format database

class ApprFormat {
  public:
	ApprFormat(const String& name) : name(name) {}
	String name, sets;
};

DECLARE_TYPEOF_COLLECTION(ApprFormat);

/// An Apprentice format database (Format.dat)
class ApprFormatDatabase : public ApprDatabase {
  public:
	inline ApprFormatDatabase(WithProgress* progress_target)
		: ApprDatabase(progress_target, _("Format.dat"))
	{}
	/// Remove a set code from all formats
	void removeSet(const String& code);
	
  protected:
	virtual void doRead(wxInputStream& in);
	virtual void doWrite(wxOutputStream& out);
	
  private:
	vector<ApprFormat> formats;
};

void ApprFormatDatabase::removeSet(const String& code) {
	// TODO?
}
	
void ApprFormatDatabase::doRead(wxInputStream& in) {
	wxTextInputStream tin(in);
	// read titles
	while (!in.Eof()) {
		String l = trim(tin.ReadLine());
		if (l == _("<Titles>")) {
			// ignore header
		} else if (l == _("<Format>")) {
			break; // to formatting step
		} else {
			size_t pos = l.find_first_of(_('='));
			if (pos == String::npos)  continue;
			// assume formats are in order
			formats.push_back( ApprFormat(l.substr(pos + 1)) );
		}
	}
	// read formats
	size_t i = 0;
	while (!in.Eof() && i < formats.size()) {
		String l = trim(tin.ReadLine());
		size_t pos = l.find_first_of(_('='));
		if (pos == String::npos)  continue;
		formats[i].sets = l.substr(pos + 1);
		i += 1;
	}
}

void ApprFormatDatabase::doWrite(wxOutputStream& out) {
	wxTextOutputStream tout(out, wxEOL_DOS);
	tout << _("<Titles>\n");
	int i = 1;
	FOR_EACH(f, formats) {
		tout << i++ << _("=") << f.name << _("\n");
	}
	tout << _("\n<Format>\n");
	i = 1;
	FOR_EACH(f, formats) {
		tout << i++ << _("=") << f.sets << _("\n");
	}
}


// ----------------------------------------------------------------------------- : Distro database

// An entry in the Distro database
class ApprDistro {
  public:
	inline ApprDistro(int bc = 0, int bu = 0, int br = 0,  int sc = 0, int su = 0, int sr = 0)
		: bc(bc), bu(bu), br(br)
		, sc(sc), su(su), sr(sr)
	{}
	
	int bc, bu, br; ///< # of cards in booster of each rarity
	int sc, su, sr; ///< .. in starter
	
	String scode, bcode; ///< alternative : numbers as a string (if numbers == 0)
	
	void write(const String& code, wxTextOutputStream& tout);
	
  private:
	void writeD(wxTextOutputStream& tout, const String& name, int c, int u, int r);
};

DECLARE_TYPEOF(map<String COMMA ApprDistro>);

/// An Apprentice distribution database (Distro.dat)
class ApprDistroDatabase : public ApprDatabase {
  public:
	inline ApprDistroDatabase(WithProgress* progress_target)
		: ApprDatabase(progress_target, _("Distro.dat"))
	{}
	/// Remove a set code
	void removeSet(const String& code);
	
  protected:
	virtual void doRead(wxInputStream& in);
	virtual void doWrite(wxOutputStream& out);
	
  public:
	map<String,ApprDistro> distros;
	vector<String> order; // order of codes
};


void ApprDistroDatabase::removeSet(const String& code) {
	// TODO ?
}

void ApprDistroDatabase::doRead(InputStream& in) {
	wxTextInputStream tin(in);
	ApprDistro* last = 0;
	while (!in.Eof()) {
		String l = trim(tin.ReadLine());
		if (l.size() > 2 && l.GetChar(0) == _('<')) {
			// new code
			l = l.substr(1, l.size() - 2);
			order.push_back(l);
			last = &distros[l];
		} else if (last && starts_with(l, _("Starter"))) {
			// no need to read the actual code, only to write it out later
			// keep it as a string
			last->scode = l;
		} else if (last && starts_with(l, _("Booster"))) {
			last->bcode = l;
		}
	}
}

void ApprDistroDatabase::doWrite(OutputStream& out) {
	wxTextOutputStream tout(out, wxEOL_DOS);
	// write in order
	FOR_EACH(c, order) {
		distros[c].write(c, tout);
		distros.erase(c);
	}
	// remaining distros (the newly added one)
	FOR_EACH(d, distros) {
		d.second.write(d.first, tout);
	}
}

void ApprDistro::write(const String& code, wxTextOutputStream& tout) {
	tout.WriteString(_("<") + code + _(">\n"));
	// starter
	if (sc || su || sr) {
		writeD(tout, _("Starter"), sc, su, sr);
	} else if (!scode.empty()) {
		tout.WriteString(_("    ") + scode + _("\n"));
	}
	// booster
	if (bc || bu || br) {
		writeD(tout, _("Booster"), bc, bu, br);
	} else if (!bcode.empty()) {
		tout.WriteString(_("    ") + bcode + _("\n"));
	}
}

void ApprDistro::writeD(wxTextOutputStream& tout, const String& name, int c, int u, int r) {
	tout.WriteString(_("    ")+name+_("=R") << r << _(",U") << u << _(",C") << c << _("\n"));
}

// ----------------------------------------------------------------------------- : Card database

/// Untag function for apprentice, replaces newlines with \r\n
String untag_appr(const String& s) {
	return replace_all(untag(s), _("\n"), _("\r\n"));
}

DECLARE_POINTER_TYPE(ApprCardRecord);
DECLARE_TYPEOF_COLLECTION(ApprCardRecordP);

/// An Apprentice card database (cardinfo.dat)
class ApprCardDatabase : public ApprDatabase {
  public:
	inline ApprCardDatabase(WithProgress* progress_target)
		: ApprDatabase(progress_target, _("sets\\cardinfo.dat"))
	{}
	/// Remove a set code
	void removeSet(const String& code);
	
  protected:
	virtual void doRead(wxInputStream& in);
	virtual void doWrite(wxOutputStream& out);
	
  public:
	vector<ApprCardRecordP> cards;
};

/// A single record in the apprentice card database
/** Each card has two records, a data record at the top of the file
 *  and a head record at the bottom
 */
class ApprCardRecord : public IntrusivePtrBase<ApprCardRecord> {
  public:
	String name, sets;
	String type, cc, pt, text, flavor;
	UInt data_pos;
	Byte color; // bitmask:
	enum Color {
		white = 0x01,
		blue  = 0x02,
		black = 0x04,
		red   = 0x08,
		green = 0x10,
		gold  = 0x20,
		arti  = 0x40,
		land  = 0x80,
	};
	
	ApprCardRecord() {}
	ApprCardRecord(const Card& card, const String& sets_);
	
	void readHead(wxDataInputStream& strm);
	void readCard(wxDataInputStream& strm);
	void writeHead(wxDataOutputStream& strm);
	void writeCard(wxDataOutputStream& strm);
	
	/// Reads a string in apprentice format
	String readString(wxDataInputStream& strm);
	/// Writes a string in apprentice format
	void writeString(wxDataOutputStream& strm, const String& out);
	
	// Remove a code from the list of set codes
	void removeSet(const String& code);
};

// conversion from MSE2 card
ApprCardRecord::ApprCardRecord(const Card& card, const String& sets_) {
	name   = untag_appr(card.value<TextValue>(_("name")).value);
	sets   = sets_ + _("-") + card_rarity_code(card.value<ChoiceValue>(_("rarity")).value);
	cc     = untag_appr(card.value<TextValue>(_("casting cost")).value);
	type   = untag_appr(card.value<TextValue>(_("super type")).value);
	String subType = untag(card.value<TextValue>(_("sub type")).value);
	if (!subType.empty())  type += _(" - ") + subType;
	text   = untag_appr(card.value<TextValue>(_("rule text")).value);
	flavor = untag_appr(card.value<TextValue>(_("flavor text")).value);
	pt     = untag_appr(card.value<TextValue>(_("pt")).value);
}

	
void ApprCardRecord::readHead(wxDataInputStream& strm) {
	name     = readString(strm);
	data_pos = strm.Read32();
	color    = strm.Read8();
	sets     = readString(strm);
	strm.Read16(); // 00 00
}
void ApprCardRecord::readCard(wxDataInputStream& strm) {
	type   = readString(strm);
	cc     = readString(strm);
	pt     = readString(strm);
	text   = readString(strm);
	flavor = readString(strm);
}

void ApprCardRecord::writeHead(wxDataOutputStream& strm) {
	name = name.substr(0, 27); // max length
	// determine color
	color = 0;
	int cnt = 0;
	if (cc.find_first_of(_('W')) != String::npos) { cnt +=1;  color |= white; }
	if (cc.find_first_of(_('U')) != String::npos) { cnt +=1;  color |= blue;  }
	if (cc.find_first_of(_('B')) != String::npos) { cnt +=1;  color |= black; }
	if (cc.find_first_of(_('R')) != String::npos) { cnt +=1;  color |= red;   }
	if (cc.find_first_of(_('G')) != String::npos) { cnt +=1;  color |= green; }
	if (cnt > 1)  color |= gold;
	if (cnt == 0) {
		if (type.find(_("Land")) != String::npos) color = land;
		else                                      color = arti;
	}
	// write
	writeString(strm, name);
	strm.Write32(data_pos);
	strm.Write8(color);
	writeString(strm, sets);
	strm.Write16(0x0000);
}
void ApprCardRecord::writeCard(wxDataOutputStream& strm) {
	writeString(strm, type);
	writeString(strm, cc);
	writeString(strm, pt);
	writeString(strm, text);
	writeString(strm, flavor);
}

String ApprCardRecord::readString(wxDataInputStream& strm) {
	size_t size = strm.Read16();
	if (size > 1000) size = 1000; // sanity check
	String ret;
	ret.reserve(size);
	for (size_t i = 0 ; i < size ; ++i) {
		ret += (Char) strm.Read8();
	}
	return ret;
}

void ApprCardRecord::writeString(wxDataOutputStream& strm, const String& out) {
	strm.Write16(UInt(out.size()));
	FOR_EACH_CONST(c, out) {
		strm.Write8(c);
	}
}

void ApprCardRecord::removeSet(const String& code) {
	size_t pos = sets.find(code);
	if (pos == String::npos)  return;
	String before = sets.substr(0, pos);
	String after  = sets.substr(pos + code.size());
	if (!before.empty() && before.GetChar(before.size()-1) == _(',')) {
		// remove comma
		before.resize(before.size() - 1);
	}
	sets = before + after;
}

// Is a card record unused, i.e. no sets refer to it?
bool unused_appr_card_record(const ApprCardRecordP& rec) {
	return rec->sets.size() < 2 || // nothing
	      (rec->sets.size() < 4 && rec->sets.find_first_of(_('-')) != String::npos); // only a rarity code
}



void ApprCardDatabase::removeSet(const String& code) {
	FOR_EACH(c, cards) {
		c->removeSet(code);
	}
	// cleanup
	cards.erase(remove_if(cards.begin(), cards.end(), unused_appr_card_record), cards.end());
}

void ApprCardDatabase::doRead(wxInputStream& in) {
	wxDataInputStream data(in);
	size_t head_pos = data.Read32();
	in.SeekI(head_pos);
	size_t card_count = data.Read32();
	cards.resize(card_count);
	// read cards
	int i = 0;
	FOR_EACH(card, cards) {
		if (++i % 100 == 0) {
			// report progress sometimes
			progress_target->onProgress(0.4f * float(i) / cards.size(),
			                            String(_("reading card ")) << i << _(" of ") << (int)cards.size());
		}
		card = new_intrusive<ApprCardRecord>();
		card->readHead(data);
		head_pos = in.TellI();
		in.SeekI(card->data_pos);
		card->readCard(data);
		in.SeekI(head_pos);
	}
}

void ApprCardDatabase::doWrite(wxOutputStream& out) {
	wxDataOutputStream data(out);
	data.Write32(0); // replaced with header pos
	// write card data
	int i = 0;
	FOR_EACH(card, cards) {
		if (++i % 100 == 0) {
			// report progress sometimes
			progress_target->onProgress(0.4f + 0.4f * float(i) / cards.size(),
			                            String(_("writing card ")) << i << _(" of ") << (int)cards.size());
		}
		card->data_pos = out.TellO();
		card->writeCard(data);
	}
	// write header location
	UInt head_start = out.TellO();
	out.SeekO(0);
	data.Write32(head_start);
	// card count
	out.SeekO(head_start);
	data.Write32((UInt)cards.size());
	// write card heads
	i = 0;
	FOR_EACH(card, cards) {
		if (++i % 100 == 0) {
			// report progress sometimes
			progress_target->onProgress(0.8f + 0.2f * float(i) / cards.size(),
			                            String(_("writing header ")) << i << _(" of ") << (int)cards.size());
		}
		card->writeHead(data);
	}
}





// ----------------------------------------------------------------------------- : Export dialog

/// Dialog for exporting a set to Apprentice
class ApprenticeExportWindow : public wxDialog, public WithProgress {
  public:
	ApprenticeExportWindow(Window* parent, const SetP& set);
	
	virtual void onProgress(float p, const String& message);
	void doStep(const String& s, float size);
	
  private:
	DECLARE_EVENT_TABLE();
	SetP set;
	
	// Gui controls
	wxTextCtrl *apprentice, *set_code;
	
	// In what step of the export process are we?
	String step;
	float step_begin, step_end;
	ExportProgressDialog* progress_target;
	
	void onApprenticeBrowse(wxCommandEvent& ev);
	void onOk(wxCommandEvent& ev);
	
	/// Export the set
	bool exportSet();
};


void export_apprentice(Window* parent, const SetP& set) {
	ApprenticeExportWindow wnd(parent, set);
	wnd.ShowModal();
}


ApprenticeExportWindow::ApprenticeExportWindow(Window* parent, const SetP& set)
	: wxDialog(parent, wxID_ANY, _("Export to Apprentice"), wxDefaultPosition)
	, set(set)
	, step_begin(0), step_end(0)
{
	if (!set->game->isMagic()) throw Error(_("Can only export Magic sets to Apprentice"));
	
	// create controls
	apprentice = new wxTextCtrl(this, wxID_ANY);
	set_code   = new wxTextCtrl(this, wxID_ANY);
	wxButton* browse = new wxButton(this, ID_APPRENTICE_BROWSE , _BUTTON_("browse"));
	// set values
	apprentice->SetValue(settings.apprentice_location);
	set_code->SetValue(set->apprentice_code);
	// init sizer
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
		// Apprentice location
		s->Add(new wxStaticText(this, wxID_ANY, _TITLE_("locate apprentice")), 0, wxALL, 4);
		wxSizer* s2 = new wxBoxSizer(wxHORIZONTAL);
			s2->Add(apprentice, 1, wxEXPAND | wxRIGHT, 4);
			s2->Add(browse,     0, wxEXPAND);
		s->Add(s2, 0, wxEXPAND | wxALL & ~wxTOP, 4);
		s->AddSpacer(8);
		// Set code
		s->Add(new wxStaticText(this, -1, _LABEL_("set code")), 0, wxALL, 4);
		s->Add(set_code, 0, wxEXPAND | wxLEFT | wxRIGHT, 4);
		s->Add(new wxStaticText(this, -1, _HELP_( "set code")), 0, wxALL, 4);
		s->AddSpacer(4);
		s->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL & ~wxTOP,    8);
	s->SetSizeHints(this);
	SetSizer(s);
}

void ApprenticeExportWindow::onApprenticeBrowse(wxCommandEvent& ev) {
	// browse for appr.exe
	// same as in DirsPreferencesPage
	wxFileDialog dlg(this, _TITLE_("locate apprentice"), apprentice->GetValue(), _(""), _LABEL_("apprentice exe") + _("|appr.exe"), wxOPEN);
	if (dlg.ShowModal() == wxID_OK) {
		wxFileName fn(dlg.GetPath());
		apprentice->SetValue(fn.GetPath());
	}
}

void ApprenticeExportWindow::onOk(wxCommandEvent& ev) {
	// store settings
	settings.apprentice_location = apprentice->GetValue();
	// store new code
	String new_set_code = set_code->GetValue();
	if (new_set_code.size() != 2) {
		wxMessageBox(_("The set code must be 2 characters long"),
			         _("Invalid set code"), wxOK | wxICON_ERROR);
		return;
	}
	new_set_code.MakeUpper();
	set->apprentice_code.MakeUpper();
	if (set->apprentice_code != new_set_code) {
		// changed something in the set
		set->apprentice_code = new_set_code;
		//set->actions.atSavePoint = false; // TODO: tell the user he needs to save
	}
	// Check if apprentice exists
	if (!wxFileExists(settings.apprentice_location + _("\\appr.exe"))) {
		wxMessageBox(_("Apprentice is not found in the specified location\n") + settings.apprentice_location,
		             _("Apprentice Not Found"), wxOK | wxICON_ERROR);
		return;
	}
	// create progress dialog
	progress_target = new ExportProgressDialog(this, _("Export to Apprentice"), _("Exporting to Apprentice, please wait"));
	progress_target->Show();
	// export!
	try {
		if (!exportSet()) {
			// canceled, but allow to try again
			progress_target->Hide();
			progress_target->Close();
			return;
		}
	} catch (const AbortException&) {
		// aborted, cleanup is already handled by dtors
		wxMessageBox(_LABEL_("apprentice export cancelled"), _TITLE_("export cancelled"), wxOK | wxICON_INFORMATION);
	}
	// Done, close progress window
	progress_target->Hide();
	progress_target->Close();
	// Close this window
	EndModal(wxID_OK);
}

bool ApprenticeExportWindow::exportSet() {
	// Expan database
	doStep(_("Exporting expansion"), 0.01f);
	ApprExpansionDatabase expan(this);
	expan.read();
	// Is there already a set with the desired code?
	map<String,String>::iterator expan_it = expan.expansions.find(set->apprentice_code); 
	if (expan_it != expan.expansions.end()) {
		int res = wxMessageBox(
			_("There is already a set with the code '")+ expan_it->first +_("' in the Apprentice database.\n") +
			_("This set has the name '")+ expan_it->second +_("'\n") +
			_("Do you want to continue, and overwrite that set?"),
			_("Overwrite Set?"), wxYES_NO | wxICON_EXCLAMATION
		);
		if (res == wxNO) return false; // abort export
	}
	// add our set
	expan.expansions[set->apprentice_code] = set->value<TextValue>(_("title")).value;
	expan.write();
	
	// Format database
	doStep(_("Exporting format"), 0.01f);
	ApprFormatDatabase format(this);
	format.read();
	// remove old with code, TODO add new?
	format.removeSet(set->apprentice_code);
	format.write();
	
	// Distro database
	doStep(_("Exporting distribution"), 0.01f);
	ApprDistroDatabase distro(this);
	distro.read();
	// remove old with code, add new
	distro.removeSet(set->apprentice_code);
	distro.distros[set->apprentice_code] = ApprDistro(11,3,1); // booster size
	distro.write();
	
	// Card database
	doStep(_("Exporting cards"), 0.96f);
	ApprCardDatabase cardlist(this);
	cardlist.read();
	// remove old cards with same code
	cardlist.removeSet(set->apprentice_code);
	// add cards from set
	FOR_EACH(card, set->cards) {
		ApprCardRecordP rec = new_intrusive2<ApprCardRecord>(*card, set->apprentice_code);
		cardlist.cards.push_back(rec);
	}
	cardlist.write();
	
	// Commit everything
	doStep(_("Committing"), 0.01f);
	expan.commit();
	format.commit();
	distro.commit();
	cardlist.commit();
	
	return true; // all went well
}

void ApprenticeExportWindow::onProgress(float p, const String& m) {
	progress_target->onProgress(
		p * (step_end - step_begin) + step_begin,
		m.empty() ? step : step + _(": ") + m
	);
}
void ApprenticeExportWindow::doStep(const String& s, float size) {
	step = s;
	step_begin = step_end;
	step_end += size;
	onProgress(0.0f, _(""));
}



BEGIN_EVENT_TABLE(ApprenticeExportWindow, wxDialog)
	EVT_BUTTON      (wxID_OK, ApprenticeExportWindow::onOk)
	EVT_BUTTON      (ID_APPRENTICE_BROWSE, ApprenticeExportWindow::onApprenticeBrowse)
END_EVENT_TABLE  ()
