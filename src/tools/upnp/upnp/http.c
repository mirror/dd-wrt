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

#include <ctype.h>
#include <bcmnvram.h>

extern const char* rfc1123_fmt;
extern struct net_connection *net_connections;
extern PDevice root_devices;

extern char *strip_chars(char *str, char *reject);
extern void fd_http_notify_set(fd_set *fds);
extern PService find_svc_by_url(char *url);
extern void remove_net_connection(int fd);
extern void service_xml(PService psvc, UFILE *);
extern void device_xml(PDevice pdev, UFILE *);
extern PDevice find_dev_by_udn(char *udn);
extern void gena_renew_subscription(UFILE *, PService psvc, char *cb, char *sid, char *to, char *nt);
extern void gena_new_subscription(UFILE *, PService psvc, char *cb, char *sid, char *to, char *nt);
extern void soap_action(UFILE *, PService , char *, char *);

extern const char *global_lan_ifname;


struct http_connection {
    struct net_connection   net;
    char		    *buf;
    u_int16		    bytes_recvd;
    u_int16		    maxbytes;
    UFILE		    *up;
};

// forward declarations of routines defined in this file.

static void process_subscribe(UFILE *, char *, char *);
extern void process_unsubscribe(UFILE *, char *, char *);
static void process_post(UFILE *, char *, char *);
static void process_get( UFILE *up, char *fname, char *msg );
static void strdecode( char* to, char* from );
static void dispatch_http_request(struct http_connection *c, char *request, int request_len);
static PService find_service_by_name(char *name);
static void process_allxml( UFILE *up, char *fname );

void accept_http_connection(caction_t, struct net_connection *, void *);
void http_receive(caction_t, struct http_connection *, void *);
int process_http_request(struct http_connection *c);
int do_substitions(var_entry_t *, int, char *, char *);

Error HTTPErrors[] = {
    { HTTP_OK, "OK" },
    { HTTP_SERVER_ERROR, "Internal Server Error" },
    { HTTP_BAD_REQUEST, "Bad Request" },
    { HTTP_NOT_FOUND, "Not Found" },
    { HTTP_NOT_IMPLEMENTED, "Not Implemented" },
    { HTTP_FORBIDDEN, "Forbidden" },
    { HTTP_PRECONDITION, "Precondition Failed" },
    { 0, NULL }
};



/* Before calling this routine, the ip address should have been set by
   looking up the address bound to a particular interface we want to run
   the server on.  
*/
struct net_connection *make_http_socket(struct iface *pif)
{
    struct net_connection *c = NULL;
    struct sockaddr_in sockaddr;
    int fd, flags;

    /* create our HTTP socket. */
    fd = socket( AF_INET, SOCK_STREAM, 0);
    UPNP_SOCKET(("%s: socket returns %d\n", __FUNCTION__, fd));
    
    if (fd < 0) 
	goto error;

    flags = TRUE;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&flags, sizeof(flags));
    
    memcpy(&sockaddr.sin_addr, &pif->inaddr, sizeof(sockaddr.sin_addr));
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons( HTTP_PORT );
	
    if ( bind(fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0 ) {
	UPNP_ERROR(("make_http_socket cannot bind to port %d - err %d.\n", HTTP_PORT, errno));
	goto error;
    }
    if ( listen(fd, 10) ) {
	UPNP_ERROR(("make_http_socket cannot listen - err %d\n", errno));
	goto error;
    }
	
    c = (struct net_connection *) malloc(sizeof(struct net_connection));
    if (c == NULL) {
	UPNP_ERROR(("make_http_socket cannot malloc.\n"));
	goto error;
    }
	
    memset(c, 0, sizeof(struct net_connection));

    c->fd = fd;
    c->expires = 0;  // never expires.
    c->func = (CONNECTION_HANDLER) accept_http_connection;
    c->arg = (void *) fd;

    c->next = net_connections;
    net_connections = (struct net_connection *) c;

    return c;

 error:
    /* cleanup code in case we need to bail out. */
    UPNP_ERROR(("%s failed - err %d\n", __FUNCTION__, errno));
    if (fd >= 0) {
	UPNP_SOCKET(("%s: close %d\n", __FUNCTION__, fd));
	close(fd);
    }
	
    return NULL;
}


void accept_http_connection(caction_t flag, struct net_connection *nc, void *arg)
{
    int sock = nc->fd;
    struct http_connection *hc;
    struct sockaddr addr;
    int addrlen;
    int s;

    switch (flag) {

    case CONNECTION_RECV: 
	memset(&addr, 0, sizeof(addr));
	addrlen = sizeof(addr);
	s = accept(sock, &addr, &addrlen);
	UPNP_SOCKET(("%s: accept returns %d\n", __FUNCTION__, s));
	UPNP_HTTP(("connection %d received on server %d\n", s, sock));
	if (s == -1) {
	    if (errno != EAGAIN)
		perror("accept");
	} else {

	    UPNP_TRACE(("Accepting http connection %d\n", s));
	    hc = (struct http_connection *) malloc(sizeof(struct http_connection));
	    if (!hc) {
		UPNP_SOCKET(("%s: close %d\n", __FUNCTION__, s));
		close(s);
	    } else {
		memset(hc, 0, sizeof(struct http_connection));
		    
		hc->net.fd = s;
		hc->net.expires = time(NULL) + HTTP_REQUEST_TIMEOUT;
		hc->net.func = (CONNECTION_HANDLER) http_receive;
		hc->net.arg = NULL;

		hc->up = udopen(hc->net.fd);
		hc->buf = malloc(HTTP_BUFSIZE);
		memset(hc->buf, 0, HTTP_BUFSIZE);
		hc->maxbytes = HTTP_BUFSIZE;
		hc->bytes_recvd = 0;

		hc->net.next = net_connections;
		net_connections = (struct net_connection *) hc;

	    } 
	}
	break;

    case CONNECTION_DELETE: 
	UPNP_HTTP(("http server %d deleted\n", nc->fd));
	UPNP_SOCKET(("%s: close %d\n", __FUNCTION__, nc->fd));
	close(nc->fd);
	free(nc);
	break;
    } /* end switch */
}

void http_receive(caction_t flag, struct http_connection *c, void *arg)
{
    int nbytes, tear_down;
    
    switch (flag) {
    case CONNECTION_RECV:

	if ((c->maxbytes - c->bytes_recvd) < HTTP_BUF_LOWATER) {
	    c->maxbytes += HTTP_BUF_INCREMENT;
	    c->buf = realloc(c->buf, c->maxbytes);
	    assert(c->buf != NULL);
	}

	tear_down = TRUE;
	nbytes = read(c->net.fd, c->buf + c->bytes_recvd, c->maxbytes - c->bytes_recvd - 1);
	if (nbytes > 0) {
	    UPNP_HTTP(("data received on http connection %d\n", c->net.fd));
	    c->bytes_recvd += nbytes;

	    // reset the expiration counter each time we receive some data.
	    c->net.expires = time(NULL) + HTTP_REQUEST_TIMEOUT;
	
	    // See if we have enough information to process this request yet.  
	    // If not, process_http_request() will return FALSE without doing much of anything.
	    //
	    if (process_http_request(c)) {
		// we are going to keep this connection alive, so 
		// reset the input count of this connection. 
		c->bytes_recvd = 0;
	    }
	} else if (nbytes == 0) {
	    // if we have read EOF on the connection or if the http request was complete,
	    // or if read() returned an error, tear down this connection.
	    //
	    UPNP_HTTP(("removing http connection %d\n", c->net.fd));
	    remove_net_connection(c->net.fd);
	} else if (tear_down) {
	    /* Keep-alive */
	    c->bytes_recvd = 0;
	}
	break;
	
    case CONNECTION_DELETE:
	UPNP_HTTP(("connection %d closed\n", c->net.fd));
	UPNP_SOCKET(("%s: close %d\n", __FUNCTION__, c->net.fd));
	close(c->net.fd);
	uclose(c->up);
	free(c->buf);
	free(c);
	break;
    } /* end switch */
}

		    
// See if we have enough information to process this request yet.  
// Return TRUE if this connection has been completed and should be 
// released.
//
int process_http_request(struct http_connection *c)
{
    int header_length, content_length, request_complete;
    char *p, *end, *body;
    
    request_complete = FALSE;   // assume we will fail this routine...
    c->buf[c->bytes_recvd] = '\0';
    if ( (body = strstr( c->buf, "\r\n\r\n" )) != NULL ||
	 (body = strstr( c->buf, "\n\n" )) != NULL ) {
	// We've got the complete header.  

	// scan the header for a content-length header.	
	content_length = 0;
	p = c->buf;
	while (p < body) {
	    end = strpbrk(p, "\r\n");
	    if (end > p && IMATCH_PREFIX(p, "content-length:")) {
		// found a content-length header
		content_length = atol(p+15);
		break;
	    }
	    p = end+1;
	}

	header_length = (body - c->buf) + ((*body == '\r') ? 4 : 2);
	if (content_length <= c->bytes_recvd - header_length) {
	    dispatch_http_request(c, c->buf, c->bytes_recvd);
	    request_complete = TRUE;
	}
    }

    return request_complete;
}

static int process_index(UFILE *up) 
{
    char date[500];
    time_t now;

    /* Prepare a GMT date string for use in the http header. */
    now = time( (time_t*) 0 );
    (void) strftime( date, sizeof(date), rfc1123_fmt, gmtime( &now ) );

    uprintf(up, "HTTP/1.0 200 OK\r\n");
    uprintf(up, "DATE: %s\r\n", date);
    uprintf(up, "CONTENT-TYPE: text/html\r\n");
    uprintf(up, "Cache-Control: no-cache\r\n");
    uprintf(up, "PRAGMA: no-cache\r\n");
    uprintf(up, "Connection: Close\r\n" );
    uprintf(up, "\r\n" );
    uprintf(up, "<html><body OnLoad=window.location.replace('http://%s')></body></html>", nvram_safe_get("lan_ipaddr"));

    uflush(up);
}

/* return TRUE if the socket should be close, FALSE otherwise. */
static void dispatch_http_request(struct http_connection *c, char *request, int request_len)
{
    char* methodstr;
    char* path;
    char *fname;
    char * lines = request, *line;
    
    // process first line here to find out what kind of request it is...
    //
    line = strsep(&lines, "\r\n");

    methodstr = strsep(&line, " \t");
    path = strsep(&line, " \t");
    
    if (path)
	strdecode( path, path );
    
    UPNP_HTTP(("dispatching method \"%s\"  path \"%s\"\n", methodstr, path));
    if (!line || !methodstr || !path || path[0] != '/') {
	UPNP_HTTP(("Bad HTTP request\n"));
	http_error( c->up, HTTP_BAD_REQUEST );
    } else {
	fname = &(path[1]);
	if ( fname[0] == '\0' )
	    fname = "./";

	if ( strcasecmp(methodstr, "GET") == 0 ) {
	    if(!strcmp(path, "/index.asp")) {
		process_index(c->up);
	    }
	    else {
	        process_get(c->up, fname, lines);
	    }
	    remove_net_connection(c->net.fd);
	}
	else if ( strcasecmp(methodstr, "SUBSCRIBE") == 0 ) {
	    process_subscribe(c->up, fname, lines);
	}
	else if ( strcasecmp(methodstr, "UNSUBSCRIBE") == 0 ) {
	    process_unsubscribe(c->up, fname, lines);
	}
	else if ( strcasecmp(methodstr, "POST" ) == 0 ) {
	    UPNP_HTTP(("POST %s\n", fname));
	    process_post(c->up, fname, lines);
	}
	else {
	    http_error( c->up, HTTP_NOT_IMPLEMENTED );
	}
    } 
}


void http_error( UFILE *up, http_error_t error_code )
{
    http_response(up, error_code, NULL, 0);
}

static int
hexit( char c )
{
    if ( c >= '0' && c <= '9' )
	return c - '0';
    if ( c >= 'a' && c <= 'f' )
	return c - 'a' + 10;
    if ( c >= 'A' && c <= 'F' )
	return c - 'A' + 10;
    return 0;           /* shouldn't happen, we're guarded by isxdigit() */
}


/* Copies and decodes a hexadecimal string.  It's ok for from and to to be the
** same string.
*/
static void 
strdecode( char* to, char* from )
{
    for ( ; *from != '\0'; ++to, ++from ) {
	if ( from[0] == '%' && isxdigit( (int) from[1] ) && isxdigit( (int) from[2] ) ) {
	    *to = hexit( from[1] ) * 16 + hexit( from[2] );
	    from += 2;
	}
	else
	    *to = *from;
    }
    *to = '\0';
}


void http_response(UFILE *up, http_error_t error_code, const char *body, int body_len)
{
    static Error last_ditch = (Error) { 900, "Unknown HTTP error!" };
    char date[100];
    time_t now;
    PError pe;

    if (body == NULL) {
	body = "";
	body_len = 0;
    }

    for (pe = HTTPErrors; pe->error_string; pe++) {
	if (pe->error_code == error_code)
	    break;
    }
    
    if (pe->error_string == NULL) 
	pe = &last_ditch;

    uprintf(up, "HTTP/1.1 %d %s\r\n", pe->error_code, pe->error_string);
    now = time( (time_t*) 0 );
    (void) strftime( date, sizeof(date), rfc1123_fmt, gmtime( &now ) );
    uprintf(up, "DATE: %s\r\n", date);
    uprintf(up, "Connection: Keep-Alive\r\n");
    uprintf(up, "Server: %s\r\n", SERVER);
    uprintf(up, "Content-Length: %d\r\n", body_len);
    uprintf(up, "Content-Type: text/xml; charset=\"utf-8\"\r\n");
    uprintf(up, "EXT:\r\n");
    uprintf(up, "\r\n" );

    uflush(up);

    if (write(ufileno(up), (char *) body, body_len) == -1) 
	UPNP_ERROR(("%s write error %d\n", __FUNCTION__, errno));
    

}



static void process_post(UFILE *up, char *fname, char *msg)
{
    PService psvc;
    char *p, *line, *body, *sa = NULL;

    if ((psvc = find_svc_by_url(fname)) == NULL) {
	http_error( up, HTTP_NOT_FOUND );
    } else {
	if ( (body = strstr(msg, "\r\n\r\n" )) != NULL )
	    body += 4;
	else if ( (body = strstr(msg, "\n\n" )) != NULL )
	    body += 2;
	
	p = msg;
	while (p != NULL && p < body) {
	    line = strsep(&p, "\r\n");
	    // Search for a soapaction header, which looks like this,
	    // SOAPACTION: urn:schemas-upnp-org:service:WANPPPConnection:1#GetExternalIPAddress
	    if (IMATCH_PREFIX(line, "SOAPACTION:")) {
		sa = strip_chars(&line[11], " \t<>\"");
		break;
	    }
	}
	
	soap_action(up, psvc, sa, body);
    } 
}



/* 

   Process a GENA SUBSCRIBE message that looks like this,

    SUBSCRIBE /upnp/event/lanhostconfigmanagement1 HTTP/1.1
    NT: upnp:event
    Callback: <http://10.19.13.45:5000/notify>
    Timeout: Second-1800
    User-Agent: Mozilla/4.0 (compatible; UPnP/1.0; Windows NT/5.1)
    Host: 10.19.13.136:5431
    Content-Length: 0
    Pragma: no-cache
*/
static void process_subscribe(UFILE *up, char *fname, char *msg)
{
    PService psvc;
    char *p, *body, *line, *cb = NULL, *sid = NULL, *to = NULL, *nt = NULL;

    if ((psvc = find_svc_by_url(fname)) == NULL) {
	UPNP_ERROR(("Cannot subscribe - service \"%s\" not found.\n", fname));
	http_error( up, HTTP_NOT_FOUND );
    } else {

	if ( (body = strstr(msg, "\r\n\r\n" )) != NULL )
	    body += 4;
	else if ( (body = strstr(msg, "\n\n" )) != NULL )
	    body += 2;
	
	p = msg;
	while (p != NULL && p < body) {
	    line = strsep(&p, "\r\n");
	    if (IMATCH_PREFIX(line, "Callback:")) {
		cb = strip_chars(&line[9], " \t<>");
	    } else if (IMATCH_PREFIX(line, "SID:")) {
		sid = strip_chars(&line[4], " \t");
	    } else if (IMATCH_PREFIX(line, "TIMEOUT:")) {
		to = strip_chars(&line[8], " \t");
	    } else if (IMATCH_PREFIX(line, "NT:")) {
		nt = strip_chars(&line[3], " \t");
	    }
	}
	
	if (nt) {
	    gena_new_subscription(up, psvc, cb, sid, to, nt);
	} else {
	    gena_renew_subscription(up, psvc, cb, sid, to, nt);
	}
    } 
}


/* return TRUE if the socket should be close, FALSE otherwise. */
static void process_get( UFILE *up, char *fname, char *msg )
{
    PService psvc = NULL;
    PDevice pdev = NULL;
    char date[500], *name;
    time_t now;
    UFILE *unull;
    int content_length = 0;

    /* strip leading '/' characters */
    while (*fname == '/')
	fname++;

    /* path should now have the form 'dynsvc/<svcname>.xml' or 
       'dyndev/<devname>.xml'. 
       
       Parse that path and look up the appropriate device or service pointer. 
    */
    name = rindex(fname, '/');
    if (name) {
	name++;  /* advance past the '/' */
	name = strsep(&name, ".");
	if (IMATCH_PREFIX(fname, "dynsvc")) {
	    psvc = find_service_by_name(name);
	} else if (IMATCH_PREFIX(fname, "dyndev")) {
	    pdev = find_dev_by_udn(name);
	} else if (IMATCH_PREFIX(fname, "allxml")) {
	    process_allxml(up, fname );
	    return;
	}
    } 

    /* If we failed to look up a device or service pointer then this request fails. */
    if (!psvc && !pdev) {
	http_error( up, HTTP_NOT_FOUND );
	return;
    }

    /* Open a null UFILE in order to calculate the length of the response. 
       This value will be used on the Content-Length: HTTP header. */
    unull = ufopen(OSL_NULL_FILE);
    if (unull) {
	if (psvc) {
	    UPNP_TRACE(("calling service_xml for \"%s\"\n", name));
	    service_xml(psvc, unull);
	} else if (pdev) {
	    UPNP_TRACE(("calling device_xml for \"%s\"\n", name));
	    device_xml(pdev, unull);
	}
	content_length = utell(unull);
	uclose(unull);
    }

    /* Fail if we could not generate the expected content. */
    if (content_length == 0) {
	http_error( up, HTTP_SERVER_ERROR );
	return;
    }


    /* Prepare a GMT date string for use in the http header. */
    now = time( (time_t*) 0 );
    (void) strftime( date, sizeof(date), rfc1123_fmt, gmtime( &now ) );
    
    /* Send the HTTP response header */
    uprintf(up, "HTTP/1.0 200 OK\r\n");
    uprintf(up, "SERVER: %s\r\n", SERVER);
    uprintf(up, "DATE: %s\r\n", date);
    /* uprintf(up, "CONTENT-TYPE: text/xml\r\n"); */
    uprintf(up, "CONTENT-TYPE: application/octet-stream\r\n");
    uprintf(up, "Cache-Control: max-age=1\r\n");
    uprintf(up, "PRAGMA: no-cache\r\n");
    uprintf(up, "Connection: Close\r\n" );
    uprintf(up, "\r\n" );
    uflush(up);
    
    /* Send the HTTP response body */
    if (psvc) {
	UPNP_TRACE(("calling service_xml for \"%s\"\n", name));
	service_xml(psvc, up);
    } else if (pdev) {
	UPNP_TRACE(("calling device_xml for \"%s\"\n", name));
	device_xml(pdev, up);
    }
    uflush(up);
}

static PService find_service_by_name(char *name)
{
    PService psvc = NULL;
    PDevice pdev;

    forall_devices(pdev) {
	forall_services(pdev, psvc) {
	    if (strcmp(name, psvc->template->name) == 0) {
		break;
	    }
	}
	if (psvc)
	    break;
    }

    return psvc;
}

/* return TRUE if the socket should be close, FALSE otherwise. */
static void process_allxml( UFILE *up, char *fname )
{
    PService psvc;
    PDevice pdev;
    UFILE *unull;
    int content_length = 0;

    /* Open a null UFILE in order to calculate the length of the response. 
       This value will be used on the Content-Length: HTTP header. */
    unull = ufopen(OSL_NULL_FILE);
    if (unull) {
	uprintf(unull, "<allxml>");

	pdev = NULL;
	forall_devices(pdev) {
	    uprintf(unull, "<devicedesc>%s</devicedesc>\r\n", pdev->template->type); 
	    device_xml(pdev, unull);
	    break;
	}

	pdev = NULL;
	forall_devices(pdev) {
	    forall_services(pdev, psvc) {
		uprintf(unull, "<servicedesc>/dynsvc/%s.xml</servicedesc>\r\n",  psvc->template->name); 
		service_xml(psvc, unull);
	    }
	}
	uprintf(unull, "</allxml>");

	content_length = utell(unull);
	uclose(unull);
    }


    /* Send the HTTP response header */
    uprintf(up, "HTTP/1.1 200 OK\r\n");
    uprintf(up, "SERVER: %s\r\n", SERVER);
    /* uprintf(up, "CONTENT-TYPE: text/xml\r\n"); */
    uprintf(up, "CONTENT-TYPE: application/octet-stream\r\n");
    uprintf(up, "Cache-Control: max-age=1\r\n");
    uprintf(up, "PRAGMA: no-cache\r\n");
    uprintf(up, "Content-Length: %d\r\n", content_length);
    uprintf(up, "Connection: Keep-Alive\r\n" );
    uprintf(up, "\r\n" );
    uflush(up);

    uprintf(up, "<allxml>");

    pdev = NULL;
    forall_devices(pdev) {
	uprintf(up, "<devicedesc>%s</devicedesc>\r\n", pdev->template->type); 
	device_xml(pdev, up);
	uflush(up);
	break;
    }
 
    pdev = NULL;
    forall_devices(pdev) {
	forall_services(pdev, psvc) {
	    uprintf(up, "<servicedesc>/dynsvc/%s.xml</servicedesc>\r\n",  psvc->template->name); 
	    service_xml(psvc, up);
	}
    }
    uprintf(up, "</allxml>");
    uflush(up);
}
