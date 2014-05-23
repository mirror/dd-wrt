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

#define SERVICE_MODULE "/lib/services.so"
static int stops_running = 0;
void *load_service(char *name)
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

static void _RELEASESTOPPED(char *method, char *name)
{
	char fname[64];
	sprintf(fname, "/tmp/services/%s.%s", name, method);
	unlink(fname);
}

static int _STOPPED(char *method,char *name)
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

#define STOPPED() if (_STOPPED(method, name)) return;
#define RELEASESTOPPED(a) _RELEASESTOPPED(a, name);

static int handle_service(char *method,char *name)
{
	if (!strcmp(method,"start"))
	    RELEASESTOPPED("stop");
	if (!strcmp(method,"stop")) {
	    stops_running++;
	    RELEASESTOPPED("start");
	}
	STOPPED();
	// lcdmessaged("Starting Service",name);
	cprintf("start_service\n");
	char service[64];

	sprintf(service, "/etc/config/%s", name);
	FILE *ck = fopen(service, "rb");

	if (ck != NULL) {
		fclose(ck);
		cprintf("found shell based service %s\n", service);
		return system(service);
	}
	void *handle = load_service(name);

	if (handle == NULL) {
		return -1;
	}
	void (*fptr) (void);

	sprintf(service, "start_%s", name);
	cprintf("resolving %s\n", service);
	fptr = (void (*)(void))dlsym(handle, service);
	if (fptr)
		(*fptr) ();
	else
		fprintf(stderr, "function %s not found \n", service);
	dlclose(handle);
	if (!strcmp(method,"stop")) {
	    stops_running--;
	}
	cprintf("start_sevice done()\n");
	return 0;
}



int start_service(char *name)
{
	handle_service("start",name);
	return 0;
}


int start_service_f(char *name)
{
	FORK(handle_service("start",name));
	return 0;
}

void *start_service_nofree(char *name, void *handle)
{
	handle_service("start",name);
	return handle;
}

void *start_service_nofree_f(char *name, void *handle)
{
	FORK(start_service(name));
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
	FORK(start_main(name, argc, argv));
}


int stop_running(void)
{
	return stops_running > 0;
}

int stop_running_main(int argc, char **argv)
{
	int dead = 0;
	while (stop_running() && dead < 50) {
		usleep(100 * 1000);
		dead++;
	}
	if (dead == 50) {
		fprintf(stderr, "stopping processes taking too long!!!\n");
	}
}

void stop_service(char *name)
{
	handle_service("stop", name);
}

void stop_service_f(char *name)
{
	FORK(handle_service("stop", name));
}

void *stop_service_nofree(char *name, void *handle)
{
	handle_service("stop", name);
	return handle;
}

void *stop_service_nofree_f(char *name, void *handle)
{
	FORK(handle_service("stop", name));
	return handle;
}

void startstop(char *name)
{
	void *handle = NULL;

	cprintf("stop and start service\n");
	handle = stop_service_nofree(name, handle);
	handle = start_service_nofree(name, handle);
	if (handle)
		dlclose(handle);
}

void startstop_f(char *name)
{
	FORK(startstop(name));
}

int startstop_main(int argc, char **argv)
{
	startstop(argv[1]);
	return 0;
}

int startstop_main_f(int argc, char **argv)
{
	char *name = argv[1];
	FORK(startstop_main(argc, argv));
	return 0;
}

void *startstop_nofree(char *name, void *handle)
{
	cprintf("stop and start service (nofree)\n");
	stop_service(name);
	start_service(name);
	return handle;
}

void *startstop_nofree_f(char *name, void *handle)
{
	FORK(startstop(name));
	return handle;
}
