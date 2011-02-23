/**********************************************************
 SixXS - Automatic IPv6 Connectivity Configuration Utility
***********************************************************
 Copyright 2003-2005 SixXS - http://www.sixxs.net
***********************************************************
 common/tic.h - Tunnel Information & Control Protocol
***********************************************************
 $Author: jeroen $
 $Id: tic.h,v 1.12 2006-12-21 14:08:50 jeroen Exp $
 $Date: 2006-12-21 14:08:50 $
**********************************************************/

#ifndef TIC_H
#define TIC_H "H5K7:W3NDY5UU5N1K1N1C0l3"

#include "common.h"

/* 
 * Tunnel Information Control Protocol
 * server
 */
/* port - uses TCP over IPv4 */
#define TIC_PORT	"3874"

/* TIC version (which document this should conform to) */
#define TIC_VERSION	"draft-00"

struct TIC_sTunnel
{
	struct TIC_sTunnel	*next;			/* Next in list */
	char			*sId;			/* Tunnel Id */
	char			*sIPv6;			/* Local IPv6 Endpoint */
	char			*sIPv4;			/* Local IPv4 Endpoint */
	char			*sPOPId;		/* POP Id */
};

struct TIC_Tunnel
{
	char			*sId;			/* Tunnel Id */
	char			*sType;			/* Tunnel Type */

	/* IPv4 information */
	char			*sIPv4_Local;		/* Local endpoint (*) */
	char			*sIPv4_POP;		/* POP endpoint	 */

	/* IPv6 information */
	char			*sIPv6_Local;		/* Local endpoint */
	char			*sIPv6_POP;		/* POP endpoint	 */
	char			*sIPv6_LinkLocal;	/* Link local address */

	/* POP information */
	char			*sPOP_Id;		/* POP's Id */
	
	/* States */
	char			*sUserState;		/* Userstate */
	char			*sAdminState;		/* Adminstate */
	
	/* AYIYA & Heartbeat */
	char			*sPassword;		/* Password for the tunnel */
	uint32_t		nHeartbeat_Interval;	/* Heartbeat interval */

	/* Misc */
	uint32_t		uses_tundev;		/* Uses Tunnel (tun/tap) device? */
	uint32_t		nIPv6_PrefixLength;	/* Length of the prefix's */
	uint32_t		nMTU;			/* MTU size */
};

/* * = 0.0.0.0 for all the dynamic tunnels */

struct TIC_sRoute
{
	struct TIC_sRoute	*next;			/* Next in list */
	char			*sId;			/* Route Id */
	char			*sTunnelId;		/* Tunnel Id */
	char			*sIPv6;			/* Prefix */
};

struct TIC_Route
{
	char			*sId;			/* Route Id */
	char			*sTunnelId;		/* Tunnel Id */
	struct in6_addr		xIPv6;			/* Prefix */
	uint32_t		nPrefixLength;		/* Length of the prefix */
	uint32_t		__pad;
};


struct TIC_sPOP
{
	struct TIC_sPOP		*next;			/* Next in list */
	char			*sId;			/* POP's Id */
};

struct TIC_POP
{
	char			*sId;			/* POP's Id */
	char			*sCity;			/* POP's City */
	char			*sCountry;		/* POP's Country */
	char			*sIPv4;			/* POP's Primary IPv4 address */
	char			*sIPv6;			/* POP's Primary IPv6 address */

	char			*sISP_Short;		/* ISP's Short name */
	char			*sISP_Name;		/* ISP's Name */
	char			*sISP_Website;		/* ISP's Website */
	char			*sISP_ASN;		/* ISP's ASN */
	char			*sISP_LIR;		/* ISP's LIR */
};

/*
 * This structure makes TIC a bit more abstracted
 * which makes this cleaner instead of passing 'sock' everywhere
 */
struct TIC_conf
{
	TLSSOCKET		sock;		/* The socket to which we are connected */
};

/**********************************************************
 TIC Functions
**********************************************************/

/* Login to/Logout from the TIC Server */
bool tic_Login(struct TIC_conf *tic, const char *username, const char *password, const char *server);
void tic_Logout(struct TIC_conf *tic, const char *quitmsg);

/* Check if the time is in range */
int tic_checktime(time_t epochtime);

/* Get Tunnel/Route/POP List */
struct TIC_sTunnel	*tic_ListTunnels(struct TIC_conf *tic);
struct TIC_sRoute	*tic_ListRoutes(struct TIC_conf *tic);
struct TIC_sPOP		*tic_ListPOPs(struct TIC_conf *tic);

/* Get Tunnel/Route/POP Information */
struct TIC_Tunnel	*tic_GetTunnel(struct TIC_conf *tic, const char *sId);
struct TIC_Route	*tic_GetRoute(struct TIC_conf *tic, const char *sId);
struct TIC_POP		*tic_GetPOP(struct TIC_conf *tic, const char *sId);

/* Free Information structures */
void			tic_Free_sTunnel(struct TIC_sTunnel *tun);
void			tic_Free_sRoute(struct TIC_sRoute *rt);
void			tic_Free_sPOP(struct TIC_sPOP *pop);
void			tic_Free_Tunnel(struct TIC_Tunnel *tun);
void			tic_Free_Route(struct TIC_Route *rt);
void			tic_Free_POP(struct TIC_POP *pop);

#endif /* TIC_H */
