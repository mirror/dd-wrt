/***************************************************************************
 projekt              : olsrd-quagga
 file                 : quagga.c  
 usage                : communication with the zebra-daemon 
 copyright            : (C) 2006 by Immo 'FaUl' Wehrenberg
 e-mail               : immo@chaostreff-dortmund.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 ***************************************************************************/


#ifdef MY_DEBUG
#include <stdio.h>
#endif

#define HAVE_SOCKLEN_T

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <quagga/zebra.h>
#include "quagga.h"

#include "olsr.h"
#include "log.h"
#include "defs.h"
#include "local_hna_set.h"

#ifdef USE_UNIX_DOMAIN_SOCKET
#include <sys/un.h>
#define ZEBRA_SOCKET "/var/run/quagga/zserv.api"
#endif

#define ZAPI_MESSAGE_NEXTHOP  0x01
#define ZAPI_MESSAGE_IFINDEX  0x02
#define ZAPI_MESSAGE_DISTANCE 0x04
#define ZAPI_MESSAGE_METRIC   0x08

#define BUFSIZE 1024

#define STATUS_CONNECTED 1

static struct {
  char status; // TODO: internal status
  int sock; // Socket to zebra...
  char redistribute[ZEBRA_ROUTE_MAX];
  char distance;
  char flags;
  struct ipv4_route *v4_rt; // routes currently exportet to zebra
} zebra;


/* prototypes intern */
static char *try_read (ssize_t *);
static char* zebra_route_packet (struct ipv4_route r, ssize_t *);
static int parse_interface_add (char *, size_t);
static int parse_interface_delete (char *, size_t);
static int parse_interface_up (char *, size_t);
static int parse_interface_down (char *, size_t);
static int parse_interface_address_add (char *, size_t);
static int parse_interface_address_delete (char *, size_t);
static int parse_ipv4_route (char *, size_t, struct ipv4_route *);
static int ipv4_route_add (char *, size_t);
static int ipv4_route_delete (char *, size_t);
static int parse_ipv6_route_add (char*, size_t);
static int zebra_reconnect (void);
static int zebra_connect (void);
static int add_v4_route_status (struct ipv4_route r);
static int del_v4_route_status (struct ipv4_route r);
static uint32_t prefixlentomask (uint8_t);
static void free_ipv4_route (struct ipv4_route);
static void update_olsr_zebra_routes (struct ipv4_route*, struct ipv4_route*);
static struct ipv4_route *zebra_create_ipv4_route_table_entry (uint32_t,
							       uint32_t,
							       uint32_t);
static struct ipv4_route *zebra_create_ipv4_route_table (void);
static void zebra_free_ipv4_route_table (struct ipv4_route*);
static uint8_t masktoprefixlen (uint32_t);



#ifdef MY_DEBUG
static void dump_ipv4_route (struct ipv4_route r, char *c) {
  int i = 0, x = 0;

  puts (c);
  printf("type: %d\n", r.type);
  puts("flags:");
  printf("  Internal: %s\n",r.flags&ZEBRA_FLAG_INTERNAL?"yes":"no");
  printf("  Selfroute %s\n",r.flags&ZEBRA_FLAG_SELFROUTE?"yes":"no");
  printf("  Blackhole %s\n",r.flags&ZEBRA_FLAG_BLACKHOLE?"yes":"no");
  printf("  IBGP: %s\n",r.flags&ZEBRA_FLAG_IBGP?"yes":"no");
  printf("  Selected: %s\n",r.flags&ZEBRA_FLAG_SELECTED?"yes":"no");
  printf("  Changed: %s\n",r.flags&ZEBRA_FLAG_CHANGED?"yes":"no");
  printf("  static: %s\n",r.flags&ZEBRA_FLAG_STATIC?"yes":"no");
  printf("  reject: %s\n",r.flags&ZEBRA_FLAG_REJECT?"yes":"no");
  puts("message:");
  printf("  nexthop: %s\n",r.message&ZAPI_MESSAGE_NEXTHOP?"yes":"no");
  printf("  ifindex: %s\n",r.message&ZAPI_MESSAGE_IFINDEX?"yes":"no");
  printf("  distance: %s\n",r.message&ZAPI_MESSAGE_DISTANCE?"yes":"no");
  printf("  metric: %s\n",r.message&ZAPI_MESSAGE_METRIC?"yes":"no");
  printf("Prefixlen: %d\n", r.prefixlen);
  printf("Prefix: %d", (unsigned char)r.prefix);
  c = (char*) &r.prefix;
  while (++i < (r.prefixlen/8 + (r.prefixlen % 8 ? 1 : 0)))
    printf(".%d",(unsigned char)*(c + i));
  while (i++ < 4)
    printf(".0");
  puts("");
  i=0;
  if (r.message&ZAPI_MESSAGE_NEXTHOP) {
    printf("nexthop-count: %d\n", r.nh_count);
    while (i++ < r.nh_count) {
      if (r.nexthops[i].type == ZEBRA_NEXTHOP_IPV4) {
	c = (unsigned char*) &r.nexthops[i].payload.v4;
	printf ("Nexthop %d: %d", i, (unsigned char) *c);
	while (++x < 4) {
	  printf (".%d", (unsigned char) c[x]);
	}
	puts("");
      }
    }
    i=0;
  }
  if (r.message&ZAPI_MESSAGE_IFINDEX) {
    
    printf("index-number: %d\n", r.ind_num);
    while (i++ < r.ind_num)
      printf("Index: %d: %d\n", i, r.index[i]);
    i=0;
    if (r.message&ZAPI_MESSAGE_DISTANCE)
      printf("Distance: %d\n",r.distance);
    if (r.message&ZAPI_MESSAGE_METRIC)
      printf("Metric: %d\n",r.metric);
    puts("\n");
  }
}
#endif

void *my_realloc (void *buf, size_t s, const char *c) {
  buf = realloc (buf, s);
  if (!buf) {
    OLSR_PRINTF (1, "OUT OF MEMORY: %s\n", strerror(errno));
    olsr_syslog(OLSR_LOG_ERR, "olsrd: out of memory!: %m\n");
    olsr_exit(c, EXIT_FAILURE);
  }
  return buf;
}

int init_zebra () {
  if (zebra_connect() < 0 || !(zebra.status&STATUS_CONNECTED)) {
    olsr_exit ("AIIIII, could not connect to zebra! is zebra running?", 
	       EXIT_FAILURE);
  }
}


static int zebra_reconnect (void) {
  struct ipv4_route *tmp;
  int i;

  if (zebra_connect() < 0 || !(zebra.status&STATUS_CONNECTED)) 
    // log: zebra-reconnect failed
    ;
  for (i = 0; ZEBRA_ROUTE_MAX - 1; i++)
    if (zebra.redistribute[i]) zebra_redistribute(i + 1);

  for (tmp = zebra.v4_rt; tmp; tmp = tmp->next)
    zebra_add_v4_route(*tmp);
}


static int add_v4_route_status (struct ipv4_route r) {

  struct ipv4_route *tmp = olsr_malloc (sizeof r, "quagga_v4_route_status");
  memcpy (tmp, &r, sizeof r);

  if (r.message & ZAPI_MESSAGE_NEXTHOP) {
    tmp->nexthops = olsr_malloc (r.nh_count * sizeof tmp->nexthops, 
				 "quagga_v4_route_status");
    memcpy (tmp->nexthops, &r.nexthops, sizeof *r.nexthops);
  }

  if (r.message & ZAPI_MESSAGE_IFINDEX) {
    tmp->index = olsr_malloc (r.ind_num * sizeof *tmp->index, 
			      "quagga_v4_route_status");
    memcpy (tmp->index, &r.index, r.ind_num * sizeof *tmp->index);
  }

  tmp->next = zebra.v4_rt;
  zebra.v4_rt = tmp;

  return 0;

}


static int cmp_v4_route (struct ipv4_route a, struct ipv4_route b) {
  if (a.type != b.type) return 1;
  if (a.flags != b.flags) return 1;
  if (a.message != b.message) return 1;
  if (a.prefixlen != b.prefixlen) return 1;
  if (a.message & ZAPI_MESSAGE_NEXTHOP) {
    if (a.nh_count != b.nh_count) return 1;
    if (memcmp (a.nexthops, b.nexthops, a.nh_count * sizeof *b.nexthops)) 
      return 1;
  }
  if (a.message & ZAPI_MESSAGE_IFINDEX) {
    if (a.ind_num != b.ind_num) return 1;
    if (memcpy (a.index, b.index, a.ind_num * sizeof *a.index)) return 1;
  }
  if (a.message & ZAPI_MESSAGE_DISTANCE) 
    if (a.distance != b.distance) return 1;
  if (a.message & ZAPI_MESSAGE_METRIC)
    if (a.metric != b.metric) return 1;
  return 0;
}

static int del_v4_route_status (struct ipv4_route r) {

  struct ipv4_route *tmp, *prv = 0;

  for (tmp = zebra.v4_rt; tmp; tmp = tmp->next) {
    if (!cmp_v4_route(*tmp, r)) {
      if (prv) prv->next = tmp->next;

      free_ipv4_route(*tmp);
      free (tmp);

      return 0;

    }
    prv = tmp;
  }

  return 1;
}


/* Connect to the zebra-daemon, returns a socket */
static int zebra_connect (void) {

#ifndef USE_UNIX_DOMAIN_SOCKET
  struct sockaddr_in i;
  close (zebra.sock);

  zebra.sock = socket (AF_INET,SOCK_STREAM, 0);
#else
  struct sockaddr_un i;
  close (zebra.sock);

  zebra.sock = socket (AF_UNIX,SOCK_STREAM, 0);
#endif

  int ret;

  if (zebra.sock <0 )
    olsr_exit("could not create socket!", EXIT_FAILURE);
  
  memset (&i, 0, sizeof i);
#ifndef USE_UNIX_DOMAIN_SOCKET
  i.sin_family = AF_INET;
  i.sin_port = htons (ZEBRA_PORT);
  i.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
#else
  i.sun_family = AF_UNIX;
  strcpy (i.sun_path, ZEBRA_SOCKET);
#endif

  ret = connect (zebra.sock, (struct sockaddr *)&i, sizeof i);
  if  (ret < 0) {
    close (zebra.sock);
  }
  else zebra.status |= STATUS_CONNECTED;
  return zebra.sock;
}

    
/* Sends a command to zebra, command is 
   the command defined in zebra.h, options is the packet-payload, 
   optlen the length, of the payload */
char zebra_send_command (unsigned char command, char * options, int optlen) {

#ifdef ZEBRA_HEADER_MARKER
  char *p = olsr_malloc (optlen + 6, "zebra_send_command");
  uint16_t length = optlen + 6; /* length of option + command + packet_length +
				   marker + zserv-version */
  uint16_t cmd;
#else
  char *p = olsr_malloc (optlen + 3, "zebra_send_command");
  uint16_t length = optlen + 3;  // length of option + command + packet_length
#endif
  
  int ret;
  
  uint16_t len = htons(length);
  memcpy (p, &len, 2);

#ifdef ZEBRA_HEADER_MARKER
  p[2] = ZEBRA_HEADER_MARKER;
  p[3] = ZSERV_VERSION;
  cmd = htons (command);
  memcpy (p + 4, &cmd, 2);
  memcpy (p + 6, options, optlen);
#else
  p[2] = command;
  memcpy (p + 3, options, optlen);
#endif

  errno = 0;

  do {
    ret = write (zebra.sock, p, length);
    if (ret < 0) {
      if (errno == EINTR) {
	errno = 0;
	continue;
      }
      else {
	zebra.status &= ~STATUS_CONNECTED;
	return -1;
      }
    }
    p = p+ret;
  } while ((length -= ret));

  return 0;
}


/* Creates a Route-Packet-Payload, needs address, netmask, nexthop, 
   distance, and a pointer of an size_t */
static char* zebra_route_packet (struct ipv4_route r, ssize_t *optlen) {

  int count;

  char *cmdopt, *t;
  *optlen = 4; // first: type, flags, message, prefixlen
  *optlen += r.prefixlen / 8 + (r.prefixlen % 8 ? 1 : 0); // + prefix
  if (r.message & ZAPI_MESSAGE_NEXTHOP)
    if (r.nexthops->type == ZEBRA_NEXTHOP_IPV4 
	|| r.nexthops->type == ZEBRA_NEXTHOP_IPV4_IFINDEX){
      *optlen += (sizeof r.nexthops->payload.v4 
		  + sizeof r.nexthops->type) * r.nh_count + 1;
    }
    else if (r.nexthops->type == 0) 
      *optlen += 5;
  if (r.message & ZAPI_MESSAGE_IFINDEX)
    *optlen += r.ind_num * sizeof *r.index + 1;
  if (r.message & ZAPI_MESSAGE_DISTANCE)
    (*optlen)++;
  if (r.message & ZAPI_MESSAGE_METRIC)
    *optlen += sizeof r.metric;

  cmdopt = olsr_malloc (*optlen, "zebra add_v4_route");

  t = cmdopt;
  *t++ = r.type; 
  *t++ = r.flags; 
  *t++ = r.message;
  *t++ = r.prefixlen;
  for (count = 0; count < r.prefixlen/8 + (r.prefixlen % 8 ? 1 : 0); count++) {
    *t++ = *((char*)&r.prefix + count); /* this is so sick!! */
  }

  if (r.message & ZAPI_MESSAGE_NEXTHOP) {
    *t++ = r.nh_count;
    *t++ = r.nexthops->type;
    if (r.nexthops->type == ZEBRA_NEXTHOP_IPV4 || 
	r.nexthops->type == ZEBRA_NEXTHOP_IPV4_IFINDEX) {
      for (count = 0; count != r.nh_count; count++) {
	memcpy (t, &r.nexthops[count].payload.v4, 
		sizeof r.nexthops->payload.v4);
	t += sizeof r.nexthops->payload.v4;
      }
    }
    else if (r.nexthops->type == 0) {
      *t++ = 0;
      *t++ = 0;
      *t++ = 0;
    }
  }
  if (r.message & ZAPI_MESSAGE_IFINDEX) {
    *t++ = r.ind_num;
    memcpy (t, r.index, sizeof *r.index * r.ind_num);
    t += sizeof r.index * r.ind_num;
  }
  if (r.message & ZAPI_MESSAGE_DISTANCE)
    *t++ = r.distance;
  if (r.message & ZAPI_MESSAGE_METRIC) {
    memcpy (t, &r.metric, sizeof r.metric);
    t += sizeof r.metric;
  }
  return cmdopt;
}


/* adds a route to zebra-daemon */
int zebra_add_v4_route (struct ipv4_route r) {
  
  char *cmdopt;
  ssize_t optlen;
  int retval;

  cmdopt = zebra_route_packet (r, &optlen);

  retval = zebra_send_command (ZEBRA_IPV4_ROUTE_ADD, cmdopt, optlen);
  free (cmdopt);
  return retval;
  
}

/* deletes a route from the zebra-daemon */
int zebra_delete_v4_route (struct ipv4_route r) {
  
  char *cmdopt;
  ssize_t optlen;
  int retval;

  cmdopt = zebra_route_packet (r, &optlen);
  
  retval = zebra_send_command (ZEBRA_IPV4_ROUTE_DELETE, cmdopt, optlen);
  free (cmdopt);

  return retval;
  
}


/* Check wether there is data from zebra aviable */
void zebra_check (void* foo) {
  char *data, *f;
  ssize_t len, ret;

  if (!(zebra.status & STATUS_CONNECTED)) {
    if (!zebra_reconnect()) return;
  }
  data = try_read (&len);
  if (data) {
    f = data;
    do {
      ret = zebra_parse_packet (f, len);
      if (!ret) {//something wired happened
	puts ("DEBUG: IIIIIIIIIIRGS");
	exit (EXIT_FAILURE);
      }
      f += ret;
    } while ((f - data) < len);
    free (data);
  }
}


// tries to read a packet from zebra_socket
// if there is something to read - make sure to read whole packages
static char *try_read (ssize_t *len) {
  char *buf = NULL;
  ssize_t ret = 0, bsize = 0;
  uint16_t length = 0, l = 0;
  int sockstate;

  *len = 0;

  sockstate = fcntl (zebra.sock, F_GETFL, 0);
  fcntl (zebra.sock, F_SETFL, sockstate|O_NONBLOCK);

  do { 
    if (*len == bsize) {
      bsize += BUFSIZE;
      buf = my_realloc (buf, bsize, "Zebra try_read");
    }
    ret = read (zebra.sock, buf + l, bsize - l);
    if (ret <= 0) {
      if (errno == EAGAIN) {
	errno = 0;
      }
      else {
	olsr_printf(1, "OOPS, something realy wired happened:"
		    "read returned %s\n", strerror(errno));
	errno = 0;
	zebra.status &= ~STATUS_CONNECTED;
	return 0;
      }
      free (buf);
      return NULL;
    }
    *len += ret;

    while ((*len - l) > length) {
      l += length;
      memcpy (&length, buf + l, 2);
      length = ntohs (length);
    }
    if (((*len) - l) == length) break; // GOT FULL PACKAGE!!
    if (*len < l) {
      fcntl (zebra.sock, F_SETFL, sockstate);
      continue;
    }
  } while (1);

  fcntl (zebra.sock, F_SETFL, sockstate);
  return buf;
}


/* Parse a packet recived from zebra */
int zebra_parse_packet (char *packet, ssize_t maxlen) {

  /* Array of functions */
  int (*foo[ZEBRA_MESSAGE_MAX]) (char *, size_t) = {
    parse_interface_add,
    parse_interface_delete,
    parse_interface_address_add,
    parse_interface_address_delete,
    parse_interface_up,
    parse_interface_down,
    ipv4_route_add,
    ipv4_route_delete,
    parse_ipv6_route_add
  };

#ifdef MY_DEBUG
  puts ("DEBUG: zebra_parse_packet");
#endif
  uint16_t length;
  
  int ret;
  memcpy (&length, packet, 2);
  length = ntohs (length);
  
  if (maxlen < length) {
    puts("Error: programmer is an idiot");
    printf ("DEBUG: maxlen = %d, packet_length = %d\n", maxlen, length);
    return maxlen;
  }

  if (packet[2] - 1 < ZEBRA_MESSAGE_MAX && foo[packet[2] - 1]) { 
    if (!(ret = foo[packet[2] - 1] (packet + 3, length - 3))) 
      return length;
    else printf ("DEBUG: Parse error: %d\n", ret);
  }
  else
    printf ("Unknown packet type: %d\n", packet[2]);

  puts ("Quagga: RECIVED PACKET FROM ZEBRA THAT I CAN'T PARSE");

  return length;
}


static int parse_interface_add (char *opt, size_t len) {
  //todo
  return 0;
}


static int parse_interface_delete (char *opt, size_t len) {
  //todo
  return 0;
}


static int parse_interface_address_add (char *opt, size_t len) {
  
  //todo
  return 0;
}

static int parse_interface_up (char *opt, size_t len) {
  
  //todo
  return 0;
}

static int parse_interface_down (char *opt, size_t len) {
  
  //todo
  return 0;
}


static int parse_interface_address_delete (char *opt, size_t  len) {
  //todo
  return 0;
}


/* Parse an ipv4-route-packet recived from zebra
 */
static int parse_ipv4_route (char *opt, size_t len, struct ipv4_route *r) {
  int c;

  if (len < 4) return -1;
  
  r->type = *opt++;
  r->flags = *opt++;
  r->message = *opt++;
  r->prefixlen = *opt++;
  len -= 4;
  r->prefix = 0;
  
  if ((int)len < r->prefixlen/8 + (r->prefixlen % 8 ? 1 : 0)) return -1;
  
  memcpy (&r->prefix, opt, r->prefixlen/8 + (r->prefixlen % 8 ? 1 : 0));
  opt += r->prefixlen/8 + (r->prefixlen % 8 ? 1 : 0);
  
  if (r->message & ZAPI_MESSAGE_NEXTHOP) {
    if (len < 1) return -1;
    r->nh_count = *opt++;
    len--;
    if (len < (sizeof (uint32_t) + 1) * r->nh_count) return -1;
    r->nexthops = olsr_malloc ((sizeof r->nexthops->type +
				sizeof r->nexthops->payload) * r->nh_count,
			       "quagga: parse_ipv4_route_add");
    for (c = 0; c < r->nh_count; c++) {
      r->nexthops[c].type = *opt++;
      memcpy (&r->nexthops[c].payload.v4, opt, sizeof (uint32_t));
      opt += sizeof (uint32_t);
      len -= sizeof (uint32_t) + 1;
    }
  }

  if (r->message & ZAPI_MESSAGE_IFINDEX) {
    if (len < 1) return -1;
    r->ind_num = *opt++;
    if (len < sizeof (uint32_t) * r->ind_num) return -1;
    r->index = olsr_malloc (sizeof (uint32_t) * r->ind_num,
			    "quagga: parse_ipv4_route_add");
    memcpy (r->index, opt, r->ind_num * sizeof (uint32_t));
    opt += sizeof (uint32_t) * r->ind_num;
    len -= sizeof (uint32_t) * r->ind_num;
  }

  if (r->message & ZAPI_MESSAGE_DISTANCE) {
    if (len < 1) return -1;
    r->distance = *opt++;
    len--;
  }

  if (r->message & ZAPI_MESSAGE_METRIC) {
    if (len < sizeof (uint32_t)) return -1;
    memcpy (&r->metric, opt, sizeof (uint32_t));
  }

  return 0;
}


static int ipv4_route_add (char *opt, size_t len) {

  struct ipv4_route r;
  int f;
    
  f = parse_ipv4_route (opt, len, &r);
  if (f < 0) {
    printf ("parse-error: %d\n",f);
    return f;
  }
  
  add_hna4_route (r);
  return 0;
}

static int ipv4_route_delete (char *opt, size_t len) {
  struct ipv4_route r;
  int f;
  
  f = parse_ipv4_route (opt, len, &r);
  if (f < 0) return f;

  return delete_hna4_route (r);
  
}

static int parse_ipv6_route_add (char *opt, size_t len) {
  //todo
  return 0;
}


/* start redistribution FROM zebra */
int zebra_redistribute (unsigned char type) {

  if (type > ZEBRA_ROUTE_MAX) return -1;
  zebra.redistribute[type - 1] = 1;

  return zebra_send_command (ZEBRA_REDISTRIBUTE_ADD, &type, 1);
  
  
}  


/* end redistribution FROM zebra */
int zebra_disable_redistribute (unsigned char type) {
  
  if (type > ZEBRA_ROUTE_MAX) return -1;
  zebra.redistribute[type - 1] = 0;

  return zebra_send_command (ZEBRA_REDISTRIBUTE_DELETE, &type, 1);

}
  
static uint32_t prefixlentomask (uint8_t prefix) {
  uint32_t mask = 0;

  if (prefix) {
    mask = 0xffffffff<<(32-prefix);
    mask = ntohl(mask);
  }

  return mask;
}

int add_hna4_route (struct ipv4_route r) {
  union olsr_ip_addr net, mask;
  
#ifdef MY_DEBUG
  dump_ipv4_route(r, "add_hna4_route");
#endif

  mask.v4 = prefixlentomask(r.prefixlen);
  net.v4 = r.prefix;

  add_local_hna4_entry(&net, &mask);
  free_ipv4_route(r);
  return 0;
}

int delete_hna4_route (struct ipv4_route r) {

  union olsr_ip_addr net, mask;

#ifdef MY_DEBUG
  dump_ipv4_route(r, "delete_hna4_route");
#endif

  mask.v4 = prefixlentomask(r.prefixlen);
  net.v4 = r.prefix;

  remove_local_hna4_entry(&net, &mask) ? 0 : -1;
  free_ipv4_route(r);
  return 0;

}

static void free_ipv4_route (struct ipv4_route r) {

  if(r.message&ZAPI_MESSAGE_IFINDEX && r.ind_num) free(r.index);
  if(r.message&ZAPI_MESSAGE_NEXTHOP && r.nh_count) free(r.nexthops);

}

static uint8_t masktoprefixlen (uint32_t mask) {
  
  uint8_t prefixlen = 0;

  mask = htonl (mask);

  if (mask) while (mask << ++prefixlen && prefixlen < 32);

  return prefixlen;
  
}

int zebra_add_olsr_v4_route (struct rt_entry *r) {
  
  struct ipv4_route route;
  int retval;
  
  route.type = ZEBRA_ROUTE_OLSR; // OLSR
  route.message = ZAPI_MESSAGE_METRIC;
  route.flags = zebra.flags;
  route.prefixlen = masktoprefixlen (r->rt_mask.v4);
  route.prefix = r->rt_dst.v4;
  if ((r->rt_router.v4 == r->rt_dst.v4 && route.prefixlen == 32)){
    route.message |= ZAPI_MESSAGE_IFINDEX | ZAPI_MESSAGE_NEXTHOP;
    route.ind_num = 1;
    route.index = olsr_malloc (sizeof *route.index, 
			       "zebra_add_olsr_v4_route");
    *route.index = htonl(r->rt_if->if_index);
    route.nexthops = olsr_malloc (sizeof route.nexthops->type +
				  sizeof route.nexthops->payload,
				  "zebra_add_olsr_v4_route");
    route.nh_count = 1;
    route.nexthops->type = 0;
  }
  else {
    route.message |= ZAPI_MESSAGE_NEXTHOP;
    route.nh_count = 1;
    route.nexthops = olsr_malloc (route.nh_count * 
				  (sizeof route.nexthops->type + 
				   sizeof route.nexthops->payload), 
				   "zebra_add_olsr_v4_route");
    route.nexthops->type = ZEBRA_NEXTHOP_IPV4;
    route.nexthops->payload.v4 = r->rt_router.v4;
  }

  route.metric = r->rt_metric;
  route.metric = htonl(route.metric);

  if (zebra.distance) {
    route.message |= ZAPI_MESSAGE_DISTANCE;
    route.distance = zebra.distance;
  }

  add_v4_route_status (route);
  retval = zebra_add_v4_route(route);
  free_ipv4_route (route);
  return retval;
}

int zebra_del_olsr_v4_route (struct rt_entry *r) {
  
  struct ipv4_route route;
  int retval;
  route.type = ZEBRA_ROUTE_OLSR; // OLSR
  route.message = ZAPI_MESSAGE_METRIC;
  route.flags = zebra.flags;
  route.prefixlen = masktoprefixlen (r->rt_mask.v4);
  route.prefix = r->rt_dst.v4;
  if ((r->rt_router.v4 == r->rt_dst.v4 && route.prefixlen == 32)){
    route.message |= ZAPI_MESSAGE_IFINDEX;
    route.ind_num = 1;
    route.index = olsr_malloc (sizeof *route.index, 
			       "zebra_add_olsr_v4_route");
    *route.index = htonl (r->rt_if->if_index);
    route.nexthops = olsr_malloc (sizeof route.nexthops->type +
				  sizeof route.nexthops->payload,
				  "zebra_add_olsr_v4_route");
    route.nh_count = 1;
    route.nexthops->type = 0;
  }
  else {
    route.message |= ZAPI_MESSAGE_NEXTHOP;
    route.nh_count = 1;
    route.nexthops = olsr_malloc (route.nh_count * 
				  (sizeof route.nexthops->type +
				   sizeof route.nexthops->payload), 
				  "zebra_add_olsr_v4_route");
    route.nexthops->type = ZEBRA_NEXTHOP_IPV4;
    route.nexthops->payload.v4 = r->rt_router.v4;
  }
  route.metric = r->rt_metric;
  route.metric = htonl (route.metric);
  
  if (zebra.distance) {
    route.message |= ZAPI_MESSAGE_DISTANCE;
    route.distance = zebra.distance;
  }

  retval = zebra_delete_v4_route(route);
  del_v4_route_status(route);
  free_ipv4_route (route);
  return retval;
}

void zebra_olsr_distance (char dist) {
  zebra.distance = dist;
}

void zebra_olsr_localpref (void) {
  zebra.flags &= ZEBRA_FLAG_SELECTED;
}
