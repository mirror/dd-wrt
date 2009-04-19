
/*
 *  (C) 2006 by Immo 'FaUl' Wehrenberg <immo@chaostreff-dortmund.de>
 *
 *  This code is covered by the GPLv2
 *
 */

#include <stdint.h>
#ifdef MY_DEBUG
#include <stdio.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define HAVE_SOCKLEN_T
#include <quagga/zebra.h>
#include "quagga.h"

#ifdef OLSR_PLUGIN
#include "olsr.h"
#include "log.h"
#include "defs.h"
#include "local_hna_set.h"
#endif

#define ZAPI_MESSAGE_NEXTHOP  0x01
#define ZAPI_MESSAGE_IFINDEX  0x02
#define ZAPI_MESSAGE_DISTANCE 0x04
#define ZAPI_MESSAGE_METRIC   0x08

#define STATUS_CONNECTED 1
#define BUFSIZE 1024
static char status = 0;

static int zsock;                      // Socket to zebra...
struct ipv4_route *quagga_routes = 0;  // routes currently exportet to zebra

/* prototypes ntern */
static char *try_read(ssize_t *);
static char *zebra_route_packet(struct ipv4_route r, ssize_t *);
static int parse_interface_add(char *, size_t);
static int parse_interface_delete(char *, size_t);
static int parse_interface_up(char *, size_t);
static int parse_interface_down(char *, size_t);
static int parse_interface_address_add(char *, size_t);
static int parse_interface_address_delete(char *, size_t);
static int parse_ipv4_route(char *, size_t, struct ipv4_route *);
static int ipv4_route_add(char *, size_t);
static int ipv4_route_delete(char *, size_t);
static int parse_ipv6_route_add(char *, size_t);
static uint32_t prefixlentomask(uint8_t);
static void free_ipv4_route(struct ipv4_route);
static void update_olsr_zebra_routes(struct ipv4_route *, struct ipv4_route *);
static struct ipv4_route *zebra_create_ipv4_route_table_entry(uint32_t, uint32_t, uint32_t);
static struct ipv4_route *zebra_create_ipv4_route_table(void);
static void zebra_free_ipv4_route_table(struct ipv4_route *);
static uint8_t masktoprefixlen(uint32_t);

#ifdef MY_DEBUG
static void
dump_ipv4_route(struct ipv4_route r, char *c)
{
  int i = 0, x = 0;

  puts(c);
  printf("type: %d\n", r.type);
  puts("flags:");
  printf("  Internal: %s\n", r.flags & ZEBRA_FLAG_INTERNAL ? "yes" : "no");
  printf("  Selfroute %s\n", r.flags & ZEBRA_FLAG_SELFROUTE ? "yes" : "no");
  printf("  Blackhole %s\n", r.flags & ZEBRA_FLAG_BLACKHOLE ? "yes" : "no");
  printf("  IBGP: %s\n", r.flags & ZEBRA_FLAG_IBGP ? "yes" : "no");
  printf("  Selected: %s\n", r.flags & ZEBRA_FLAG_SELECTED ? "yes" : "no");
  printf("  Changed: %s\n", r.flags & ZEBRA_FLAG_CHANGED ? "yes" : "no");
  printf("  static: %s\n", r.flags & ZEBRA_FLAG_STATIC ? "yes" : "no");
  printf("  reject: %s\n", r.flags & ZEBRA_FLAG_REJECT ? "yes" : "no");
  puts("message:");
  printf("  nexthop: %s\n", r.message & ZAPI_MESSAGE_NEXTHOP ? "yes" : "no");
  printf("  ifindex: %s\n", r.message & ZAPI_MESSAGE_IFINDEX ? "yes" : "no");
  printf("  distance: %s\n", r.message & ZAPI_MESSAGE_DISTANCE ? "yes" : "no");
  printf("  metric: %s\n", r.message & ZAPI_MESSAGE_METRIC ? "yes" : "no");
  printf("Prefixlen: %d\n", r.prefixlen);
  printf("Prefix: %d", (unsigned char)r.prefix);
  c = (char *)&r.prefix;
  while (++i < (r.prefixlen / 8 + (r.prefixlen % 8 ? 1 : 0)))
    printf(".%d", (unsigned char)*(c + i));
  while (i++ < 4)
    printf(".0");
  puts("");
  i = 0;
  if (r.message & ZAPI_MESSAGE_NEXTHOP) {

    printf("nexthop-count: %d\n", r.nh_count);
    while (i++ < r.nh_count) {
      c = (unsigned char *)&r.nexthops[i];
      printf("Nexthop %d: %d", i, (unsigned char)*c);
      while (++x < 4) {
        printf(".%d", (unsigned char)c[x]);
      }
      puts("");
    }
    i = 0;
  }
  if (r.message & ZAPI_MESSAGE_IFINDEX) {

    printf("index-number: %d\n", r.ind_num);
    while (i++ < r.ind_num)
      printf("Index: %d: %d\n", i, r.index[i]);
    i = 0;
    if (r.message & ZAPI_MESSAGE_DISTANCE)
      printf("Distance: %d\n", r.distance);
    if (r.message & ZAPI_MESSAGE_METRIC)
      printf("Metric: %d\n", r.metric);
    puts("\n");
  }
}
#endif

void *
my_realloc(void *buf, size_t s, const char *c)
{
  buf = realloc(buf, s);
  if (!buf) {
#ifdef OLSR_PLUGIN
    OLSR_PRINTF(1, "OUT OF MEMORY: %s\n", strerror(errno));
    olsr_syslog(OLSR_LOG_ERR, "olsrd: out of memory!: %m\n");
    olsr_exit(c, EXIT_FAILURE);
#else
    exit(EXIT_FAILURE);
#endif
  }
  return buf;
}

#ifndef OLSR_PLUGIN
void *
olsr_malloc(size_t f, const char *c)
{
  void *v = malloc(f);
  return v;
}
#endif

/* Connect to the zebra-daemon, returns a socket */
int
init_zebra()
{
  struct sockaddr_in i;
  int ret;

  zsock = socket(AF_INET, SOCK_STREAM, 0);
  if (zsock < 0)                // TODO: Could not create socket
    return -1;
  memset(&i, 0, sizeof i);
  i.sin_family = AF_INET;
  i.sin_port = htons(ZEBRA_PORT);
  //  i.sin_len = sizeof i;
  i.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  ret = connect(zsock, (struct sockaddr *)&i, sizeof i);
  if (ret < 0) {
    close(zsock);
    return -1;
  }
  status |= STATUS_CONNECTED;
  return zsock;
}

/* Sends a command to zebra, command is
   the command defined in zebra.h, options is the packet-payload,
   optlen the length, of the payload */
char
zebra_send_command(unsigned char command, char *options, int optlen)
{

  char *p = olsr_malloc(optlen + 3, "zebra send_command");
  uint16_t length = optlen + 3;        // length of option + command + packet_length

  int ret;

  uint16_t len = htons(length);
  memcpy(p, &len, 2);
  p[2] = command;
  memcpy(p + 3, options, optlen);

  do {
    ret = write(zsock, p, length);
    if (ret < 0) {
      if (errno == EINTR)
        continue;
    } else
      return -1;
    p = p + ret;
  }
  while ((length -= ret));

  return 0;
}

/* Creates a Route-Packet-Payload, needs address, netmask, nexthop,
   distance, and a pointer of an size_t */
static char *
zebra_route_packet(struct ipv4_route r, ssize_t * optlen)
{

  char *cmdopt, *t;
  *optlen = 9;                  // first: type, flags, message, prefixlen, nexthop number, nexthop)
  *optlen += r.prefixlen / 8 + (r.prefixlen % 8 ? 1 : 0);

  cmdopt = olsr_malloc(*optlen, "zebra add_v4_route");
  t = cmdopt;
  *t++ = 10;                    // Type: olsr
  *t++ = r.flags;               // flags
  *t++ = r.message;             // message: contains nexthop
  *t++ = r.prefixlen;
  memcpy(t, &r.prefix, r.prefixlen / 8 + (r.prefixlen % 8 ? 1 : 0));
  *t += r.prefixlen / 8 + (r.prefixlen % 8 ? 1 : 0);
  *t++ = r.nh_count;
  memcpy(t, r.nexthops, r.nh_count * sizeof *r.nexthops);
  return cmdopt;
}

/* adds a route to zebra-daemon (needs socket from zebra,
   address = prefix of the route
   mask = netmask of the route
   nexthop = nexthop of the route
   distance = distance-value of the route
*/
int
zebra_add_v4_route(struct ipv4_route r)
{

  char *cmdopt;
  ssize_t optlen;

  cmdopt = zebra_route_packet(r, &optlen);

  puts("DEBUG: zebra_route_packet returned");

  return zebra_send_command(ZEBRA_IPV4_ROUTE_ADD, cmdopt, optlen);

}

/* deletes a route from the zebra-daemon (
   needs socket from zebra,
   address = prefix of the route
   mask = netmask of the route
   nexthop = nexthop of the route
   distance = distance-value of the route
*/
int
zebra_delete_v4_route(struct ipv4_route r)
{

  char *cmdopt;
  ssize_t optlen;

  cmdopt = zebra_route_packet(r, &optlen);

  return zebra_send_command(ZEBRA_IPV4_ROUTE_DELETE, cmdopt, optlen);

}

/* Check wether there is data from zebra aviable */
void
zebra_check(void *foo)
{
  char *data, *f;
  ssize_t len, ret;

  if (!status & STATUS_CONNECTED) {
  }
  data = try_read(&len);
  if (data) {
    f = data;
    do {
      ret = zebra_parse_packet(f, len);
      if (!ret) {               //something wired happened
        puts("DEBUG: IIIIIIIIIIRGS");
        exit(EXIT_FAILURE);
      }
      f += ret;
    }
    while ((f - data) < len);
    free(data);
  }
}

// tries to read a packet from zebra_socket
// if there is something to read - make sure to read whole packages
static char *
try_read(ssize_t * len)
{
  char *buf = NULL;
  ssize_t ret = 0, bsize = 0;
  uint16_t length = 0, l = 0;
  int sockstate;

  *len = 0;

  sockstate = fcntl(zsock, F_GETFL, 0);
  fcntl(zsock, F_SETFL, sockstate | O_NONBLOCK);

  do {
    if (*len == bsize) {
      bsize += BUFSIZE;
      buf = my_realloc(buf, bsize, "Zebra try_read");
    }
    ret = read(zsock, buf + l, bsize - l);
    if (ret <= 0) {
      if (errno == EAGAIN) {
        errno = 0;
      } else {
        // TODO: errorhandling
        ;
      }
      free(buf);
      return NULL;
    }
    *len += ret;

    while ((*len - l) > length) {
      //      printf ("DEBUG: *len -l > length - %d - %d > %d\n", *len, l, length);
      l += length;
      memcpy(&length, buf + l, 2);
      length = ntohs(length);
    }
    //    printf ("DEBUG: *len, l, length: %d,%d,%d\n", *len, l, length);
    if (((*len) - l) == length)
      break;                    // GOT FULL PACKAGE!!
    if (*len < l) {
      //      printf ("DEBUG: *len, l, length: %d,%d,%d\n", *len, l, length);
      fcntl(zsock, F_SETFL, sockstate);
      continue;
    }
  }
  while (1);

  fcntl(zsock, F_SETFL, sockstate);
  return buf;
}

/* Parse a packet recived from zebra */
int
zebra_parse_packet(char *packet, ssize_t maxlen)
{

  /* Array of functions */
  int (*foo[ZEBRA_MESSAGE_MAX]) (char *, size_t) = {
  parse_interface_add, parse_interface_delete, parse_interface_address_add, parse_interface_address_delete, parse_interface_up,
      parse_interface_down, ipv4_route_add, ipv4_route_delete, parse_ipv6_route_add};

  puts("DEBUG: zebra_parse_packet");
  uint16_t length;

  int ret;
  memcpy(&length, packet, 2);
  length = ntohs(length);

  if (maxlen < length) {
    puts("Error: programmer is an idiot");
    printf("DEBUG: maxlen = %d, packet_length = %d\n", maxlen, length);
    return maxlen;
  }

  if (packet[2] - 1 < ZEBRA_MESSAGE_MAX && foo[packet[2] - 1]) {
    if (!(ret = foo[packet[2] - 1] (packet + 3, length - 3)))
      return length;
    else
      printf("DEBUG: Parse error: %d\n", ret);
  } else
    printf("Unknown packet type: %d\n", packet[2]);

  puts("Quagga: RECIVED PACKET FROM ZEBRA THAT I CAN'T PARSE");

  return length;
}

static int
parse_interface_add(char *opt, size_t len)
{
  //todo
  return 0;
}

static int
parse_interface_delete(char *opt, size_t len)
{
  //todo
  return 0;
}

static int
parse_interface_address_add(char *opt, size_t len)
{

  //todo
  return 0;
}

static int
parse_interface_up(char *opt, size_t len)
{

  //todo
  return 0;
}

static int
parse_interface_down(char *opt, size_t len)
{

  //todo
  return 0;
}

static int
parse_interface_address_delete(char *opt, size_t len)
{
  //todo
  return 0;
}

/* Parse an ipv4-route-packet recived from zebra
 */
static int
parse_ipv4_route(char *opt, size_t len, struct ipv4_route *r)
{
  //  puts ("DEBUG: parse_ipv4_route");
  if (len < 4)
    return -1;

  r->type = *opt++;
  r->flags = *opt++;
  r->message = *opt++;
  r->prefixlen = *opt++;
  len -= 4;
  r->prefix = 0;

  if ((int)len < r->prefixlen / 8 + (r->prefixlen % 8 ? 1 : 0))
    return -1;

  memcpy(&r->prefix, opt, r->prefixlen / 8 + (r->prefixlen % 8 ? 1 : 0));
  opt += r->prefixlen / 8 + (r->prefixlen % 8 ? 1 : 0);

  if (r->message & ZAPI_MESSAGE_NEXTHOP) {
    if (len < 1)
      return -1;
    r->nh_count = *opt++;
    if (len < sizeof(uint32_t) * r->nh_count)
      return -1;
    r->nexthops = olsr_malloc(sizeof(uint32_t) * r->nh_count, "quagga: parse_ipv4_route_add");
    memcpy(r->nexthops, opt, sizeof(uint32_t) * r->nh_count);
    opt += sizeof(uint32_t) * r->nh_count;
    len -= sizeof(uint32_t) * r->nh_count + 1;
  }

  if (r->message & ZAPI_MESSAGE_IFINDEX) {
    if (len < 1)
      return -2;
    r->ind_num = *opt++;
    if (len < sizeof(uint32_t) * r->ind_num)
      return -3;
    r->index = olsr_malloc(sizeof(uint32_t) * r->ind_num, "quagga: parse_ipv4_route_add");
    memcpy(r->index, opt, r->ind_num * sizeof(uint32_t));
    opt += sizeof(uint32_t) * r->ind_num;
    len -= sizeof(uint32_t) * r->ind_num;
  }

  if (r->message & ZAPI_MESSAGE_DISTANCE)
    // todo
    ;

  if (r->message & ZAPI_MESSAGE_METRIC) {
    if (len < sizeof(uint32_t))
      return -4;
    memcpy(&r->metric, opt, sizeof(uint32_t));
  }

  return 0;
}

static int
ipv4_route_add(char *opt, size_t len)
{

  struct ipv4_route r;
  int f;

  //  puts ("DEBUG: ipv4_route_add");

  f = parse_ipv4_route(opt, len, &r);
  if (f < 0) {
    printf("parse-error: %d\n", f);
    return f;
  }

  add_hna4_route(r);
  return 0;
}

static int
ipv4_route_delete(char *opt, size_t len)
{
  struct ipv4_route r;
  int f;

  f = parse_ipv4_route(opt, len, &r);
  if (f < 0)
    return f;

  return delete_hna4_route(r);
  // OK, now delete that foo

}

static int
parse_ipv6_route_add(char *opt, size_t len)
{
  //todo
  return 0;
}

/* start redistribution FROM zebra */
int
zebra_redistribute(unsigned char type)
{

  return zebra_send_command(ZEBRA_REDISTRIBUTE_ADD, &type, 1);

}

/* end redistribution FROM zebra */
int
zebra_disable_redistribute(unsigned char type)
{

  return zebra_send_command(ZEBRA_REDISTRIBUTE_DELETE, &type, 1);

}

static uint32_t
prefixlentomask(uint8_t prefix)
{
  uint32_t mask;
  mask = prefix_to_netmask4(prefix);
  mask = ntohl(mask);
  return mask;
}

int
add_hna4_route(struct ipv4_route r)
{
  union olsr_ip_addr net, mask;

#ifdef MY_DEBUG
  dump_ipv4_route(r, "add_hna4_route");
#endif

  mask.v4 = prefixlentomask(r.prefixlen);
  net.v4 = r.prefix;

#ifdef OLSR_PLUGIN
  add_local_hna4_entry(&net, &mask);
#endif
  free_ipv4_route(r);
  return 0;
}

int
delete_hna4_route(struct ipv4_route r)
{

  union olsr_ip_addr net, mask;

#ifdef MY_DEBUG
  dump_ipv4_route(r, "delete_hna4_route");
#endif

  mask.v4 = prefixlentomask(r.prefixlen);
  net.v4 = r.prefix;

#ifdef OLSR_PLUGIN
  return remove_local_hna4_entry(&net, &mask) ? 0 : -1;
#endif

  free_ipv4_route(r);
  return 0;

}

static void
free_ipv4_route(struct ipv4_route r)
{

  if (r.message & ZAPI_MESSAGE_IFINDEX && r.ind_num)
    free(r.index);
  if (r.message & ZAPI_MESSAGE_NEXTHOP && r.nh_count)
    free(r.nexthops);

}

void
zebra_clear_routes(void)
{

  struct ipv4_route *t;

  t = quagga_routes;
  while (t) {
    zebra_delete_v4_route(*t);
    t = t->next;
  }
  zebra_free_ipv4_route_table(quagga_routes);

  quagga_routes = NULL;
}

void
zebra_update_hna(void *f)
{

  struct ipv4_route *a = zebra_create_ipv4_route_table();
  update_olsr_zebra_routes(a, quagga_routes);
  zebra_free_ipv4_route_table(quagga_routes);

  quagga_routes = a;

}

static struct ipv4_route *
zebra_create_ipv4_route_table(void)
{

  struct ipv4_route *r = 0, *t = 0 /* make compiler happy */ ;
  int i;
  struct hna_entry *e;
  struct hna_net *n;

  for (i = 0; i < HASHSIZE; i++) {
    e = hna_set[i].next;
    for (; e != &hna_set[i]; e = e->next) {
      n = e->networks.next;
      for (; n != &e->networks; n = n->next) {
        if (!r) {
          r = zebra_create_ipv4_route_table_entry(n->A_network_addr.v4, n->A_netmask.v4, e->A_gateway_addr.v4);
          t = r;
        } else {
          t->next = zebra_create_ipv4_route_table_entry(n->A_network_addr.v4, n->A_netmask.v4, e->A_gateway_addr.v4);
          t = t->next;
        }
      }
    }
  }

  return r;

}

static struct ipv4_route *
zebra_create_ipv4_route_table_entry(uint32_t addr, uint32_t mask, uint32_t gw)
{

  struct ipv4_route *r;

  r = olsr_malloc(sizeof *r, "zebra_create_ipv4_route_table_entry");
  memset(r, 0, sizeof *r);
  r->prefix = addr;
  r->prefixlen = masktoprefixlen(mask);
  r->message |= ZAPI_MESSAGE_NEXTHOP;
  r->nh_count = 1;
  r->nexthops = olsr_malloc(sizeof(uint32_t), "zebra_create_ipv4_route_table_entry");
  *r->nexthops = gw;
  r->next = NULL;

  return r;
}

static uint8_t
masktoprefixlen(uint32_t mask)
{

  uint8_t prefixlen = 0;
while (mask & (1 << ++prefixlen && prefixlen < 32); return prefixlen;}

       static void update_olsr_zebra_routes(struct ipv4_route *a, struct ipv4_route *r) {

       struct ipv4_route * t; if (!r) {
       puts("no quagga_routing_table aviable"); for (; a; a = a->next) {
       dump_ipv4_route(*a, "adding this route");
       //      zebra_add_v4_route(*r);
       }
       return;}

       while (a) {
       for (t = r; t; t = t->next) {
       if (a->prefix == t->prefix) if (a->prefixlen == t->prefixlen) if (*a->nexthops == *t->nexthops) {
       goto foo;}
       }
       dump_ipv4_route(*a, "adding this route");
       //zebra_add_v4_route(*a);
  foo:
       a = a->next;}

       while (r) {
       for (t = a; t; t = t->next) {
       if (r->prefix == t->prefix) if (r->prefixlen == t->prefixlen) if (*r->nexthops == *t->nexthops) {
       goto bar;}
       }
       dump_ipv4_route(*r, "deleting this route");
       //zebra_delete_v4_route(*r);
  bar:
       r = r->next;}

       }

       static void zebra_free_ipv4_route_table(struct ipv4_route *r) {
       struct ipv4_route * n; if (!r) return; while ((n = r->next)) {
       if (r->message & ZAPI_MESSAGE_NEXTHOP) free(r->nexthops); if (r->message & ZAPI_MESSAGE_IFINDEX) free(r->index); free(r);
       r = n;}
       }

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
