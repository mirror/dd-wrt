/* text2ip.c - Converts a text string to an IP address. */

/* Written 1998 by Werner Almesberger, EPFL ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "atm.h"
#include "atmd.h"


static void complain(const char *component,const char *item,const char *msg)
{
    if (!component) fprintf(stderr,"%s: %s\n",item,msg);
    else diag(component,DIAG_ERROR,"%s: %s",item,msg);
}


uint32_t text2ip(const char *text,const char *component,int flags)
{
    struct hostent *hostent;
    uint32_t ip;

    if (strspn(text,"0123456789.") == strlen(text)) {
	ip = inet_addr(text);
	if (ip != INADDR_NONE) return ip;
	if (flags & T2I_ERROR) complain(component,text,"invalid address");
	return INADDR_NONE;
    }
    if (!(flags & T2I_NAME)) {
	if (flags & T2I_ERROR)
	    complain(component,text,"numeric IP address expected");
	return INADDR_NONE;
    }
    hostent = gethostbyname(text);
    if (!hostent) {
	if (flags & T2I_ERROR) complain(component,text,"no such host");
	return INADDR_NONE;
    }
    if (hostent->h_addrtype != AF_INET) {
	if (flags & T2I_ERROR)
	    complain(component,text,"unknown address family");
	return INADDR_NONE;
    }
    memcpy(&ip,hostent->h_addr,hostent->h_length);
    return ip;
}
