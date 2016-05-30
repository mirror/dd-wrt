 /* src/native/vm/openjdk/management.cpp - HotSpot management interface functions

   Copyright (C) 1996-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO
   Copyright (C) 2008, 2009 Theobroma Systems Ltd.

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


#include "config.h"

// Include our JNI header before the JMM header, because the JMM
// header include jni.h and we want to override the typedefs in jni.h.
#include "native/jni.hpp"

#include INCLUDE_JMM_H

#include "native/vm/openjdk/management.hpp"

#include "threads/threadlist.hpp"

#include "toolbox/logging.hpp"

#include "vm/array.hpp"
#include "vm/classcache.hpp"
#include "vm/globals.hpp" // XXX Remove!!!
#include "vm/options.hpp"
#include "vm/os.hpp"
#include "vm/vm.hpp"


/* debugging macros ***********************************************************/

#if !defined(NDEBUG)

# define TRACEJMMCALLS(x)               \
	do {                                \
		if (opt_TraceJMMCalls) {        \
			log_println x;              \
		}                               \
	} while (0)

#else

# define TRACEJMMCALLS(x)

#endif


/**
 * Initialize the Management subsystem.
 */
Management::Management()
{
	// Initialize optional support
	_optional_support.isLowMemoryDetectionSupported = 1;
	_optional_support.isCompilationTimeMonitoringSupported = 1;
	_optional_support.isThreadContentionMonitoringSupported = 1;

// 	if (os::is_thread_cpu_time_supported()) {
	if (false) {
		_optional_support.isCurrentThreadCpuTimeSupported = 1;
		_optional_support.isOtherThreadCpuTimeSupported = 1;
	}
	else {
		_optional_support.isCurrentThreadCpuTimeSupported = 0;
		_optional_support.isOtherThreadCpuTimeSupported = 0;
	}

	_optional_support.isBootClassPathSupported = 1;
	_optional_support.isObjectMonitorUsageSupported = 1;
	_optional_support.isSynchronizerUsageSupported = 1;
}


/**
 * Return a pointer to the optional support structure.
 *
 * @param Pointer to optional support structure.
 */
const jmmOptionalSupport& Management::get_optional_support() const
{
	return _optional_support;
}


// Interface functions are exported as C functions.
extern "C" {

jint jmm_GetVersion(JNIEnv* env)
{
	return JMM_VERSION;
}


jint jmm_GetOptionalSupport(JNIEnv* env, jmmOptionalSupport* support)
{
	if (support == NULL) {
		return -1;
	}

	Management& mm = VM::get_current()->get_management();

	// memcpy the structure.
	os::memcpy(support, &mm.get_optional_support(), sizeof(jmmOptionalSupport));

	return 0;
}


jobject jmm_GetInputArguments(JNIEnv* env)
{
	log_println("jmm_GetInputArguments: IMPLEMENT ME!");
	return NULL;
}


jobjectArray jmm_GetInputArgumentArray(JNIEnv* env)
{
	log_println("jmm_GetInputArgumentArray: IMPLEMENT ME!");
	return NULL;
}


jobjectArray jmm_GetMemoryPools(JNIEnv* env, jobject obj)
{
	TRACEJMMCALLS(("jmm_GetMemoryPools(env=%p, obj=%p)", env, obj));

	// XXX This should be an array of java/lang/management/MemoryPoolMXBean.
	log_println("jmm_GetMemoryPools: FIX ME!");
	ObjectArray oa(0, class_java_lang_String);

	return oa.get_handle();
}


jobjectArray jmm_GetMemoryManagers(JNIEnv* env, jobject obj)
{
	TRACEJMMCALLS(("jmm_GetMemoryManagers(env=%p, obj=%p)", env, obj));

	// XXX This should be an array of java/lang/management/MemoryManagerMXBean.
	log_println("jmm_GetMemoryManagers: FIX ME!");
	ObjectArray oa(0, class_java_lang_String);

	return oa.get_handle();
}


jobject jmm_GetMemoryPoolUsage(JNIEnv* env, jobject obj)
{
	log_println("jmm_GetMemoryPoolUsage: IMPLEMENT ME!");
	return NULL;
}


jobject jmm_GetPeakMemoryPoolUsage(JNIEnv* env, jobject obj)
{
	log_println("jmm_GetPeakMemoryPoolUsage: IMPLEMENT ME!");
	return NULL;
}


jobject jmm_GetPoolCollectionUsage(JNIEnv* env, jobject obj)
{
	log_println("jmm_GetPoolCollectionUsage: IMPLEMENT ME!");
	return NULL;
}


void jmm_SetPoolSensor(JNIEnv* env, jobject obj, jmmThresholdType type, jobject sensorObj)
{
	log_println("jmm_SetPoolSensor: IMPLEMENT ME!");
}


jlong jmm_SetPoolThreshold(JNIEnv* env, jobject obj, jmmThresholdType type, jlong threshold)
{
	log_println("jmm_SetPoolThreshold: IMPLEMENT ME!");
	return 0;
}


jobject jmm_GetMemoryUsage(JNIEnv* env, jboolean heap)
{
	log_println("jmm_GetMemoryUsage: IMPLEMENT ME!");
	return NULL;
}


jboolean jmm_GetBoolAttribute(JNIEnv* env, jmmBoolAttribute att)
{
	TRACEJMMCALLS(("jmm_GetBoolAttribute(env=%p, att=%d)", env, att));

	jboolean result;

	switch (att) {
	case JMM_VERBOSE_GC:
		result = opt_verbosegc;
		break;
	case JMM_VERBOSE_CLASS:
		result = opt_verboseclass;
		break;
	default:
		log_println("jmm_GetBoolAttribute: Unknown attribute %d", att);
		return false;
	}

	return result;
}


jboolean jmm_SetBoolAttribute(JNIEnv* env, jmmBoolAttribute att, jboolean flag)
{
	TRACEJMMCALLS(("jmm_SetBoolAttribute(env=%p, att=%d, flag=%d)", env, att, flag));

	switch (att) {
	case JMM_VERBOSE_GC:
		opt_verbosegc = flag;
		break;
	case JMM_VERBOSE_CLASS:
		opt_verboseclass = flag;
		break;
	default:
		log_println("jmm_SetBoolAttribute: Unknown attribute %d", att);
		return false;
	}

	return true;
}


jlong jmm_GetLongAttribute(JNIEnv* env, jobject obj, jmmLongAttribute att)
{
	TRACEJMMCALLS(("jmm_GetLongAttribute(env=%p, obj=%p, att=%d)", env, obj, att));

	jlong result;

	switch (att) {
	case JMM_CLASS_LOADED_COUNT:
		result = classcache_get_loaded_class_count();
		break;
	case JMM_CLASS_UNLOADED_COUNT:
		// XXX Fix this once we support class unloading!
		result = 0;
		break;
	case JMM_THREAD_TOTAL_COUNT:
		result = ThreadList::get()->get_number_of_started_java_threads();
		break;
	case JMM_THREAD_LIVE_COUNT:
		result = ThreadList::get()->get_number_of_active_java_threads();
		break;
	case JMM_THREAD_PEAK_COUNT:
		result = ThreadList::get()->get_peak_of_active_java_threads();
		break;
	case JMM_THREAD_DAEMON_COUNT:
		result = ThreadList::get()->get_number_of_daemon_java_threads();
		break;
	case JMM_JVM_INIT_DONE_TIME_MS:
		result = VM::get_current()->get_inittime();
		break;
	case JMM_OS_PROCESS_ID:
		result = os::getpid();
		break;
	default:
		log_println("jmm_GetLongAttribute: Unknown attribute %d", att);
		return -1;
	}

	return result;
}


jint jmm_GetLongAttributes(JNIEnv* env, jobject obj, jmmLongAttribute* atts, jint count, jlong* result)
{
	log_println("jmm_GetLongAttributes: IMPLEMENT ME!");
	return 0;
}


jint jmm_GetThreadInfo(JNIEnv* env, jlongArray ids, jint maxDepth, jobjectArray infoArray)
{
	log_println("jmm_GetThreadInfo: IMPLEMENT ME!");
	return 0;
}


jobjectArray jmm_DumpThreads(JNIEnv* env, jlongArray thread_ids, jboolean locked_monitors, jboolean locked_synchronizers)
{
	log_println("jmm_DumpThreads: IMPLEMENT ME!");
	return NULL;
}


jobjectArray jmm_GetLoadedClasses(JNIEnv* env)
{
	log_println("jmm_GetLoadedClasses: IMPLEMENT ME!");
	return NULL;
}


jboolean jmm_ResetStatistic(JNIEnv* env, jvalue obj, jmmStatisticType type)
{
	TRACEJMMCALLS(("jmm_ResetStatistic(env=%p, obj=%p, type=%d)", env, obj, type));

	switch (type) {
	case JMM_STAT_PEAK_THREAD_COUNT:
		ThreadList::get()->reset_peak_of_active_java_threads();
		break;
	default:
		log_println("jmm_ResetStatistic: Unknown statistic type %d", type);
		return false;
	}

	return true;
}


jlong jmm_GetThreadCpuTime(JNIEnv* env, jlong thread_id)
{
	log_println("jmm_GetThreadCpuTime: IMPLEMENT ME!");
	return 0;
}


jlong jmm_GetThreadCpuTimeWithKind(JNIEnv* env, jlong thread_id, jboolean user_sys_cpu_time)
{
	log_println("jmm_GetThreadCpuTimeWithKind: IMPLEMENT ME!");
	return 0;
}


jobjectArray jmm_GetVMGlobalNames(JNIEnv* env)
{
	log_println("jmm_GetVMGlobalNames: IMPLEMENT ME!");
	return NULL;
}


jint jmm_GetVMGlobals(JNIEnv* env, jobjectArray names, jmmVMGlobal* globals, jint count)
{
	log_println("jmm_GetVMGlobals: IMPLEMENT ME!");
	return 0;
}


void jmm_SetVMGlobal(JNIEnv* env, jstring flag_name, jvalue new_value)
{
	log_println("jmm_SetVMGlobal: IMPLEMENT ME!");
}


jint jmm_GetInternalThreadTimes(JNIEnv* env, jobjectArray names, jlongArray times)
{
	log_println("jmm_GetInternalThreadTimes: IMPLEMENT ME!");
	return 0;
}


jobjectArray jmm_FindDeadlockedThreads(JNIEnv* env, jboolean object_monitors_only)
{
	log_println("jmm_FindDeadlockedThreads: IMPLEMENT ME!");
	return NULL;
}


jobjectArray jmm_FindMonitorDeadlockedThreads(JNIEnv* env)
{
	log_println("jmm_FindMonitorDeadlockedThreads: IMPLEMENT ME!");
	return NULL;
}


jint jmm_GetGCExtAttributeInfo(JNIEnv* env, jobject mgr, jmmExtAttributeInfo* info, jint count)
{
	log_println("jmm_GetGCExtAttributeInfo: IMPLEMENT ME!");
	return 0;
}


void jmm_GetLastGCStat(JNIEnv* env, jobject obj, jmmGCStat* gc_stat)
{
	log_println("jmm_GetLastGCStat: IMPLEMENT ME!");
}


jint jmm_DumpHeap0(JNIEnv* env, jstring outputfile, jboolean live)
{
	log_println("jmm_DumpHeap0: IMPLEMENT ME!");
	return 0;
}

} // extern "C"


const struct jmmInterface_1_ jmm_interface = {
	NULL,
	NULL,
	jmm_GetVersion,
	jmm_GetOptionalSupport,
	jmm_GetInputArguments,
	jmm_GetThreadInfo,
	jmm_GetInputArgumentArray,
	jmm_GetMemoryPools,
	jmm_GetMemoryManagers,
	jmm_GetMemoryPoolUsage,
	jmm_GetPeakMemoryPoolUsage,
	NULL,
	jmm_GetMemoryUsage,
	jmm_GetLongAttribute,
	jmm_GetBoolAttribute,
	jmm_SetBoolAttribute,
	jmm_GetLongAttributes,
	jmm_FindMonitorDeadlockedThreads,
	jmm_GetThreadCpuTime,
	jmm_GetVMGlobalNames,
	jmm_GetVMGlobals,
	jmm_GetInternalThreadTimes,
	jmm_ResetStatistic,
	jmm_SetPoolSensor,
	jmm_SetPoolThreshold,
	jmm_GetPoolCollectionUsage,
	jmm_GetGCExtAttributeInfo,
	jmm_GetLastGCStat,
	jmm_GetThreadCpuTimeWithKind,
	NULL,
	jmm_DumpHeap0,
	jmm_FindDeadlockedThreads,
	jmm_SetVMGlobal,
	NULL,
	jmm_DumpThreads
};


/**
 * Return the requested management interface.
 *
 * @param version Requested management interface version.
 *
 * @return Pointer to management interface structure.
 */
void* Management::get_jmm_interface(int version)
{
	if (version == JMM_VERSION_1_0) {
		return (void*) &jmm_interface;
	}

	return NULL;
}

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
