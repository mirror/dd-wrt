/* Pathname intrinsic functions */
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

#include "slang.h"
#include "_slang.h"

#ifdef VMS
# include <stat.h>
#else
# include <sys/stat.h>
#endif

static void path_concat (char *a, char *b)
{
   SLang_push_malloced_string (SLpath_dircat (a,b));
}

static void path_extname (char *path)
{
#ifdef VMS
   char *p;
#endif

   path = SLpath_extname (path);
#ifndef VMS
   SLang_push_string (path);
#else
   p = strchr (path, ';');
   if (p == NULL)
     (void)SLang_push_string (p);
   else
     (void)SLang_push_malloced_string (SLmake_nstring (path, (unsigned int)(p - path)));
#endif
}

static void path_basename (char *path)
{
   (void) SLang_push_string (SLpath_basename (path));
}

static void path_dirname (char *path)
{
   (void) SLang_push_malloced_string (SLpath_dirname (path));
}

static void path_sans_extname (char *path)
{
   (void) SLang_push_malloced_string (SLpath_pathname_sans_extname (path));
}

static void path_basename_sans_extname (char *path)
{
   if (NULL == (path = SLpath_pathname_sans_extname (path)))
     return;

   path_basename (path);
   SLfree (path);
}

static SLFUTURE_CONST char *Load_Path;
int SLpath_set_load_path (SLFUTURE_CONST char *path)
{
   if (path == NULL)
     {
	SLang_free_slstring ((char *) Load_Path);
	Load_Path = NULL;
	return 0;
     }

   path = SLang_create_slstring (path);
   if (path == NULL)
     return -1;

   if (Load_Path != NULL)
     SLang_free_slstring ((char *) Load_Path);

   Load_Path = path;
   return 0;
}

char *SLpath_get_load_path (void)
{
   return SLang_create_slstring (Load_Path);
}

static SLCONST char *get_load_path (void)
{
   if (Load_Path == NULL)
     return "";
   return Load_Path;
}

static void set_load_path (char *path)
{
   (void) SLpath_set_load_path (path);
}

static char *more_recent (char *a, char *b)
{
   unsigned long ta, tb;
   struct stat st;

   if (a == NULL)
     return b;
   if (b == NULL)
     return a;

   if (-1 == stat (a, &st))
     return b;
   ta = (unsigned long) st.st_mtime;
   if (-1 == stat (b, &st))
     return a;
   tb = (unsigned long) st.st_mtime;

   if (tb >= ta)
     return b;

   return a;
}

/* returns SLmalloced string */
static char *find_file (SLFUTURE_CONST char *path, SLFUTURE_CONST char *file)
{
   char *dirfile;
   char *extname;
   char *filebuf;
   char *filesl, *fileslc;
   unsigned int len;

   if (NULL != (dirfile = SLpath_find_file_in_path (path, file)))
     return dirfile;

   /* Not found, or an error occured. */
   if (_pSLang_Error)
     return NULL;

   extname = SLpath_extname (file);
   if (*extname != 0)
     return NULL;

   /* No extension.  So look for .slc and .sl forms */
   len = (extname - file);
   filebuf = SLmalloc (len + 5);
   strcpy (filebuf, file);
   strcpy (filebuf + len, ".sl");

   filesl = SLpath_find_file_in_path (path, filebuf);
   if ((filesl == NULL) && _pSLang_Error)
     {
	SLfree (filebuf);
	return NULL;
     }
   strcpy (filebuf + len, ".slc");
   fileslc = SLpath_find_file_in_path (path, filebuf);
   SLfree (filebuf);

   dirfile = more_recent (filesl, fileslc);

   if (dirfile != filesl)
     SLfree (filesl);
   if (dirfile != fileslc)
     SLfree (fileslc);

   return dirfile;
}

char *_pSLpath_find_file (SLFUTURE_CONST char *file, int signal_error)
{
   SLFUTURE_CONST char *path;
   char *dirfile;

   if (file == NULL)
     return NULL;

   path = Load_Path;
   if ((path == NULL) || (*path == 0))
     path = ".";

   dirfile = find_file (path, file);

   if (dirfile != NULL)
     {
	file = SLang_create_slstring (dirfile);
	SLfree (dirfile);
	return (char *) file;
     }

   if (signal_error)
     _pSLang_verror (SL_OBJ_NOPEN, "Unable to locate %s on load path", file);
   return NULL;
}

static void get_path_delimiter (void)
{
   (void) SLang_push_char ((char) SLpath_get_delimiter ());
}

static int path_is_absolute_path (char *s)
{
   return SLpath_is_absolute_path (s);
}

#if 0
static void set_path_delimiter (int *d)
{
   (void) SLpath_set_delimiter (*d);
}
#endif
static SLang_Intrin_Fun_Type Path_Name_Table [] =
{
   MAKE_INTRINSIC_S("set_slang_load_path", set_load_path, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("get_slang_load_path", get_load_path, SLANG_STRING_TYPE),
   MAKE_INTRINSIC_0("path_get_delimiter", get_path_delimiter, SLANG_VOID_TYPE),
   /* MAKE_INTRINSIC_I("path_set_delimiter", set_path_delimiter, SLANG_VOID_TYPE), */
   MAKE_INTRINSIC_SS("path_concat", path_concat, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("path_extname", path_extname, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("path_dirname", path_dirname, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("path_basename", path_basename, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("path_basename_sans_extname", path_basename_sans_extname, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("path_sans_extname", path_sans_extname, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("path_is_absolute", path_is_absolute_path, SLANG_INT_TYPE),
   SLANG_END_INTRIN_FUN_TABLE
};

int SLang_init_ospath (void)
{
   if (-1 == SLadd_intrin_fun_table(Path_Name_Table, "__OSPATH__"))
     return -1;

   return 0;
}

