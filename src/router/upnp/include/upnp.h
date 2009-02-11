/*
 * Broadcom UPnP module include file
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp.h,v 1.8 2007/12/12 04:07:06 Exp $
 */

#ifndef __UPNP_H__
#define __UPNP_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <upnp_osl.h>

#include <upnp_type.h>
#include <soap.h>
#include <gena.h>
#include <ssdp.h>
#include <upnp_http.h>
#include <upnp_description.h>
#include <upnp_device.h>
#include <upnp_req.h>
#include <upnp_util.h>

/*
 * Declaration
 */
#define	UPNP_FLAG_SHUTDOWN	1
#define	UPNP_FLAG_RESTART	2

/*
 * Functions
 */
extern int upnp_osl_primary_lanmac(char *mac);
extern int upnp_osl_read_config(UPNP_CONFIG *config);
extern int upnp_osl_ifname_list(char *ifname_list);

void 	upnp_mainloop();
void 	upnp_stop_handler();
void 	upnp_restart_handler();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __UPNP_H__ */
