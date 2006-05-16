/*
 * Minimal debug/trace/assert driver definitions for
 * Broadcom UPNP implementation
 *
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: upnp_dbg.h,v 1.1.1.8 2005/03/07 07:31:12 kanki Exp $
 */

#ifndef _upnp_dbg_
#define _upnp_dbg_

//#define DEBUG	// !!!

#ifdef DEBUG

#include <stdio.h>

#define	UPNP_MAPPING(args)					printf args
#define	UPNP_ERROR(args)					printf args
#define	UPNP_TRACE(args)					printf args
#define	UPNP_PRHDRS(i, s, h, p, n, l)
#define	UPNP_PRPKT(m, b, n)
#define	UPNP_INFORM(args)					printf args
#define UPNP_TRACE_ACTION(svc, ac) 
#define	UPNP_HTTP(args)						printf args
#define	UPNP_SOCKET(args) 					printf args

#define UPNP_MAPPING_ON()	0
#define UPNP_RESPONSE_ON()	0
#define UPNP_ERROR_ON()		0
#define UPNP_PRHDRS_ON()	0
#define UPNP_PRPKT_ON()		0
#define UPNP_INFORM_ON()	0
#define UPNP_PRINTRX_ON()	0
#define UPNP_PRINTTX_ON()	0
#define UPNP_HTTP_TRACE_ON()	0
#define UPNP_HTTP_HDRS_ON()	0

#define	UPNP_PREVENT(args)
#define	UPNP_SUBSCRIBE(args)

#define UPNP_ACTION(psvc, ac, args, nargs)
#define UPNP_RESPONSE(ns, ac, args, nargs)
#define UPNP_TRACE_ACTION_ON()	0
#define UPNP_DUMP_ACTION_ON()	0

#else

#define	UPNP_MAPPING(args)
#define	UPNP_ERROR(args)
#define	UPNP_TRACE(args)
#define	UPNP_PRHDRS(i, s, h, p, n, l)
#define	UPNP_PRPKT(m, b, n)
#define	UPNP_INFORM(args)
#define UPNP_TRACE_ACTION(svc, ac) 
#define	UPNP_HTTP(args)
#define	UPNP_SOCKET(args) 

#define UPNP_MAPPING_ON()	0
#define UPNP_RESPONSE_ON()	0
#define UPNP_ERROR_ON()		0
#define UPNP_PRHDRS_ON()	0
#define UPNP_PRPKT_ON()		0
#define UPNP_INFORM_ON()	0
#define UPNP_PRINTRX_ON()	0
#define UPNP_PRINTTX_ON()	0
#define UPNP_HTTP_TRACE_ON()	0
#define UPNP_HTTP_HDRS_ON()	0

#define	UPNP_PREVENT(args)
#define	UPNP_SUBSCRIBE(args)

#define UPNP_ACTION(psvc, ac, args, nargs)
#define UPNP_RESPONSE(ns, ac, args, nargs)
#define UPNP_TRACE_ACTION_ON()	0
#define UPNP_DUMP_ACTION_ON()	0

#endif


#endif /* _upnp_dbg_ */
