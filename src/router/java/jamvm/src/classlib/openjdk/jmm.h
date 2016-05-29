/*
 * Copyright (C) 2010, 2014 Robert Lougher <rob@jamvm.org.uk>.
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

#define JMM_VERSION_1_0 0x20010000

typedef enum {
    JMM_CLASS_LOADED_COUNT     = 1,
    JMM_CLASS_UNLOADED_COUNT   = 2,
    JMM_THREAD_TOTAL_COUNT     = 3,
    JMM_THREAD_LIVE_COUNT      = 4,
    JMM_THREAD_PEAK_COUNT      = 5,
    JMM_THREAD_DAEMON_COUNT    = 6,
    JMM_JVM_INIT_DONE_TIME_MS  = 7,
    JMM_OS_PROCESS_ID          = 201
} jmmLongAttribute;

typedef enum {
    JMM_VERBOSE_GC                   = 21,
    JMM_VERBOSE_CLASS                = 22,
    JMM_THREAD_CONTENTION_MONITORING = 23,
    JMM_THREAD_CPU_TIME              = 24,
    JMM_THREAD_ALLOCATED_MEMORY      = 25
} jmmBoolAttribute;

typedef enum {
    JMM_STAT_PEAK_THREAD_COUNT = 801
} jmmStatisticType;

typedef int jmmThresholdType;
typedef int jmmVMGlobalType;

typedef void* jmmExtAttributeInfo;
typedef void* jmmVMGlobal;
typedef void* jmmGCStat;

typedef struct {
    unsigned int isLowMemoryDetectionSupported : 1;
    unsigned int isCompilationTimeMonitoringSupported : 1;
    unsigned int isThreadContentionMonitoringSupported : 1;
    unsigned int isCurrentThreadCpuTimeSupported : 1;
    unsigned int isOtherThreadCpuTimeSupported : 1;
    unsigned int isBootClassPathSupported : 1;
    unsigned int isObjectMonitorUsageSupported : 1;
    unsigned int isSynchronizerUsageSupported : 1;
    unsigned int : 24;
} jmmOptionalSupport;

struct jmmInterface_1_ {
    void *reserved1;
    void *reserved2;
    jint (*GetVersion)(JNIEnv *env);
    jint (*GetOptionalSupport)(JNIEnv *env, jmmOptionalSupport *support_ptr);
    jobject (*GetInputArguments)(JNIEnv *env);
    jint (*GetThreadInfo)(JNIEnv *env, jlongArray ids, jint maxDepth, jobjectArray infoArray);
    jobjectArray (*GetInputArgumentArray)(JNIEnv *env);
    jobjectArray (*GetMemoryPools)(JNIEnv *env, jobject mgr);
    jobjectArray (*GetMemoryManagers)(JNIEnv *env, jobject pool);
    jobject (*GetMemoryPoolUsage)(JNIEnv *env, jobject pool);
    jobject (*GetPeakMemoryPoolUsage)(JNIEnv *env, jobject pool);
    void *reserved4;
    jobject (*GetMemoryUsage)(JNIEnv *env, jboolean heap);
    jlong (*GetLongAttribute)(JNIEnv *env, jobject obj, jmmLongAttribute att);
    jboolean (*GetBoolAttribute)(JNIEnv *env, jmmBoolAttribute att);
    jboolean (*SetBoolAttribute)(JNIEnv *env, jmmBoolAttribute att, jboolean flag);
    jint (*GetLongAttributes)(JNIEnv *env, jobject obj, jmmLongAttribute *atts, jint count, jlong *result);
    jobjectArray (*FindCircularBlockedThreads)(JNIEnv *env);
    jlong (*GetThreadCpuTime)(JNIEnv *env, jlong thread_id);
    jobjectArray (*GetVMGlobalNames)(JNIEnv *env);
    jint (*GetVMGlobals)(JNIEnv *env, jobjectArray names, jmmVMGlobal *globals, jint count);
    jint (*GetInternalThreadTimes)(JNIEnv *env, jobjectArray names, jlongArray times);
    jboolean (*ResetStatistic)(JNIEnv *env, jvalue obj, jmmStatisticType type);
    void (*SetPoolSensor)(JNIEnv *env, jobject pool, jmmThresholdType type, jobject sensor);
    jlong (*SetPoolThreshold)(JNIEnv *env, jobject pool, jmmThresholdType type, jlong threshold);
    jobject (*GetPoolCollectionUsage)(JNIEnv *env, jobject pool);
    jint (*GetGCExtAttributeInfo)(JNIEnv *env, jobject mgr, jmmExtAttributeInfo *ext_info, jint count);
    void (*GetLastGCStat)(JNIEnv *env, jobject mgr, jmmGCStat *gc_stat);
    jlong (*GetThreadCpuTimeWithKind)(JNIEnv *env, jlong thread_id, jboolean user_sys_cpu_time);
    void *reserved5;
    jint (*DumpHeap0)(JNIEnv *env, jstring outputfile, jboolean live);
    jobjectArray (*FindDeadlocks)(JNIEnv *env, jboolean object_monitors_only);
    void (*SetVMGlobal)(JNIEnv *env, jstring flag_name, jvalue new_value);
    void *reserved6;
    jobjectArray (*DumpThreads)(JNIEnv *env, jlongArray ids, jboolean lockedMonitors, jboolean lockedSynchronizers);
};

