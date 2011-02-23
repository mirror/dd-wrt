/**********************************************************
 SixXS - Automatic IPv6 Connectivity Configuration Utility
***********************************************************
 Copyright 2003-2005 SixXS - http://www.sixxs.net
***********************************************************
 unix-client/aiccu.c - AICCU - The client for UNIX
***********************************************************
 $Author: jeroen $
 $Id: main.c,v 1.20 2007-01-15 11:57:34 jeroen Exp $
 $Date: 2007-01-15 11:57:34 $
**********************************************************/

#include "../common/aiccu.h"
#include "../common/tun.h"

#ifndef _WIN32
/* Enable/Disable heartbeating */
void sigusr1(int i);
void sigusr1(int i)
{
	/* Toggle the flag */
	g_aiccu->makebeats = !g_aiccu->makebeats;

	/* Reset the signal */
	signal(i, &sigusr1);
}

void sigterm(int i);
void sigterm(int i)
{
	g_aiccu->running = false;
	signal(i, SIG_IGN);
}

int sigrunning(int sig);
int sigrunning(int sig)
{
	int	pid;
	FILE	*f;

	if (!g_aiccu) return 0;

	/* Open our PID file */
	f = fopen(g_aiccu->pidfile, "r");
	if (!f) return 0;

	/* Get the PID from the file or make it invalid when the format is wrong */
	if (fscanf(f, "%d", &pid) != 1) pid = -1;

	/* Close the file again */
	fclose(f);

	/* If we can HUP it, it still runs */
	return (pid > 0 && kill(pid, sig) == 0 ? 1 : 0);
}

#else

static BOOL sigterm(DWORD sig);
static BOOL sigterm(DWORD sig)
{
	D(dolog(LOG_DEBUG, "Terminating due to CTRL event\n"));
	g_aiccu->running = false;
	return true;
}
static BOOL sigterm_testing(DWORD sig);
static BOOL sigterm_testing(DWORD sig)
{
	D(dolog(LOG_DEBUG, "Ignoring CTRL event\n"));
	return true;
}

#endif

int list_tunnels(void);
int list_tunnels(void)
{
	struct TIC_sTunnel *hsTunnel, *t;

	if (!tic_Login(g_aiccu->tic, g_aiccu->username, g_aiccu->password, g_aiccu->server)) return 0;

	hsTunnel = tic_ListTunnels(g_aiccu->tic);

	if (!hsTunnel)
	{
		tic_Logout(g_aiccu->tic, "Getting current tunnel listing");
		return 1;
	}

	for (t = hsTunnel; t; t = t->next)
	{
		printf("%s %s %s %s\n", t->sId, t->sIPv6, t->sIPv4, t->sPOPId);
	}

	tic_Free_sTunnel(hsTunnel);
	tic_Logout(g_aiccu->tic, "Getting current tunnel listing");
	return 1;
}

static unsigned int prevnum = 54321;

/* Due to broken DNS servers out there, make sure that we get at least the SixXS TIC server */
static bool foundsixxs = false;

void gotrr(unsigned int num, int type, const char *record);
void gotrr(unsigned int num, int type, const char *record)
{
	/* Skip non-TXT records + Comments */
	if (type != T_TXT || record[0] == '#') return;
	/* If the record number changed and it is not the first one, add a return */
	if (num != prevnum && prevnum != 54321) printf("\n");

	/* The current record = the last one seen */
	prevnum = num;

	/* Print the component */
	printf("%s|", record);

	/* Found SixXS? */
	if (strcmp(record, "SixXS") == 0) foundsixxs = true;
}

/* Get Tunnel Brokers from _aiccu.<search path> and from _aiccu.sixxs.net */
int list_brokers(void);
int list_brokers(void)
{
	foundsixxs = false;
	prevnum = 54321;
	getrrs("_aiccu", T_TXT, gotrr);
	prevnum = 54321;
	getrrs("_aiccu.sixxs.net", T_TXT, gotrr);
	printf("\n");

	if (!foundsixxs)
	{
		printf("SixXS|tic://tic.sixxs.net|http://www.sixxs.net|be de ee fi gb ie it nl pl pt si se us");

		/* Warn the user of the missing global tb's */
		fprintf(stderr, "Warning: Couldn't find global Tunnel Brokers List, please check your DNS settings and read the FAQ.\n");
	}

	return 1;
}

/* 
 * AICCU! - Aka... let's get connected ;)
 * returns a TIC_Tunnel which can then be
 * used for configuring and keeping it running
 */
struct TIC_Tunnel *get_tunnel(void);
struct TIC_Tunnel *get_tunnel(void)
{
	
	struct TIC_sTunnel	*hsTunnel, *t;
	struct TIC_Tunnel	*hTunnel;

	/* Login to the TIC Server */
	if (!tic_Login(g_aiccu->tic, g_aiccu->username, g_aiccu->password, g_aiccu->server)) return NULL;

	/* 
	 * Don't try to list the tunnels when
	 * we already have a tunnel_id configured
	 */
	if (!g_aiccu->tunnel_id)
	{	
		hsTunnel = tic_ListTunnels(g_aiccu->tic);
		if (!hsTunnel)
		{
			dolog(LOG_ERR, "No tunnel available, request one first\n");
			tic_Free_sTunnel(hsTunnel);
			tic_Logout(g_aiccu->tic, "I didn't have any tunnels to select");
			return NULL;
		}
	
		if (hsTunnel->next)
		{
			dolog(LOG_ERR, "Multiple tunnels available, please pick one from the following list and configure the aiccu.conf using it\n");
			for (t = hsTunnel; t; t = t->next)
			{
				dolog(LOG_ERR, "%s %s %s %s\n", t->sId, t->sIPv6, t->sIPv4, t->sPOPId);
			}
			tic_Free_sTunnel(hsTunnel);
			tic_Logout(g_aiccu->tic, "User still needed to select a tunnel");
			return NULL;
		}
		g_aiccu->tunnel_id = strdup(hsTunnel->sId);

		/* Free the info */
		tic_Free_sTunnel(hsTunnel);
	}

	/* Get Tunnel Information */
	hTunnel = tic_GetTunnel(g_aiccu->tic, g_aiccu->tunnel_id);
	if (!hTunnel)
	{
		tic_Logout(g_aiccu->tic, "No such tunnel");
		return NULL;
	}

	/* Logout, TIC is not needed any more */
	tic_Logout(g_aiccu->tic, NULL);

	/* Swee.... sufficient information */
	return hTunnel;
}

enum AICCU_MODES
{
	A_NONE = 0,
	A_START,
	A_STOP,
	A_BROKERS,
	A_TUNNELS,
	A_TEST,
	A_AUTOTEST,
	A_LICENSE,
#ifdef _WIN32
	A_LISTTAPS,
#endif
	A_VERSION
};

const char *options = "aiccu (start|stop|brokers|tunnels|test|autotest|license|"
#ifdef _WIN32
	"listtaps|"
#endif
	"version) [<configfile>]\n";

int main(int argc, char *argv[])
{
	enum AICCU_MODES	mode = A_NONE;

	struct TIC_Tunnel	*hTunnel;
#ifdef _WIN32
	WSADATA			wsadata;
	unsigned int		i;

	/* Initialize Winsock so that we can do network functions */
	WSAStartup(WINSOCK_VERSION, &wsadata);
#endif

	/* Initialize Configuration */
	aiccu_InitConfig();

	/* Make sure we actually have an IPv6 stack */
	aiccu_install();

	/* Require start/stop/test */
	if (argc == 2 || argc == 3)
	{
		     if (strcasecmp(argv[1], "start")	== 0) mode = A_START;
		else if (strcasecmp(argv[1], "stop")	== 0) mode = A_STOP;
		else if (strcasecmp(argv[1], "brokers") == 0) mode = A_BROKERS;
		else if (strcasecmp(argv[1], "tunnels") == 0) mode = A_TUNNELS;
		else if (strcasecmp(argv[1], "test")	== 0) mode = A_TEST;
		else if (strcasecmp(argv[1], "autotest")== 0) mode = A_AUTOTEST;
		else if (strcasecmp(argv[1], "license")	== 0) mode = A_LICENSE;
#ifdef _WIN32
		else if (strcasecmp(argv[1], "listtaps") == 0) mode = A_LISTTAPS;
#endif
		else if (strcasecmp(argv[1], "version")	== 0) mode = A_VERSION;
	}

	/* Optionally we want a second argument: a config file */
	if ((	argc != 2 &&
		argc != 3) ||
		mode == A_NONE)
	{
		dolog(LOG_ERR, "%s", options);
		return -1;
	}

	if (	mode == A_LICENSE)
	{
		printf("%s\n", aiccu_license());
		return 0;
	}

	if (	mode == A_VERSION)
	{
		printf("AICCU %s by Jeroen Massar\n", AICCU_VERSION);
		return 0;
	}

#ifdef _WIN32
	if (	mode == A_LISTTAPS)
	{
		tun_list_tap_adapters();
		return 0;
	}
#endif

	if (	mode == A_BROKERS)
	{
		int ret = list_brokers();
		aiccu_FreeConfig();
		return ret == 0 ? -1 : 0;
	}

	if (!aiccu_LoadConfig(argc <= 2 ? NULL : argv[2]))
	{
		return -1;
	}

#ifndef _WIN32
	/* start or stop? */
	if (	mode != A_TEST &&
		mode != A_AUTOTEST)
	{
		/* Already running? */
		if (sigrunning(mode == A_STOP ? SIGTERM : 0) == 1)
		{
			dolog(LOG_ERR, "Already running instance HUP'ed, exiting\n");
			return 0;
		}
	}
#endif

	/* Verify required parameters */
	if (!g_aiccu->username || !g_aiccu->password)
	{
		dolog(LOG_ERR, "Required parameters missing, make sure that username and password are given\n");
		aiccu_FreeConfig();
		return -1;
	}

	if (mode == A_TUNNELS)
	{
		int ret = list_tunnels();
		aiccu_FreeConfig();
		return ret == 0 ? -1 : 0;
	}

	/* Get our tunnel */
	hTunnel = get_tunnel();
	
	if (!hTunnel)
	{
		dolog(LOG_ERR, "Couldn't retrieve first tunnel for the above reason, aborting\n");
		aiccu_FreeConfig();
		return -1;
	}

	/* 
	 * We now have sufficient information.
	 * Thus we can logout from the TIC server
	 */
	tic_Logout(g_aiccu->tic, NULL);
	g_aiccu->tic = NULL;

	if (g_aiccu->verbose)
	{
		printf("Tunnel Information for %s:\n",hTunnel->sId);
		printf("POP Id      : %s\n", hTunnel->sPOP_Id);
		printf("IPv6 Local  : %s/%u\n", hTunnel->sIPv6_Local,hTunnel->nIPv6_PrefixLength);
		printf("IPv6 Remote : %s/%u\n", hTunnel->sIPv6_POP,hTunnel->nIPv6_PrefixLength);
		printf("Tunnel Type : %s\n", hTunnel->sType);
		printf("Adminstate  : %s\n", hTunnel->sAdminState);
		printf("Userstate   : %s\n", hTunnel->sUserState);
	}

	/* One can always try to stop it */
	if (mode == A_STOP)
	{
		aiccu_delete(hTunnel);

		/* Free stuff and exit */
		tic_Free_Tunnel(hTunnel);
		aiccu_FreeConfig();
		return 0;
	}

	if (	(strcmp(hTunnel->sAdminState,	"enabled") != 0) ||
		(strcmp(hTunnel->sUserState,	"enabled") != 0))
	{
		dolog(LOG_ERR, "Tunnel is not enabled (UserState: %s, AdminState: %s)\n", hTunnel->sAdminState, hTunnel->sUserState);
		return -1;
	}

	/* Do the test thing */
	if (	mode == A_TEST ||
		mode == A_AUTOTEST)
	{
#ifdef _WIN32
		SetConsoleCtrlHandler((PHANDLER_ROUTINE)sigterm_testing, true);
#endif
		/* Setup the tunnel */
		if (aiccu_setup(hTunnel, true))
		{
			aiccu_test(hTunnel, strcasecmp(argv[1], "autotest") == 0 ? true : false);

			/* Tear the tunnel down again */
			aiccu_delete(hTunnel);
		}
		else
		{
			dolog(LOG_ERR, "Tunnel Setup Failed\n");
		}

		/* exit as all is done */
		tic_Free_Tunnel(hTunnel);
		aiccu_FreeConfig();
		return 0;
	}

#ifndef _WIN32
	if (	mode == A_START &&
		g_aiccu->daemonize != 0)
	{
		FILE	*f;

		/* Daemonize */
		int i = fork();
		if (i < 0)
		{
			fprintf(stderr, "Couldn't fork\n");
			return -1;
		}
		/* Exit the mother fork */
		if (i != 0) return 0;

		/* Child fork */
		setsid();

		/* Chdir to minimise disruption to FS umounts */
		(void)chdir("/");

		/* Cleanup stdin/out/err */
		freopen("/dev/null","r",stdin);
		freopen("/dev/null","w",stdout);
		freopen("/dev/null","w",stderr);

		/* */
		f = fopen(g_aiccu->pidfile, "w");
		if (!f)
		{
			dolog(LOG_ERR, "Could not store PID in file %s\n", g_aiccu->pidfile);
			return 0;
		}

		fprintf(f, "%d", getpid());
		fclose(f);

		dolog(LOG_INFO, "AICCU running as PID %d\n", getpid());
	}

#endif /* !_WIN32 */

	/* mode == A_START */

#ifndef _WIN32
	/*
	 * Install a signal handler so that
	 * one can disable beating with SIGUSR1
	 */
	signal(SIGUSR1, &sigusr1);

	/*
	 * Install a signal handler so that
	 * one can stop this program with SIGTERM
	 */
	signal(SIGTERM, &sigterm);
	signal(SIGINT, &sigterm);
#else
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)sigterm, true);
#endif

	/* 
	 * Setup our tunnel
	 * This also spawns required threads for AYIYA
	 */
	if (aiccu_setup(hTunnel, true))
	{
		/* We need to stay running when doing Heartbeat or AYIYA */
		if (	strcasecmp(hTunnel->sType, "6in4-heartbeat") == 0 ||
			strcasecmp(hTunnel->sType, "ayiya") == 0)
		{
			/* We are spawned, now just beat once in a while. */
			while (g_aiccu->running)
			{
				aiccu_beat(hTunnel);
#ifndef _WIN32
				sleep(hTunnel->nHeartbeat_Interval);
#else
				for (i=0; g_aiccu->running && i <= hTunnel->nHeartbeat_Interval; i++) Sleep(1000);
#endif
			}

			/* Clean up the the tunnel, no beat anyway */
			aiccu_delete(hTunnel);
		}

#ifndef _WIN32
		/* Remove our PID file */
		if (g_aiccu) unlink(g_aiccu->pidfile);
#endif
	}

	/* Free our resources */
	aiccu_FreeConfig();

	return 0;
}

