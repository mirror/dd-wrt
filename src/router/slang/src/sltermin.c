/* This file contains enough terminfo reading capabilities sufficient for
 * the slang SLtt interface.
 */

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

/*
 * The majority of the comments found in the file were taken from the
 * term(4) man page on an SGI.
 */

/* Short integers are stored in two 8-bit bytes.  The first byte contains
 * the least significant 8 bits of the value, and the second byte contains
 * the most significant 8 bits.  (Thus, the value represented is
 * 256*second+first.)  The value -1 is represented by 0377,0377, and the
 * value -2 is represented by 0376,0377; other negative values are illegal.
 * The -1 generally means that a capability is missing from this terminal.
 * The -2 means that the capability has been cancelled in the terminfo
 * source and also is to be considered missing.
 */

static int make_integer (unsigned char *buf)
{
   register int lo, hi;
   lo = (int) *buf++; hi = (int) *buf;
   if (hi == 0377)
     {
	if (lo == 0377) return -1;
	if (lo == 0376) return -2;
     }
   return lo + 256 * hi;
}

/*
 * The compiled file is created from the source file descriptions of the
 * terminals (see the -I option of infocmp) by using the terminfo compiler,
 * tic, and read by the routine setupterm [see curses(3X).]  The file is
 * divided into six parts in the following order:  the header, terminal
 * names, boolean flags, numbers, strings, and string table.
 *
 * The header section begins the file.  This section contains six short
 * integers in the format described below.  These integers are (1) the magic
 * number (octal 0432); (2) the size, in bytes, of the names section; (3)
 * the number of bytes in the boolean section; (4) the number of short
 * integers in the numbers section; (5) the number of offsets (short
 * integers) in the strings section; (6) the size, in bytes, of the string
 * table.
 */

#define MAGIC 0432

/* In this structure, all char * fields are malloced EXCEPT if the
 * structure is SLTERMCAP.  In that case, only terminal_names is malloced
 * and the other fields are pointers into it.
 */
struct _pSLterminfo_Type
{
#define SLTERMINFO 1
#define SLTERMCAP  2
   unsigned int flags;

   unsigned int name_section_size;
   char *terminal_names;

   unsigned int boolean_section_size;
   unsigned char *boolean_flags;

   unsigned int num_numbers;
   unsigned char *numbers;

   unsigned int num_string_offsets;
   unsigned char *string_offsets;

   unsigned int string_table_size;
   char *string_table;

};

static char *tcap_getstr (SLCONST char *, SLterminfo_Type *);
static int tcap_getnum (SLCONST char *, SLterminfo_Type *);
static int tcap_getflag (SLCONST char *, SLterminfo_Type *);
static int tcap_getent (SLCONST char *, SLterminfo_Type *);

static FILE *open_terminfo (char *file, SLterminfo_Type *h)
{
   FILE *fp;
   unsigned char buf[12];

   /* Alan Cox reported a security problem here if the application using the
    * library is setuid.  So, I need to make sure open the file as a normal
    * user.  Unfortunately, there does not appear to be a portable way of
    * doing this, so I am going to use 'setfsgid' and 'setfsuid', which
    * are not portable.
    *
    * I will also look into the use of setreuid, seteuid and setregid, setegid.
    * FIXME: Priority=medium
    */
   fp = fopen (file, "rb");
   if (fp == NULL) return NULL;

   if ((12 == fread ((char *) buf, 1, 12, fp) && (MAGIC == make_integer (buf))))
     {
	h->name_section_size = make_integer (buf + 2);
	h->boolean_section_size = make_integer (buf + 4);
	h->num_numbers = make_integer (buf + 6);
	h->num_string_offsets = make_integer (buf + 8);
	h->string_table_size = make_integer (buf + 10);
     }
   else
     {
	fclose (fp);
	fp = NULL;
     }
   return fp;
}

/*
 * The terminal names section comes next.  It contains the first line of the
 * terminfo description, listing the various names for the terminal,
 * separated by the bar ( | ) character (see term(5)).  The section is
 * terminated with an ASCII NUL character.
 */

/* returns pointer to malloced space */
static unsigned char *read_terminfo_section (FILE *fp, unsigned int size)
{
   char *s;

   if (NULL == (s = (char *) SLmalloc (size))) return NULL;
   if (size != fread (s, 1, size, fp))
     {
	SLfree (s);
	return NULL;
     }
   return (unsigned char *) s;
}

static char *read_terminal_names (FILE *fp, SLterminfo_Type *t)
{
   return t->terminal_names = (char *) read_terminfo_section (fp, t->name_section_size);
}

/*
 * The boolean flags have one byte for each flag.  This byte is either 0 or
 * 1 as the flag is present or absent.  The value of 2 means that the flag
 * has been cancelled.  The capabilities are in the same order as the file
 * <term.h>.
 */

static unsigned char *read_boolean_flags (FILE *fp, SLterminfo_Type *t)
{
   /* Between the boolean section and the number section, a null byte is
    * inserted, if necessary, to ensure that the number section begins on an
    * even byte offset. All short integers are aligned on a short word
    * boundary.
    */

   unsigned int size = (t->name_section_size + t->boolean_section_size) % 2;
   size += t->boolean_section_size;

   return t->boolean_flags = read_terminfo_section (fp, size);
}

/*
 * The numbers section is similar to the boolean flags section.  Each
 * capability takes up two bytes, and is stored as a short integer.  If the
 * value represented is -1 or -2, the capability is taken to be missing.
 */

static unsigned char *read_numbers (FILE *fp, SLterminfo_Type *t)
{
   return t->numbers = read_terminfo_section (fp, 2 * t->num_numbers);
}

/* The strings section is also similar.  Each capability is stored as a
 * short integer, in the format above.  A value of -1 or -2 means the
 * capability is missing.  Otherwise, the value is taken as an offset from
 * the beginning of the string table.  Special characters in ^X or \c
 * notation are stored in their interpreted form, not the printing
 * representation.  Padding information ($<nn>) and parameter information
 * (%x) are stored intact in uninterpreted form.
 */

static unsigned char *read_string_offsets (FILE *fp, SLterminfo_Type *t)
{
   return t->string_offsets = (unsigned char *) read_terminfo_section (fp, 2 * t->num_string_offsets);
}

/* The final section is the string table.  It contains all the values of
 * string capabilities referenced in the string section.  Each string is
 * null terminated.
 */

static char *read_string_table (FILE *fp, SLterminfo_Type *t)
{
   return t->string_table = (char *) read_terminfo_section (fp, t->string_table_size);
}

/*
 * Compiled terminfo(4) descriptions are placed under the directory
 * /usr/share/lib/terminfo.  In order to avoid a linear search of a huge
 * UNIX system directory, a two-level scheme is used:
 * /usr/share/lib/terminfo/c/name where name is the name of the terminal,
 * and c is the first character of name.  Thus, att4425 can be found in the
 * file /usr/share/lib/terminfo/a/att4425.  Synonyms for the same terminal
 * are implemented by multiple links to the same compiled file.
 */

static SLCONST char *Terminfo_Dirs [] =
{
   "", /* $TERMINFO */
   "", /* $HOME/.terminfo */
#ifdef MISC_TERMINFO_DIRS
   MISC_TERMINFO_DIRS,
#endif
   "/usr/local/etc/terminfo",
   "/usr/local/share/terminfo",
   "/usr/local/lib/terminfo",
   "/etc/terminfo",
   "/usr/share/terminfo",
   "/usr/lib/terminfo",
   "/usr/share/lib/terminfo",
   "/lib/terminfo",
   NULL,
};

void _pSLtt_tifreeent (SLterminfo_Type *t)
{
   if (t == NULL)
     return;

   SLfree ((char *)t->string_table);
   SLfree ((char *)t->string_offsets);
   SLfree ((char *)t->numbers);
   SLfree ((char *)t->boolean_flags);
   SLfree ((char *)t->terminal_names);
   SLfree ((char *)t);
}

SLterminfo_Type *_pSLtt_tigetent (SLCONST char *term)
{
   SLCONST char **tidirs, *tidir;
   FILE *fp = NULL;
   char file[1024];
   static char home_ti [1024];
   char *env;
   SLterminfo_Type *ti;

   if (
       (term == NULL)
#ifdef SLANG_UNTIC
       && (SLang_Untic_Terminfo_File == NULL)
#endif
       )
     return NULL;

   if (_pSLsecure_issetugid ()
       && ((term[0] == '.') || (NULL != strchr (term, '/'))))
     return NULL;

   if (NULL == (ti = (SLterminfo_Type *) SLmalloc (sizeof (SLterminfo_Type))))
     {
	return NULL;
     }
   memset ((char *)ti, 0, sizeof (SLterminfo_Type));

#ifdef SLANG_UNTIC
   if (SLang_Untic_Terminfo_File != NULL)
     {
	fp = open_terminfo (SLang_Untic_Terminfo_File, ti);
	goto fp_open_label;
     }
   else
#endif
   /* If we are on a termcap based system, use termcap */
   if (0 == tcap_getent (term, ti)) return ti;

   if (NULL != (env = _pSLsecure_getenv ("TERMINFO")))
     Terminfo_Dirs[0] = env;

   if (NULL != (env = _pSLsecure_getenv ("HOME")))
     {
	strncpy (home_ti, env, sizeof (home_ti) - 11);
	home_ti [sizeof(home_ti) - 11] = 0;
	strcat (home_ti, "/.terminfo");
	Terminfo_Dirs [1] = home_ti;
     }

   tidirs = Terminfo_Dirs;
   while (NULL != (tidir = *tidirs++))
     {
	if (*tidir == 0)
	  continue;

	if (sizeof (file) > strlen (tidir) + 5 + strlen (term))
	  {
	     sprintf (file, "%s/%c/%s", tidir, *term, term);
	     if (NULL != (fp = open_terminfo (file, ti)))
	       break;
	     sprintf (file, "%s/%02x/%s", tidir, (unsigned char)*term, term);
	     if (NULL != (fp = open_terminfo (file, ti)))
	       break;
	  }
     }

#ifdef SLANG_UNTIC
   fp_open_label:
#endif

   if (fp == NULL)
     {
	SLfree ((char *) ti);
	return NULL;
     }

   ti->flags = SLTERMINFO;
   if ((NULL == read_terminal_names (fp, ti))
       || (NULL == read_boolean_flags (fp, ti))
       || (NULL == read_numbers (fp, ti))
       || (NULL == read_string_offsets (fp, ti))
       || (NULL == read_string_table (fp, ti)))
     {
	_pSLtt_tifreeent (ti);
	ti = NULL;
     }
   (void) fclose (fp);
   return ti;
}

#ifdef SLANG_UNTIC
# define UNTIC_COMMENT(x) ,x
#else
# define UNTIC_COMMENT(x)
#endif

typedef SLCONST struct
{
   char name[3];
   int offset;
#ifdef SLANG_UNTIC
   char *comment;
#endif
}
Tgetstr_Map_Type;

#include "terminfo.inc"

static int compute_cap_offset (SLCONST char *cap, SLterminfo_Type *t, Tgetstr_Map_Type *map, unsigned int max_ofs)
{
   char cha, chb;

   (void) t;
   cha = *cap++; chb = *cap;

   while (*map->name != 0)
     {
	if ((cha == *map->name) && (chb == *(map->name + 1)))
	  {
	     if (map->offset >= (int) max_ofs) return -1;
	     return map->offset;
	  }
	map++;
     }
   return -1;
}

char *_pSLtt_tigetstr (SLterminfo_Type *t, SLCONST char *cap)
{
   int offset;

   if (t == NULL)
     return NULL;

   if (t->flags == SLTERMCAP) return tcap_getstr (cap, t);

   offset = compute_cap_offset (cap, t, Tgetstr_Map, t->num_string_offsets);
   if (offset < 0) return NULL;
   offset = make_integer (t->string_offsets + 2 * offset);
   if (offset < 0) return NULL;
   return t->string_table + offset;
}

int _pSLtt_tigetnum (SLterminfo_Type *t, SLCONST char *cap)
{
   int offset;

   if (t == NULL)
     return -1;

   if (t->flags == SLTERMCAP) return tcap_getnum (cap, t);

   offset = compute_cap_offset (cap, t, Tgetnum_Map, t->num_numbers);
   if (offset < 0) return -1;
   return make_integer (t->numbers + 2 * offset);
}

int _pSLtt_tigetflag (SLterminfo_Type *t, SLCONST char *cap)
{
   int offset;

   if (t == NULL) return -1;

   if (t->flags == SLTERMCAP) return tcap_getflag (cap, t);

   offset = compute_cap_offset (cap, t, Tgetflag_Map, t->boolean_section_size);

   if (offset < 0) return -1;
   return (int) *(t->boolean_flags + offset);
}

/* These are my termcap routines.  They only work with the TERMCAP environment
 * variable.  This variable must contain the termcap entry and NOT the file.
 */

static int tcap_getflag (SLCONST char *cap, SLterminfo_Type *t)
{
   char a, b;
   char *f = (char *) t->boolean_flags;
   char *fmax;

   if (f == NULL) return 0;
   fmax = f + t->boolean_section_size;

   a = *cap;
   b = *(cap + 1);
   while (f < fmax)
     {
	if ((a == f[0]) && (b == f[1]))
	  return 1;
	f += 2;
     }
   return 0;
}

static char *tcap_get_cap (unsigned char *cap, unsigned char *caps, unsigned int len)
{
   unsigned char c0, c1;
   unsigned char *caps_max;

   c0 = cap[0];
   c1 = cap[1];

   if (caps == NULL) return NULL;
   caps_max = caps + len;
   while (caps < caps_max)
     {
	if ((c0 == caps[0]) && (c1 == caps[1]))
	  {
	     return (char *) caps + 3;
	  }
	caps += (int) caps[2];
     }
   return NULL;
}

static int tcap_getnum (SLCONST char *cap, SLterminfo_Type *t)
{
   cap = tcap_get_cap ((unsigned char *) cap, t->numbers, t->num_numbers);
   if (cap == NULL) return -1;
   return atoi (cap);
}

static char *tcap_getstr (SLCONST char *cap, SLterminfo_Type *t)
{
   return tcap_get_cap ((unsigned char *) cap, (unsigned char *) t->string_table, t->string_table_size);
}

static int tcap_extract_field (unsigned char *t0)
{
   register unsigned char ch, *t = t0;
   while (((ch = *t) != 0) && (ch != ':')) t++;
   if (ch == ':') return (int) (t - t0);
   return -1;
}

int SLtt_Try_Termcap = 1;
static int tcap_getent (SLCONST char *term, SLterminfo_Type *ti)
{
   unsigned char *termcap, ch;
   unsigned char *buf, *b;
   unsigned char *t;
   int len;

   if (SLtt_Try_Termcap == 0) return -1;
#if 1
   /* XFREE86 xterm sets the TERMCAP environment variable to an invalid
    * value.  Specifically, it lacks the tc= string.
    */
   if (!strncmp (term, "xterm", 5))
     return -1;
#endif
   termcap = (unsigned char *) getenv ("TERMCAP");
   if ((termcap == NULL) || (*termcap == '/')) return -1;

   /* SUN Solaris 7&8 have bug in tset program under tcsh,
    * eval `tset -s -A -Q` sets value of TERMCAP to ":",
    *  under other shells it works fine.
    *  SUN was informed, they marked it as duplicate of bug 4086585
    *  but didn't care to fix it... <mikkopa@cs.tut.fi>
    */
   if ((termcap[0] == ':') && (termcap[1] == 0))
     return -1;

   /* We have a termcap so lets use it provided it does not have a reference
    * to another terminal via tc=.  In that case, use terminfo.  The alternative
    * would be to parse the termcap file which I do not want to do right now.
    * Besides, this is a terminfo based system and if the termcap were parsed
    * terminfo would almost never get a chance to run.  In addition, the tc=
    * thing should not occur if tset is used to set the termcap entry.
    */
   t = termcap;
   while ((len = tcap_extract_field (t)) != -1)
     {
	if ((len > 3) && (t[0] == 't') && (t[1] == 'c') && (t[2] == '='))
	  return -1;
	t += (len + 1);
     }

   /* malloc some extra space just in case it is needed. */
   len = strlen ((char *) termcap) + 256;
   if (NULL == (buf = (unsigned char *) SLmalloc ((unsigned int) len)))
     return -1;

   b = buf;

   /* The beginning of the termcap entry contains the names of the entry.
    * It is terminated by a colon.
    */

   ti->terminal_names = (char *) b;
   t = termcap;
   len = tcap_extract_field (t);
   if (len < 0)
     {
	SLfree ((char *)buf);
	return -1;
     }
   strncpy ((char *) b, (char *) t, (unsigned int) len);
   b[len] = 0;
   b += len + 1;
   ti->name_section_size = len;

   /* Now, we are really at the start of the termcap entries.  Point the
    * termcap variable here since we want to refer to this a number of times.
    */
   termcap = t + (len + 1);

   /* Process strings first. */
   ti->string_table = (char *) b;
   t = termcap;
   while (-1 != (len = tcap_extract_field (t)))
     {
	unsigned char *b1;
	unsigned char *tmax;

	/* We are looking for: XX=something */
	if ((len < 4) || (t[2] != '=') || (*t == '.'))
	  {
	     t += len + 1;
	     continue;
	  }
	tmax = t + len;
	b1 = b;

	while (t < tmax)
	  {
	     ch = *t++;
	     if ((ch == '\\') && (t < tmax))
	       {
		  SLwchar_Type wch;

		  t = (unsigned char *) _pSLexpand_escaped_char ((char *) t, (char *) tmax, &wch, NULL);
		  if (t == NULL)
		    {
		       SLfree ((char *)buf);
		       return -1;
		    }
		  ch = (char) wch;
	       }
	     else if ((ch == '^') && (t < tmax))
	       {
		  ch = *t++;
		  if (ch == '?') ch = 127;
		  else ch = (ch | 0x20) - ('a' - 1);
	       }
	     *b++ = ch;
	  }
	/* Null terminate it. */
	*b++ = 0;
	len = (int) (b - b1);
	b1[2] = (unsigned char) len;    /* replace the = by the length */
	/* skip colon to next field. */
	t++;
     }
   ti->string_table_size = (int) (b - (unsigned char *) ti->string_table);

   /* Now process the numbers. */

   t = termcap;
   ti->numbers = b;
   while (-1 != (len = tcap_extract_field (t)))
     {
	unsigned char *b1;
	unsigned char *tmax;

	/* We are looking for: XX#NUMBER */
	if ((len < 4) || (t[2] != '#') || (*t == '.'))
	  {
	     t += len + 1;
	     continue;
	  }
	tmax = t + len;
	b1 = b;

	while (t < tmax)
	  {
	     *b++ = *t++;
	  }
	/* Null terminate it. */
	*b++ = 0;
	len = (int) (b - b1);
	b1[2] = (unsigned char) len;    /* replace the # by the length */
	t++;
     }
   ti->num_numbers = (int) (b - ti->numbers);

   /* Now process the flags. */
   t = termcap;
   ti->boolean_flags = b;
   while (-1 != (len = tcap_extract_field (t)))
     {
	/* We are looking for: XX#NUMBER */
	if ((len != 2) || (*t == '.') || (*t <= ' '))
	  {
	     t += len + 1;
	     continue;
	  }
	b[0] = t[0];
	b[1] = t[1];
	t += 3;
	b += 2;
     }
   ti->boolean_section_size = (int) (b - ti->boolean_flags);
   ti->flags = SLTERMCAP;
   return 0;
}

/* These routines are provided only for backward binary compatability.
 * They will vanish in V2.x
 */
char *SLtt_tigetent (SLFUTURE_CONST char *s)
{
   return (char *) _pSLtt_tigetent (s);
}

char *SLtt_tigetstr (SLFUTURE_CONST char *s, char **p)
{
   if (p == NULL)
     return NULL;
   return _pSLtt_tigetstr ((SLterminfo_Type *) *p, s);
}

extern int SLtt_tigetnum (SLFUTURE_CONST char *s, char **p)
{
   if (p == NULL)
     return -1;
   return _pSLtt_tigetnum ((SLterminfo_Type *) *p, s);
}

