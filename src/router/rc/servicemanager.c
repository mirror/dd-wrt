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

//#define DEBUG() if (nvram_match("service_debug","1")) fprintf(stderr,"%s: calling %s\n",__func__,name);
#define DEBUG(a)

#define SERVICE_MODULE "/lib/services.so"
static int *stops_running = NULL;

static void init_shared(void)
{
	if (!stops_running)
		stops_running = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

}

void *load_service(const char *name)
{
	cprintf("load service %s\n", name);
	void *handle = dlopen(SERVICE_MODULE, RTLD_LAZY);

	cprintf("done()\n");
	if (handle == NULL && name != NULL) {
		cprintf("not found, try to load alternate\n");
		char dl[64];

		sprintf(dl, "/usr/lib/%s_service.so", name);
		cprintf("try to load %s\n", dl);
		handle = dlopen(dl, RTLD_LAZY);
		if (handle == NULL) {
			fprintf(stderr, "cannot load %s\n", dl);
			return NULL;
		}
	}
	cprintf("found it, returning handle\n");
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
		return 1;
	}
	fp = fopen(fname, "wb");
	if (fp) {
		fputs("s", fp);
		fclose(fp);
	}
	return 0;
}

#define STOPPED() if (_STOPPED(method, name)) { \
		    if (nvram_match("service_debug","1")) \
			    fprintf(stderr,"calling %s_%s not required!\n",method,name); \
		    if (!strcmp(method, "stop")) { \
			if (stops_running) \
				stops_running[0]--; \
		    } \
		     return 0; \
		    }

#define RELEASESTOPPED(a) _RELEASESTOPPED(a, name);

static int handle_service(const char *method, const char *name)
{

	if (!strcmp(method, "start"))
		RELEASESTOPPED("stop");
	if (!strcmp(method, "stop")) {
		RELEASESTOPPED("start");
	}
	STOPPED();

	if (nvram_match("service_debug", "1"))
		fprintf(stderr, "calling %s_%s\n", method, name);
	// lcdmessaged("Starting Service",name);
	cprintf("start_service\n");
	char service[64];

	sprintf(service, "/etc/config/%s", name);
	FILE *ck = fopen(service, "rb");

	if (ck != NULL) {
		fclose(ck);
		cprintf("found shell based service %s\n", service);
		return sysprintf("%s %s", service, method);
	}
	void *handle = load_service(name);


	if (handle == NULL) {
		return -1;
	}
	void (*fptr) (void);

	sprintf(service, "%s_%s", method, name);
	cprintf("resolving %s\n", service);
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
	if (nvram_match("service_debug", "1")) {
		if (stops_running)
			fprintf(stderr, "calling done %s_%s (pending stops %d)\n", method, name, stops_running[0]);
		else
			fprintf(stderr, "calling done %s_%s\n", method, name);
	}
	cprintf("start_sevice done()\n");
	return 0;
}

int start_service(char *name)
{
	DEBUG();
	handle_service("start", name);
	return 0;
}

int start_service_force(char *name)
{
	DEBUG();
	RELEASESTOPPED("start");
	return start_service(name);
}

int start_service_f(char *name)
{
	DEBUG();
	FORK(start_service(name));
	return 0;
}

int start_service_force_f(char *name)
{
	DEBUG();
	FORK(start_service_force(name));
	return 0;
}

void *start_service_nofree_delay(char *name, void *handle, int delay)
{
	DEBUG();
	if (delay)
		sleep(delay);
	handle_service("start", name);
	return handle;
}

void *start_service_nofree(char *name, void *handle)
{
	return start_service_nofree_delay(name, handle, 0);
}

void *start_service_nofree_force(char *name, void *handle)
{
	DEBUG();
	RELEASESTOPPED("start");
	return start_service_nofree(name, handle);
}

void *start_service_nofree_f(char *name, void *handle)
{
	DEBUG();
	FORK(start_service_nofree(name, handle));
	return handle;
}

void *start_service_nofree_fdelay(char *name, void *handle, int delay)
{
	DEBUG();
	FORK(start_service_nofree_delay(name, handle, delay));
	return handle;
}

void *start_service_nofree_force_f(char *name, void *handle)
{
	DEBUG();
	FORK(start_service_nofree_force(name, handle));
	return handle;
}

void start_servicei(char *name, int param)
{
	// lcdmessaged("Starting Service",name);
	cprintf("start_servicei\n");
	void *handle = load_service(name);

	if (handle == NULL) {
		return;
	}
	void (*fptr) (int);
	char service[64];

	sprintf(service, "start_%s", name);
	cprintf("resolving %s\n", service);
	fptr = (void (*)(int))dlsym(handle, service);
	if (fptr)
		(*fptr) (param);
	else
		fprintf(stderr, "function %s not found \n", service);
	dlclose(handle);
	cprintf("start_sevicei done()\n");
	return;
}

void start_servicei_f(char *name, int param)
{
	DEBUG();
	FORK(start_servicei(name, param));
}

void start_main(char *name, int argc, char **argv)
{
	cprintf("start_main\n");
	void *handle = load_service(name);

	if (handle == NULL) {
		return;
	}
	int (*fptr) (int, char **);
	char service[64];

	sprintf(service, "%s_main", name);
	cprintf("resolving %s\n", service);
	fptr = (int (*)(int, char **))dlsym(handle, service);
	if (fptr)
		(*fptr) (argc, argv);
	else
		fprintf(stderr, "function %s not found \n", service);
	dlclose(handle);
	cprintf("start_main done()\n");
	return;
}

void start_main_f(char *name, int argc, char **argv)
{
	DEBUG();
	FORK(start_main(name, argc, argv));
}

int stop_running(void)
{
	return stops_running[0] > 0;
}

int stop_running_main(int argc, char **argv)
{
	int dead = 0;
	while (stops_running != NULL && stop_running() && dead < 100) {
		if (nvram_match("service_debugrunnings","1"))
			fprintf(stderr,"%s: dead: %d running %d\n",__func__,dead,stops_running[0]);
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

void stop_service(char *name)
{
	DEBUG();
	init_shared();
	if (stops_running)
		stops_running[0]++;
	handle_service("stop", name);
}

void stop_service_force(char *name)
{
	DEBUG();
	RELEASESTOPPED("stop");
	stop_service(name);
}

void stop_service_f(char *name)
{
	DEBUG();
	init_shared();
	if (stops_running)
		stops_running[0]++;
	FORK(handle_service("stop", name));
}

void stop_service_force_f(char *name)
{
	DEBUG();
	RELEASESTOPPED("stop");
	init_shared();
	if (stops_running)
		stops_running[0]++;
	RELEASESTOPPED("stop");
	FORK(handle_service("stop", name));
}

void *stop_service_nofree(char *name, void *handle)
{
	DEBUG();
	init_shared();
	if (stops_running)
		stops_running[0]++;
	handle_service("stop", name);
	return handle;
}

void *stop_service_nofree_force(char *name, void *handle)
{
	DEBUG();
	RELEASESTOPPED("stop");
	return stop_service_nofree(name, handle);
}

void *stop_service_nofree_f(char *name, void *handle)
{
	DEBUG();
	init_shared();
	if (stops_running)
		stops_running[0]++;
	FORK(handle_service("stop", name));
	return handle;
}

void *stop_service_nofree_force_f(char *name, void *handle)
{
	DEBUG();
	init_shared();
	if (stops_running)
		stops_running[0]++;
	RELEASESTOPPED("stop");
	FORK(handle_service("stop", name));
	return handle;
}

static void startstop_delay(char *name, int delay)
{
	void *handle = NULL;
	DEBUG();
	if (delay)
		sleep(delay);
	RELEASESTOPPED("stop");
	RELEASESTOPPED("start");
	cprintf("stop and start service\n");
	handle_service("stop", name);
	handle_service("start", name);
	if (handle)
		dlclose(handle);
}

void startstop(char *name)
{
	init_shared();
	if (stops_running)
		stops_running[0]++;
	startstop_delay(name, 0);
}

void startstop_fdelay(char *name, int delay)
{
	DEBUG();
	init_shared();
	if (stops_running)
		stops_running[0]++;
	FORK(startstop_delay(name, 0));
}

void startstop_f(char *name)
{
	startstop_fdelay(name, 0);
}

int startstop_main(int argc, char **argv)
{
	startstop(argv[1]);
	return 0;
}

int startstop_main_f(int argc, char **argv)
{
	char *name = argv[1];
	RELEASESTOPPED("stop");
	RELEASESTOPPED("start");
	init_shared();
	if (stops_running)
		stops_running[0]++;
	FORK(startstop_delay(argc, argv));
	return 0;
}

static void *startstop_nofree_delay(char *name, void *handle, int delay)
{
	cprintf("stop and start service (nofree)\n");
	DEBUG();
	if (delay)
		sleep(delay);
	RELEASESTOPPED("stop");
	RELEASESTOPPED("start");
	handle_service("stop", name);
	handle_service("start", name);
	return handle;
}

void *startstop_nofree(char *name, void *handle)
{
	init_shared();
	if (stops_running)
		stops_running[0]++;
	return startstop_nofree_delay(name, handle, 0);
}

void *startstop_nofree_f(char *name, void *handle)
{
	DEBUG();
	init_shared();
	if (stops_running)
		stops_running[0]++;
	FORK(startstop_nofree_delay(name, handle, 0));
	return handle;
}

void *startstop_nofree_fdelay(char *name, void *handle, int delay)
{
	DEBUG();
	init_shared();
	if (stops_running)
		stops_running[0]++;
	FORK(startstop_nofree_delay(name, handle, delay));
	return handle;
}
