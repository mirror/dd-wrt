
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <dlfcn.h>
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
static char *get_arg(char *args, char **next);
//static void call(char *func, FILE *stream);
static void *call(void *handle, char *func, webs_t stream);
#define PATTERN_BUFFER 1000

#define LOG(a)			//fprintf(stderr,"%s\n",a);

char *uqstrchr(char *buf, char find)
{
	int q = 0;
	while (*buf) {
		if (*buf == '"')
			q ^= 1;
		else if ((*buf == find) && (!q))
			return buf;
		++buf;
	}
	return NULL;
}

/* Look for unquoted character within a string */
static char *unqstrstr(char *haystack, char *needle)
{
	char *cur;
	int q;
	int needlelen = strlen(needle);
	int haylen = strlen(haystack);
	for (cur = haystack, q = 0;
	     cur < &haystack[haylen] && !(!q
					  && !strncmp(needle, cur, needlelen));
	     cur++) {
		if (*cur == '"')
			q ? q-- : q++;
	}
	return (cur < &haystack[haylen]) ? cur : NULL;
}

static char *get_arg(char *args, char **next)
{
	char *arg, *end;

	/* Parse out arg, ... */
	if (!(end = uqstrchr(args, ','))) {
		end = args + strlen(args);
		*next = NULL;
	} else
		*next = end + 1;

	/* Skip whitespace and quotation marks on either end of arg */
	for (arg = args; isspace((int)*arg) || *arg == '"'; arg++) ;
	for (*end-- = '\0'; isspace((int)*end) || *end == '"'; end--)
		*end = '\0';

	return arg;
}

static void *call(void *handle, char *func, webs_t stream)	//jimmy, https, 8/4/2003
{
	char *args, *end, *next;
	int argc;
	char *argv[16];

	/* Parse out ( args ) */
	if (!(args = strchr(func, '(')))
		return handle;
	if (!(end = uqstrchr(func, ')')))
		return handle;
	*args++ = *end = '\0';

	/* Set up argv list */
	for (argc = 0; argc < 16 && args; argc++, args = next) {
		if (!(argv[argc] = get_arg(args, &next)))
			break;
	}

	/* Call handler */
	return call_ej(func, handle, stream, argc, argv);
}

static int decompress(webs_t stream, char *pattern, int len)
{

	if (!strncmp(pattern, "{i}", len)) {
		if (len == 3) {
			websWrite(stream, "<input type=");
		}
		return 1;
	}
	if (!strncmp(pattern, "{c}", len)) {
		if (len == 3) {
			websWrite(stream, "<input class=");
		}
		return 1;
	}
	if (!strncmp(pattern, "{d}", len)) {
		if (len == 3) {
			websWrite(stream, "<input id=");
		}
		return 1;
	}
	if (!strncmp(pattern, "{e}", len)) {
		if (len == 3) {
			websWrite(stream, "<div class=");
		}
		return 1;
	}
	if (!strncmp(pattern, "{n}", len)) {
		if (len == 3) {
			websWrite(stream, "<div id=");
		}
		return 1;
	}
	if (!strncmp(pattern, "{j}", len)) {
		if (len == 3) {
			websWrite(stream, "<a href=\"");
		}
		return 1;
	}
	if (!strncmp(pattern, "{o}", len)) {
		if (len == 3) {
			websWrite(stream, "<option value=");
		}
		return 1;
	}
	if (!strncmp(pattern, "{s}", len)) {
		if (len == 3) {
			websWrite(stream, "<select name=");
		}
		return 1;
	}
	if (!strncmp(pattern, "{u}", len)) {
		if (len == 3) {
			websWrite(stream, "<span class=");
		}
		return 1;
	}
	if (!strncmp(pattern, "{z}", len)) {
		if (len == 3) {
			websWrite(stream, "<input name=");
		}
		return 1;
	}
	if (!strncmp(pattern, "{x}", len)) {
		if (len == 3) {
			websWrite(stream, "document.write(\"");
		}
		return 1;
	}
	if (!strncmp(pattern, "{y}", len)) {
		if (len == 3) {
			websWrite(stream, "<document.");
		}
		return 1;
	}
	if (!strncmp(pattern, "{m}", len)) {
		if (len == 3) {
			websWrite(stream, "<script type=\"text/javascript\">");
		}
		return 1;
	}
	return 0;
}

void do_ej_file(FILE * fp, int filelen, webs_t stream)	// jimmy, https, 8/4/2003
{
	void *handle = NULL;
	int c, ret;
	char *pattern, *asp = NULL, *func = NULL, *end = NULL;
	int len = 0;
	int filecount = 0;

	pattern = (char *)malloc(PATTERN_BUFFER + 1);
	while (((c = getc(fp)) != EOF) && filecount < filelen) {
		filecount++;
		/* Add to pattern space */
		pattern[len++] = c;
		pattern[len] = '\0';
		if (len == (PATTERN_BUFFER - 1))
			goto release;

		/* Look for <% ... */
//      LOG("look start");

		if (!asp && pattern[0] == '{') {
			ret = decompress(stream, pattern, len);
			if (ret && len == 3) {
				len = 0;
				continue;
			}
			if (ret)
				continue;
		}
		if (!asp && !strncmp(pattern, "<%", len)) {
			if (len == 2)
				asp = pattern + 2;
			continue;
		}

		/* Look for ... %> */
//      LOG("look end");
		if (asp) {
			if (unqstrstr(asp, "%>")) {
				for (func = asp; func < &pattern[len];
				     func = end) {
					/* Skip initial whitespace */
					for (; isspace((int)*func); func++) ;
					if (!(end = uqstrchr(func, ';')))
						break;
					*end++ = '\0';

					/* Call function */
					handle = call(handle, func, stream);
				}
				asp = NULL;
				len = 0;
			}
			continue;
		}

	      release:
		/* Release pattern space */
		//fputs(pattern, stream);
		wfputs(pattern, stream);	//jimmy, https, 8/4/2003
		len = 0;
	}
	free(pattern);
	if (handle)
		dlclose(handle);
}

void do_ej_buffer(char *buffer, webs_t stream)	// jimmy, https, 8/4/2003
{
	void *handle = NULL;
	int c, ret;
	char *pattern, *asp = NULL, *func = NULL, *end = NULL;
	int len = 0;
	char *filebuffer;
	int filecount = 0;

	if (buffer == NULL)
		return;
	filebuffer = buffer;
	pattern = (char *)malloc(PATTERN_BUFFER + 1);
	while ((c = filebuffer[filecount++]) != 0) {

		/* Add to pattern space */
		pattern[len++] = c;
		pattern[len] = '\0';
		if (len == (PATTERN_BUFFER - 1))
			goto release;

		/* Look for <% ... */
//      LOG("look start");

		if (!asp && pattern[0] == '{') {
			ret = decompress(stream, pattern, len);
			if (ret && len == 3) {
				len = 0;
				continue;
			}
			if (ret)
				continue;
		}
		if (!asp && !strncmp(pattern, "<%", len)) {
			if (len == 2)
				asp = pattern + 2;
			continue;
		}

		/* Look for ... %> */
//      LOG("look end");
		if (asp) {
			if (unqstrstr(asp, "%>")) {
				for (func = asp; func < &pattern[len];
				     func = end) {
					/* Skip initial whitespace */
					for (; isspace((int)*func); func++) ;
					if (!(end = uqstrchr(func, ';')))
						break;
					*end++ = '\0';

					/* Call function */
					handle = call(handle, func, stream);
				}
				asp = NULL;
				len = 0;
			}
			continue;
		}

	      release:
		/* Release pattern space */
		//fputs(pattern, stream);
		wfputs(pattern, stream);	//jimmy, https, 8/4/2003
		len = 0;
	}
	free(pattern);
	if (handle)
		dlclose(handle);
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
FILE *getWebsFile(char *path)
{
	cprintf("opening %s\n", path);
	int i = 0;
	while (websRomPageIndex[i].path != NULL) {
		if (!strcmp(websRomPageIndex[i].path, path)) {
			FILE *web = fopen("/etc/www", "rb");
			fseek(web, websRomPageIndex[i].offset, 0);
			cprintf("found %s\n", path);
			return web;
		}
		i++;
	}
	cprintf("not found %s\n", path);

	return NULL;
}

int getWebsFileLen(char *path)
{
	int len = 0;
	int i = 0;
/*char tmpfile[64];
sprintf(tmpfile,"/tmp/%s",path);
//fprintf(stderr,"read %s\n",path);
FILE *web = fopen(tmpfile,"rb");
if (web!=NULL)
    {
    fseek(web,0,SEEK_END);
    len=ftell(web);
    fclose(web);
    return len;
    }*/
	while (websRomPageIndex[i].path != NULL) {
		if (!strcmp(websRomPageIndex[i].path, path)) {
			len = websRomPageIndex[i].size;
			break;
		}
		i++;
	}
	return len;
}

void do_ej(struct mime_handler *handler, char *path, webs_t stream, char *query)	// jimmy, https, 8/4/2003
{
//fprintf(stderr,"load page %s\n",path);
//open file and read into memory
	FILE *fp = NULL;
#ifdef HAVE_VFS
	entry *e;
#endif
	int len;
	int i;
#ifdef DEBUGLOG
	if (log == NULL)
		log = fopen("/tmp/log.tmp", "wb");
#endif

	i = 0;
	len = 0;
	while (websRomPageIndex[i].path != NULL) {
		if (!strcmp(websRomPageIndex[i].path, path)) {
			fp = fopen("/etc/www", "rb");
			fseek(fp, websRomPageIndex[i].offset, SEEK_SET);
			len = websRomPageIndex[i].size;
			break;
		}
		i++;
	}
	if (fp == NULL) {
		fp = fopen(path, "rb");
		if (fp == NULL)
			return;
		fseek(fp, 0, SEEK_END);
		len = ftell(fp);
		rewind(fp);
		do_ej_file(fp, len, stream);
	} else {
		do_ej_file(fp, len, stream);
	}
	fclose(fp);

}

int ejArgs(int argc, char **argv, char *fmt, ...)
{
	va_list ap;
	int arg;
	char *c;

	if (!argv)
		return 0;

	va_start(ap, fmt);
	for (arg = 0, c = fmt; c && *c && arg < argc;) {
		if (*c++ != '%')
			continue;
		switch (*c) {
		case 'd':
			*(va_arg(ap, int *)) = atoi(argv[arg]);
			break;
		case 's':
			*(va_arg(ap, char **)) = argv[arg];
			break;
		}
		arg++;
	}
	va_end(ap);

	return arg;
}
