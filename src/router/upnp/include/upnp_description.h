/*
 * Broadcom UPnP module XML description protocol include file
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp_description.h,v 1.8 2008/06/19 06:22:26 Exp $
 */

#ifndef __UPNP_DESCRIPTION_H__
#define __UPNP_DESCRIPTION_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Constants */
#define UPNP_DESC_MAXLEN 2048 /* maximum length per send */

#define DEVICE_BTAG "<deviceType>"
#define DEVICE_ETAG "</deviceType>"
#define SERVICE_BTAG "<serviceType>"
#define SERVICE_ETAG "</serviceType>"
#define UDN_BTAG "<UDN>"
#define UDN_ETAG "</UDN>"
#define URL_BTAG "<presentationURL>"
#define URL_ETAG "</presentationURL>"

/*
 * Functions
 */
int description_process(UPNP_CONTEXT *context);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __UPNP_DESCRIPTION_H__ */
