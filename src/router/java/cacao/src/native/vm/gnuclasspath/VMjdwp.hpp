/* src/native/vm/VMjdwp.c - jvmti->jdwp interface

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

   Contact: cacao@cacaojvm.org

   Author: Martin Platter

   Changes:

*/

#ifndef VMJDWP_HPP_
#define VMJDWP_HPP_ 1

#include "native/jvmti/jvmti.h"

extern "C" {

extern jvmtiEnv            *jvmtienv;
extern jvmtiEventCallbacks  jvmti_jdwp_EventCallbacks;
extern char                *jdwpoptions;
extern bool                 suspend;     // should the virtual machine suspend on startup?
extern jthread              jdwpthread;

void printjvmtierror(char *desc, jvmtiError err);

} // extern "C"

#endif // VMJDWP_HPP_


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
