/*
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: upnp.c,v 1.2 2005/05/25 02:12:07 honor Exp $
 */

#include <signal.h>
#include <stdarg.h>
#include <string.h>
#include <bcmnvram.h>

#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"

extern void periodic_advertiser(timer_t, ssdp_t sstype);
extern void send_advertisements(ssdp_t);
extern timer_t enqueue_event(struct itimerspec *value, event_callback_t func, void *arg);
extern void alarm_handler(int i);
extern void init_event_queue(int);
extern void gena_subscription_reaper(timer_t, void *arg);
extern void uuidstr_create(char *, int);
extern struct net_connection *make_http_socket(struct iface *);
extern struct net_connection *make_ssdp_socket(struct iface *);
extern void delete_all_subscriptions();
extern void delete_all_connections();
extern PDevice init_device(PDevice, PDeviceTemplate, ...);
extern void destroy_device(PDevice);
extern int dd_timer_connect(timer_t, VOIDFUNCPTR, int);


void interrupt_handler(int i);


static struct iface *create_lan_interface(char *);
static int fd_net_set(fd_set *fds);
static void process_receive_fds(fd_set *fds);
static void connection_reaper(timer_t, void *arg);
static void block_alarm();
static void unblock_alarm();
static void generate_uuids();

struct net_connection *net_connections = NULL;

int global_exit_now = FALSE;

extern int ssdp_interval;

struct iface *global_lans = NULL;

Error UPNPErrors[] = {
    { SOAP_INVALID_ACTION, "Invalid Action" },
    { SOAP_INVALID_ARGS, "Invalid Args" },
    { SOAP_INVALID_VAR, "Invalid Var" },
    { SOAP_ACTION_FAILED, "Action Failed" },
    { SOAP_INVALIDDEVICEUUID, "InvalidDeviceUUID" },
    { SOAP_INVALIDSERVICEID, "InvalidServiceID" },
    { 0, NULL }
};


int upnp_main(PDeviceTemplate pdevtmpl, char *ifname) 
{
    fd_set rfds;
    int n;
    struct  itimerspec  timer;
    struct iface *lanif;
    timer_t td1, td2, td3;
    PDevice device;

    UPNP_TRACE(("Entered upnp_main pdevtmpl=0x%x \"%s\" ifname=0x%x \"%s\"\n", 
		(uint) pdevtmpl, (pdevtmpl ? pdevtmpl->type : "NULL"),
		(uint) ifname, (ifname ? ifname : "NULL")));

    soap_register_errors(UPNPErrors);

    if ((lanif = create_lan_interface(ifname)) == NULL) {
	UPNP_ERROR(("cannot use LAN interface \"%s\"\n", ifname));
	exit(1);
    } else {
	lanif->next = global_lans;
	global_lans = lanif;
    }

    device = init_device(NULL, pdevtmpl);

    UPNP_TRACE(("Generating UUIDs.\n"));
    generate_uuids();
    UPNP_TRACE(("Finished generating UUIDs.\n"));
    fflush(stdout);

    block_alarm();

    signal(SIGINT, interrupt_handler);
    signal(SIGTERM, interrupt_handler);

    memset(&timer, 0, sizeof(timer));
    timer.it_interval.tv_sec = 30;
    timer.it_value.tv_sec = 30;
    td1 = enqueue_event(&timer, (event_callback_t)connection_reaper, NULL);

    memset(&timer, 0, sizeof(timer));
    timer.it_interval.tv_sec = 30;
    timer.it_value.tv_sec = 30;
    td2 = enqueue_event(&timer, (event_callback_t)gena_subscription_reaper, NULL);

    memset(&timer, 0, sizeof(timer));
    timer.it_interval.tv_sec = ssdp_interval;
    timer.it_value.tv_sec = ssdp_interval;
    td3 = enqueue_event(&timer, (event_callback_t)periodic_advertiser, (void *) SSDP_ALIVE);

    UPNP_TRACE(("Sending initial advertisements.\n"));
    send_advertisements(SSDP_ALIVE);
    send_advertisements(SSDP_ALIVE);
    send_advertisements(SSDP_ALIVE);

    // enter the top of the event loop.
    //
    UPNP_TRACE(("Entering upnp loop\n"));

    global_exit_now = FALSE;
    while (global_exit_now == FALSE) {
	FD_ZERO(&rfds);

	n = fd_net_set(&rfds);

	unblock_alarm();
	n = select(n, &rfds, NULL, NULL, NULL);
	block_alarm();

	if (global_exit_now)
	    break;
	if (n > 0) 
	    process_receive_fds(&rfds);
    }

    UPNP_TRACE(("Exiting upnp loop\n"));

    /* cancel timers */
    dd_timer_delete(td1);
    dd_timer_delete(td2);
    dd_timer_delete(td3);

    delete_all_subscriptions();
    delete_all_connections();

    destroy_device(device);

    return 0;
}


void interrupt_handler(int i)
{
    send_advertisements(SSDP_BYEBYE);
    global_exit_now = TRUE;
}


/* 
   Strip characters in 'reject' from start and end of the string 'str'.
   Return a pointer to the start of the modified string.
*/
char *strip_chars(char *str, char *reject)
{
    char *end;

    str += strspn(str, reject);
    end = &str[strlen(str)-1];
    while (end > str && strpbrk(end, reject))
	*end-- = '\0';
	
    return str;
}



static struct iface *create_lan_interface(char *ifname)
{
    struct net_connection *nc;
    struct iface *pif = NULL;

    for (pif = (struct iface *)malloc(sizeof(struct iface));
	 pif != NULL;
	 free(pif), pif = NULL) {
	
	memset(pif, 0, sizeof(*pif));
	pif->ifname = ifname;

	if ( !osl_ifaddr(ifname, &pif->inaddr) ) {
	    UPNP_ERROR(("osl_ifaddr failed.\n"));
	    continue;
	}

	if ((nc = make_http_socket(pif)) == NULL) {
	    continue;
	}

	if ((nc = make_ssdp_socket(pif)) == NULL) {
	    // need to destroy the http connection here...
	    continue;
	}
	pif->ssdp_connection = nc;

	break;
    }

    return pif;
}


void reinit_lan_interface(struct iface *pif)
{
    struct net_connection *nc;

    do {
	delete_all_connections();
	
	if ( !osl_ifaddr(pif->ifname, &pif->inaddr) ) {
	    UPNP_ERROR(("osl_ifaddr failed.\n"));
	    continue;
	}
	
	if ((nc = make_http_socket(pif)) == NULL) {
	    continue;
	}
	
	if ((nc = make_ssdp_socket(pif)) == NULL) {
	    // need to destroy the http connection here...
	    continue;
	}
	pif->ssdp_connection = nc;
    } while (0);
}



/* Construct the bitmask of file descriptors that will be passed to select(2).
   For each network connection that we are maintaining, set the
   appropriate bit in a mask that will be used with a BSD-style select
   call. */
static int fd_net_set(fd_set *fds)
{
    struct net_connection *net;
    int max = 0;

    for (net = net_connections; net; net = net->next) {
	    if (net->fd > max)
		    max = net->fd;
	    FD_SET(net->fd, fds);
    }

    return (max + 1);
}

static void process_receive_fds(fd_set *fds)
{
    struct net_connection *net, *next;

    for (net = net_connections; net; net = next) {
	next = net->next;
	if (FD_ISSET(net->fd, fds)) {
	    assert(net->func);
	    (*(net->func))(CONNECTION_RECV, net, net->arg);
	}
    }
}

/* Big hammer to delete all existing connections. 
   Typically this should only be used in preparation for shutting down the UPNP server. 
*/
void delete_all_connections()
{
    struct net_connection **pnet, *net;

    pnet = &net_connections; 
    while (*pnet) {
	net = *pnet;
	*pnet = (*pnet)->next;
	assert(net->func);

	(*(net->func))(CONNECTION_DELETE, net, net->arg);
    }
}

void remove_net_connection(int fd)
{
    struct net_connection *net, **pnet;

    pnet = &net_connections;
    while (*pnet) {
	if ((*pnet)->fd == fd) {
	    net = *pnet;
	    *pnet = (*pnet)->next;
	    assert(net->func);
	    (*(net->func))(CONNECTION_DELETE, net, net->arg);
	    break;
	}
	pnet = &((*pnet)->next);
    }
}


static void connection_reaper(timer_t t, void *arg)
{
    struct net_connection **pnet, *net;
    time_t now;
    int i = 0;

    now = time(NULL);
    pnet = &net_connections;
    while (*pnet) {
	i++;
	if ((*pnet)->expires && (*pnet)->expires < now) {
	    net = *pnet;
	    *pnet = (*pnet)->next;
	    assert(net->func);
	    (*(net->func))(CONNECTION_DELETE, net, net->arg);
	    continue;
	}
	pnet = &((*pnet)->next);
    }
}





timer_t enqueue_event(struct itimerspec *value, event_callback_t func, void *arg)
{
    timer_t          td;

    dd_timer_create(CLOCK_REALTIME, NULL, &td);

    dd_timer_connect(td, (VOIDFUNCPTR) func, (int) arg);

    dd_timer_settime(td, 0, value, NULL);

    return(td);
}


static void block_alarm()
{
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    sigprocmask(SIG_BLOCK, &set, NULL);
}

static void unblock_alarm()
{
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    sigprocmask(SIG_UNBLOCK, &set, NULL);
}


// Create the unique device identifiers for each device.
//
static void generate_uuids()
{
    char *udn;
    PDevice pdev;

    forall_devices(pdev) {
	udn = malloc(50);
	strcpy(udn, "uuid:");
	uuidstr_create(udn+5, 50 - 5);
    
	// the device definition contains a unique string that should be substituted whenever 
	// the http server sends out the XML description document.
	pdev->udn = udn;
    }

}


static void delayed_call_helper(timer_t t, void *arg)
{
    voidfp_t f = (voidfp_t) arg;

    dd_timer_delete(t);
    (*f)();
}

void delayed_call(uint seconds, voidfp_t f)
{
    struct  itimerspec  timer;

    memset(&timer, 0, sizeof(timer));
    timer.it_interval.tv_sec = 0;
    timer.it_value.tv_sec = seconds;
    enqueue_event(&timer, (event_callback_t)delayed_call_helper, f);
}

