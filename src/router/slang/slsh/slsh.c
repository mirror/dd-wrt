/*
Copyright (C) 2005-2011 John E. Davis

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
#include <stdlib.h>
#ifdef __WIN32__
# include <windows.h>
# include <io.h>
#endif

#include <sys/stat.h>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <string.h>
#include <signal.h>
#include <slang.h>

static char *Slsh_Version = "0.8.6-0";
#define SLSHRC_FILE "slsh.rc"
#include "slsh.h"

#ifndef SLSH_CONF_DIR_ENV
# define SLSH_CONF_DIR_ENV "SLSH_CONF_DIR"
#endif

#ifndef SLSH_PATH_ENV
# define SLSH_PATH_ENV "SLSH_PATH"
#endif

#ifndef SLSH_LIB_DIR_ENV
# define SLSH_LIB_DIR_ENV "SLSH_LIB_DIR"
#endif

#if defined(REAL_UNIX_SYSTEM) || defined(__APPLE__)
/* # define DEFAULT_LIBRARY_PATH "/usr/local/share/slsh:/usr/local/lib/slsh:/usr/share/slsh:/usr/lib/slsh"; */
# define DEFAULT_CONF_PATH "/usr/local/etc:/usr/local/etc/slsh:/etc:/etc/slsh";
# define USER_SLSHRC ".slshrc"
#else
# define DEFAULT_CONF_PATH NULL
# define USER_SLSHRC "slsh.rc"
#endif

#ifdef __os2__
# ifdef __IBMC__
/* IBM VA3 doesn't declare S_IFMT */
#  define	S_IFMT	(S_IFDIR | S_IFCHR | S_IFREG)
# endif
#endif

#ifndef S_ISLNK
# ifdef S_IFLNK
#   define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
# else
#   define S_ISLNK(m) 0
# endif
#endif

#ifndef S_ISREG
# ifdef S_IFREG
#   define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
# else
#   define S_ISREG(m) 0
# endif
#endif

#ifndef S_ISDIR
# ifdef S_IFDIR
#   define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
# else
#   define S_ISDIR(m) 0
# endif
#endif

#ifndef S_ISCHR
# ifdef S_IFCHR
#   define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
# else
#   define S_ISCHR(m) 0
# endif
#endif

#ifndef S_ISBLK
# ifdef S_IFBLK
#   define S_ISBLK(m) (((m) & S_IFMT) == S_IFBLK)
# else
#   define S_ISBLK(m) 0
# endif
#endif

#ifndef S_ISFIFO
# ifdef S_IFIFO
#   define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
# else
#   define S_ISFIFO(m) 0
# endif
#endif

#ifndef S_ISSOCK
# ifdef S_IFSOCK
#   define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)
# else
#   define S_ISSOCK(m) 0
# endif
#endif

#ifndef S_IRUSR
# define S_IRUSR	0400
#endif
#ifndef S_IWUSR
# define S_IWUSR	0200
#endif
#ifndef S_IXUSR
# define S_IXUSR	0100
#endif
#ifndef S_IRGRP
# define S_IRGRP	0040
#endif
#ifndef S_IWGRP
# define S_IWGRP	0020
#endif
#ifndef S_IXGRP
# define S_IXGRP	0010
#endif
#ifndef S_IROTH
# define S_IROTH	0004
#endif
#ifndef S_IWOTH
# define S_IWOTH	0002
#endif
#ifndef S_IXOTH
# define S_IXOTH	0001
#endif
#ifndef S_ISUID
# define S_ISUID	04000
#endif
#ifndef S_ISGID
# define S_ISGID	02000
#endif
#ifndef S_ISVTX
# define S_ISVTX	01000
#endif

typedef struct _AtExit_Type
{
   SLang_Name_Type *nt;
   struct _AtExit_Type *next;
}
AtExit_Type;

static AtExit_Type *AtExit_Hooks;

static void at_exit (SLang_Ref_Type *ref)
{
   SLang_Name_Type *nt;
   AtExit_Type *a;

   if (NULL == (nt = SLang_get_fun_from_ref (ref)))
     return;

   a = (AtExit_Type *) SLmalloc (sizeof (AtExit_Type));
   if (a == NULL)
     return;

   a->nt = nt;
   a->next = AtExit_Hooks;
   AtExit_Hooks = a;
}

static void c_exit (int status)
{
   while (AtExit_Hooks != NULL)
     {
	AtExit_Type *next = AtExit_Hooks->next;
	if (SLang_get_error () == 0)
	  (void) SLexecute_function (AtExit_Hooks->nt);

	SLfree ((char *) AtExit_Hooks);
	AtExit_Hooks = next;
     }

   if (SLang_get_error ())
     SLang_restart (1);

   exit (status);
}

static void exit_intrin (void)
{
   int status;

   if (SLang_Num_Function_Args == 0)
     status = 0;
   else if (-1 == SLang_pop_int (&status))
     return;

   c_exit (status);
}

static void stat_mode_to_string (void)
{
   int mode, opts;
   char mode_string[12];

   opts = 0;
   if (SLang_Num_Function_Args == 2)
     {
	if (-1 == SLang_pop_integer (&opts))
	  return;
     }

   if (-1 == SLang_pop_integer (&mode))
     return;

   if (S_ISREG(mode)) mode_string[0] = '-';
   else if (S_ISDIR(mode)) mode_string[0] = 'd';
   else if (S_ISLNK(mode)) mode_string[0] = 'l';
   else if (S_ISCHR(mode)) mode_string[0] = 'c';
   else if (S_ISFIFO(mode)) mode_string[0] = 'p';
   else if (S_ISSOCK(mode)) mode_string[0] = 's';
   else if (S_ISBLK(mode)) mode_string[0] = 'b';

   if (mode & S_IRUSR) mode_string[1] = 'r'; else mode_string[1] = '-';
   if (mode & S_IWUSR) mode_string[2] = 'w'; else mode_string[2] = '-';
   if (mode & S_IXUSR) mode_string[3] = 'x'; else mode_string[3] = '-';
   if (mode & S_ISUID) mode_string[3] = 's';

#ifdef __WIN32__
   mode_string[4] = '-';
   mode_string[5] = '-';
   mode_string[6] = '-';

   if (opts & FILE_ATTRIBUTE_ARCHIVE) mode_string[7] = 'A'; else mode_string[7] = '-';
   if (opts & FILE_ATTRIBUTE_SYSTEM) mode_string[8] = 'S'; else mode_string[8] = '-';
   if (opts & FILE_ATTRIBUTE_HIDDEN) mode_string[9] = 'H'; else mode_string[9] = '-';
#else
   if (mode & S_IRGRP) mode_string[4] = 'r'; else mode_string[4] = '-';
   if (mode & S_IWGRP) mode_string[5] = 'w'; else mode_string[5] = '-';
   if (mode & S_IXGRP) mode_string[6] = 'x'; else mode_string[6] = '-';
   if (mode & S_ISGID) mode_string[6] = 'g';

   if (mode & S_IROTH) mode_string[7] = 'r'; else mode_string[7] = '-';
   if (mode & S_IWOTH) mode_string[8] = 'w'; else mode_string[8] = '-';
   if (mode & S_IXOTH) mode_string[9] = 'x'; else mode_string[9] = '-';
   if (mode & S_ISVTX) mode_string[9] = 't';
#endif

   mode_string[10] = 0;
   (void) SLang_push_string (mode_string);
}

#ifdef __WIN32__

/* Root/
 * 	bin/
 * 		slsh.exe
 * 	slsh/
 * 		slsh.rc
 * 		autoload.sl
 * 		( ... )
 */
static char *get_win32_root (void)
{
   static char base_path[MAX_PATH] = "";

   if (*base_path == 0)
     {
	if (GetModuleFileName(NULL, base_path, MAX_PATH-10) > 0)
	  {
	     /* drop file name */
	     char *p = strrchr(base_path, '\\');
	     if (p != NULL)
	       {
		  *p = 0;
		  /* drop also 'bin' */
		  p = strrchr(base_path, '\\');
		  if (p != NULL)
		    {
		       strcpy(p, "\\slsh\\");
		    }
	       }
	  }
     }

   return base_path;
}
#endif

static int Verbose_Loading;

static int try_to_load_file (char *path, char *file, char *ns)
{
   int status;

   if (path == NULL)
     path = ".";

   if (file != NULL)
     {
	file = SLpath_find_file_in_path (path, file);
	if (file == NULL)
	  return 0;
     }
   /* otherwise use stdin */

   status = SLns_load_file (file, ns);
   SLfree (file);
   if (status == 0)
     return 1;
   return -1;
}

static int load_startup_file (int is_interactive)
{
   char *dir;
   int status;

   dir = getenv (SLSH_CONF_DIR_ENV);
   if (dir == NULL)
     dir = getenv (SLSH_LIB_DIR_ENV);

   if (NULL == dir)
     {
#ifdef SLSH_CONF_DIR
	dir = SLSH_CONF_DIR;
	if (dir != NULL)
	  {
	     status = try_to_load_file (dir, SLSHRC_FILE, NULL);
	     if (status == -1)
	       return -1;
	     if (status == 1)
	       return 0;
	  }
#endif

#ifdef __WIN32__
	if (NULL == (dir = get_win32_root ()))
	  dir = DEFAULT_CONF_PATH;
#else
	dir = DEFAULT_CONF_PATH;
#endif
     }

   if (-1 == (status = try_to_load_file (dir, SLSHRC_FILE, NULL)))
     return -1;

   if ((status == 0)
       && (Verbose_Loading || is_interactive))
     {
	SLang_vmessage ("*** Installation Problem?  Unable to find the %s config file.",
			SLSHRC_FILE);
     }

   return 0;
}

#if 0
static int is_script (char *file)
{
   FILE *fp;
   char buf[3];
   int is;

   if (NULL == (fp = fopen (file, "r")))
     return 0;

   is = ((NULL != fgets (buf, sizeof(buf), fp))
	 && (buf[0] == '#') && (buf[1] == '!'));

   fclose (fp);
   return is;
}
#endif

static int setup_paths (void)
{
   char *libpath;

   if (NULL == (libpath = getenv (SLSH_PATH_ENV)))
     {
#ifdef SLSH_PATH
	libpath = SLSH_PATH;
#else
# ifdef __WIN32__
	libpath = get_win32_root ();
# endif
#endif
     }

   return SLpath_set_load_path (libpath);
}

static int set_verbose_loading (int *val)
{
   return SLang_load_file_verbose (*val);
}

/* Create the Table that S-Lang requires */
static SLang_Intrin_Fun_Type Intrinsics [] =
{
   MAKE_INTRINSIC_0("exit", exit_intrin, VOID_TYPE),
   MAKE_INTRINSIC_1("atexit", at_exit, VOID_TYPE, SLANG_REF_TYPE),
   MAKE_INTRINSIC_0("stat_mode_to_string", stat_mode_to_string, VOID_TYPE),
   MAKE_INTRINSIC_1("set_verbose_loading", set_verbose_loading, SLANG_INT_TYPE, SLANG_INT_TYPE),
   SLANG_END_INTRIN_FUN_TABLE
};

static void usage (void)
{
   char *libpath;
   fprintf (stderr, "\
Usage: slsh [OPTIONS] [-|file [args...]]\n\
 --help           Print this help\n\
 --version        Show slsh version information\n\
 -e string        Execute 'string' as S-Lang code\n\
 -g               Compile with debugging code, tracebacks, etc\n\
 -n               Don't load personal init file\n\
 --init file      Use this file instead of ~/%s\n",
	    USER_SLSHRC);
   fprintf (stderr, "\
 --no-readline    Do not use readline\n\
 -i               Force interactive input\n\
 -q, --quiet      Do not print startup messages\n\
 -t               Test mode.  If slsh_main exists, do not call it\n\
 -v               Show verbose loading messages\n\
 -Dname           Define \"name\" as a preprocessor symbol\n\
\n\
  Note: - and -i are mutually exclusive\n\
\n\
"
	    );
   libpath = SLpath_get_load_path ();
   fprintf (stderr, "Default search path: %s\n", (libpath == NULL) ? "" : libpath);
   SLang_free_slstring (libpath);

   exit (1);
}

static void output_version (void)
{
   fprintf (stdout, "slsh version %s; ", Slsh_Version);
   fprintf (stdout, "S-Lang version: %s\n", SLang_Version_String);
   if (SLANG_VERSION != SLang_Version)
     {
	fprintf (stdout, "\t** Note: This program was compiled against version %s.\n",
		 SLANG_VERSION_STRING);
     }
}

static int output_copyright (void)
{
   output_version ();
   fprintf (stdout, "Copyright (C) 2005-2011 John E. Davis <jed@jedsoft.org>\r\n");
   fprintf (stdout, "This is free software with ABSOLUTELY NO WARRANTY.\r\n");
   fprintf (stdout, "\n");

   return 0;
}

static void version (void)
{
   output_version ();
   exit (0);
}

int main (int argc, char **argv)
{
   char *file = NULL;
   char *init_file = USER_SLSHRC;
   char *init_file_dir;
   int exit_val;
   int is_interactive = 0;
   int use_readline = 1;
   int test_mode = 0;
   char *exec_string = NULL;
   int quiet = 0;

   (void) SLutf8_enable (-1);

   if ((-1 == SLang_init_all ())
       || (-1 == SLang_init_array_extra ())
       || (-1 == SLang_init_import ()) /* dynamic linking */
       || (-1 == SLadd_intrin_fun_table (Intrinsics, NULL))
       || (-1 == slsh_init_readline_intrinsics ()))
     {
	fprintf(stderr, "Unable to initialize S-Lang.\n");
	return 1;
     }

#ifdef SIGPIPE
   (void) SLsignal (SIGPIPE, SIG_IGN);
#endif

   /* FIXME for other systems */
#ifdef __WIN32__
   init_file_dir = getenv ("USERPROFILE");
#else
   init_file_dir = getenv ("HOME");
#endif

   if (-1 == setup_paths ())
     return -1;

#ifdef __WIN32__
   /* hack a directory for modules */
     {
	char *slshdir;
	char buffer[MAX_PATH] = "";

	slshdir = get_win32_root();
	if (slshdir != NULL)
	  strcpy(buffer, slshdir);
	strcat(buffer, "modules\\");
	SLang_set_module_load_path (buffer);
     }
#endif

   while (argc > 1)
     {
	char *arg = argv[1];

	if (0 == strcmp (arg, "--version"))
	  version ();

	if (0 == strcmp (arg, "--help"))
	  usage ();

	if (0 == strcmp (arg, "-i"))
	  {
	     argc--;
	     argv++;
	     is_interactive = 1;
	     continue;
	  }

	if ((0 == strcmp (arg, "-e"))
	    && (argc > 2))
	  {
	     argc -= 2;
	     argv += 2;
	     exec_string = *argv;
	     continue;
	  }

	if (0 == strcmp (arg, "-g"))
	  {
	     SLang_generate_debug_info (1);
	     SLang_Traceback = SL_TB_FULL;
	     argc--;
	     argv++;
	     continue;
	  }

	if (0 == strcmp (arg, "-n"))
	  {
	     init_file = NULL;
	     argc--;
	     argv++;
	     continue;
	  }

	if ((0 == strcmp (arg, "-q")) || (0 == strcmp (arg, "--quiet")))
	  {
	     quiet = 1;
	     argc--;
	     argv++;
	     continue;
	  }

	if (0 == strcmp (arg, "-t"))
	  {
	     test_mode = 1;
	     argc--;
	     argv++;
	     continue;
	  }

	if (0 == strcmp (arg, "-v"))
	  {
	     (void) SLang_load_file_verbose (3);
	     Verbose_Loading = 1;
	     argc--;
	     argv++;
	     continue;
	  }

	if (0 == strcmp (arg, "--no-readline"))
	  {
	     use_readline = 0;
	     argc--;
	     argv++;
	     continue;
	  }

	if ((0 == strcmp (arg, "--init"))
	    && (argc > 2))
	  {
	     init_file = argv[2];
	     init_file_dir = NULL;
	     argc -= 2;
	     argv += 2;
	     continue;
	  }

	if (0 == strncmp (arg, "-D", 2))
	  {
	     char *prep = arg + 2;
	     if (*prep != 0)
	       (void) SLdefine_for_ifdef (prep);
	     argc--;
	     argv++;
	  }

	break;
     }

   if (argc == 1)
     {
	if ((exec_string == NULL) && (is_interactive == 0))
	  is_interactive = (isatty (fileno(stdin)) && isatty (fileno(stdout)));
	file = NULL;
     }
   else
     {
	file = argv[1];
	if (0 == strcmp (file, "-"))
	  {
	     if (is_interactive)
	       usage ();
	     file = NULL;
	  }

	argc--;
	argv++;
     }

   if ((is_interactive == 0)
       && (SLang_Version < SLANG_VERSION))
     {
	fprintf (stderr, "***Warning: Executable compiled against S-Lang %s but linked to %s\n",
		 SLANG_VERSION_STRING, SLang_Version_String);
	fflush (stderr);
     }

   /* fprintf (stdout, "slsh: argv[0]=%s\n", argv[0]); */
   if (-1 == SLang_set_argc_argv (argc, argv))
     return 1;

   if (is_interactive)
     (void) SLdefine_for_ifdef ("__INTERACTIVE__");

   if (-1 == load_startup_file (is_interactive))
     return SLang_get_error ();

   /* Initializing the readline interface causes the .slrlinerc file
    * to be loaded.  It is put here after the startup files have been loaded.
    */
   if (-1 == slsh_use_readline (SLpath_basename (argv[0]), use_readline, is_interactive))
     return 1;

   if ((init_file != NULL)
       && (-1 == try_to_load_file (init_file_dir, init_file, NULL)))
     return SLang_get_error ();

   if ((file != NULL)
       || ((is_interactive == 0) && (exec_string == NULL)))
     {
	if (0 == try_to_load_file (NULL, file, NULL))
	  {
	     fprintf (stderr, "%s: file not found\n", file);
	     exit (1);
	  }
	if (test_mode == 0)
	  (void) SLang_run_hooks ("slsh_main", 0);
     }

   if (exec_string != NULL)
     {
	(void) SLang_load_string (exec_string);
     }

   if (is_interactive)
     {
	if (quiet == 0)
	  output_copyright ();
	if (SLang_Traceback != SL_TB_FULL)
	  SLang_Traceback = SL_TB_NONE;

	(void) slsh_interactive ();
     }

   exit_val = SLang_get_error ();
   c_exit (exit_val);
   return SLang_get_error ();
}
