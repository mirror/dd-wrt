#include <jni.h>
#include <stdio.h>

static jobject ref = NULL;
static jobject weak_ref = NULL;

JNIEXPORT void JNICALL Java_NativeGlobalRef_setReference(JNIEnv *env, jclass c, jobject o)
{
	printf("Native-World: setReference()\n");

	//ref = o;
	ref = (*env)->NewGlobalRef(env, o);

	return;
}

JNIEXPORT jobject JNICALL Java_NativeGlobalRef_getReference(JNIEnv *env, jclass c)
{
	printf("Native-World: getReference()\n");

	return ref;
}

JNIEXPORT void JNICALL Java_NativeGlobalRef_delReference(JNIEnv *env, jclass c)
{
	printf("Native-World: delReference()\n");

	(*env)->DeleteGlobalRef(env, ref);

	return;
}

JNIEXPORT void JNICALL Java_NativeWeakRef_setWeakReference(JNIEnv *env, jclass c, jobject o)
{
	printf("Native-World: setWeakReference()\n");

	weak_ref = (*env)->NewWeakGlobalRef(env, o);

	return;
}

JNIEXPORT jobject JNICALL Java_NativeWeakRef_getWeakReference(JNIEnv *env, jclass c)
{
	jobject local_ref;

	printf("Native-World: getWeakReference()\n");

	//local_ref = weak_ref;
	local_ref = (*env)->NewLocalRef(env, weak_ref);

	return local_ref;
}

JNIEXPORT void JNICALL Java_NativeWeakRef_delWeakReference(JNIEnv *env, jclass c)
{
	printf("Native-World: delWeakReference()\n");

	(*env)->DeleteWeakGlobalRef(env, weak_ref);

	return;
}
