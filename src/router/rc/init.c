/*
 * init.c
 *
 * Copyright (C) 2006 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <errno.h>
#include <paths.h>
#include <signal.h>
#include <stdarg.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <limits.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <shutils.h>
#include <utils.h>
#include <wlutils.h>
#include "rc.h"
#include <cyutils.h>
#include <revision.h>
#include <bcmnvram.h>

int isregistered_real(void);

#define loop_forever()    \
	do {              \
		sleep(1); \
	} while (1)
#define SHELL "/bin/login"
#define _PATH_CONSOLE "/dev/console"

#define start_service(a) eval("startservice", a)
#define start_service_force(a) eval("startservice", a, "-f")
#define start_service_f(a) eval("startservice_f", a)
#define start_service_force_f(a) eval("startservice_f", a, "-f")
#define start_services() eval("startservices")
#define stop_service(a) eval("stopservice", a)
#define stop_service_force(a) eval("stopservice", "-f", a)
#define stop_running(a) eval("stop_running")
#define stop_service_f(a) eval("stopservice_f", a)
#define stop_service_force_f(a) eval("stopservice_f", a, "-f")
#define stop_services() eval("stopservices")
#define service_shutdown() eval("service","shutdown")
#define restart(a) eval("restart", a)
#define restart_f(a) eval("restart_f", a)
#define start_single_service() eval("start_single_service")

static void set_term(int fd)
{
	struct termios tty;

	tcgetattr(fd, &tty);

	/* 
	 * set control chars 
	 */
	tty.c_cc[VINTR] = 3; /* C-c */
	tty.c_cc[VQUIT] = 28; /* C-\ */
	tty.c_cc[VERASE] = 127; /* C-? */
	tty.c_cc[VKILL] = 21; /* C-u */
	tty.c_cc[VEOF] = 4; /* C-d */
	tty.c_cc[VSTART] = 17; /* C-q */
	tty.c_cc[VSTOP] = 19; /* C-s */
	tty.c_cc[VSUSP] = 26; /* C-z */

	/* 
	 * use line dicipline 0 
	 */
	tty.c_line = 0;

	/* 
	 * Make it be sane 
	 */
	tty.c_cflag &= CBAUD | CBAUDEX | CSIZE | CSTOPB | PARENB | PARODD;
	tty.c_cflag |= CREAD | HUPCL | CLOCAL;

	/* 
	 * input modes 
	 */
	tty.c_iflag = ICRNL | IXON | IXOFF;

	/* 
	 * output modes 
	 */
	tty.c_oflag = OPOST | ONLCR;

	/* 
	 * local modes 
	 */
	tty.c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHOCTL | ECHOKE | IEXTEN;

	tcsetattr(fd, TCSANOW, &tty);
}

int console_init()
{
	int fd;

	/* 
	 * Clean up 
	 */
	ioctl(0, TIOCNOTTY, 0);
	close(0);
	close(1);
	close(2);
	setsid();

	/* 
	 * Reopen console 
	 */
	if ((fd = open(_PATH_CONSOLE, O_RDWR)) < 0) {
		/* 
		 * Avoid debug messages is redirected to socket packet if no exist a
		 * UART chip, added by honor, 2003-12-04 
		 */
		(void)open("/dev/null", O_RDONLY);
		(void)open("/dev/null", O_WRONLY);
		(void)open("/dev/null", O_WRONLY);
		perror(_PATH_CONSOLE);
		return errno;
	}
	dup2(fd, 0);
	dup2(fd, 1);
	dup2(fd, 2);

	ioctl(0, TIOCSCTTY, 1);
	tcsetpgrp(0, getpgrp());
	set_term(0);

	return 0;
}

pid_t ddrun_shell(int timeout, int nowait)
{
	pid_t pid;
	char tz[1000];
	char *envp[] = {
		"TERM=vt100",
		"TERMINFO=/etc/terminfo",
		"HOME=/",
		"PS1=\\[\\033]0;\\u@\\h: \\w\\a\\]\\[\\033[01;31m\\]\\u@\\h\\[\\033[00m\\]:\\[\\033[01;34m\\]\\w\\[\\033[00m\\]\\$ ",
		"PATH=/sbin:/bin:/usr/sbin:/usr/bin:/jffs/sbin:/jffs/bin:/jffs/usr/sbin:/jffs/usr/bin:/mmc/sbin:/mmc/bin:/mmc/usr/sbin:/mmc/usr/bin:/opt/bin:/opt/sbin:/opt/usr/bin:/opt/usr/sbin",
		"LD_LIBRARY_PATH=/usr/lib:/lib:/jffs/usr/lib:/jffs/lib:/opt/lib:/opt/usr/lib",
		"SHELL=" SHELL,
		"USER=root",
		tz,
		NULL
	};
	int sig;

	/* 
	 * Wait for user input 
	 */
	// cprintf("Hit enter to continue...");
	if (waitfor(STDIN_FILENO, timeout) <= 0)
		return 0;

	switch ((pid = fork())) {
	case -1:
		perror("fork");
		return 0;
	case 0:
		/* 
		 * Reset signal handlers set for parent process 
		 */
		for (sig = 0; sig < (_NSIG - 1); sig++)
			signal(sig, SIG_DFL);

		/* 
		 * Reopen console 
		 */
		console_init();
		// if (ret) exit(0); //no console running
		/* 
		 * Pass on TZ 
		 */
		snprintf(tz, sizeof(tz), "TZ=%s", getenv("TZ"));
		/* 
		 * Now run it.  The new program will take over this PID, so
		 * nothing further in init.c should be run. 
		 */
#ifdef HAVE_REGISTER
		if (isregistered_real())
#endif
		{
			execve(SHELL, (char *[]){ "/bin/login", NULL }, envp);
		}
#ifdef HAVE_REGISTER
		else {
			envp[6] = "SHELL=/sbin/regshell";
			execve("/sbin/regshell", (char *[]){ "/sbin/regshell", NULL }, envp);
		}
#endif

		/* 
		 * We're still here? Some error happened. 
		 */
		perror(SHELL);
		exit(errno);
	default:
		if (nowait)
			return pid;
		else {
			waitpid(pid, NULL, 0);
			return 0;
		}
	}
}

static void unmount_fs(void)
{
	char dev[32];
	char mpoint[128];
	char fstype[32];
	char flags[128];
	int a, b;
	eval("sync");
	writeprocsys("vm/drop_caches", "3"); // flush fs cache
	FILE *fp = fopen("/proc/mounts", "rb");
	while (!feof(fp) && fscanf(fp, "%s %s %s %s %d %d", dev, mpoint, fstype, flags, &a, &b) == 6) {
		if (!strcmp(fstype, "proc"))
			continue;
		if (!strcmp(fstype, "sysfs"))
			continue;
		if (!strcmp(fstype, "debugfs"))
			continue;
		if (!strcmp(fstype, "ramfs"))
			continue;
		if (!strcmp(fstype, "tmpfs"))
			continue;
		if (!strcmp(fstype, "devpts"))
			continue;
		if (!strcmp(fstype, "usbfs"))
			continue;
#if defined(HAVE_X86) || defined(HAVE_VENTANA) || defined(HAVE_NEWPORT) || defined(HAVE_OPENRISC)
		if (!strcmp(mpoint, "/usr/local")) {
			continue;
		}
		if (!strcmp(mpoint, "/")) {
			continue;
		}
#endif
		dd_loginfo("init", "unmounting %s", mpoint);
		eval("mount", "-o", "remount,sync,ro",
		     mpoint); // set to readonly first since unmounting may fail.
		eval("umount", "-r", "-f", mpoint);
	}
	fclose(fp);
#ifdef HAVE_RAID
	int i = 0;
	while (1) {
		char *raid = nvram_nget("raid%d", i);
		if (!*raid)
			break;
		char *poolname = nvram_nget("raidname%d", i);
		sysprintf("mdadm --stop /dev/md%d", i);
		sysprintf("zfs unmount %s", poolname);
		i++;
	}
#endif
}
static char *critical_programs[] = { "upnpd",	     "transmissiond", "process_monitor", "cron",    "proftpd",	  "dnsmasq",
				     "ksmbd.mountd", "hotplug2",      "ubusd",		 "rpcbind", "rpc.mountd", "httpd",
				     "minidlna",     "rsyncd",	      "dropbear",	 "wland",   "smartd",	  "rpc.statd",
				     "/bin/sh",	     "telnetd",	      "mactelnetd",	 "syslogd", "klogd",	  "wsdd2",
				     "udhcpc",	     "async_commit" };
void shutdown_system(void)
{
	int sig;
	int deadcount = 0;
	while (pidof("async_commit") > 0 && (deadcount++) < 10) // wait for any process of this type to finish
	{
		dd_loginfo("init", "wait for nvram write to finish");
		sleep(1);
	}
	deadcount = 0;

	/* 
	 * Disable signal handlers 
	 */
	for (sig = 0; sig < (_NSIG - 1); sig++)
		signal(sig, SIG_DFL);
	if (!nvram_match("shutdown", "fast")) {
		start_service("run_rc_shutdown");
		service_shutdown();
		if (pidof("udhcpc") > 0) {
			dd_loginfo("init", "send dhcp lease release signal");
			killall("udhcpc", SIGUSR2);
			sleep(1);
		}

		dd_loginfo("init", "Sending SIGTERM to all processes");
		kill(-1, SIGTERM);
		dd_loginfo("init", "Waiting some seconds to give programs time to flush");
		int i;
		while (deadcount++ < 10) {
			for (i = 0; i < ARRAY_SIZE(critical_programs); i++) {
				if (pidof(critical_programs[i]) > 0) {
					dd_loginfo("init", "waiting for %s to stop", critical_programs[i]);
					break;
				}
			}
			if (i == ARRAY_SIZE(critical_programs))
				break;
			sleep(1);
		}
		sync();
		dd_loginfo("init", "Sending SIGKILL to all processes");
		kill(-1, SIGKILL);
		sync();
		unmount_fs(); // try to unmount
#if defined(HAVE_X86) || defined(HAVE_VENTANA) || defined(HAVE_NEWPORT) || defined(HAVE_OPENRISC)
		eval("mount", "-o", "remount,ro", "/usr/local");
		eval("mount", "-o", "remount,ro", "/");
		eval("umount", "-r", "-f", "/usr/local");
		eval("umount", "-r", "-f", "/");
#endif
		start_service("sysshutdown");
		nvram_seti("end_time", time(NULL));
		nvram_commit();
	}
}

static int fatal_signals[] = {
	SIGQUIT, SIGILL,
#ifndef HAVE_RB500
	SIGABRT,
#endif
	SIGFPE, SIGPIPE, SIGBUS,
	// SIGSEGV, // Don't shutdown, when Segmentation fault.
	SIGSYS, SIGTRAP, SIGPWR, SIGTERM, /* reboot */
	// SIGUSR1, /* halt */ // We use the for some purpose
};

void fatal_signal(int sig)
{
	char *message = NULL;

	switch (sig) {
	case SIGQUIT:
		message = "Quit";
		break;
	case SIGILL:
		message = "Illegal instruction";
		break;
	case SIGABRT:
		message = "Abort";
		break;
	case SIGFPE:
		message = "Floating exception";
		break;
	case SIGPIPE:
		message = "Broken pipe";
		break;
	case SIGBUS:
		message = "Bus error";
		break;
	case SIGSEGV:
		message = "Segmentation fault";
		break;
	case SIGSYS:
		message = "Bad system call";
		break;
	case SIGTRAP:
		message = "Trace trap";
		break;
	case SIGPWR:
		message = "Power failure";
		break;
	case SIGTERM:
		message = "Terminated";
		break;
	case SIGUSR1:
		message = "User Signal 1";
		break;
		// case SIGUSR1: message = "User-defined signal 1"; break;
	}

	if (message)
		dd_loginfo("init", "%s....................................", message);
	else
		dd_loginfo("init", "Caught signal %d.......................................", sig);

	shutdown_system();
	eval("sync");
	sysprintf("echo 3 > /proc/sys/vm/drop_caches");

	/* 
	 * Halt on SIGUSR1 
	 */
#ifdef HAVE_NEWPORT
	sleep(5);
	writeproc("/proc/sysrq-trigger", "b");
#endif
	reboot(RB_AUTOBOOT);
#ifndef HAVE_VENTANA
	sleep(10);
	writeproc("/proc/sysrq-trigger", "b");
#endif
	loop_forever();
}

static void reap(int sig)
{
	pid_t pid;

	while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
		cprintf("Reaped %d\n", pid);
}

void signal_init(void)
{
	int i;

	for (i = 0; i < sizeof(fatal_signals) / sizeof(fatal_signals[0]); i++)
		signal(fatal_signals[i], fatal_signal);

	signal(SIGCHLD, reap);
}

/* 
 * States 
 */
enum {
	RESTART,
	STOP,
	START,
	TIMER,
	USER,
	IDLE,
};

static int state = START;
static int signalled = -1;

/* 
 * Signal handling 
 */
static void rc_signal(int sig)
{
	if (state == IDLE) {
		if (sig == SIGHUP) {
			lcdmessage("Signal RESTART");
			printf("signalling RESTART\n");
			signalled = RESTART;
		} else if (sig == SIGUSR2) {
			lcdmessage("Signal START");
			printf("signalling START\n");
			signalled = START;
		} else if (sig == SIGINT) {
			lcdmessage("Signal STOP");
			printf("signalling STOP\n");
			signalled = STOP;
		} else if (sig == SIGALRM) {
			lcdmessage("Signal TIMER");
			printf("signalling TIMER\n");
			signalled = TIMER;
		} else if (sig == SIGUSR1) { // Receive from WEB
			lcdmessage("Signal USER");
			printf("signalling USER1\n");
			signalled = USER;
		}
	}
}

/* 
 * Timer procedure 
 */
int do_timer(void)
{
	// do_ntp();
	return 0;
}

static int noconsole = 0;

/* 
 * Main loop 
 */
int main(int argc, char **argv)
{
	sigset_t sigset;
	pid_t shell_pid = 0;

	/* 
	 * Basic initialization 
	 */
#ifdef HAVE_MICRO
	if (console_init())
		noconsole = 1;
#endif
	cprintf("init lcd\n");
	initlcd();
	cprintf("first message\n");
	lcdmessage("System Start");

	signal_init();
	signal(SIGHUP, rc_signal);
	signal(SIGUSR1, rc_signal); // Start single service from WEB, by
	// honor
	signal(SIGUSR2, rc_signal);
	signal(SIGINT, rc_signal);
	signal(SIGALRM, rc_signal);
	sigemptyset(&sigset);

	dd_loginfo("init", "starting devinit");
	start_service("devinit"); //init /dev /proc etc.
	writeproc("/proc/sys/kernel/sysrq", "1");
	start_service("check_bootfails");
#if defined(HAVE_X86) || defined(HAVE_NEWPORT) || (defined(HAVE_RB600) && !defined(HAVE_WDR4900)) //special treatment
	FILE *out = fopen("/tmp/.nvram_done", "wb");
	putc(1, out);
	fclose(out);
#endif
	dd_loginfo("init", "starting Architecture code for " ARCHITECTURE "");
	start_service("sysinit");
#ifndef HAVE_MICRO
	start_service("watchdog");
	if (console_init())
		noconsole = 1;
#endif //HAVE_MICRO
	cprintf("setup signals\n");
	/* 
	 * Setup signal handlers 
	 */

	start_service("post_sysinit");

	/* 
	 * Give user a chance to run a shell before bringing up the rest of the
	 * system 
	 */

	if (!noconsole)
		ddrun_shell(1, 0);
	cprintf("setup nvram\n");

	/* 
	 * Loop forever 
	 */
	for (;;) {
		switch (state) {
		case USER: // Restart single service from WEB of tftpd,
			// by honor
			lcdmessage("RESTART SERVICES");
			cprintf("USER1\n");
			start_single_service();
#ifdef HAVE_CHILLI
			start_service_force_f("chilli");
#endif
#ifdef HAVE_WIFIDOG
			start_service_force_f("wifidog");
#endif

			state = IDLE;
			break;

		case RESTART:
			lcdmessage("RESTART SYSTEM");
			start_service_force("init_restart");

			/* 
			 * Fall through 
			 */
		case STOP:
			if (state == STOP && check_action() != ACT_IDLE) {
				state = IDLE;
				break; //force reboot on upgrade
			}
			setenv("PATH",
			       "/sbin:/bin:/usr/sbin:/usr/bin:/jffs/sbin:/jffs/bin:/jffs/usr/sbin:/jffs/usr/bin:/mmc/sbin:/mmc/bin:/mmc/usr/sbin:/mmc/usr/bin:/opt/sbin:/opt/bin:/opt/usr/sbin:/opt/usr/bin",
			       1);
			setenv("LD_LIBRARY_PATH",
			       "/lib:/usr/lib:/jffs/lib:/jffs/usr/lib:/mmc/lib:/mmc/usr/lib:/opt/lib:/opt/usr/lib", 1);
#ifdef HAVE_REGISTER
			if (isregistered_real())
#endif
			{
				start_service_force("run_rc_shutdown");
			}

			start_service_force("init_stop");

			if (state == STOP) {
				state = IDLE;
				break;
			}
			/* 
			 * Fall through 
			 */
		case START:
			cprintf("START\n");
			setenv("PATH",
			       "/sbin:/bin:/usr/sbin:/usr/bin:/jffs/sbin:/jffs/bin:/jffs/usr/sbin:/jffs/usr/bin:/mmc/sbin:/mmc/bin:/mmc/usr/sbin:/mmc/usr/bin:/opt/sbin:/opt/bin:/opt/usr/sbin:/opt/usr/bin",
			       1);
			setenv("LD_LIBRARY_PATH",
			       "/lib:/usr/lib:/jffs/lib:/jffs/usr/lib:/mmc/lib:/mmc/usr/lib:/opt/lib:/opt/usr/lib", 1);
			update_timezone();
			start_service_force("init_start");

			/* 
			 * Fall through 
			 */
		case TIMER:
			cprintf("TIMER\n");
			do_timer();
			/* 
			 * Fall through 
			 */
		case IDLE:
			cprintf("IDLE\n");
			state = IDLE;
			/* 
			 * Wait for user input or state change 
			 */
			while (signalled == -1) {
				if (!noconsole && (!shell_pid || kill(shell_pid, 0) != 0))
					shell_pid = ddrun_shell(0, 1);
				else
					sigsuspend(&sigset);
			}
			state = signalled;
			signalled = -1;
			break;
		default:
			cprintf("UNKNOWN\n");
			return 0;
		}
	}
}
