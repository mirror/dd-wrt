/* Copyright (c) 2010-2011 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of the GNU General Public
 * License.
 */
#include <stdio.h>
#include <slang.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

SLANG_MODULE(select);

static int pop_fd_set (SLang_Array_Type **ats,
		       fd_set **fd_set_p, fd_set *fd_set_buf,
		       int *max_n)
{
   unsigned int num, i;
   SLang_Array_Type *at;
   SLFile_FD_Type **f;

   *ats = NULL;
   *fd_set_p = NULL;

   if (SLang_peek_at_stack () == SLANG_NULL_TYPE)
     return SLang_pop_null ();

   if (-1 == SLang_pop_array_of_type (&at, SLANG_FILE_FD_TYPE))
     return -1;

   FD_ZERO(fd_set_buf);
   *fd_set_p = fd_set_buf;

   *ats = at;
   num = at->num_elements;
   f = (SLFile_FD_Type **) at->data;

   for (i = 0; i < num; i++)
     {
	int fd;

	if (-1 == SLfile_get_fd (f[i], &fd))
	  continue;

	if (fd > *max_n)
	  *max_n = fd;

	FD_SET(fd, fd_set_buf);
     }

   return 0;
}

static SLang_Array_Type *do_fdisset (int nready, SLang_Array_Type *fds, fd_set *fdset)
{
   SLang_Array_Type *at;
   int i, num;
   SLFile_FD_Type **f;
   SLindex_Type ind_nready;

   if (fds == NULL)
     nready = 0;

   if (nready)
     {
	nready = 0;
	num = fds->num_elements;
	f = (SLFile_FD_Type **) fds->data;
	for (i = 0; i < num; i++)
	  {
	     int fd;

	     if (-1 == SLfile_get_fd (f[i], &fd))
	       continue;

	     if (FD_ISSET(fd, fdset))
	       nready++;
	  }
     }

   ind_nready = (SLindex_Type) nready;
   at = SLang_create_array (SLANG_INT_TYPE, 0, NULL, &ind_nready, 1);
   if (at == NULL)
     return NULL;

   if (nready)
     {
	int *indx = (int *) at->data;
	f = (SLFile_FD_Type **) fds->data;
	num = fds->num_elements;
	for (i = 0; i < num; i++)
	  {
	     int fd;

	     if (-1 == SLfile_get_fd (f[i], &fd))
	       continue;

	     if (FD_ISSET(fd, fdset))
	       *indx++ = (int) i;
	  }
     }

   return at;
}

static int push_select_struct (int num,
			       SLang_Array_Type *at_read,
			       SLang_Array_Type *at_write,
			       SLang_Array_Type *at_except,
			       fd_set *readfs, fd_set *writefds, fd_set *exceptfds)
{
   SLFUTURE_CONST char *field_names [4];
   SLtype field_types[4];
   VOID_STAR field_values [4];
   SLang_Array_Type *iread, *iwrite, *iexcept;
   int status;

   iread = iwrite = iexcept = NULL;

   field_names[0] = "nready";
   field_names[1] = "iread";
   field_names[2] = "iwrite";
   field_names[3] = "iexcept";
   field_types[0] = SLANG_INT_TYPE;
   field_types[1] = SLANG_ARRAY_TYPE;
   field_types[2] = SLANG_ARRAY_TYPE;
   field_types[3] = SLANG_ARRAY_TYPE;
   field_values[0] = &num;

   if ((NULL == (iread = do_fdisset (num, at_read, readfs)))
       || (NULL == (iwrite = do_fdisset (num, at_write, writefds)))
       || (NULL == (iexcept = do_fdisset (num, at_except, exceptfds))))
     {
	SLang_free_array (iread);
	SLang_free_array (iwrite);
	return -1;
     }

   field_values[1] = &iread;
   field_values[2] = &iwrite;
   field_values[3] = &iexcept;

   /* Note: This function call pushes the struct and frees it upon error. */
   status = SLstruct_create_struct (4, field_names, field_types, field_values);
   SLang_free_array (iexcept);
   SLang_free_array (iwrite);
   SLang_free_array (iread);
   return status;
}

/* Usage: Struct_Type select (R[],W[],E[],TIME) */

static void select_intrin (double *secsp)
{
   SLang_Array_Type *at_read, *at_write, *at_except;
   fd_set readfs_buf, writefds_buf, exceptfds_buf;
   fd_set readfs_save_buf, writefds_save_buf, exceptfds_save_buf;
   fd_set *readfs, *writefds, *exceptfds;
   struct timeval tv, *tv_ptr;
   double secs;
   int ret, n;

   secs = *secsp;
   if (secs < 0.0) tv_ptr = NULL;
   else
     {
	tv.tv_sec = (unsigned long) secs;
	tv.tv_usec = (unsigned long) ((secs - tv.tv_sec) * 1e6);
	tv_ptr = &tv;
     }

   n = 0;
   if (-1 == pop_fd_set (&at_except, &exceptfds, &exceptfds_buf, &n))
     return;
   if (-1 == pop_fd_set (&at_write, &writefds, &writefds_buf, &n))
     {
	SLang_free_array (at_except);
	return;
     }
   if (-1 == pop_fd_set (&at_read, &readfs, &readfs_buf, &n))
     goto free_return;

   readfs_save_buf = readfs_buf;
   writefds_save_buf = writefds_buf;
   exceptfds_save_buf = exceptfds_buf;

   n += 1;
   while (-1 == (ret = select (n, readfs, writefds, exceptfds, tv_ptr)))
     {
#ifdef EINTR
	if (errno == EINTR)
	  {
	     readfs_buf = readfs_save_buf;
	     writefds_buf = writefds_save_buf;
	     exceptfds_buf = exceptfds_save_buf;
	     if (0 == SLang_handle_interrupt ())
	       continue;
	  }
#endif
	(void) SLerrno_set_errno (errno);
	break;
     }

   if (ret == -1)
     (void) SLang_push_null ();
   else
     (void) push_select_struct (ret, at_read, at_write, at_except,
				readfs, writefds, exceptfds);

   free_return:
   SLang_free_array (at_read);
   SLang_free_array (at_write);
   SLang_free_array (at_except);
}

static SLang_Intrin_Fun_Type Select_Intrinsics [] =
{
   MAKE_INTRINSIC_1("select", select_intrin, SLANG_VOID_TYPE, SLANG_DOUBLE_TYPE),
   SLANG_END_INTRIN_FUN_TABLE
};

int init_select_module_ns (char *ns_name)
{
   SLang_NameSpace_Type *ns;

   ns = SLns_create_namespace (ns_name);
   if (ns == NULL)
     return -1;

   if (-1 == SLns_add_intrin_fun_table (ns, Select_Intrinsics, "__SELECT__"))
     return -1;

   return 0;
}

/* This function is optional */
void deinit_select_module (void)
{
}
