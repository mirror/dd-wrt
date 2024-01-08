/*
 * ej.c
 *
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */

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

static char *uqstrchr(char *buf, char find)
{
	int q = 0;
	char val;
	while ((val = *buf)) {
		if (val == '"')
			q ^= 1;
		else if ((val == find) && (!q))
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
	char *target = &haystack[haylen];
	for (cur = haystack, q = 0; cur < target && !(!q && !strncmp(needle, cur, needlelen)); cur++) {
		if (*cur == '"')
			q ? q-- : q++;
	}
	return (cur < target) ? cur : NULL;
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
	for (arg = args; isspace((int)*arg) || *arg == '"'; arg++)
		;
	for (*end-- = '\0'; isspace((int)*end) || *end == '"'; end--)
		*end = '\0';

	return arg;
}

static void *call_ej(char *name, void *handle, webs_t wp, int argc, char_t **argv);

static void *call(void *handle, char *func,
		  webs_t stream) //jimmy, https, 8/4/2003
{
	char *args, *end, *next;
	int argc;
	char *argv[16];

	/* Parse out ( args ) */
	if (!(args = strchr(func, '(')) || !(end = uqstrchr(func, ')')))
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

size_t websWrite(webs_t wp, char *fmt, ...);

static inline int decompress(webs_t stream, char *pattern, int len, int last)
{
	struct DECODE {
		char src;
		char *dst;
	};

	struct DECODE decode[] = {
		{ 'i', "<input type=" }, //
		{ 'c', "<input class=\"spaceradio\"" }, //
		{ 't', "<input class=\"text\"" }, //
		{ 'p', "<input class=\"num\"" }, //
		{ 'h', "<input class=\"button\"" }, //
		{ 'g', "<input class=\\\"button\\\"" }, //
		{ 'd', "<input id=" }, //
		{ 'f', "<div class=\"setting\"" }, //
		{ 'e', "<div class=" }, //
		{ 'n', "<div id=" }, //
		{ 'j', "<a href=\"" }, //
		{ 'o', "<option value=" }, //
		{ 's', "<select name=" }, //
		{ 'u', "<span class=" }, //
		{ 'z', "<input name=" }, //
		{ 'x', "document.write(\"" }, //
		{ 'y', "<document." }, //
		{ 'm', "<script type=\"text/javascript\">" }, //
	};
	int i;
	int l = sizeof(decode) / sizeof(struct DECODE);
	switch (len) {
	case 1:
		return 1;
		break;
	case 2:
		for (i = 0; i < l; i++) {
			if (pattern[1] == decode[i].src)
				return i + 1;
		}
		break;
	case 3:
		if (last && pattern[2] == '}') {
			websWrite(stream, decode[last - 1].dst);
			return 1;
		}
		break;
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

static int buffer_get(webs_t wp)
{
	unsigned char c;
	if (wp->s_filecount >= wp->s_filelen)
		return EOF;
	c = wp->s_filebuffer[wp->s_filecount++];
	return c;
}

#ifdef HAVE_MICRO
static int file_get(webs_t wp)
{
	if (wp->s_filecount >= wp->s_filelen)
		return EOF;
	wp->s_filecount++;
	return getc(wp->s_fp);
}
#endif
int wfputs(char *buf, webs_t fp);

static void *global_handle = NULL;
static void do_ej_s(int (*get)(webs_t wp),
		    webs_t stream) // jimmy, https, 8/4/2003
{
	int c = 0, ret = 0;
	char *pattern, *asp = NULL, *func = NULL, *end = NULL;
	int len = 0;
	memdebug_enter();
	pattern = (char *)safe_malloc(PATTERN_BUFFER + 1);
	while ((c = get(stream)) != EOF) {
		/* Add to pattern space */
		pattern[len++] = c;
		pattern[len] = '\0';
		if (len == (PATTERN_BUFFER - 1))
			goto release;

		if (!asp) {
			char pat = pattern[0];
			if (pat == '{') {
				ret = decompress(stream, pattern, len, ret);
				if (ret) {
					if (len == 3) {
						len = 0;
					}
					continue;
				}
			}
			/* Look for <% ... */
			if (pat == 0x3c) {
				if (len == 1)
					continue;
				if (pattern[1] == 0x25) {
					asp = pattern + 2;
					continue;
				}
			}
			pat = pattern[len - 1];
			if (pat == '{' || pat == 0x3c) {
				pattern[len - 1] = '\0';
				wfputs(pattern,
				       stream); //jimmy, https, 8/4/2003
				pattern[0] = pat;
				len = 1;
			}
			continue;
		} else {
			if (unqstrstr(asp, "%>")) {
				for (func = asp; func < &pattern[len]; func = end) {
					/* Skip initial whitespace */
					for (; isspace((int)*func); func++)
						;
					if (!(end = uqstrchr(func, ';')))
						break;
					*end++ = '\0';
					/* Call function */
					webs clone;
					memcpy(&clone, stream, sizeof(webs));
					global_handle = call(global_handle, func, &clone);
					// restore pointers
				}
				asp = NULL;
				len = 0;
			}
			continue;
		}

release:
		/* Release pattern space */
		wfputs(pattern, stream); //jimmy, https, 8/4/2003
		len = 0;
	}
	if (len)
		wfputs(pattern, stream); //jimmy, https, 8/4/2003

#ifndef MEMLEAK_OVERRIDE
	if (global_handle)
		dlclose(global_handle);
	global_handle = NULL;
#endif
	debug_free(pattern);
	memdebug_leave();
}

static void do_ej_buffer(char *buffer, webs_t stream)
{
	stream->s_filecount = 0;
	stream->s_filelen = strlen(buffer);
	stream->s_filebuffer = (unsigned char *)buffer;
	do_ej_s(&buffer_get, stream);
}

static void do_ej_file(FILE *fp, int len, webs_t stream)
{
#ifndef HAVE_MICRO
	stream->s_filebuffer = (unsigned char *)malloc(len);
	stream->s_filelen = len;
	stream->s_filecount = 0;
	fread(stream->s_filebuffer, 1, len, fp);
	do_ej_s(&buffer_get, stream);
	debug_free(stream->s_filebuffer);
#else
	stream->s_fp = fp;
	stream->s_filecount = 0;
	stream->s_filelen = len;
	do_ej_s(&file_get, stream);
#endif
}

#define WEBS_PAGE_ROM

#include "../html.c"

FILE *_getWebsFile(webs_t wp, char *path2, size_t *len)
{
	FILE *web;

	char *path = strdup(path2);
	char *query = strchr(path, '?');
	if (query)
		*query++ = 0;
	//      fprintf(stderr, "open %s\n", path);
	cprintf("opening %s\n", path);
	int i = 0;
	size_t curoffset = 0;
	size_t sensitive = -1;
	size_t insensitive = -1;
	size_t sensitive_len;
	size_t insensitive_len;
	int found = 0;
	int found2 = 0;
	while (websRomPageIndex[i].path != NULL) {
		*len = websRomPageIndex[i].size - WEBSOFFSET;
		if (!found && (endswith(path, ".asp") || endswith(path, ".htm") || endswith(path, ".html"))) {
			found = !strcasecmp(websRomPageIndex[i].path, path);
			if (found) {
				insensitive_len = *len;
				insensitive = curoffset;
			}
		}
		if (!found2) {
			found2 = !strcmp(websRomPageIndex[i].path, path);
			if (found2) {
				sensitive_len = *len;
				sensitive = curoffset;
			}
		}
		curoffset += *len;
		i++;
	}
	if (found || found2) {
		/* to prevent stack overwrite problems */
		web = fopen("/tmp/debug/www", "rb");
		if (!web)
			web = fopen("/etc/www", "rb");
		if (web == NULL)
			goto err;
		if (sensitive != -1) {
			fseek(web, sensitive, SEEK_SET);
			*len = sensitive_len;
		} else {
			fseek(web, insensitive, SEEK_SET);
			*len = insensitive_len;
		}
		debug_free(path);
		return web;
	}

err:
	*len = 0;
	web = fopen(path, "rb");
	if (web == NULL)
		goto err2;
	fseek(web, 0, SEEK_END);
	*len = ftell(web);
	fseek(web, 0, SEEK_SET);
err2:
	debug_free(path);
	return web;
}

FILE *getWebsFile(webs_t wp, char *path2)
{
	size_t len;
	return _getWebsFile(wp, path2, &len);
}

size_t getWebsFileLen(webs_t wp, char *path2)
{
	size_t len = 0;
	FILE *fp = _getWebsFile(wp, path2, &len);
	if (fp)
		fclose(fp);
	return len;
}

static void send_headers(webs_t conn_fp, int status, char *title, char *extra_header, char *mime_type, int length,
			 char *attach_file, int nocache);

int do_ej(unsigned char method, struct mime_handler *handler, char *path,
	  webs_t stream) // jimmy, https, 8/4/2003
{
	FILE *fp = NULL;
	size_t len;
	int i;
	memdebug_enter();
	i = 0;
	len = 0;
	fp = _getWebsFile(stream, path, &len);
	if (fp) {
		if (handler && !handler->send_headers)
			send_headers(stream, 200, "OK", handler->extra_header, handler->mime_type, -1, NULL, 1);
		stream->path = path;
		do_ej_file(fp, len, stream);
		fclose(fp);
		return 0;
	} else
		return -1;

	memdebug_leave_info(path);
}
