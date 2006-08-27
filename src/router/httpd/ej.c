
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <bcmnvram.h>
#include <shutils.h>
#include "httpd.h"


//#define CDEBUG 1
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

#define LOG(a)			//fprintf(stderr,"%s\n",a);


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



void
do_ej_buffer (char *buffer, webs_t stream)	// jimmy, https, 8/4/2003
{

  int c;
  char *pattern, *asp = NULL, *func = NULL, *end = NULL;
  int len = 0;
  char *filebuffer;
  int filecount = 0;

  cdebug ("do_ej_buffer2 entry");
  if (buffer == NULL)
    return;
  filebuffer = buffer;
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
//      LOG("look start");

      if (!asp)
	{
	  if (!strncmp (pattern, "<i>", len))
	    {
	      if (len == 3)
		{
		  websWrite (stream, "<input type=");
		  len = 0;
		}
	      continue;
	    }
	  if (!strncmp (pattern, "<c>", len))
	    {
	      if (len == 3)
		{
		  websWrite (stream, "<input class=");
		  len = 0;
		}
	      continue;
	    }
	  if (!strncmp (pattern, "<d>", len))
	    {
	      if (len == 3)
		{
		  websWrite (stream, "<input id=");
		  len = 0;
		}
	      continue;
	    }
	  if (!strncmp (pattern, "<e>", len))
	    {
	      if (len == 3)
		{
		  websWrite (stream, "<div class=");
		  len = 0;
		}
	      continue;
	    }
	  if (!strncmp (pattern, "<n>", len))
	    {
	      if (len == 3)
		{
		  websWrite (stream, "<div id=");
		  len = 0;
		}
	      continue;
	    }
	  if (!strncmp (pattern, "<j>", len))
	    {
	      if (len == 3)
		{
		  websWrite (stream, "<a href=\"");
		  len = 0;
		}
	      continue;
	    }
	  if (!strncmp (pattern, "<o>", len))
	    {
	      if (len == 3)
		{
		  websWrite (stream, "<option value=");
		  len = 0;
		}
	      continue;
	    }
	  if (!strncmp (pattern, "<s>", len))
	    {
	      if (len == 3)
		{
		  websWrite (stream, "<select name=");
		  len = 0;
		}
	      continue;
	    }
	  if (!strncmp (pattern, "<u>", len))
	    {
	      if (len == 3)
		{
		  websWrite (stream, "<span class=");
		  len = 0;
		}
	      continue;
	    }
	  if (!strncmp (pattern, "<p>", len))
	    {
	      if (len == 3)
		{
		  websWrite (stream, "<input name=");
		  len = 0;
		}
	      continue;
	    }
	  if (!strncmp (pattern, "<m>", len))
	    {
	      if (len == 3)
		{
		  websWrite (stream, "<script type=\"text/javascript\">");
		  len = 0;
		}
	      continue;
	    }
	}
      if (!asp && !strncmp (pattern, "<%", len))
	{
	  if (len == 2)
	    asp = pattern + 2;
	  continue;
	}

      /* Look for ... %> */
//      LOG("look end");
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
		  LOG ("exec");
		  LOG (func);
		  cdebug (func);
		  //("Call %s\n",func);
		  call (func, stream);
		  //cprintf("Return %s okay\n",func);
		  cdebug (func);
		  LOG ("return");
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
  cdebug ("do_ej_buffer2 leave");
}


#ifdef HAVE_VFS
#include <vfs.h>
#endif

#define WEBS_PAGE_ROM

#include "html.c"

/*
#include <LzmaDecode.h>


#define LZMA_LC 2
#define LZMA_LP 0
#define LZMA_PB 0

#define LZMA_WORKSPACE_SIZE ((LZMA_BASE_SIZE + \
      (LZMA_LIT_SIZE << (LZMA_LC + LZMA_LP))) * sizeof(CProb))
*/

/*
char *decodeWebs(websRomPageIndexType *web)
{
unsigned char lzma_workspace[LZMA_WORKSPACE_SIZE];
int bytes;
char *buf;
int len = web->size;
buf = malloc(len);
LzmaDecode(lzma_workspace, LZMA_WORKSPACE_SIZE, LZMA_LC, LZMA_LP, LZMA_PB, web->page, web->csize, buf, len, &bytes);
return buf;
}
*/
char *
getWebsFile (char *path)
{
  char *buf = NULL;
  int i = 0;
  while (websRomPageIndex[i].path != NULL)
    {
      if (!strcmp (websRomPageIndex[i].path, path))
	{
	  buf = websRomPageIndex[i].page;
	  break;
	}
      i++;
    }

  return buf;
}

int
getWebsFileLen (char *path)
{
  int len = 0;
  int i = 0;
  while (websRomPageIndex[i].path != NULL)
    {
      if (!strcmp (websRomPageIndex[i].path, path))
	{
	  len = websRomPageIndex[i].size;
	  break;
	}
      i++;
    }
  return len;
}

void
do_ej (char *path, webs_t stream)	// jimmy, https, 8/4/2003
{

//open file and read into memory

  char *buffer = NULL;
#ifdef HAVE_VFS
  entry *e;
#endif
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
      i = 0;
      len = 0;
      while (websRomPageIndex[i].path != NULL)
	{
//fprintf(stderr,"try to find %s from %s\n",path,websRomPageIndex[i].path);
	  if (!strcmp (websRomPageIndex[i].path, path))
	    {
	      buffer = websRomPageIndex[i].page;
	      len = websRomPageIndex[i].size;
	      break;
	    }
	  i++;
	}
      int le = 0;
      if (buffer == NULL)
	{
	  le = 1;
	  FILE *in = fopen (path, "rb");
	  if (in == NULL)
	    return;
	  fseek (in, 0, SEEK_END);
	  len = ftell (in);
	  rewind (in);
	  buffer = (char *) malloc (len + 1);
	  fread (buffer, 1, len, in);
	  buffer[len] = 0;
	  fclose (in);
	}
      /*if (x)
         {
         for (i = 0;i<len;i++)
         buffer[i]^='d';
         char b;
         for (i = 0;i<len/2;i++)
         {
         b = buffer[i];
         buffer[i]=buffer[(len-1)-i];
         buffer[(len-1)-i]=b;
         }
         } */


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
//fprintf(stderr,"do_ej_buffer\n");

  do_ej_buffer (buffer, stream);
//fprintf(stderr,"do_ej_buffer done\n");
  if (le)
    free (buffer);
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
