
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
#include <bcmnvram.h>
#include "rc.h"
#include <cyutils.h>
#include <revision.h>

#define loop_forever() do { sleep(1); } while (1)
#define SHELL "/bin/login"
#define	_PATH_CONSOLE	"/dev/console"

#define start_service(a) sysprintf("startservice %s",a);
#define start_service_force(a) sysprintf("startservice %s -f",a);
#define start_service_f(a) sysprintf("startservice_f %s",a);
#define start_service_force_f(a) sysprintf("startservice_f %s -f",a);
#define start_services() system("startservices");
#define start_single_service() system("start_single_service");
#define stop_service(a) sysprintf("stopservice %s",a);
#define stop_service_force(a) sysprintf("stopservice %s -f",a);
#define stop_running(a) sysprintf("stop_running");
#define stop_service_f(a) sysprintf("stopservice_f %s",a);
#define stop_service_force_f(a) sysprintf("stopservice_f %s -f",a);
#define stop_services() system("stopservices");
#define startstop(a) sysprintf("startstop %s",a);
#define startstop_f(a) sysprintf("startstop_f %s",a);

static void set_term(int fd)
{
	struct termios tty;

	tcgetattr(fd, &tty);

	/* 
	 * set control chars 
	 */
	tty.c_cc[VINTR] = 3;	/* C-c */
	tty.c_cc[VQUIT] = 28;	/* C-\ */
	tty.c_cc[VERASE] = 127;	/* C-? */
	tty.c_cc[VKILL] = 21;	/* C-u */
	tty.c_cc[VEOF] = 4;	/* C-d */
	tty.c_cc[VSTART] = 17;	/* C-q */
	tty.c_cc[VSTOP] = 19;	/* C-s */
	tty.c_cc[VSUSP] = 26;	/* C-z */

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
		"PS1=\\u@\\h:\\w\\$ ",
		"PATH=/sbin:/bin:/usr/sbin:/usr/bin:/jffs/sbin:/jffs/bin:/jffs/usr/sbin:/jffs/usr/bin:/mmc/sbin:/mmc/bin:/mmc/usr/sbin:/mmc/usr/bin:/opt/bin:/opt/sbin:/opt/usr/bin:/opt/usr/sbin",
		"LD_LIBRARY_PATH=/usr/lib:/lib:/jffs/usr/lib:/jffs/lib:/opt/lib:/opt/usr/lib",
		"SHELL=" SHELL,
		"USER=root",
#ifdef HAVE_QTN
		"QCSAPI_RPC_TARGET=169.254.39.2",
#endif
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
			execve(SHELL, (char *[]) {
			       "/bin/login", NULL}
			       , envp);
		}
#ifdef HAVE_REGISTER
		else {
			envp[6] = "SHELL=/sbin/regshell";
			execve("/sbin/regshell", (char *[]) {
			       "/sbin/regshell", NULL}, envp);
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

void shutdown_system(void)
{
	int sig;

	/* 
	 * Disable signal handlers 
	 */
	for (sig = 0; sig < (_NSIG - 1); sig++)
		signal(sig, SIG_DFL);

	/* 
	 * Blink led before reboot 
	 */
	diag_led(DIAG, START_LED);
	led_control(LED_DIAG, LED_ON);
	start_service("run_rc_shutdown");
#ifdef HAVE_LAGUNA
	start_service("deconfigurewifi");
#endif
	fprintf(stderr, "Sending SIGTERM to all processes\n");
	kill(-1, SIGTERM);
	sync();
	sleep(5);

	fprintf(stderr, "Sending SIGKILL to all processes\n");
	kill(-1, SIGKILL);
	sync();
	sleep(1);
	system("/bin/umount -a -r");

}

static int fatal_signals[] = {
	SIGQUIT,
	SIGILL,
#ifndef HAVE_RB500
	SIGABRT,
#endif
	SIGFPE,
	SIGPIPE,
	SIGBUS,
	// SIGSEGV, // Don't shutdown, when Segmentation fault.
	SIGSYS,
	SIGTRAP,
	SIGPWR,
	SIGTERM,		/* reboot */
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
		// case SIGUSR1: message = "User-defined signal 1"; break;
	}

	if (message)
		cprintf("%s....................................\n", message);
	else
		cprintf("Caught signal %d.......................................\n", sig);

	shutdown_system();
	sleep(2);

	/* 
	 * Halt on SIGUSR1 
	 */
	reboot(sig == SIGUSR1 ? RB_HALT_SYSTEM : RB_AUTOBOOT);
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
		} else if (sig == SIGUSR1) {	// Receive from WEB
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

static void set_tcp_params(void)
{
	system("/etc/preinit");	// sets default values for ip_conntrack

	FILE *fp = fopen("/proc/sys/net/ipv4/tcp_available_congestion_control", "rb");
	if (fp == NULL) {
		char *vegas = "1";
		char *westwood = "0";
		char *bic = "0";
		if (nvram_match("tcp_congestion_control", "westwood")) {
			westwood = "1";
			vegas = "0";
		}
		if (nvram_match("tcp_congestion_control", "bic")) {
			bic = "1";
			vegas = "0";
		}
		writeproc("/proc/sys/net/ipv4/tcp_westwood", westwood);
		writeproc("/proc/sys/net/ipv4/tcp_vegas_cong_avoid", vegas);
		writeproc("/proc/sys/net/ipv4/tcp_bic", bic);
		writeproc("/proc/sys/net/ipv4/tcp_vegas_alpha", "3");
		writeproc("/proc/sys/net/ipv4/tcp_vegas_beta", "3");
	} else {
		fclose(fp);
		writeproc("/proc/sys/net/ipv4/tcp_congestion_control", nvram_default_get("tcp_congestion_control", "vegas"));
	}

}

/* 
 * Main loop 
 */
int main(int argc, char **argv)
{
	sigset_t sigset;
	pid_t shell_pid = 0;
	uint boardflags;

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
	fprintf(stderr, "start service\n");
	fprintf(stderr, "starting Architecture code for " ARCHITECTURE "\n");
	start_service("devinit");	//init /dev /proc etc.
	start_service("sysinit");
#ifndef HAVE_MICRO
	if (console_init())
		noconsole = 1;
#endif				//HAVE_MICRO
	start_service("drivers");
	cprintf("setup signals\n");
	/* 
	 * Setup signal handlers 
	 */
	signal_init();
	signal(SIGHUP, rc_signal);
	signal(SIGUSR1, rc_signal);	// Start single service from WEB, by
	// honor
	signal(SIGUSR2, rc_signal);
	signal(SIGINT, rc_signal);
	signal(SIGALRM, rc_signal);
	sigemptyset(&sigset);

	/* 
	 * Give user a chance to run a shell before bringing up the rest of the
	 * system 
	 */

	if (!noconsole)
		ddrun_shell(1, 0);
	cprintf("setup nvram\n");

	start_service("nvram");

	/* 
	 * Restore defaults if necessary 
	 */
	int brand = getRouterBrand();

#ifdef HAVE_SKYTEL
	nvram_set("vlan0ports", "0 1 2 3 4 5*");
	nvram_set("vlan1ports", "");
#else

	if (brand == ROUTER_WRT600N || brand == ROUTER_WRT610N) {
		nvram_set("vlan2hwname", "et0");
	}
#endif
	start_service("restore_defaults");

	/* 
	 * Add vlan 
	 */
	boardflags = strtoul(nvram_safe_get("boardflags"), NULL, 0);
	nvram_set("wanup", "0");

#ifndef HAVE_RB500
	switch (brand) {
	case ROUTER_WRT600N:
	case ROUTER_WRT610N:
	case ROUTER_ASUS_WL500GD:
	case ROUTER_ASUS_WL550GE:
	case ROUTER_MOTOROLA:
	case ROUTER_RT480W:
	case ROUTER_WRT350N:
	case ROUTER_BUFFALO_WZRG144NH:
	case ROUTER_DELL_TRUEMOBILE_2300_V2:
		start_service("config_vlan");
		break;
	default:
		if (check_vlan_support()) {
			start_service("config_vlan");
		}
		break;

	}
#endif

	set_ip_forward('1');
	set_tcp_params();
#ifdef HAVE_JFFS2
	start_service("jffs2");
#endif
#ifdef HAVE_MMC
	start_service("mmc");
#endif

	start_service("mkfiles");
	char *hostname;

	/* 
	 * set hostname to wan_hostname or router_name 
	 */
	if (strlen(nvram_safe_get("wan_hostname")) > 0)
		hostname = nvram_safe_get("wan_hostname");
	else if (strlen(nvram_safe_get("router_name")) > 0)
		hostname = nvram_safe_get("router_name");
	else
		hostname = "dd-wrt";

	sethostname(hostname, strlen(hostname));
	stop_service("httpd");

	// create loginprompt
	FILE *fp = fopen("/tmp/loginprompt", "wb");

#ifndef HAVE_MAKSAT
#ifndef HAVE_TRIMAX
#ifndef HAVE_WIKINGS
#ifndef HAVE_ESPOD
#ifndef HAVE_IPR
#ifndef HAVE_NEXTMEDIA
#ifndef HAVE_ERC
#ifndef HAVE_CORENET
#ifdef HAVE_TMK
	fprintf(fp, "KMT-WAS %s (c) 2014 KMT GmbH\nRelease: " BUILD_DATE " (SVN revision: %s)\n", DIST, SVN_REVISION);
#elif HAVE_SANSFIL
	fprintf(fp, "SANSFIL %s (c) 2014 NewMedia-NET GmbH\nRelease: " BUILD_DATE " (SVN revision: %s)\n", DIST, SVN_REVISION);
#elif HAVE_KORENRON
        fprintf(fp, "KORENRON %s (c) 2014 NewMedia-NET GmbH\nRelease: " BUILD_DATE " (SVN revision: %s)\n", DIST, SVN_REVISION);
#elif HAVE_ONNET
#ifdef HAVE_ONNET_BLANK
        fprintf(fp, "Enterprise AP %s (c) 2014 NewMedia-NET GmbH\nRelease: " BUILD_DATE " (SVN revision: %s)\n", DIST, SVN_REVISION);
#elif HAVE_UNFY
        //fprintf(fp, "UNIFY %s (c) 2013 \nRelease: " BUILD_DATE " (SVN revision: %s)\n", DIST, SVN_REVISION);
        fprintf(fp, "Firmware %s (c) 2013 \nRelease: " BUILD_DATE " (SVN revision: %s)\n", DIST, SVN_REVISION);
#else
        fprintf(fp, "OTAi %s (c) 2014 NewMedia-NET GmbH\nRelease: " BUILD_DATE " (SVN revision: %s)\n", DIST, SVN_REVISION);
#endif
#elif HAVE_HDWIFI
        fprintf(fp, "HDWIFI %s (c) 2014 NewMedia-NET GmbH\nRelease: " BUILD_DATE " (SVN revision: %s)\n", DIST, SVN_REVISION);
#else
#ifdef DIST
	if (strlen(DIST) > 0)
		fprintf(fp, "DD-WRT v24-sp2 %s (c) 2014 NewMedia-NET GmbH\nRelease: " BUILD_DATE " (SVN revision: %s)\n", DIST, SVN_REVISION);
	else
		fprintf(fp, "DD-WRT v24-sp2 custom (c) 2014 NewMedia-NET GmbH\nRelease: " BUILD_DATE " (SVN revision: %s)\n", SVN_REVISION);
#else
	fprintf(fp, "DD-WRT v24-sp2 custom (c) 2014 NewMedia-NET GmbH\nRelease: " BUILD_DATE " (SVN revision: %s)\n", SVN_REVISION);
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif

	fclose(fp);

#ifndef HAVE_MADWIFI
	int cnt = get_wl_instances();
#ifdef HAVE_QTN
	cnt = 1;
#endif
#endif
	int c;

	/* 
	 * Loop forever 
	 */
	for (;;) {
		switch (state) {
		case USER:	// Restart single service from WEB of tftpd,
			// by honor
			lcdmessage("RESTART SERVICES");
			cprintf("USER1\n");
			start_single_service();
#ifdef HAVE_CHILLI
			start_service_f("chilli");
#endif
#ifdef HAVE_WIFIDOG
			start_service_f("wifidog");
#endif

			state = IDLE;
			break;

		case RESTART:
			lcdmessage("RESTART SYSTEM");
#ifdef HAVE_OVERCLOCKING
			start_service_force("overclocking");
#endif
			cprintf("RESET NVRAM VARS\n");
			nvram_set("wl0_lazy_wds", nvram_safe_get("wl_lazy_wds"));

			cprintf("RESTART\n");

#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
			for (c = 0; c < cnt; c++) {
				sysprintf("wlconf %s down", get_wl_instance_name(c));
				char *next;
				char var[80];
				char *vifs = nvram_nget("wl%d_vifs", c);

				if (vifs != NULL)
					foreach(var, vifs, next) {
					sysprintf("ifconfig %s down", var);
					}
			}
#endif

			/* 
			 * Fall through 
			 */
		case STOP:
			if (state == STOP && check_action() != ACT_IDLE) {
				state = IDLE;
				break;	//force reboot on upgrade
			}
#ifdef HAVE_REGISTER
			if (isregistered_real())
#endif
			{
				start_service_force("run_rc_shutdown");
			}

			if (state == STOP) {
				state = IDLE;
				break;
			}

			lcdmessage("STOPPING SERVICES");
			cprintf("STOP\n");
			killall("udhcpc", SIGKILL);
			setenv("PATH", "/sbin:/bin:/usr/sbin:/usr/bin:/jffs/sbin:/jffs/bin:/jffs/usr/sbin:/jffs/usr/bin:/mmc/sbin:/mmc/bin:/mmc/usr/sbin:/mmc/usr/bin:/opt/bin:/opt/sbin:/opt/usr/bin:/opt/usr/sbin", 1);
			setenv("LD_LIBRARY_PATH", "/lib:/usr/lib:/jffs/lib:/jffs/usr/lib:/mmc/lib:/mmc/usr/lib:/opt/lib:/opt/usr/lib", 1);
#ifdef HAVE_QTN
			setenv("QCSAPI_RPC_TARGET","169.254.39.2",1);
#endif

			cprintf("STOP SERVICES\n");

			stop_services();
			stop_service("radio_timer");
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
			stop_service("nas");
#endif
			cprintf("STOP WAN\n");
			stop_service("ttraff");
			stop_service_force("wan");
			stop_service("mkfiles");
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
			stop_service("wlconf");
#endif
			cprintf("STOP LAN\n");
#ifdef HAVE_MADWIFI
			stop_service("stabridge");
#endif
#ifdef HAVE_EMF
			stop_service("emf");
#endif
#ifdef HAVE_VLANTAGGING
			stop_service("bridging");
#endif
#ifdef HAVE_BONDING
			stop_service("bonding");
#endif

#ifdef HAVE_VLANTAGGING
			stop_service("bridgesif");
			stop_service("vlantagging");
#endif
			stop_service("lan");
#ifndef HAVE_RB500
			stop_service("resetbutton");
#endif
#ifdef HAVE_IPV6
			stop_service("ipv6");
#endif
#ifdef HAVE_REGISTER
			if (isregistered_real())
#endif
			{
				start_service("run_rc_shutdown");
			}
			/* 
			 * Fall through 
			 */
		case START:
			set_tcp_params();
			lcdmessage("START SERVICES");
			nvram_set("wl0_lazy_wds", nvram_safe_get("wl_lazy_wds"));
			cprintf("START\n");
			setenv("PATH", "/sbin:/bin:/usr/sbin:/usr/bin:/jffs/sbin:/jffs/bin:/jffs/usr/sbin:/jffs/usr/bin:/mmc/sbin:/mmc/bin:/mmc/usr/sbin:/mmc/usr/sbin:/opt/sbin:/opt/bin:/opt/usr/sbin:/opt/usr/sbin", 1);
			setenv("LD_LIBRARY_PATH", "/lib:/usr/lib:/jffs/lib:/jffs/usr/lib:/mmc/lib:/mmc/usr/lib:/opt/lib:/opt/usr/lib", 1);
#ifdef HAVE_QTN
			setenv("QCSAPI_RPC_TARGET","169.254.39.2",1);
#endif
#ifdef HAVE_IPV6
			start_service_f("ipv6");
#endif
#ifndef HAVE_RB500
			start_service_f("resetbutton");
#endif
			start_service_force("setup_vlans");
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
			start_service("wlconf");
#endif
#ifdef HAVE_VLANTAGGING
			start_service("bridging");
#endif
			start_service("lan");
#ifdef HAVE_BONDING
			start_service("bonding");
#endif
#ifdef HAVE_REGISTER
			start_service_force("mkfiles");
#endif
#ifdef HAVE_MADWIFI
			start_service_f("stabridge");
#endif

			cprintf("start services\n");
			start_services();

			cprintf("start wan boot\n");
#ifdef HAVE_VLANTAGGING
			start_service("vlantagging");
			start_service("bridgesif");
#endif
#ifdef HAVE_EMF
			start_service("emf");
#endif
			start_service_force("wan_boot");
			start_service_f("ttraff");

			cprintf("diag STOP LED\n");
			diag_led(DIAG, STOP_LED);
			cprintf("set led release wan control\n");
			SET_LED(RELEASE_WAN_CONTROL);

#ifndef HAVE_ERC
#ifdef HAVE_RADIOOFF
			if (nvram_match("radiooff_button", "1")
			    && nvram_match("radiooff_boot_off", "1")) {
				start_service_force("radio_off");
				led_control(LED_SEC0, LED_OFF);
				led_control(LED_SEC1, LED_OFF);
			} else
#endif
			{
				start_service_force("radio_on");
			}
#endif
			start_service_f("radio_timer");

			cprintf("run rc file\n");
#ifdef HAVE_REGISTER
			if (isregistered_real())
#endif
			{
				startstop_f("run_rc_startup");
// start init scripts				
				system("/etc/init.d/rcS");	
				system("/opt/etc/init.d/rcS");	
				system("/jffs/etc/init.d/rcS");	
				system("/mmc/etc/init.d/rcS");
				// startup script
				// (siPath impl)
				cprintf("start modules\n");
				start_service_force_f("modules");
#ifdef HAVE_MILKFISH
				start_service_force_f("milkfish_boot");
#endif
				if (nvram_invmatch("rc_custom", ""))	// create
					// custom
					// script
				{
					nvram2file("rc_custom", "/tmp/custom.sh");
					chmod("/tmp/custom.sh", 0700);
				}
			}
#ifdef HAVE_CHILLI
			start_service_f("chilli");
#endif
#ifdef HAVE_WIFIDOG
			start_service_f("wifidog");
#endif
			cprintf("start syslog\n");
#ifdef HAVE_SYSLOG
			startstop_f("syslog");
#endif

			system("/etc/postinit&");
			start_service_f("httpd");
			led_control(LED_DIAG, LED_OFF);
			lcdmessage("System Ready");
#ifndef HAVE_RB500
			startstop_f("resetbutton");
#endif
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
