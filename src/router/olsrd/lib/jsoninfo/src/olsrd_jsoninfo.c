/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tonnesen(andreto@olsr.org)
 *                     includes code by Bruno Randolf
 *                     includes code by Andreas Lopatic
 *                     includes code by Sven-Ola Tuecke
 *                     includes code by Lorenz Schori
 *                     includes bugs by Markus Kittenberger
 *                     includes bugs by Hans-Christoph Steiner
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

/*
 * Dynamic linked library for the olsr.org olsr daemon
 */


#include <sys/types.h>
#include <sys/socket.h>
#ifndef _WIN32
#include <sys/select.h>
#endif /* _WIN32 */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <libgen.h>

#ifdef __linux__
#include <fcntl.h>
#endif /* __linux__ */

#include "ipcalc.h"
#include "olsr.h"
#include "builddata.h"
#include "olsr_types.h"
#include "neighbor_table.h"
#include "two_hop_neighbor_table.h"
#include "mpr_selector_set.h"
#include "tc_set.h"
#include "hna_set.h"
#include "mid_set.h"
#include "link_set.h"
#include "net_olsr.h"
#include "lq_plugin.h"
#include "common/autobuf.h"
#include "gateway.h"

#include "olsrd_jsoninfo.h"
#include "olsrd_plugin.h"

#ifdef _WIN32
#define close(x) closesocket(x)
#endif /* _WIN32 */

static int ipc_socket;

/* Response types */
#define HTTP_200 "HTTP/1.1 200 OK"

/* IPC initialization function */
static int plugin_ipc_init(void);

static int read_uuid_from_file(const char *file);

static void abuf_json_open_object(struct autobuf *abuf, const char* header);
static void abuf_json_close_object(struct autobuf *abuf);
static void abuf_json_open_array(struct autobuf *abuf, const char* header);
static void abuf_json_close_array(struct autobuf *abuf);
static void abuf_json_open_array_entry(struct autobuf *abuf);
static void abuf_json_close_array_entry(struct autobuf *abuf);
static void abuf_json_boolean(struct autobuf *abuf, const char* key, int value);
static void abuf_json_string(struct autobuf *abuf, const char* key, const char* value);
static void abuf_json_int(struct autobuf *abuf, const char* key, long value);
static void abuf_json_float(struct autobuf *abuf, const char* key, float value);

static void send_info(unsigned int /*send_what*/, int /*socket*/);
static void ipc_action(int, void *, unsigned int);
static void ipc_print_neighbors(struct autobuf *);
static void ipc_print_links(struct autobuf *);
static void ipc_print_routes(struct autobuf *);
static void ipc_print_topology(struct autobuf *);
static void ipc_print_hna(struct autobuf *);
static void ipc_print_mid(struct autobuf *);
static void ipc_print_gateways(struct autobuf *);
static void ipc_print_config(struct autobuf *);
static void ipc_print_interfaces(struct autobuf *);
static void ipc_print_plugins(struct autobuf *);
static void ipc_print_olsrd_conf(struct autobuf *abuf);

static size_t build_http_header(const char *status, const char *mime,
  uint32_t msgsize, char *buf, uint32_t bufsize);

/*
 * this is the size of the buffer used for build_http_header
 * the amount of data written into the buffer will be less than
 * 400 bytes approximatively.
 * The size may vary because the Content-Length header contains
 * the length of the json data
 */
#define MAX_HTTPHEADER_SIZE 512

#define TXT_IPC_BUFSIZE 256

/* these provide all of the runtime status info */
#define SIW_NEIGHBORS 0x0001
#define SIW_LINKS 0x0002
#define SIW_ROUTES 0x0004
#define SIW_HNA 0x0008
#define SIW_MID 0x0010
#define SIW_TOPOLOGY 0x0020
#define SIW_GATEWAYS 0x0040
#define SIW_INTERFACES 0x0080
#define SIW_RUNTIME_ALL 0x00FF

/* these only change at olsrd startup */
#define SIW_CONFIG 0x0100
#define SIW_PLUGINS 0x0200
#define SIW_STARTUP_ALL 0x0F00

/* this is everything in JSON format */
#define SIW_ALL 0x0FFF

/* this data is not JSON format but olsrd.conf format */
#define SIW_OLSRD_CONF 0x1000

#define MAX_CLIENTS 3

static char *outbuffer[MAX_CLIENTS];
static size_t outbuffer_size[MAX_CLIENTS];
static size_t outbuffer_written[MAX_CLIENTS];
static int outbuffer_socket[MAX_CLIENTS];
static int outbuffer_count;

char uuid[UUIDLEN + 1];
char uuidfile[FILENAME_MAX];

static struct timeval start_time;
static struct timer_entry *writetimer_entry;


/* JSON support functions */

/* JSON does not allow commas dangling at the end of arrays, so we need to
 * count which entry number we're at in order to make sure we don't tack a
 * dangling comma on at the end */
static int entrynumber[5] = {0,0,0,0,0};
static int currentjsondepth = 0;

static void
abuf_json_open_object(struct autobuf *abuf, const char* header)
{
  abuf_appendf(abuf, "\"%s\": {", header);
  entrynumber[currentjsondepth]++;
  currentjsondepth++;
  entrynumber[currentjsondepth] = 0;
}

static void
abuf_json_close_object(struct autobuf *abuf)
{
  abuf_appendf(abuf, "\t}\n");
  currentjsondepth--;
}

static void
abuf_json_open_array(struct autobuf *abuf, const char* header)
{
  if (entrynumber[currentjsondepth])
    abuf_appendf(abuf, ",\n\t");
  abuf_appendf(abuf, "\"%s\": [\n", header);
  entrynumber[currentjsondepth]++;
  currentjsondepth++;
  entrynumber[currentjsondepth] = 0;
}

static void
abuf_json_close_array(struct autobuf *abuf)
{
  abuf_appendf(abuf, "]\n");
  entrynumber[currentjsondepth] = 0;
  currentjsondepth--;
}

static void
abuf_json_open_array_entry(struct autobuf *abuf)
{
  if (entrynumber[currentjsondepth])
    abuf_appendf(abuf, ",\n{");
  else
    abuf_appendf(abuf, "{");
  entrynumber[currentjsondepth]++;
  currentjsondepth++;
  entrynumber[currentjsondepth] = 0;
}

static void
abuf_json_close_array_entry(struct autobuf *abuf)
{
  abuf_appendf(abuf, "}");
  entrynumber[currentjsondepth] = 0;
  currentjsondepth--;
}

static void
abuf_json_insert_comma(struct autobuf *abuf)
{
  if (entrynumber[currentjsondepth])
    abuf_appendf(abuf, ",\n");
  else
    abuf_appendf(abuf, "\n");
  entrynumber[currentjsondepth]++;
}

static void
abuf_json_boolean(struct autobuf *abuf, const char* key, int value)
{
  abuf_json_insert_comma(abuf);
  abuf_appendf(abuf, "\t\"%s\": %s", key, value ? "true" : "false");
}

static void
abuf_json_string(struct autobuf *abuf, const char* key, const char* value)
{
  abuf_json_insert_comma(abuf);
  abuf_appendf(abuf, "\t\"%s\": \"%s\"", key, value);
}

static void
abuf_json_int(struct autobuf *abuf, const char* key, long value)
{
  abuf_json_insert_comma(abuf);
  abuf_appendf(abuf, "\t\"%s\": %li", key, value);
}

static void
abuf_json_float(struct autobuf *abuf, const char* key, float value)
{
  abuf_json_insert_comma(abuf);
  abuf_appendf(abuf, "\t\"%s\": %.03f", key, (double)value);
}



/* Linux specific functions for getting system info */

#ifdef __linux__
static int get_string_from_file(const char* filename, char* buf, int len)
{
  int bytes = -1;
  int fd = open(filename, O_RDONLY);
  if (fd > -1) {
    bytes = read(fd, buf, len);
    if (bytes < len)
      buf[bytes-1] = '\0'; // remove trailing \n
    else
      buf[len-1] = '\0';
    close(fd);
  }
  return bytes;
}

static int
abuf_json_sysdata(struct autobuf *abuf, const char* key, const char* syspath)
{
  int ret = -1;
  char buf[256];
  *buf = 0;
  ret = get_string_from_file(syspath, buf, 256);
  if (*buf != 0)
    abuf_json_string(abuf, key, buf);
  return ret;
}

static void
abuf_json_sys_class_net(struct autobuf *abuf, const char* key,
                        const char* ifname, const char* datapoint)
{
  char filename[256];
  snprintf(filename, 255, "/sys/class/net/%s/%s", ifname, datapoint);
  abuf_json_sysdata(abuf, key, filename);
}

#endif /* __linux__ */



/**
 *Do initialization here
 *
 *This function is called by the my_init
 *function in uolsrd_plugin.c
 */
int
olsrd_plugin_init(void)
{
  /* Initial IPC value */
  ipc_socket = -1;

  /* Get start time */
  gettimeofday(&start_time, NULL);

  if (!strlen(uuidfile))
    strscpy(uuidfile, "uuid.txt", sizeof(uuidfile));
  read_uuid_from_file(uuidfile);

  plugin_ipc_init();
  return 1;
}

/**
 * destructor - called at unload
 */
void
olsr_plugin_exit(void)
{
  if (ipc_socket != -1)
    close(ipc_socket);
}

static int
plugin_ipc_init(void)
{
  union olsr_sockaddr sst;
  uint32_t yes = 1;
  socklen_t addrlen;

  /* Init ipc socket */
  if ((ipc_socket = socket(olsr_cnf->ip_version, SOCK_STREAM, 0)) == -1) {
#ifndef NODEBUG
    olsr_printf(1, "(JSONINFO) socket()=%s\n", strerror(errno));
#endif /* NODEBUG */
    return 0;
  } else {
    if (setsockopt(ipc_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof(yes)) < 0) {
#ifndef NODEBUG
      olsr_printf(1, "(JSONINFO) setsockopt()=%s\n", strerror(errno));
#endif /* NODEBUG */
      return 0;
    }
#if (defined __FreeBSD__ || defined __FreeBSD_kernel__) && defined SO_NOSIGPIPE
    if (setsockopt(ipc_socket, SOL_SOCKET, SO_NOSIGPIPE, (char *)&yes, sizeof(yes)) < 0) {
      perror("SO_REUSEADDR failed");
      return 0;
    }
#endif /* (defined __FreeBSD__ || defined __FreeBSD_kernel__) && defined SO_NOSIGPIPE */
#if defined linux
    if (jsoninfo_ipv6_only && olsr_cnf->ip_version == AF_INET6) {
      if (setsockopt(ipc_socket, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&yes, sizeof(yes)) < 0) {
        perror("IPV6_V6ONLY failed");
        return 0;
      }
    }
#endif /* defined linux */
    /* Bind the socket */

    /* complete the socket structure */
    memset(&sst, 0, sizeof(sst));
    if (olsr_cnf->ip_version == AF_INET) {
      sst.in4.sin_family = AF_INET;
      addrlen = sizeof(struct sockaddr_in);
#ifdef SIN6_LEN
      sst.in4.sin_len = addrlen;
#endif /* SIN6_LEN */
      sst.in4.sin_addr.s_addr = jsoninfo_listen_ip.v4.s_addr;
      sst.in4.sin_port = htons(ipc_port);
    } else {
      sst.in6.sin6_family = AF_INET6;
      addrlen = sizeof(struct sockaddr_in6);
#ifdef SIN6_LEN
      sst.in6.sin6_len = addrlen;
#endif /* SIN6_LEN */
      sst.in6.sin6_addr = jsoninfo_listen_ip.v6;
      sst.in6.sin6_port = htons(ipc_port);
    }

    /* bind the socket to the port number */
    if (bind(ipc_socket, &sst.in, addrlen) == -1) {
#ifndef NODEBUG
      olsr_printf(1, "(JSONINFO) bind()=%s\n", strerror(errno));
#endif /* NODEBUG */
      return 0;
    }

    /* show that we are willing to listen */
    if (listen(ipc_socket, 1) == -1) {
#ifndef NODEBUG
      olsr_printf(1, "(JSONINFO) listen()=%s\n", strerror(errno));
#endif /* NODEBUG */
      return 0;
    }

    /* Register with olsrd */
    add_olsr_socket(ipc_socket, &ipc_action, NULL, NULL, SP_PR_READ);

#ifndef NODEBUG
    olsr_printf(2, "(JSONINFO) listening on port %d\n", ipc_port);
#endif /* NODEBUG */
  }
  return 1;
}

static int
read_uuid_from_file(const char *file)
{
  FILE *f;
  char* end;
  int r = 0;
  size_t chars;

  memset(uuid, 0, sizeof(uuid));

  f = fopen(file, "r");
  olsr_printf(1, "(JSONINFO) Reading UUID from '%s'\n", file);
  if (f == NULL ) {
    olsr_printf(1, "(JSONINFO) Could not open '%s': %s\n",
                file, strerror(errno));
    return -1;
  }
  chars = fread(uuid, 1, UUIDLEN, f);
  if (chars > 0) {
    uuid[chars] = '\0'; /* null-terminate the string */

    /* we only use the first line of the file */
    end = strchr(uuid, '\n');
    if(end)
      *end = 0;
    r = 0;
  } else {
    olsr_printf(1, "(JSONINFO) Could not read UUID from '%s': %s\n",
                file, strerror(errno));
    r = -1;
  }

  fclose(f);
  return r;
}

static void
ipc_action(int fd, void *data __attribute__ ((unused)), unsigned int flags __attribute__ ((unused)))
{
  union olsr_sockaddr pin;

  char addr[INET6_ADDRSTRLEN];
  fd_set rfds;
  struct timeval tv;
  unsigned int send_what = 0;
  int ipc_connection;

  socklen_t addrlen = sizeof(pin);

  if ((ipc_connection = accept(fd, &pin.in, &addrlen)) == -1) {
#ifndef NODEBUG
    olsr_printf(1, "(JSONINFO) accept()=%s\n", strerror(errno));
#endif /* NODEBUG */
    return;
  }

  tv.tv_sec = tv.tv_usec = 0;
  if (olsr_cnf->ip_version == AF_INET) {
    if (inet_ntop(olsr_cnf->ip_version, &pin.in4.sin_addr, addr, INET6_ADDRSTRLEN) == NULL)
      addr[0] = '\0';
    if (!ip4equal(&pin.in4.sin_addr, &jsoninfo_accept_ip.v4) && jsoninfo_accept_ip.v4.s_addr != INADDR_ANY) {
#ifdef JSONINFO_ALLOW_LOCALHOST
      if (pin.in4.sin_addr.s_addr != INADDR_LOOPBACK) {
#endif /* JSONINFO_ALLOW_LOCALHOST */
        olsr_printf(1, "(JSONINFO) From host(%s) not allowed!\n", addr);
        close(ipc_connection);
        return;
#ifdef JSONINFO_ALLOW_LOCALHOST
      }
#endif /* JSONINFO_ALLOW_LOCALHOST */
    }
  } else {
    if (inet_ntop(olsr_cnf->ip_version, &pin.in6.sin6_addr, addr, INET6_ADDRSTRLEN) == NULL)
      addr[0] = '\0';
    /* Use in6addr_any (::) in olsr.conf to allow anybody. */
    if (!ip6equal(&in6addr_any, &jsoninfo_accept_ip.v6) && !ip6equal(&pin.in6.sin6_addr, &jsoninfo_accept_ip.v6)) {
      olsr_printf(1, "(JSONINFO) From host(%s) not allowed!\n", addr);
      close(ipc_connection);
      return;
    }
  }

#ifndef NODEBUG
  olsr_printf(2, "(JSONINFO) Connect from %s\n", addr);
#endif /* NODEBUG */

  /* purge read buffer to prevent blocking on linux */
  FD_ZERO(&rfds);
  FD_SET((unsigned int)ipc_connection, &rfds);  /* Win32 needs the cast here */
  if (0 <= select(ipc_connection + 1, &rfds, NULL, NULL, &tv)) {
    char requ[1024];
    ssize_t s = recv(ipc_connection, (void *)&requ, sizeof(requ)-1, 0);   /* Win32 needs the cast here */

    if (s == sizeof(requ)-1) {
      /* input was too much long, just skip the rest */
      char dummy[1024];

      while (recv(ipc_connection, (void *)&dummy, sizeof(dummy), 0) == sizeof(dummy));
    }
    if (0 < s) {
      requ[s] = 0;
      /* print out the requested tables */
      if (0 != strstr(requ, "/olsrd.conf"))
        send_what |= SIW_OLSRD_CONF;
      else if (0 != strstr(requ, "/all"))
        send_what = SIW_ALL;
      else {
        // these are the two overarching categories
        if (0 != strstr(requ, "/runtime")) send_what |= SIW_RUNTIME_ALL;
        if (0 != strstr(requ, "/startup")) send_what |= SIW_STARTUP_ALL;
        // these are the individual sections
        if (0 != strstr(requ, "/neighbors")) send_what |= SIW_NEIGHBORS;
        if (0 != strstr(requ, "/links")) send_what |= SIW_LINKS;
        if (0 != strstr(requ, "/routes")) send_what |= SIW_ROUTES;
        if (0 != strstr(requ, "/hna")) send_what |= SIW_HNA;
        if (0 != strstr(requ, "/mid")) send_what |= SIW_MID;
        if (0 != strstr(requ, "/topology")) send_what |= SIW_TOPOLOGY;
        if (0 != strstr(requ, "/gateways")) send_what |= SIW_GATEWAYS;
        if (0 != strstr(requ, "/interfaces")) send_what |= SIW_INTERFACES;
        if (0 != strstr(requ, "/config")) send_what |= SIW_CONFIG;
        if (0 != strstr(requ, "/plugins")) send_what |= SIW_PLUGINS;
      }
    }
    if ( send_what == 0 ) send_what = SIW_ALL;
  }

  send_info(send_what, ipc_connection);
}

static void
ipc_print_neighbors(struct autobuf *abuf)
{
  struct ipaddr_str buf1;
  struct neighbor_entry *neigh;
  struct neighbor_2_list_entry *list_2;
  int thop_cnt;

  abuf_json_open_array(abuf, "neighbors");

  /* Neighbors */
  OLSR_FOR_ALL_NBR_ENTRIES(neigh) {
    abuf_json_open_array_entry(abuf);
    abuf_json_string(abuf, "ipv4Address",
                     olsr_ip_to_string(&buf1, &neigh->neighbor_main_addr));
    abuf_json_boolean(abuf, "symmetric", (neigh->status == SYM));
    abuf_json_boolean(abuf, "multiPointRelay", neigh->is_mpr);
    abuf_json_boolean(abuf, "multiPointRelaySelector",
                      olsr_lookup_mprs_set(&neigh->neighbor_main_addr) != NULL);
    abuf_json_int(abuf, "willingness", neigh->willingness);
    abuf_appendf(abuf, ",\n");

    thop_cnt = 0;
    if (neigh->neighbor_2_list.next) {
      abuf_appendf(abuf, "\t\"twoHopNeighbors\": [");
      for (list_2 = neigh->neighbor_2_list.next; list_2 != &neigh->neighbor_2_list; list_2 = list_2->next) {
        if (thop_cnt)
          abuf_appendf(abuf, ", ");
        abuf_appendf(abuf,
                     "\"%s\"",
                     olsr_ip_to_string(&buf1, &list_2->neighbor_2->neighbor_2_addr));
        thop_cnt++;
      }
    }
    abuf_appendf(abuf, "]");
    abuf_json_int(abuf, "twoHopNeighborCount", thop_cnt);
    abuf_json_close_array_entry(abuf);
  }
  OLSR_FOR_ALL_NBR_ENTRIES_END(neigh);
  abuf_json_close_array(abuf);
}

static void
ipc_print_links(struct autobuf *abuf)
{
  struct ipaddr_str buf1, buf2;
  struct lqtextbuffer lqbuffer1;

  struct link_entry *my_link = NULL;

  abuf_json_open_array(abuf, "links");
  OLSR_FOR_ALL_LINK_ENTRIES(my_link) {
    const char* lqs;
    int diff = (unsigned int)(my_link->link_timer->timer_clock - now_times);

    abuf_json_open_array_entry(abuf);
    abuf_json_string(abuf, "localIP",
                     olsr_ip_to_string(&buf1, &my_link->local_iface_addr));
    abuf_json_string(abuf, "remoteIP",
                     olsr_ip_to_string(&buf2, &my_link->neighbor_iface_addr));
    abuf_json_int(abuf, "validityTime", diff);
    lqs = get_link_entry_text(my_link, '\t', &lqbuffer1);
    abuf_json_float(abuf, "linkQuality", atof(lqs));
    abuf_json_float(abuf, "neighborLinkQuality", atof(strrchr(lqs, '\t')));
    if (my_link->linkcost >= LINK_COST_BROKEN)
      abuf_json_int(abuf, "linkCost", LINK_COST_BROKEN);
    else
      abuf_json_int(abuf, "linkCost", my_link->linkcost);
    abuf_json_close_array_entry(abuf);
  }
  OLSR_FOR_ALL_LINK_ENTRIES_END(my_link);
  abuf_json_close_array(abuf);
}

static void
ipc_print_routes(struct autobuf *abuf)
{
  struct ipaddr_str buf1, buf2;
  struct rt_entry *rt;

  abuf_json_open_array(abuf, "routes");

  /* Walk the route table */
  OLSR_FOR_ALL_RT_ENTRIES(rt) {
    abuf_json_open_array_entry(abuf);
    abuf_json_string(abuf, "destination",
                     olsr_ip_to_string(&buf1, &rt->rt_dst.prefix));
    abuf_json_int(abuf, "genmask", rt->rt_dst.prefix_len);
    abuf_json_string(abuf, "gateway",
                     olsr_ip_to_string(&buf2, &rt->rt_best->rtp_nexthop.gateway));
    abuf_json_int(abuf, "metric", rt->rt_best->rtp_metric.hops);
    if (rt->rt_best->rtp_metric.cost >= ROUTE_COST_BROKEN)
      abuf_json_int(abuf, "rtpMetricCost", ROUTE_COST_BROKEN);
    else
      abuf_json_int(abuf, "rtpMetricCost", rt->rt_best->rtp_metric.cost);
    abuf_json_string(abuf, "networkInterface",
                     if_ifwithindex_name(rt->rt_best->rtp_nexthop.iif_index));
    abuf_json_close_array_entry(abuf);
  }
  OLSR_FOR_ALL_RT_ENTRIES_END(rt);

  abuf_json_close_array(abuf);
}

static void
ipc_print_topology(struct autobuf *abuf)
{
  struct tc_entry *tc;

  abuf_json_open_array(abuf, "topology");

  /* Topology */
  OLSR_FOR_ALL_TC_ENTRIES(tc) {
    struct tc_edge_entry *tc_edge;
    OLSR_FOR_ALL_TC_EDGE_ENTRIES(tc, tc_edge) {
      if (tc_edge->edge_inv) {
        struct ipaddr_str dstbuf, addrbuf;
        struct lqtextbuffer lqbuffer1;
        uint32_t vt = tc->validity_timer != NULL ? (tc->validity_timer->timer_clock - now_times) : 0;
        int diff = (int)(vt);
        const char* lqs;
        abuf_json_open_array_entry(abuf);
        abuf_json_string(abuf, "destinationIP",
                         olsr_ip_to_string(&dstbuf, &tc_edge->T_dest_addr));
        abuf_json_string(abuf, "lastHopIP",
                         olsr_ip_to_string(&addrbuf, &tc->addr));
        lqs = get_tc_edge_entry_text(tc_edge, '\t', &lqbuffer1);
        abuf_json_float(abuf, "linkQuality", atof(lqs));
        abuf_json_float(abuf, "neighborLinkQuality", atof(strrchr(lqs, '\t')));
        if (tc_edge->cost >= LINK_COST_BROKEN)
          abuf_json_int(abuf, "tcEdgeCost", LINK_COST_BROKEN);
        else
          abuf_json_int(abuf, "tcEdgeCost", tc_edge->cost);
        abuf_json_int(abuf, "validityTime", diff);
        abuf_json_close_array_entry(abuf);
      }
    }
    OLSR_FOR_ALL_TC_EDGE_ENTRIES_END(tc, tc_edge);
  }
  OLSR_FOR_ALL_TC_ENTRIES_END(tc);

  abuf_json_close_array(abuf);
}

static void
ipc_print_hna(struct autobuf *abuf)
{
  struct hna_entry *tmp_hna;
  struct hna_net *tmp_net;
  struct ipaddr_str buf, mainaddrbuf;

  abuf_json_open_array(abuf, "hna");

  OLSR_FOR_ALL_HNA_ENTRIES(tmp_hna) {

    /* Check all networks */
    for (tmp_net = tmp_hna->networks.next; tmp_net != &tmp_hna->networks; tmp_net = tmp_net->next) {
      uint32_t vt = tmp_net->hna_net_timer != NULL ? (tmp_net->hna_net_timer->timer_clock - now_times) : 0;
      int diff = (int)(vt);
      abuf_json_open_array_entry(abuf);
      abuf_json_string(abuf, "destination",
                       olsr_ip_to_string(&buf, &tmp_net->hna_prefix.prefix)),
      abuf_json_int(abuf, "genmask", tmp_net->hna_prefix.prefix_len);
      abuf_json_string(abuf, "gateway",
                       olsr_ip_to_string(&mainaddrbuf, &tmp_hna->A_gateway_addr));
      abuf_json_int(abuf, "validityTime", diff);
      abuf_json_close_array_entry(abuf);
    }
  }
  OLSR_FOR_ALL_HNA_ENTRIES_END(tmp_hna);

  abuf_json_close_array(abuf);
}

static void
ipc_print_mid(struct autobuf *abuf)
{
  int idx;
  struct mid_entry *entry;
  struct mid_address *alias;

  abuf_json_open_array(abuf, "mid");

  /* MID */
  for (idx = 0; idx < HASHSIZE; idx++) {
    entry = mid_set[idx].next;

    while (entry != &mid_set[idx]) {
      struct ipaddr_str buf, buf2;
      abuf_json_open_array_entry(abuf);
      abuf_json_string(abuf, "ipAddress",
                       olsr_ip_to_string(&buf, &entry->main_addr));

      abuf_json_open_array(abuf, "aliases");
      alias = entry->aliases;
      while (alias) {
        uint32_t vt = alias->vtime - now_times;
        int diff = (int)(vt);

        abuf_json_open_array_entry(abuf);
        abuf_json_string(abuf, "ipAddress",
                         olsr_ip_to_string(&buf2, &alias->alias));
        abuf_json_int(abuf, "validityTime", diff);
        abuf_json_close_array_entry(abuf);

        alias = alias->next_alias;
      }
      abuf_json_close_array(abuf); // aliases
      abuf_json_close_array_entry(abuf);
      entry = entry->next;
    }
  }
  abuf_json_close_array(abuf); // mid
}

static void
ipc_print_gateways(struct autobuf *abuf)
{
#ifndef __linux__
  abuf_json_string(abuf, "error", "Gateway mode is only supported in Linux");
#else /* __linux__ */

  struct ipaddr_str buf;
  struct gateway_entry *gw;

  abuf_json_open_array(abuf, "gateways");
  OLSR_FOR_ALL_GATEWAY_ENTRIES(gw) {
    const char *v4 = "", *v6 = "";
    bool autoV4 = false, autoV6 = false;
    const char *ipType = "";
    struct tc_entry *tc;

    if ((tc = olsr_lookup_tc_entry(&gw->originator)) == NULL) {
      continue;
    }

    if (gw == olsr_get_inet_gateway(false)) {
      v4 = "s";
    } else if (gw->ipv4 && (olsr_cnf->ip_version == AF_INET || olsr_cnf->use_niit)
               && (olsr_cnf->smart_gw_allow_nat || !gw->ipv4nat)) {
      v4 = "u";
    }

    if (gw == olsr_get_inet_gateway(true)) {
      v6 = "s";
    } else if (gw->ipv6 && olsr_cnf->ip_version == AF_INET6) {
      v6 = "u";
    }

    abuf_json_open_array_entry(abuf);
    if (gw->ipv4) {
      ipType = "ipv4";
      abuf_json_string(abuf, "ipv4Status", v4);
    } else if (gw->ipv6) {
      ipType = "ipv6";
      abuf_json_string(abuf, "ipv6Status", v6);
    }
    abuf_json_string(abuf, "ipType", ipType);
    abuf_json_boolean(abuf, "ipv4", gw->ipv4);
    abuf_json_boolean(abuf, "ipv4Nat", gw->ipv4nat);
    abuf_json_boolean(abuf, "ipv6", gw->ipv6);
    abuf_json_boolean(abuf, "autoIpv4", autoV4);
    abuf_json_boolean(abuf, "autoIpv6", autoV6);
    abuf_json_string(abuf, "ipAddress",
                     olsr_ip_to_string(&buf, &gw->originator));
    if (tc->path_cost >= ROUTE_COST_BROKEN)
      abuf_json_int(abuf, "tcPathCost", ROUTE_COST_BROKEN);
    else
      abuf_json_int(abuf, "tcPathCost", tc->path_cost);
    abuf_json_int(abuf, "hopCount", tc->hops);
    abuf_json_int(abuf, "uplinkSpeed", gw->uplink);
    abuf_json_int(abuf, "downlinkSpeed", gw->downlink);
    if(gw->external_prefix.prefix_len == 0)
      abuf_json_string(abuf, "externalPrefix",
                       olsr_ip_prefix_to_string(&gw->external_prefix));
    abuf_json_close_array_entry(abuf);
  }
  OLSR_FOR_ALL_GATEWAY_ENTRIES_END(gw)
  abuf_json_close_array(abuf);
#endif /* __linux__ */
}


static void
ipc_print_plugins(struct autobuf *abuf)
{
  struct plugin_entry *pentry;
  struct plugin_param *pparam;
  abuf_json_open_array(abuf, "plugins");
  if (olsr_cnf->plugins)
    for (pentry = olsr_cnf->plugins; pentry; pentry = pentry->next) {
      abuf_json_open_array_entry(abuf);
      abuf_json_string(abuf, "plugin", pentry->name);
      for (pparam = pentry->params; pparam; pparam = pparam->next) {
        int i, keylen = strlen(pparam->key);
        char key[keylen + 1];
        long value;
        char valueTest[256];
        strcpy(key, pparam->key);
        for (i = 0; i < keylen; i++)
          key[i] = tolower(key[i]);

        // test if a int/long and set as such in JSON
        value = atol(pparam->value);
        snprintf(valueTest, 255, "%li", value);
        if (strcmp(valueTest, pparam->value) == 0)
          abuf_json_int(abuf, key, value);
        else
          abuf_json_string(abuf, key, pparam->value);
      }
      abuf_json_close_array_entry(abuf);
    }
  abuf_json_close_array(abuf);
}


static void
ipc_print_config(struct autobuf *abuf)
{
  struct ip_prefix_list *hna;
  struct ipaddr_str buf, mainaddrbuf;
  struct ip_prefix_list *ipcn;
  struct olsr_lq_mult *mult;
  char ipv6_buf[INET6_ADDRSTRLEN];                  /* buffer for IPv6 inet_htop */

  abuf_json_open_object(abuf, "config");

  abuf_json_int(abuf, "olsrPort", olsr_cnf->olsrport);
  abuf_json_int(abuf, "debugLevel", olsr_cnf->debug_level);
  abuf_json_boolean(abuf, "noFork", olsr_cnf->no_fork);
  abuf_json_boolean(abuf, "hostEmulation", olsr_cnf->host_emul);
  abuf_json_int(abuf, "ipVersion", olsr_cnf->ip_version);
  abuf_json_boolean(abuf, "allowNoInterfaces", olsr_cnf->allow_no_interfaces);
  abuf_json_int(abuf, "typeOfService", olsr_cnf->tos);
  abuf_json_int(abuf, "rtProto", olsr_cnf->rt_proto);
  abuf_json_int(abuf, "rtTable", olsr_cnf->rt_table);
  abuf_json_int(abuf, "rtTableDefault", olsr_cnf->rt_table_default);
  abuf_json_int(abuf, "rtTableTunnel", olsr_cnf->rt_table_tunnel);
  abuf_json_int(abuf, "rtTablePriority", olsr_cnf->rt_table_pri);
  abuf_json_int(abuf, "rtTableTunnelPriority", olsr_cnf->rt_table_tunnel_pri);
  abuf_json_int(abuf, "rtTableDefauiltOlsrPriority", olsr_cnf->rt_table_defaultolsr_pri);
  abuf_json_int(abuf, "rtTableDefaultPriority", olsr_cnf->rt_table_default_pri);
  abuf_json_int(abuf, "willingness", olsr_cnf->willingness);
  abuf_json_boolean(abuf, "willingnessAuto", olsr_cnf->willingness_auto);

  abuf_json_int(abuf, "brokenLinkCost", LINK_COST_BROKEN);
  abuf_json_int(abuf, "brokenRouteCost", ROUTE_COST_BROKEN);

  abuf_json_string(abuf, "fibMetrics", FIB_METRIC_TXT[olsr_cnf->fib_metric]);

  abuf_json_string(abuf, "defaultIpv6Multicast",
                   inet_ntop(AF_INET6, &olsr_cnf->interface_defaults->ipv6_multicast.v6,
                             ipv6_buf, sizeof(ipv6_buf)));
  if (olsr_cnf->interface_defaults->ipv4_multicast.v4.s_addr)
    abuf_json_string(abuf, "defaultIpv4Broadcast",
                     inet_ntoa(olsr_cnf->interface_defaults->ipv4_multicast.v4));
  else
    abuf_json_string(abuf, "defaultIpv4Broadcast", "auto");

  if (olsr_cnf->interface_defaults->mode==IF_MODE_ETHER)
    abuf_json_string(abuf, "defaultInterfaceMode", "ether");
  else
    abuf_json_string(abuf, "defaultInterfaceMode", "mesh");

  abuf_json_float(abuf, "defaultHelloEmissionInterval",
                  olsr_cnf->interface_defaults->hello_params.emission_interval);
  abuf_json_float(abuf, "defaultHelloValidityTime",
                  olsr_cnf->interface_defaults->hello_params.validity_time);
  abuf_json_float(abuf, "defaultTcEmissionInterval",
                  olsr_cnf->interface_defaults->tc_params.emission_interval);
  abuf_json_float(abuf, "defaultTcValidityTime",
                  olsr_cnf->interface_defaults->tc_params.validity_time);
  abuf_json_float(abuf, "defaultMidEmissionInterval",
                  olsr_cnf->interface_defaults->mid_params.emission_interval);
  abuf_json_float(abuf, "defaultMidValidityTime",
                  olsr_cnf->interface_defaults->mid_params.validity_time);
  abuf_json_float(abuf, "defaultHnaEmissionInterval",
                  olsr_cnf->interface_defaults->hna_params.emission_interval);
  abuf_json_float(abuf, "defaultHnaValidityTime",
                  olsr_cnf->interface_defaults->hna_params.validity_time);
  abuf_json_boolean(abuf, "defaultAutoDetectChanges",
                    olsr_cnf->interface_defaults->autodetect_chg);

  abuf_json_open_array(abuf, "defaultLinkQualityMultipliers");
  for (mult = olsr_cnf->interface_defaults->lq_mult; mult != NULL; mult = mult->next) {
    abuf_json_open_array_entry(abuf);
    abuf_json_string(abuf, "route",
                     inet_ntop(olsr_cnf->ip_version, &mult->addr, ipv6_buf, sizeof(ipv6_buf)));
    abuf_json_float(abuf, "multiplier", mult->value / 65535.0);
    abuf_json_close_array_entry(abuf);
  }
  abuf_json_close_array(abuf);

  abuf_json_open_array(abuf, "hna");
  for (hna = olsr_cnf->hna_entries; hna != NULL; hna = hna->next) {
    abuf_json_open_array_entry(abuf);
    abuf_json_string(abuf, "destination",
                     olsr_ip_to_string(&buf, &hna->net.prefix));
    abuf_json_int(abuf, "genmask", hna->net.prefix_len);
    abuf_json_string(abuf, "gateway",
                     olsr_ip_to_string(&mainaddrbuf, &olsr_cnf->main_addr));
    abuf_json_close_array_entry(abuf);
  }
  abuf_json_close_array(abuf);


  abuf_json_int(abuf, "totalIpcConnectionsAllowed", olsr_cnf->ipc_connections);
  abuf_json_open_array(abuf, "ipcAllowedAddresses");
  if (olsr_cnf->ipc_connections) {
    for (ipcn = olsr_cnf->ipc_nets; ipcn != NULL; ipcn = ipcn->next) {
      abuf_json_open_array_entry(abuf);
      abuf_json_string(abuf, "ipAddress",
                       olsr_ip_to_string(&mainaddrbuf, &ipcn->net.prefix));
      abuf_json_int(abuf, "netmask", ipcn->net.prefix_len);
      abuf_json_close_array_entry(abuf);
    }
  }
  abuf_json_close_array(abuf);

  // keep all time in ms, so convert these two, which are in seconds
  abuf_json_int(abuf, "pollRate", olsr_cnf->pollrate * 1000);
  abuf_json_int(abuf, "nicChangePollInterval", olsr_cnf->nic_chgs_pollrate * 1000);
  abuf_json_boolean(abuf, "clearScreen", olsr_cnf->clear_screen);
  abuf_json_int(abuf, "tcRedundancy", olsr_cnf->tc_redundancy);
  abuf_json_int(abuf, "mprCoverage", olsr_cnf->mpr_coverage);

  if (olsr_cnf->lq_level == 0) {
    abuf_json_boolean(abuf, "useHysteresis", olsr_cnf->use_hysteresis);
    if (olsr_cnf->use_hysteresis) {
      abuf_json_float(abuf, "hysteresisScaling", olsr_cnf->hysteresis_param.scaling);
      abuf_json_float(abuf, "hysteresisLowThreshold", olsr_cnf->hysteresis_param.thr_low);
      abuf_json_float(abuf, "hysteresisHighThreshold", olsr_cnf->hysteresis_param.thr_high);
    }
  }
  abuf_json_int(abuf, "linkQualityLevel", olsr_cnf->lq_level);
  abuf_json_float(abuf, "linkQualityAging", olsr_cnf->lq_aging);
  abuf_json_boolean(abuf, "linkQualityFisheye", olsr_cnf->lq_fish);
  abuf_json_string(abuf, "linkQualityAlgorithm", olsr_cnf->lq_algorithm);
  // keep all time in ms, so convert this from seconds
  abuf_json_int(abuf, "minTcValidTime", olsr_cnf->min_tc_vtime * 1000);
  abuf_json_boolean(abuf, "setIpForward", olsr_cnf->set_ip_forward);
  abuf_json_string(abuf, "lockFile", olsr_cnf->lock_file);
  abuf_json_boolean(abuf, "useNiit", olsr_cnf->use_niit);

#ifdef __linux__
  abuf_json_boolean(abuf, "smartGateway", olsr_cnf->smart_gw_active);
  if (olsr_cnf->smart_gw_active) {
    abuf_json_boolean(abuf, "smartGatewayAlwaysRemoveServerTunnel", olsr_cnf->smart_gw_always_remove_server_tunnel);
    abuf_json_int(abuf, "smartGatewayUseCount", olsr_cnf->smart_gw_use_count);
    abuf_json_string(abuf, "smartGatewayPolicyRoutingScript", olsr_cnf->smart_gw_policyrouting_script);
    {
      struct autobuf egressbuf;
      struct sgw_egress_if * egressif = olsr_cnf->smart_gw_egress_interfaces;

      abuf_init(&egressbuf, (olsr_cnf->smart_gw_egress_interfaces_count * IFNAMSIZ) /* interface names */
          + (olsr_cnf->smart_gw_egress_interfaces_count - 1) /* commas */);
      while (egressif) {
        if (egressbuf.len) {
          abuf_puts(&egressbuf, ",");
        }
        abuf_appendf(&egressbuf, "%s", egressif->name);
        egressif = egressif->next;
      }
      abuf_json_string(abuf, "smartGatewayEgressInterfaces", egressbuf.buf);
      abuf_free(&egressbuf);
    }
    abuf_json_int(abuf, "smartGatewayTablesOffset", olsr_cnf->smart_gw_offset_tables);
    abuf_json_int(abuf, "smartGatewayRulesOffset", olsr_cnf->smart_gw_offset_rules);
    abuf_json_boolean(abuf, "smartGatewayAllowNat", olsr_cnf->smart_gw_allow_nat);
    abuf_json_boolean(abuf, "smartGatewayUplinkNat", olsr_cnf->smart_gw_uplink_nat);
    abuf_json_int(abuf, "smartGatewayPeriod", olsr_cnf->smart_gw_period);
    abuf_json_int(abuf, "smartGatewayStableCount", olsr_cnf->smart_gw_stablecount);
    abuf_json_int(abuf, "smartGatewayThreshold", olsr_cnf->smart_gw_thresh);
    abuf_json_int(abuf, "smartGatewayUplink", olsr_cnf->smart_gw_uplink);
    abuf_json_int(abuf, "smartGatewayDownlink", olsr_cnf->smart_gw_downlink);
    abuf_json_int(abuf, "smartGatewayType", olsr_cnf->smart_gw_type);
    abuf_json_string(abuf, "smartGatewayPrefix",
                     olsr_ip_to_string(&mainaddrbuf, &olsr_cnf->smart_gw_prefix.prefix));
    abuf_json_int(abuf, "smartGatewayPrefixLength", olsr_cnf->smart_gw_prefix.prefix_len);
  }
#endif /* __linux__ */

  abuf_json_string(abuf, "mainIpAddress",
                   olsr_ip_to_string(&mainaddrbuf, &olsr_cnf->main_addr));
  abuf_json_string(abuf, "unicastSourceIpAddress",
                   olsr_ip_to_string(&mainaddrbuf, &olsr_cnf->unicast_src_ip));

  abuf_json_boolean(abuf, "useSourceIpRoutes", olsr_cnf->use_src_ip_routes);

  abuf_json_int(abuf, "maxPrefixLength", olsr_cnf->maxplen);
  abuf_json_int(abuf, "ipSize", olsr_cnf->ipsize);
  abuf_json_boolean(abuf, "deleteInternetGatewaysAtStartup", olsr_cnf->del_gws);
  // keep all time in ms, so convert this from seconds
  abuf_json_int(abuf, "willingnessUpdateInterval", olsr_cnf->will_int * 1000);
  abuf_json_float(abuf, "maxSendMessageJitter", olsr_cnf->max_jitter);
  abuf_json_int(abuf, "exitValue", olsr_cnf->exit_value);
  // keep all time in ms, so convert this from seconds
  abuf_json_int(abuf, "maxTcValidTime", olsr_cnf->max_tc_vtime * 1000);

  abuf_json_int(abuf, "niit4to6InterfaceIndex", olsr_cnf->niit4to6_if_index);
  abuf_json_int(abuf, "niit6to4InterfaceIndex", olsr_cnf->niit6to4_if_index);

  abuf_json_boolean(abuf, "hasIpv4Gateway", olsr_cnf->has_ipv4_gateway);
  abuf_json_boolean(abuf, "hasIpv6Gateway", olsr_cnf->has_ipv6_gateway);

  abuf_json_int(abuf, "ioctlSocket", olsr_cnf->ioctl_s);
#ifdef __linux__
  abuf_json_int(abuf, "routeNetlinkSocket", olsr_cnf->rtnl_s);
  abuf_json_int(abuf, "routeMonitorSocket", olsr_cnf->rt_monitor_socket);
#endif /* __linux__ */

#if defined __FreeBSD__ || defined __FreeBSD_kernel__ || defined __APPLE__ || defined __NetBSD__ || defined __OpenBSD__
  abuf_json_int(abuf, "routeChangeSocket", olsr_cnf->rts);
#endif /* defined __FreeBSD__ || defined __FreeBSD_kernel__ || defined __APPLE__ || defined __NetBSD__ || defined __OpenBSD__ */
  abuf_json_float(abuf, "linkQualityNatThreshold", olsr_cnf->lq_nat_thresh);

  abuf_json_string(abuf, "olsrdVersion", olsrd_version);
  abuf_json_string(abuf, "olsrdBuildDate", build_date);
  abuf_json_string(abuf, "olsrdBuildHost", build_host);

#if defined _WIN32 || defined _WIN64
  abuf_json_string(abuf, "os", "Windows");
#elif defined __gnu_linux__
  abuf_json_string(abuf, "os", "GNU/Linux");
#elif defined __ANDROID__
  abuf_json_string(abuf, "os", "Android");
#elif defined __APPLE__
  abuf_json_string(abuf, "os", "Mac OS X");
#elif defined __NetBSD__
  abuf_json_string(abuf, "os", "NetBSD");
#elif defined __OpenBSD__
  abuf_json_string(abuf, "os", "OpenBSD");
#elif defined __FreeBSD__ || defined __FreeBSD_kernel__
  abuf_json_string(abuf, "os", "FreeBSD");
#else /* OS detection */
  abuf_json_string(abuf, "os", "Undefined");
#endif /* OS detection */

  abuf_json_int(abuf, "startTime", start_time.tv_sec);

  abuf_json_close_object(abuf);
}

static void
ipc_print_interfaces(struct autobuf *abuf)
{
#ifdef __linux__
  int linklen;
  char path[PATH_MAX], linkpath[PATH_MAX];
#endif /* __linux__ */
  char ipv6_buf[INET6_ADDRSTRLEN];                  /* buffer for IPv6 inet_htop */
  struct olsr_lq_mult *mult;
  const struct olsr_if *ifs;
  abuf_json_open_array(abuf, "interfaces");
  for (ifs = olsr_cnf->interfaces; ifs != NULL; ifs = ifs->next) {
    const struct interface *const rifs = ifs->interf;
    abuf_json_open_array_entry(abuf);
    abuf_json_string(abuf, "name", ifs->name);

    abuf_json_open_array(abuf, "linkQualityMultipliers");
    for (mult = ifs->cnf->lq_mult; mult != NULL; mult = mult->next) {
      abuf_json_open_array_entry(abuf);
      abuf_json_string(abuf, "route",
                       inet_ntop(olsr_cnf->ip_version, &mult->addr, ipv6_buf, sizeof(ipv6_buf)));
      abuf_json_float(abuf, "multiplier", mult->value / 65535.0);
      abuf_json_close_array_entry(abuf);
    }
    abuf_json_close_array(abuf);

    if (!rifs) {
      abuf_json_string(abuf, "state", "down");
    } else {
      abuf_json_string(abuf, "state", "up");
      abuf_json_string(abuf, "nameFromKernel", rifs->int_name);
      abuf_json_int(abuf, "interfaceMode", rifs->mode);
      abuf_json_boolean(abuf, "emulatedHostClientInterface", rifs->is_hcif);
      abuf_json_boolean(abuf, "sendTcImmediately", rifs->immediate_send_tc);
      abuf_json_int(abuf, "fishEyeTtlIndex", rifs->ttl_index);
      abuf_json_int(abuf, "olsrForwardingTimeout", rifs->fwdtimer);
      abuf_json_int(abuf, "olsrMessageSequenceNumber", rifs->olsr_seqnum);
      abuf_json_int(abuf, "olsrInterfaceMetric", rifs->int_metric);
      abuf_json_int(abuf, "olsrMTU", rifs->int_mtu);
      abuf_json_int(abuf, "helloEmissionInterval", rifs->hello_etime);
      abuf_json_int(abuf, "helloValidityTime", rifs->valtimes.hello);
      abuf_json_int(abuf, "tcValidityTime", rifs->valtimes.tc);
      abuf_json_int(abuf, "midValidityTime", rifs->valtimes.mid);
      abuf_json_int(abuf, "hnaValidityTime", rifs->valtimes.hna);
      abuf_json_boolean(abuf, "wireless", rifs->is_wireless);

#ifdef __linux__
      abuf_json_boolean(abuf, "icmpRedirect", rifs->nic_state.redirect);
      abuf_json_boolean(abuf, "spoofFilter", rifs->nic_state.spoof);
#endif /* __linux__ */

      if (olsr_cnf->ip_version == AF_INET) {
        struct ipaddr_str addrbuf, maskbuf, bcastbuf;
        abuf_json_string(abuf, "ipv4Address",
                             ip4_to_string(&addrbuf, rifs->int_addr.sin_addr));
        abuf_json_string(abuf, "netmask",
                             ip4_to_string(&maskbuf, rifs->int_netmask.sin_addr));
        abuf_json_string(abuf, "broadcast",
                             ip4_to_string(&bcastbuf, rifs->int_broadaddr.sin_addr));
      } else {
        struct ipaddr_str addrbuf, maskbuf;
        abuf_json_string(abuf, "ipv6Address",
                             ip6_to_string(&addrbuf, &rifs->int6_addr.sin6_addr));
        abuf_json_string(abuf, "multicast",
                             ip6_to_string(&maskbuf, &rifs->int6_multaddr.sin6_addr));
      }
    }
#ifdef __linux__
    snprintf(path, PATH_MAX, "/sys/class/net/%s/device/driver/module", ifs->name);
    linklen = readlink(path, linkpath, PATH_MAX - 1);
    if (linklen > 1) {
      linkpath[linklen] = '\0';
      abuf_json_string(abuf, "kernelModule", basename(linkpath));
    }

    abuf_json_sys_class_net(abuf, "addressLength", ifs->name, "addr_len");
    abuf_json_sys_class_net(abuf, "carrier", ifs->name, "carrier");
    abuf_json_sys_class_net(abuf, "dormant", ifs->name, "dormant");
    abuf_json_sys_class_net(abuf, "features", ifs->name, "features");
    abuf_json_sys_class_net(abuf, "flags", ifs->name, "flags");
    abuf_json_sys_class_net(abuf, "linkMode", ifs->name, "link_mode");
    abuf_json_sys_class_net(abuf, "macAddress", ifs->name, "address");
    abuf_json_sys_class_net(abuf, "ethernetMTU", ifs->name, "mtu");
    abuf_json_sys_class_net(abuf, "operationalState", ifs->name, "operstate");
    abuf_json_sys_class_net(abuf, "txQueueLength", ifs->name, "tx_queue_len");
    abuf_json_sys_class_net(abuf, "collisions", ifs->name, "statistics/collisions");
    abuf_json_sys_class_net(abuf, "multicastPackets", ifs->name, "statistics/multicast");
    abuf_json_sys_class_net(abuf, "rxBytes", ifs->name, "statistics/rx_bytes");
    abuf_json_sys_class_net(abuf, "rxCompressed", ifs->name, "statistics/rx_compressed");
    abuf_json_sys_class_net(abuf, "rxCrcErrors", ifs->name, "statistics/rx_crc_errors");
    abuf_json_sys_class_net(abuf, "rxDropped", ifs->name, "statistics/rx_dropped");
    abuf_json_sys_class_net(abuf, "rxErrors", ifs->name, "statistics/rx_errors");
    abuf_json_sys_class_net(abuf, "rxFifoErrors", ifs->name, "statistics/rx_fifo_errors");
    abuf_json_sys_class_net(abuf, "rxFrameErrors", ifs->name, "statistics/rx_frame_errors");
    abuf_json_sys_class_net(abuf, "rxLengthErrors", ifs->name, "statistics/rx_length_errors");
    abuf_json_sys_class_net(abuf, "rxMissedErrors", ifs->name, "statistics/rx_missed_errors");
    abuf_json_sys_class_net(abuf, "rxOverErrors", ifs->name, "statistics/rx_over_errors");
    abuf_json_sys_class_net(abuf, "rxPackets", ifs->name, "statistics/rx_packets");
    abuf_json_sys_class_net(abuf, "txAbortedErrors", ifs->name, "statistics/tx_aborted_errors");
    abuf_json_sys_class_net(abuf, "txBytes", ifs->name, "statistics/tx_bytes");
    abuf_json_sys_class_net(abuf, "txCarrierErrors", ifs->name, "statistics/tx_carrier_errors");
    abuf_json_sys_class_net(abuf, "txCompressed", ifs->name, "statistics/tx_compressed");
    abuf_json_sys_class_net(abuf, "txDropped", ifs->name, "statistics/tx_dropped");
    abuf_json_sys_class_net(abuf, "txErrors", ifs->name, "statistics/tx_errors");
    abuf_json_sys_class_net(abuf, "txFifoErrors", ifs->name, "statistics/tx_fifo_errors");
    abuf_json_sys_class_net(abuf, "txHeartbeatErrors", ifs->name, "statistics/tx_heartbeat_errors");
    abuf_json_sys_class_net(abuf, "txPackets", ifs->name, "statistics/tx_packets");
    abuf_json_sys_class_net(abuf, "txWindowErrors", ifs->name, "statistics/tx_window_errors");
    abuf_json_sys_class_net(abuf, "beaconing", ifs->name, "wireless/beacon");
    abuf_json_sys_class_net(abuf, "encryptionKey", ifs->name, "wireless/crypt");
    abuf_json_sys_class_net(abuf, "fragmentationThreshold", ifs->name, "wireless/fragment");
    abuf_json_sys_class_net(abuf, "signalLevel", ifs->name, "wireless/level");
    abuf_json_sys_class_net(abuf, "linkQuality", ifs->name, "wireless/link");
    abuf_json_sys_class_net(abuf, "misc", ifs->name, "wireless/misc");
    abuf_json_sys_class_net(abuf, "noiseLevel", ifs->name, "wireless/noise");
    abuf_json_sys_class_net(abuf, "nwid", ifs->name, "wireless/nwid");
    abuf_json_sys_class_net(abuf, "wirelessRetries", ifs->name, "wireless/retries");
    abuf_json_sys_class_net(abuf, "wirelessStatus", ifs->name, "wireless/status");
#endif /* __linux__ */
    abuf_json_close_array_entry(abuf);
  }
  abuf_json_close_array(abuf);
}


static void
ipc_print_olsrd_conf(struct autobuf *abuf)
{
  olsrd_write_cnf_autobuf(abuf, olsr_cnf);
}


static void
jsoninfo_write_data(void *foo __attribute__ ((unused)))
{
  fd_set set;
  int result, i, j, max;
  struct timeval tv;

  FD_ZERO(&set);
  max = 0;
  for (i=0; i<outbuffer_count; i++) {
    /* And we cast here since we get a warning on Win32 */
    FD_SET((unsigned int)(outbuffer_socket[i]), &set);

    if (outbuffer_socket[i] > max) {
      max = outbuffer_socket[i];
    }
  }

  tv.tv_sec = 0;
  tv.tv_usec = 0;

  result = select(max + 1, NULL, &set, NULL, &tv);
  if (result <= 0) {
    return;
  }

  for (i=0; i<outbuffer_count; i++) {
    if (FD_ISSET(outbuffer_socket[i], &set)) {
      result = send(outbuffer_socket[i],
                    outbuffer[i] + outbuffer_written[i],
                    outbuffer_size[i] - outbuffer_written[i],
                    0);
      if (result > 0) {
        outbuffer_written[i] += result;
      }

      if (result <= 0 || outbuffer_written[i] == outbuffer_size[i]) {
        /* close this socket and cleanup*/
        close(outbuffer_socket[i]);
        free (outbuffer[i]);

        for (j=i+1; j<outbuffer_count; j++) {
          outbuffer[j-1] = outbuffer[j];
          outbuffer_size[j-1] = outbuffer_size[j];
          outbuffer_socket[j-1] = outbuffer_socket[j];
          outbuffer_written[j-1] = outbuffer_written[j];
        }
        outbuffer_count--;
      }
    }
  }
  if (outbuffer_count == 0) {
    olsr_stop_timer(writetimer_entry);
  }
}

static void
send_info(unsigned int send_what, int the_socket)
{
  struct autobuf abuf;
  size_t header_len = 0;
  char header_buf[MAX_HTTPHEADER_SIZE];
  const char *content_type = "application/json";

  /* global variables for tracking when to put a comma in for JSON */
  entrynumber[0] = 0;
  currentjsondepth = 0;

  abuf_init(&abuf, 32768);

 // only add if outputing JSON
  if (send_what & SIW_ALL) abuf_puts(&abuf, "{");

  if ((send_what & SIW_LINKS) == SIW_LINKS) ipc_print_links(&abuf);
  if ((send_what & SIW_NEIGHBORS) == SIW_NEIGHBORS) ipc_print_neighbors(&abuf);
  if ((send_what & SIW_TOPOLOGY) == SIW_TOPOLOGY) ipc_print_topology(&abuf);
  if ((send_what & SIW_HNA) == SIW_HNA) ipc_print_hna(&abuf);
  if ((send_what & SIW_MID) == SIW_MID) ipc_print_mid(&abuf);
  if ((send_what & SIW_ROUTES) == SIW_ROUTES) ipc_print_routes(&abuf);
  if ((send_what & SIW_GATEWAYS) == SIW_GATEWAYS) ipc_print_gateways(&abuf);
  if ((send_what & SIW_INTERFACES) == SIW_INTERFACES) ipc_print_interfaces(&abuf);
  if ((send_what & SIW_CONFIG) == SIW_CONFIG) {
    if (send_what != SIW_CONFIG) abuf_puts(&abuf, ",");
    ipc_print_config(&abuf);
  }
  if ((send_what & SIW_PLUGINS) == SIW_PLUGINS) ipc_print_plugins(&abuf);

  /* output overarching meta data last so we can use abuf_json_* functions, they add a comma at the beginning */
  if (send_what & SIW_ALL) {
    abuf_json_int(&abuf, "systemTime", time(NULL));
    abuf_json_int(&abuf, "timeSinceStartup", now_times);
    if (*uuid != 0)
      abuf_json_string(&abuf, "uuid", uuid);
    abuf_puts(&abuf, "}\n");
  }

  /* this outputs the olsrd.conf text directly, not JSON */
  if ((send_what & SIW_OLSRD_CONF) == SIW_OLSRD_CONF) {
    ipc_print_olsrd_conf(&abuf);
  }

  if(http_headers) {
    header_len = build_http_header(HTTP_200, content_type, abuf.len, header_buf, sizeof(header_buf));
  }

  outbuffer[outbuffer_count] = olsr_malloc(header_len + abuf.len, "json output buffer");
  outbuffer_size[outbuffer_count] = header_len + abuf.len;
  outbuffer_written[outbuffer_count] = 0;
  outbuffer_socket[outbuffer_count] = the_socket;

  memcpy(outbuffer[outbuffer_count], header_buf, header_len);
  memcpy((outbuffer[outbuffer_count]) + header_len, abuf.buf, abuf.len);
  outbuffer_count++;

  if (outbuffer_count == 1) {
    writetimer_entry = olsr_start_timer(100,
                                        0,
                                        OLSR_TIMER_PERIODIC,
                                        &jsoninfo_write_data,
                                        NULL,
                                        0);
  }

  abuf_free(&abuf);
}

static size_t
build_http_header(const char *status, const char *mime, uint32_t msgsize,
  char *buf, uint32_t bufsize)
{
  time_t currtime;
  size_t size;

  size = snprintf(buf, bufsize, "%s\r\n", status);

  /* Date */
  time(&currtime);
  size += strftime(&buf[size], bufsize - size, "Date: %a, %d %b %Y %H:%M:%S GMT\r\n", localtime(&currtime));

  /* Server version */
  size += snprintf(&buf[size], bufsize - size, "Server: OLSRD JSONInfo plugin\r\n");

  /* connection-type */
  size += snprintf(&buf[size], bufsize - size, "Connection: closed\r\n");

  /* MIME type */
  if(mime != NULL) {
    size += snprintf(&buf[size], bufsize - size, "Content-type: %s\r\n", mime);
  }

  /* CORS data */
  /**No needs to be strict here, access control is based on source IP*/
  size += snprintf(&buf[size], bufsize - size, "Access-Control-Allow-Origin: *\r\n");
  size += snprintf(&buf[size], bufsize - size, "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n");
  size += snprintf(&buf[size], bufsize - size, "Access-Control-Allow-Headers: Accept, Origin, X-Requested-With\r\n");
  size += snprintf(&buf[size], bufsize - size, "Access-Control-Max-Age: 1728000\r\n");

  /* Content length */
  if (msgsize > 0) {
    size += snprintf(&buf[size], bufsize - size, "Content-length: %i\r\n", msgsize);
  }

  /* Cache-control
   * No caching dynamic pages
   */
  size += snprintf(&buf[size], bufsize - size, "Cache-Control: no-cache\r\n");

  /* End header */
  size += snprintf(&buf[size], bufsize - size, "\r\n");

  return size;
}

/*
 * Local Variables:
 * mode: c
 * style: linux
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
