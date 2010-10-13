//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <script/profiler.hpp>

#if USE_SCRIPT_PROFILING

// don't use script profiling in final build
#ifndef UNICODE
	#error "It looks like you are building the final release; disable USE_SCRIPT_PROFILING!"
#endif

DECLARE_TYPEOF(map<size_t COMMA FunctionProfileP>);

// ----------------------------------------------------------------------------- : Timer

Timer::Timer() {
	start = timer_now() + delta;
}

ProfileTime Timer::time() {
	ProfileTime end = timer_now() + delta;
	ProfileTime diff = end - start;
	start = end;
	return diff;
}

void Timer::exclude_time() {
	ProfileTime delta_delta = time();
	delta -= delta_delta;
	start -= delta_delta;
}

ProfileTime Timer::delta = 0;

// ----------------------------------------------------------------------------- : FunctionProfile

FunctionProfile profile_root(_("root"));

inline bool compare_time(const FunctionProfileP& a, const FunctionProfileP& b) {
	return a->time_ticks < b->time_ticks;
}
void FunctionProfile::get_children(vector<FunctionProfileP>& out) const {
	FOR_EACH_CONST(c,children) {
		out.push_back(c.second);
	}
	sort(out.begin(), out.end(), compare_time);
}

// note: not thread safe
FunctionProfile profile_aggr(_("everywhere"));

void profile_aggregate(FunctionProfile& parent, int level, int max_level, const FunctionProfile& p);
void profile_aggregate(FunctionProfile& parent, int level, int max_level, size_t idx, const FunctionProfile& p) {
	// add to item at idx
	FunctionProfileP& fpp = parent.children[idx];
	if (!fpp) {
		fpp = intrusive(new FunctionProfile(p.name));
	}
	fpp->time_ticks += p.time_ticks;
	fpp->calls      += p.calls;
	// recurse
	if (level == 0) {
		profile_aggregate(parent, level, max_level, p);
	}
	if (level < max_level) {
		profile_aggregate(*fpp, level + 1, max_level, p);
	}
}
void profile_aggregate(FunctionProfile& parent, int level, int max_level, const FunctionProfile& p) {
	FOR_EACH_CONST(c, p.children) {
		profile_aggregate(parent, level, max_level, c.first, *c.second);
	}
}

const FunctionProfile& profile_aggregated(int max_level) {
	profile_aggr.children.clear();
	profile_aggregate(profile_aggr, 0, max_level, profile_root);
	return profile_aggr;
}

// ----------------------------------------------------------------------------- : Profiler

FunctionProfile* Profiler::function = &profile_root;

// Enter a function
Profiler::Profiler(Timer& timer, Variable function_name)
	: timer(timer)
	, parent(function) // push
{
	if ((int)function_name >= 0) {
		FunctionProfileP& fpp = parent->children[(size_t)function_name << 1 | 1];
		if (!fpp) {
			fpp = intrusive(new FunctionProfile(variable_to_string(function_name)));
		}
		function = fpp.get();
	}
	timer.exclude_time();
}

// Enter a function
Profiler::Profiler(Timer& timer, const Char* function_name)
	: timer(timer)
	, parent(function) // push
{
	FunctionProfileP& fpp = parent->children[(size_t)function_name];
	if (!fpp) {
		fpp = intrusive(new FunctionProfile(function_name));
	}
	function = fpp.get();
	timer.exclude_time();
}

// Enter a function
Profiler::Profiler(Timer& timer, void* function_object, const String& function_name)
	: timer(timer)
	, parent(function) // push
{
	FunctionProfileP& fpp = parent->children[(size_t)function_object];
	if (!fpp) {
		fpp = intrusive(new FunctionProfile(function_name));
	}
	function = fpp.get();
	timer.exclude_time();
}

// Leave a function
Profiler::~Profiler() {
	ProfileTime time = timer.time();
	if (function == parent) return; // don't count
	function->time_ticks += time;
	function->time_ticks_max = max(function->time_ticks_max,time);
	function->calls      += 1;
	function = parent; // pop
}

// ----------------------------------------------------------------------------- : EOF
#endif
