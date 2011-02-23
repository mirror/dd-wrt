/**********************************************************
 SixXS - Automatic IPv6 Connectivity Configuration Utility
***********************************************************
 Copyright 2003-2005 SixXS - http://www.sixxs.net
***********************************************************
 common/tic.c - Tunnel Information & Control Protocol
***********************************************************
 $Author: jeroen $
 $Id: tic.c,v 1.17 2007-01-11 13:41:31 jeroen Exp $
 $Date: 2007-01-11 13:41:31 $
**********************************************************/

#include "common.h"
#include "aiccu.h"
#include "tic.h"

/* Specific includes only used here */
#ifndef _WIN32
#include <sys/utsname.h>
#endif

/* getline vars */
char		tic_buf[2048];
unsigned int	tic_filled;

/* 
 * epochtime = epochtime as received in the packet
 * Don't forget to convert byteorder using ntohl()
 */
int tic_checktime(time_t epochtime)
{
	/* Number of seconds we allow the clock to be off */
	#define CLOCK_OFF 120
	int i;

	/* Get the current time */
	time_t curr_time = time(NULL);

	/* Is one of the times in the loop range? */
	if (	(curr_time >= -CLOCK_OFF) ||
        	(epochtime >= -CLOCK_OFF))
	{
		/* Shift the times out of the loop range */
		i =(int)(((int)curr_time) + (CLOCK_OFF*2)) -
			(((int)epochtime) + (CLOCK_OFF*2));
	}
	else i = ((int)curr_time) - ((int)epochtime);

	/* The clock may be faster, thus flip the sign */
	if (i < 0) i = -i;

	/* Compare the clock offset */
	if (i > CLOCK_OFF)
	{
		/* Time is off */
		return i;
	}

	/* Time is in the allowed range */
	return 0;
}

bool tic_Login(struct TIC_conf *tic, const char *username, const char *password, const char *server)
{
	char		buf[1024], sSignature[33], sChallenge[1024];
	int		i;
#ifndef _WIN32
	struct utsname	uts_name;
#else
	OSVERSIONINFO	osv;
	OSVERSIONINFOEX	osvEx;
	char		*platform = NULL;
	char		version[100];
#endif

	D(dolog(LOG_DEBUG, "Trying to connect to TIC server %s\n", server));

	/* Connect to the TIC server */
	tic->sock = connect_client(server, TIC_PORT, AF_INET, SOCK_STREAM);
	if (!tic->sock)
	{
		dolog(LOG_ERR, "Couldn't connect to the TIC server %s\n", server);
		return false;
	}

	/* Fetch the welcome */
	if (sock_getline(tic->sock, tic_buf, sizeof(tic_buf), &tic_filled, buf, sizeof(buf)) == -1)
	{
		return false;
	}
	if (buf[0] != '2')
	{
		dolog(LOG_ERR, "TIC Server is currently not available\n");
		return false;
	}

	/* Send our client identification */
#ifndef _WIN32
	uname(&uts_name);
	sock_printf(tic->sock, "client TIC/%s %s/%s %s/%s\n",
		TIC_VERSION,
		TIC_CLIENT_NAME, TIC_CLIENT_VERSION,
		uts_name.sysname, uts_name.release);
#else
	osv.dwOSVersionInfoSize	= sizeof(OSVERSIONINFO);
	osvEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if (!GetVersionEx(&osv))
	{
		platform = "Windows";
		snprintf(version, sizeof(version), "%s", "Unknown");
	}
	else
	{

		platform = (osv.dwPlatformId  == VER_PLATFORM_WIN32s)		? "Win32s" :
			   ((osv.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)	? "Win9x" :
			   ((osv.dwPlatformId == VER_PLATFORM_WIN32_NT)		? "WinNT" :
										  "Windows"));

		if (	osv.dwMajorVersion < 5 ||
			!GetVersionEx((OSVERSIONINFO *)&osvEx) ||
			osvEx.wServicePackMajor <= 0)
		{
			snprintf(version, sizeof(version), "%d.%d.%d",
				osv.dwMajorVersion, osv.dwMinorVersion, osv.dwBuildNumber);
		}
		else if (osvEx.wServicePackMinor <= 0)
		{
			snprintf(version, sizeof(version), "%d.%d.%d-SP%d",
				osv.dwMajorVersion, osv.dwMinorVersion, osv.dwBuildNumber,
				osvEx.wServicePackMajor);
		}
		else
		{
			snprintf(version, sizeof(version), "%d.%d.%d-SP%d.%d",
				osv.dwMajorVersion, osv.dwMinorVersion, osv.dwBuildNumber,
				osvEx.wServicePackMajor, osvEx.wServicePackMinor);
		}
	}
	sock_printf(tic->sock, "client TIC/%s %s/%s %s/%s\n",
		TIC_VERSION,
		TIC_CLIENT_NAME, TIC_CLIENT_VERSION,
		platform, version);
#endif

	/* Fetch the answer */
	if (sock_getline(tic->sock, tic_buf, sizeof(tic_buf), &tic_filled, buf, sizeof(buf)) == -1)
	{
		return false;
	}
	if (buf[0] != '2')
	{
		dolog(LOG_ERR, "Couldn't pass client information: %s.\n", &buf[4]);
		return false;
	}

	/* Request current time */
	sock_printf(tic->sock, "get unixtime\n");

	/* Fetch the answer */
	if (sock_getline(tic->sock, tic_buf, sizeof(tic_buf), &tic_filled, buf, sizeof(buf)) == -1)
	{
		return false;
	}
	if (buf[0] != '2')
	{
		dolog(LOG_ERR, "Time not available? %s\n", &buf[4]);
		return false;
	}

	/* Check if the time is correct */
	i = tic_checktime(atoi(&buf[4]));
	if (i != 0)
	{
		char quitmsg[100];
		dolog(LOG_ERR, "The clock is off by %d seconds, use NTP to sync it!\n", i);
		snprintf(quitmsg, sizeof(quitmsg), "Aborting: Clock is off by %d seconds\n", i);
		tic_Logout(tic, quitmsg);
		return false;
	}

#ifdef AICCU_GNUTLS
  /* Upgrade to TLS */
	sock_printf(tic->sock, "starttls\n");

	/* Fetch the welcome */
	if (sock_getline(tic->sock, tic_buf, sizeof(tic_buf), &tic_filled, buf, sizeof(buf)) == -1)
	{
		return false;
	}
	if (buf[0] == '2')
	{
		/* Go to TLS mode */
		if (!sock_gotls(tic->sock)) return false;
	}
	else
	{
		if (g_aiccu->requiretls)
		{
			dolog(LOG_ERR, "TIC Server does not support TLS and TLS is required\n");
			return false;
		}
		if (g_aiccu->verbose) dolog(LOG_WARNING, "TIC Server does not support TLS but TLS is not required, continuing\n");
	}

#endif

	/* Send our username */
	sock_printf(tic->sock, "username %s\n", username);

	/* Fetch the answer */
	if (sock_getline(tic->sock, tic_buf, sizeof(tic_buf), &tic_filled, buf, sizeof(buf)) == -1)
	{
		return false;
	}
	if (buf[0] != '2')
	{
		dolog(LOG_ERR, "Username not accepted: %s.\n", &buf[4]);
		return false;
	}
	
	/* Pick a challenge */
	sock_printf(tic->sock, "challenge md5\n");

	/* Fetch the answer */
	if (sock_getline(tic->sock, tic_buf, sizeof(tic_buf), &tic_filled, buf, sizeof(buf)) == -1)
	{
		return false;
	}
	if (buf[0] != '2')
	{
		dolog(LOG_ERR, "Challenge not correct: %s.\n", &buf[4]);
		return false;
	}

	/* Send the response */
	/* sSignature = md5(challenge.md5(password)); */
	MD5String(password, sSignature, sizeof(sSignature));
	snprintf(sChallenge, sizeof(sChallenge), "%s%s", &buf[4], sSignature);
	MD5String(sChallenge, sSignature, sizeof(sSignature));

	sock_printf(tic->sock, "authenticate md5 %s\n", sSignature);

	/* Fetch the answer */
	if (sock_getline(tic->sock, tic_buf, sizeof(tic_buf), &tic_filled, buf, sizeof(buf)) == -1)
	{
		tic_Logout(tic, NULL);
		return false;
	}
	if (buf[0] != '2')
	{
		tic_Logout(tic, NULL);
		dolog(LOG_ERR, "Response not accepted: %s.\n", &buf[4]);
		return false;
	}

	/* Connect OK */
	return true;
}

void tic_Logout(struct TIC_conf *tic, const char *quitmsg)
{
	/* A list of appropriate quit messages */
	const char *byers[] = {
		/* Swiss-German form of "Ciao" */
		"Tschau!",

		/* Dutch for "they who are going, greet you" */
		"Zij die gaan, groeten u",
		"See you later alligator",
		"A bitter thought, but I have to go",

		/* Dutch for "see you later" */
		"Ajuuu paraplu",
		"Thank you for the information",
		"It was lovely talking to you again",
		"Tschussss...",
		"Aufwiedersehen",
		"I'll be back. Ha, you didn't know I was going to say that!",
		"We will be the only two people left in the world, Yes--Adam and Evil!",

		/* Blutengel */
		"Stranded",
		"Die With You",
		"The End Of Love",

		/* Chamber */
		"In My Garden",
		"Set Me Free",

		/* Faithless */
		"Don't Leave",
		"Insomnia",
		"Why Go",

		/* Garbage */
		"The Trick Is To Keep Breathing",

		/* The Gathering */
		"We just stopped breating",
		"Even the spirits are afraid",

		/* Goldfrapp */
		"Deer Stop",

		/* Hooverphonic */
		"The Last Thing I Need Is You",
		"Every Time We Live Together",
		"My Autumn's Done Come",

		/* Infected Mushroom */
		"Never Ever Land",
		"None of this is real",
		"Nothing Comes Easy",
		"Illuminaughty",

		/* Nine Inch Nails */
		"Something I can never have",
		"And All That Could Have Been...",
		"That's what I get",

		/* Opeth */
		"Under the weeping moon",
		"For Absent Friends",

		/* Portishead */
		"It Could Be Sweet",
		"Half Day Closing",

		/* Suicide Commando */
		"Better Off Dead",

		/* VNV Nation */
		"Solitary",
		"Forsaken",
		"Holding On",

		/* Within Temptation */
		"This is not our farewell",
		"Running Down That Hill",

		/* Wumpscut */
		"Schaltet den schmerz ab",
		"Down where we belong",
	};

	/* Already disconnected? */
	if (!tic->sock) return;

	if (!quitmsg)
	{
		/* Stupid random quit messages, got to put some form of easteregg in it :) */
		srand((unsigned)time(NULL));

		quitmsg = (char *)byers[rand()%(sizeof(byers)/sizeof(char *))];
	}

	/* Send our bye bye */
	sock_printf(tic->sock, "QUIT %s\n", quitmsg);

	/* Disconnect */
	sock_free(tic->sock);
	tic->sock = NULL;
}

struct TIC_sTunnel *tic_ListTunnels(struct TIC_conf *tic)
{
	char			buf[1024], buf2[1024];
	struct TIC_sTunnel	*start = NULL, *last = NULL, *tun = NULL;
	int			i;

/* Request a list of Tunnels */
	sock_printf(tic->sock, "tunnel list\n");

	/* Fetch the answer */
	if (sock_getline(tic->sock, tic_buf, sizeof(tic_buf), &tic_filled, buf, sizeof(buf)) == -1)
	{
		return NULL;
	}

	/* 201 (start of list) ? */
	if (buf[0] != '2' || buf[1] != '0' || buf[2] != '1')
	{
		dolog(LOG_ERR, "Couldn't list tunnels: %s.\n", &buf[4]);
		return NULL;
	}

	/* Process all the lines */
	while (sock_getline(tic->sock, tic_buf, sizeof(tic_buf), &tic_filled, buf, sizeof(buf)) != -1)
	{
		/* 202 (end of list) ? */
		if (buf[0] == '2' && buf[1] == '0' && buf[2] == '2') break;
		
		i = countfields(buf);
		if (i != 4)
		{
			dolog(LOG_ERR, "Wrong field format when listing tunnels\n");
			break;
		}

		/* Allocate a new struct */
		tun = (struct TIC_sTunnel *)malloc(sizeof(*tun));
		if (!tun)
		{
			dolog(LOG_ERR, "Memory problem while listing tunnels\n");
			break;
		}
		memset(tun, 0, sizeof(*tun));

		/* Copy the fields into the struct */
		if (!copyfield(buf, 1, buf2, sizeof(buf2))) break;
		tun->sId = strdup(buf2);
		if (!copyfield(buf, 2, buf2, sizeof(buf2))) break;
		tun->sIPv6 = strdup(buf2);
		if (!copyfield(buf, 3, buf2, sizeof(buf2))) break;
		tun->sIPv4 = strdup(buf2);
		if (!copyfield(buf, 4, buf2, sizeof(buf2))) break;
		tun->sPOPId = strdup(buf2);
		
		/* Add it into the list */
		if (last)
		{
			last->next = tun;
			last = tun;
		}
		else
		{
			start = last = tun;
		}
	}

	/* All went okay? */
	if (buf[0] == '2' && buf[1] == '0' && buf[2] == '2')
	{
		return start;
	}

	/* Free the structure, it was broken anyway */
	tic_Free_sTunnel(start);

	dolog(LOG_ERR, "Tunnel list went wrong: %s\n", &buf[4]);
	return NULL;
}

struct TIC_sRoute *tic_ListRoutes(struct TIC_conf *tic)
{
	dolog(LOG_ERR, "Not implemented - tic_ListRoutes(%x)\n", tic);
	return NULL;
}

struct TIC_sPOP *tic_ListPOPs(struct TIC_conf *tic)
{
	dolog(LOG_ERR, "Not implemented - tic_ListPOPs(%x)\n", tic);
	return NULL;
}

struct pl_rule tunnel_rules[] =
{
	{"TunnelId",		PLRT_STRING,	offsetof(struct TIC_Tunnel, sId)},
	{"Type",		PLRT_STRING,	offsetof(struct TIC_Tunnel, sType)},
	{"IPv6 Endpoint",	PLRT_STRING,	offsetof(struct TIC_Tunnel, sIPv6_Local)},
	{"IPv6 POP",		PLRT_STRING,	offsetof(struct TIC_Tunnel, sIPv6_POP)},
	{"IPv6 PrefixLength",	PLRT_INTEGER,	offsetof(struct TIC_Tunnel, nIPv6_PrefixLength)},
	{"POP Id",		PLRT_STRING,	offsetof(struct TIC_Tunnel, sPOP_Id)},
	{"IPv4 Endpoint",	PLRT_STRING,	offsetof(struct TIC_Tunnel, sIPv4_Local)},
	{"IPv4 POP",		PLRT_STRING,	offsetof(struct TIC_Tunnel, sIPv4_POP)},
	{"UserState",		PLRT_STRING,	offsetof(struct TIC_Tunnel, sUserState)},
	{"AdminState",		PLRT_STRING,	offsetof(struct TIC_Tunnel, sAdminState)},
	{"Password",		PLRT_STRING,	offsetof(struct TIC_Tunnel, sPassword)},
	{"Heartbeat_Interval",	PLRT_INTEGER,	offsetof(struct TIC_Tunnel, nHeartbeat_Interval)},
	{"Tunnel MTU",		PLRT_INTEGER,	offsetof(struct TIC_Tunnel, nMTU)},
	{NULL,			PLRT_END,	0},
};

struct TIC_Tunnel *tic_GetTunnel(struct TIC_conf *tic, const char *sId)
{
	char			buf[1024];
	struct TIC_Tunnel	*tun;

	/* Get a Tunnel */
	sock_printf(tic->sock, "tunnel show %s\n", sId);

	/* Fetch the answer */
	if (sock_getline(tic->sock, tic_buf, sizeof(tic_buf), &tic_filled, buf, sizeof(buf)) == -1)
	{
		return NULL;
	}

	/* 201 (start of information) ? */
	if (buf[0] != '2' || buf[1] != '0' || buf[2] != '1')
	{
		dolog(LOG_ERR, "Couldn't show tunnel %s: %s.\n", sId, buf);
		return NULL;
	}

	/* Allocate a new struct */
	tun = (struct TIC_Tunnel *)malloc(sizeof(*tun));
	if (!tun)
	{
		dolog(LOG_ERR, "Memory problem while getting tunnel %s\n", sId);
		return NULL;
	}
	memset(tun, 0, sizeof(*tun));

	/* Gather the information */
	while (sock_getline(tic->sock, tic_buf, sizeof(tic_buf), &tic_filled, buf, sizeof(buf)) != -1)
	{
		/* 202 (end of list) ? */
		if (buf[0] == '2' && buf[1] == '0' && buf[2] == '2') break;
		
		parseline(buf, ": ", tunnel_rules, tun);
	}
	/* All went okay? */
	if (buf[0] == '2' && buf[1] == '0' && buf[2] == '2')
	{
		struct in6_addr ipv6_ll, ipv6_local;
		char ll[100];

		/* Log that the fetch was succesful */
		dolog(LOG_INFO, "Succesfully retrieved tunnel information for %s\n", sId);

		/*
		 * Some TUN/TAP devices don't have any
		 * link local addresses and we want multicast and MLD to work
		 * thus we invent one based on the following:
		 *
		 * ipv6_us = 2001:0db8:1234:5678:    :    :    :0001
		 * ipv6_ll = fe80:    :    :    :0db8:1234:5678:0001
		 *
		 * Thus we ignore the first 16bits, take the following 48 bits
		 * and then add the last 16bits.
		 *
		 * As we are not 100% sure that this LL is unique we clear that bit.
                */

		inet_pton(AF_INET6, tun->sIPv6_Local, &ipv6_local);

		/* Link Local (fe80::/64) */
		ipv6_ll.s6_addr[ 0] = 0xfe;
		ipv6_ll.s6_addr[ 1] = 0x80;
		ipv6_ll.s6_addr[ 2] = 0x00;
		ipv6_ll.s6_addr[ 3] = 0x00;
		ipv6_ll.s6_addr[ 4] = 0x00;
		ipv6_ll.s6_addr[ 5] = 0x00;
		ipv6_ll.s6_addr[ 6] = 0x00;
		ipv6_ll.s6_addr[ 7] = 0x00;
		ipv6_ll.s6_addr[ 8] = ipv6_local.s6_addr[ 2] & 0xfc; /* Clear the LL Unique Bit */
		ipv6_ll.s6_addr[ 9] = ipv6_local.s6_addr[ 3];
		ipv6_ll.s6_addr[10] = ipv6_local.s6_addr[ 4];
		ipv6_ll.s6_addr[11] = ipv6_local.s6_addr[ 5];
		ipv6_ll.s6_addr[12] = ipv6_local.s6_addr[ 6];
		ipv6_ll.s6_addr[13] = ipv6_local.s6_addr[ 7];
		ipv6_ll.s6_addr[14] = ipv6_local.s6_addr[14];
		ipv6_ll.s6_addr[15] = ipv6_local.s6_addr[15];

		inet_ntop(AF_INET6, &ipv6_ll, ll, sizeof(ll));
		if (tun->sIPv6_LinkLocal) free(tun->sIPv6_LinkLocal);
		tun->sIPv6_LinkLocal = strdup(ll);

		if (	strcmp(tun->sType, "ayiya") == 0 ||
			strcmp(tun->sType, "l2tp") == 0)
		{
			tun->uses_tundev = 1;
#ifdef NO_IFHEAD
			dolog(LOG_ERR, "This build doesn't support the Tun/TAP device and thus can't instantiate tunnels of type %s, please fix your OS and recompile\n", tun->sType);
			tic_Free_Tunnel(tun);
			return NULL;
#endif
		}
		else tun->uses_tundev = 0;

		/* Need to override the local IPv4 address? */
		if (g_aiccu->local_ipv4_override)
		{
			dolog(LOG_INFO, "Overriding Local IPv4 address from %s to %s\n", tun->sIPv4_Local, g_aiccu->local_ipv4_override);
			free(tun->sIPv4_Local);
			tun->sIPv4_Local = strdup(g_aiccu->local_ipv4_override);
		}

		return tun;
	}

	/* Free the structure, it is broken anyway */
	tic_Free_Tunnel(tun);

	dolog(LOG_ERR, "Tunnel Get for %s went wrong: %s\n", sId, buf);
	return NULL;
}

struct TIC_Route *tic_GetRoute(struct TIC_conf *tic, const char *sId)
{
	dolog(LOG_ERR, "Not implemented - tic_GetRoute(%x, \"%s\")\n", tic, sId);
	return NULL;
}

struct pl_rule pop_rules[] =
{
	{"POPId",		PLRT_STRING,	offsetof(struct TIC_POP, sId)},
	{"City",		PLRT_STRING,	offsetof(struct TIC_POP, sCity)},
	{"Country",		PLRT_STRING,	offsetof(struct TIC_POP, sCountry)},
	{"IPv4",		PLRT_STRING,	offsetof(struct TIC_POP, sIPv4)},
	{"IPv6",		PLRT_STRING,	offsetof(struct TIC_POP, sIPv6)},

	{"ISP Short",		PLRT_STRING,	offsetof(struct TIC_POP, sISP_Short)},
	{"ISP Name",		PLRT_STRING,	offsetof(struct TIC_POP, sISP_Name)},
	{"ISP Website",		PLRT_STRING,	offsetof(struct TIC_POP, sISP_Website)},
	{"ISP ASN",		PLRT_STRING,	offsetof(struct TIC_POP, sISP_ASN)},
	{"ISP LIR",		PLRT_STRING,	offsetof(struct TIC_POP, sISP_LIR)},

	{NULL,			PLRT_END,	0},
};

struct TIC_POP *tic_GetPOP(struct TIC_conf *tic, const char *sId)
{
	char			buf[1024];
	struct TIC_POP		*pop;

	/* Get a Tunnel */
	sock_printf(tic->sock, "pop show %s\n", sId);

	/* Fetch the answer */
	if (sock_getline(tic->sock, tic_buf, sizeof(tic_buf), &tic_filled, buf, sizeof(buf)) == -1)
	{
		return NULL;
	}

	/* 201 (start of info) ? */
	if (buf[0] != '2' || buf[1] != '0' || buf[2] != '1')
	{
		dolog(LOG_ERR, "Couldn't show POP %s: %s.\n", sId, buf);
		return NULL;
	}

	/* Allocate a new struct */
	pop = (struct TIC_POP *)malloc(sizeof(*pop));
	if (!pop)
	{
		dolog(LOG_ERR, "Memory problem while getting POP\n");
		return NULL;
	}
	memset(pop, 0, sizeof(*pop));

	/* Gather the information */
	while (sock_getline(tic->sock, tic_buf, sizeof(tic_buf), &tic_filled, buf, sizeof(buf)) != -1)
	{
		/* 202 (end of list) ? */
		if (buf[0] == '2' && buf[1] == '0' && buf[2] == '2') break;
		
		parseline(buf, ": ", pop_rules, pop);
	}
	/* All went okay? */
	if (buf[0] == '2' && buf[1] == '0' && buf[2] == '2')
	{
		dolog(LOG_INFO, "Succesfully retrieved POP information for %s\n", sId);
		return pop;
	}

	/* Free the structure, it is broken anyway */
	tic_Free_POP(pop);

	dolog(LOG_ERR, "POP Get for %s went wrong: %s\n", sId, buf);
	return NULL;
}

void tic_Free_sTunnel(struct TIC_sTunnel *tun)
{
	struct TIC_sTunnel *next;

	for (; tun; tun = next)
	{
		next = tun->next;
		if (tun->sId)		free(tun->sId);
		if (tun->sIPv6)		free(tun->sIPv6);
		if (tun->sIPv4)		free(tun->sIPv4);
		if (tun->sPOPId)	free(tun->sPOPId);
		free(tun);
	}
}

void tic_Free_sRoute(struct TIC_sRoute *rt)
{
	struct TIC_sRoute *next;

	for (; rt; rt = next)
	{
		next = rt->next;
		if (rt->sId)		free(rt->sId);
		if (rt->sTunnelId)	free(rt->sTunnelId);
		if (rt->sIPv6)		free(rt->sIPv6);
		free(rt);
	}
}

void tic_Free_sPOP(struct TIC_sPOP *pop)
{
	struct TIC_sPOP *next;

	for (; pop; pop = next)
	{
		next = pop->next;
		if (pop->sId)	free(pop->sId);
		free(pop);
	}
}

void tic_Free_Tunnel(struct TIC_Tunnel *tun)
{
	if (tun->sId)		{ free(tun->sId);		tun->sId		= NULL; }
	if (tun->sType)		{ free(tun->sType);		tun->sType		= NULL; }
	if (tun->sPOP_Id)	{ free(tun->sPOP_Id);		tun->sPOP_Id		= NULL; }
	if (tun->sUserState)	{ free(tun->sUserState);	tun->sUserState		= NULL; }
	if (tun->sAdminState)	{ free(tun->sAdminState);	tun->sAdminState	= NULL; }
	if (tun->sPassword)	{ free(tun->sPassword);		tun->sPassword		= NULL; }
	if (tun->sIPv4_Local)	{ free(tun->sIPv4_Local);	tun->sIPv4_Local	= NULL; }
	if (tun->sIPv4_POP)	{ free(tun->sIPv4_POP);		tun->sIPv4_POP		= NULL; }
	if (tun->sIPv6_Local)	{ free(tun->sIPv6_Local);	tun->sIPv6_Local	= NULL; }
	if (tun->sIPv6_POP)	{ free(tun->sIPv6_POP);		tun->sIPv6_POP		= NULL;	}
	free(tun);
	tun = NULL;
}

void tic_Free_Route(struct TIC_Route *rt)
{
	if (rt->sId)		free(rt->sId);
	if (rt->sTunnelId)	free(rt->sTunnelId);
	free(rt);
}

void tic_Free_POP(struct TIC_POP *pop)
{
	if (pop->sId)		free(pop->sId);
	if (pop->sCity)		free(pop->sCity);
	if (pop->sCountry)	free(pop->sCountry);
	if (pop->sIPv4)		free(pop->sIPv4);
	if (pop->sIPv6)		free(pop->sIPv6);
	if (pop->sISP_Short)	free(pop->sISP_Short);
	if (pop->sISP_Name)	free(pop->sISP_Name);
	if (pop->sISP_Website)	free(pop->sISP_Website);
	if (pop->sISP_ASN)	free(pop->sISP_ASN);
	if (pop->sISP_LIR)	free(pop->sISP_LIR);

	free(pop);
}
