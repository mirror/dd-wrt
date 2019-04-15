/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
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

#ifndef _OLSRD_LIB_INFO_INFO_TYPES_H_
#define _OLSRD_LIB_INFO_INFO_TYPES_H_

#include <stdbool.h>
#include <assert.h>
#include <netinet/in.h>

#include "common/autobuf.h"

#define CACHE_TIMEOUT_DEFAULT 1000
#define REQUEST_TIMEOUT_DEFAULT 20

typedef struct {
    union olsr_ip_addr accept_ip;
    union olsr_ip_addr listen_ip;
    int ipc_port;
    bool http_headers;
    bool allow_localhost;
    bool ipv6_only;
    long cache_timeout;
    long request_timeout;
    long request_timeout_sec; /* derived */
    long request_timeout_usec; /* derived */
} info_plugin_config_t;

#define INFO_PLUGIN_CONFIG_PLUGIN_PARAMETERS(config) \
  { .name = "port", .set_plugin_parameter = &set_plugin_port, .data = &config.ipc_port }, \
  { .name = "accept", .set_plugin_parameter = &set_plugin_ipaddress, .data = &config.accept_ip }, \
  { .name = "listen", .set_plugin_parameter = &set_plugin_ipaddress, .data = &config.listen_ip }, \
  { .name = "httpheaders", .set_plugin_parameter = &set_plugin_boolean, .data = &config.http_headers }, \
  { .name = "allowlocalhost", .set_plugin_parameter = &set_plugin_boolean, .data = &config.allow_localhost }, \
  { .name = "ipv6only", .set_plugin_parameter = &set_plugin_boolean, .data = &config.ipv6_only },\
  { .name = "cachetimeout", .set_plugin_parameter = &set_plugin_long, .data = &config.cache_timeout },\
  { .name = "requesttimeout", .set_plugin_parameter = &set_plugin_long, .data = &config.request_timeout }

/* these provide all of the runtime status info */
#define SIW_NEIGHBORS                    (1ULL <<  0)
#define SIW_LINKS                        (1ULL <<  1)
#define SIW_ROUTES                       (1ULL <<  2)
#define SIW_HNA                          (1ULL <<  3)
#define SIW_MID                          (1ULL <<  4)
#define SIW_TOPOLOGY                     (1ULL <<  5)
#define SIW_GATEWAYS                     (1ULL <<  6)
#define SIW_INTERFACES                   (1ULL <<  7)
#define SIW_2HOP                         (1ULL <<  8)
#define SIW_SGW                          (1ULL <<  9)
#define SIW_PUD_POSITION                 (1ULL << 10)
#define SIW_RUNTIME_ALL                  (SIW_NEIGHBORS | SIW_LINKS | SIW_ROUTES | SIW_HNA | SIW_MID | SIW_TOPOLOGY | SIW_GATEWAYS | SIW_INTERFACES | SIW_2HOP | SIW_SGW | SIW_PUD_POSITION)
#define SIW_NEIGHBORS_FREIFUNK           (SIW_NEIGHBORS | SIW_LINKS) /* special */

/* these only change at olsrd startup */
#define SIW_VERSION                      (1ULL << 11)
#define SIW_CONFIG                       (1ULL << 12)
#define SIW_PLUGINS                      (1ULL << 13)
#define SIW_STARTUP_ALL                  (SIW_VERSION | SIW_CONFIG | SIW_PLUGINS)

/* this is everything in normal format */
#define SIW_ALL                          (SIW_RUNTIME_ALL | SIW_STARTUP_ALL)

/* this data is not normal format but olsrd.conf format */
#define SIW_OLSRD_CONF                   (1ULL << 14)

/* netjson */
#define SIW_NETJSON_NETWORK_ROUTES       (1ULL << 15)
#define SIW_NETJSON_NETWORK_GRAPH        (1ULL << 16)
#define SIW_NETJSON_DEVICE_CONFIGURATION (1ULL << 17)
#define SIW_NETJSON_DEVICE_MONITORING    (1ULL << 18)
#define SIW_NETJSON_NETWORK_COLLECTION   (1ULL << 19)
#define SIW_NETJSON                      (SIW_NETJSON_NETWORK_ROUTES | SIW_NETJSON_NETWORK_GRAPH | SIW_NETJSON_DEVICE_CONFIGURATION | SIW_NETJSON_DEVICE_MONITORING | SIW_NETJSON_NETWORK_COLLECTION)

/* poprouting */
#define SIW_POPROUTING_HELLO             (1ULL << 20)
#define SIW_POPROUTING_TC                (1ULL << 21)
#define SIW_POPROUTING_HELLO_MULT        (1ULL << 22)
#define SIW_POPROUTING_TC_MULT           (1ULL << 23)
#define SIW_POPROUTING                   (SIW_POPROUTING_HELLO | SIW_POPROUTING_TC | SIW_POPROUTING_HELLO_MULT | SIW_POPROUTING_TC_MULT)

/* everything */
#define SIW_EVERYTHING                   ((SIW_POPROUTING_TC_MULT << 1) - 1)

/* command prefixes */
#define SIW_PREFIX_HTTP                  "/http"
#define SIW_PREFIX_HTTP_LEN              (sizeof(SIW_PREFIX_HTTP) - 1)
#define SIW_PREFIX_PLAIN                 "/plain"
#define SIW_PREFIX_PLAIN_LEN             (sizeof(SIW_PREFIX_PLAIN) - 1)

typedef void (*init_plugin)(const char *plugin_name);
typedef unsigned long long (*supported_commands_mask_func)(void);
typedef bool (*command_matcher)(const char *str, unsigned long long siw);
typedef long (*cache_timeout_func)(info_plugin_config_t *plugin_config, unsigned long long siw);
typedef const char * (*mime_type)(unsigned int send_what);
typedef void (*output_start_end)(struct autobuf *abuf);
typedef void (*printer_error)(struct autobuf *abuf, unsigned int status, const char * req, bool http_headers);
typedef void (*printer_generic)(struct autobuf *abuf);

typedef struct {
    bool supportsCompositeCommands;
    init_plugin init;
    supported_commands_mask_func supported_commands_mask;
    command_matcher is_command;
    cache_timeout_func cache_timeout;
    mime_type determine_mime_type;
    output_start_end output_start;
    output_start_end output_end;
    printer_error output_error;
    printer_generic neighbors;
    printer_generic links;
    printer_generic routes;
    printer_generic topology;
    printer_generic hna;
    printer_generic mid;
    printer_generic gateways;
    printer_generic sgw;
    printer_generic pudPosition;

    printer_generic version;
    printer_generic olsrd_conf;
    printer_generic interfaces;
    printer_generic twohop;
    printer_generic config;
    printer_generic plugins;

    printer_generic networkRoutes;
    printer_generic networkGraph;
    printer_generic deviceConfiguration;
    printer_generic deviceMonitoring;
    printer_generic networkCollection;

    printer_generic tcTimer;
    printer_generic helloTimer;
    printer_generic tcTimerMult;
    printer_generic helloTimerMult;
} info_plugin_functions_t;

struct info_cache_entry_t {
    long long timestamp;
    struct autobuf buf;
};

struct info_cache_t {
    struct info_cache_entry_t neighbors;
    struct info_cache_entry_t links;
    struct info_cache_entry_t routes;
    struct info_cache_entry_t hna;
    struct info_cache_entry_t mid;
    struct info_cache_entry_t topology;
    struct info_cache_entry_t gateways;
    struct info_cache_entry_t interfaces;
    struct info_cache_entry_t twohop;
    struct info_cache_entry_t sgw;
    struct info_cache_entry_t pudPosition;

    struct info_cache_entry_t version;
    struct info_cache_entry_t config;
    struct info_cache_entry_t plugins;

    struct info_cache_entry_t networkRoutes;
    struct info_cache_entry_t networkGraph;
    struct info_cache_entry_t deviceConfiguration;
    struct info_cache_entry_t deviceMonitoring;
    struct info_cache_entry_t networkCollection;
};

static INLINE struct info_cache_entry_t * info_cache_get_entry(struct info_cache_t * cache, unsigned long long siw) {
  struct info_cache_entry_t * r = NULL;

  if (!cache) {
    return r;
  }

  switch (siw) {
    case SIW_NEIGHBORS:
      r = &cache->neighbors;
      break;

    case SIW_LINKS:
      r = &cache->links;
      break;

    case SIW_ROUTES:
      r = &cache->routes;
      break;

    case SIW_HNA:
      r = &cache->hna;
      break;

    case SIW_MID:
      r = &cache->mid;
      break;

    case SIW_TOPOLOGY:
      r = &cache->topology;
      break;

    case SIW_GATEWAYS:
      r = &cache->gateways;
      break;

    case SIW_INTERFACES:
      r = &cache->interfaces;
      break;

    case SIW_2HOP:
      r = &cache->twohop;
      break;

    case SIW_SGW:
      r = &cache->sgw;
      break;

    case SIW_PUD_POSITION:
      r = &cache->pudPosition;
      break;

    case SIW_VERSION:
      r = &cache->version;
      break;

    case SIW_CONFIG:
      r = &cache->config;
      break;

    case SIW_PLUGINS:
      r = &cache->plugins;
      break;

    case SIW_NETJSON_NETWORK_ROUTES:
      r = &cache->networkRoutes;
      break;

    case SIW_NETJSON_NETWORK_GRAPH:
      r = &cache->networkGraph;
      break;

    case SIW_NETJSON_DEVICE_CONFIGURATION:
      r = &cache->deviceConfiguration;
      break;

    case SIW_NETJSON_DEVICE_MONITORING:
      r = &cache->deviceMonitoring;
      break;

    case SIW_NETJSON_NETWORK_COLLECTION:
      r = &cache->networkCollection;
      break;

    default:
      /* not cached */
      break;
  }

  return r;
}

static INLINE void info_plugin_config_init(info_plugin_config_t *config, unsigned short port) {
  assert(config);

  if (olsr_cnf->ip_version == AF_INET) {
    config->accept_ip.v4.s_addr = htonl(INADDR_LOOPBACK);
    config->listen_ip.v4.s_addr = htonl(INADDR_ANY);
  } else {
    config->accept_ip.v6 = in6addr_loopback;
    config->listen_ip.v6 = in6addr_any;
  }

  config->ipc_port = port;
  config->http_headers = true;
  config->allow_localhost = false;
  config->ipv6_only = false;
  config->cache_timeout = CACHE_TIMEOUT_DEFAULT;
  config->request_timeout = REQUEST_TIMEOUT_DEFAULT;
}

#endif /* _OLSRD_LIB_INFO_INFO_TYPES_H_ */
