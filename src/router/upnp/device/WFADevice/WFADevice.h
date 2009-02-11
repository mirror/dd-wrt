/*
 * Broadcom UPnP module, WFADevice.h
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: WFADevice.h,v 1.6 2008/08/25 08:17:40 Exp $
 */

#ifndef __WFADEVICE_H__
#define __WFADEVICE_H__

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

#define UPNP_WFA_PORT			40040			/* WFA wlan receive port */
#define UPNP_WFA_ADDR			"127.0.0.1"

#define UPNP_WFA_DATA_MAX_LENGTH	1024
#define UPNP_WFA_READ_WPS_TIMEOUT	2

#define WFA_DBG(a...)			upnp_syslog(LOG_INFO, ##a)

/*
 * WFA soft contorl
 */
typedef	struct upnp_wfactrl
{
	int m_write;			/* for write to UPnPDev */
	int m_read;			/* for read from UPnPDev */
	char *m_devInfo;		/* device information */
	int m_devInfoLen;
}
UPNP_WFACTRL;

/*
 * Function protocol type
 */
int	wfa_SetSelectedRegistrar(UPNP_CONTEXT *context, UPNP_VALUE *NewMessage);
int	wfa_PutMessage			(UPNP_CONTEXT *context, UPNP_VALUE *NewInMessage, UPNP_VALUE *NewOutMessage);
int	wfa_GetDeviceInfo		(UPNP_CONTEXT *context, UPNP_VALUE *NewDeviceInfo);
int	wfa_PutWLANResponse		(UPNP_CONTEXT *context, UPNP_VALUE *NewMessage);

/* OSL dependent function */
extern void upnp_osl_update_wfa_subc_num(int if_instance, int num);

/* << TABLE BEGIN */
/*
 * WARNNING: DON'T MODIFY THE FOLLOWING TABLES
 * AND DON'T REMOVE TAG :
 *          "<< TABLE BEGIN"
 *          ">> TABLE END"
 */

extern UPNP_DEVICE WFADevice;

int WFADevice_common_init(UPNP_CONTEXT *context);
int WFADevice_open(UPNP_CONTEXT *context);
int WFADevice_close(UPNP_CONTEXT *context);
int WFADevice_request(UPNP_CONTEXT *context, void *cmd);
int WFADevice_timeout(UPNP_CONTEXT *context, time_t now);
int WFADevice_notify(UPNP_CONTEXT *context, UPNP_SERVICE *service);
/* >> TABLE END */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __WFADEVICE_H__ */
