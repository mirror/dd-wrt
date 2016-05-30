/* src/vm/rt-timing.hpp - POSIX real-time timing utilities

   Copyright (C) 1996-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

*/

/**
 * @file
 *
 * This file contains the real-time timing utilities.
 *
 * @ingroup rt-timing
 */

/**
 * @defgroup rt-timing Real-time timing framework
 * Real-time timing framework.
 *
 * For day to day use only the following macros are of importance:
 * - RT_REGISTER_TIMER(var,name,description)
 * - RT_REGISTER_GROUP_TIMER(var,name,description,group)
 * - RT_REGISTER_GROUP(var,name,description)
 * - RT_REGISTER_SUBGROUP(var,name,description,group)
 * - RT_TIMER_START(var)
 * - RT_TIMER_STOP(var)
 * - RT_TIMER_STOPSTART(var1,var2)
 * - RT_REGISTER_TIMER_EXTERN(var,name,description)
 * - RT_REGISTER_GROUP_TIMER_EXTERN(var,name,description,group)
 * - RT_REGISTER_GROUP_EXTERN(var,name,description)
 * - RT_REGISTER_SUBGROUP_EXTERN(var,name,description,group)
 * - RT_DECLARE_TIMER(var)
 * - RT_DECLARE_GROUP(var)
 *
 * A typical usage would look something like this:
 *
 * @code
 * // NOTE: toplevel, outside a function!
 * RT_REGISTER_GROUP(my_group,"my-group","this is my very own timing group");
 * RT_REGISTER_GROUP_TIMER(my_timer1,"my-timer1","this is my first timer",my_group);
 * RT_REGISTER_GROUP_TIMER(my_timer2,"my-timer1","this is my second timer",my_group);
 *
 * void do_something() {
 *    RT_TIMER_START(my_timer1);
 *    // do some work
 *    RT_TIMER_STOPSTART(my_timer1,my_timer2);
 *    // do even more work
 *    RT_TIMER_STOP(my_timer2);
 * }
 * @endcode
 *
 * @note
 * The implementation of the RT_REGISTER_* macros is a bit tricky. In general
 * we simply want to create global variables for groups and timers. Unfortunately
 * RTGroup and RTTimer are no "plain old datatypes" (pod). There is a constructor
 * which is dependant on other global non-pod variables. The C++ standard does not
 * guarantee any specific order of construction so it might be the case that
 * a subgroup is registered before its super group. There are many dirty tricks to
 * work around this issue. The one we are using does the following. We create a global
 * variable as well as a function which returns this variable. The C++ standard
 * guarantees that a global variable is initialized before its first use. Therefore,
 * if we access the group/timer variables only through these functions it is
 * guaranteed that everything is initialized in the right order.
 * Also note that the variable must be global (i.e. they can not be declared as
 * static function variables) otherwise the objects might be destroyed eagerly.
 */

/**
 * @addtogroup rt-timing
 * @{
 */

/**
 * @def RT_REGISTER_TIMER(var,name,description)
 * Register a new (toplevel) timer. Create a timer and add it to the toplevel
 * timing group RTGroup::root().
 *
 * @note This macro creates a function RTTimer& var(). Consider using namespaces
 * to avoid name clashes.
 */

/**
 * @def RT_REGISTER_GROUP_TIMER(var,name,description,group)
 * Register a new timer. Create a timer and add it to group specified.
 *
 * @note This macro creates a function RTTimer& var(). Consider using namespaces
 * to avoid name clashes.
 */

/**
 * @def RT_REGISTER_GROUP(var,name,description)
 * Register a new (toplevel) group. Create a group and add it to the toplevel
 * timing group RTGroup::root().
 *
 * @note This macro creates a function RTGroup& var(). Consider using namespaces
 * to avoid name clashes.
 */

/**
 * @def RT_REGISTER_SUBGROUP(var,name,description,group)
 * Register a new subgroup. Create a group and add it to group specified.
 *
 * @note This macro creates a function RTGroup& var(). Consider using namespaces
 * to avoid name clashes.
 */

/**
 * @def RT_REGISTER_TIMER_EXTERN(var,name,description)
 *
 * Define external timer.
 *
 * @see RT_REGISTER_TIMER
 * @see RT_DECLARE_TIMER
 **/

/**
 * @def RT_REGISTER_GROUP_TIMER_EXTERN(var,name,description,group)
 *
 * Define external group timer.
 *
 * @see RT_REGISTER_GROUP_TIMER
 * @see RT_DECLARE_TIMER
 **/

/**
 * @def RT_REGISTER_GROUP_EXTERN(var,name,description)
 *
 * Define external group.
 *
 * @see RT_REGISTER_GROUP
 * @see RT_DECLARE_GROUP
 **/

/**
 * @def RT_REGISTER_SUBGROUP_EXTERN(var,name,description,group)
 *
 * Define external subgroup.
 *
 * @see RT_REGISTER_SUBGROUP
 * @see RT_DECLARE_GROUP
 **/

/**
 * @def RT_DECLARE_TIMER(var)
 *
 * Declare an external timer variable.
 *
 * @see RT_REGISTER_TIMER_EXTERN
 * @see RT_REGISTER_GROUP_TIMER_EXTERN
 **/

/**
 * @def RT_DECLARE_GROUP(var)
 *
 * Declare an external timer group.
 *
 * @see RT_REGISTER_GROUP_EXTERN
 * @see RT_REGISTER_SUBGROUP_EXTERN
 **/

/**
 * @def RT_TIMER_START(var)
 * Start the timer var.
 */

/**
 * @def RT_TIMER_STOP(var)
 * Stop the timer var.
 */

/**
 * @def RT_TIMER_STOPSTART(var1,var2)
 * Stop the timer var1 and start the timer var2
 */

/** @} */
#ifndef _RT_TIMING_HPP
#define _RT_TIMING_HPP

#include "config.h"

#if defined(ENABLE_RT_TIMING)

#include <ctime>
#include <cstdlib>
#include <cerrno>

#include "vm/types.hpp"

#include "mm/memory.hpp"

#include "vm/global.hpp"

#include "toolbox/logging.hpp"

// debugging macro
// @note: LOG* can not be used because opt_DebugName is not initialized
#if 0
#define _RT_LOG(expr) do { cacao::dbg() <<expr;} while (0)
#else
#define _RT_LOG(expr)
#endif

namespace {
#if 0
/**
 * @note: http://www.gnu.org/software/libc/manual/html_node/Elapsed-Time.html
 */
inline timespec operator-(const timespec &a, timespec b) {
	timespec result;
	/* Perform the carry for the later subtraction by updating b. */
	if (a.tv_nsec < b.tv_nsec) {
		int xsec = (b.tv_nsec - a.tv_nsec) / 1000000000L + 1;
		b.tv_nsec -= 1000000000L * xsec;
		b.tv_sec += xsec;
	}
	if (a.tv_nsec - b.tv_nsec > 1000000000L) {
		int xsec = (a.tv_nsec - b.tv_nsec) / 1000000000L;
		b.tv_nsec += 1000000000L * xsec;
		b.tv_sec -= xsec;
	}

	/* Compute the time remaining to wait. tv_usec is certainly positive. */
	result.tv_sec = a.tv_sec - b.tv_sec;
	result.tv_nsec = a.tv_nsec - b.tv_nsec;

	return result;
}

inline timespec operator+(const timespec a, const timespec &b) {
	timespec result;
	result.tv_sec = a.tv_sec + b.tv_sec;
	result.tv_nsec = a.tv_nsec + b.tv_nsec;
	if (result.tv_nsec > 1000000000L) {
		result.tv_nsec -= 1000000000L;
		result.tv_sec ++;
	}

	return result;
}
#endif

inline void operator+=(timespec &result, const timespec &b) {
	result.tv_sec += b.tv_sec;
	result.tv_nsec += b.tv_nsec;
	if (result.tv_nsec > 1000000000L) {
		result.tv_nsec -= 1000000000L;
		result.tv_sec ++;
	}
}

inline timespec operator/(const timespec &a, const int b) {
	long int r; // remainder
	timespec result;

	if (b==0) {
		result.tv_sec = -1;
		result.tv_nsec = -1;
		return result;
	}

	r = a.tv_sec % b;
	result.tv_sec = a.tv_sec / b;
	result.tv_nsec = r * 1000000000L / b;
	result.tv_nsec += a.tv_nsec / b;

	return result;
}

/**
 * @note: this is not accurate
 */
inline long int operator/(const timespec &a, const timespec &b) {
	if (a.tv_sec != 0) {
		if (b.tv_sec == 0) {
			return -1;
		} else {
			return a.tv_sec / b.tv_sec;
		}
	} else {
		if (b.tv_nsec == 0) {
			return -1;
		} else {
			return a.tv_nsec / b.tv_nsec;
		}
	}
}

inline bool operator==(const timespec &a, const timespec &b) {
	return (a.tv_sec == b.tv_sec) && (a.tv_nsec == b.tv_nsec);
}
} // end anonymous namespace

namespace cacao {
namespace {
inline OStream& operator<<(OStream &ostr, timespec ts) {
	const char *unit;
	if (ts.tv_sec >= 10) {
		// display seconds if at least 10 sec
		ostr << ts.tv_sec;
		unit = "sec";
	} else {
		ts.tv_nsec += ts.tv_sec * 1000000000L;
		if (ts.tv_nsec >= 100000000) {
			// display milliseconds if at least 100ms
			ostr << ts.tv_nsec/1000000;
			unit = "msec";
		} else {
			// otherwise display microseconds
			ostr << ts.tv_nsec/1000;
			unit = "usec";
		}
	}
	ostr << setw(5) << unit;
	return ostr;
}

inline void rt_timing_gettime_inline(timespec &ts) {
	if (clock_gettime(CLOCK_THREAD_CPUTIME_ID,&ts) != 0) {
		fprintf(stderr,"could not get time by clock_gettime: %s\n",strerror(errno));
		abort();
	}
}

inline long rt_timing_diff_usec_inline(const timespec &a, const timespec &b)
{
	long diff;
	time_t atime;

	diff = (b.tv_nsec - a.tv_nsec) / 1000;
	atime = a.tv_sec;
	while (atime < b.tv_sec) {
		atime++;
		diff += 1000000;
	}
	return diff;
}

} // end anonymous namespace
} // end namespace cacao

#include <vector>

namespace cacao {

/**
 * @addtogroup rt-timing
 * @{
 */

/**
 * Superclass of real-time group entries.
 */
class RTEntry {
protected:
	const char* name;
	const char* description;
public:
	static timespec invalid_ts;   //< invalid time stamp
	/**
	 * Constructor.
	 */
	RTEntry(const char* name, const char* description) : name(name), description(description) {
		_RT_LOG("RTEntry() name: " << name << nl);
	}
	/**
	 * Destructor.
	 */
	virtual ~RTEntry() {
		_RT_LOG("~RTEntry() name: " << name << nl);
	}
	/**
	 * Print the timing information to an OStream.
	 * @param[in,out] O     output stream
	 * @param[in]     ref   time reference. Used to calculate percentage .
	 *	Normally the time of the parent.
	 */
	virtual void print(OStream &O,timespec ref) const = 0;
	/**
	 * Get the elapsed time of a RTEntry.
	 */
	virtual timespec time() const = 0;

};

/**
 * Real-time group.
 * Used to collect a set of RTEntry's
 */
class RTGroup : public RTEntry {
private:
	typedef std::vector<RTEntry*> RTEntryList;
	/// List of members.
	RTEntryList members;

	RTGroup(const char* name, const char* description) : RTEntry(name, description) {
		_RT_LOG("RTGroup() name: " << name << nl);
	}
public:
	/**
	 * Get the root group
	 */
	static RTGroup& root() {
		static RTGroup rg("vm","vm");
		return rg;
	}

	/**
	 * Create a new real-time group.
	 * @param[in] name name of the group
	 * @param[in] description description of the group
	 * @param[in] group parent group.
	 */
	RTGroup(const char* name, const char* description, RTGroup &group) : RTEntry(name, description) {
		group.add(this);
	}

	virtual ~RTGroup() {
		_RT_LOG("~RTGroup() name: " << name << nl);
	}

	/**
	 * Add an entry to the group
	 */
	void add(RTEntry *re) {
		members.push_back(re);
	}

	virtual timespec time() const {
		timespec time = {0,0};
		for(RTEntryList::const_iterator i = members.begin(), e = members.end(); i != e; ++i) {
			RTEntry* re = *i;
			time += re->time();
		}
		return time;
	}

	virtual void print(OStream &O,timespec ref = invalid_ts) const {
		timespec duration = time();
		if (ref == invalid_ts)
			ref = duration;
		// O << setw(10) << left << name << right <<"   " << description << nl;
		//O << indent;
		for(RTEntryList::const_iterator i = members.begin(), e = members.end(); i != e; ++i) {
			RTEntry* re = *i;
			re->print(O,duration);
		}
		//O << dedent;
		int percent = duration / (ref / 100);
		O << bold
		  << setw(10) << duration << " "
		  << setw(4) << percent << "% "
		  << setw(20) << name << ": total time"
		  << reset_color
		  << nl<< nl;
	}
};

/**
 * Real-time timer.
 *
 * @Cpp11 should be ported to std::chrono if C++11 is available
 */
class RTTimer : public RTEntry {
private:
	timespec startstamp;  //< start timestamp
	long int duration;    //< time in usec
public:
	/**
	 * Create a new real-time timer.
	 * @param[in] name name of the timer
	 * @param[in] description description of the timer
	 * @param[in] group parent group.
	 */
	RTTimer(const char* name, const char* description, RTGroup &parent) : RTEntry(name, description) {
		//reset();
		_RT_LOG("RTTimer() name: " << name << nl);
		duration = 0;
		parent.add(this);
	}

	/**
	 * Destructor
	 */
	virtual ~RTTimer() {
		_RT_LOG("~RTTimer() name: " << name << nl);
	}

	/**
	 * Start the timer.
	 * @see stop()
	 */
	inline void start() {
		rt_timing_gettime_inline(startstamp);
	}

	/**
	 * Start the timer.
	 * @note Only use after a start() or stopstart()
	 * @see start()
	 */
	inline void stop() {
		timespec stopstamp;
		rt_timing_gettime_inline(stopstamp);
		duration += rt_timing_diff_usec_inline(startstamp,stopstamp);
	}

	virtual timespec time() const {
		timespec ts;
		int sec = duration/1000000;
		ts.tv_sec = sec;
		ts.tv_nsec = (duration - sec * 1000000) * 1000;
		return ts;
	}

	virtual void print(OStream &O,timespec ref = invalid_ts) const {
		timespec ts = time();
		int percent;

		if (ref == invalid_ts) {
			percent = -1;
		} else {
			percent = ts / (ref / 100);
		}

		O << setw(10) << ts.tv_nsec/1000 << " usec "
		  << setw(4) << percent << "% "
		  << setw(20) << name << ": " << description << nl;
	}
};

/** @} */

} // end namespace cacao

#undef _RT_LOG

#define RT_REGISTER_TIMER(var,name,description)                              \
	static inline cacao::RTTimer& var() {	                                 \
		static cacao::RTTimer v(name,description,cacao::RTGroup::root());    \
		return v;                                                            \
	}

#define RT_REGISTER_GROUP_TIMER(var,name,description,group)                  \
	static inline cacao::RTTimer& var() {                                    \
		static cacao::RTTimer v(name, description, group());                 \
		return v;                                                            \
	}

#define RT_REGISTER_GROUP(var,name,description)                              \
	inline cacao::RTGroup& var() {                                           \
		static cacao::RTGroup v(name,description,cacao::RTGroup::root());    \
		return v;                                                            \
	}

#define RT_REGISTER_SUBGROUP(var,name,description,group)                     \
	inline cacao::RTGroup& var() {                                           \
		static cacao::RTGroup v(name, description, group());                 \
		return v;                                                            \
	}


#define RT_REGISTER_TIMER_EXTERN(var,name,description)                       \
	inline cacao::RTTimer& var() {	                                         \
		static cacao::RTTimer v(name,description,cacao::RTGroup::root());    \
		return v;                                                            \
	}

#define RT_REGISTER_GROUP_TIMER_EXTERN(var,name,description,group)           \
	inline cacao::RTTimer& var() {                                           \
		static cacao::RTTimer v(name, description, group());                 \
		return v;                                                            \
	}

#define RT_REGISTER_GROUP_EXTERN(var,name,description)                       \
	inline cacao::RTGroup& var() {                                           \
		static cacao::RTGroup v(name,description,cacao::RTGroup::root());    \
		return v;                                                            \
	}

#define RT_REGISTER_SUBGROUP_EXTERN(var,name,description,group)              \
	inline cacao::RTGroup& var() {                                           \
		static cacao::RTGroup v(name, description, group());                 \
		return v;                                                            \
	}

#define RT_DECLARE_TIMER(var) inline cacao::RTTimer& var();
#define RT_DECLARE_GROUP(var) inline cacao::RTGroup& var();

#define RT_TIMER_START(var) var().start()
#define RT_TIMER_STOP(var) var().stop()
#define RT_TIMER_STOPSTART(var1,var2) do { var1().stop();var2().start(); } while(0)

#else /* !defined(ENABLE_RT_TIMING) */

#define RT_REGISTER_TIMER(var,name,description)
#define RT_REGISTER_GROUP_TIMER(var,name,description,group)
#define RT_REGISTER_GROUP(var,name,description)
#define RT_REGISTER_SUBGROUP(var,name,description,group)

#define RT_REGISTER_TIMER_EXTERN(var,name,description)
#define RT_REGISTER_GROUP_TIMER_EXTERN(var,name,description,group)
#define RT_REGISTER_GROUP_EXTERN(var,name,description)
#define RT_REGISTER_SUBGROUP_EXTERN(var,name,description,group)

#define RT_DECLARE_TIMER(var)
#define RT_DECLARE_GROUP(var)

#define RT_TIMER_START(var)
#define RT_TIMER_STOP(var)
#define RT_TIMER_STOPSTART(var1,var2)

#endif /* defined(ENABLE_RT_TIMING) */

#endif /* _RT_TIMING_HPP */

/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */
