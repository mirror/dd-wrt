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

#if defined (HAVE_DLFCN_H) || defined(__WIN32__)
# define SLANG_HAS_DYNAMIC_LINKING	1
#else
# define SLANG_HAS_DYNAMIC_LINKING	0
#endif

/* The rest of this file is in the if block */
#if SLANG_HAS_DYNAMIC_LINKING

#ifdef HAVE_DLFCN_H
# include <dlfcn.h>
#endif

#ifdef __WIN32__
# include <windows.h>
# define dlopen(a, b) LoadLibrary(a)
# define dlsym (void *)GetProcAddress
# define dlclose FreeLibrary
# define dlerror() NULL
# define SO_SUFFIX "dll"
#else
# define SO_SUFFIX "so"
#endif

static SLFUTURE_CONST char *Module_Path;
#ifndef MODULE_PATH_ENV_NAME
# define MODULE_PATH_ENV_NAME "SLANG_MODULE_PATH"
#endif
#ifndef MODULE_INSTALL_DIR
# define MODULE_INSTALL_DIR "/usr/local/lib/slang/v2/modules"
#endif

#define MAX_MODULE_NAME_SIZE 256

typedef struct Namespace_List_Type
{
   char *ns;
   struct Namespace_List_Type *next;
}
Namespace_List_Type;

typedef struct _Handle_Type
{
   struct _Handle_Type *next;
   char *module_name;
   VOID_STAR handle;
   int (*ns_init_fun) (SLCONST char *);
   void (*deinit_fun) (void);
   Namespace_List_Type *ns_list;
}
Handle_Type;

static Handle_Type *Handle_List;

static void free_namespace_list (Namespace_List_Type *ns_list)
{
   while (ns_list != NULL)
     {
	Namespace_List_Type *next = ns_list->next;
	SLang_free_slstring (ns_list->ns);
	SLfree ((char *)ns_list);
	ns_list = next;
     }
}

static void free_handle_type (Handle_Type *h)
{
   if (h == NULL)
     return;

   SLang_free_slstring (h->module_name);
   free_namespace_list (h->ns_list);
   SLfree ((char *)h);
}

static void delete_handles (void)
{
   while (Handle_List != NULL)
     {
	Handle_Type *next = Handle_List->next;

	if (Handle_List->deinit_fun != NULL)
	  Handle_List->deinit_fun ();
	/* (void) dlclose (Handle_List->handle); */
	free_handle_type (Handle_List);
	Handle_List = next;
     }
}

static Handle_Type *allocate_handle_type (SLFUTURE_CONST char *module_name, VOID_STAR handle)
{
   Handle_Type *h;

   h = (Handle_Type *) SLcalloc (1, sizeof (Handle_Type));
   if (h == NULL)
     return NULL;
   if (NULL == (h->module_name = SLang_create_slstring (module_name)))
     {
	SLfree ((char *) h);
	return NULL;
     }
   h->handle = handle;
   return h;
}

static Handle_Type *find_handle (SLCONST char *module_name)
{
   Handle_Type *l;

   l = Handle_List;
   while (l != NULL)
     {
	if (0 == strcmp (l->module_name, module_name))
	  break;
	l = l->next;
     }
   return l;
}

static int check_api_version (char *file, int api_version)
{
   if (api_version/10000 == SLANG_VERSION/10000)
     return 0;

   _pSLang_verror (SL_Import_Error, "Module %s is incompatible with this version of S-Lang",
		 file);
   return -1;
}

static FVOID_STAR do_dlsym (VOID_STAR handle, SLFUTURE_CONST char *file, int check_error, SLFUTURE_CONST char *fmt, char *module)
{
   char symbol[MAX_MODULE_NAME_SIZE + 32];
   FVOID_STAR s;

   SLsnprintf (symbol, sizeof(symbol), fmt, module);
   if (NULL != (s = (FVOID_STAR) dlsym (handle, symbol)))
     return s;

   if (check_error)
     {
	SLCONST char *err;

	if (NULL == (err = (char *) dlerror ()))
	  err = "UNKNOWN";

	_pSLang_verror (SL_Import_Error,
		      "Unable to get symbol %s from %s: %s",
		      symbol, file, err);
     }
   return NULL;
}

static Handle_Type *dynamic_link_module (SLFUTURE_CONST char *module)
{
   Handle_Type *h;
   VOID_STAR handle;
   SLFUTURE_CONST char *err;
   char filebuf[1024];
   char *save_file;
   char *save_err;
   int api_version;
   int *api_version_ptr;
#define MAX_MODULE_NAME_SIZE 256
   char module_so[MAX_MODULE_NAME_SIZE + 32];
   char *module_name;
   char *file, *pathfile;

   if (strlen (module) >= MAX_MODULE_NAME_SIZE)
     {
	_pSLang_verror (SL_LimitExceeded_Error, "module name too long");
	return NULL;
     }
   SLsnprintf (module_so, sizeof(module_so), "%s-module.%s", module, SO_SUFFIX);

   if (Module_Path != NULL)
     pathfile = SLpath_find_file_in_path (Module_Path, module_so);
   else pathfile = NULL;

   if ((pathfile == NULL)
       && (NULL != (pathfile = _pSLsecure_getenv (MODULE_PATH_ENV_NAME))))
     pathfile = SLpath_find_file_in_path (pathfile, module_so);

   if (pathfile == NULL)
     pathfile = SLpath_find_file_in_path (MODULE_INSTALL_DIR, module_so);

   if (pathfile != NULL)
     file = pathfile;
   else
     file = module_so;

   save_err = NULL;
   save_file = file;
   while (1)
     {
#ifndef RTLD_GLOBAL
# define RTLD_GLOBAL 0
#endif
#ifdef RTLD_NOW
	handle = (VOID_STAR) dlopen (file, RTLD_NOW | RTLD_GLOBAL);
#else
	handle = (VOID_STAR) dlopen (file, RTLD_LAZY | RTLD_GLOBAL);
#endif

	if (handle != NULL)
	  {
	     if (_pSLang_Load_File_Verbose & SLANG_LOAD_MODULE_VERBOSE)
	       SLang_vmessage ("Importing %s", file);
	     if (save_err != NULL)
	       SLfree (save_err);
	     break;
	  }

	/* Purify reports that dlerror returns a pointer that generates UMR
	 * errors.  There is nothing that I can do about that....
	 */
	if ((NULL == strchr (file, '/'))
	    && (strlen(file) < sizeof(filebuf)))
	  {
	     err = (char *) dlerror ();
	     if (err != NULL)
	       save_err = SLmake_string (err);

	     SLsnprintf (filebuf, sizeof (filebuf), "./%s", file);
	     file = filebuf;
	     continue;
	  }

	if ((NULL == (err = save_err))
	    && (NULL == (err = (char *) dlerror ())))
	  err = "UNKNOWN";

	_pSLang_verror (SL_Import_Error,
		      "Error linking to %s: %s", save_file, err);

	if (save_err != NULL)
	  SLfree (save_err);
	if (pathfile != NULL)
	  SLfree (pathfile);

	return NULL;
     }

   /* Using SLpath_basename allows, e.g., import ("/path/to/module"); */
   module_name = SLpath_basename (module);

   api_version_ptr = (int *) do_dlsym (handle, file, 0, "SLmodule_%s_api_version", module_name);
   if (api_version_ptr == NULL)
     api_version_ptr = (int *) do_dlsym (handle, file, 0, "_SLmodule_%s_api_version", module_name);

   if (api_version_ptr == NULL)
     api_version = 0;
   else
     api_version = *api_version_ptr;

   if ((-1 == check_api_version (file, api_version))
       || (NULL == (h = allocate_handle_type (module, handle))))
     {
	SLfree (pathfile);	       /* NULL ok */
	dlclose (handle);
	return NULL;
     }

   if (NULL == (h->ns_init_fun = (int (*)(SLCONST char *)) do_dlsym (handle, file, 1, "init_%s_module_ns", module_name)))
     {
	SLfree (pathfile);
	free_handle_type (h);
	dlclose (handle);
	return NULL;
     }
   h->deinit_fun = (void (*)(void)) do_dlsym (handle, file, 0, "deinit_%s_module", module_name);

   SLfree (pathfile);		       /* NULL ok */
   h->next = Handle_List;
   Handle_List = h;

   return h;
}

static int import_module (SLFUTURE_CONST char *module, SLFUTURE_CONST char *ns)
{
   Handle_Type *h;
   Namespace_List_Type *ns_list;

   if (ns == NULL)
     ns = _pSLang_cur_namespace_intrinsic ();

   if (*ns == 0)
     ns = "Global";

   if ((NULL == (h = find_handle (module)))
       && (NULL == (h = dynamic_link_module (module))))
     return -1;

   ns_list = h->ns_list;
   while (ns_list != NULL)
     {
	if (0 == strcmp (ns, ns_list->ns))
	  return 0;		       /* already linked to this ns */
	ns_list = ns_list->next;
     }

   if (NULL == (ns_list = (Namespace_List_Type *)SLmalloc (sizeof (Namespace_List_Type))))
     return -1;

   if (NULL == (ns_list->ns = SLang_create_slstring (ns)))
     {
	SLfree ((char *)ns_list);
	return -1;
     }
   ns_list->next = h->ns_list;
   h->ns_list = ns_list;

   if (-1 == (h->ns_init_fun (ns)))
     return -1;

   return 0;
}

static void import_module_intrin (void)
{
   char *module;
   char *ns = NULL;

   if (SLang_Num_Function_Args == 2)
     {
	if (-1 == SLang_pop_slstring (&ns))
	  return;
     }

   if (-1 == SLang_pop_slstring (&module))
     {
	SLang_free_slstring (ns);      /* NULL ok */
	return;
     }

   (void) import_module (module, ns);
   SLang_free_slstring (module);
   SLang_free_slstring (ns);	       /* NULL ok */
}

static void set_import_module_path (char *path)
{
   (void) SLang_set_module_load_path (path);
}

static SLCONST char *get_import_module_path (void)
{
   char *path;
   if (Module_Path != NULL)
     return Module_Path;
   if (NULL != (path = _pSLsecure_getenv (MODULE_PATH_ENV_NAME)))
     return path;
   return MODULE_INSTALL_DIR;
}

static SLang_Intrin_Fun_Type Module_Intrins [] =
{
   MAKE_INTRINSIC_0("import", import_module_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("set_import_module_path", set_import_module_path, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("get_import_module_path", get_import_module_path, SLANG_STRING_TYPE),
   SLANG_END_INTRIN_FUN_TABLE
};

#endif				       /* SLANG_HAS_DYNAMIC_LINKING */

int SLang_set_module_load_path (SLFUTURE_CONST char *path)
{
#if SLANG_HAS_DYNAMIC_LINKING
   if (NULL == (path = SLang_create_slstring (path)))
     return -1;
   SLang_free_slstring ((char *) Module_Path);
   Module_Path = path;
   return 0;
#else
   (void) path;
   return -1;
#endif
}

int SLang_init_import (void)
{
#if SLANG_HAS_DYNAMIC_LINKING
   (void) SLang_add_cleanup_function (delete_handles);
   return SLadd_intrin_fun_table (Module_Intrins, "__IMPORT__");
#else
   return 0;
#endif
}
