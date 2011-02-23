/**********************************************************
 SixXS - Automatic IPv6 Connectivity Configuration Utility
***********************************************************
 Copyright 2003-2005 SixXS - http://www.sixxs.net
***********************************************************
 common/aiccu.h - Abstracted Functions and Configuration
 All compile-time configurable items are in this file
***********************************************************
 $Author: jeroen $
 $Id: aiccu.h,v 1.23 2007-01-15 12:01:43 jeroen Exp $
 $Date: 2007-01-15 12:01:43 $
**********************************************************/

#ifndef AICCU_H
#define AICCU_H "H5K7:W3NDY5UU5N1K1N1C0l3"

#include <sys/types.h>
#include "common.h"
#include "tic.h"
#include "heartbeat.h"
#include "ayiya.h"
#include "resolver.h"

#ifdef NEWSTUFF_TSP
#include "tsp.h"
#endif
#ifdef NEWSTUFF_TSP
#include "teepee.h"
#endif

/* AICCU Version */
#define AICCU_VER		"2007.01.15"
#define AICCU_VERSION_NUM	2007,01,15,0
/* _NUM = required for Windows Resources */

#ifdef _WIN32
#define AICCU_TYPE "win32"
#endif

/* Append -gui or -console? */
#ifndef AICCU_CONSOLE
#ifdef AICCU_TYPE
#define AICCU_VERSION AICCU_VER "-gui-" AICCU_TYPE
#else
#define AICCU_VERSION AICCU_VER "-gui"
#endif
#else
#ifdef AICCU_TYPE
#define AICCU_VERSION AICCU_VER "-console-" AICCU_TYPE
#else
#define AICCU_VERSION AICCU_VER "-console"
#endif
#endif

/* Needed for TIC */
#define TIC_CLIENT_NAME "AICCU"
#define TIC_CLIENT_VERSION AICCU_VERSION

/* Needed for TSP */
#define TSP_CLIENT_NAME TIC_CLIENT_NAME
#define TSP_CLIENT_VERSION TIC_CLIENT_VERSION

/* 
 * AICCU configuration Cache
 * allows reconnects even when we don't update
 * the data. Could be useful in the event
 * where we can't make contact to the main server
 */
#define AICCU_CACHE	"/var/cache/aiccu.cache"

/* The PID we are running as when daemonized */
#define AICCU_PID	"/var/run/aiccu.pid"

/* AICCU Configuration file */
#ifdef _WIN32
/* GetWindowsDirectory() is used to figure out the directory to store the config */
#define AICCU_CONFIG	"aiccu.conf"
#else
#define AICCU_CONFIG	"/etc/aiccu.conf"
#endif

/* Inbound listen queue */
#define LISTEN_QUEUE    128

#ifndef UNUSED
#ifdef _AIX
#define UNUSED
#else
#define UNUSED __attribute__ ((__unused__))
#endif
#endif

/* AICCU Configuration */
struct AICCU_conf
{
	/* Only for AICCU */
	char		*username;		/* Username */
	char		*password;		/* Password */
	char		*protocol;		/* TIC/TSP/L2TP */
	char		*server;		/* TIC/TSP etc server */
	char		*ipv6_interface;	/* IPv6 interface (tunnel interface: sit0, tun0 etc) */
	char		*tunnel_id;		/* ID of the tunnel to use */
	char		*local_ipv4_override;	/* Local IPv4 override, for behind-NAT scenario's */
	char		*setupscript;		/* Script to run after having set up the tunnel */
	char		*pidfile;		/* File to store the PID */

	/* used by other parts */

	struct TIC_conf	*tic;			/* TIC Structure */
#ifdef NEWSTUFF_TSP
	struct TSP_conf	*tsp;			/* TSP Structure */
#endif

#ifdef AICCU_GNUTLS
	gnutls_certificate_credentials	tls_cred;	/* GNUTLS credentials */
#endif

	bool		daemonize;		/* Daemonize? */
	bool		verbose;		/* Verbosity */
	bool		running;		/* Still running? */
	bool		tunrunning;		/* Is the tundev running? */

	bool		automatic;		/* Try to be totally automatic? */
	bool		behindnat;		/* Behind a NAT */
	bool		requiretls;		/* Require TLS for TIC? */
	bool		makebeats;		/* Make heartbeats? */
	bool		noconfigure;		/* No configuration (used to only send heartbeats) */
	bool		defaultroute;		/* Configure a default route */
};

/* Global configuration */
extern struct AICCU_conf *g_aiccu;

/* AICCU Abstracted Functions */
bool aiccu_InitConfig(void);
bool aiccu_LoadConfig(const char *filename);
bool aiccu_SaveConfig(const char *filename);
void aiccu_FreeConfig(void);

void aiccu_install(void);
bool aiccu_setup(struct TIC_Tunnel *hTunnel, bool firstrun);
void aiccu_beat(struct TIC_Tunnel *hTunnel);
void aiccu_reconfig(struct TIC_Tunnel *hTunnel);
void aiccu_delete(struct TIC_Tunnel *hTunnel);
void aiccu_test(struct TIC_Tunnel *hTunnel, bool automatic);
bool aiccu_exec(const char *fmt, ...);
const char *aiccu_license(void);

/* OS Specific */
bool aiccu_os_install(void);
bool aiccu_os_setup(struct TIC_Tunnel *hTunnel);
void aiccu_os_reconfig(struct TIC_Tunnel *hTunnel);
void aiccu_os_delete(struct TIC_Tunnel *hTunnel);
void aiccu_os_test(struct TIC_Tunnel *hTunnel, bool automatic);

#ifdef _WIN32
void aiccu_win32_rename_adapter(const char *orig);
#endif

#endif /* AICCU_H */
