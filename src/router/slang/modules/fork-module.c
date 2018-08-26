/* -*- mode: C; mode: fold; -*- */
/*
Copyright (C) 2009-2011 John E. Davis

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

#include "config.h"

#include <stdio.h>
#include <errno.h>
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#include <unistd.h>

#include <string.h>
#include <slang.h>

SLANG_MODULE(fork);

static int fork_intrinsic (void)
{
   int ret = fork ();
   if (ret == -1)
     (void) SLerrno_set_errno (errno);
   return ret;
}

typedef struct
{
   int pid;
   int exited;
   int exit_status;
   int signal;
   int coredump;
   int stopped;
   int continued;
}
Waitpid_Type;

static SLang_CStruct_Field_Type Waitpid_Struct [] =
{
   MAKE_CSTRUCT_FIELD(Waitpid_Type, pid, "pid", SLANG_INT_TYPE, 0),
   MAKE_CSTRUCT_FIELD(Waitpid_Type, exited, "exited", SLANG_INT_TYPE, 0),
   MAKE_CSTRUCT_FIELD(Waitpid_Type, exit_status, "exit_status", SLANG_INT_TYPE, 0),
   MAKE_CSTRUCT_FIELD(Waitpid_Type, signal, "signal", SLANG_INT_TYPE, 0),
   MAKE_CSTRUCT_FIELD(Waitpid_Type, coredump, "coredump", SLANG_INT_TYPE, 0),
   MAKE_CSTRUCT_FIELD(Waitpid_Type, stopped, "stopped", SLANG_INT_TYPE, 0),
   MAKE_CSTRUCT_FIELD(Waitpid_Type, continued, "continued", SLANG_INT_TYPE, 0),
   SLANG_END_CSTRUCT_TABLE
};

static void waitpid_intrinsic (int *pid, int *options)
{
   int status, ret;
   Waitpid_Type s;

   while (-1 == (ret = waitpid ((pid_t)*pid, &status, *options)))
     {
	if (errno == EINTR)
	  {
	     if (-1 != SLang_handle_interrupt ())
	       continue;
	  }
	(void) SLerrno_set_errno (errno);
	(void) SLang_push_null ();
	return;
     }

   memset ((char *)&s, 0, sizeof(Waitpid_Type));
   if (WIFEXITED(status))
     {
	s.exited = 1;
	s.exit_status = WEXITSTATUS(status);
     }
   if (WIFSIGNALED(status))
     {
	s.signal = WTERMSIG(status);
#ifdef WCOREDUMP
	s.coredump = WCOREDUMP(status) != 0;
#endif
     }
   if (WIFSTOPPED(status))
     s.stopped = WSTOPSIG(status);
#ifdef WIFCONTINUED
   s.continued = WIFCONTINUED(status);
#endif
   s.pid = ret;
   (void) SLang_push_cstruct ((VOID_STAR)&s, Waitpid_Struct);
}

static char **pop_argv (SLang_Array_Type **atp)
{
   SLang_Array_Type *at;
   char **argv;
   unsigned int i, num, argc;
   char **strp;

   *atp = NULL;

   if (-1 == SLang_pop_array_of_type (&at, SLANG_STRING_TYPE))
     return NULL;

   num = at->num_elements;
   if (NULL == (argv = (char **)SLmalloc ((num+1)*sizeof(char *))))
     {
	SLang_free_array (at);
	return NULL;
     }

   strp = (char **)at->data;
   argc = 0;
   for (i = 0; i < num; i++)
     {
	if (strp[i] != NULL)
	  argv[argc++] = strp[i];
     }
   argv[argc] = NULL;
   *atp = at;
   return argv;
}

#define CALL_EXECV	1
#define CALL_EXECVP	2
#define CALL_EXECVE	3

static int call_what (int what, char *path, char **argv, char **envp)
{
   while (1)
     {
	int ret;

	switch (what)
	  {
	   case CALL_EXECV:
	     ret = execv (path, argv);
	     break;

	   case CALL_EXECVP:
	     ret = execvp (path, argv);
	     break;

	   case CALL_EXECVE:
	     ret = execve (path, argv, envp);
	     break;
	  }

	if (ret == 0)
	  return 0;		       /* should never happen */

	SLerrno_set_errno (errno);
	if (errno == EINTR)
	  {
	     if (-1 != SLang_handle_interrupt ())
	       continue;
	  }
	break;
     }
   return -1;
}

static int exec_what (int what, int has_envp)
{
   SLang_Array_Type *at_argv = NULL;
   SLang_Array_Type *at_envp = NULL;
   char **argv = NULL, **envp = NULL;
   char *path = NULL;
   int status = -1;

   if (has_envp)
     {
	if (NULL == (envp = pop_argv (&at_envp)))
	  goto free_and_return;
     }

   if (NULL == (argv = pop_argv (&at_argv)))
     goto free_and_return;

   if (-1 == SLang_pop_slstring (&path))
     goto free_and_return;

   status = call_what (what, path, argv, envp);

free_and_return:

   if (path != NULL) SLang_free_slstring (path);
   if (argv != NULL) SLfree ((char *)argv);
   if (at_argv != NULL) SLang_free_array (at_argv);
   if (envp != NULL) SLfree ((char *)envp);
   if (at_envp != NULL) SLang_free_array (at_envp);
   return status;
}

static int execv_intrin (void)
{
   if (SLang_Num_Function_Args != 2)
     SLang_verror (SL_Usage_Error, "Usage: ret = execv(path, argv[]);");

   return exec_what (CALL_EXECV, 0);
}

static int execvp_intrin (void)
{
   if (SLang_Num_Function_Args != 2)
     SLang_verror (SL_Usage_Error, "Usage: ret = execvp(path, argv[]);");

   return exec_what (CALL_EXECVP, 0);
}

static int execve_intrin (void)
{
   if (SLang_Num_Function_Args != 2)
     SLang_verror (SL_Usage_Error, "Usage: ret = execvp(path, argv[]);");

   return exec_what (CALL_EXECVE, 0);
}

static void _exit_intrin (int *s)
{
   (void) fflush (stdout);
   (void) fflush (stderr);
   _exit (*s);
}

static void pipe_intrin (void)
{
   int fds[2];
   SLFile_FD_Type *f0;
   SLFile_FD_Type *f1;

   while (-1 == pipe (fds))
     {
	if (errno == EINTR)
	  {
	     if (-1 != SLang_handle_interrupt ())
	       continue;
	  }
	SLerrno_set_errno (errno);
	SLang_verror (SL_OS_Error, "pipe failed: %s", SLerrno_strerror(errno));
	return;
     }

   f0 = SLfile_create_fd ("*pipe*", fds[0]);
   f1 = SLfile_create_fd ("*pipe*", fds[1]);
   if ((NULL != f0) && (NULL != f1))
     {
	/* Ignore errors and allow the free_fd routines to clean up */
	(void) SLfile_push_fd (f0);
	(void) SLfile_push_fd (f1);
     }
   SLfile_free_fd (f1);
   SLfile_free_fd (f0);
}

static SLang_IConstant_Type Module_IConstants [] =
{
   MAKE_ICONSTANT("WNOHANG", WNOHANG),
   MAKE_ICONSTANT("WUNTRACED", WUNTRACED),
   MAKE_ICONSTANT("WCONTINUED", WCONTINUED),
   SLANG_END_ICONST_TABLE
};

static SLang_Intrin_Fun_Type Module_Intrinsics [] =
{
   MAKE_INTRINSIC_0("fork", fork_intrinsic, SLANG_INT_TYPE),
   MAKE_INTRINSIC_2("waitpid", waitpid_intrinsic, SLANG_VOID_TYPE, SLANG_INT_TYPE, SLANG_INT_TYPE),
   MAKE_INTRINSIC_0("execv", execv_intrin, SLANG_INT_TYPE),
   MAKE_INTRINSIC_0("execvp", execvp_intrin, SLANG_INT_TYPE),
   MAKE_INTRINSIC_0("execve", execve_intrin, SLANG_INT_TYPE),
   MAKE_INTRINSIC_0("pipe", pipe_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_1("_exit", _exit_intrin, SLANG_VOID_TYPE, SLANG_INT_TYPE),
   SLANG_END_INTRIN_FUN_TABLE
};

int init_fork_module_ns (char *ns_name)
{
   SLang_NameSpace_Type *ns = SLns_create_namespace (ns_name);
   if (ns == NULL)
     return -1;

   if ((-1 == SLns_add_intrin_fun_table (ns, Module_Intrinsics, NULL))
       || (-1 == SLns_add_iconstant_table (ns, Module_IConstants, NULL)))
     return -1;

   return 0;
}

/* This function is optional */
void deinit_fork_module (void)
{
}
