/**********************************************************
 SixXS - Automatic IPv6 Connectivity Configuration Utility
***********************************************************
 Copyright 2003-2005 SixXS - http://www.sixxs.net
***********************************************************
 common/aiccu.c - AICCU Abstracted functions
***********************************************************
 $Author: jeroen $
 $Id: aiccu.c,v 1.20 2007-01-15 12:02:10 jeroen Exp $
 $Date: 2007-01-15 12:02:10 $
**********************************************************/

#include "aiccu.h"

struct AICCU_conf *g_aiccu = NULL;

/* Config */
struct pl_rule aiccu_conf_rules[] =
{
	/* Configuration */
	{"username",		PLRT_STRING,	offsetof(struct AICCU_conf, username)},
	{"password",		PLRT_STRING,	offsetof(struct AICCU_conf, password)},
	{"protocol",		PLRT_STRING,	offsetof(struct AICCU_conf, protocol)},
	{"server",		PLRT_STRING,	offsetof(struct AICCU_conf, server)},
	{"ipv6_interface",	PLRT_STRING,	offsetof(struct AICCU_conf, ipv6_interface)},
	{"tunnel_id",		PLRT_STRING,	offsetof(struct AICCU_conf, tunnel_id)},
	{"local_ipv4_override",	PLRT_STRING,	offsetof(struct AICCU_conf, local_ipv4_override)},

	/* Post Setup script path */
	{"setupscript",		PLRT_STRING,	offsetof(struct AICCU_conf, setupscript)},

	/* Automatic */
	{"automatic",		PLRT_BOOL,	offsetof(struct AICCU_conf, automatic)},

	/* Operational options */
	{"daemonize",		PLRT_BOOL,	offsetof(struct AICCU_conf, daemonize)},
	{"verbose",		PLRT_BOOL,	offsetof(struct AICCU_conf, verbose)},
	{"behindnat",		PLRT_BOOL,	offsetof(struct AICCU_conf, behindnat)},
	{"requiretls",		PLRT_BOOL,	offsetof(struct AICCU_conf, requiretls)},
	{"noconfigure",		PLRT_BOOL,	offsetof(struct AICCU_conf, noconfigure)},
	{"makebeats",		PLRT_BOOL,	offsetof(struct AICCU_conf, makebeats)},
	{"defaultroute",	PLRT_BOOL,	offsetof(struct AICCU_conf, defaultroute)},
	{"pidfile",		PLRT_STRING,	offsetof(struct AICCU_conf, pidfile)},
	{NULL,			PLRT_END,	0},
};

#ifdef AICCU_GNUTLS
void aiccu_tls_log(int level, const char *message);
void aiccu_tls_log(int level, const char *message)
{
	dolog(level, "[GNUTLS] %s\n", message);
}
#endif

bool aiccu_InitConfig()
{
#ifdef AICCU_GNUTLS
	int ret;
#define CAFILE "ca.pem"
#endif
	/* Allocate & Initialize */
	g_aiccu = (struct AICCU_conf *)malloc(sizeof(*g_aiccu));
	if (!g_aiccu) return false;
	memset(g_aiccu, 0, sizeof(*g_aiccu));
	g_aiccu->tic = (struct TIC_conf *)malloc(sizeof(*g_aiccu->tic));
	memset(g_aiccu->tic, 0, sizeof(*g_aiccu->tic));

	/* Initialize config to defaults */
	g_aiccu->running	= true;
	g_aiccu->tunrunning	= false;
	g_aiccu->daemonize	= 0;
	g_aiccu->verbose	= false;
	g_aiccu->requiretls	= false;		/* Not mandatory yet */
	g_aiccu->noconfigure	= false;
	g_aiccu->makebeats	= true;
	g_aiccu->defaultroute	= true;
	g_aiccu->ipv6_interface	= strdup("aiccu");
	if (!g_aiccu->ipv6_interface) return false;
	g_aiccu->protocol	= strdup("tic");
	if (!g_aiccu->protocol) return false;
	g_aiccu->server		= strdup("tic.sixxs.net");
	if (!g_aiccu->server) return false;
	g_aiccu->pidfile	= strdup(AICCU_PID);
	if (!g_aiccu->pidfile) return false;

#ifdef AICCU_GNUTLS
	/* Initialize GNUTLS */
	ret = gnutls_global_init();
	if (ret != 0)
	{
		dolog(LOG_ERR, "GNUTLS failed to initialize: %s (%d)\n", gnutls_strerror(ret), ret);
		return false;
	}

	/* X509 credentials */
	ret = gnutls_certificate_allocate_credentials(&g_aiccu->tls_cred);
	if (ret != 0)
	{
		dolog(LOG_ERR, "GNUTLS failed to initialize: %s (%d)\n", gnutls_strerror(ret), ret);
		return false;
	}

	/* For the time being don't load the PEM as it is not there... */

#if 0
	/* Sets the trusted cas file */
 	ret = gnutls_certificate_set_x509_trust_file(g_aiccu->tls_cred, CAFILE, GNUTLS_X509_FMT_PEM);
	if (ret < 0)
	{
		dolog(LOG_ERR, "GNUTLS failed to initialize: %s (%d)\n", gnutls_strerror(ret), ret);
		return false;
	}
#endif

	/* Configure GNUTLS logging to happen using our own logging interface */
	gnutls_global_set_log_function(aiccu_tls_log);

#ifdef DEBUG
	/* Show some GNUTLS debugging information */
	gnutls_global_set_log_level(5);
#endif

#endif /* AICCU_GNUTLS */

	return true;
}

/* Locate where the configfile is stored */
void aiccu_LocateFile(const char *what, char *filename, unsigned int length);
void aiccu_LocateFile(const char *what, char *filename, unsigned int length)
{
	memset(filename, 0, length);
#ifdef _WIN32
	/* Figure out the "C:\Windows" location */
	/* as that is where we store our configuration */
	GetWindowsDirectory(filename, length);
	strncat(filename, "\\", length);
	strncat(filename, what, length);
#else
	/* Use the default location */
	strncat(filename, what, length);
#endif
}

/* configure this client */
bool aiccu_LoadConfig(const char *filename)
{
	FILE			*f;
	char			buf[1000];
	char			filenames[256];
	unsigned int		line = 0;

	if (!filename)
	{
		aiccu_LocateFile(AICCU_CONFIG, filenames, sizeof(filenames));
		filename = filenames;
	}

	f = fopen(filename, "r");
	if (!f)
	{
		dolog(LOG_ERR, "Could not open config file \"%s\"\n", filename);
		return false;
	}

	while (fgets(buf, sizeof(buf), f))
	{
		line++;
		if (parseline(buf, " ", aiccu_conf_rules, g_aiccu)) continue;

		dolog(LOG_WARNING, "Unknown configuration statement on line %u of %s: \"%s\"\n", line, filename, buf);
	}
	fclose(f);

	return true;
}

/* Save the configuration */
bool aiccu_SaveConfig(const char *filename)
{
	FILE *f;
	char filenames[512];

	if (!filename)
	{
		aiccu_LocateFile(AICCU_CONFIG, filenames, sizeof(filenames));
		filename = filenames;
	}

	f = fopen(filename, "w");
	if (!f)
	{
		dolog(LOG_ERR, "Could not open config file \"%s\" for writing\n", filename);
		return false;
	}

	fprintf(f, "# AICCU Configuration (Saved by AICCU %s)\n", AICCU_VER);
	fprintf(f, "\n");
	fprintf(f, "# Login information\n");
	fprintf(f, "username %s\n", g_aiccu->username);
	fprintf(f, "password %s\n", g_aiccu->password);
	fprintf(f, "protocol %s\n", g_aiccu->protocol);
	fprintf(f, "server %s\n", g_aiccu->server);
	fprintf(f, "\n");
	fprintf(f, "# Interface names to use\n");
	fprintf(f, "ipv6_interface %s\n", g_aiccu->ipv6_interface);
	fprintf(f, "\n");
	fprintf(f, "# The tunnel_id to use\n");
	fprintf(f, "# (only required when there are multiple tunnels in the list)\n");
	fprintf(f, "tunnel_id %s\n", g_aiccu->tunnel_id);
	fprintf(f, "\n");
	fprintf(f, "# Try to automatically login and setup the tunnel?\n");
	fprintf(f, "automatic %s\n", g_aiccu->automatic ? "true" : "false");
	fprintf(f, "\n");
	fprintf(f, "# Script to run after setting up the interfaces (default: none)\n");
	fprintf(f, "%ssetupscript %s\n", g_aiccu->setupscript ? "" : "#", g_aiccu->setupscript ? g_aiccu->setupscript : "<path>");
	fprintf(f, "\n");
	fprintf(f, "# TLS Required?\n");
	fprintf(f, "requiretls %s\n", g_aiccu->requiretls ? "true" : "false");
	fprintf(f, "\n");
	fprintf(f, "# Be verbose?\n");
	fprintf(f, "verbose %s\n", g_aiccu->verbose ? "true" : "false");
	fprintf(f, "\n");
	fprintf(f, "# Daemonize?\n");
	fprintf(f, "daemonize %s\n", g_aiccu->daemonize ? "true" : "false");
	fprintf(f, "\n");
	fprintf(f, "# Behind NAT (default: false)\n");
	fprintf(f, "# Notify the user that a NAT-kind network is detected\n");
	fprintf(f, "behindnat %s\n", g_aiccu->behindnat ? "true" : "false");
	fprintf(f, "\n");
	fprintf(f, "# PID File\n");
	fprintf(f, "pidfile %s\n", g_aiccu->pidfile);
	fprintf(f, "\n");
	fprintf(f, "# Make heartbeats (default true)\n");
	fprintf(f, "# In general you don't want to turn this off\n");
	fprintf(f, "# Of course only applies to AYIYA and heartbeat tunnels not to static ones\n");
	fprintf(f, "makebeats %s\n", g_aiccu->makebeats ? "true" : "false");
	fprintf(f, "\n");
	fprintf(f, "# Add a default route (default: true)\n");
	fprintf(f, "defaultroute %s\n", g_aiccu->defaultroute ? "true" : "false");
	fprintf(f, "\n");
	fprintf(f, "# Don't configure anything (default: false)\n");
	fprintf(f, "noconfigure %s\n", g_aiccu->noconfigure ? "true" : "false");
	fclose(f);
	return true;
}

void aiccu_FreeConfig()
{
	if (!g_aiccu) return;

#ifdef AICCU_GNUTLS
	gnutls_certificate_free_credentials(g_aiccu->tls_cred);
	gnutls_global_deinit();
#endif

	if (g_aiccu->username)		{ free(g_aiccu->username);	g_aiccu->username	= NULL; }
	if (g_aiccu->password)		{ free(g_aiccu->password);	g_aiccu->password	= NULL; }
	if (g_aiccu->ipv6_interface)	{ free(g_aiccu->ipv6_interface);g_aiccu->ipv6_interface	= NULL; }
	if (g_aiccu->tunnel_id)		{ free(g_aiccu->tunnel_id);	g_aiccu->tunnel_id	= NULL; }
	if (g_aiccu->tic)		{ free(g_aiccu->tic);		g_aiccu->tic		= NULL; }
	if (g_aiccu->setupscript)	{ free(g_aiccu->setupscript);	g_aiccu->setupscript	= NULL; }
	if (g_aiccu->pidfile)		{ free(g_aiccu->pidfile);	g_aiccu->pidfile	= NULL; }

	free(g_aiccu);
	g_aiccu = NULL;
}

/* Make sure the OS understands IPv6 */
void aiccu_install(void)
{
	D(dolog(LOG_DEBUG, "aiccu_install()\n");)
	aiccu_os_install();
}

bool aiccu_setup(struct TIC_Tunnel *hTunnel, bool firstrun)
{
	bool ret = false;

	D(dolog(LOG_DEBUG, "aiccu_setup(%s, %s)\n", hTunnel->sIPv6_Local, firstrun ? "first" : "other");)

	/* AYIYA calls aiccu_setup(hTunnel,false) after preparing the tunnel interface */
	if (firstrun && strcasecmp(hTunnel->sType, "ayiya") == 0)
	{
		ret = ayiya(hTunnel);
	}
#ifdef NEWSTUFF_TEEPEE
	else if (firstrun && strcasecmp(hTunnel->sType, "l2tp") == 0)
	{
		ret = teepee(hTunnel);
	}
#endif
	else
	{
		ret = aiccu_os_setup(hTunnel);
	}

	/* Beat for the first time */
	if (ret) aiccu_beat(hTunnel);

	return ret;
}

void aiccu_beat(struct TIC_Tunnel *hTunnel)
{
	if (!g_aiccu->makebeats)
	{
		D(dolog(LOG_DEBUG, "aiccu_beat() - Beating disabled\n"));
		return;
	}

	D(dolog(LOG_DEBUG, "aiccu_beat() - Beating %s...\n", hTunnel->sType));

	if (strcasecmp(hTunnel->sType, "6in4-heartbeat") == 0)
	{
		heartbeat_beat(hTunnel);
	}
	else if (strcasecmp(hTunnel->sType, "ayiya") == 0)
	{
		ayiya_beat();
	}
	else
	{
		D(dolog(LOG_DEBUG, "aiccu_beat() - No beat for %s!?\n", hTunnel->sType));
	}

	/* L2TP Hello's are handled inside TeePee */
}

void aiccu_reconfig(struct TIC_Tunnel *hTunnel)
{
	D(dolog(LOG_DEBUG, "aiccu_reconfig(%s)\n", hTunnel->sIPv6_Local);)
	if (!g_aiccu->noconfigure) aiccu_os_reconfig(hTunnel);
}

void aiccu_delete(struct TIC_Tunnel *hTunnel)
{
	D(dolog(LOG_DEBUG, "aiccu_delete(%s)\n", hTunnel->sIPv6_Local);)
	if (!g_aiccu->noconfigure) aiccu_os_delete(hTunnel);
}

void aiccu_test(struct TIC_Tunnel *hTunnel, bool automatic)
{
	D(dolog(LOG_DEBUG, "aiccu_test()\n"));
	aiccu_os_test(hTunnel, automatic);
}

bool aiccu_exec(const char *fmt, ...)
{
#ifndef _WIN32
	char buf[1024];
	int ret;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf,sizeof(buf),fmt,ap);
	D(dolog(LOG_DEBUG, "aiccu_os_exec(\"%s\")\n", buf));
	ret = system(buf);
	if (ret == -1) dolog(LOG_WARNING, "Execution of \"%s\" failed!? (Please check if the command is available)\n", buf);
	va_end(ap);
#endif
	return true;
}

#define SIXXS_LICENSE_PART1 "\
The SixXS License - http://www.sixxs.net/\n\
\n\
Copyright (C) SixXS Staff <info@sixxs.net>\n\
All rights reserved.\n\
\n\
Redistribution and use in source and binary forms, with or without\n\
modification, are permitted provided that the following conditions\n\
are met:\n\
1. Redistributions of source code must retain the above copyright\n\
   notice, this list of conditions and the following disclaimer.\n"

#define SIXXS_LICENSE_PART2 "\
2. Redistributions in binary form must reproduce the above copyright\n\
   notice, this list of conditions and the following disclaimer in the\n\
   documentation and/or other materials provided with the distribution.\n\
3. Neither the name of SixXS nor the names of its contributors\n\
   may be used to endorse or promote products derived from this software\n\
   without specific prior permission.\n\
\n\
\n"

#define SIXXS_LICENSE_PART3 "\
THIS SOFTWARE IS PROVIDED BY SIXXS AND CONTRIBUTORS ``AS IS'' AND\n\
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\n\
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE\n\
ARE DISCLAIMED.  IN NO EVENT SHALL SIXXS OR CONTRIBUTORS BE LIABLE\n\
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL\n\
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS\n"

#define SIXXS_LICENSE_PART4 "\
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)\n\
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT\n\
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY\n\
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF\n\
SUCH DAMAGE.\n"

const char *aiccu_license()
{
#ifndef NOPEDANTIC
	/*
	* Pedantic doesn't allow this long strings, thus we will
	 * play nice and malloc it, copy them in separately and
	 * then return the buffer.
	 * What we don't do for compliancy....
	*/
	static char *license = NULL;
	if (!license)
	{
		/*
		 * Make one big block out of it
		 * too bad that the \0's get inserted,
		 * remove them and tada one big text...
		*/
		static char
			l1[] = SIXXS_LICENSE_PART1,
			l2[] = SIXXS_LICENSE_PART2,
			l3[] = SIXXS_LICENSE_PART3,
			l4[] = SIXXS_LICENSE_PART4;
		size_t
			a = strlen(l1),
			b = strlen(l2),
			c = strlen(l3),
			d = strlen(l4);

		/* Create the 'long' string our selves then */
		license = (char *)malloc(a+b+c+d+1);
		if (!license) return NULL;

		memset(license, 0, a+b+c+d+1);
		memcpy(license    , l1, a);
		memcpy(license + a, l2, b);
		memcpy(license + a + b, l3, c);
		memcpy(license + a + b + c, l4, d);
	}
	return license;
#else
	return SIXXS_LICENSE_PART1 SIXXS_LICENSE_PART2 SIXXS_LICENSE_PART3 SIXXS_LICENSE_PART4;
#endif
}
