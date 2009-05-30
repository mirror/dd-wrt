#ifdef HAVE_WOL

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <bcmnvram.h>
#include <shutils.h>
// #include <snmp.h>
#include <signal.h>

#define WOL_INTERVAL 15

int wol_run(void)
{
	int ret = 0;
	char *macs = NULL;
	char passwd_param[32] = { 0 };
	char *passwd = NULL;
	char hostname_param[32] = { 0 };
	char *hostname = NULL;

	if (nvram_match("wol_enable", "0"))
		return 0;

	/* 
	 * Most of time it goes to sleep 
	 */
	while (nvram_match("wol_enable", "1")) {
		int interval = 0;

		interval =
		    atoi(nvram_safe_get("wol_interval")) >
		    WOL_INTERVAL ? atoi(nvram_safe_get("wol_interval")) :
		    WOL_INTERVAL;

		if (strlen(nvram_get("wol_passwd")) <= 0)
			passwd_param[0] = 0;
		else
			strcpy(passwd_param, "-P");

		if (strlen(nvram_get("wol_hostname")) <= 0)
			hostname_param[0] = 0;
		else
			strcpy(hostname_param, "-h");

		passwd = nvram_safe_get("wol_passwd");
		hostname = nvram_safe_get("wol_hostname");
		macs = nvram_safe_get("wol_macs");

		ret =
		    eval("/usr/sbin/wol", passwd_param, passwd, hostname_param,
			 hostname, macs);

		sleep(interval);
	}

	cprintf("done\n");

	return ret;
}

int wol_main(int argc, char **argv)
{
	/* 
	 * Run it in the background 
	 */

	signal(SIGCLD, SIG_IGN);

	switch (fork()) {
	case -1:
		// can't fork
		exit(0);
		break;
	case 0:
		/* 
		 * child process 
		 */
		// fork ok
		(void)setsid();
		break;
	default:
		/* 
		 * parent process should just die 
		 */
		_exit(0);
	}

	wol_run();

	return 0;
}				// end main

#endif				/* HAVE_WOL */
