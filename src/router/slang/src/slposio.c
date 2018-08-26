/* This module implements an interface to posix system calls */
/* file stdio intrinsics for S-Lang */
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

#if defined(__unix__) || (defined (__os2__) && defined (__EMX__))
# include <sys/types.h>
#endif

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#ifdef HAVE_SYS_FCNTL_H
# include <sys/fcntl.h>
#endif

#ifdef __unix__
# include <sys/file.h>
#endif

#ifdef HAVE_IO_H
# include <io.h>
#endif

#if defined(__BORLANDC__)
# include <dir.h>
#endif

#if defined(__DECC) && defined(VMS)
# include <unixio.h>
# include <unixlib.h>
#endif

#ifdef VMS
# include <stat.h>
#else
# include <sys/stat.h>
#endif

#include <errno.h>

#include "slang.h"
#include "_slang.h"

typedef struct _Stdio_MMT_List_Type
{
   SLang_MMT_Type *stdio_mmt;
   struct _Stdio_MMT_List_Type *next;
}
Stdio_MMT_List_Type;

struct _pSLFile_FD_Type
{
   char *name;
   unsigned int num_refs;	       /* reference counting */
   int fd;			       /* used if get_fd method is NULL */

   Stdio_MMT_List_Type *stdio_mmt_list;/* fdopen'd stdio objects */

   int is_closed;		       /* non-zero if closed */

#define _SLFD_NO_AUTO_CLOSE	1
   unsigned int flags;

   /* methods */
   int clientdata_id;
   VOID_STAR clientdata;
   void (*free_client_data)(VOID_STAR);

   int (*get_fd) (VOID_STAR, int *);
   /* If non-NULL, get_fd will be used to obtain the descriptor when needed */

   int (*close) (VOID_STAR);
   int (*read)(VOID_STAR, char *, unsigned int);
   int (*write)(VOID_STAR, char *, unsigned int);
   SLFile_FD_Type *(*dup)(VOID_STAR);

   SLFile_FD_Type *next;	       /* next in the list */
};

static int get_fd (SLFile_FD_Type *f, int *fdp)
{
   if (f->is_closed == 0)
     {
	if (f->get_fd == NULL)
	  {
	     *fdp = f->fd;
	     return 0;
	  }

	if (0 == (*f->get_fd)(f->clientdata, fdp))
	  return 0;
     }
   *fdp = -1;
#ifdef EBADF
   SLerrno_set_errno (EBADF);
#endif
   return -1;
}

static SLFile_FD_Type *FD_Type_List = NULL;

static void chain_fd_type (SLFile_FD_Type *f)
{
   f->next = FD_Type_List;
   FD_Type_List = f;
}

static void unchain_fdtype (SLFile_FD_Type *f)
{
   SLFile_FD_Type *prev;
   SLFile_FD_Type *curr;

   curr = FD_Type_List;
   if (curr == f)
     {
	FD_Type_List = f->next;
	return;
     }

   while (curr != NULL)
     {
	prev = curr;
	curr = curr->next;
	if (curr == f)
	  {
	     prev->next = f->next;
	     return;
	  }
     }
}

static SLFile_FD_Type *find_chained_fd (int fd)
{
   SLFile_FD_Type *f;

   f = FD_Type_List;
   while (f != NULL)
     {
	int fd1;

	if ((0 == get_fd (f, &fd1))
	    && (fd1 == fd))
	  return f;

	f = f->next;
     }

   return NULL;
}

/* This function gets called when the fclose intrinsic is called on an fdopen
 * derived object.
 */
void _pSLfclose_fdopen_fp (SLang_MMT_Type *mmt)
{
   SLFile_FD_Type *f;

   f = FD_Type_List;
   while (f != NULL)
     {
	Stdio_MMT_List_Type *prev, *curr;

	prev = NULL;
	curr = f->stdio_mmt_list;
	while (curr != NULL)
	  {
	     if (curr->stdio_mmt != mmt)
	       {
		  prev = curr;
		  curr = curr->next;
		  continue;
	       }

	     if (prev == NULL)
	       f->stdio_mmt_list = curr->next;
	     else
	       prev->next = curr->next;

	     SLang_free_mmt (mmt);
	     SLfree ((char *) curr);
	     return;
	  }
	f = f->next;
     }
}

static void free_stdio_mmts (SLFile_FD_Type *f)
{
   Stdio_MMT_List_Type *curr = f->stdio_mmt_list;

   while (curr != NULL)
     {
	Stdio_MMT_List_Type *next = curr->next;
	SLang_free_mmt (curr->stdio_mmt);
	SLfree ((char *) curr);
	curr = next;
     }
   f->stdio_mmt_list = NULL;
}

static Stdio_MMT_List_Type *alloc_stdio_list_elem (void)
{
   Stdio_MMT_List_Type *elem;

   elem = (Stdio_MMT_List_Type *) SLmalloc(sizeof(Stdio_MMT_List_Type));
   if (elem != NULL)
     memset ((char *)elem, 0, sizeof (Stdio_MMT_List_Type));
   return elem;
}

/* Returns 0 the system call should not be restarted, 1 otherwise */
static int is_interrupt (int e, int check_eagain)
{
   SLerrno_set_errno (e);

#ifdef EINTR
   if (e == EINTR)
     {
	if (0 == SLang_handle_interrupt ())
	  return 1;
     }
#endif
#ifdef EAGAIN
   if (e == EAGAIN)
     {
	if (check_eagain
	    && (0 == SLang_handle_interrupt ()))
	  return 1;
     }
#endif
   return 0;
}

static int do_close (SLFile_FD_Type *f)
{
   int fd;

   if (-1 == get_fd (f, &fd))
     return -1;

   while (1)
     {
	int status;

	errno = 0;
	if (f->close != NULL)
	  status = (*f->close)(f->clientdata);
	else
	  status = close (fd);

	if (status == 0)
	  {
	     f->fd = -1;
	     f->is_closed = 1;
	     if ((f->clientdata != NULL) && (f->free_client_data != NULL))
	       (*f->free_client_data) (f->clientdata);
	     f->clientdata = NULL;
	     return status;
	  }

	if (0 == is_interrupt (errno, 1))
	  return -1;
     }
}

static int do_write (SLFile_FD_Type *f, char *buf, unsigned int *nump)
{
   int fd;

   if (-1 == get_fd (f, &fd))
     {
	*nump = 0;
	return -1;
     }

   while (1)
     {
	int num;

	errno = 0;
	if (f->write != NULL)
	  num = (*f->write)(f->clientdata, buf, *nump);
	else
	  num = write (fd, buf, *nump);

	if (num != -1)
	  {
	     *nump = (unsigned int) num;
	     return 0;
	  }

	if (is_interrupt (errno, 0))
	  continue;

	*nump = 0;
	return -1;
     }
}

static int do_read (SLFile_FD_Type *f, char *buf, unsigned int *nump)
{
   int fd;

   if (-1 == get_fd (f, &fd))
     {
	*nump = 0;
	return -1;
     }

   while (1)
     {
	int num;

	errno = 0;
	if (f->read != NULL)
	  num = (*f->read)(f->clientdata, buf, *nump);
	else
	  num = read (fd, buf, *nump);

	if (num != -1)
	  {
	     *nump = (unsigned int) num;
	     return 0;
	  }

	if (is_interrupt (errno, 0))
	  continue;

	*nump = 0;
	return -1;
     }
}

static int posix_close_fd (int *fd)
{
   while (-1 == close (*fd))
     {
	if (0 == is_interrupt (errno, 1))
	  return -1;
     }

   return 0;
}

static int posix_close (SLFile_FD_Type *f)
{
   int status = do_close (f);

   free_stdio_mmts (f);
   return status;
}

/* Usage: Uint write (f, buf); */
static void posix_write (SLFile_FD_Type *f, SLang_BString_Type *bstr)
{
   unsigned int len;
   char *p;

   if ((NULL == (p = (char *)SLbstring_get_pointer (bstr, &len)))
       || (-1 == do_write (f, p, &len)))
     {
	SLang_push_integer (-1);
	return;
     }
   (void) SLang_push_uinteger (len);
}

/* Usage: nn = read (f, &buf, n); */
static void posix_read (SLFile_FD_Type *f, SLang_Ref_Type *ref, unsigned int *nbytes)
{
   unsigned int len;
   char *b;
   SLang_BString_Type *bstr;

   b = NULL;

   len = *nbytes;
   if ((NULL == (b = SLmalloc (len + 1)))
       || (-1 == do_read (f, b, &len)))
     goto return_error;

   if (len != *nbytes)
     {
	char *b1 = SLrealloc (b, len + 1);
	if (b1 == NULL)
	  goto return_error;
	b = b1;
     }

   bstr = SLbstring_create_malloced ((unsigned char *) b, len, 0);
   if (bstr != NULL)
     {
	if (-1 == SLang_assign_to_ref (ref, SLANG_BSTRING_TYPE, (VOID_STAR)&bstr))
	  {
	     SLbstring_free (bstr);
	     return;
	  }
	SLbstring_free (bstr);
	(void) SLang_push_uinteger (len);
	return;
     }

   return_error:
   if (b != NULL) SLfree ((char *)b);
   (void) SLang_assign_to_ref (ref, SLANG_NULL_TYPE, NULL);
   (void) SLang_push_integer (-1);
}

SLFile_FD_Type *SLfile_create_fd (SLFUTURE_CONST char *name, int fd)
{
   SLFile_FD_Type *f;

   if (name == NULL)
     name = "";

   if (NULL == (f = (SLFile_FD_Type *) SLmalloc (sizeof (SLFile_FD_Type))))
     return NULL;

   memset ((char *) f, 0, sizeof (SLFile_FD_Type));
   if (NULL == (f->name = SLang_create_slstring (name)))
     {
	SLfree ((char *)f);
	return NULL;
     }

   f->fd = fd;
   f->num_refs = 1;

   f->clientdata_id = 0;
   f->clientdata = NULL;
   /* If NULL, use the standard routines on a file descriptor */
   f->close = NULL;
   f->read = NULL;
   f->write = NULL;

   chain_fd_type (f);

   return f;
}

int SLfile_set_getfd_method (SLFile_FD_Type *f, int (*func)(VOID_STAR, int *))
{
   if (f == NULL)
     return -1;
   f->get_fd = func;
   return 0;
}

int Last_Client_Data_ID = 0;
int SLfile_create_clientdata_id (int *idp)
{
   if (Last_Client_Data_ID != -1)
     Last_Client_Data_ID++;

   if (Last_Client_Data_ID == -1)
     {
	*idp = -1;
	return -1;
     }
   *idp = Last_Client_Data_ID;
   return 0;
}

int SLfile_get_clientdata (SLFile_FD_Type *f, int id, VOID_STAR *cdp)
{
   if ((f == NULL)
       || (f->clientdata_id != id))
     {
	*cdp = NULL;
	return -1;
     }

   *cdp = f->clientdata;
   return 0;
}

int SLfile_set_clientdata (SLFile_FD_Type *f, void (*func)(VOID_STAR), VOID_STAR cd, int id)
{
   if (f == NULL)
     return -1;
   if (id == -1)
     {
	_pSLang_verror (SL_Application_Error, "SLfile_set_client_data: invalid id");
	return -1;
     }

   f->free_client_data = func;
   f->clientdata = cd;
   f->clientdata_id = id;
   return 0;
}

int SLfile_set_close_method (SLFile_FD_Type *f, int (*func)(VOID_STAR))
{
   if (f == NULL)
     return -1;
   f->close = func;
   return 0;
}

int SLfile_set_read_method (SLFile_FD_Type *f, int (*func)(VOID_STAR, char*, unsigned int))
{
   if (f == NULL)
     return -1;
   f->read = func;
   return 0;
}

int SLfile_set_write_method (SLFile_FD_Type *f, int (*func)(VOID_STAR, char*, unsigned int))
{
   if (f == NULL)
     return -1;
   f->write = func;
   return 0;
}

int SLfile_set_dup_method (SLFile_FD_Type *f, SLFile_FD_Type *(*func)(VOID_STAR))
{
   if (f == NULL)
     return -1;
   f->dup = func;
   return 0;
}

SLFile_FD_Type *SLfile_dup_fd (SLFile_FD_Type *f0)
{
   SLFile_FD_Type *f;
   int fd0, fd;

   if (f0 == NULL)
     return NULL;

   if (-1 == get_fd (f0, &fd0))
     return NULL;

   if (f0->dup != NULL)
     return (*f0->dup)(f0->clientdata);

   while (-1 == (fd = dup (fd0)))
     {
	if (is_interrupt (errno, 1))
	  continue;

	return NULL;
     }

   if (NULL == (f = SLfile_create_fd (f0->name, fd)))
     {
	while ((-1 == close (fd)) && is_interrupt (errno, 1))
	  ;
	return NULL;
     }

   return f;
}

/* Not yet a public function */
static int SLfile_dup2_fd (SLFile_FD_Type *f0, int newfd)
{
   int fd0, fd;

   if ((f0 == NULL)
       || (-1 == get_fd (f0, &fd0)))
     {
#ifdef EBADF
	SLerrno_set_errno (EBADF);
#endif
	return -1;
     }

   while (-1 == (fd = dup2 (fd0, newfd)))
     {
	if (is_interrupt (errno, 1))
	  continue;

	return -1;
     }
   return fd;
}

int SLfile_get_fd (SLFile_FD_Type *f, int *fd)
{
   if (f == NULL)
     return -1;

   return get_fd (f, fd);
}

void SLfile_free_fd (SLFile_FD_Type *f)
{
   if (f == NULL)
     return;

   if (f->num_refs > 1)
     {
	f->num_refs -= 1;
	return;
     }

   if (0 == (f->flags & _SLFD_NO_AUTO_CLOSE))
     (void) do_close (f);

   if ((f->clientdata != NULL)
       && (f->free_client_data != NULL))
     (*f->free_client_data) (f->clientdata);

   free_stdio_mmts (f);

   unchain_fdtype (f);

   SLfree ((char *) f);
}

static int pop_string_int (char **s, int *i)
{
   *s = NULL;
   if ((-1 == SLang_pop_integer (i))
       || (-1 == SLang_pop_slstring (s)))
     return -1;

   return 0;
}

static int pop_string_int_int (char **s, int *a, int *b)
{
   *s = NULL;
   if ((-1 == SLang_pop_integer (b))
       || (-1 == pop_string_int (s, a)))
     return -1;

   return 0;
}

static void posix_open (void)
{
   char *file;
   int mode, flags;
   SLFile_FD_Type *f;

   switch (SLang_Num_Function_Args)
     {
      case 3:
	if (-1 == pop_string_int_int (&file, &flags, &mode))
	  {
	     SLang_push_null ();
	     return;
	  }
	break;

      case 2:
      default:
	if (-1 == pop_string_int (&file, &flags))
	  return;
	mode = 0777;
	break;
     }

   f = SLfile_create_fd (file, -1);
   if (f == NULL)
     {
	SLang_free_slstring (file);
	SLang_push_null ();
	return;
     }
   SLang_free_slstring (file);

   while (-1 == (f->fd = open (f->name, flags, mode)))
     {
	if (is_interrupt (errno, 1))
	  continue;

	SLfile_free_fd (f);
	SLang_push_null ();
	return;
     }

   if (-1 == SLfile_push_fd (f))
     SLang_push_null ();
   SLfile_free_fd (f);
}

static int dummy_close (VOID_STAR cd)
{
   (void) cd;
   return 0;
}

static int posix_fileno_int (void)
{
   int fd;
   SLFile_FD_Type *f;

   if (SLang_peek_at_stack () == SLANG_FILE_PTR_TYPE)
     {
	SLang_MMT_Type *mmt;
	FILE *fp;

	if (-1 == SLang_pop_fileptr (&mmt, &fp))
	  return -1;

	fd = fileno (fp);
	SLang_free_mmt (mmt);
	return fd;
     }

   if (-1 == SLfile_pop_fd (&f))
     return -1;

   if (-1 == get_fd (f, &fd))
     fd = -1;

   SLfile_free_fd (f);
   return fd;
}

static void posix_fileno (void)
{
   FILE *fp;
   SLang_MMT_Type *mmt;
   int fd;
   SLFile_FD_Type *f;
   SLFUTURE_CONST char *name;

   if (-1 == SLang_pop_fileptr (&mmt, &fp))
     {
	SLang_push_null ();
	return;
     }
   name = SLang_get_name_from_fileptr (mmt);
   fd = fileno (fp);

   f = SLfile_create_fd (name, fd);
   if (f != NULL)
     {
	/* prevent fd from being closed  when it goes out of scope */
	f->flags |= _SLFD_NO_AUTO_CLOSE;
	f->close = dummy_close;
     }

   SLang_free_mmt (mmt);

   if (-1 == SLfile_push_fd (f))
     SLang_push_null ();
   SLfile_free_fd (f);
}

static void posix_fdopen (SLFile_FD_Type *f, char *mode)
{
   Stdio_MMT_List_Type *elem;

   if (NULL == (elem = alloc_stdio_list_elem ()))
     return;

   if (-1 == _pSLstdio_fdopen (f->name, f->fd, mode))
     {
	SLfree ((char *)elem);
	return;
     }

   if (NULL == (elem->stdio_mmt = SLang_pop_mmt (SLANG_FILE_PTR_TYPE)))
     {
	SLfree ((char *) elem);
	return;
     }

   if (-1 == SLang_push_mmt (elem->stdio_mmt))
     {
	SLfree ((char *) elem);
	return;
     }

   elem->next = f->stdio_mmt_list;
   f->stdio_mmt_list = elem;
}

static _pSLc_off_t_Type posix_lseek (SLFile_FD_Type *f, _pSLc_off_t_Type *ofs, int *whence)
{
   _pSLc_off_t_Type status;
   int fd;

   if (-1 == get_fd (f, &fd))
     return -1;

   while (-1 == (status = lseek (fd, *ofs, *whence)))
     {
	if (is_interrupt (errno, 1))
	  continue;
	return -1;
     }

   return status;
}

static int pop_fd (int *fdp, SLFile_FD_Type **fp, SLang_MMT_Type **mmtp)
{
   int fd;

   *fp = NULL; *mmtp = NULL;

   switch (SLang_peek_at_stack ())
     {
      case SLANG_FILE_PTR_TYPE:
	  {
	     SLang_MMT_Type *mmt;
	     FILE *p;

	     if (-1 == SLang_pop_fileptr (&mmt, &p))
	       return -1;
	     fd = fileno (p);
	     *mmtp = mmt;
	  }
	break;

      case SLANG_FILE_FD_TYPE:
	  {
	     SLFile_FD_Type *f;
	     if (-1 == SLfile_pop_fd (&f))
	       return -1;
	     if (-1 == get_fd (f, &fd))
	       {
		  SLfile_free_fd (f);
		  return -1;
	       }
	  }
	break;

      default:
	if (-1 == SLang_pop_int (&fd))
	  return -1;
     }
   *fdp = fd;
   return 0;
}

static int posix_isatty (void)
{
   int ret;
   SLFile_FD_Type *f;
   SLang_MMT_Type *mmt;
   int fd;

   if (-1 == pop_fd (&fd, &f, &mmt))
     return 0;		       /* invalid descriptor */

   if (0 == (ret = isatty (fd)))
     _pSLerrno_errno = errno;

   if (mmt != NULL) SLang_free_mmt (mmt);
   if (f != NULL) SLfile_free_fd (f);

   return ret;
}

#ifdef HAVE_TTYNAME_R
/* Older POSIX standards had a different interface for this.  Avoid it. */
# if !defined(_POSIX_C_SOURCE) || (_POSIX_C_SOURCE < 199506L)
#  undef HAVE_TTYNAME_R
# endif
#endif

#ifdef HAVE_TTYNAME_R
# define TTYNAME_R ttyname_r
#else
# ifdef HAVE_TTYNAME
#  define TTYNAME_R my_ttyname_r
static int my_ttyname_r (int fd, char *buf, size_t buflen)
{
   char *tty = ttyname (fd);
   if (tty == NULL)
     {
	int e = errno;
	if (e == 0)
	  e = -1;
	return e;
     }
   strncpy (buf, tty, buflen);
   buf[buflen-1] = 0;
   return 0;
}
# endif
#endif

#ifdef TTYNAME_R
static void posix_ttyname (void)
{
   SLFile_FD_Type *f;
   SLang_MMT_Type *mmt;
   int fd;
   char buf[512];
   int e;

   if (SLang_Num_Function_Args == 0)
     {
	fd = 0;
	f = NULL;
	mmt = NULL;
     }
   else if (-1 == pop_fd (&fd, &f, &mmt))
     return;

   if (0 != (e = TTYNAME_R (fd, buf, sizeof(buf))))
     {
	_pSLerrno_errno = e;
	SLang_push_null ();
     }
   else
     (void) SLang_push_string (buf);

   if (mmt != NULL) SLang_free_mmt (mmt);
   if (f != NULL) SLfile_free_fd (f);
}
#endif

static void posix_dup (SLFile_FD_Type *f)
{
   if ((NULL == (f = SLfile_dup_fd (f)))
       || (-1 == SLfile_push_fd (f)))
     SLang_push_null ();

   SLfile_free_fd (f);
}

static int posix_dup2 (SLFile_FD_Type *f, int *new_fd)
{
   return SLfile_dup2_fd (f, *new_fd);
}

static int fdtype_datatype_deref (SLtype type)
{
   SLFile_FD_Type *f;
   int status;
   int fd;

   (void) type;

   if (-1 == SLang_pop_int (&fd))
     return -1;
#ifdef F_GETFL
   while (-1 == fcntl (fd, F_GETFL))
     {
	if (is_interrupt (errno, 1))
	  continue;

	return SLang_push_null ();
     }
#endif
   f = find_chained_fd (fd);
   if (f != NULL)
     return SLfile_push_fd (f);

   /* The descriptor is valid, but we have no record of what it is.  So make sure
    * it is not automatically closed.
    */
   if (NULL == (f = SLfile_create_fd (NULL, fd)))
     return -1;
   f->flags |= _SLFD_NO_AUTO_CLOSE;

   status = SLfile_push_fd (f);
   SLfile_free_fd (f);
   return status;
}

#define I SLANG_INT_TYPE
#define V SLANG_VOID_TYPE
#define F SLANG_FILE_FD_TYPE
#define B SLANG_BSTRING_TYPE
#define R SLANG_REF_TYPE
#define U SLANG_UINT_TYPE
#define S SLANG_STRING_TYPE
#define L SLANG_LONG_TYPE
static SLang_Intrin_Fun_Type Fd_Name_Table [] =
{
   MAKE_INTRINSIC_0("fileno", posix_fileno, V),
   MAKE_INTRINSIC_0("_fileno", posix_fileno_int, I),
   MAKE_INTRINSIC_0("isatty", posix_isatty, I),
   MAKE_INTRINSIC_0("open", posix_open, V),
   MAKE_INTRINSIC_3("read", posix_read, V, F, R, U),
   MAKE_INTRINSIC_3("lseek", posix_lseek, SLANG_C_OFF_T_TYPE, F, SLANG_C_OFF_T_TYPE, I),
   MAKE_INTRINSIC_2("fdopen", posix_fdopen, V, F, S),
   MAKE_INTRINSIC_2("write", posix_write, V, F, B),
   MAKE_INTRINSIC_1("dup_fd", posix_dup, V, F),
   MAKE_INTRINSIC_2("dup2_fd", posix_dup2, I, F, I),
   MAKE_INTRINSIC_1("close", posix_close, I, F),
   MAKE_INTRINSIC_1("_close", posix_close_fd, I, I),
#if defined(TTYNAME_R)
   MAKE_INTRINSIC_0("ttyname", posix_ttyname, V),
#endif
   SLANG_END_INTRIN_FUN_TABLE
};
#undef I
#undef V
#undef F
#undef B
#undef R
#undef S
#undef L
#undef U

static SLang_IConstant_Type PosixIO_Consts [] =
{
#ifdef O_RDONLY
   MAKE_ICONSTANT("O_RDONLY", O_RDONLY),
#endif
#ifdef O_WRONLY
   MAKE_ICONSTANT("O_WRONLY", O_WRONLY),
#endif
#ifdef O_RDWR
   MAKE_ICONSTANT("O_RDWR", O_RDWR),
#endif
#ifdef O_APPEND
   MAKE_ICONSTANT("O_APPEND", O_APPEND),
#endif
#ifdef O_CREAT
   MAKE_ICONSTANT("O_CREAT", O_CREAT),
#endif
#ifdef O_EXCL
   MAKE_ICONSTANT("O_EXCL", O_EXCL),
#endif
#ifdef O_NOCTTY
   MAKE_ICONSTANT("O_NOCTTY", O_NOCTTY),
#endif
#ifdef O_NONBLOCK
   MAKE_ICONSTANT("O_NONBLOCK", O_NONBLOCK),
#endif
#ifdef O_TRUNC
   MAKE_ICONSTANT("O_TRUNC", O_TRUNC),
#endif
#ifndef O_BINARY
# define O_BINARY 0
#endif
   MAKE_ICONSTANT("O_BINARY", O_BINARY),
#ifndef O_TEXT
# define O_TEXT 0
#endif
   MAKE_ICONSTANT("O_TEXT", O_TEXT),
#ifdef O_LARGEFILE
   MAKE_ICONSTANT("O_LARGEFILE", O_LARGEFILE),
#endif
   SLANG_END_ICONST_TABLE
};

int SLfile_push_fd (SLFile_FD_Type *f)
{
   if (f == NULL)
     return SLang_push_null ();

   f->num_refs += 1;

   if (0 == SLclass_push_ptr_obj (SLANG_FILE_FD_TYPE, (VOID_STAR) f))
     return 0;

   f->num_refs -= 1;

   return -1;
}

int SLfile_pop_fd (SLFile_FD_Type **f)
{
   return SLclass_pop_ptr_obj (SLANG_FILE_FD_TYPE, (VOID_STAR *) f);
}

static void destroy_fd_type (SLtype type, VOID_STAR ptr)
{
   /* Avoid setting errno when a variable goes out of scope */
   int e = _pSLerrno_errno;

   (void) type;

   SLfile_free_fd (*(SLFile_FD_Type **) ptr);
   e = _pSLerrno_errno;
}

static int fd_push (SLtype type, VOID_STAR v)
{
   (void) type;
   return SLfile_push_fd (*(SLFile_FD_Type **)v);
}

static int
fd_fd_bin_op_result (int op, SLtype a, SLtype b,
			     SLtype *c)
{
   (void) a;
   (void) b;
   switch (op)
     {
      default:
	return 0;

#if 0
      case SLANG_GT:
      case SLANG_GE:
      case SLANG_LT:
      case SLANG_LE:
#endif
      case SLANG_EQ:
      case SLANG_NE:
	*c = SLANG_CHAR_TYPE;
	break;
     }
   return 1;
}

static int
fd_fd_bin_op (int op,
		      SLtype a_type, VOID_STAR ap, SLuindex_Type na,
		      SLtype b_type, VOID_STAR bp, SLuindex_Type nb,
		      VOID_STAR cp)
{
   char *ic;
   SLFile_FD_Type **a, **b;
   SLuindex_Type n, n_max;
   SLuindex_Type da, db;

   (void) a_type;
   (void) b_type;

   if (na == 1) da = 0; else da = 1;
   if (nb == 1) db = 0; else db = 1;

   if (na > nb) n_max = na; else n_max = nb;

   a = (SLFile_FD_Type **) ap;
   b = (SLFile_FD_Type **) bp;
   ic = (char *) cp;

   switch (op)
     {
      case SLANG_NE:
	for (n = 0; n < n_max; n++)
	  {
	     if ((*a == NULL) || (*b == NULL))
	       ic [n] = (*a != *b);
	     else
	       ic [n] = (*a)->fd != (*b)->fd;
	     a += da;
	     b += db;
	  }
	break;
      case SLANG_EQ:
	for (n = 0; n < n_max; n++)
	  {
	     if ((*a == NULL) || (*b == NULL))
	       ic [n] = (*a == *b);
	     else
	       ic [n] = (*a)->fd == (*b)->fd;
	     a += da;
	     b += db;
	  }
	break;

      default:
	return 0;
     }
   return 1;
}

int SLang_init_posix_io (void)
{
   SLang_Class_Type *cl;

   if (NULL == (cl = SLclass_allocate_class ("FD_Type")))
     return -1;
   cl->cl_destroy = destroy_fd_type;
   (void) SLclass_set_push_function (cl, fd_push);
   cl->cl_datatype_deref = fdtype_datatype_deref;

   if ((-1 == SLclass_register_class (cl, SLANG_FILE_FD_TYPE, sizeof (SLFile_FD_Type), SLANG_CLASS_TYPE_PTR))
       || (-1 == SLclass_add_binary_op (SLANG_FILE_FD_TYPE, SLANG_FILE_FD_TYPE, fd_fd_bin_op, fd_fd_bin_op_result)))
     return -1;

   if ((-1 == SLadd_intrin_fun_table(Fd_Name_Table, "__POSIXIO__"))
       || (-1 == SLadd_iconstant_table (PosixIO_Consts, NULL))
       || (-1 == _pSLerrno_init ()))
     return -1;

   return 0;
}

