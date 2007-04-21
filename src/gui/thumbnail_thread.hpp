//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_THUMBNAIL_THREAD
#define HEADER_GUI_THUMBNAIL_THREAD

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <wx/datetime.h>
#include <wx/filename.h>
#include <queue>

DECLARE_POINTER_TYPE(ThumbnailRequest);
class ThumbnailThreadWorker;

// ----------------------------------------------------------------------------- : ThumbnailRequest

/// A request for some kind of thumbnail
class ThumbnailRequest {
  public:
	ThumbnailRequest(void* owner, const String& cache_name, const wxDateTime& modified)
		: owner(owner), cache_name(cache_name), modified(modified) {}
	
	virtual ~ThumbnailRequest() {}
	
	/// Generate the thumbnail, called in another thread
	virtual Image generate() = 0;
	/// Store the thumbnail, called from the main thread
	virtual void store(const Image&) = 0;
	
	/// Object that requested the thumbnail
	void* const owner;
	/// Name under which this object will be stored in the image cache
	String cache_name;
	/// Modification time for the object of which the thumnail is generated
	wxDateTime modified;
};

// ----------------------------------------------------------------------------- : ThumbnailThread

/// A (generic) class that generates thumbnails in another thread
/** All requests have an 'owner', the object that requested the thumbnail.
 *  This object should regularly call "done(this)".
 *  Multiple requests can be open at the same time.
 *  Thumbnails are cached, and need not be generated in a thread
 */
class ThumbnailThread {
  public:
	ThumbnailThread();
	
	/// Request a thumbnail, it may be store()d immediatly if the thumbnail is cached
	void request(const ThumbnailRequestP& request);
	/// Is one or more thumbnail for the given owner finished?
	/** If so, call their store() functions */
	bool done(void* owner);
	/// Abort all thumbnail requests for the given owner
	void abort(void* owner);
	/// Abort all computations
	/** *must* be called at application exit */
	void abortAll();
	
  private:
	wxMutex     mutex;  ///< Mutex used by the worker when accessing the request lists or the thread pointer
	wxCondition completed; ///< Event signaled when a request is completed
	
	deque<ThumbnailRequestP>                open_requests;		///< Requests on which work hasn't finished
	vector<pair<ThumbnailRequestP,Image> >  closed_requests;	///< Requests for which work is completed
	set<ThumbnailRequestP>                  request_names;		///< Requests that haven't been stored yet, to prevent duplicates
	friend class ThumbnailThreadWorker;
	ThumbnailThreadWorker* worker;								///< The worker thread. invariant: no requests ==> worker==nullptr
};

/// The global thumbnail generator thread
extern ThumbnailThread thumbnail_thread;

// ----------------------------------------------------------------------------- : EOF
#endif
