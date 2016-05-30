/* tests/regression/native/testarguments.c - tests argument passing

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

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <cstdio>
#include <cstdlib>
#include <inttypes.h>
#include <stdint.h>
#include "vm/types.hpp"

#include INCLUDE_JNI_MD_H
#include INCLUDE_JNI_H

static inline void print_jlong(jlong n) {
    printf(" 0x%" PRIx64, (s8) n);
}
static inline void print_jint (jint  n) {
    printf(" 0x%x", n);
}

static inline void print_jfloat(jfloat  f) {
    union {
        jfloat f;
        jint   i;
    } x;

    x.f = f;

    print_jint(x.i);
}
static inline void print_jdouble(jdouble d) {
    union {
        jdouble d;
        jlong   l;
    } x;

    x.d = d;

    print_jlong(x.l);
}

static inline void print_jobject(jobject o) {
#if defined(ENABLE_HANDLES)
    intptr_t p = *((intptr_t *) o);
#else
    intptr_t p = (intptr_t) o;
#endif

    printf(" 0x%" PRIxPTR, (uintptr_t) p);
}

extern "C" {

JNIEXPORT jobject JNICALL Java_testarguments_adr(JNIEnv *env, jclass clazz, jint i)
{
  intptr_t p;

  p = 0x11111111 * ((intptr_t) i);

#if defined(ENABLE_HANDLES)
  return env->NewLocalRef(&p);
#else
  return (jobject) p;
#endif
}


JNIEXPORT void JNICALL Java_testarguments_np(JNIEnv *env, jclass clazz, jobject o)
{
    print_jobject(o);

    fflush(stdout);
}


JNIEXPORT void JNICALL Java_testarguments_nisub(JNIEnv *env, jclass clazz, jint a, jint b, jint c, jint d, jint e, jint f, jint g, jint h, jint i, jint j, jint k, jint l, jint m, jint n, jint o)
{
    printf("java-native:");

    print_jint(a);
    print_jint(b);
    print_jint(c);
    print_jint(d);
    print_jint(e);
    print_jint(f);
    print_jint(g);
    print_jint(h);
    print_jint(i);
    print_jint(j);
    print_jint(k);
    print_jint(l);
    print_jint(m);
    print_jint(n);
    print_jint(o);

    printf("\n");
    fflush(stdout);

    jmethodID mid = env->GetStaticMethodID(clazz, "jisub", "(IIIIIIIIIIIIIII)V");

    if (mid == 0) {
        printf("native: couldn't find jisub\n");
        return;
    }

    env->CallStaticVoidMethod(clazz, mid, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
}


JNIEXPORT void JNICALL Java_testarguments_nlsub(JNIEnv *env, jclass clazz, jlong a, jlong b, jlong c, jlong d, jlong e, jlong f, jlong g, jlong h, jlong i, jlong j, jlong k, jlong l, jlong m, jlong n, jlong o)
{
    printf("java-native:");

    print_jlong(a);
    print_jlong(b);
    print_jlong(c);
    print_jlong(d);
    print_jlong(e);
    print_jlong(f);
    print_jlong(g);
    print_jlong(h);
    print_jlong(i);
    print_jlong(j);
    print_jlong(k);
    print_jlong(l);
    print_jlong(m);
    print_jlong(n);
    print_jlong(o);

    printf("\n");
    fflush(stdout);

    jmethodID mid = env->GetStaticMethodID(clazz, "jlsub", "(JJJJJJJJJJJJJJJ)V");

    if (mid == 0) {
        printf("native: couldn't find jlsub\n");
        return;
    }

    env->CallStaticVoidMethod(clazz, mid, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
}


JNIEXPORT void JNICALL Java_testarguments_nfsub(JNIEnv *env, jclass clazz, jfloat a, jfloat b, jfloat c, jfloat d, jfloat e, jfloat f, jfloat g, jfloat h, jfloat i, jfloat j, jfloat k, jfloat l, jfloat m, jfloat n, jfloat o)
{
    printf("java-native:");

    print_jfloat(a);
    print_jfloat(b);
    print_jfloat(c);
    print_jfloat(d);
    print_jfloat(e);
    print_jfloat(f);
    print_jfloat(g);
    print_jfloat(h);
    print_jfloat(i);
    print_jfloat(j);
    print_jfloat(k);
    print_jfloat(l);
    print_jfloat(m);
    print_jfloat(n);
    print_jfloat(o);

    printf("\n");
    fflush(stdout);

    jmethodID mid = env->GetStaticMethodID(clazz, "jfsub", "(FFFFFFFFFFFFFFF)V");

    if (mid == 0) {
        printf("native: couldn't find jfsub\n");
        return;
    }

    env->CallStaticVoidMethod(clazz, mid, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
}


JNIEXPORT void JNICALL Java_testarguments_ndsub(JNIEnv *env, jclass clazz, jdouble a, jdouble b, jdouble c, jdouble d, jdouble e, jdouble f, jdouble g, jdouble h, jdouble i, jdouble j, jdouble k, jdouble l, jdouble m, jdouble n, jdouble o)
{
    printf("java-native:");

    print_jdouble(a);
    print_jdouble(b);
    print_jdouble(c);
    print_jdouble(d);
    print_jdouble(e);
    print_jdouble(f);
    print_jdouble(g);
    print_jdouble(h);
    print_jdouble(i);
    print_jdouble(j);
    print_jdouble(k);
    print_jdouble(l);
    print_jdouble(m);
    print_jdouble(n);
    print_jdouble(o);

    printf("\n");
    fflush(stdout);

    jmethodID mid = env->GetStaticMethodID(clazz, "jdsub", "(DDDDDDDDDDDDDDD)V");

    if (mid == 0) {
        printf("native: couldn't find jdsub\n");
        return;
    }

    env->CallStaticVoidMethod(clazz, mid, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
}


JNIEXPORT void JNICALL Java_testarguments_nasub(JNIEnv *env, jclass clazz, jobject a, jobject b, jobject c, jobject d, jobject e, jobject f, jobject g, jobject h, jobject i, jobject j, jobject k, jobject l, jobject m, jobject n, jobject o)
{
    printf("java-native:");

    print_jobject(a);
    print_jobject(b);
    print_jobject(c);
    print_jobject(d);
    print_jobject(e);
    print_jobject(f);
    print_jobject(g);
    print_jobject(h);
    print_jobject(i);
    print_jobject(j);
    print_jobject(k);
    print_jobject(l);
    print_jobject(m);
    print_jobject(n);
    print_jobject(o);

    printf("\n");
    fflush(stdout);

    jmethodID mid = env->GetStaticMethodID(clazz, "jasub", "(Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;)V");

    if (mid == 0) {
        printf("native: couldn't find jasub\n");
        return;
    }

    env->CallStaticVoidMethod(clazz, mid, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
}


JNIEXPORT void JNICALL Java_testarguments_nmsub(JNIEnv *env, jclass clazz, jint a, jlong b, jfloat c, jdouble d, jint e, jlong f, jfloat g, jdouble h, jint i, jlong j, jfloat k, jdouble l, jint m, jlong n, jfloat o)
{
    printf("java-native:");

    print_jint(a);
    print_jlong(b);
    print_jfloat(c);
    print_jdouble(d);
    print_jint(e);
    print_jlong(f);
    print_jfloat(g);
    print_jdouble(h);
    print_jint(i);
    print_jlong(j);
    print_jfloat(k);
    print_jdouble(l);
    print_jint(m);
    print_jlong(n);
    print_jfloat(o);

    printf("\n");
    fflush(stdout);

    jmethodID mid = env->GetStaticMethodID(clazz, "jmsub", "(IJFDIJFDIJFDIJF)V");

    if (mid == 0) {
        printf("native: couldn't find jmsub\n");
        return;
    }

    env->CallStaticVoidMethod(clazz, mid, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
}

JNIEXPORT void JNICALL Java_testarguments_nmfsub(JNIEnv *env, jclass clazz, jdouble a, jfloat b, jdouble c, jfloat d, jdouble e, jfloat f, jdouble x_, jfloat y, jfloat g, jdouble h, jint i, jfloat j, jdouble k, jint l, jfloat m, jdouble n, jint o, jint p, jlong q, jfloat r, jdouble s, jint t, jlong u, jfloat v, jdouble w)
{
    printf("java-native:");

    print_jdouble(a);
    print_jfloat(b);
    print_jdouble(c);
    print_jfloat(d);
    print_jdouble(e);
    print_jfloat(f);
    print_jdouble(x_);
    print_jfloat(y);

    print_jfloat(g);
    print_jdouble(h);
    print_jint(i);

    print_jfloat(j);
    print_jdouble(k);
    print_jint(l);

    print_jfloat(m);
    print_jdouble(n);
    print_jint(o);

    print_jint(p);
    print_jlong(q);
    print_jfloat(r);
    print_jdouble(s);

    print_jint(t);
    print_jlong(u);
    print_jfloat(v);
    print_jdouble(w);

    printf("\n");
    fflush(stdout);

    jmethodID mid = env->GetStaticMethodID(clazz, "jmfsub", "(DFDFDFDFFDIFDIFDIIJFDIJFD)V");

    if (mid == 0) {
        printf("native: couldn't find jmfsub\n");
        return;
    }

    env->CallStaticVoidMethod(clazz, mid, a, b, c, d, e, f, x_, y, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w);
}

} // extern "C"

/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */
