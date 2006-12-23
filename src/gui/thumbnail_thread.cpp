//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/thumbnail_thread.hpp>
#include <wx/thread.h>

typedef pair<ThumbnailRequestP,Image> pair_ThumbnailRequestP_Image;
DECLARE_TYPEOF_COLLECTION(pair_ThumbnailRequestP_Image);

// ----------------------------------------------------------------------------- : Image Cache

String user_settings_dir();
String image_cache_dir() {
	String dir = user_settings_dir() + _("/cache");
	if (!wxDirExists(dir)) wxMkDir(dir);
	return dir + _("/");
}

/// A name that is safe to use as a filename, for the cache
String safe_filename(const String& str) {
	String ret; ret.reserve(str.size());
	FOR_EACH_CONST(c, str) {
		if (isAlnum(c)) {
			ret += c;
		} else if (c==_(' ') || c==_('-')) {
			ret += _('-');
		} else {
			ret += _('_');
		}
	}
	return ret;
}

// ----------------------------------------------------------------------------- : ThumbnailThreadWorker

class ThumbnailThreadWorker : public wxThread {
  public:
	ThumbnailThreadWorker(ThumbnailThread* parent);
	
	virtual ExitCode Entry();
	
	ThumbnailRequestP current; ///< Request we are working on
	ThumbnailThread*  parent;
	bool              stop; ///< Suspend computation
};

ThumbnailThreadWorker::ThumbnailThreadWorker(ThumbnailThread* parent)
	: parent(parent)
	, stop(false)
{}

wxThread::ExitCode ThumbnailThreadWorker::Entry() {
	while (true) {
		do {
			Sleep(1);
			if (TestDestroy()) return 0;
		} while (stop);
		// get a request
		{
			wxMutexLocker lock(parent->mutex);
			if (parent->open_requests.empty()) {
				parent->worker = nullptr;
				return 0; // No more requests
			}
			current = parent->open_requests.front();
			parent->open_requests.pop_front();
		}
		// perform request
		if (TestDestroy()) return 0;
		Image img = current->generate();
		if (TestDestroy()) return 0;
		// store in cache
		if (img.Ok()) {
			String filename = image_cache_dir() + safe_filename(current->cache_name) + _(".png");
			img.SaveFile(filename, wxBITMAP_TYPE_PNG);
			// set modification time
			wxFileName fn(filename);
			fn.SetTimes(0, &current->modified, 0);
		}
		// store result in closed request list
		{
			wxMutexLocker lock(parent->mutex);
			parent->closed_requests.push_back(make_pair(current,img));
			current.reset();
			parent->completed.Signal();
		}
	}
}

bool operator < (const ThumbnailRequestP& a, const ThumbnailRequestP& b) {
	if (a->owner < b->owner) return true;
	if (a->owner > b->owner) return false;
	return a->cache_name < b->cache_name;
}

// ----------------------------------------------------------------------------- : ThumbnailThread

ThumbnailThread thumbnail_thread;

ThumbnailThread::ThumbnailThread()
	: completed(mutex)
	, worker(nullptr)
{}

ThumbnailThread::~ThumbnailThread() {
	abortAll();
}

void ThumbnailThread::request(const ThumbnailRequestP& request) {
	assert(wxThread::IsMain());
	// Is the request in progress?
	if (request_names.find(request) != request_names.end()) {
		return;
	}
	request_names.insert(request);
	// Is the image in the cache?
	String filename = image_cache_dir() + safe_filename(request->cache_name) + _(".png");
	wxFileName fn(filename);
	if (fn.FileExists()) {
		wxDateTime modified;
		if (fn.GetTimes(0, &modified, 0) && modified >= request->modified) {
			// yes it is
			Image img(filename);
			request->store(img);
			return;
		}
	}
	// request generation
	{
		wxMutexLocker lock(mutex);
		open_requests.push_back(request);
	}
	// is there a worker?
	if (!worker) {
		worker = new ThumbnailThreadWorker(this);
		worker->Create();
		worker->Run();
	}
}

bool ThumbnailThread::done(void* owner) {
	assert(wxThread::IsMain());
	// find finished requests
	vector<pair<ThumbnailRequestP,Image> > finished;
	{
		wxMutexLocker lock(mutex);
		for (size_t i = 0 ; i < closed_requests.size() ; ) {
			if (closed_requests[i].first->owner == owner) {
				// move to finished list
				finished.push_back(closed_requests[i]);
				closed_requests.erase(closed_requests.begin() + i, closed_requests.begin() + i + 1);
			} else {
				++i;
			}
		}
	}
	// store them
	FOR_EACH(r, finished) {
		// store image
		r.first->store(r.second);
		// remove from name list
		request_names.erase(r.first);
	}
	return !finished.empty();
}

void ThumbnailThread::abort(void* owner) {
	assert(wxThread::IsMain());
	mutex.Lock();
	if (worker && worker->current->owner == owner) {
		// a request for this owner is in progress, wait until it is done
		worker->stop = true;
		completed.Wait();
		mutex.Lock();
		worker->stop = false;
	}
	// remove open requests for this owner
	for (size_t i = 0 ; i < open_requests.size() ; ) {
		if (open_requests[i]->owner == owner) {
			// remove
			open_requests.erase(open_requests.begin() + i, open_requests.begin() + i + 1);
			request_names.erase(open_requests[i]);
		} else {
			++i;
		}
	}
	// remove closed requests for this owner
	for (size_t i = 0 ; i < closed_requests.size() ; ) {
		if (closed_requests[i].first->owner == owner) {
			// remove
			closed_requests.erase(closed_requests.begin() + i, closed_requests.begin() + i + 1);
			request_names.erase(closed_requests[i].first);
		} else {
			++i;
		}
	}
	mutex.Unlock();
}

void ThumbnailThread::abortAll() {
	assert(wxThread::IsMain());
	mutex.Lock();
	open_requests.clear();
	closed_requests.clear();
	request_names.clear();
	if (worker) {
		// a request is in progress, wait until it is done, killing the worker
		completed.Wait();
	} else {
		mutex.Unlock();
	}
}
