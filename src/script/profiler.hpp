//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_SCRIPT_PROFILER
#define HEADER_SCRIPT_PROFILER

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <script/script.hpp>
#include <script/context.hpp>

#ifndef USE_SCRIPT_PROFILING
#define USE_SCRIPT_PROFILING 1
#endif

#if USE_SCRIPT_PROFILING

DECLARE_POINTER_TYPE(FunctionProfile);

// ----------------------------------------------------------------------------- : Timer

#ifdef WIN32
	typedef LONGLONG ProfileTime;

	inline ProfileTime timer_now() {
		LARGE_INTEGER i;
		QueryPerformanceCounter(&i);
		return i.QuadPart;
	}
	inline ProfileTime timer_resolution() {
		LARGE_INTEGER i;
		QueryPerformanceFrequency(&i);
		return i.QuadPart;
	}

	inline const char * mangled_name(const type_info& t) {
		return t.raw_name();
	}
#else
	// clock() has nanosecond resolution on Linux
	// on any other platform, stil the best way.
	typedef clock_t ProfileTime;

	inline ProfileTime timer_now() {
		return clock();
	}
	inline ProfileTime timer_resolution() {
		return CLOCKS_PER_SEC;
	}

	inline const char * mangled_name(const type_info& t) {
		return t.name();
	}
#endif

/// Simple execution timer
class Timer {
  public:
	Timer();
	/// The time the timer has been running, resets the timer
	inline ProfileTime time();
	/// Exclude the time since the last reset from ALL running timers
	inline void exclude_time();
  private:
	ProfileTime start;
	static ProfileTime delta; ///< Time excluded
};

// ----------------------------------------------------------------------------- : FunctionProfile

/// How much time was spent in a function?
class FunctionProfile : public IntrusivePtrBase<FunctionProfile> {
  public:
	FunctionProfile(const String& name) : name(name), time_ticks(0), calls(0) {}

	String      name;
	ProfileTime time_ticks;
	UInt        calls;
	/// for each id, called children
	/** we (ab)use the fact that all pointers are even to store both pointers and ids */
	map<size_t,FunctionProfileP> children;

	/// The children, sorted by time
	void get_children(vector<FunctionProfileP>& out) const;

	/// Time in seconds
	inline double time() const { return time_ticks / (double)timer_resolution(); }
	inline double avg_time() const { return time() / calls; }
};

/// The root profile
extern FunctionProfile profile_root;

const FunctionProfile& profile_aggregated(int level = 1);

// ----------------------------------------------------------------------------- : Profiler

/// Profile a single function call
class Profiler {
  public:
	/// Log the fact that the function  function_name  is entered, ends when profiler goes out of scope.
	/** Time between the construction of Timer and the construction of Profiler is excluded from ALL profiles.
	 */
	Profiler(Timer& timer, Variable function_name);
	/// As above, but with a constant name
	Profiler(Timer& timer, const Char* function_name);
	/// As above, but using a function object instead of a name,
	/** if we haven't seen the object before, it gets the given name. */
	Profiler(Timer& timer, void* function_object, const String& function_name);
	/// Log the fact that the function is left
	~Profiler();
  private:
	Timer&                  timer;
	static FunctionProfile* function; ///< function we are in
	FunctionProfile*        parent;
};

// ----------------------------------------------------------------------------- : EOF
#endif
#endif
