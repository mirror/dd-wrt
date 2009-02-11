/*
 * Broadcom UPnP module utilities include file
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp_util.h,v 1.7 2008/06/19 06:22:26 Exp $
 */

#ifndef __UPNP_UTIL_H__
#define __UPNP_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

#define	UPNP_STR(value)		(value)->val.str
#define	UPNP_BOOL(value)	(value)->val.bool
#define	UPNP_I1(value)		(value)->val.i1
#define	UPNP_I2(value)		(value)->val.i2
#define	UPNP_I4(value)		(value)->val.i4
#define	UPNP_UI1(value)		(value)->val.ui1
#define	UPNP_UI2(value)		(value)->val.ui2
#define	UPNP_UI4(value)		(value)->val.ui4
#define	UPNP_BIN_LEN(value)	(value)->len
#define	UPNP_BIN_DATA(value)	(value)->val.data

#define	ARG_VALUE(arg)		(&(arg)->value)
#define	ARG_STR(arg)		(arg)->value.val.str
#define	ARG_BOOL(arg)		(arg)->value.val.bool
#define	ARG_I1(arg)		(arg)->value.val.i1
#define	ARG_I2(arg)		(arg)->value.val.i2
#define	ARG_I4(arg)		(arg)->value.val.i4
#define	ARG_UI1(arg)		(arg)->value.val.ui1
#define	ARG_UI2(arg)		(arg)->value.val.ui2
#define	ARG_UI4(arg)		(arg)->value.val.ui4
#define	ARG_BIN_LEN(arg)	(arg)->value.len
#define	ARG_BIN_DATA(arg)	(arg)->value.val.data

/* Functions */
int convert_value(UPNP_CONTEXT *context, UPNP_VALUE *value);
void translate_value(UPNP_CONTEXT *context, UPNP_VALUE *value);
int gmt_time(char *time_buf);

IN_ARGUMENT *upnp_get_in_argument
(
	IN_ARGUMENT *arguments,
	char *arg_name
);

OUT_ARGUMENT *upnp_get_out_argument
(
	OUT_ARGUMENT *arguments,
	char *arg_name
);

void	upnp_host_addr(unsigned char *host_addr, struct in_addr ipaddr, unsigned short port);

#define	UPNP_USE_HINT(a...)
#define	UPNP_CONST_HINT(a)
#define	UPNP_IN_HINT(a)
#define	UPNP_OUT_HINT(a)
#define	UPNP_IN_ARG(a)		upnp_get_in_argument(in_argument, a)
#define	UPNP_OUT_ARG(a)		upnp_get_out_argument(out_argument, a)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __UPNP_UTIL_H__ */
