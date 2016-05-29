/*
 * Copyright (C) 2010, 2011, 2014 Robert Lougher <rob@jamvm.org.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <sys/types.h>
#include <unistd.h>

#include "jam.h"
#include "jni.h"
#include "jmm.h"
#include "trace.h"
#include "symbol.h"

jint jmm_GetVersion(JNIEnv *env) {
    return JMM_VERSION_1_0;
}

jint jmm_GetOptionalSupport(JNIEnv *env, jmmOptionalSupport *support) {
    TRACE("jmm_GetOptionalSupport(env=%p, support=%p)", env, support);

    if(support == NULL)
        return -1;

    support->isLowMemoryDetectionSupported = 1;
    support->isCompilationTimeMonitoringSupported = 1;
    support->isThreadContentionMonitoringSupported = 1;
    support->isCurrentThreadCpuTimeSupported = 0;
    support->isOtherThreadCpuTimeSupported = 0;
    support->isBootClassPathSupported = 1;
    support->isObjectMonitorUsageSupported = 1;
    support->isSynchronizerUsageSupported = 1;

    return 0;
}

jobject jmm_GetInputArguments(JNIEnv *env) {
    UNIMPLEMENTED("jmm_GetInputArguments");
    return NULL;
}

jobjectArray jmm_GetInputArgumentArray(JNIEnv *env) {
    UNIMPLEMENTED("jmm_GetInputArgumentArray");
    return NULL;
}

jobjectArray jmm_GetMemoryPools(JNIEnv *env, jobject obj) {
    Class *array_class = findArrayClass(SYMBOL(array_java_lang_String));

    if(array_class == NULL)
        return NULL;

     return allocArray(array_class, 0, sizeof(Object*));
}

jobjectArray jmm_GetMemoryManagers(JNIEnv *env, jobject obj) {
    Class *array_class = findArrayClass(SYMBOL(array_java_lang_String));

    if(array_class == NULL)
        return NULL;

     return allocArray(array_class, 0, sizeof(Object*));
}

jobject jmm_GetMemoryPoolUsage(JNIEnv *env, jobject obj) {
    UNIMPLEMENTED("jmm_GetMemoryPoolUsage");
    return NULL;
}

jobject jmm_GetPeakMemoryPoolUsage(JNIEnv *env, jobject obj) {
    UNIMPLEMENTED("jmm_GetPeakMemoryPoolUsage");
    return NULL;
}

jobject jmm_GetPoolCollectionUsage(JNIEnv *env, jobject obj) {
    UNIMPLEMENTED("jmm_GetPoolCollectionUsage");
    return NULL;
}

void jmm_SetPoolSensor(JNIEnv *env, jobject obj, jmmThresholdType type,
                       jobject sensorObj) {

    UNIMPLEMENTED("jmm_SetPoolSensor");
}

jlong jmm_SetPoolThreshold(JNIEnv *env, jobject obj, jmmThresholdType type,
                           jlong threshold) {

    UNIMPLEMENTED("jmm_SetPoolThreshold");
    return 0;
}

jobject jmm_GetMemoryUsage(JNIEnv *env, jboolean heap) {
    UNIMPLEMENTED("jmm_GetMemoryUsage");
    return NULL;
}

jboolean jmm_GetBoolAttribute(JNIEnv *env, jmmBoolAttribute att) {
    TRACE("jmm_GetBoolAttribute(env=%p, att=%d)", env, att);

    switch (att) {
        case JMM_VERBOSE_GC:
        case JMM_VERBOSE_CLASS:
        case JMM_THREAD_CONTENTION_MONITORING:
        case JMM_THREAD_CPU_TIME:
        case JMM_THREAD_ALLOCATED_MEMORY:
            break;

        default:
            UNIMPLEMENTED("jmm_GetBoolAttribute: Unknown attribute %d", att);
            break;
    }

    return FALSE;
}

jboolean jmm_SetBoolAttribute(JNIEnv *env, jmmBoolAttribute att,
                              jboolean flag) {

    TRACE("jmm_SetBoolAttribute(env=%p, att=%d, flag=%d)", env, att, flag);

    switch (att) {
        case JMM_VERBOSE_GC:
        case JMM_VERBOSE_CLASS:
        case JMM_THREAD_CONTENTION_MONITORING:
        case JMM_THREAD_CPU_TIME:
        case JMM_THREAD_ALLOCATED_MEMORY:
            break;

        default:
            UNIMPLEMENTED("jmm_SetBoolAttribute: Unknown attribute %d", att);
            break;
    }

    return FALSE;
}

jlong jmm_GetLongAttribute(JNIEnv *env, jobject obj, jmmLongAttribute att) {
    TRACE("jmm_GetLongAttribute(env=%p, obj=%p, att=%d)", env, obj, att);

    switch (att) {
        case JMM_OS_PROCESS_ID:
            return getpid();

        case JMM_JVM_INIT_DONE_TIME_MS:
        case JMM_CLASS_LOADED_COUNT:
        case JMM_CLASS_UNLOADED_COUNT:
        case JMM_THREAD_TOTAL_COUNT:
        case JMM_THREAD_LIVE_COUNT:
        case JMM_THREAD_PEAK_COUNT:
        case JMM_THREAD_DAEMON_COUNT:
            break;

        default:
            UNIMPLEMENTED("jmm_GetLongAttribute: Unknown attribute %d", att);
            break;
    }

    return 0;
}

jint jmm_GetLongAttributes(JNIEnv *env, jobject obj, jmmLongAttribute *tts,
                           jint count, jlong *result) {

    UNIMPLEMENTED("jmm_GetLongAttributes");
    return 0;
}

jint jmm_GetThreadInfo(JNIEnv *env, jlongArray ids, jint maxDepth,
                       jobjectArray infoArray) {

    UNIMPLEMENTED("jmm_GetThreadInfo");
    return 0;
}

jobjectArray jmm_DumpThreads(JNIEnv *env, jlongArray thread_ids,
                             jboolean locked_monitors,
                             jboolean locked_synchronizers) {

    UNIMPLEMENTED("jmm_DumpThreads");
    return NULL;
}

jobjectArray jmm_GetLoadedClasses(JNIEnv *env) {
    UNIMPLEMENTED("jmm_GetLoadedClasses");
    return NULL;
}

jboolean jmm_ResetStatistic(JNIEnv *env, jvalue obj, jmmStatisticType type) {
    TRACE("jmm_ResetStatistic(env=%p, obj=%p, type=%d)", env, obj, type);

    switch(type) {
        case JMM_STAT_PEAK_THREAD_COUNT:
            break;

        default:
            UNIMPLEMENTED("jmm_ResetStatistic: Unknown statistic type %d", type);
            break;
    }

    return FALSE;
}

jlong jmm_GetThreadCpuTime(JNIEnv *env, jlong thread_id) {
    UNIMPLEMENTED("jmm_GetThreadCpuTime");
    return 0;
}

jlong jmm_GetThreadCpuTimeWithKind(JNIEnv *env, jlong thread_id,
                                   jboolean user_sys_cpu_time) {

    UNIMPLEMENTED("jmm_GetThreadCpuTimeWithKind");
    return 0;
}

jobjectArray jmm_GetVMGlobalNames(JNIEnv *env) {
    UNIMPLEMENTED("jmm_GetVMGlobalNames");
    return NULL;
}

jint jmm_GetVMGlobals(JNIEnv *env, jobjectArray names, jmmVMGlobal *globals,
                      jint count) {

    UNIMPLEMENTED("jmm_GetVMGlobals");
    return 0;
}

void jmm_SetVMGlobal(JNIEnv *env, jstring flag_name, jvalue new_value) {
    UNIMPLEMENTED("jmm_SetVMGlobal");
}

jint jmm_GetInternalThreadTimes(JNIEnv *env, jobjectArray names,
                                jlongArray times) {

    UNIMPLEMENTED("jmm_GetInternalThreadTimes");
    return 0;
}

jobjectArray jmm_FindDeadlockedThreads(JNIEnv *env,
                                       jboolean object_monitors_only) {

    UNIMPLEMENTED("jmm_FindDeadlockedThreads");
    return NULL;
}

jobjectArray jmm_FindMonitorDeadlockedThreads(JNIEnv *env) {
    UNIMPLEMENTED("jmm_FindMonitorDeadlockedThreads");
    return NULL;
}

jint jmm_GetGCExtAttributeInfo(JNIEnv *env, jobject mgr,
                               jmmExtAttributeInfo *info, jint count) {

    UNIMPLEMENTED("jmm_GetGCExtAttributeInfo");
    return 0;
}

void jmm_GetLastGCStat(JNIEnv *env, jobject obj, jmmGCStat *gc_stat) {
    UNIMPLEMENTED("jmm_GetLastGCStat");
}

jint jmm_DumpHeap0(JNIEnv *env, jstring outputfile, jboolean live) {
    UNIMPLEMENTED("jmm_DumpHeap0");
    return 0;
}

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

void *getJMMInterface(int version) {
    if (version == JMM_VERSION_1_0)
        return (void*)&jmm_interface;

    return NULL;
}
