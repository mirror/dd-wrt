/* src/native/vm/gnu/gnu_classpath_jdwp_VMMethod.c - jdwp->jvmti interface

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


#include "config.h"

#include <stdint.h>

#include "native/jni.hpp"

#include "native/include/gnu_classpath_jdwp_VMMethod.h"

#include "native/jvmti/jvmti.h"
#include "native/jvmti/VMjdwp.h"


void printjvmtierror(char *desc, jvmtiError err) {
    char* errdesc;

	if (err == JVMTI_ERROR_NONE) return;
	(*jvmtienv)->GetErrorName(jvmtienv,err, &errdesc);
	fprintf(stderr,"%s: jvmti error %s\n",desc, errdesc);
	fflush(stderr);
	(*jvmtienv)->Deallocate(jvmtienv,(unsigned char*)errdesc);
}



/*
 * Class:     gnu/classpath/jdwp/VMMethod
 * Method:    getName
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT struct java_lang_String* JNICALL Java_gnu_classpath_jdwp_VMMethod_getName(JNIEnv *env, gnu_classpath_jdwp_VMMethod* this)
{
    jvmtiError err;
    char *name;
    jstring stringname;

    if (JVMTI_ERROR_NONE != (err= (*jvmtienv)->
                             GetMethodName(jvmtienv,
										   (jmethodID)(long)this->_methodId,
                                           &name,NULL, NULL))) {
		printjvmtierror("VMMethod.getName GetMethodName",err);
        return NULL;
    }

    stringname = (*env)->NewStringUTF(env,name);
    (*jvmtienv)->Deallocate(jvmtienv,(unsigned char*)name);

    return stringname;
}


/*
 * Class:     gnu/classpath/jdwp/VMMethod
 * Method:    getSignature
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT struct java_lang_String* JNICALL Java_gnu_classpath_jdwp_VMMethod_getSignature(JNIEnv *env, gnu_classpath_jdwp_VMMethod* this)
{
    jvmtiError err;
    char *signature;
    jstring stringsignature;

    if (JVMTI_ERROR_NONE != (err= (*jvmtienv)->
                             GetMethodName(jvmtienv,
										   (jmethodID)(long)this->_methodId,
                                           NULL, &signature, NULL))) {
		printjvmtierror("VMMethod.getSignature GetMethodName",err);
        return NULL;
    }

    stringsignature = (*env)->NewStringUTF(env,signature);
    (*jvmtienv)->Deallocate(jvmtienv,(unsigned char*)signature);

    return stringsignature;
}


/*
 * Class:     gnu/classpath/jdwp/VMMethod
 * Method:    getModifiers
 * Signature: ()I
 */
JNIEXPORT int32_t JNICALL Java_gnu_classpath_jdwp_VMMethod_getModifiers(JNIEnv *env, gnu_classpath_jdwp_VMMethod* this)
{
    jvmtiError err;
    jint modifiers;

    if (JVMTI_ERROR_NONE!=(err= (*jvmtienv)->
						   GetMethodModifiers(jvmtienv,
											  (jmethodID)(long)this->_methodId,
											  &modifiers))) {
		printjvmtierror("VMMethod.getModifiers GetMethodModifiers",err);
        return 0;
    }

    return modifiers;
}


/*
 * Class:     gnu/classpath/jdwp/VMMethod
 * Method:    getLineTable
 * Signature: ()Lgnu/classpath/jdwp/util/LineTable;
 */
JNIEXPORT struct gnu_classpath_jdwp_util_LineTable* JNICALL Java_gnu_classpath_jdwp_VMMethod_getLineTable(JNIEnv *env, struct gnu_classpath_jdwp_VMMethod* this)
{
    jclass cl;
    jmethodID m;
    jobject ol;
    jlongArray jlineCI;
    jintArray jlineNum;
    jint count = 0, i;
    int *lineNum;
    long *lineCI;
    jvmtiLineNumberEntry *lne;
    jlocation start,end;

    jvmtiError err;

    if (JVMTI_ERROR_NONE!=(err= (*jvmtienv)->
						   GetLineNumberTable(jvmtienv,
											  (jmethodID)(long)this->_methodId,
											  &count, &lne))) {
		printjvmtierror("VMMethod.getlinetable GetLineNumberTable",err);
        return NULL;
    }

    cl = (*env)->FindClass(env,"gnu.classpath.jdwp.util.LineTable");
    if (!cl) return NULL;

    m = (*env)->GetMethodID(env, cl, "<init>", "(JJ[I[J)V");
    if (!m) return NULL;

    jlineNum = (*env)->NewIntArray(env, count);
    if (!jlineNum) return NULL;
    jlineCI = (*env)->NewLongArray(env, count);
    if (!jlineCI) return NULL;
    lineNum = (*env)->GetIntArrayElements(env, jlineNum, NULL);
    lineCI = (*env)->GetLongArrayElements(env, jlineCI, NULL);
    for (i = 0; i < count; ++i) {
        lineNum[i] = lne[i].line_number;
        lineCI[i] = lne[i].start_location;
    }
    (*env)->ReleaseLongArrayElements(env, jlineCI, lineCI, 0);
    (*env)->ReleaseIntArrayElements(env, jlineNum, lineNum, 0);
    (*jvmtienv)->Deallocate(jvmtienv, lne);

    if (JVMTI_ERROR_NONE!=(err= (*jvmtienv)->
						   GetMethodLocation(jvmtienv,
											 (jmethodID)(long)this->_methodId,
											 &start, &end))) {
		printjvmtierror("VMMethod.getlinetable GetMethodLocation",err);
        return NULL;
    }

    ol = (*env)->NewObject(env, cl, m, start,
                           end, jlineNum, jlineCI);

    return (struct gnu_classpath_jdwp_util_LineTable*)ol;

}

static bool fillVariableTable(JNIEnv *env, jvmtiLocalVariableEntry* entries,
							  int count, jlongArray *jlineCI,
							  jobjectArray *names, jobjectArray *sigs,
							  jintArray *jlengths, jintArray *jslot) {
	jint *lengths, *slot,i;
	jclass cl;
	jlong *lineCI;

	*jlineCI = (*env)->NewLongArray(env, count);
	if (!*jlineCI) return false;

	cl=(*env)->FindClass(env,"java/lang/String");
	if (!cl) return false;

	*names = (*env)->NewObjectArray(env, count, cl, NULL);
	if (names) return false;
	sigs = (*env)->NewObjectArray(env, count, cl, NULL);
	if (sigs) return false;

	jlengths = (*env)->NewIntArray(env, count);
	if (!lengths) return false;

	jslot = (*env)->NewIntArray(env, count);
	if (!slot) return false;

	lineCI = (*env)->GetLongArrayElements(env, *jlineCI, NULL);
	lengths = (*env)->GetIntArrayElements(env, *jlengths, NULL);
	slot = (*env)->GetIntArrayElements(env, jslot, NULL);

	for (i=0; i<count; i++) {
		(*env)->
			SetObjectArrayElement(env, *names, i,
								  (*env)->NewStringUTF(env,entries[i].name));
		(*env)->
			SetObjectArrayElement(env, *sigs, i, (*env)->NewStringUTF(
									  env,entries[i].signature));
		lineCI[i]=entries[i].start_location;
		lengths[i]=entries[i].length;
		slot[i]=entries[i].slot;
	}
    (*env)->ReleaseLongArrayElements(env, jlineCI, lineCI, 0);
    (*env)->ReleaseIntArrayElements(env, jlengths, lengths, 0);
    (*env)->ReleaseIntArrayElements(env, jslot, slot, 0);
	return true;
}


/*
 * Class:     gnu/classpath/jdwp/VMMethod
 * Method:    getVariableTable
 * Signature: ()Lgnu/classpath/jdwp/util/VariableTable;
 */
JNIEXPORT struct gnu_classpath_jdwp_util_VariableTable* JNICALL Java_gnu_classpath_jdwp_VMMethod_getVariableTable(JNIEnv *env, struct gnu_classpath_jdwp_VMMethod* this)
{
	jvmtiLocalVariableEntry* localvarentries;
	jint entry_count, argCnt, slots;
	jclass cl;
	jmethodID m, vmmethodid;
    jobject o;
	jobjectArray names, sigs;
    jvmtiError err;
    jlongArray jlineCI;
	jintArray jlengths, jslot;

	vmmethodid = (jmethodID)(long)this->_methodId;

	err= (*jvmtienv)->GetLocalVariableTable(jvmtienv,
											vmmethodid,
											&entry_count,
											&localvarentries);
    if (JVMTI_ERROR_NONE != err) {
		if (err == JVMTI_ERROR_ABSENT_INFORMATION) {
			/* no local variable table available for this method.
			   return an empty local variable table */
			argCnt = slots = 0;
			names = sigs = jlineCI = jlengths = jslot = NULL;
		} else {
			printjvmtierror("VMMethod.getVariableTable GetLocalVariableTable",err);
			return NULL;
		}
	} else {
		if (JVMTI_ERROR_NONE != (err=
								 (*jvmtienv)->GetArgumentsSize(jvmtienv,
															   vmmethodid,
															   &argCnt))) {
			printjvmtierror("VMMethod.getVariableTable GetArgumentsSize",err);
			return NULL;
		}

		if (JVMTI_ERROR_NONE != (err= (*jvmtienv)->GetMaxLocals(jvmtienv,
																vmmethodid,
																&slots))) {
			printjvmtierror("VMMethod.getVariableTable GetMaxLocals",err);
			return NULL;
		}

		slots = slots - argCnt;
		if (!fillVariableTable(env, localvarentries, entry_count, &jlineCI,
							   &names, &sigs, &jlengths, &jslot))
			return NULL;
		(*jvmtienv)->
			Deallocate(jvmtienv, (unsigned char*)localvarentries->signature);
		(*jvmtienv)->
			Deallocate(jvmtienv, (unsigned char*)localvarentries->name);
		if (localvarentries->generic_signature != NULL)
			(*jvmtienv)-> Deallocate(jvmtienv, (unsigned char*)
									 localvarentries->generic_signature);


		(*jvmtienv)->Deallocate(jvmtienv,(unsigned char*)localvarentries);
	}

    cl = (*env)->FindClass(env,"gnu.classpath.jdwp.util.VariableTable");
    if (!cl) return NULL;

    m = (*env)->
		GetMethodID(env, cl,"<init>",
					"(II[J[Ljava/lang/String;[Ljava/lang/String;[I[I)V");
    if (!m) return NULL;

    o = (*env)->NewObject(env, cl, m, argCnt, slots, jlineCI,
						  names, sigs, jlengths, jslot);

    return (struct gnu_classpath_jdwp_util_VariableTable*) o;
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
 */
