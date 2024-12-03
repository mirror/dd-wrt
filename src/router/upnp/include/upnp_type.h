/*
 * Broadcom UPnP module type defintions
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp_type.h,v 1.12 2008/08/25 08:22:28 Exp $
 */

#ifndef __UPNP_TYPE_H__
#define __UPNP_TYPE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <device_config.h>

/*
 * Definitions
 */
#if !defined(OK) || !defined(ERROR)
#define OK 0
#define ERROR -1
#endif

#if !defined(TRUE) || !defined(FALSE)
#define TRUE 1
#define FALSE 0
#endif

#define UPNP_VALUE_SIZE 1024

/* Forward definition */
typedef struct upnp_evalue UPNP_EVALUE;
typedef struct upnp_state_var UPNP_STATE_VAR;
typedef struct upnp_evar UPNP_EVAR;
typedef struct upnp_service UPNP_SERVICE;
typedef struct upnp_device UPNP_DEVICE;
typedef struct upnp_devchain UPNP_DEVCHAIN;
typedef struct upnp_subscriber UPNP_SUBSCRIBER;
typedef struct upnp_scbrchain UPNP_SCBRCHAIN;
typedef struct in_argument IN_ARGUMENT;
typedef struct out_argument OUT_ARGUMENT;
typedef struct upnp_if UPNP_INTERFACE;
typedef struct upnp_context UPNP_CONTEXT;

/* UPNP type definition */
enum UPNP_DATA_TYPE_E {
	UPNP_TYPE_STR = 3,
	UPNP_TYPE_BOOL,
	UPNP_TYPE_UI1,
	UPNP_TYPE_UI2,
	UPNP_TYPE_UI4,
	UPNP_TYPE_I1,
	UPNP_TYPE_I2,
	UPNP_TYPE_I4,
	UPNP_TYPE_BIN_BASE64
};

typedef struct upnp_value {
	int type;
	int len;
	union {
		char i1;
		short i2;
		long i4;
		unsigned char ui1;
		unsigned short ui2;
		unsigned long ui4;
		unsigned int bool2;
		char str[UPNP_VALUE_SIZE];
		char data[UPNP_VALUE_SIZE];
	} val;
} UPNP_VALUE;

/* Service XML */
typedef int (*ACTION_FUNC)(UPNP_CONTEXT *, UPNP_SERVICE *, IN_ARGUMENT *, OUT_ARGUMENT *);
typedef int (*QUERY_FUNC)(UPNP_CONTEXT *, UPNP_SERVICE *, UPNP_STATE_VAR *, UPNP_VALUE *);

/* State variables */
struct upnp_evalue {
	UPNP_EVALUE *next;
	UPNP_INTERFACE *ifp;

	int init;
	int changed;
	UPNP_VALUE value;
};

struct upnp_state_var {
	UPNP_STATE_VAR *next;

	char *name;
	int type;
	QUERY_FUNC func;
	int eflag;
	UPNP_EVALUE *evalue;
};

struct upnp_evar {
	char statevar[128];
	UPNP_VALUE value;
};

/* Action and arguments */
typedef struct {
	char *name;
	int type;
	int related_id;
} ACTION_ARGUMENT;

typedef struct upnp_action {
	char *name;
	int in_num;
	ACTION_ARGUMENT *in_argument;
	int out_num;
	ACTION_ARGUMENT *out_argument;
	ACTION_FUNC action;
} UPNP_ACTION;

/* Service */
struct upnp_service {
	char *control_url;
	char *event_url;
	char *name;
	char *service_id;
	UPNP_ACTION *action_table;
	UPNP_STATE_VAR *statevar_table;

	int evented;
	UPNP_STATE_VAR *event_var_list;
	UPNP_SCBRCHAIN *scbrchain;
};

/* UPnP advertise */
typedef struct upnp_advertise {
	char *name;
	char uuid[40];
	int type;
} UPNP_ADVERTISE;

/* UPnP description */
typedef struct upnp_description {
	char *name;
	char *xml;
} UPNP_DESCRIPTION;

/* UPnP device */
#define DEVICE_ATTACH_ALWAYS 0
#define DEVICE_ATTACH_DYNAMICALLY 1

struct upnp_device {
	UPNP_DEVICE *next;

	char *root_device_xml;
	UPNP_SERVICE *service_table;
	UPNP_ADVERTISE *advertise_table;
	UPNP_DESCRIPTION *description_table;
	int (*common_init)(UPNP_CONTEXT *);
	int (*open)(UPNP_CONTEXT *);
	int (*close)(UPNP_CONTEXT *);
	int (*request)(UPNP_CONTEXT *, void *cmd);
	int (*timeout)(UPNP_CONTEXT *, time_t);
	int (*notify)(UPNP_CONTEXT *, UPNP_SERVICE *);
	int attach_mode;
};

struct upnp_devchain {
	UPNP_DEVCHAIN *next;

	UPNP_INTERFACE *ifp;
	UPNP_DEVICE *device;
	void *devctrl;
};

/* UPNP GENA protocol */
#define UPNP_MAX_SID 32

struct upnp_subscriber {
	UPNP_SUBSCRIBER *next;
	UPNP_SUBSCRIBER *prev;

	struct in_addr ipaddr;
	unsigned short port;
	char *uri;
	char sid[UPNP_MAX_SID];
	unsigned int expire_time;
	unsigned int seq;
};

struct upnp_scbrchain {
	UPNP_SCBRCHAIN *next;

	UPNP_INTERFACE *ifp;
	UPNP_SUBSCRIBER *subscriberlist;
};

/* UPNP SOAP protocol */
struct in_argument {
	IN_ARGUMENT *next;

	char *name;
	UPNP_STATE_VAR *statevar;
	UPNP_VALUE value;
};

/* Output argument defintions */
struct out_argument {
	OUT_ARGUMENT *next;

	char *name;
	UPNP_STATE_VAR *statevar;
	UPNP_VALUE value;
};

/* UPnP protocol suite */
/*
 * UPnP interface
 */
#define IFF_IPCHANGED 0x01 /* interface IP address changed */
#define IFF_MJOINED 0x02 /* SSDP multicast group joined */

struct upnp_if {
	UPNP_INTERFACE *next; /* pointer to next if */

	char ifname[IFNAMSIZ]; /* interface name */
	int if_instance; /* interface instance index */
	int flag; /* ip changed, multicast joined? */
	int http_sock; /* upnp_http socket */
	int req_sock; /* Per interface request socket */
	struct in_addr ipaddr;
	struct in_addr netmask;

	UPNP_DEVCHAIN *device_chain;
	UPNP_DEVCHAIN *focus_devchain;
};

/*
 * UPnP context
 */
#define MAX_HEADERS 64 /* default message header num */
#define MAX_HEADER_LEN 4096
#define MAX_BUF_LEN 2048

typedef struct upnp_config {
	unsigned int http_port; /* upnp_http port number */
	unsigned int adv_time; /* ssdp advertising time interval */
	unsigned int sub_time; /* gena subscription time interval */

	char os_name[32];
	char os_ver[32];
} UPNP_CONFIG;

struct upnp_context {
	/* Configuration */
	UPNP_CONFIG config;

	UPNP_INTERFACE *iflist;
	UPNP_INTERFACE *focus_ifp;

	time_t adv_seconds;
	time_t gena_last_check;

	int ssdp_sock; /* Socket to recive ssdp multicast packets */

	/* Status */
	int method; /* M_GET, M_POST, ... */
	int method_id; /* method index */
	int fd;

	/* client socket descriptor */
	int status; /* R_OK, R_ERROR, R_BAD_REQUEST... */

	char buf[MAX_HEADER_LEN * 2]; /* upnp_http input buffer */
	char *msghdrs[MAX_HEADERS]; /* delimited headers */
	int header_num; /* num of headers */

	char head_buffer[MAX_HEADER_LEN]; /* output header buffer */
	char body_buffer[MAX_BUF_LEN]; /* outout body buffer */

	int index; /* index to next unread char */
	int end; /* index to one char past last */

	char *url;
	char *content;
	int content_len;

	char *CONTENT_LENGTH;
	char *SOAPACTION;
	char *SID;
	char *CALLBACK;
	char *TIMEOUT;
	char *NT;
	char *HOST;
	char *MAN;
	char *ST;

	IN_ARGUMENT *in_arguments;
	OUT_ARGUMENT *out_arguments;

	IN_ARGUMENT in_args[UPNP_MAX_IN_ARG];
	OUT_ARGUMENT out_args[UPNP_MAX_OUT_ARG];

	struct sockaddr_in *dst; /* client address, for SSDP MSEARCH */
	struct sockaddr_in dst_addr;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __UPNP_TYPE_H__ */
