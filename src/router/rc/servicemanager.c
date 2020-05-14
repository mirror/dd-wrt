/* 
 * DD-WRT servicemanager.c
 *
 * Copyright (C) 2005 - 2019 Sebastian Gottschall <sebastian.gottschall@newmedia-net.de>
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
		dd_logerror("servicemanager", "Cannot open library: %s", dlerror());
	}

	if (handle == NULL && name != NULL) {
		char dl[64];

		sprintf(dl, "/usr/lib/%s_service.so", name);
		handle = dlopen(dl, RTLD_LAZY);
		if (handle == NULL) {
			dd_logerror("servicemanager", "cannot load %s\n", dl);
			return NULL;
		}
	}
	return handle;
}

#define START 0x0
#define STOP 0x1
#define RESTART 0x2

static void _RELEASESTOPPED(const int method, const char *name)
{
	char fname[64];
	sprintf(fname, "/tmp/services/%s.%d", name, method);
	unlink(fname);
}

static int _STOPPED(const int method, const char *name)
{
	char fname[64];
	sprintf(fname, "/tmp/services/%s.%d", name, method);
	FILE *fp = fopen(fname, "rb");
	if (fp) {
		fclose(fp);
#if defined(HAVE_X86) || defined(HAVE_NEWPORT) || defined(HAVE_RB600) && !defined(HAVE_WDR4900)
		if (method == STOP) {
			if (stops_running)
				(*stops_running)--;
		}
#else
		dd_debug(DEBUG_SERVICE, "calling %s_%s not required!\n", method ? "stop" : "start", name);

		if (method == STOP) {
			if (stops_running)
				(*stops_running)--;
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

static void _stopcondition(const int method, char *name)
{

}

#define STOPPED() if (_STOPPED(method, name)) { \
		     return 0; \
		    }

#define RELEASESTOPPED(a) _RELEASESTOPPED(a, name);

static int handle_service(const int method, const char *name, int force)
{
	int ret = 0;
	char *method_name = "start";
	if (method == STOP)
		method_name = "stop";
	if (method == RESTART)
		method_name = "restart";

	if (strcmp(name, "hotplug_block") && method != RESTART) {
		if (method == START)
			RELEASESTOPPED(STOP);
		if (method == STOP)
			RELEASESTOPPED(START);
		STOPPED();
	}
#if (!defined(HAVE_X86) && !defined(HAVE_NEWPORT) && !defined(HAVE_RB600)) || defined(HAVE_WDR4900)
	dd_debug(DEBUG_SERVICE, "%s:%s_%s", __func__, method_name, name);
#endif
	// lcdmessaged("Starting Service",name);
	char service[64];

	sprintf(service, "/etc/config/%s", name);
	FILE *ck = fopen(service, "rb");

	if (ck != NULL) {
		fclose(ck);
		return sysprintf("%s %s", service, method_name);
	}
	void *handle = load_service(name);

	if (handle == NULL) {
		return -1;
	}
	void (*fptr)(void);

	sprintf(service, "%s_%s", method_name, name);
	fptr = (void (*)(void))dlsym(handle, service);
	char *deps = NULL;
	if (fptr) {
		int state = 1;
		if (method == START) {
			char dep_name[64];
			char proc_name[64];
			snprintf(dep_name, sizeof(dep_name), "%s_deps", name);
			char *(*dep_func)(void) = (char *(*)(void))dlsym(handle, dep_name);
			if (dep_func) {
				deps = dep_func();
				dd_debug(DEBUG_SERVICE, "%s exists, check nvram params %s\n", dep_name, deps);
				state = nvram_states(deps);
			}
			if (!state) {
				snprintf(proc_name, sizeof(proc_name), "%s_proc", name);
				char *(*proc_func)(void) = (char *(*)(void))dlsym(handle, proc_name);
				if (proc_func) {
					dd_debug(DEBUG_SERVICE, "%s exists, check process\n", proc_name);
					char *proc = proc_func();
					int pid = pidof(proc);
					dd_debug(DEBUG_SERVICE, "process name is %s, pid is %d\n", proc, pidof(proc));
					if (pid == -1)
						state = 1;
				}
			}

		}
		if (force || state) {
			(*fptr) ();
			if (deps)
				nvram_states(deps);
		}
	} else {
		dd_debug(DEBUG_SERVICE, "function %s not found \n", service);

		ret = -1;
	}
	dlclose(handle);
	if (method == STOP) {
		if (stops_running)
			(*stops_running)--;
	}
#if (!defined(HAVE_X86) && !defined(HAVE_NEWPORT) && !defined(HAVE_RB600)) || defined(HAVE_WDR4900)
	if (stops_running)
		dd_debug(DEBUG_SERVICE, "calling done %s_%s (pending stops %d)\n", method_name, name, *stops_running);
	else
		dd_debug(DEBUG_SERVICE, "calling done %s_%s\n", method_name, name);
#endif
	return ret;
}

static void start_service_arg(char *name, int force)
{
	handle_service(START, name, force);
}

static void start_service(char *name)
{
	start_service_arg(name, 0);
}

static void start_service_force_arg(char *name, int force)
{
	RELEASESTOPPED(START);
	start_service_arg(name, force);
}

static void start_service_force(char *name)
{
	start_service_force_arg(name, 0);
}

static void start_service_f_arg(char *name, int force)
{
	FORK(start_service_arg(name, force));
}

static void start_service_f(char *name)
{
	start_service_f_arg(name, 0);
}

static void start_service_force_f_arg(char *name, int force)
{
	FORK(start_service_force_arg(name, force));
}

static void start_service_force_f(char *name)
{
	start_service_force_f_arg(name, 0);
}

static void start_servicei(char *name, int param)
{
	// lcdmessaged("Starting Service",name);
	void *handle = load_service(name);

	if (handle == NULL) {
		return;
	}
	void (*fptr)(int);
	char service[64];

	sprintf(service, "start_%s", name);
	fptr = (void (*)(int))dlsym(handle, service);
	if (fptr)
		(*fptr) (param);
	else
		dd_logerror("servicemanager", "function %s not found \n", service);
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
	int (*fptr)(int, char **);
	char service[64];

	sprintf(service, "%s_main", name);
	fptr = (int (*)(int, char **))dlsym(handle, service);
	if (fptr)
		ret = (*fptr) (argc, argv);
	else
		dd_logerror("servicemanager", "function %s not found \n", service);
	dlclose(handle);
	return ret;
}

static void start_main_f(char *name, int argc, char **argv)
{
	FORK(start_main(name, argc, argv));
}

static int stop_running(void)
{
	return *stops_running > 0;
}

static int stop_running_main(int argc, char **argv)
{
	int dead = 0;
	while (stops_running != NULL && stop_running() && dead < 100) {
#if (!defined(HAVE_X86) && !defined(HAVE_NEWPORT) && !defined(HAVE_RB600)) || defined(HAVE_WDR4900)
		if (nvram_matchi("service_debugrunnings", 1))
			dd_loginfo("servicemanager", "%s: dead: %d running %d\n", __func__, dead, *stops_running);
#endif
		if (dead == 0)
			dd_loginfo("servicemanager", "waiting for services to finish (%d)...\n", *stops_running);
		usleep(100 * 1000);
		dead++;
	}
	if (dead == 50) {
		dd_logerror("servicemanager", "stopping processes taking too long!!!\n");
	} else if (stops_running != NULL && *stops_running == 0) {
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
		(*stops_running)++;
	handle_service(STOP, name, 0);
}

static void stop_service_force(char *name)
{
	RELEASESTOPPED(STOP);
	stop_service(name);
}

static void stop_service_f(char *name)
{
	init_shared();
	if (stops_running)
		(*stops_running)++;
	FORK(handle_service(STOP, name, 0));
}

static void stop_service_force_f(char *name)
{
	RELEASESTOPPED(STOP);
	init_shared();
	if (stops_running)
		(*stops_running)++;
	RELEASESTOPPED(STOP);
	FORK(handle_service(STOP, name, 0));
}

static void _restart_delay(char *name, int delay)
{
	if (delay)
		sleep(delay);
	if (handle_service(RESTART, name, 0)) {
		RELEASESTOPPED(STOP);
		RELEASESTOPPED(START);
		handle_service(STOP, name, 0);
		handle_service(START, name, 0);
	} else {
		if (stops_running)
			(*stops_running)--;
	}
}

static void restart(char *name)
{
	init_shared();
	if (stops_running)
		(*stops_running)++;
	_restart_delay(name, 0);
}

static void restart_fdelay(char *name, int delay)
{
	init_shared();
	if (stops_running)
		(*stops_running)++;
	FORK(_restart_delay(name, delay));
}

static void restart_f(char *name)
{
	restart_fdelay(name, 0);
}

static int restart_main(int argc, char **argv)
{
	restart(argv[1]);
	return 0;
}

static int restart_main_f(int argc, char **argv)
{
	char *name = argv[1];
	RELEASESTOPPED(STOP);
	RELEASESTOPPED(START);
	init_shared();
	if (stops_running)
		(*stops_running)++;
	FORK(_restart_delay(name, 0));
	return 0;
}
