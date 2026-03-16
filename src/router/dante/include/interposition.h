/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2004, 2008, 2009, 2010, 2011,
 *               2013, 2016
 *      Inferno Nettverk A/S, Norway.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. The above copyright notice, this list of conditions and the following
 *    disclaimer must appear in all copies of the software, derivative works
 *    or modified versions, and any portions thereof, aswell as in all
 *    supporting documentation.
 * 2. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by
 *      Inferno Nettverk A/S, Norway.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Inferno Nettverk A/S requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  sdc@inet.no
 *  Inferno Nettverk A/S
 *  Oslo Research Park
 *  Gaustadalléen 21
 *  NO-0349 Oslo
 *  Norway
 *
 * any improvements or extensions that they make and grant Inferno Nettverk A/S
 * the rights to redistribute these changes.
 *
 */

/* $Id: interposition.h,v 1.86.6.6.8.1 2024/11/21 10:22:41 michaels Exp $ */

#ifndef _INTERPOSITION_H_
#define _INTERPOSITION_H_

#include "symbols.h"

typedef enum { pid = 0, thread } which_id_t;
typedef struct socks_id_t {
   which_id_t        whichid;
   union {
      pid_t          pid;
#if HAVE_PTHREAD_H
      pthread_t      thread;
#endif /* HAVE_PTHREAD_H */
   } id;

   struct socks_id_t *next;
} socks_id_t;

typedef struct {
   const char  *symbol;         /* name of the symbol.         */
   const char  *library;        /* library symbol is in.       */
   void        *handle;         /* handle to the library.      */
   void        *function;       /* the bound symbol.           */

   socks_id_t  *dosyscall; /*
                            * if this value is not set, the corresponding
                            * syscall should be used for the given id.
                            * This is for cases where we are unable to
                            * base the decision concerning whether the
                            * function should resolve to a R*() function
                            * or a syscall in other ways.
                            */

} libsymbol_t;

#if SOCKSLIBRARY_DYNAMIC

int
socks_shouldcallasnative(const char *functionname);
/*
 * If calls to the function with the name "functionname" should at the
 * moment, for the calling thread/process, always resolve to the
 * corresponding system call/native function, return true.
 * Otherwise, return false.
 */


#else

#define socks_shouldcallasnative(functioname) (0)

#endif /* !SOCKSLIBRARY_DYNAMIC */

void socks_mark_io_as_native(void);
void socks_mark_io_as_normal(void);
/*
 * Marks i/o calls as native or normal,
 * using the socks_markas{native,normal}() functions.
 */


#if SOCKS_CLIENT

void socks_syscall_start(const int s);
/*
 * Marks that functions involving the descriptor "s" should resolve
 * to system calls.
 */

void socks_syscall_end(const int s);
/*
 * Removes the marking that functions involving the descriptor "s" should
 * resolve to system calls.
 */

int
socks_issyscall(const int s, const char *name);
/*
 * Checks whether the function with the name "name" should resolve
 * to a system call when used with the file descriptor "s".
 *
 * Returns true if so, false otherwise.
 */

socks_id_t *
socks_whoami(socks_id_t *id);
/*
 * Returns a unique id identifying the calling thread or process,
 * depending on whether the process is threaded or not.
 * The id is stored in the object "id".
 * Returns "id".
 */

#else /* !SOCKS_CLIENT */

#define socks_syscall_start(s)
#define socks_syscall_end(s)

#define socks_whoami(_id)                                                      \
do {                                                                           \
   (_id)->whichid = pid;                                                       \
   (_id)->id.pid  = sockscf.state.pid;                                         \
   (_id)->next    = NULL;                                                      \
} while (/* CONSTCOND */ 0)

#endif /* !SOCKSLIBRARY_DYNAMIC  || !SOCKS_CLIENT  */

void
socks_markasnative(const char *functionname);
/*
 * Marks the function "functionname" as a function that should
 * always resolve to the native system call for the calling thread,
 * process if not threaded, regardless of anything else.
 */

void
socks_markasnormal(const char *functionname);
/*
 * Removes the "mark as native" marker set by socks_markasnative(),
 * meaning the usual semantics will again be used to determine whether
 * the native system call or the corresponding R*() function should be
 * used when resolving "functionname".
 */

void *
symbolfunction(const char *symbol);
/*
 * Returns the address binding of the symbol "symbol" and updates
 * libsymbol_t structure "symbol" is defined in if necessary.
 * Exits on failure.
 */

void
symbolcheck(void);
/*
 * Checks that all defined symbols are loadable (and loads them).
 * Note that this might open file descriptors (and keep them open).
 */

#endif /* !_INTERPOSITION_H_ */
