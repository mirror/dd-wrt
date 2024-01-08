/*
 * DD-WRT callvalidate.c
 *
 * Copyright (C) 2005 - 2006 Sebastian Gottschall <sebastian.gottschall@newmedia-net.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License
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
#ifdef WEBS
#include <uemf.h>
#include <ej.h>
#else /* !WEBS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <httpd.h>
#include <errno.h>
#endif /* WEBS */

#include <proto/ethernet.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#include <dd_defs.h>
#include <cy_conf.h>
// #ifdef EZC_SUPPORT
#include <ezc.h>
// #endif
#include <broadcom.h>
#include <wlutils.h>
#include <netdb.h>
#include <utils.h>

#include <dlfcn.h>
#include <stdio.h>
// #include <shutils.h>

static char *path_modules[] = { "/jffs/usr/lib", "/tmp/debug", "/usr/lib", "/lib", NULL };

#define cprintf(fmt, args...)

#ifndef cprintf
#define cprintf(fmt, args...)                                                     \
	do {                                                                      \
		FILE *fp = fopen("/dev/console", "w");                            \
		if (fp) {                                                         \
			fprintf(fp, "%s (%d):%s ", __FILE__, __LINE__, __func__); \
			fprintf(fp, fmt, ##args);                                 \
			fclose(fp);                                               \
		}                                                                 \
	} while (0)
#endif
size_t websWrite(webs_t wp, char *fmt, ...);
size_t vwebsWrite(webs_t wp, char *fmt, va_list args);
char *websGetVar(webs_t wp, char *var, char *d)
{
	return get_cgi(wp, var) ?: d;
}

char *websGetSaneVar(webs_t wp, char *var, char *d)
{
	char *sanevar = websGetVar(wp, var, d);
	if (d && sanevar && !*var)
		sanevar = d;
	return sanevar;
}

int websGetVari(webs_t wp, char *var, int d)
{
	char *res = get_cgi(wp, var);
	return res ? atoi(res) : d;
}

char *GOZILA_GET(webs_t wp, char *name)
{
	if (!name)
		return NULL;
	if (!wp)
		return nvram_safe_get(name);
	return wp->gozila_action ? websGetVar(wp, name, NULL) : nvram_safe_get(name);
}

void *openlib(char *type)
{
	int i = 0;
	void *handle;
	char *lib;
	while ((lib = path_modules[i++])) {
		char name[64];
		snprintf(name, sizeof(name) - 1, "%s/%s.so", lib, type);
		handle = dlopen(name, RTLD_LAZY | RTLD_GLOBAL);
		if (handle)
			return handle;
	}
	char *err = dlerror();
	dd_logerror("httpd", "Cannot load %s/%s.so library (%s)\n", path_modules[i - 1], type, err ? err : "unknown");
	return NULL;
}

#define load_visual_service(name) openlib("visuals")
#define load_service(name) openlib("validate")

extern const websRomPageIndexType websRomPageIndex[];

char *live_translate(webs_t wp, const char *tran);
void validate_cgi(webs_t wp);

static void *s_service = NULL;

static void start_gozila(char *name, webs_t wp)
{
	// lcdmessaged("Starting Service",name);
	cprintf("start_gozila %s\n", name);
	char service[64];
	if (!s_service) {
		s_service = load_service(name);
	}
	if (s_service == NULL) {
		return;
	}

	int (*fptr)(webs_t wp);

	snprintf(service, sizeof(service), "%s", name);
	dd_logdebug("httpd", "Start gozila %s\n", service);
	cprintf("resolving %s\n", service);
	fptr = (int (*)(webs_t wp))dlsym(s_service, service);
	if (fptr)
		(*fptr)(wp);
	else
		dd_logdebug("httpd", "Function %s not found \n", service);
#ifndef MEMLEAK_OVERRIDE
	dlclose(s_service);
	s_service = NULL;
#endif
	cprintf("start_sevice done()\n");
}

static int start_validator(char *name, webs_t wp, char *value, struct variable *v)
{
	// lcdmessaged("Starting Service",name);
	cprintf("start_validator %s\n", name);
	char service[64];
	if (!s_service) {
		s_service = load_service(name);
	}
	if (s_service == NULL) {
		return FALSE;
	}
	int ret = FALSE;

	int (*fptr)(webs_t wp, char *value, struct variable *v);

	snprintf(service, sizeof(service), "%s", name);
	dd_logdebug("httpd", "Start validator %s\n", service);
	cprintf("resolving %s\n", service);
	fptr = (int (*)(webs_t wp, char *value, struct variable *v))dlsym(s_service, service);
	if (fptr)
		ret = (*fptr)(wp, value, v);
	else
		dd_logdebug("httpd", "Function %s not found \n", service);
#ifndef MEMLEAK_OVERRIDE
	dlclose(s_service);
	s_service = NULL;
#endif

	cprintf("start_sevice done()\n");
	return ret;
}

static void *start_validator_nofree(char *name, void *handle, webs_t wp, char *value, struct variable *v)
{
	// lcdmessaged("Starting Service",name);
	cprintf("start_service_nofree %s\n", name);
	char service[64];

	if (!handle) {
		handle = load_service(name);
	}
	if (handle == NULL) {
		return NULL;
	}
	void (*fptr)(webs_t wp, char *value, struct variable *v);

	snprintf(service, sizeof(service), "%s", name);
	cprintf("resolving %s\n", service);
	fptr = (void (*)(webs_t wp, char *value, struct variable *v))dlsym(handle, service);
	cprintf("found. pointer is %p\n", fptr);
	if (fptr)
		(*fptr)(wp, value, v);
	else
		dd_logdebug("httpd", "Function %s not found \n", service);
	cprintf("start_sevice_nofree done()\n");
	return handle;
}

static void *call_ej(char *name, void *handle, webs_t wp, int argc, char_t **argv)
{
	struct timeval before, after, r;

	if (nvram_matchi("httpd_debug", 1)) {
		dd_syslog(LOG_INFO, "%s:%s", __func__, name);
		fprintf(stderr, "call_ej %s", name);
		int i = 0;
		for (i = 0; i < argc; i++)
			fprintf(stderr, " %s", argv[i]);
	}
	char service[64];

	{
		memdebug_enter();
		if (!handle) {
			cprintf("load visual_service\n");
			handle = load_visual_service(name);
		}
		memdebug_leave_info("loadviz");
	}
	if (handle == NULL) {
		cprintf("handle null\n");
		return NULL;
	}
	cprintf("pointer init\n");
	void (*fptr)(webs_t wp, int argc, char_t **argv);

	snprintf(service, sizeof(service), "ej_%s", name);
	cprintf("resolving %s\n", service);
	fptr = (void (*)(webs_t wp, int argc, char_t **argv))dlsym(handle, service);

	cprintf("found. pointer is %p\n", fptr);
	{
		memdebug_enter();
		if (fptr) {
			gettimeofday(&before, NULL);
			(*fptr)(wp, argc, argv);
			gettimeofday(&after, NULL);
			timersub(&after, &before, &r);
			dd_logdebug("httpd", " %s duration %ld.%06ld\n", service, (long int)r.tv_sec, (long int)r.tv_usec);

		} else {
			char *err = dlerror();
			dd_logdebug("httpd", " function %s not found (%s)\n", service, err ? err : "unknown");
		}
		memdebug_leave_info(service);
	}
	cprintf("start_sevice_nofree done()\n");
	return handle;
}
