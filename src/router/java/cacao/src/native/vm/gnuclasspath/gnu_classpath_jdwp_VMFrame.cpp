/* src/native/vm/gnu/gnu_classpath_jdwp_VMFrame.c - jdwp->jvmti interface

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

#include "native/include/java_lang_Object.h"
#include "native/include/gnu_classpath_jdwp_VMFrame.h"

#include "toolbox/logging.hpp"


/*
 * Class:     gnu/classpath/jdwp/VMFrame
 * Method:    getValue
 * Signature: (I)Ljava/lang/Object;
 */
JNIEXPORT java_lang_Object* JNICALL Java_gnu_classpath_jdwp_VMFrame_getValue(JNIEnv *env, gnu_classpath_jdwp_VMFrame* this, int32_t par1)
{
	log_text ("JVMTI-Call: IMPLEMENT ME!!!");
	return 0;
}


/*
 * Class:     gnu/classpath/jdwp/VMFrame
 * Method:    setValue
 * Signature: (ILjava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_gnu_classpath_jdwp_VMFrame_setValue(JNIEnv *env, gnu_classpath_jdwp_VMFrame* this, int32_t par1, java_lang_Object* par2)
{
	log_text ("JVMTI-Call: IMPLEMENT ME!!!");
	return 0;
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
