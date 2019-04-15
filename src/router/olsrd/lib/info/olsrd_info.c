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

#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>

#include "olsrd_info.h"
#include "olsr.h"
#include "scheduler.h"
#include "ipcalc.h"
#include "http_headers.h"

#ifdef _WIN32
#define close(x) closesocket(x)
#endif /* _WIN32 */

#define MAX_CLIENTS 8

/*
 * There is the problem that writing to a network socket can block,
 * and the olsrd scheduler does not care about write events.
 *
 * There was a case that olsrd just froze for minutes when people used
 * jsoninfo/txtinfo with large topologies over bad WiFi.
 *
 * This is the solution that was chosen at that time, unwilling to
 * rewrite the whole scheduler:
 * A timer was added and each time it expires each non-empty buffer
 * in this structure will try to write data into a "non-blocking"
 * socket until all data is sent, so that no blocking occurs.
 */
typedef struct {
  int socket[MAX_CLIENTS];
  char *buffer[MAX_CLIENTS];
  size_t size[MAX_CLIENTS];
  size_t written[MAX_CLIENTS];
  int count;
} info_plugin_outbuffer_t;

static const char * name;

static info_plugin_functions_t *functions = NULL;

static info_plugin_config_t *config = NULL;

static int ipc_socket = -1;

static info_plugin_outbuffer_t outbuffer;

static struct timer_entry *writetimer_entry = NULL;

static struct info_cache_t info_cache;

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

static char * skipMultipleSlashes(char * requ, size_t* len) {
  char * r = requ;

  if (!r // null pointer
      || (len && !*len) // zero length
      || (r[0] == '\0') // zero length
      || (r[0] != '/') // does not start with a slash
      || (r[1] != '/')) // does not have another slash
  {
    return r;
  }

  while (r[1] == '/') {
    r++;
    if (len) {
      *len = *len - 1;
    }
  }
  return r;
}

static unsigned long long SIW_ENTRIES_ALL[] = {
//
    SIW_NEIGHBORS,//
    SIW_LINKS, //
    SIW_ROUTES, //
    SIW_HNA, //
    SIW_MID, //
    SIW_TOPOLOGY, //
    SIW_GATEWAYS, //
    SIW_INTERFACES, //
    SIW_2HOP, //
    SIW_SGW, //
    SIW_PUD_POSITION, //
    SIW_RUNTIME_ALL,//
    SIW_NEIGHBORS_FREIFUNK, //
    //
    SIW_VERSION,//
    SIW_CONFIG, //
    SIW_PLUGINS, //
    SIW_STARTUP_ALL, //
    //
    SIW_ALL, //
    //
    SIW_OLSRD_CONF, //
    //
    SIW_NETJSON_NETWORK_ROUTES, //
    SIW_NETJSON_NETWORK_GRAPH, //
    SIW_NETJSON_DEVICE_CONFIGURATION, //
    SIW_NETJSON_DEVICE_MONITORING, //
    SIW_NETJSON_NETWORK_COLLECTION, //
    //
    SIW_POPROUTING_HELLO,
    SIW_POPROUTING_TC, //
    SIW_POPROUTING_HELLO_MULT,
    SIW_POPROUTING_TC_MULT //
    };

long cache_timeout_generic(info_plugin_config_t *plugin_config, unsigned long long siw) {
  long timeout = !plugin_config ? 0 : plugin_config->cache_timeout;
  if (timeout <= 0) {
    return timeout;
  }

  switch (siw) {
    case SIW_NEIGHBORS:
    case SIW_LINKS:
    case SIW_ROUTES:
    case SIW_HNA:
    case SIW_MID:
    case SIW_TOPOLOGY:
    case SIW_GATEWAYS:
    case SIW_INTERFACES:
    case SIW_2HOP:
    case SIW_SGW:
    case SIW_PUD_POSITION:

    case SIW_NETJSON_NETWORK_ROUTES:
    case SIW_NETJSON_NETWORK_GRAPH:
    case SIW_NETJSON_DEVICE_CONFIGURATION:
    case SIW_NETJSON_DEVICE_MONITORING:
    case SIW_NETJSON_NETWORK_COLLECTION:
      return timeout;

    case SIW_VERSION:
    case SIW_CONFIG:
    case SIW_PLUGINS:
      return LONG_MAX;

    default:
      /* not cached */
      return false;
  }
}

static void info_plugin_cache_init(bool init) {
  unsigned int i;

  if (!functions->cache_timeout) {
    return;
  }

  for (i = 0; i < ARRAY_SIZE(SIW_ENTRIES_ALL); ++i) {
    unsigned long long siw = SIW_ENTRIES_ALL[i];
    struct info_cache_entry_t * entry = info_cache_get_entry(&info_cache, siw);
    if (!entry) {
      continue;
    }

    if (init) {
      entry->timestamp = 0;
      abuf_init(&entry->buf, 0);
    } else {
      abuf_free(&entry->buf);
      entry->timestamp = 0;
    }
  }
}

static INLINE void info_plugin_cache_init_entry(struct info_cache_entry_t * entry) {
  if (!entry->buf.buf) {
    entry->timestamp = 0;
    abuf_init(&entry->buf, AUTOBUFCHUNK);
    assert(!entry->timestamp);
    assert(!entry->buf.len);
    assert(entry->buf.size == AUTOBUFCHUNK);
    assert(entry->buf.buf);
  } else {
    assert(entry->timestamp >= 0);
    assert(entry->buf.len >= 0);
    assert(entry->buf.size >= AUTOBUFCHUNK);
    assert(entry->buf.buf);
  }
}

static unsigned int determine_single_action(char *requ) {
  unsigned int i;
  unsigned long long siw_mask = !functions->supported_commands_mask ? SIW_EVERYTHING : functions->supported_commands_mask();

  if (!functions->is_command || !siw_mask)
    return 0;

  for (i = 0; i < ARRAY_SIZE(SIW_ENTRIES_ALL); ++i) {
    unsigned long long siw = SIW_ENTRIES_ALL[i];
    if ((siw & siw_mask) && functions->is_command(requ, siw))
      return siw;
  }

  return 0;
}

static unsigned int determine_action(char *requ) {
  if (!functions->is_command) {
    return 0;
  }

  if (!requ || (requ[0] == '\0')) {
    /* no more text */
    return 0;
  }

  /* requ is guaranteed to be at least 1 character long */

  if (!functions->supportsCompositeCommands) {
    /* no support for composite commands */
    return determine_single_action(requ);
  }

  /* composite commands */

  {
    unsigned int action = 0;

    char * requestSegment = requ;
    while (requestSegment) {
      requestSegment = skipMultipleSlashes(requestSegment, NULL);
      if (requestSegment[0] == '\0') {
        /* there is no more text */
        requestSegment = NULL;
        continue;
      }

      /* there is more text */

      {
        unsigned int r = 0;
        char * requestSegmentTail = strchr(&requestSegment[1], '/');

        if (!requestSegmentTail) {
          /* there isn't another slash, process everything that is left */
          r = determine_single_action(requestSegment);
        } else {
          /* there is another slash, process everything before the slash */
          char savedCharacter = *requestSegmentTail;
          *requestSegmentTail = '\0';
          r = determine_single_action(requestSegment);
          *requestSegmentTail = savedCharacter;
        }

        if (!r) {
          /* not found */
          return 0;
        }

        action |= r;

        /* process everything that is left in the next iteration */
        requestSegment = requestSegmentTail;
      }
    }

    return action;
  }
}

static void send_status_no_retries(const char * req, bool add_headers, int the_socket, unsigned int status) {
  struct autobuf abuf;

  abuf_init(&abuf, AUTOBUFCHUNK);

  if (add_headers) {
    http_header_build_result(status, &abuf);
  } else if (status != INFO_HTTP_OK) {
    if (functions->output_error) {
      functions->output_error(&abuf, status, req, add_headers);
    } else if (status == INFO_HTTP_NOCONTENT) {
      /* wget can't handle output of zero length */
      abuf_puts(&abuf, "\n");
    }
  }

  (void) send(the_socket, abuf.buf, abuf.len,
#ifdef _WIN32
    0
#else
    MSG_DONTWAIT
#endif
  );
  close(the_socket);
  abuf_free(&abuf);
}

static void write_data(void *unused __attribute__((unused))) {
  fd_set set;
  int result, i, max;
  struct timeval tv;

  if (outbuffer.count <= 0) {
    /* exit early if there is nothing to send */
    return;
  }

  FD_ZERO(&set);
  max = 0;
  for (i = 0; i < MAX_CLIENTS; i++) {
    if (outbuffer.socket[i] < 0) {
      continue;
    }

    /* And we cast here since we get a warning on Win32 */
    FD_SET((unsigned int ) (outbuffer.socket[i]), &set);

    if (outbuffer.socket[i] > max) {
      max = outbuffer.socket[i];
    }
  }

  tv.tv_sec = 0;
  tv.tv_usec = 0;

  result = select(max + 1, NULL, &set, NULL, &tv);
  if (result <= 0) {
    /* exit early if any of the sockets is not ready for writing */
    return;
  }

  for (i = 0; i < MAX_CLIENTS; i++) {
    if (outbuffer.socket[i] < 0) {
      continue;
    }

    result = send(outbuffer.socket[i], outbuffer.buffer[i] + outbuffer.written[i], outbuffer.size[i] - outbuffer.written[i],
#ifdef _WIN32
    0
#else
    MSG_DONTWAIT
#endif
    );
    if (result > 0) {
      outbuffer.written[i] += result;
    }

#if EWOULDBLOCK == EAGAIN
    if ((result < 0) && (errno == EAGAIN)) {
#else
    if ((result < 0) && ((errno == EWOULDBLOCK) || (errno == EAGAIN))) {
#endif
      continue;
    }

    if ((result < 0) || (outbuffer.written[i] >= outbuffer.size[i])) {
      /* close this socket and cleanup*/
      close(outbuffer.socket[i]);
      outbuffer.socket[i] = -1;
      free(outbuffer.buffer[i]);
      outbuffer.buffer[i] = NULL;
      outbuffer.size[i] = 0;
      outbuffer.written[i] = 0;

      outbuffer.count--;
    }
  }

  if (!outbuffer.count) {
    olsr_stop_timer(writetimer_entry);
    writetimer_entry = NULL;
  }
}

typedef struct {
  unsigned long long siw;
  printer_generic func;
} SiwLookupTableEntry;

static void send_info_from_table(struct autobuf *abuf, unsigned int send_what, SiwLookupTableEntry *funcs, unsigned int funcsSize, unsigned int *outputLength) {
  unsigned int i;
  unsigned int preLength;
  unsigned int what = send_what;
  cache_timeout_func cache_timeout_f = functions->cache_timeout;

  if (functions->output_start) {
    functions->output_start(abuf);
  }

  preLength = abuf->len;

  for (i = 0; (i < funcsSize) && what; i++) {
    unsigned long long siw = funcs[i].siw;
    if (what & siw) {
      printer_generic func = funcs[i].func;
      if (func) {
        long cache_timeout = 0;
        struct info_cache_entry_t *cache_entry = NULL;

        if (cache_timeout_f) {
          cache_timeout = cache_timeout_f(config, siw);
          cache_entry = (cache_timeout <= 0) ? NULL : info_cache_get_entry(&info_cache, siw);
        }

        if (!cache_entry) {
            func(abuf);
        } else {
          long long now;
          long long age;

          info_plugin_cache_init_entry(cache_entry);

          now = olsr_times();
          age = llabs(now - cache_entry->timestamp);
          if (!cache_entry->timestamp || (age >= cache_timeout)) {
            /* cache is never used before or cache is too old */
            cache_entry->buf.buf[0] = '\0';
            cache_entry->buf.len = 0;
            cache_entry->timestamp = now;
            func(&cache_entry->buf);
          }

          abuf_concat(abuf, &cache_entry->buf);
        }
      }
    }
    what &= ~siw;
  }

  *outputLength = abuf->len - preLength;

  if (functions->output_end) {
    functions->output_end(abuf);
  }
}

static void send_info(const char * req, bool add_headers, unsigned int send_what, int the_socket, unsigned int status) {
  struct autobuf abuf;
  unsigned int outputLength = 0;
  unsigned int send_index = 0;
  bool first_reply = false;

  const char *content_type = functions->determine_mime_type ? functions->determine_mime_type(send_what) : "text/plain; charset=utf-8";
  int contentLengthIndex = 0;
  int headerLength = 0;

  assert(outbuffer.count <= MAX_CLIENTS);

  abuf_init(&abuf, AUTOBUFCHUNK);

  if (add_headers) {
    http_header_build(name, status, content_type, &abuf, &contentLengthIndex);
    headerLength = abuf.len;
  }

  if (status == INFO_HTTP_OK) {
    /* OK */

    // only add if normal format
    if (send_what & SIW_ALL) {
      SiwLookupTableEntry funcs[] = {
        { SIW_NEIGHBORS   , functions->neighbors   }, //
        { SIW_LINKS       , functions->links       }, //
        { SIW_ROUTES      , functions->routes      }, //
        { SIW_HNA         , functions->hna         }, //
        { SIW_MID         , functions->mid         }, //
        { SIW_TOPOLOGY    , functions->topology    }, //
        { SIW_GATEWAYS    , functions->gateways    }, //
        { SIW_INTERFACES  , functions->interfaces  }, //
        { SIW_2HOP        , functions->twohop      }, //
        { SIW_SGW         , functions->sgw         }, //
        { SIW_PUD_POSITION, functions->pudPosition }, //
        //
        { SIW_VERSION     , functions->version     }, //
        { SIW_CONFIG      , functions->config      }, //
        { SIW_PLUGINS     , functions->plugins     } //
      };

      send_info_from_table(&abuf, send_what, funcs, ARRAY_SIZE(funcs), &outputLength);
    } else if (send_what & SIW_NETJSON) {
      SiwLookupTableEntry funcs[] = {
        { SIW_NETJSON_NETWORK_ROUTES      , functions->networkRoutes      }, //
        { SIW_NETJSON_NETWORK_GRAPH       , functions->networkGraph       }, //
        { SIW_NETJSON_DEVICE_CONFIGURATION, functions->deviceConfiguration}, //
        { SIW_NETJSON_DEVICE_MONITORING   , functions->deviceMonitoring   }, //
        { SIW_NETJSON_NETWORK_COLLECTION  , functions->networkCollection  } //
      };

      send_info_from_table(&abuf, send_what, funcs, ARRAY_SIZE(funcs), &outputLength);
    } else if(send_what & SIW_POPROUTING){
      SiwLookupTableEntry funcs[] = {
        { SIW_POPROUTING_TC               , functions->tcTimer           }, //
        { SIW_POPROUTING_HELLO            , functions->helloTimer        }, //
        { SIW_POPROUTING_TC_MULT          , functions->tcTimerMult       }, //
        { SIW_POPROUTING_HELLO_MULT       , functions->helloTimerMult    } //
      };
      
      send_info_from_table(&abuf, send_what, funcs, ARRAY_SIZE(funcs), &outputLength);
    } else if ((send_what & SIW_OLSRD_CONF) && functions->olsrd_conf) {
      /* this outputs the olsrd.conf text directly, not normal format */
      unsigned int preLength = abuf.len;
      functions->olsrd_conf(&abuf);
      outputLength = abuf.len - preLength;
    }

    if (!abuf.len || !outputLength) {
      status = INFO_HTTP_NOCONTENT;
      abuf.buf[0] = '\0';
      abuf.len = 0;
      if (add_headers) {
        http_header_build(name, status, content_type, &abuf, &contentLengthIndex);
        headerLength = abuf.len;
      }
    }
  }

  if (status != INFO_HTTP_OK) {
    if (functions->output_error) {
      functions->output_error(&abuf, status, req, add_headers);
    } else if (status == INFO_HTTP_NOCONTENT) {
      /* wget can't handle output of zero length */
      abuf_puts(&abuf, "\n");
    }
  }

  if (add_headers) {
    http_header_adjust_content_length(&abuf, contentLengthIndex, abuf.len - headerLength);
  }

  /*
   * Determine the last available outbuffer slot.
   * Search from the end towards the start to avoid starvation of
   * older replies that are still in-flight (since the send function
   * iterates from the start towards the end).
   */
  send_index = MAX_CLIENTS;
  while (send_index) {
    send_index--;
    if (!outbuffer.buffer[send_index]) {
      break;
    }
  }
  assert(send_index < MAX_CLIENTS);
  assert(!outbuffer.buffer[send_index]);

  /* avoid a memcpy: just move the abuf.buf pointer and clear abuf */
  outbuffer.buffer[send_index] = abuf.buf;
  outbuffer.size[send_index] = abuf.len;
  outbuffer.written[send_index] = 0;
  outbuffer.socket[send_index] = the_socket;
  abuf.buf = NULL;
  abuf.len = 0;
  abuf.size = 0;

  first_reply = !outbuffer.count;

  outbuffer.count++;

  write_data(NULL);

  if (first_reply && outbuffer.buffer[send_index]) {
    writetimer_entry = olsr_start_timer(10, 0, OLSR_TIMER_PERIODIC, &write_data, NULL, 0);
  }
}

static char * skipLeadingWhitespace(char * requ, size_t *len) {
  if (!requ || !len || !*len) {
    return requ;
  }

  while (isspace(*requ) && (*requ != '\0')) {
    *len = *len - 1;
    requ++;
  }
  return requ;
}

static char * stripTrailingWhitespace(char * requ, size_t *len) {
  if (!requ || !len || !*len) {
    return requ;
  }

  while (isspace(requ[*len - 1]) && (requ[*len - 1] != '\0')) {
    *len = *len - 1;
    requ[*len] = '\0';
  }
  return requ;
}

static char * stripTrailingSlashes(char * requ, size_t *len) {
  if (!requ || !len || !*len) {
    return requ;
  }

  while ((requ[*len - 1] == '/') && (requ[*len - 1] != '\0')) {
    *len = *len - 1;
    requ[*len] = '\0';
  }
  return requ;
}

static char * cutAtFirstEOL(char * requ, size_t *len) {
  char * s = requ;
  size_t l = 0;

  if (!requ || !len || !*len) {
    return requ;
  }

  while (!((*s == '\n') || (*s == '\r')) && (*s != '\0')) {
    s++;
    l++;
  }
  if ((*s == '\n') || (*s == '\r')) {
    *s = '\0';
  }
  *len = l;
  return requ;
}

static char * parseRequest(char * req, size_t *len, bool *add_headers) {
  if (!req || !len || !*len) {
    return req;
  }

  /* HTTP request: GET whitespace URI whitespace HTTP/1.[01] */
  if (*len < (3 + 1 + 1 + 1 + 8) //
      || strncasecmp(req, "GET", 3) //
      || !isspace(req[3]) //
      || !isspace(req[*len - 9]) //
      || strncasecmp(&req[*len - 8], "HTTP/1.", 7) //
      || ((req[*len - 1] != '1') && (req[*len - 1] != '0'))) {
    /* too short or does not start with 'GET ' nor ends with ' HTTP/1.[01]'*/
    *add_headers = false;
    return req;
  }

  /* skip 'GET ' */
  *len = *len - 4;
  req = &req[4];

  /* strip ' HTTP/1.[01]' */
  *len = *len - 9;
  req[*len] = '\0';

  *add_headers = true;

  return req;
}

static char * checkCommandPrefixes(char * req, size_t *len, bool *add_headers) {
  size_t l;

  if (!req || !len || !*len || !add_headers) {
    return req;
  }

  l = *len;

  /* '/http/...' */

  if ((l >= (SIW_PREFIX_HTTP_LEN + 1)) && !strncasecmp(req, SIW_PREFIX_HTTP "/", SIW_PREFIX_HTTP_LEN + 1)) {
    *len = l - SIW_PREFIX_HTTP_LEN;
    req = &req[SIW_PREFIX_HTTP_LEN];
    *add_headers = true;
    return req;
  }

  /* '/http' */

  if ((l == SIW_PREFIX_HTTP_LEN) && !strcasecmp(req, SIW_PREFIX_HTTP)) {
    *len = 0;
    *req = '\0';
    *add_headers = true;
    return req;
  }

  /* '/plain/...' */

  if ((l >= (SIW_PREFIX_PLAIN_LEN + 1)) && !strncasecmp(req, SIW_PREFIX_PLAIN "/", SIW_PREFIX_PLAIN_LEN + 1)) {
    *len = l - SIW_PREFIX_PLAIN_LEN;
    req = &req[SIW_PREFIX_PLAIN_LEN];
    *add_headers = false;
    return req;
  }

  /* '/plain' */

  if ((l == SIW_PREFIX_PLAIN_LEN) && !strcasecmp(req, SIW_PREFIX_PLAIN)) {
    *len = 0;
    *req = '\0';
    *add_headers = false;
    return req;
  }

  /* no prefixes */

  return req;
}

static void drain_request(int ipc_connection) {
  static char drain_buffer[AUTOBUFCHUNK];

  ssize_t r;
  do {
#ifdef _WIN32
    r = recv(ipc_connection, (void *) &drain_buffer, sizeof(drain_buffer), MSG_PEEK);
    if (r > 0) {
      r = recv(ipc_connection, (void *) &drain_buffer, sizeof(drain_buffer), 0);
    }
#else
    r = recv(ipc_connection, (void *) &drain_buffer, sizeof(drain_buffer), MSG_DONTWAIT);
#endif
  } while ((r > 0) && (r <= (ssize_t) sizeof(drain_buffer)));
}

static void ipc_action(int fd, void *data __attribute__ ((unused)), unsigned int flags __attribute__ ((unused))) {
#ifndef NODEBUG
  char addr[INET6_ADDRSTRLEN];
#endif /* NODEBUG */

  int ipc_connection = -1;
  union olsr_sockaddr sock_addr;
  socklen_t sock_addr_len = sizeof(sock_addr);
  bool hostDenied = false;
  struct timeval timeout;
  fd_set read_fds;
  char req_buffer[1024]; /* maximum size is the size of an IP packet */
  char * req = req_buffer;
  ssize_t rx_count = 0;
  unsigned int send_what = 0;
  unsigned int http_status = INFO_HTTP_OK;
  bool add_headers = config->http_headers;
  int r = 0;

  *req = '\0';

  if ((ipc_connection = accept(fd, &sock_addr.in, &sock_addr_len)) < 0) {
#ifndef NODEBUG
    olsr_printf(1, "(%s) accept()=%s\n", name, strerror(errno));
#endif /* NODEBUG */
    /* the caller will retry later */
    return;
  }

#ifdef _WIN32
  /* set the connection socket to non-blocking */
  {
    u_long iMode = 1;
    ioctlsocket(ipc_connection, FIONBIO, &iMode);
  }
#endif

  /* Wait at most this much time for the request to arrive on the connection */
  timeout.tv_sec = (outbuffer.count >= MAX_CLIENTS) ? 0 : config->request_timeout_sec;
  timeout.tv_usec = (outbuffer.count >= MAX_CLIENTS) ? 0 : config->request_timeout_usec;

  FD_ZERO(&read_fds);
#ifndef _WIN32
  FD_SET(ipc_connection, &read_fds);
#else
  FD_SET((unsigned int ) ipc_connection, &read_fds); /* Win32 needs the cast here */
#endif

  /* On success, select() and pselect() return the number of file descriptors
   * contained in the three returned descriptor sets (that is, the total number
   * of bits that are set in readfds, writefds, exceptfds) which may be zero if
   * the timeout expires before anything interesting happens. On error, -1 is
   * returned, and errno is set to indicate the error; the file descriptor sets
   * are unmodified, and timeout becomes undefined.
   */

  r = select(ipc_connection + 1, &read_fds, NULL, NULL, &timeout);
  if (r < 0) {
    /* ipc_connection is not ready for reading */
#ifndef NODEBUG
    olsr_printf(1, "(%s) select()=%s\n", name, strerror(errno));
#endif /* NODEBUG */
    drain_request(ipc_connection);
    if (outbuffer.count >= MAX_CLIENTS) {
      send_status_no_retries(req, add_headers, ipc_connection, INFO_HTTP_INTERNAL_SERVER_ERROR);
    } else {
      send_info(req, add_headers, send_what, ipc_connection, INFO_HTTP_INTERNAL_SERVER_ERROR);
    }
    return;
  }

  if (!r) {
    /* ipc_connection is not ready for reading within the timeout */
#ifndef NODEBUG
    olsr_printf(1, "(%s) select() timeout\n", name);
#endif /* NODEBUG */
    drain_request(ipc_connection);
    if (outbuffer.count >= MAX_CLIENTS) {
      send_status_no_retries(req, add_headers, ipc_connection, INFO_HTTP_REQUEST_TIMEOUT);
    } else {
      send_info(req, add_headers, send_what, ipc_connection, INFO_HTTP_REQUEST_TIMEOUT);
    }
    return;
  }

#ifdef _WIN32
  rx_count = recv(ipc_connection, req, sizeof(req_buffer), MSG_PEEK);
  if (rx_count > 0) {
    rx_count = recv(ipc_connection, req, sizeof(req_buffer), 0);
  }
#else
  rx_count = recv(ipc_connection, req, sizeof(req_buffer), MSG_DONTWAIT);
#endif

  /* Upon successful completion, recv() shall return the length of the message
   * in bytes. If no messages are available to be received and the peer has
   * performed an orderly shutdown, recv() shall return 0. Otherwise, −1 shall
   * be returned and errno set to indicate the error.
   */

  /* ensure proper request termination */
  if (rx_count <= 0) {
    *req = '\0';
  } else if (rx_count < (ssize_t) sizeof(req_buffer)) {
    req[rx_count] = '\0';
  } else {
    req[sizeof(req_buffer) - 1] = '\0';
  }

  /* sanitise the request */
  if (rx_count > 0) {
    req = cutAtFirstEOL(req, (size_t*) &rx_count);

    req = stripTrailingWhitespace(req, (size_t*) &rx_count);
    req = skipLeadingWhitespace(req, (size_t*) &rx_count);

    /* detect http requests */
    req = parseRequest(req, (size_t*) &rx_count, &add_headers);

    req = stripTrailingWhitespace(req, (size_t*) &rx_count);
    req = stripTrailingSlashes(req, (size_t*) &rx_count);
    req = skipLeadingWhitespace(req, (size_t*) &rx_count);
    req = skipMultipleSlashes(req, (size_t*) &rx_count);

    req = checkCommandPrefixes(req, (size_t*) &rx_count, &add_headers);

    req = skipMultipleSlashes(req, (size_t*) &rx_count);
  }

  if (outbuffer.count >= MAX_CLIENTS) {
    /* limit the number of replies that are in-flight */
    drain_request(ipc_connection);
    send_status_no_retries(req, add_headers, ipc_connection, INFO_HTTP_SERVICE_UNAVAILABLE);
    return;
  }

  if (olsr_cnf->ip_version == AF_INET) {
    hostDenied = //
        (ntohl(config->accept_ip.v4.s_addr) != INADDR_ANY) //
        && !ip4equal(&sock_addr.in4.sin_addr, &config->accept_ip.v4) //
        && (!config->allow_localhost //
            || (ntohl(sock_addr.in4.sin_addr.s_addr) != INADDR_LOOPBACK));
  } else {
    hostDenied = //
        !ip6equal(&config->accept_ip.v6, &in6addr_any) //
        && !ip6equal(&sock_addr.in6.sin6_addr, &config->accept_ip.v6) //
        && (!config->allow_localhost //
            || !ip6equal(&config->accept_ip.v6, &in6addr_loopback));
  }

#ifndef NODEBUG
  if (!inet_ntop( //
      olsr_cnf->ip_version, //
      (olsr_cnf->ip_version == AF_INET) ? (void *) &sock_addr.in4.sin_addr : (void *) &sock_addr.in6.sin6_addr, //
      addr, //
      sizeof(addr))) {
    addr[0] = '\0';
  }
#endif /* NODEBUG */

  if (hostDenied) {
#ifndef NODEBUG
    olsr_printf(1, "(%s) Connect from host %s is not allowed!\n", name, addr);
#endif /* NODEBUG */
    drain_request(ipc_connection);
    send_info(req, add_headers, send_what, ipc_connection, INFO_HTTP_FORBIDDEN);
    return;
  }

#ifndef NODEBUG
  olsr_printf(1, "(%s) Connect from host %s is allowed\n", name, addr);
#endif /* NODEBUG */

  if (rx_count < 0) {
#ifndef NODEBUG
    olsr_printf(1, "(%s) rx_count < 0\n", name);
#endif /* NODEBUG */
    drain_request(ipc_connection);
    send_info(req, add_headers, send_what, ipc_connection, INFO_HTTP_INTERNAL_SERVER_ERROR);
    return;
  }

  /* rx_count >= 0 */

  if (!rx_count) {
#ifndef NODEBUG
    olsr_printf(1, "(%s) rx_count == 0\n", name);
#endif /* NODEBUG */
    drain_request(ipc_connection);
    send_info(req, add_headers, SIW_EVERYTHING, ipc_connection, INFO_HTTP_OK);
    return;
  }

  /* rx_count > 0 */

  if (rx_count >= (ssize_t) sizeof(req_buffer)) {
#ifndef NODEBUG
    olsr_printf(1, "(%s) rx_count > %ld\n", name, (long int) sizeof(req_buffer));
#endif /* NODEBUG */

    /* input was much too long: read until the end for graceful connection termination
     * because wget can't handle the premature connection termination that is allowed
     * by the INFO_HTTP_REQUEST_ENTITY_TOO_LARGE HTTP status code
     */
    drain_request(ipc_connection);
    send_info(req, add_headers, send_what, ipc_connection, INFO_HTTP_REQUEST_ENTITY_TOO_LARGE);
    return;
  }

  /* 0 < rx_count < sizeof(requ) */

  if (!rx_count //
      || ((rx_count == 1) && (*req == '/'))) {
    /* empty or '/' */
    send_what = SIW_EVERYTHING;
  } else {
    send_what = determine_action(req);
  }

  if (!send_what) {
    http_status = INFO_HTTP_NOTFOUND;
  }

  send_info(req, add_headers, send_what, ipc_connection, http_status);
}

static int plugin_ipc_init(void) {
  union olsr_sockaddr sock_addr;
  uint32_t yes = 1;
  socklen_t sock_addr_len;

  /* Init ipc socket */
  if ((ipc_socket = socket(olsr_cnf->ip_version, SOCK_STREAM, 0)) == -1) {
#ifndef NODEBUG
    olsr_printf(1, "(%s) socket()=%s\n", name, strerror(errno));
#endif /* NODEBUG */
    goto error_out;
  }

  if (setsockopt(ipc_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &yes, sizeof(yes)) < 0) {
#ifndef NODEBUG
    olsr_printf(1, "(%s) setsockopt()=%s\n", name, strerror(errno));
#endif /* NODEBUG */
    goto error_out;
  }

#if (defined __FreeBSD__ || defined __FreeBSD_kernel__) && defined SO_NOSIGPIPE
  if (setsockopt(ipc_socket, SOL_SOCKET, SO_NOSIGPIPE, (char *) &yes, sizeof(yes)) < 0) {
    perror("SO_NOSIGPIPE failed");
    goto error_out;
  }
#endif /* (defined __FreeBSD__ || defined __FreeBSD_kernel__) && defined SO_NOSIGPIPE */

#if defined __linux__ && defined IPV6_V6ONLY
  if (config->ipv6_only && (olsr_cnf->ip_version == AF_INET6) //
      && (setsockopt(ipc_socket, IPPROTO_IPV6, IPV6_V6ONLY, (char *) &yes, sizeof(yes)) < 0)) {
    perror("IPV6_V6ONLY failed");
    goto error_out;
  }
#endif /* defined __linux__ && defined IPV6_V6ONLY */

  /* complete the socket structure */
  memset(&sock_addr, 0, sizeof(sock_addr));
  if (olsr_cnf->ip_version == AF_INET) {
    sock_addr.in4.sin_family = AF_INET;
    sock_addr_len = sizeof(struct sockaddr_in);
#ifdef SIN6_LEN
    sock_addr.in4.sin_len = sock_addr_len;
#endif /* SIN6_LEN */
    sock_addr.in4.sin_addr.s_addr = config->listen_ip.v4.s_addr;
    sock_addr.in4.sin_port = htons(config->ipc_port);
  } else {
    sock_addr.in6.sin6_family = AF_INET6;
    sock_addr_len = sizeof(struct sockaddr_in6);
#ifdef SIN6_LEN
    sock_addr.in6.sin6_len = sock_addr_len;
#endif /* SIN6_LEN */
    sock_addr.in6.sin6_addr = config->listen_ip.v6;
    sock_addr.in6.sin6_port = htons(config->ipc_port);
  }

  /* bind the socket to the port number */
  if (bind(ipc_socket, &sock_addr.in, sock_addr_len) == -1) {
#ifndef NODEBUG
    olsr_printf(1, "(%s) bind()=%s\n", name, strerror(errno));
#endif /* NODEBUG */
    goto error_out;
  }

  /* show that we are willing to listen */
  if (listen(ipc_socket, 1) == -1) {
#ifndef NODEBUG
    olsr_printf(1, "(%s) listen()=%s\n", name, strerror(errno));
#endif /* NODEBUG */
    goto error_out;
  }

  /* Register with olsrd */
  add_olsr_socket(ipc_socket, &ipc_action, NULL, NULL, SP_PR_READ);

#ifndef NODEBUG
  olsr_printf(1, "(%s) listening on port %d\n", name, config->ipc_port);
#endif /* NODEBUG */

  return 1;

  error_out: //
  if (ipc_socket >= 0) {
    close(ipc_socket);
    ipc_socket = -1;
  }
  return 0;
}

static void info_sanitise_config(info_plugin_config_t *cfg) {
  if (cfg->ipc_port < 1) {
    cfg->ipc_port = 1;
  }

  if (cfg->request_timeout < 0) {
    cfg->request_timeout = 0;
  }

  cfg->request_timeout_sec = cfg->request_timeout / 1000;
  cfg->request_timeout_usec = (cfg->request_timeout % 1000) * 1000;
}

int info_plugin_init(const char * plugin_name, info_plugin_functions_t *plugin_functions, info_plugin_config_t *plugin_config) {
  int i;

  assert(plugin_name);
  assert(plugin_functions);
  assert(plugin_config);

  name = plugin_name;
  functions = plugin_functions;
  config = plugin_config;

  info_sanitise_config(config);

  memset(&outbuffer, 0, sizeof(outbuffer));
  for (i = 0; i < MAX_CLIENTS; ++i) {
    outbuffer.socket[i] = -1;
  }

  ipc_socket = -1;

  if (functions->init) {
    functions->init(name);
  }

  info_plugin_cache_init(true);

  return plugin_ipc_init();
}

void info_plugin_exit(void) {
  int i;

  if (ipc_socket != -1) {
    close(ipc_socket);
    ipc_socket = -1;
  }
  for (i = 0; i < MAX_CLIENTS; ++i) {
    if (outbuffer.buffer[i]) {
      free(outbuffer.buffer[i]);
      outbuffer.buffer[i] = NULL;
    }
    outbuffer.size[i] = 0;
    outbuffer.written[i] = 0;
    if (outbuffer.socket[i]) {
      close(outbuffer.socket[i]);
      outbuffer.socket[i] = -1;
    }
  }
  outbuffer.count = 0;

  info_plugin_cache_init(false);
}
