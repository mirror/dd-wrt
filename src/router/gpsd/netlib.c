/* $Id: netlib.c 3666 2006-10-26 23:11:51Z ckuethe $ */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "gpsd_config.h"
#include "gpsd.h"

#if !defined (INADDR_NONE)
#define INADDR_NONE   ((in_addr_t)-1)
#endif

int netlib_connectsock(const char *host, const char *service, const char *protocol)
{
    struct hostent *phe;
    struct servent *pse;
    struct protoent *ppe;
    struct sockaddr_in sin;
    int s, type, one = 1;

    memset((char *) &sin, 0, sizeof(sin));
    /*@ -type -mustfreefresh @*/
    sin.sin_family = AF_INET;
    if ((pse = getservbyname(service, protocol)))
	sin.sin_port = htons(ntohs((unsigned short) pse->s_port));
    else if ((sin.sin_port = htons((unsigned short) atoi(service))) == 0)
	return NL_NOSERVICE;
    if ((phe = gethostbyname(host)))
	memcpy((char *) &sin.sin_addr, phe->h_addr, phe->h_length);
    else if ((sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
	return NL_NOHOST;
    if ((ppe = getprotobyname(protocol)) == 0)
	return NL_NOPROTO;
    if (strcmp(protocol, "udp") == 0)
	type = SOCK_DGRAM;
    else
	type = SOCK_STREAM;

    if ((s = socket(PF_INET, type, ppe->p_proto)) < 0)
	return NL_NOSOCK;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one))==-1) {
        (void)close(s);
	return NL_NOSOCKOPT;
    }
    if (connect(s, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
        (void)close(s);
	return NL_NOCONNECT;
    }
    return s;
    /*@ +type +mustfreefresh @*/
}

