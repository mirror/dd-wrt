/* ----------------------------------------------------------------------------
    NSTX -- tunneling network-packets over DNS

     (C) 2000 by Florian Heinz and Julien Oster

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
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <pwd.h>
#include <sysexits.h>
#include <syslog.h>

#include "nstxfun.h"
#include "nstx_pstack.h"
#include "nstxdns.h"

#define DNSTIMEOUT 3

#define MAX(a,b) ((a>b)?a:b)

#define BUFLEN 2000

static void nstx_getpacket (void);
static struct nstx_senditem * alloc_senditem(void);
static void queue_senditem(const char *buf, int len);
static char *dequeue_senditem (int *len);
struct nstx_senditem * nstx_sendlist = NULL;

static void
usage(const char *prog, int code)
{
	fprintf (stderr, "usage: %s [options] <domainname>\n"
	    "Where options are:\n"
	    "\t-d tun-device (use this tun/tap device instead of default\n"
	    "\t-i ip.to.bi.nd (bind to port 53 on this IP only)\n"
	    "\t-C dir (chroot() to this directory after initialization)\n"
	    "\t-D (call daemon(3) to detach from terminal)\n"
	    "\t-g (enable debug messages)\n"
	    "\t-u user (switch to this user after initialization)\n"
	    "example:\n"
	    "\t%s -u nobody -C /var/empty tun.yomama.com\n", prog, prog);
	exit(code);
}

int main (int argc, char *argv[]) {
   signed char	 ch;
   const char	*device = NULL, *dir = NULL;
   in_addr_t	 bindto = INADDR_ANY;
   uid_t	 uid = 0;
   int		 daemonize = 0;
   int		 logmask = LOG_UPTO(LOG_INFO);
   
   while ((ch = getopt(argc, argv, "gDC:u:hd:i:")) != -1) {
	switch(ch) {
	case 'i':
		bindto = inet_addr(optarg);
		if (bindto == INADDR_NONE) {
			fprintf(stderr, "`%s' is not an IP-address\n",
			    optarg);
			exit(EX_USAGE);
		}
		break;
	case 'd':
		device = optarg;
		break;
	case 'D':
		daemonize = 1;
		break;
	case 'g':
		logmask = LOG_UPTO(LOG_DEBUG);
		break;
	case 'u':
		uid = atol(optarg); /* Is it numeric already? */
		if (uid == 0) {
			struct passwd *pwd;

			pwd = getpwnam(optarg);
			if (pwd == NULL) {
				perror(optarg);
				exit(EX_NOUSER);
			}
			uid = pwd->pw_uid;
		}
		break;
	case 'C':
		dir = optarg;
		break;
	case 'h':
		usage(argv[0], 0);	/* no return */
	default:
		usage(argv[0], EX_USAGE);	/* no return */
	}
   }

   if (argc - optind < 1)
	usage(argv[0], EX_USAGE);

   dns_setsuffix(argv[optind]);
   
   open_tuntap(device);
   open_ns_bind(bindto);
   
   if (dir) {
	/* Open the log-socket now (with LOG_NDELAY) before chroot-ing */
	openlog(argv[0], LOG_PERROR|LOG_PID|LOG_CONS|LOG_NDELAY, LOG_DAEMON);
	if (chroot(dir)) {
		syslog(LOG_ERR, "Can't chroot to %s: %m", dir);
		exit(EXIT_FAILURE); /* Too many possible causes */
	}
   } else
	openlog(argv[0], LOG_PERROR|LOG_PID|LOG_CONS, LOG_DAEMON);

   setlogmask(logmask);
	
   if (uid && setuid(uid)) {
	syslog(LOG_ERR, "Can't setuid to %ld: %m", (long)uid);
	exit(EX_NOPERM);
   }
   if (daemonize && daemon(0, 0)) {
	syslog(LOG_ERR, "Can't become a daemon: %m");
	exit(EX_OSERR);
   }
   while (1)
     nstx_getpacket();
   
   exit(0);
}

struct nstx_senditem * nstx_get_senditem(void) {
   struct nstx_senditem *ptr = nstx_sendlist;
   
   if (!nstx_sendlist)
     return NULL;
   
   ptr = nstx_sendlist;
   nstx_sendlist = nstx_sendlist->next;
   
   return ptr;
}

static void do_timeout (struct nstxqueue *q)
{
   struct dnspkt *pkt;
   int len;
   char *buf;
   
   pkt = dns_alloc();
   dns_setid(pkt, q->id);
   dns_settype(pkt, DNS_RESPONSE);
   dns_addanswer(pkt, "\xb4\x00\x00\x00", 4, dns_addquery(pkt, q->name));
   buf = (char*)dns_constructpacket (pkt, &len);
   sendns(buf, len, &q->peer);
   free(buf);
}  

void nstx_getpacket (void) {
   int len, link;
   const char *name, *buf, *data;
   struct nstxmsg *msg;
   struct nstxqueue *qitem;
   struct dnspkt *pkt;

   msg = nstx_select(1);
   
   if (msg) {
     if (msg->src == FROMNS) {
	pkt = dns_extractpkt((unsigned char*)msg->data, msg->len);
	if (pkt)
	  {
	     name = dns_getquerydata(pkt);
	     if (name)
	       {
		  syslog(LOG_DEBUG, "getpacket: asked for name `%s'",
			name);
		  queueitem(pkt->id, name, &msg->peer);
		  if ((data = dns_fqdn2data(name)) &&
		      (buf = nstx_decode((unsigned char*)data, &len)))
		    {
		       nstx_handlepacket(buf, len, &sendtun);
		    }
	       }
	     dns_free(pkt);
	  }
     } else if (msg->src == FROMTUN)
	  queue_senditem(msg->data, msg->len);
   }
   
   while (queuelen()) {
      if (!nstx_sendlist)
	break;
      qitem = dequeueitem(-1);
      pkt = dns_alloc();
      dns_setid(pkt, qitem->id);
      dns_settype(pkt, DNS_RESPONSE);
      link = dns_addquery(pkt, qitem->name);
      len = dns_getfreespace(pkt, DNS_RESPONSE);
      buf = dequeue_senditem(&len);
      dns_addanswer(pkt, buf, len, link);
      buf = (char*)dns_constructpacket(pkt, &len);
      sendns(buf, len, &qitem->peer);
   }
   timeoutqueue(do_timeout);
}



static struct nstx_senditem * alloc_senditem(void) {
   struct nstx_senditem *ptr = nstx_sendlist;

   if (!nstx_sendlist) {
      ptr = nstx_sendlist = malloc(sizeof(struct nstx_senditem));
   } else {
      while (ptr->next)
	ptr = ptr->next;
      ptr->next = malloc(sizeof(struct nstx_senditem));
      ptr = ptr->next;
   }

   memset(ptr, 0, sizeof(struct nstx_senditem));
   
   return ptr;
}

static void
queue_senditem(const char *buf, int len) {
   static int id = 0;
   struct nstx_senditem *item;
   
   item = alloc_senditem();
   item->data = malloc(len);
   memcpy(item->data, buf, len);
   item->len = len;
   item->id = ++id;
}

static char *
dequeue_senditem (int *len) {
   static char *buf;
   struct nstx_senditem *item = nstx_sendlist;
   struct nstxhdr *nh;
   int remain, dlen;
   
   remain = item->len - item->offset;
   dlen = *len - sizeof(struct nstxhdr);
   if (dlen > remain)
     dlen = remain;
   *len = dlen + sizeof(struct nstxhdr);
   buf = realloc(buf, *len);
   nh = (struct nstxhdr *)buf;
   memset(nh, 0, sizeof(struct nstxhdr));
   memcpy(buf+sizeof(struct nstxhdr), item->data + item->offset, dlen);
   nh->magic = NSTX_MAGIC;
   nh->seq = item->seq++;
   nh->id = item->id;
   item->offset += dlen;
   if (item->offset == item->len) {
      nh->flags = NSTX_LF;
      nstx_sendlist = item->next;
      free(item->data);
      free(item);
   }
   
   return buf;
}
