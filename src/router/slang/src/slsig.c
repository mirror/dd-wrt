/* interpreter signal handling functions */
/*
Copyright (C) 2004-2011 John E. Davis

This file is part of the S-Lang Library.

The S-Lang Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The S-Lang Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
USA.
*/

#include "slinclud.h"

#include <signal.h>
#include <errno.h>

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

#include "slang.h"
#include "_slang.h"

#if SLANG_HAS_SIGNALS		       /* until end of file */

/* Do not trust these environments */
#if defined(__MINGW32__) || defined(AMIGA)
# ifdef SLANG_POSIX_SIGNALS
#  undef SLANG_POSIX_SIGNALS
# endif
#endif

typedef struct
{
   int sig;
   SLFUTURE_CONST char *name;
   SLang_Name_Type *handler;
   void (*c_handler)(int);
   int pending;			       /* if set, the signal needs to be delivered */
   int forbidden;		       /* If set, this signal is off-limits */
}
Signal_Type;

#define SIG_DFL_CONSTANT	0
#define SIG_IGN_CONSTANT	1
#define SIG_APP_CONSTANT	2

/* SIG_APP represents the application's handler */
#ifdef SLANG_POSIX_SIGNALS
typedef sigset_t Signal_Set_Type;
#else
typedef int Signal_Set_Type;
#endif

#define MAKE_SIGNAL(sig,name) {sig,name,NULL,NULL,0,0}
static Signal_Type Signal_Table [] =
{
#ifdef SIGHUP
     MAKE_SIGNAL(SIGHUP,"SIGHUP"),
#endif
#ifdef SIGINT
     MAKE_SIGNAL(SIGINT,"SIGINT"),
#endif
#ifdef SIGQUIT
     MAKE_SIGNAL(SIGQUIT,"SIGQUIT"),
#endif
#ifdef SIGILL
     MAKE_SIGNAL(SIGILL,"SIGILL"),
#endif
#ifdef SIGTRAP
     MAKE_SIGNAL(SIGTRAP,"SIGTRAP"),
#endif
#ifdef SIGABRT
     MAKE_SIGNAL(SIGABRT,"SIGABRT"),
#endif
#ifdef SIGIOT
     MAKE_SIGNAL(SIGIOT,"SIGIOT"),
#endif
#ifdef SIGBUS
     MAKE_SIGNAL(SIGBUS,"SIGBUS"),
#endif
#ifdef SIGFPE
     MAKE_SIGNAL(SIGFPE,"SIGFPE"),
#endif
#ifdef SIGKILL
     MAKE_SIGNAL(SIGKILL,"SIGKILL"),
#endif
#ifdef SIGUSR1
     MAKE_SIGNAL(SIGUSR1,"SIGUSR1"),
#endif
#ifdef SIGSEGV
     MAKE_SIGNAL(SIGSEGV,"SIGSEGV"),
#endif
#ifdef SIGUSR2
     MAKE_SIGNAL(SIGUSR2,"SIGUSR2"),
#endif
#ifdef SIGPIPE
     MAKE_SIGNAL(SIGPIPE,"SIGPIPE"),
#endif
#ifdef SIGALRM
     MAKE_SIGNAL(SIGALRM,"SIGALRM"),
#endif
#ifdef SIGTERM
     MAKE_SIGNAL(SIGTERM,"SIGTERM"),
#endif
#ifdef SIGSTKFLT
     MAKE_SIGNAL(SIGSTKFLT,"SIGSTKFLT"),
#endif
#ifdef SIGCHLD
     MAKE_SIGNAL(SIGCHLD,"SIGCHLD"),
#endif
#ifdef SIGCONT
     MAKE_SIGNAL(SIGCONT,"SIGCONT"),
#endif
#ifdef SIGSTOP
     MAKE_SIGNAL(SIGSTOP,"SIGSTOP"),
#endif
#ifdef SIGTSTP
     MAKE_SIGNAL(SIGTSTP,"SIGTSTP"),
#endif
#ifdef SIGTTIN
     MAKE_SIGNAL(SIGTTIN,"SIGTTIN"),
#endif
#ifdef SIGTTOU
     MAKE_SIGNAL(SIGTTOU,"SIGTTOU"),
#endif
#ifdef SIGURG
     MAKE_SIGNAL(SIGURG,"SIGURG"),
#endif
#ifdef SIGXCPU
     MAKE_SIGNAL(SIGXCPU,"SIGXCPU"),
#endif
#ifdef SIGXFSZ
     MAKE_SIGNAL(SIGXFSZ,"SIGXFSZ"),
#endif
#ifdef SIGVTALRM
     MAKE_SIGNAL(SIGVTALRM,"SIGVTALRM"),
#endif
#ifdef SIGPROF
     MAKE_SIGNAL(SIGPROF,"SIGPROF"),
#endif
#ifdef SIGWINCH
     MAKE_SIGNAL(SIGWINCH,"SIGWINCH"),
#endif
#ifdef SIGIO
     MAKE_SIGNAL(SIGIO,"SIGIO"),
#endif
#ifdef SIGPOLL
     MAKE_SIGNAL(SIGPOLL,"SIGPOLL"),
#endif
#ifdef SIGPWR
     MAKE_SIGNAL(SIGPWR,"SIGPWR"),
#endif
#ifdef SIGSYS
     MAKE_SIGNAL(SIGSYS,"SIGSYS"),
#endif
     MAKE_SIGNAL(-1, NULL),
};

static Signal_Type *find_signal (int sig)
{
   Signal_Type *s = Signal_Table;

   while (s->name != NULL)
     {
	if (s->sig == sig)
	  return s;
	s++;
     }
   return NULL;
}

#ifdef SLANG_POSIX_SIGNALS
static int do_sigprocmask (int how, sigset_t *new_mask, sigset_t *old_mask)
{
   while (-1 == sigprocmask (how, new_mask, old_mask))
     {
#ifdef EINTR
	if (errno == EINTR)
	  continue;
#endif
	return -1;
     }
   return 0;
}
#endif

static int block_signal (int sig, int *was_blocked)
{
#ifdef SLANG_POSIX_SIGNALS
   sigset_t new_mask;
   sigset_t old_mask;
   sigemptyset (&new_mask);
   sigaddset (&new_mask, sig);
   (void) do_sigprocmask (SIG_BLOCK, &new_mask, &old_mask);
   *was_blocked = sigismember (&old_mask, sig);
   return 0;
#else
   *was_blocked = 0;
   return 0;
#endif
}

static int unblock_signal (int sig)
{
#ifdef SLANG_POSIX_SIGNALS
   sigset_t new_mask;
   sigemptyset (&new_mask);
   sigaddset (&new_mask, sig);
   (void) do_sigprocmask (SIG_UNBLOCK, &new_mask, NULL);
#endif
   return 0;
}

static void signal_handler (int sig)
{
   Signal_Type *s;
   int e = errno;

   /* Until the signal has been delivered, block it. */
   /* Hmmm... it appears that the system unblocks the signal as soon
    * as this function exits.  Fortunately, this is not an issue.
    */
   /* (void) block_signal (sig); */
   (void) SLsignal_intr (sig, &signal_handler);

   s = find_signal (sig);	       /* cannot fail here */

   s->pending = 1;
   if (sig == SIGINT)
     SLKeyBoard_Quit = 1;
   _pSLang_signal_interrupt ();
   errno = e;
}

int _pSLsig_block_and_call (int (*func)(VOID_STAR), VOID_STAR cd)
{
   Signal_Type *s;
   int status;

#ifdef SLANG_POSIX_SIGNALS
   sigset_t new_mask, old_mask;
   sigemptyset (&new_mask);
#endif

   s = Signal_Table;
   while (s->name != NULL)
     {
	if (s->handler == NULL)
	  {
	     s++;
	     continue;
	  }
#ifdef SLANG_POSIX_SIGNALS
	sigaddset (&new_mask, s->sig);
#endif
	s++;
     }

#ifdef SLANG_POSIX_SIGNALS
   (void) do_sigprocmask (SIG_BLOCK, &new_mask, &old_mask);
#endif

   status = (*func) (cd);

#ifdef SLANG_POSIX_SIGNALS
   (void) do_sigprocmask (SIG_SETMASK, &old_mask, NULL);
#endif

   return status;
}

/* This function is reentrant */
static int handle_signal (Signal_Type *s)
{
   int status = 0;
   int was_blocked;

   (void) block_signal (s->sig, &was_blocked);

   /* At this point, sig is blocked and the handler is about to be called.
    * The pending flag can be safely set to 0 here.
    */
   s->pending = 0;

   if (s->handler != NULL)
     {
	int depth = SLstack_depth ();

	if ((-1 == SLang_start_arg_list ())
	    || (-1 == SLang_push_integer (s->sig))
	    || (-1 == SLang_end_arg_list ())
	    || (-1 == SLexecute_function (s->handler)))
	  status = -1;

	if ((status == 0)
	    && (depth != SLstack_depth ()))
	  {
	     SLang_verror (SL_Application_Error, "The signal handler %s corrupted the stack", s->handler->name);
	     status = -1;
	  }
     }

   if (was_blocked == 0)
     (void) unblock_signal (s->sig);

   return status;
}

/* This routine gets called by the interpreter when a signal needs to be
 * handled.
 */
int _pSLsig_handle_signals (void)
{
   Signal_Type *s = Signal_Table;
   int status = 0;

   while (s->name != NULL)
     {
	if ((s->pending != 0)
	    && (-1 == handle_signal (s)))
	  status = -1;

	s++;
     }
   return status;
}

static int pop_signal (Signal_Type **sp)
{
   int sig;
   Signal_Type *s;

   if (-1 == SLang_pop_int (&sig))
     return -1;

   s = Signal_Table;
   while (s->name != NULL)
     {
	if (s->sig == sig)
	  {
	     if (s->forbidden)
	       {
		  SLang_set_error (SL_Forbidden_Error);
		  return -1;
	       }

	     *sp = s;
	     return 0;
	  }
	s++;
     }

   _pSLang_verror (SL_INVALID_PARM, "Signal %d invalid or unknown", sig);
   return -1;
}

static int set_old_handler (Signal_Type *s, SLang_Ref_Type *ref, void (*old_handler)(int))
{
   if (old_handler == (void (*)(int))SIG_ERR)
     {
	_pSLang_verror (0, "signal system call failed");
	return -1;
     }

   if (ref != NULL)
     {
	int ret;

	if (old_handler == signal_handler)
	  ret = SLang_assign_nametype_to_ref (ref, s->handler);
	else
	  {
	     int h;

	     if (old_handler == SIG_IGN)
	       h = SIG_IGN_CONSTANT;
	     else if (old_handler == SIG_DFL)
	       h = SIG_DFL_CONSTANT;
	     else
	       h = SIG_APP_CONSTANT;

	     ret = SLang_assign_to_ref (ref, SLANG_INT_TYPE, &h);
	  }
	if (ret == -1)
	  {
	     (void) SLsignal_intr (s->sig, old_handler);
	     return -1;
	  }
     }

   if (old_handler != signal_handler)
     s->c_handler = old_handler;

   return 0;
}

static void signal_intrinsic (void)
{
   SLang_Name_Type *f;
   Signal_Type *s;
   void (*old_handler) (int);
   SLang_Ref_Type *old_ref;

   if (SLang_Num_Function_Args == 3)
     {
	if (-1 == SLang_pop_ref (&old_ref))
	  return;
     }
   else old_ref = NULL;

   if (SLang_Num_Function_Args == 0)
     {
	SLang_verror (SL_Internal_Error, "signal called with 0 args");
	return;
     }

   if (SLANG_INT_TYPE == SLang_peek_at_stack ())
     {
	int h;

	if ((-1 == SLang_pop_int (&h))
	    || (-1 == pop_signal (&s)))
	  {
	     SLang_free_ref (old_ref);
	     return;
	  }

	/* If this signal has already been caught, deliver it now to the old handler */
	if (s->pending)
	  handle_signal (s);
	/* Note that the signal has the potential of being lost if the user has
	 * blocked its delivery.  For this reason, the unblock_signal intrinsic
	 * will have to deliver the signal via an explicit kill if it is pending.
	 */

	if (h == SIG_IGN_CONSTANT)
	  old_handler = SLsignal_intr (s->sig, SIG_IGN);
	else if (h == SIG_DFL_CONSTANT)
	  old_handler = SLsignal_intr (s->sig, SIG_DFL);
	else if (h == SIG_APP_CONSTANT)
	  old_handler = SLsignal_intr (s->sig, s->c_handler);
	else
	  {
	     SLang_free_ref (old_ref);
	     _pSLang_verror (SL_INVALID_PARM, "Signal handler '%d' is invalid", h);
	     return;
	  }

	if (-1 == set_old_handler (s, old_ref, old_handler))
	  {
	     SLang_free_ref (old_ref);
	     return;
	  }

	if (s->handler != NULL)
	  {
	     SLang_free_function (s->handler);
	     s->handler = NULL;
	  }

	SLang_free_ref (old_ref);
	return;
     }

   if (NULL == (f = SLang_pop_function ()))
     {
	SLang_free_ref (old_ref);
	return;
     }

   if (-1 == pop_signal (&s))
     {
	SLang_free_ref (old_ref);
	SLang_free_function (f);
	return;
     }

   old_handler = SLsignal_intr (s->sig, signal_handler);
   if (-1 == set_old_handler (s, old_ref, old_handler))
     {
	SLang_free_ref (old_ref);
	SLang_free_function (f);
	return;
     }

   if (s->handler != NULL)
     SLang_free_function (s->handler);
   s->handler = f;
   SLang_free_ref (old_ref);
}

static void alarm_intrinsic (void)
{
#ifndef HAVE_ALARM
   SLang_set_error (SL_NotImplemented_Error);
#else
   SLang_Ref_Type *ref = NULL;
   unsigned int secs;
   Signal_Type *s;

   if (SLang_Num_Function_Args == 2)
     {
	if (-1 == SLang_pop_ref (&ref))
	  return;
     }

   if (-1 == SLang_pop_uint (&secs))
     {
	SLang_free_ref (ref);	       /* NULL ok */
	return;
     }
#ifdef SIGALRM
   if ((NULL != (s = find_signal (SIGALRM)))
       && s->forbidden)
     {
	SLang_set_error (SL_Forbidden_Error);
	return;
     }
#endif
   secs = alarm (secs);
   if (ref != NULL)
     (void) SLang_assign_to_ref (ref, SLANG_UINT_TYPE, &secs);
#endif
}

#ifdef SLANG_POSIX_SIGNALS
static int pop_signal_mask (sigset_t *maskp)
{
   SLang_Array_Type *at;
   unsigned int i, num, num_set;
   int *sigs;

   if (-1 == SLang_pop_array_of_type (&at, SLANG_INT_TYPE))
     return -1;

   sigemptyset (maskp);
   num = at->num_elements;
   sigs = (int *) at->data;
   num_set = 0;

   for (i = 0; i < num; i++)
     {
	int sig = sigs[i];
	if (NULL == find_signal (sig))
	  continue;

	sigaddset (maskp, sig);
	num_set++;
     }
   SLang_free_array (at);

   return 0;
}
#endif 				       /* SLANG_POSIX_SIGNALS */

static void sigsuspend_intrinsic (void)
{
#ifdef SLANG_POSIX_SIGNALS
   sigset_t mask;
#endif

   if (SLang_Num_Function_Args == 0)
     {
#ifdef HAVE_PAUSE
	(void) pause ();
#else
	SLang_set_error (SL_NotImplemented_Error);
#endif
	return;
     }

#ifndef SLANG_POSIX_SIGNALS
   SLang_set_error (SL_NotImplemented_Error);
#else
   if (-1 == pop_signal_mask (&mask))
     return;

   (void) sigsuspend (&mask);
#endif
}

#ifdef SLANG_POSIX_SIGNALS
static SLang_Array_Type *mask_to_array (sigset_t *mask)
{
   SLang_Array_Type *at;
   SLindex_Type num;
   Signal_Type *s;
   int *data;

   num = 0;
   s = Signal_Table;
   while (s->name != NULL)
     {
	if (sigismember (mask, s->sig))
	  num++;
	s++;
     }
   at = SLang_create_array (SLANG_INT_TYPE, 0, NULL, &num, 1);
   if (at == NULL)
     return NULL;

   s = Signal_Table;
   data = (int *)at->data;
   while (s->name != NULL)
     {
	if (sigismember (mask, s->sig))
	  *data++ = s->sig;
	s++;
     }

   return at;
}

static int assign_mask_to_ref (sigset_t *mask, SLang_Ref_Type *ref)
{
   SLang_Array_Type *at = mask_to_array (mask);

   if (at == NULL)
     return -1;

   if (-1 == SLang_assign_to_ref (ref, SLANG_ARRAY_TYPE, (VOID_STAR)&at))
     {
	SLang_free_array (at);
	return -1;
     }
   SLang_free_array (at);
   return 0;
}

static void sigprocmask_intrinsic (void)
{
   sigset_t mask, oldmask;
   SLang_Ref_Type *ref = NULL;
   int how;

   if (SLang_Num_Function_Args == 3)
     {
       if (-1 == SLang_pop_ref (&ref))
	  return;
     }

   if (-1 == pop_signal_mask (&mask))
     {
	SLang_free_ref (ref);
	return;
     }

   if (-1 == SLang_pop_int (&how))
     {
	SLang_free_ref (ref);
	return;
     }

   if ((how != SIG_BLOCK) && (how != SIG_UNBLOCK) && (how != SIG_SETMASK))
     {
	_pSLang_verror (SL_InvalidParm_Error, "sigprocmask: invalid operation");
	SLang_free_ref (ref);
	return;
     }

   do_sigprocmask (how, &mask, &oldmask);

   if (ref == NULL)
     return;

   if (-1 == assign_mask_to_ref (&oldmask, ref))
     do_sigprocmask (SIG_SETMASK, &oldmask, NULL);

   SLang_free_ref (ref);
}

#endif

static SLang_IConstant_Type IConsts [] =
{
#ifdef SLANG_POSIX_SIGNALS
   MAKE_ICONSTANT("SIG_BLOCK", SIG_BLOCK),
   MAKE_ICONSTANT("SIG_UNBLOCK", SIG_UNBLOCK),
   MAKE_ICONSTANT("SIG_SETMASK", SIG_SETMASK),
#endif
   MAKE_ICONSTANT("SIG_IGN", SIG_IGN_CONSTANT),
   MAKE_ICONSTANT("SIG_DFL", SIG_DFL_CONSTANT),
   MAKE_ICONSTANT("SIG_APP", SIG_APP_CONSTANT),
   SLANG_END_ICONST_TABLE
};

static SLang_Intrin_Fun_Type Intrin_Table [] =
{
   MAKE_INTRINSIC_0("signal", signal_intrinsic, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("alarm", alarm_intrinsic, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("sigsuspend", sigsuspend_intrinsic, SLANG_VOID_TYPE),
#ifdef SLANG_POSIX_SIGNALS
   MAKE_INTRINSIC_0("sigprocmask", sigprocmask_intrinsic, SLANG_VOID_TYPE),
#endif
   SLANG_END_INTRIN_FUN_TABLE
};

int SLang_init_signal (void)
{
   Signal_Type *s;

   if ((-1 == SLadd_intrin_fun_table(Intrin_Table, NULL))
       || (-1 == SLadd_iconstant_table (IConsts, NULL)))
     return -1;

   s = Signal_Table;
   while (s->name != NULL)
     {
	if (-1 == SLns_add_iconstant (NULL, s->name, SLANG_INT_TYPE, s->sig))
	  return -1;

	s++;
     }

   return 0;
}
#endif				       /* SLANG_HAS_SIGNALS */

int SLsig_forbid_signal (int sig)
{
#if SLANG_HAS_SIGNALS
   Signal_Type *s = find_signal (sig);
   if (s != NULL)
     s->forbidden = 1;
   return 0;
#else
   (void) sig;
   return 0;
#endif
}

