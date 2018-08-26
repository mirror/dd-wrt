/* Copyright (c) 2001-2011 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */

#include <stdio.h>
#include <slang.h>

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

SLANG_MODULE(fcntl);

static int check_and_set_errno (int e)
{
#ifdef EINTR
   if (e == EINTR)
     return 0;
#endif
   (void) SLerrno_set_errno (e);
   return -1;
}

static int do_fcntl_2 (int fd, int cmd)
{
   int ret;

   while ((-1 == (ret = fcntl (fd, cmd)))
	  && (0 == check_and_set_errno (errno)))
     ;

   return ret;
}

static int do_fcntl_3_int (int fd, int cmd, int flags)
{
   int ret;

   while ((-1 == (ret = fcntl (fd, cmd, flags)))
	  && (0 == check_and_set_errno (errno)))
     ;

   return ret;
}

static int pop_fd (int *fdp)
{
   SLFile_FD_Type *f;
   int status;

   if (SLang_peek_at_stack () == SLANG_INT_TYPE)
     return SLang_pop_int (fdp);

   if (-1 == SLfile_pop_fd (&f))
     return -1;

   status = SLfile_get_fd (f, fdp);
   SLfile_free_fd (f);
   return status;
}

static int fcntl_getfd (void)
{
   int fd;

   if (-1 == pop_fd (&fd))
     return -1;

   return do_fcntl_2 (fd, F_GETFD);
}

static int fcntl_setfd (int *flags)
{
   int fd;

   if (-1 == pop_fd (&fd))
     return -1;
   return do_fcntl_3_int (fd, F_SETFD, *flags);
}

static int fcntl_getfl (void)
{
   int fd;

   if (-1 == pop_fd (&fd))
     return -1;

   return do_fcntl_2 (fd, F_GETFL);
}

static int fcntl_setfl (int *flags)
{
   int fd;

   if (-1 == pop_fd (&fd))
     return -1;

   return do_fcntl_3_int (fd, F_SETFL, *flags);
}

#define F SLANG_FILE_FD_TYPE
#define I SLANG_INT_TYPE
static SLang_Intrin_Fun_Type Fcntl_Intrinsics [] =
{
   MAKE_INTRINSIC_0("fcntl_getfd", fcntl_getfd, I),
   MAKE_INTRINSIC_1("fcntl_setfd", fcntl_setfd, I, I),
   MAKE_INTRINSIC_0("fcntl_getfl", fcntl_getfl, I),
   MAKE_INTRINSIC_1("fcntl_setfl", fcntl_setfl, I, I),

   SLANG_END_INTRIN_FUN_TABLE
};
#undef I
#undef F

static SLang_IConstant_Type Fcntl_Consts [] =
{
   MAKE_ICONSTANT("FD_CLOEXEC", FD_CLOEXEC),
#ifndef O_ACCMODE
# define O_ACCMODE (O_RDONLY | O_WRONLY | O_RDWR)
#endif
   MAKE_ICONSTANT("O_ACCMODE", O_ACCMODE),
   SLANG_END_ICONST_TABLE
};

int init_fcntl_module_ns (char *ns_name)
{
   SLang_NameSpace_Type *ns;

   ns = SLns_create_namespace (ns_name);
   if (ns == NULL)
     return -1;

   if ((-1 == SLns_add_intrin_fun_table (ns, Fcntl_Intrinsics, "__FCNTL__"))
       || (-1 == SLns_add_iconstant_table (ns, Fcntl_Consts, NULL)))
     return -1;

   return 0;
}

/* This function is optional */
void deinit_fcntl_module (void)
{
}
