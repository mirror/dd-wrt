/* ----------------------------------------------------------------------------
    NSTX -- tunneling network-packets over DNS

     (C) 2000 by Julien Oster and Florian Heinz

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  -------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>

#include <resolv.h>
#include <netdb.h>

#include <time.h>
#include <sysexits.h>

#include "nstxfun.h"
#include "nstx_dns.h"
#include "nstxdns.h"
#include "nstx_pstack.h"

#define DRQLEN 10

static void nstxc_handle_reply(char *, int);
static int nstxc_send_packet(char *, int);

static int nsid;
int gotpacket = 0;

static void
usage(const char *prog, int code)
{
	fprintf(stderr, "Usage: %s [-d tun-device] <domainname> <dns-server>\n"
	    "Example: %s tun.yomama.com 125.23.53.12\n", prog, prog);
	exit(code);
}

int main (int argc, char * argv[]) {
  struct nstxmsg *msg;
  const char	*device = NULL;
  int 		 ch;

  nsid = time(NULL);
 
  if (argc < 3)
	usage(argv[0], EX_USAGE);

  while ((ch = getopt(argc, argv, "hd:")) != -1) {
	switch (ch) {
	case 'd':
		device = optarg;
		break;
	case 'h':
		usage(argv[0], 0);
	default:
		usage(argv[0], EX_USAGE);
	}
  }

  dns_setsuffix(argv[optind]);

  qsettimeout(10);
  open_tuntap(device);
  open_ns(argv[optind + 1]);

  for (;;) {
    msg = nstx_select(1);
    if (msg) {
       if (msg->src == FROMNS) {
	  nstxc_handle_reply (msg->data, msg->len);
       } else if (msg->src == FROMTUN) {
	  nstxc_send_packet (msg->data, msg->len);
       }
    }
    timeoutqueue(NULL);
    while (queuelen() < DRQLEN)
      nstxc_send_packet (NULL, 0);
  }

  return 0;
}

static void nstxc_handle_reply (char * reply, int len) {
   struct dnspkt *pkt;
   const char *data;
   int datalen;
   
   pkt = dns_extractpkt ((unsigned char*)reply, len);
   if (!pkt)
     return;
   while ((data = dns_getanswerdata(pkt, &datalen))) {
      data = (char*)txt2data((unsigned char*)data, &datalen);
      nstx_handlepacket (data, datalen, &sendtun);
   }
   dequeueitem(pkt->id);
   dns_free(pkt);
}
  
static int nstxc_send_packet (char *data, int datalen) {
  static int id = -1;

  char *p;
  struct nstxhdr nh;
  struct dnspkt *pkt;
  int l;

  if (id < 0)
    id = time(NULL);
        
  nh.magic = NSTX_MAGIC;
  nh.seq = 0;
  nh.id = id++;
  nh.flags = 0;

  do {
    pkt = dns_alloc();
    dns_settype(pkt, DNS_QUERY);
    dns_setid(pkt, nsid);
    
    l = dns_getfreespace(pkt, DNS_QUERY);
    if (l <= 0) {
       printf("Fatal: no free space in dns-packet?!\n");
       exit(1);
    }
    p = malloc(l);
    l -= sizeof(nh);
    if (l > datalen) {
       l = datalen;
       nh.flags = NSTX_LF;
    }
    memcpy (p, (char*)&nh, sizeof(nh));
    if (data)
       memcpy (p + sizeof(nh), data, l);
    data += l;
    datalen -= l;
    
    dns_addquery(pkt, dns_data2fqdn(nstx_encode((unsigned char*)p, sizeof(nh)+l)));
    free(p);
    p = (char*)dns_constructpacket(pkt, &l);
    sendns(p, l, NULL);
    free(p);

    queueid(nsid);
    nsid++;
    nh.seq++;
  } while (datalen);

  return 0;
}
