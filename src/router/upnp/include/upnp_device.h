/*
 * Broadcom UPnP module device specific include file
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp_device.h,v 1.7 2008/05/16 02:20:45 Exp $
 */

#ifndef __UPNP_DEVICE_H__
#define __UPNP_DEVICE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern UPNP_DEVICE *upnp_device_table[];

int upnp_device_attach(UPNP_CONTEXT *context, UPNP_DEVICE *device);
void upnp_device_detach(UPNP_CONTEXT *context, UPNP_DEVICE *device);
int upnp_device_renew_uuid(UPNP_CONTEXT *context, UPNP_DEVICE *device);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __UPNP_H__ */
