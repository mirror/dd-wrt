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
#include <stdio.h>
#include <shutils.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <bcmnvram.h>
#include <utils.h>

#define SERVICE_MODULE "/lib/services.so"
static int *stops_running = NULL;

static void init_shared(void)
{
	if (!stops_running)
		stops_running = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
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
		dd_debug(DEBUG_SERVICE, "calling %s_%s not required!\n", method ? "stop" : "start", name);

		if (method == STOP) {
			if (stops_running)
				(*stops_running)--;
		}

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

#define STOPPED()                     \
	if (_STOPPED(method, name)) { \
		return 0;             \
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
	dd_debug(DEBUG_SERVICE, "%s:%s_%s", __func__, method_name, name);
	// lcdmessaged("Starting Service",name);
	char service[64];

	sprintf(service, "/etc/config/%s", name);
	FILE *ck = fopen(service, "rb");

	if (ck != NULL) {
		fclose(ck);
		return sysprintf("%s %s", service, method_name);
	}
	char *args[] = { "/sbin/service", (char *)name, method_name, NULL };
	char *args_f[] = { "/sbin/service", (char *)name, method_name, "-f", NULL };

	if (force)
		ret = _evalpid(args_f, NULL, 0, NULL);
	else
		ret = _evalpid(args, NULL, 0, NULL);

	if (method == STOP) {
		if (stops_running)
			(*stops_running)--;
	}
	if (stops_running)
		dd_debug(DEBUG_SERVICE, "calling done %s_%s (pending stops %d)\n", method_name, name, *stops_running);
	else
		dd_debug(DEBUG_SERVICE, "calling done %s_%s\n", method_name, name);

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

static int stop_running(void)
{
	return *stops_running > 0;
}

static int stop_running_main(int argc, char **argv)
{
	int dead = 0;
	while (stops_running != NULL && stop_running() && dead < 100) {
		if (debug_ready() && nvram_matchi("service_debugrunnings", 1))
			dd_loginfo("servicemanager", "%s: dead: %d running %d", __func__, dead, *stops_running);

		if (dead == 0)
			dd_loginfo("servicemanager", "waiting for services to finish (%d)...", *stops_running);
		usleep(100 * 1000);
		dead++;
	}
	if (dead == 50) {
		dd_logerror("servicemanager", "stopping processes taking too long!!!");
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
	if (argc < 2) {
		fprintf(stderr, "missing argument!. use \"restart servicename\"\n");
	}
	restart(argv[1]);
	return 0;
}

static int restart_main_f(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "missing argument!. use \"restart_f servicename\"\n");
	}
	char *name = argv[1];
	RELEASESTOPPED(STOP);
	RELEASESTOPPED(START);
	init_shared();
	if (stops_running)
		(*stops_running)++;
	FORK(_restart_delay(name, 0));
	return 0;
}
#include "services.c"

int main(int argc, char *argv[])
{
	char *base = argv[0];

	int force = (argc == 3 && !strcmp(argv[2], "-f"));
	if (strstr(base, "stopservices")) {
		stop_services_main(argc, argv);
		goto out;
	}
	if (strstr(base, "start_single_service")) {
		start_single_service_main(argc, argv);
		goto out;
	}
	if (strstr(base, "stop_running")) {
		stop_running_main(argc, argv);
		goto out;
	}
	if (strstr(base, "startservices")) {
		start_services_main(argc, argv);
		goto out;
	}

	if (argc < 2) {
		fprintf(stdout, "%s: servicename [-f]\n", base);
		fprintf(stdout,
			"-f : forces start/stop of service and without any care if service was already started or stopped\n");
		return 1;
	}

	if (strstr(base, "startservice_f")) {
		if (force)
			start_service_force_f_arg(argv[1], 1);
		else
			start_service_f(argv[1]);
		goto out;
	}
	if (strstr(base, "startservice")) {
		if (force)
			start_service_force_arg(argv[1], 1);
		else
			start_service(argv[1]);
		goto out;
	}
	if (strstr(base, "stopservice_f")) {
		if (force)
			stop_service_force_f(argv[1]);
		else
			stop_service_f(argv[1]);
		goto out;
	}
	if (strstr(base, "stopservice")) {
		if (force)
			stop_service_force(argv[1]);
		else
			stop_service(argv[1]);
		goto out;
	}
	if (strstr(base, "restart_f")) {
		restart_main(argc, argv);
		goto out;
	}
	if (strstr(base, "restart")) {
		restart_main_f(argc, argv);
		goto out;
	}
out:;
	return 0;
}
