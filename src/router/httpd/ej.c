
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <dlfcn.h>
#include <ctype.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <endian.h>
#include <unistd.h>
#include <stdint.h>

#include "httpd.h"

//#define CDEBUG 1
#ifdef CDEBUG
#include <utils.h>
#endif

#ifndef CDEBUG
#define cdebug(a)
#endif
static char *get_arg(char *args, char **next);
static void *call(void *handle, char *func, webs_t stream);
#define PATTERN_BUFFER 1000

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
	for (cur = haystack, q = 0; cur < &haystack[haylen] && !(!q && !strncmp(needle, cur, needlelen)); cur++) {
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
	struct DECODE {
		char *src;
		char *dst;
	};

	struct DECODE decode[] = {
		{"{i}", "<input type="},	//
		{"{c}", "<input class="},	//
		{"{d}", "<input id="},	//
		{"{e}", "<div class="},	//
		{"{n}", "<div id="},	//
		{"{j}", "<a href=\""},	//
		{"{o}", "<option value="},	//
		{"{s}", "<select name="},	//
		{"{u}", "<span class="},	//
		{"{z}", "<input name="},	//
		{"{x}", "document.write(\""},	//
		{"{y}", "<document."},	//
		{"{m}", "<script type=\"text/javascript\">"},	//
	};
	int i;
	int l = sizeof(decode) / sizeof(struct DECODE);
	for (i = 0; i < l; i++) {
		if (!strncmp(pattern, decode[i].src, len)) {
			if (len == 3) {
				websWrite(stream, decode[i].dst);
			}
			return 1;
		}

	}
	return 0;
}

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define PATTERN 0x253c
#elif __BYTE_ORDER == __BIG_ENDIAN
#define PATTERN 0x3c25
#else
#error "no endian type"
#endif

static FILE *s_fp;
static unsigned char *s_filebuffer;
static int s_filecount;
static int s_filelen;

static int buffer_get(void)
{
	unsigned char c;
	c = s_filebuffer[s_filecount++];
	if (!c)
		return EOF;
	return c;
}

static int file_get(void)
{
	if (s_filecount >= s_filelen)
		return EOF;
	s_filecount++;
	return getc(s_fp);
}

static void *global_handle = NULL;
static void do_ej_s(int (*get) (void), webs_t stream)	// jimmy, https, 8/4/2003
{
	int c, ret;
	char *pattern, *asp = NULL, *func = NULL, *end = NULL;
	int len = 0;
	memdebug_enter();
	FILE *backup_fp = s_fp;
	int backup_filecount = s_filecount;
	unsigned char *backup_filebuffer = s_filebuffer;
	unsigned int backup_filelen = s_filelen;

	pattern = (char *)safe_malloc(PATTERN_BUFFER + 1);
	while ((c = get()) != EOF) {
		/* Add to pattern space */
		pattern[len++] = c;
		pattern[len] = '\0';
		if (len == (PATTERN_BUFFER - 1))
			goto release;

		if (!asp) {
			if (pattern[0] == '{') {
				ret = decompress(stream, pattern, len);
				if (ret) {
					if (len == 3)
						len = 0;
					continue;
				}
			}
			/* Look for <% ... */
			if (pattern[0] == 0x3c) {
				if (len == 1)
					continue;
				if (pattern[1] == 0x25) {
					asp = pattern + 2;
					continue;
				}
			}
		} else {
			if (unqstrstr(asp, "%>")) {
				for (func = asp; func < &pattern[len]; func = end) {
					/* Skip initial whitespace */
					for (; isspace((int)*func); func++) ;
					if (!(end = uqstrchr(func, ';')))
						break;
					*end++ = '\0';
					/* Call function */
					backup_filecount = s_filecount;
					global_handle = call(global_handle, func, stream);
					// restore pointers
					s_fp = backup_fp;
					s_filebuffer = backup_filebuffer;
					s_filecount = backup_filecount;
					s_filelen = backup_filelen;
				}
				asp = NULL;
				len = 0;
			}
			continue;
		}

	      release:
		/* Release pattern space */
		wfputs(pattern, stream);	//jimmy, https, 8/4/2003
		len = 0;
	}

#ifndef MEMLEAK_OVERRIDE
	if (global_handle)
		dlclose(global_handle);
	global_handle = NULL
#endif
	    free(pattern);
	memdebug_leave();
}

void do_ej_buffer(char *buffer, webs_t stream)
{
	s_filecount = 0;
	s_filebuffer = (unsigned char *)buffer;
	do_ej_s(&buffer_get, stream);
}

void do_ej_file(FILE * fp, int len, webs_t stream)
{
	s_fp = fp;
	s_filecount = 0;
	s_filelen = len;
	do_ej_s(&file_get, stream);
}

#define WEBS_PAGE_ROM

#include "webs.h"
#include "html.c"

FILE *getWebsFile(char *path)
{
	static FILE *web = NULL;
	cprintf("opening %s\n", path);
	int i = 0;
	unsigned int curoffset = 0;
	while (websRomPageIndex[i].path != NULL) {

		if (!strcmp(websRomPageIndex[i].path, path)) {
			/* to prevent stack overwrite problems */
			web = fopen("/etc/www", "rb");
			if (web == NULL)
				return NULL;
			fseek(web, curoffset, SEEK_SET);
			cprintf("found %s\n", path);
			return web;
		}
		curoffset += (websRomPageIndex[i].size - WEBSOFFSET);
		i++;
	}
	cprintf("not found %s\n", path);

	return NULL;
}

int getWebsFileLen(char *path)
{
	unsigned int len = 0;
	int i = 0;
	while (websRomPageIndex[i].path != NULL) {
		if (!strcmp(websRomPageIndex[i].path, path)) {
			len = websRomPageIndex[i].size - WEBSOFFSET;
			break;
		}
		i++;
	}
	return len;
}

void do_ej(struct mime_handler *handler, char *path, webs_t stream, char *query)	// jimmy, https, 8/4/2003
{
	FILE *fp = NULL;
	unsigned int len;
	int i;
	memdebug_enter();

	i = 0;
	len = 0;
	unsigned int curoffset = 0;
	while (websRomPageIndex[i].path != NULL) {
		if (!strcmp(websRomPageIndex[i].path, path)) {
			fp = fopen("/etc/www", "rb");
			if (fp == NULL)
				return;
			fseek(fp, curoffset, SEEK_SET);
			len = websRomPageIndex[i].size - WEBSOFFSET;
			break;
		}
		curoffset += websRomPageIndex[i].size - WEBSOFFSET;
		i++;
	}
	if (fp == NULL) {
		fp = fopen(path, "rb");
		if (fp == NULL)
			return;
		fseek(fp, 0, SEEK_END);
		len = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		do_ej_file(fp, len, stream);
	} else {
		do_ej_file(fp, len, stream);
	}
	fclose(fp);
	memdebug_leave_info(path);

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
