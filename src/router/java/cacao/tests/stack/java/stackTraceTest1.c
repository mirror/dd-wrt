/*GPL license Joseph Wenninger 2004*/
#include "stackTraceTest1.h"

void JNICALL Java_stackTraceTest1_b_nested
  (JNIEnv *env, jclass clazz) {

        jmethodID mid;

        printf("Java_stackTraceTest1_b\n");

        mid = (*env)->GetStaticMethodID(env, clazz, "c", "()V");

        (*env)->CallStaticVoidMethod(env, clazz, mid);
}


JNIEXPORT void JNICALL Java_stackTraceTest1_b
  (JNIEnv *env, jclass clazz) {


	Java_stackTraceTest1_b_nested(env,clazz);

}

