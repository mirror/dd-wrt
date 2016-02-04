/* 
 * DD-WRT servicemanager.c
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

#include <dlfcn.h>
#include <stdio.h>
#include <shutils.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <bcmnvram.h>

#define SERVICE_MODULE "/lib/services.so"
static int *stops_running = NULL;

static void init_shared(void)
{
	if (!stops_running)
		stops_running = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

}

static void *load_service(const char *name)
{
	void *handle = dlopen(SERVICE_MODULE, RTLD_LAZY);
	if (!handle) {
		fprintf(stderr, "Cannot open library: %s", dlerror());
	}

	if (handle == NULL && name != NULL) {
		char dl[64];

		sprintf(dl, "/usr/lib/%s_service.so", name);
		handle = dlopen(dl, RTLD_LAZY);
		if (handle == NULL) {
			fprintf(stderr, "cannot load %s\n", dl);
			return NULL;
		}
	}
	return handle;
}

static void _RELEASESTOPPED(const char *method, const char *name)
{
	char fname[64];
	sprintf(fname, "/tmp/services/%s.%s", name, method);
	unlink(fname);
}

static int _STOPPED(const char *method, const char *name)
{
	char fname[64];
	sprintf(fname, "/tmp/services/%s.%s", name, method);
	FILE *fp = fopen(fname, "rb");
	if (fp) {
		fclose(fp);
#ifdef HAVE_X86
		if (!strcmp(method, "stop")) {
			if (stops_running)
				stops_running[0]--;
		}
#else
		if (nvram_match("service_debug", "1"))
			fprintf(stderr, "calling %s_%s not required!\n", method, name);
		if (!strcmp(method, "stop")) {
			if (stops_running)
				stops_running[0]--;
		}
#endif

		return 1;
	}
	fp = fopen(fname, "wb");
	if (fp) {
		fputs("s", fp);
		fclose(fp);
	}
	return 0;
}

static void _stopcondition(char *method, char *name)
{

}

#define STOPPED() if (_STOPPED(method, name)) { \
		     return 0; \
		    }

#define RELEASESTOPPED(a) _RELEASESTOPPED(a, name);

static int handle_service(const char *method, const char *name)
{

	if (strcmp(name, "hotplug_block")) {
		if (!strcmp(method, "start"))
			RELEASESTOPPED("stop");
		if (!strcmp(method, "stop"))
			RELEASESTOPPED("start");
		STOPPED();
	}
#ifndef HAVE_X86
	if (nvram_match("service_debug", "1"))
		fprintf(stderr, "calling %s_%s\n", method, name);
#endif
	// lcdmessaged("Starting Service",name);
	char service[64];

	sprintf(service, "/etc/config/%s", name);
	FILE *ck = fopen(service, "rb");

	if (ck != NULL) {
		fclose(ck);
		return sysprintf("%s %s", service, method);
	}
	void *handle = load_service(name);

	if (handle == NULL) {
		return -1;
	}
	void (*fptr) (void);

	sprintf(service, "%s_%s", method, name);
	fptr = (void (*)(void))dlsym(handle, service);
	if (fptr)
		(*fptr) ();
	else
		fprintf(stderr, "function %s not found \n", service);
	dlclose(handle);
	if (!strcmp(method, "stop")) {
		if (stops_running)
			stops_running[0]--;
	}
#ifndef HAVE_X86
	if (nvram_match("service_debug", "1")) {
		if (stops_running)
			fprintf(stderr, "calling done %s_%s (pending stops %d)\n", method, name, stops_running[0]);
		else
			fprintf(stderr, "calling done %s_%s\n", method, name);
	}
#endif
	return 0;
}

static void start_service(char *name)
{
	handle_service("start", name);
}

static void start_service_force(char *name)
{
	RELEASESTOPPED("start");
	start_service(name);
}

static void start_service_f(char *name)
{
	FORK(start_service(name));
}

static void start_service_force_f(char *name)
{
	FORK(start_service_force(name));
}

static void start_servicei(char *name, int param)
{
	// lcdmessaged("Starting Service",name);
	void *handle = load_service(name);

	if (handle == NULL) {
		return;
	}
	void (*fptr) (int);
	char service[64];

	sprintf(service, "start_%s", name);
	fptr = (void (*)(int))dlsym(handle, service);
	if (fptr)
		(*fptr) (param);
	else
		fprintf(stderr, "function %s not found \n", service);
	dlclose(handle);
	return;
}

static void start_servicei_f(char *name, int param)
{
	FORK(start_servicei(name, param));
}

static int start_main(char *name, int argc, char **argv)
{
	int ret = -1;
	void *handle = load_service(name);

	if (handle == NULL) {
		return -1;
	}
	int (*fptr) (int, char **);
	char service[64];

	sprintf(service, "%s_main", name);
	fptr = (int (*)(int, char **))dlsym(handle, service);
	if (fptr)
		ret = (*fptr) (argc, argv);
	else
		fprintf(stderr, "function %s not found \n", service);
	dlclose(handle);
	return ret;
}

static void start_main_f(char *name, int argc, char **argv)
{
	FORK(start_main(name, argc, argv));
}

static int stop_running(void)
{
	return stops_running[0] > 0;
}

static int stop_running_main(int argc, char **argv)
{
	int dead = 0;
	while (stops_running != NULL && stop_running() && dead < 100) {
#ifndef HAVE_X86
		if (nvram_match("service_debugrunnings", "1"))
			fprintf(stderr, "%s: dead: %d running %d\n", __func__, dead, stops_running[0]);
#endif
		if (dead == 0)
			fprintf(stderr, "waiting for services to finish (%d)...\n", stops_running[0]);
		usleep(100 * 1000);
		dead++;
	}
	if (dead == 50) {
		fprintf(stderr, "stopping processes taking too long!!!\n");
	} else if (stops_running != NULL && stops_running[0] == 0) {
		int *run = stops_running;
		stops_running = NULL;
		munmap(run, sizeof(int));
	}
	return 0;
}

static void stop_service(char *name)
{
	init_shared();
	if (stops_running)
		stops_running[0]++;
	handle_service("stop", name);
}

static void stop_service_force(char *name)
{
	RELEASESTOPPED("stop");
	stop_service(name);
}

static void stop_service_f(char *name)
{
	init_shared();
	if (stops_running)
		stops_running[0]++;
	FORK(handle_service("stop", name));
}

static void stop_service_force_f(char *name)
{
	RELEASESTOPPED("stop");
	init_shared();
	if (stops_running)
		stops_running[0]++;
	RELEASESTOPPED("stop");
	FORK(handle_service("stop", name));
}

static void startstop_delay(char *name, int delay)
{
	void *handle = NULL;
	if (delay)
		sleep(delay);
	RELEASESTOPPED("stop");
	RELEASESTOPPED("start");
	handle_service("stop", name);
	handle_service("start", name);
	if (handle)
		dlclose(handle);
}

static void startstop(char *name)
{
	init_shared();
	if (stops_running)
		stops_running[0]++;
	startstop_delay(name, 0);
}

static void startstop_fdelay(char *name, int delay)
{
	init_shared();
	if (stops_running)
		stops_running[0]++;
	FORK(startstop_delay(name, delay));
}

static void startstop_f(char *name)
{
	startstop_fdelay(name, 0);
}

static int startstop_main(int argc, char **argv)
{
	startstop(argv[1]);
	return 0;
}

static int startstop_main_f(int argc, char **argv)
{
	char *name = argv[1];
	RELEASESTOPPED("stop");
	RELEASESTOPPED("start");
	init_shared();
	if (stops_running)
		stops_running[0]++;
	FORK(startstop_delay(name, 0));
	return 0;
}
