* *Tiny Embedded JavaScript parser * *Copyright 2001 - 2003, Broadcom Corporation * All Rights Reserved. * *THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.BROADCOM * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE. * *$Id: ej.c, v 1.5 2005 / 11 / 30 11: 55:05 seg Exp $
  *
  /
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <bcmnvram.h>
#include <shutils.h>
#include "httpd.h"
#ifdef CDEBUG
#include <utils.h>
#endif
#ifndef CDEBUG
#define cdebug(a)
#endif
static char *get_arg (char *args, char **next);
//static void call(char *func, FILE *stream);
static void call (char *func, webs_t stream);
#define PATTERN_BUFFER 1000

#ifndef HAVE_NOTRANS
static char *lastlanguage = NULL;
typedef struct
{
  char *original;
  char *translation;
} LANGUAGE;

//#define DEBUGLOG 1

#ifdef DEBUGLOG
FILE *log;
#define LOG(a) if (log!=NULL)fprintf(log,"%s\n",a); fflush(log);
#else
#define LOG(a)
#endif


static LANGUAGE **language = NULL;
static unsigned int langcount = 0;
static void
StringStart (FILE * in)
{
  int b = 0;
  while (b != '"')
    {
      b = getc (in);
      if (b == '\\')
	b = getc (in);
      if (feof (in))
	return;
    }
}

static char *
getFileString (FILE * in)
{
  char *buf;
  int i, b;
  buf = malloc (1024);
  StringStart (in);
  for (i = 0; i < 1024; i++)
    {
      b = getc (in);
      if (b == EOF)
	return NULL;
      if (b == '\\')
	{
	  buf[i] = getc (in);
	  continue;
	}
      if (b == '"')
	{
	  buf[i] = 0;
	  buf = realloc (buf, strlen (buf) + 1);
	  return buf;
	}
      buf[i] = b;
    }
  return buf;
}

#define LANG_PREFIX "/etc/langpack/"
#define LANG_POSTFIX ".lang"


void
initLanguage ()
{
  int i;
  char *fd;
  char *original;
  char *translate;
  char *langstr;
  LANGUAGE *desc;
  FILE *in;
//if (language!=NULL)return;
  langcount = 0;
  LOG ("langstr");
  langstr = nvram_safe_get ("language");
  if (langstr == NULL)
    return;
  if (language != NULL && lastlanguage != NULL)
    {
      if (strcmp (langstr, lastlanguage) != 0)
	{
	  lastlanguage = langstr;
	  for (i = 0; i < langcount; i++)
	    {
	      free (language[i]->original);
	      free (language[i]->translation);
	    }
	  langcount = 0;
	}
    }
  else if (lastlanguage == NULL)
    lastlanguage = langstr;

  LOG ("malloc");
  fd =
    (char *) malloc (strlen (LANG_PREFIX) + strlen (langstr) +
		     strlen (LANG_POSTFIX) + 1);
  sprintf (fd, "%s%s%s", LANG_PREFIX, langstr, LANG_POSTFIX);
  LOG ("open");
  LOG (fd);
  in = fopen (fd, "rb");
  if (in == NULL)
    {
      sprintf (fd, "/tmp/langpack/%s%s", langstr, LANG_POSTFIX);
      in = fopen (fd, "rb");
      if (in == NULL)
	{
	  free (fd);
	  return;
	}
    }
  free (fd);
  LOG ("read desc");
  while (1)
    {
      original = getFileString (in);
      if (original == NULL)
	break;
      translate = getFileString (in);
      desc = (LANGUAGE *) malloc (sizeof (LANGUAGE));
      LOG ("entry");
      LOG (original);
      LOG (translate);
      desc->original = original;
      desc->translation = translate;
//realloc space
      language =
	(LANGUAGE **) realloc (language,
			       sizeof (struct LANGUAGE **) * (langcount + 2));
      language[langcount++] = desc;
      language[langcount] = NULL;
    }
  fclose (in);

}

char *
translatePage (char *buffer)
{
  char *dest;
  long len;
  char *search;
  char *replace;
  int i, a, z, sl, rl;
  LOG ("translate");
  len = strlen (buffer);
  for (a = 0; a < langcount; a++)
    {
      search = language[a]->original;
      replace = language[a]->translation;
      sl = strlen (search);
      rl = strlen (replace);
      char *cp = strstr (buffer, search);
      if (cp == NULL)
	continue;
      int diff = rl - sf;
      if (diff > 0)
	{
	  buffer = remalloc (buffer, len + diff);	// remalloc space
	  for (i = len - 1; i == i * cp - *buffer; i--)	// align buffer
	    {
	      buffer[i] = buffer[i - diff];
	    }
	  memcpy (cp, replace, rl);
	}
      if (diff < 0)
	{
	  memcpy (cp, replace, rl);	// replace string
	  for (i = (*cp - *buffer) + rl; i < len; i++)	// align buffer
	    {
	      buffer[i] = buffer[i + sl];
	    }
	  buffer = remalloc (buffer, len + diff);	// remalloc space
	}
      if (diff == 0)
	{
	  memcpy (cp, replace, rl);
	}
      /*
         for (i=0;i<len-sl;i++)
         {
         for (z=0;z<sl;z++)
         {
         if (search[z]!=buffer[i+z])break;
         }
         if (z==sl)
         {
         //LOG("replace");
         //LOG(search);
         //LOG(replace);
         dest = (char*)malloc(len-sl+rl+1+PATTERN_BUFFER);
         for (z=0;z<i;z++)dest[z] = buffer[z];
         for (z=i;z<i+rl;z++) dest[z] = replace[z-i];
         for (z=i+sl;z<len;z++) dest[z-sl+rl] = buffer[z];
         dest[z-sl+rl]=0;
         free(buffer);
         buffer = dest;
         len=strlen(buffer);
         i-=sl; //fix for translation crashes
         i+=rl;
         }
         } */
    }
  LOG ("write");
  return buffer;
}
#else
#define LOG(a)
#endif


char *
uqstrchr (char *buf, char find)
{
  int q = 0;
  while (*buf)
    {
      if (*buf == '"')
	q ^= 1;
      else if ((*buf == find) && (!q))
	return buf;
      ++buf;
    }
  return NULL;
}

/* Look for unquoted character within a string */
static char *
unqstrstr (char *haystack, char *needle)
{
  char *cur;
  int q;

  for (cur = haystack, q = 0;
       cur < &haystack[strlen (haystack)] && !(!q
					       && !strncmp (needle, cur,
							    strlen (needle)));
       cur++)
    {
      if (*cur == '"')
	q ? q-- : q++;
    }
  return (cur < &haystack[strlen (haystack)]) ? cur : NULL;
}

static char *
get_arg (char *args, char **next)
{
  char *arg, *end;

  /* Parse out arg, ... */
  if (!(end = uqstrchr (args, ',')))
    {
      end = args + strlen (args);
      *next = NULL;
    }
  else
    *next = end + 1;

  /* Skip whitespace and quotation marks on either end of arg */
  for (arg = args; isspace ((int) *arg) || *arg == '"'; arg++);
  for (*end-- = '\0'; isspace ((int) *end) || *end == '"'; end--)
    *end = '\0';

  return arg;
}
static void
//call(char *func, FILE *stream)
call (char *func, webs_t stream)	//jimmy, https, 8/4/2003
{
  char *args, *end, *next;
  int argc;
  char *argv[16];
  struct ej_handler *handler;

  /* Parse out ( args ) */
  if (!(args = strchr (func, '(')))
    return;
  if (!(end = uqstrchr (func, ')')))
    return;
  *args++ = *end = '\0';

  /* Set up argv list */
  for (argc = 0; argc < 16 && args; argc++, args = next)
    {
      if (!(argv[argc] = get_arg (args, &next)))
	break;
    }

  /* Call handler */
  for (handler = &ej_handlers[0]; handler->pattern; handler++)
    {
      //if (strncmp(handler->pattern, func, strlen(handler->pattern)) == 0)
      if (strcmp (handler->pattern, func) == 0)
	handler->output (0, stream, argc, argv);
    }
}



static void
do_ej_buffer2 (char *buffer, webs_t stream)	// jimmy, https, 8/4/2003
{

  int c;
  char *pattern, *asp = NULL, *func = NULL, *end = NULL;
  int len = 0;
  int trans = 0;
  char *filebuffer;
  int filecount = 0;

  cdebug ("do_ej_buffer2 entry");
#ifndef HAVE_NOTRANS
  initLanguage ();
  if (language == NULL)
    {
      LOG ("no lang defined");
      if (buffer == NULL)
	return;
      filebuffer = buffer;
    }
  else
    {
      trans = 1;
      LOG ("translate");
      filebuffer = translatePage (buffer);
      if (filebuffer == NULL)
	filebuffer = "file not found!";
      LOG ("ready");
    }
#else
  if (buffer == NULL)
    return;
  filebuffer = buffer;
#endif
  LOG ("alloc");
  pattern = (char *) malloc (PATTERN_BUFFER + 1);
  LOG ("parse");
  LOG (filebuffer);
  while ((c = filebuffer[filecount++]) != 0)
    {

      /* Add to pattern space */
      pattern[len++] = c;
      pattern[len] = '\0';
      if (len == (PATTERN_BUFFER - 1))
	goto release;


      /* Look for <% ... */
      //LOG("look start");
      if (!asp && !strncmp (pattern, "<%", len))
	{
	  if (len == 2)
	    asp = pattern + 2;
	  continue;
	}

      /* Look for ... %> */
      //LOG("look end");
      if (asp)
	{
	  if (unqstrstr (asp, "%>"))
	    {
	      for (func = asp; func < &pattern[len]; func = end)
		{
		  /* Skip initial whitespace */
		  for (; isspace ((int) *func); func++);
		  if (!(end = uqstrchr (func, ';')))
		    break;
		  *end++ = '\0';

		  /* Call function */
//                                      LOG("exec");
//                                      LOG(func);
		  //      cdebug(func);
		  //      cprintf("Call %s\n",func);
		  call (func, stream);
		  //      cprintf("Return okay\n");
		  //      cdebug(func);
//                                      LOG("return");
		}
	      asp = NULL;
	      len = 0;
	    }
	  continue;
	}

    release:
      /* Release pattern space */
      //fputs(pattern, stream);
      wfputs (pattern, stream);	//jimmy, https, 8/4/2003
      len = 0;
    }
  free (pattern);
#ifndef HAVE_NOTRANS
  if (language != NULL)
    free (filebuffer);
#endif
  cdebug ("do_ej_buffer2 leave");
}

void
do_ej_buffer (char *buffer, webs_t stream)
{
  char *b;
  int i, len;
  cdebug ("do_ej_buffer entry");
  if (buffer == NULL)
    return;
#ifdef DEBUGLOG
  if (log == NULL)
    log = fopen ("/tmp/log.tmp", "wb");
#endif
  LOG ("copy stuff");
  len = strlen (buffer);
  b = malloc (len + 1);
  for (i = 0; i < len; i++)
    b[i] = buffer[i];
  b[len] = 0;
  LOG ("send to do_ej");
  do_ej_buffer2 (b, stream);
  LOG ("free buffer space");
#ifndef HAVE_NOTRANS
  if (language == NULL)
    free (b);
#else
  free (b);
#endif
#ifdef DEBUGLOG
  fclose (log);
  log = NULL;
#endif
  cdebug ("do_ej_buffer leave");

}

#ifdef HAVE_VFS
#include <vfs.h>
#endif
void
do_ej (char *path, webs_t stream)	// jimmy, https, 8/4/2003
{

//open file and read into memory

  char *buffer;
#ifdef HAVE_VFS
  entry *e;
#endif
  FILE *in;
  int len;
  int i;
  cdebug ("do_ej entry");
#ifdef DEBUGLOG
  if (log == NULL)
    log = fopen ("/tmp/log.tmp", "wb");
#endif

#ifdef HAVE_VFS
  e = vfsopen (path, "rb");
  if (e == NULL)
    {
#endif

      in = fopen (path, "rb");
      if (in == NULL)
	return;
      fseek (in, 0, SEEK_END);
      len = ftell (in);
      rewind (in);
      buffer = (char *) malloc (len + 1 + PATTERN_BUFFER);
      fread (buffer, 1, len, in);
      for (i = len; i < len + PATTERN_BUFFER; i++)
	buffer[i] = 0;
      fclose (in);
#ifdef HAVE_VFS
    }
  else
    {

      len = e->filelen;
      buffer = (char *) malloc (len + 1 + PATTERN_BUFFER);
      vfsread (buffer, 1, len, e);
      for (i = len; i < len + PATTERN_BUFFER; i++)
	buffer[i] = 0;
      vfsclose (e);

    }
#endif




//do_ej
  do_ej_buffer2 (buffer, stream);



#ifndef HAVE_NOTRANS
  if (language == NULL)
    free (buffer);
#else
  free (buffer);
#endif
  cdebug ("do_ej leave");
}

int
ejArgs (int argc, char **argv, char *fmt, ...)
{
  va_list ap;
  int arg;
  char *c;

  if (!argv)
    return 0;

  va_start (ap, fmt);
  for (arg = 0, c = fmt; c && *c && arg < argc;)
    {
      if (*c++ != '%')
	continue;
      switch (*c)
	{
	case 'd':
	  *(va_arg (ap, int *)) = atoi (argv[arg]);
	  break;
	case 's':
	  *(va_arg (ap, char **)) = argv[arg];
	  break;
	}
      arg++;
    }
  va_end (ap);

  return arg;
}
