/*
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: upnp.h,v 1.1.1.7 2005/03/07 07:31:12 kanki Exp $
 */
 
#ifndef _upnp_h_
#define _upnp_h_

#include "typedefs.h"
#include "uio.h"
#include <time.h>
#include <stdarg.h>

#define  SSDP_IP   "239.255.255.250"
#define  SSDP_PORT 1900
#define  HTTP_PORT 5431
// #define  HTTP_PORT 80

// number of seconds to wait before refershing device advertisements.
#define SSDP_REFRESH   1800
#define UPNP_REFRESH   ((SSDP_REFRESH * 2)/3)
#define NOTIFY_RECEIPT_TIMEOUT 1
#define HTTP_REQUEST_TIMEOUT   1

#if !defined(FALSE) || !defined(TRUE)
#define TRUE 1
#define FALSE (!TRUE)
#endif

#define UPNP_BUFSIZE 2500
#define MAX_VALUE_LEN 33

#define ARRAYSIZE(a)  (sizeof(a)/sizeof(a[0]))

#ifdef UPNP_DEBUG
#define IFDEBUG(a)  a
#else
#define IFDEBUG(a)  
#endif

#define SERVER "LINUX/2.4 UPnP/1.0 BRCM400/1.0" /* server field for ssdp advertisement calls */
#define UPNP_MAX_VAL_LEN 100

#define MAX_HTTP_CONNECTIONS 5

struct Service;
struct Device;
struct iface;
struct net_connection;
struct VarTemplate;
struct StateVar;
struct var_entry;

struct Device;
struct Service;
struct DeviceTemplate;
struct DerviceTemplate;
struct Action;
struct Param;

typedef struct Device Device, *PDevice;
typedef struct Service Service, *PService;
typedef struct DeviceTemplate DeviceTemplate, *PDeviceTemplate;
typedef struct ServiceTemplate ServiceTemplate, *PServiceTemplate;
typedef struct Action Action, *PAction;
typedef struct Param Param, *PParam;

typedef enum { SSDP_REPLY = 2, SSDP_ALIVE=1, SSDP_BYEBYE=0 } ssdp_t;
typedef enum { CONNECTION_RECV = 0, CONNECTION_DELETE = 1 } caction_t;
typedef enum { SERVICE_CREATE = 0, SERVICE_DESTROY = 1 } service_state_t;
typedef enum { DEVICE_CREATE = 0, DEVICE_DESTROY = 1 } device_state_t;

typedef unsigned int u_int32;
typedef unsigned short u_int16;
typedef unsigned char u_int8;

typedef void (*PFDEVFOREACH)(PDevice, va_list);
typedef int (*PFDEVINIT)(PDevice, device_state_t, va_list);  
typedef void (*PFDEVXML)(PDevice, UFILE*);  
typedef void (*PFSVCXML)(PService, UFILE*);  

typedef void (*PFSVCFOREACH)(PService, va_list);
typedef int (*PFSVCINIT)(PService, service_state_t);  

typedef int (*PFVACTION)(UFILE *, PService, PAction, struct var_entry *, int);
typedef int (*PFGETVAR)(PService, int);
typedef int (*PFSETVAR)(PService, const struct VarTemplate *, struct StateVar *);
typedef void (*CONNECTION_HANDLER)(caction_t, struct net_connection *, void *);

typedef  unsigned long vartype_t;

#define VAR_USHORT	(1 << 0)
#define VAR_ULONG	(1 << 1)
#define VAR_STRING	(1 << 2) 
#define VAR_BOOL	(1 << 3)
#define VAR_SHORT	(1 << 4)
#define VAR_LONG	(1 << 5)
#define VAR_BYTE	(1 << 6) 
#define VAR_UBYTE	(1 << 7)

#define VAR_CHANGED	(1 << 8)
#define VAR_EVENTED	(1 << 9)

#define VAR_RANGE	(1 << 10)
#define VAR_LIST	(1 << 11)

#define VAR_TYPE_MASK	0x000000FF
#define VAR_TYPE_SHIFT	10


struct Param {
    char *name;
    int related;
    int flags;
    char            *value;
};

struct Action {
    char *name;
    PFVACTION func;
    Param *params;
};

#define VAR_IN  1
#define VAR_OUT 2


typedef struct {
    char *minimum;
    char *maximum;
    char *step;
} allowedValueRange, *PallowedValueRange;

typedef char *allowedValueList[];

typedef union {
    char **    list;
    allowedValueRange   *range;
} allowedValue, *PallowedValue;


typedef struct var_entry {
    char            *name;
    char            *value;
    vartype_t            type;
} var_entry_t, *pvar_entry_t;

typedef struct VarTemplate {
    const char *name;
    const char *value;
    unsigned long flags;
    allowedValue allowed;
} VarTemplate, *PVarTemplate;

typedef struct Subscription {
    char *		cb_host;
    char *		cb_path;
    int			cb_port;
    char		sid[50];
    unsigned int	event_key;
    struct sockaddr_in	addr;
    int			addrlen;
    int			sock;                      // socket used to send out last notify 
    time_t		expires;
    // timer_t		hevent;		/* event handle for triggering initial evented variable update. */
    struct Subscription *next;
} Subscription, *PSubscription;

/* Structure for storing Service identifiers and state table */
struct ServiceTemplate {
    char	*name;		/* name of the service, e.g. "WLANHostConfigManagement" */
    
    /* pointers to functions. May be NULL. */
    PFSVCINIT 	 svcinit;	/* service initialization */
    PFGETVAR  	 getvars;   	/* state variable handler */
    PFSVCXML	 svcxml;	/* xml generator */

    int		 nvariables;
    VarTemplate	*variables;
    PAction 	*actions;
    int		 count;
    char	*serviceid;
    char	*schema;
};

typedef struct StateVar {
    unsigned long            flags;
    char            value[UPNP_MAX_VAL_LEN];
}  __attribute__ ((packed)) StateVar, *PStateVar;

struct Service { 
    PServiceTemplate	template;
    PDevice		device;
    Subscription    *subscriptions;
    struct Service  *next;
    StateVar        *vars;
    u_int32	    flags;
    void            *opaque;   // opaque user data
    int instance;
};

struct DeviceTemplate {
    char *		type;
    char *		udn;
    PFDEVINIT		devinit;   
    PFDEVXML		devxml;
    int			nservices;
    PServiceTemplate *	services;
    int			ndevices;
    PDeviceTemplate	devicelist;  // templates for sub-devices
    char *schema;
};

/*
  
  udn - the device definition contains a unique string (a unique
  device number) that should be substituted in the XML device
  description, and in device advertisements.  This string looks like a
  UUID (or GUID) and it generated at runtime.  Ideally this string is
  unuque across all devices in all routers in the world.  It should be
  generated based upon the lac address of the publically accessible
  interface, but that work is not completed.
  
*/
struct Device {
    DeviceTemplate  *template;
    char            *udn;
    Service         *services;
    struct Device   *parent;	// parent device (NULL if root)
    struct Device   *subdevs;	// subdevices
    struct Device   *next;	// link to next device on global device list
    void            *opaque;	// opaque user data
    char            *friendlyname;
};

#define ISROOT(pd) (pd->parent == NULL)

typedef void (*event_callback_t)(timer_t, void *);

struct iface {
    struct iface *next;

    char *ifname;

    // The address of the HTTP server
    struct in_addr inaddr;

    struct net_connection *http_connection;
    struct net_connection *ssdp_connection;
};

/* 
   Used to store information about network connections that have not
   completed yet.  This might include http connections that have been
   accepted or partially read, but which have not yet been completed by
   the client.  It may also include connections for GENA notifications
   which have been send but not yet acknowledged.  
*/

struct net_connection {
    int			    fd;
    time_t		    expires;  // 0 means never expires.
    CONNECTION_HANDLER	    func;
    void *		    arg;
    struct net_connection * next;
};

typedef uint soap_error_t;

typedef struct Error {
    soap_error_t error_code;
    char *error_string;
} Error, *PError;


#define MATCH_PREFIX(a, b)  (strncmp((a),(b),sizeof(b)-1)==0)
#define IMATCH_PREFIX(a, b)  (strncasecmp((a),(b),sizeof(b)-1)==0)

/* Generic UPNP errors 

   Device-specific errors may also exist, but those will be defined in
   device-specific header files, e.g. igd.h
*/
#define SOAP_INVALID_ACTION	401 
#define SOAP_INVALID_ARGS	402 
#define SOAP_INVALID_VAR	404 
#define SOAP_ACTION_FAILED	501 
#define SOAP_INVALIDDEVICEUUID	720
#define SOAP_INVALIDSERVICEID	721

typedef enum {
    HTTP_OK = 200, 
    HTTP_BAD_REQUEST = 400,
    HTTP_FORBIDDEN = 403,
    HTTP_NOT_FOUND = 404,
    HTTP_PRECONDITION = 412,
    HTTP_SERVER_ERROR = 500, 
    HTTP_NOT_IMPLEMENTED = 501
} http_error_t;


extern void soap_error(UFILE *, soap_error_t error_code);
extern void soap_success( UFILE *up, PService psvc, PAction ac, pvar_entry_t args, int nargs );
extern void http_error(UFILE *, http_error_t error_code);
extern void http_response(UFILE *, http_error_t, const char *, int );
extern void soap_register_errors(Error *errors);

#ifdef VXWORKS
#include "wsIntrn.h"
#include "utils.h"

#define log printf

#elif defined(linux)
typedef void (*VOIDFUNCPTR)();
#define log printf
#endif

typedef void (*voidfp_t)(void);

#define HTTP_BUF_LOWATER 200
#define HTTP_BUFSIZE (2*HTTP_BUF_LOWATER)
#define HTTP_BUF_INCREMENT (2*HTTP_BUF_LOWATER)


extern int upnp_main(PDeviceTemplate pdevtmpl, char *ifname);
extern void mark_changed(PService psvc, int varindex);
extern timer_t enqueue_event(struct itimerspec *value, event_callback_t func, void *arg);
extern void update_all_subscriptions(PService psvc);
extern PDevice device_iterator(PDevice pdev);
extern PDevice rootdev(PDevice pdev);
extern int NotImplemented(UFILE *, PService psvc, PAction ac, pvar_entry_t args, int nargs);
extern int DefaultAction(UFILE *, struct Service *psvc, PAction ac, pvar_entry_t args, int nargs);

extern int unique;

     // extern void forall_devices(PFDEVFOREACH func, ...);
     // extern void forall_services(PFSVCFOREACH func, ...);

#define forall_devices(pdev) \
  for (pdev = device_iterator(NULL); pdev; pdev = device_iterator(pdev)) 

#define forall_services(pdev, psvc) \
      for (psvc = pdev->services; psvc; psvc = psvc->next) 

#define	PHY_TYPE_A		0
#define	PHY_TYPE_B		1
#define	PHY_TYPE_G		2

#endif /* _upnp_h_ */






