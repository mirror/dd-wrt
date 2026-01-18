/* $Id: natpmpc.c,v 1.13 2012/08/21 17:23:38 nanard Exp $ */
/* libnatpmp
Copyright (c) 2007-2011, Thomas BERNARD
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * The name of the author may not be used to endorse or promote products
	  derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdio.h>
#include <errno.h>
#include <string.h>
#if defined(_MSC_VER)
#if _MSC_VER >= 1400
#define strcasecmp _stricmp
#else
#define strcasecmp stricmp
#endif
#else
#include <unistd.h>
#endif
#ifdef _WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include "natpmp.h"

/* List of IP address blocks which are private / reserved and therefore not suitable for public external IP addresses */
#define IP(a, b, c, d) (((a) << 24) + ((b) << 16) + ((c) << 8) + (d))
#define MSK(m) (32-(m))
static const struct { uint32_t address; uint32_t rmask; } reserved[] = {
	{ IP(  0,   0,   0, 0), MSK( 8) }, /* RFC1122 "This host on this network" */
	{ IP( 10,   0,   0, 0), MSK( 8) }, /* RFC1918 Private-Use */
	{ IP(100,  64,   0, 0), MSK(10) }, /* RFC6598 Shared Address Space */
	{ IP(127,   0,   0, 0), MSK( 8) }, /* RFC1122 Loopback */
	{ IP(169, 254,   0, 0), MSK(16) }, /* RFC3927 Link-Local */
	{ IP(172,  16,   0, 0), MSK(12) }, /* RFC1918 Private-Use */
	{ IP(192,   0,   0, 0), MSK(24) }, /* RFC6890 IETF Protocol Assignments */
	{ IP(192,   0,   2, 0), MSK(24) }, /* RFC5737 Documentation (TEST-NET-1) */
	{ IP(192,  31, 196, 0), MSK(24) }, /* RFC7535 AS112-v4 */
	{ IP(192,  52, 193, 0), MSK(24) }, /* RFC7450 AMT */
	{ IP(192,  88,  99, 0), MSK(24) }, /* RFC7526 6to4 Relay Anycast */
	{ IP(192, 168,   0, 0), MSK(16) }, /* RFC1918 Private-Use */
	{ IP(192, 175,  48, 0), MSK(24) }, /* RFC7534 Direct Delegation AS112 Service */
	{ IP(198,  18,   0, 0), MSK(15) }, /* RFC2544 Benchmarking */
	{ IP(198,  51, 100, 0), MSK(24) }, /* RFC5737 Documentation (TEST-NET-2) */
	{ IP(203,   0, 113, 0), MSK(24) }, /* RFC5737 Documentation (TEST-NET-3) */
	{ IP(224,   0,   0, 0), MSK( 4) }, /* RFC1112 Multicast */
	{ IP(240,   0,   0, 0), MSK( 4) }, /* RFC1112 Reserved for Future Use + RFC919 Limited Broadcast */
};
#undef IP
#undef MSK

static int addr_is_reserved(struct in_addr * addr)
{
	uint32_t address = ntohl(addr->s_addr);
	size_t i;

	for (i = 0; i < sizeof(reserved)/sizeof(reserved[0]); ++i) {
		if ((address >> reserved[i].rmask) == (reserved[i].address >> reserved[i].rmask))
			return 1;
	}

	return 0;
}

void usage(FILE * out, const char * argv0)
{
    fprintf(out, "Usage :\n"
        "  %s [options]\n"
        "\tdisplay the public IP address.\n"
        "  %s -h\n"
        "\tdisplay this help screen.\n"
        "  %s [options] -a <public port> <private port> <protocol> [lifetime]\n"
        "\tadd a port mapping.\n"
        "\nOption available :\n"
        "  -g ipv4address\n"
        "\tforce the gateway to be used as destination for NAT-PMP commands.\n"
        "\n  In order to remove a mapping, set it with a lifetime of 0 seconds.\n"
        "  To remove all mappings for your machine, use 0 as private port and lifetime.\n", argv0, argv0, argv0);
}

static void handlenatpmpbadreplytype(natpmp_t* pnatpmp, const natpmpresp_t* presponse, int* preturncode){
	char retry = (pnatpmp->try_number <= 9);
	printf("readnatpmpresponseorretry received unexpected reply type %hu , %s...\n", presponse->type, (retry == 1 ? "retrying" : "no more retry"));
	if (retry) {
		*preturncode = NATPMP_TRYAGAIN;
		pnatpmp->has_pending_request = 1;
	}
}

/* sample code for using libnatpmp */
int main(int argc, char * * argv)
{
	natpmp_t natpmp;
	natpmpresp_t response;
	int r;
	int sav_errno;
	struct timeval timeout;
	fd_set fds;
	int i;
	int protocol = 0;
	uint16_t privateport = 0;
	uint16_t publicport = 0;
	uint32_t lifetime = 3600;
	int command = 0;
	int forcegw = 0;
	in_addr_t gateway = 0;
	struct in_addr gateway_in_use;

#ifdef _WIN32
	WSADATA wsaData;
	int nResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(nResult != NO_ERROR)
	{
		fprintf(stderr, "WSAStartup() failed.\n");
		return -1;
	}
#endif

	/* argument parsing */
	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-') {
			switch(argv[i][1]) {
			case 'h':
				usage(stdout, argv[0]);
				return 0;
			case 'g':
				forcegw = 1;
				if(argc < i + 1) {
					fprintf(stderr, "Not enough arguments for option -%c\n", argv[i][1]);
					return 1;
				}
				gateway = inet_addr(argv[++i]);
				break;
			case 'a':
				command = 'a';
				if(argc < i + 4) {
					fprintf(stderr, "Not enough arguments for option -%c\n", argv[i][1]);
					return 1;
				}
				i++;
				if(1 != sscanf(argv[i], "%hu", &publicport)) {
					fprintf(stderr, "%s is not a correct 16bits unsigned integer\n", argv[i]);
					return 1;
				}
				i++;
				if(1 != sscanf(argv[i], "%hu", &privateport)) {
					fprintf(stderr, "%s is not a correct 16bits unsigned integer\n", argv[i]);
					return 1;
				}
				i++;
				if(0 == strcasecmp(argv[i], "tcp"))
					protocol = NATPMP_PROTOCOL_TCP;
				else if(0 == strcasecmp(argv[i], "udp"))
					protocol = NATPMP_PROTOCOL_UDP;
				else {
					fprintf(stderr, "%s is not a valid protocol\n", argv[i]);
					return 1;
				}
				if(argc > i + 1) {
					if(1 != sscanf(argv[i+1], "%u", &lifetime)) {
						fprintf(stderr, "%s is not a correct 32bits unsigned integer\n", argv[i]);
					} else {
						i++;
					}
				}
				break;
			default:
				fprintf(stderr, "Unknown option %s\n", argv[i]);
				usage(stderr, argv[0]);
				return 1;
			}
		} else {
			fprintf(stderr, "Unknown option %s\n", argv[i]);
			usage(stderr, argv[0]);
			return 1;
		}
	}

	/* initnatpmp() */
	r = initnatpmp(&natpmp, forcegw, gateway);
	printf("initnatpmp() returned %d (%s)\n", r, r?"FAILED":"SUCCESS");
	if(r<0)
		return 1;

	gateway_in_use.s_addr = natpmp.gateway;
	printf("using gateway : %s\n", inet_ntoa(gateway_in_use));

	/* sendpublicaddressrequest() */
	r = sendpublicaddressrequest(&natpmp);
	printf("sendpublicaddressrequest returned %d (%s)\n",
	       r, r==2?"SUCCESS":"FAILED");
	if(r<0)
		return 1;

	do {
		FD_ZERO(&fds);
		FD_SET(natpmp.s, &fds);
		getnatpmprequesttimeout(&natpmp, &timeout);
		r = select(FD_SETSIZE, &fds, NULL, NULL, &timeout);
		if(r<0) {
			fprintf(stderr, "select()");
			return 1;
		}
		r = readnatpmpresponseorretry(&natpmp, &response);
		sav_errno = errno;
		printf("readnatpmpresponseorretry returned %d (%s)\n",
		       r, r==0?"OK":(r==NATPMP_TRYAGAIN?"TRY AGAIN":"FAILED"));
		if(r<0 && r!=NATPMP_TRYAGAIN) {
#ifdef ENABLE_STRNATPMPERR
			fprintf(stderr, "readnatpmpresponseorretry() failed : %s\n",
			        strnatpmperr(r));
#endif
			fprintf(stderr, "  errno=%d '%s'\n",
			        sav_errno, strerror(sav_errno));
		}
		if (r >= 0 && response.type != 0) {
			handlenatpmpbadreplytype(&natpmp, &response, &r);
		}
	} while(r==NATPMP_TRYAGAIN);
	if(r<0)
		return 1;

	if(response.type!=NATPMP_RESPTYPE_PUBLICADDRESS) {
		fprintf(stderr, "readnatpmpresponseorretry() failed : invalid response type %u\n", response.type);
		return 1;
	}

	if(addr_is_reserved(&response.pnu.publicaddress.addr)) {
		fprintf(stderr, "readnatpmpresponseorretry() failed : invalid Public IP address %s\n", inet_ntoa(response.pnu.publicaddress.addr));
		return 1;
	}

	printf("Public IP address : %s\n", inet_ntoa(response.pnu.publicaddress.addr));
	printf("epoch = %u\n", response.epoch);

	if(command == 'a') {
		/* sendnewportmappingrequest() */
		r = sendnewportmappingrequest(&natpmp, protocol,
        	                      privateport, publicport,
								  lifetime);
		printf("sendnewportmappingrequest returned %d (%s)\n",
		       r, r==12?"SUCCESS":"FAILED");
		if(r < 0)
			return 1;

		do {
			FD_ZERO(&fds);
			FD_SET(natpmp.s, &fds);
			getnatpmprequesttimeout(&natpmp, &timeout);
			select(FD_SETSIZE, &fds, NULL, NULL, &timeout);
			r = readnatpmpresponseorretry(&natpmp, &response);
			printf("readnatpmpresponseorretry returned %d (%s)\n",
			       r, r==0?"OK":(r==NATPMP_TRYAGAIN?"TRY AGAIN":"FAILED"));
			if (r >= 0 && (
					protocol == NATPMP_PROTOCOL_TCP && response.type != NATPMP_RESPTYPE_TCPPORTMAPPING ||
					protocol == NATPMP_PROTOCOL_UDP && response.type != NATPMP_RESPTYPE_UDPPORTMAPPING)) {
				handlenatpmpbadreplytype(&natpmp, &response, &r);
			}
		} while(r==NATPMP_TRYAGAIN);
		if(r<0) {
#ifdef ENABLE_STRNATPMPERR
			fprintf(stderr, "readnatpmpresponseorretry() failed : %s\n",
			        strnatpmperr(r));
#endif
			return 1;
		}

		printf("Mapped public port %hu protocol %s to local port %hu "
		       "lifetime %u\n",
	    	   response.pnu.newportmapping.mappedpublicport,
			   response.type == NATPMP_RESPTYPE_UDPPORTMAPPING ? "UDP" :
			    (response.type == NATPMP_RESPTYPE_TCPPORTMAPPING ? "TCP" :
			     "UNKNOWN"),
			   response.pnu.newportmapping.privateport,
			   response.pnu.newportmapping.lifetime);
		printf("epoch = %u\n", response.epoch);
	}

	r = closenatpmp(&natpmp);
	printf("closenatpmp() returned %d (%s)\n", r, r==0?"SUCCESS":"FAILED");
	if(r<0)
		return 1;

	return 0;
}

