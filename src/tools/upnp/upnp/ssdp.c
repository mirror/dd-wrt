/*
    Copyright 2007, Broadcom Corporation      
    All Rights Reserved.      
          
    THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY      
    KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM      
    SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS      
    FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.      
*/

#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"

// #include <bits/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>

#include <bcmnvram.h>

extern struct iface *global_lans;
extern struct net_connection *net_connections;
extern PDevice root_devices;

extern char *strip_chars(char *str, char *reject);
extern PDevice find_dev_by_udn(char *udn);

extern int max_age;

static void advertise_device(PDevice pdev, ssdp_t sstype, struct iface *pif, struct sockaddr *addr, int addrlen);
static void process_msearch(char *, struct iface *, struct sockaddr *, int);
static void ssdp_packet( UFILE *, ssdp_t, char * ntst, char *Usn, const char * location, int Duration);

const char* rfc1123_fmt = "%a, %d %b %Y %H:%M:%S GMT";

void send_advertisements(ssdp_t sstype);
void ssdp_receive(caction_t flag, struct net_connection *nc, struct iface *pif);

/* create a UDP socket bound to the SSDP multicast address. */
struct net_connection *make_ssdp_socket(struct iface *pif)
{
    struct net_connection *c = NULL;
    int                fd;
    u_char             ttl;
    
    fd = socket( AF_INET, SOCK_DGRAM, 0 );
    UPNP_SOCKET(("%s: socket returns %d\n", __FUNCTION__, fd));
    if (fd < 0) {
	goto error;
    }

    if (!osl_join_multicast(pif, fd, inet_addr(SSDP_IP), SSDP_PORT)) 
	goto error;
	
    // set the multicast TTL
    ttl = 4;
    if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl))) {
	goto error;
    }
    
    c = (struct net_connection *) malloc(sizeof(struct net_connection));
    if (c == NULL) 
	goto error;
    
    memset(c, 0, sizeof(struct net_connection));
    
    c->fd = fd;
    c->expires = 0;  // never expires.
    c->func = (CONNECTION_HANDLER) ssdp_receive;
    c->arg = pif;
    
    c->next = net_connections;
    net_connections = (struct net_connection *) c;
    
    /* a successful return ... */
    return c;

 error:
    UPNP_ERROR(("make_ssdp_socket failed\n"));
    UPNP_SOCKET(("%s: close %d\n", __FUNCTION__, fd));
    close(fd);
    fd = -1;
    return NULL;

}

void ssdp_receive(caction_t flag, struct net_connection *nc, struct iface *pif)
{
    char buf[UPNP_BUFSIZE];
    struct sockaddr_in srcaddr;
    int nbytes, addrlen;
    
    UPNP_TRACE(("%s\n", __FUNCTION__));
    if (flag == CONNECTION_RECV) {
	addrlen = sizeof(srcaddr);
	memset(&srcaddr, 0, addrlen);

	// memset(buf, 0, sizeof(buf));
	nbytes = recvfrom(nc->fd, buf, sizeof(buf), 0, (struct sockaddr*)&srcaddr, &addrlen);
	buf[nbytes] = '\0';
#if 1
    /* zhijian 2006-12-18 don't support SSDP mesage from wan side.*/
    {
      struct in_addr lanaddr;
      struct in_addr lanmask;
                                                                                                                             
      if(inet_aton(nvram_safe_get("lan_ipaddr"), &lanaddr))
      {
        if(inet_aton(nvram_safe_get("lan_netmask"), &lanmask))
        {
          if((srcaddr.sin_addr.s_addr & lanmask.s_addr) != (lanaddr.s_addr & lanmask.s_addr))
          {
            printf("ssdp_receive Warning: Unknown Ip address %s.\n",inet_ntoa(srcaddr.sin_addr));
            return;
          }
        }
      }
    }
#endif

	if (MATCH_PREFIX(buf, "M-SEARCH * HTTP/1.1")) {
#if 0
	    unsigned int lan_ip = 0, lan_mask = 0, src_ip = 0;
	
	    // for cdrouter_ssdp_2
	    inet_aton(inet_ntoa(srcaddr.sin_addr),(void *)&src_ip);
	    inet_aton(nvram_safe_get("lan_ipaddr"),(void *)&lan_ip);
	    inet_aton(nvram_safe_get("lan_netmask"),(void *)&lan_mask);

	    if((src_ip & lan_mask) != (lan_ip & lan_mask)) {
	    	UPNP_ERROR(("Can't respond to SSDP discovery requests on WAN side\n"));
		close(nc->fd);
		free(nc);
		return;
	    }
#endif
	    process_msearch(buf, pif, (struct sockaddr *)&srcaddr, addrlen);
	} 
	else if (MATCH_PREFIX(buf, "NOTIFY ")) 
	    ;
	else {
	    UPNP_ERROR(("unknown multicast message:\n%s\r\n", buf));
	}
    } else if (flag == CONNECTION_DELETE) {
	close(nc->fd);
	free(nc);
    }
}

static void process_msearch_all(char *st, struct iface *pif, struct sockaddr *srcaddr, int addrlen)
{
    PDevice pdev;

    UPNP_TRACE(("%s\n", __FUNCTION__));
    forall_devices(pdev) {
	advertise_device(pdev, SSDP_REPLY, pif, srcaddr,  addrlen);
    }
}

static const char *location(PDevice pdev, struct iface *pif)
{
    static char location[100];
    const unsigned char *bytep = (const unsigned char *) &(pif->inaddr.s_addr);

    snprintf(location, sizeof(location), "http://%d.%d.%d.%d:%d/dyndev/%s", 
	     bytep[0], bytep[1], bytep[2], bytep[3], HTTP_PORT, rootdev(pdev)->udn);

    return location;
}

static void process_msearch_rootdevice(char *st, struct iface *pif, struct sockaddr *srcaddr, int addrlen)
{
    UFILE *upkt;
    char tmpbuf[100];
    PDevice pdev;
    
    UPNP_TRACE(("%s\n", __FUNCTION__));
    if ((upkt = usopen(NULL, 0))) {
	
	for (pdev = root_devices; pdev; pdev = pdev->next) {
	    snprintf(tmpbuf, sizeof(tmpbuf), "%s::upnp:rootdevice", pdev->udn);

	    // build the SSDP packet
	    ssdp_packet( upkt, SSDP_REPLY, st, tmpbuf, location(pdev, pif), max_age);
	    
	    if (sendto(pif->ssdp_connection->fd, ubuffer(upkt), utell(upkt), 0, srcaddr, addrlen) < 0)
		UPNP_ERROR(("sendto failed at %s:%d - err %d\n", __FILE__, __LINE__, errno));
	    uflush(upkt); 	// reset the packet buffer before we build another.
	}
	uclose(upkt);
    }
}


static void process_msearch_uuid(char *st, struct iface *pif, struct sockaddr *srcaddr, int addrlen)
{
    UFILE *upkt;
    PDevice pdev;

    UPNP_TRACE(("%s\n", __FUNCTION__));
    if ((upkt = usopen(NULL, 0))) {
	if ((pdev = find_dev_by_udn(st)) != NULL) {

	    // build the SSDP packet
	    ssdp_packet( upkt, SSDP_REPLY, pdev->udn, pdev->udn,
			 location(pdev, pif), max_age);

	    if (sendto(pif->ssdp_connection->fd, ubuffer(upkt), utell(upkt), 0, srcaddr, addrlen) < 0)
		UPNP_ERROR(("sendto failed at %s:%d - err %d\n", __FILE__, __LINE__, errno));
	}    

	uclose(upkt);
    }
}


static void process_msearch_devicetype(char *st, struct iface *pif, struct sockaddr *addr, int addrlen)
{
    UFILE *upkt;
    PDevice pdev;
    char tmpbuf[100];

    UPNP_TRACE(("%s\n", __FUNCTION__));
    if ((upkt = usopen(NULL, 0))) {
	forall_devices(pdev) {
	    if (strcmp(pdev->template->type, st) == 0) {
		snprintf(tmpbuf, sizeof(tmpbuf), "%s::%s", pdev->udn, pdev->template->type);

		// build the SSDP packet
		ssdp_packet( upkt, SSDP_REPLY, pdev->template->type, tmpbuf,
			     location(pdev, pif), max_age);
		if (sendto(pif->ssdp_connection->fd, ubuffer(upkt), utell(upkt), 0, addr, addrlen) < 0)
		    UPNP_ERROR(("sendto failed at %s:%d - err %d\n", __FILE__, __LINE__, errno));
		uflush(upkt); 	// reset the packet buffer before we build another.
	    }
	}

	uclose(upkt);
    }
}


static void process_msearch_service(char *st, struct iface *pif, struct sockaddr *addr, int addrlen)
{
    UFILE *upkt;
    char *name = NULL, *p;
    int namelen = 0;
    PDevice pdev;
    PService psvc;
    char tmpbuf[100];
    
    UPNP_TRACE(("%s\n", __FUNCTION__));
    p = strstr(st, ":service:");
    if (p) {
	name = p+strlen(":service:");
	namelen = strlen(name);
    }
    
    if (namelen) {
	UPNP_TRACE(("%s: looking for service \"%s\"\n", __FUNCTION__, name));
	if ((upkt = usopen(NULL, 0))) {
	    forall_devices(pdev) {
		const char *loc = location(pdev, pif);
		forall_services(pdev, psvc) {
		    UPNP_TRACE(("\tcomparing to \"%s\"\n", psvc->template->name));
		    if (strlen(psvc->template->name) == namelen && strncmp(psvc->template->name, name, namelen) == 0) {
			UPNP_TRACE(("\tmatched!\n"));
			snprintf(tmpbuf, sizeof(tmpbuf), "%s::%s", pdev->udn, st);

			// build the SSDP packet
			ssdp_packet(upkt, SSDP_REPLY, st, tmpbuf,
				    loc, max_age);

			if (sendto(pif->ssdp_connection->fd, ubuffer(upkt), utell(upkt), 0, addr, addrlen) < 0)
			    UPNP_ERROR(("sendto failed at %s:%d - err %d\n", __FILE__, __LINE__, errno));
			uflush(upkt); 	// reset the packet buffer before we build another.
		    } else {
			UPNP_TRACE(("\tdoes not match\n"));
		    }
		}
	    }

	    uclose(upkt);
	}
    } else {
	UPNP_TRACE(("%s: namelen == 0\n", __FUNCTION__));
    }
}

/* process an SSDP M-SEARCH request. */
static void process_msearch(char *msg, struct iface *pif, struct sockaddr *srcaddr, int addrlen)
{
    char *line, *body, *p, *mxend;
    char *st = NULL;   // the ST: header
    char *mx = NULL;   // the MX: header
    char *man = NULL;  // the MAN: header
    long int mxval;

    UPNP_TRACE(("%s\n", __FUNCTION__));
    if ( (body = strstr(msg, "\r\n\r\n" )) != NULL )
	body += 4;
    else if ( (body = strstr(msg, "\r\n" )) != NULL )
	body += 2; 
    else {
	UPNP_ERROR(("M-SEARCH is missing blank line after header!\n%s", msg));
	return;
    }

    p = msg;
    while (p != NULL && p < body) {
	line = strsep(&p, "\r\n");
	if (IMATCH_PREFIX(line, "ST:")) {
	    st = strip_chars(&line[3], " \t");
	} else if (IMATCH_PREFIX(line, "MX:")) {
	    mx = strip_chars(&line[3], " \t");
	} else if (IMATCH_PREFIX(line, "MAN:")) {
	    man = strip_chars(&line[4], " \t");
	}
    }

    if (!st || !mx || !man) 
	UPNP_ERROR(("M-SEARCH is missing required ST:, MX:, or MAN: header!\r\n"));
    else if (!man || (strcmp(man, "\"ssdp:discover\"") != 0)) 
	UPNP_ERROR(("M-SEARCH has invalid MAN: header - \"%s\"\r\n", man));
    else {
	mxval = strtol(mx, &mxend, 10);
	if (mxend == mx || *mxend != '\0' || mxval < 0) {
	    UPNP_ERROR(("M-SEARCH has invalid MX: header - \"%s\"\r\n", mx));
	    return;
	}
	if (strcmp(st, "ssdp:all") == 0) {
	    // Is client searching for any and all devices and services?
	    //
	    process_msearch_all(st, pif, srcaddr, addrlen);
	}
	else if (strcmp(st, "upnp:rootdevice") == 0) {
	    // Is client searching for root devices?
	    //
	    process_msearch_rootdevice(st, pif, srcaddr, addrlen);
	}
	else if (MATCH_PREFIX(st, "uuid:")) {
	    // Is client searching for a particular device ID?
	    //
	    process_msearch_uuid(st, pif, srcaddr, addrlen);
	}
	else if (MATCH_PREFIX(st, "urn:") && strstr(st, ":device:")) {
	    // Is client searching for a particular device type?
	    //
	    process_msearch_devicetype(st, pif, srcaddr, addrlen);
	}
	else if (MATCH_PREFIX(st, "urn:") && strstr(st, ":service:")) {
	    // Is client searching for a particular service type?
	    //
	    process_msearch_service(st, pif, srcaddr, addrlen);
	} else {
	    UPNP_ERROR(("unrecognized ST: header - \"%s\"\r\n", st));
	}
    }
}


/* Should really call this twice to give the UDP packets a better chance of reaching their destination. 
*/
static void advertise_device(PDevice pdev, ssdp_t sstype, struct iface *pif, struct sockaddr *addr, int addrlen)
{
    UFILE *upkt;
    PService psvc;
    char tmpbuf[100], svctype[100];
    const char *loc = location(pdev, pif);

    // create a growable string UFILE.
    if ((upkt = usopen(NULL, 0))) {

	if (ISROOT(pdev)) {
	    snprintf(tmpbuf, sizeof(tmpbuf), "%s::upnp:rootdevice", pdev->udn);
	    ssdp_packet(
		upkt, sstype, "upnp:rootdevice", tmpbuf,
		loc, max_age);
	    if (sendto(pif->ssdp_connection->fd, ubuffer(upkt), utell(upkt), 0, addr, addrlen) < 0)
		UPNP_ERROR(("sendto failed at %s:%d - err %d\n", __FILE__, __LINE__, errno));
	    uflush(upkt); 	// reset the packet buffer before we build another.
	}

	// advertise device by UDN
	ssdp_packet( upkt, sstype, pdev->udn, pdev->udn, 
		     loc, max_age);
	if (sendto(pif->ssdp_connection->fd, ubuffer(upkt), utell(upkt), 0, addr, addrlen) < 0)
	    UPNP_ERROR(("sendto failed at %s:%d - err %d\n", __FILE__, __LINE__, errno));
	uflush(upkt); 	// reset the packet buffer before we build another.
    
	// advertise device by combination of UDN and type
	snprintf(tmpbuf, sizeof(tmpbuf), "%s::%s", pdev->udn, pdev->template->type);
	ssdp_packet( upkt, sstype, pdev->template->type, tmpbuf,
		     loc, max_age);
	if (sendto(pif->ssdp_connection->fd, ubuffer(upkt), utell(upkt), 0, addr, addrlen) < 0)
	    UPNP_ERROR(("sendto failed at %s:%d - err %d\n", __FILE__, __LINE__, errno));
	uflush(upkt); 	// reset the packet buffer before we build another.

	// advertise the services...
	for (psvc = pdev->services; psvc; psvc = psvc->next) {
	    snprintf(svctype, sizeof(svctype), "urn:%s:service:%s", psvc->template->schema, psvc->template->name);
	    snprintf(tmpbuf, sizeof(tmpbuf), "%s::%s", pdev->udn, svctype);

	    ssdp_packet( upkt, sstype, svctype, tmpbuf,
			 loc, max_age);

	    if (sendto(pif->ssdp_connection->fd, ubuffer(upkt), utell(upkt), 0, addr, addrlen) == -1) 
		UPNP_ERROR(("sendto failed at %s:%d - err %d\n", __FILE__, __LINE__, errno));
	    uflush(upkt); 	// reset the packet buffer before we build another.
	}

	// release the growable string UFILE.
	uclose(upkt);
    }

}

void periodic_advertiser(timer_t t, ssdp_t sstype)
{
    send_advertisements(sstype);
}

/* Construct the multicast address and send out the appropriate device advertisement packets. */
void send_advertisements(ssdp_t sstype)
{
    struct sockaddr_in addr;
    struct iface *pif;
    PDevice pdev;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(SSDP_IP);
    addr.sin_port = htons(SSDP_PORT);

    for (pif = global_lans; pif; pif = pif->next) {
	forall_devices(pdev) {
	    advertise_device(pdev, sstype, pif, (struct sockaddr *)&addr,  sizeof(addr));
	}
    }
}


static void ssdp_packet(
    UFILE *up, ssdp_t ptype,
    char * ntst, char *Usn, 
    const char * location, int Duration)
{
    extern const char* rfc1123_fmt;
    time_t now;
    char date[100];

    UPNP_TRACE(("%s\n", __FUNCTION__));
    switch (ptype) {
    case SSDP_REPLY:
	uprintf(up, "HTTP/1.1 200 OK\r\n");
	uprintf(up, "ST:%s\r\n", ntst);
	uprintf(up, "USN:%s\r\n",Usn);
	uprintf(up, "Location: %s\r\n", location);
	uprintf(up, "Server: Custom/1.0 UPnP/1.0 Proc/Ver\r\n");
	uprintf(up, "EXT:\r\n");
	uprintf(up, "Cache-Control:max-age=%d\r\n", Duration);
	now = time( (time_t*) 0 );
	(void) strftime( date, sizeof(date), rfc1123_fmt, gmtime( &now ) );
	uprintf(up, "DATE: %s\r\n", date);
	break;
    case SSDP_ALIVE:
	uprintf(up, "NOTIFY * HTTP/1.1 \r\n");
	uprintf(up, "HOST: %s:%d\r\n", SSDP_IP, SSDP_PORT);
	uprintf(up, "CACHE-CONTROL: max-age=%d\r\n", Duration);
	uprintf(up, "Location: %s\r\n", location);
	uprintf(up, "NT: %s\r\n", ntst);
	uprintf(up, "NTS: ssdp:alive\r\n");
	uprintf(up, "SERVER:%s\r\n", SERVER);
    uprintf(up, "USN: %s\r\n",Usn);
	break;
    case SSDP_BYEBYE:
	uprintf(up, "NOTIFY * HTTP/1.1 \r\n");
	uprintf(up, "HOST: %s:%d\r\n", SSDP_IP, SSDP_PORT);

	// Following two header is added to interop with Windows Millenium but this is not
	// a part of UPNP spec 1.0 
	uprintf(up, "CACHE-CONTROL: max-age=%d\r\n",Duration);
	uprintf(up, "Location: %s\r\n", location);
	uprintf(up, "NT: %s\r\n", ntst);
	uprintf(up, "NTS: ssdp:byebye\r\n");  
    uprintf(up, "USN: %s\r\n",Usn);
	break;
    default:
	UPNP_ERROR(("Bad SSDP packet type supplied - %d\r\n", ptype));
    }

    /*     uprintf(up, "USN: %s\r\n",Usn); */
    uprintf(up, "\r\n");
}


