
/*
 * HTTP Info plugin for the olsr.org OLSR daemon
 * Copyright (c) 2004, Andreas Tonnesen(andreto@olsr.org)
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#ifdef WIN32
#include <io.h>
#else
#include <netdb.h>
#endif

#include "olsr.h"
#include "olsr_cfg.h"
#include "interfaces.h"
#include "olsr_protocol.h"
#include "net_olsr.h"
#include "link_set.h"
#include "ipcalc.h"
#include "lq_plugin.h"
#include "common/autobuf.h"

#include "olsrd_httpinfo.h"
#include "admin_interface.h"
#include "gfx.h"

#ifdef OS
#undef OS
#endif

#ifdef WIN32
#define close(x) closesocket(x)
#define OS "Windows"
#endif
#ifdef linux
#define OS "GNU/Linux"
#endif
#if defined __FreeBSD__ || defined __FreeBSD_kernel__
#define OS "FreeBSD"
#endif

#ifndef OS
#define OS "Undefined"
#endif

static char copyright_string[] __attribute__ ((unused)) =
  "olsr.org HTTPINFO plugin Copyright (c) 2004, Andreas Tonnesen(andreto@olsr.org) All rights reserved.";

#define MAX_CLIENTS 3

#define MAX_HTTPREQ_SIZE (1024 * 10)

#define DEFAULT_TCP_PORT 1978

#define HTML_BUFSIZE (1024 * 4000)

#define FRAMEWIDTH (resolve_ip_addresses ? 900 : 800)

#define FILENREQ_MATCH(req, filename) \
        !strcmp(req, filename) || \
        (strlen(req) && !strcmp(&req[1], filename))

static const char httpinfo_css[] =
  "#A {text-decoration: none}\n" "TH{text-align: left}\n" "H1, H3, TD, TH {font-family: Helvetica; font-size: 80%}\n"
  "h2\n {\nfont-family: Helvetica;\n font-size: 14px;text-align: center;\n"
  "line-height: 16px;\ntext-decoration: none;\nborder: 1px solid #ccc;\n" "margin: 5px;\nbackground: #ececec;\n}\n"
  "hr\n{\nborder: none;\npadding: 1px;\nbackground: url(grayline.gif) repeat-x bottom;\n}\n"
  "#maintable\n{\nmargin: 0px;\npadding: 5px;\nborder-left: 1px solid #ccc;\n"
  "border-right: 1px solid #ccc;\nborder-bottom: 1px solid #ccc;\n}\n"
  "#footer\n{\nfont-size: 10px;\nline-height: 14px;\ntext-decoration: none;\ncolor: #666;\n}\n"
  "#hdr\n{\nfont-size: 14px;\ntext-align: center;\nline-height: 16px;\n" "text-decoration: none;\nborder: 1px solid #ccc;\n"
  "margin: 5px;\nbackground: #ececec;\n}\n"
  "#container\n{\nwidth: 1000px;\npadding: 30px;\nborder: 1px solid #ccc;\nbackground: #fff;\n}\n"
  "#tabnav\n{\nheight: 20px;\nmargin: 0;\npadding-left: 10px;\n" "background: url(grayline.gif) repeat-x bottom;\n}\n"
  "#tabnav li\n{\nmargin: 0;\npadding: 0;\ndisplay: inline;\nlist-style-type: none;\n}\n"
  "#tabnav a:link, #tabnav a:visited\n{\nfloat: left;\nbackground: #ececec;\n"
  "font-size: 12px;\nline-height: 14px;\nfont-weight: bold;\npadding: 2px 10px 2px 10px;\n"
  "margin-right: 4px;\nborder: 1px solid #ccc;\ntext-decoration: none;\ncolor: #777;\n}\n"
  "#tabnav a:link.active, #tabnav a:visited.active\n{\nborder-bottom: 1px solid #fff;\n" "background: #ffffff;\ncolor: #000;\n}\n"
  "#tabnav a:hover\n{\nbackground: #777777;\ncolor: #ffffff;\n}\n"
  ".input_text\n{\nbackground: #E5E5E5;\nmargin-left: 5px; margin-top: 0px;\n"
  "text-align: left;\n\nwidth: 100px;\npadding: 0px;\ncolor: #000000;\n"
  "text-decoration: none;\nfont-family: verdana;\nfont-size: 12px;\n" "border: 1px solid #ccc;\n}\n"
  ".input_button\n{\nbackground: #B5D1EE;\nmargin-left: 5px;\nmargin-top: 0px;\n"
  "text-align: center;\nwidth: 120px;\npadding: 0px;\ncolor: #000000;\n"
  "text-decoration: none;\nfont-family: verdana;\nfont-size: 12px;\n" "border: 1px solid #000;\n}\n";

typedef void (*build_body_callback) (struct autobuf *);

struct tab_entry {
  const char *tab_label;
  const char *filename;
  build_body_callback build_body_cb;
  bool display_tab;
};

struct static_bin_file_entry {
  const char *filename;
  unsigned char *data;
  unsigned int data_size;
};

struct static_txt_file_entry {
  const char *filename;
  const char *data;
};

struct dynamic_file_entry {
  const char *filename;
  int (*process_data_cb) (char *, uint32_t, char *, uint32_t);
};

static int get_http_socket(int);

static void build_tabs(struct autobuf *, int);

static void parse_http_request(int fd, void *, unsigned int);

static int build_http_header(http_header_type, bool, uint32_t, char *, uint32_t);

static void build_frame(struct autobuf *, const char *, const char *, int, build_body_callback frame_body_cb);

static void build_routes_body(struct autobuf *);

static void build_config_body(struct autobuf *);

static void build_neigh_body(struct autobuf *);

static void build_topo_body(struct autobuf *);

static void build_mid_body(struct autobuf *);

static void build_nodes_body(struct autobuf *);

static void build_all_body(struct autobuf *);

static void build_about_body(struct autobuf *);

static void build_cfgfile_body(struct autobuf *);

static int check_allowed_ip(const struct allowed_net *const /*allowed_nets*/, const union olsr_ip_addr *const /*addr*/);

static void build_ip_txt(struct autobuf *, const bool want_link, const char *const ipaddrstr, const int prefix_len);

static void build_ipaddr_link(struct autobuf *, const bool want_link, const union olsr_ip_addr *const ipaddr,
                             const int prefix_len);
static void section_title(struct autobuf *, const char *title);

static void httpinfo_write_data(void *foo);

static struct timeval start_time;
static struct http_stats stats;
static int http_socket;

static char *outbuffer[MAX_CLIENTS];
static size_t outbuffer_size[MAX_CLIENTS];
static size_t outbuffer_written[MAX_CLIENTS];
static int outbuffer_socket[MAX_CLIENTS];
static int outbuffer_count;

static struct timer_entry *writetimer_entry;

#if 0
int netsprintf(char *str, const char *format, ...) __attribute__ ((format(printf, 2, 3)));
static int netsprintf_direct = 0;
static int netsprintf_error = 0;
#define sprintf netsprintf
#define NETDIRECT
#endif

static const struct tab_entry tab_entries[] = {
  {"Configuration", "config", build_config_body, true},
  {"Routes", "routes", build_routes_body, true},
  {"Links/Topology", "nodes", build_nodes_body, true},
  {"All", "all", build_all_body, true},
#ifdef ADMIN_INTERFACE
  {"Admin", "admin", build_admin_body, true},
#endif
  {"About", "about", build_about_body, true},
  {"FOO", "cfgfile", build_cfgfile_body, false},
  {NULL, NULL, NULL, false}
};

static const struct static_bin_file_entry static_bin_files[] = {
  {"favicon.ico", favicon_ico, sizeof(favicon_ico)}
  ,
  {"logo.gif", logo_gif, sizeof(logo_gif)}
  ,
  {"grayline.gif", grayline_gif, sizeof(grayline_gif)}
  ,
  {NULL, NULL, 0}
};

static const struct static_txt_file_entry static_txt_files[] = {
  {"httpinfo.css", httpinfo_css},
  {NULL, NULL}
};

static const struct dynamic_file_entry dynamic_files[] = {
#ifdef ADMIN_INTERFACE
  {"set_values", process_set_values},
#endif
  {NULL, NULL}
};

static int
get_http_socket(int port)
{
  struct sockaddr_in sock_in;
  uint32_t yes = 1;

  /* Init ipc socket */
  int s = socket(AF_INET, SOCK_STREAM, 0);
  if (s == -1) {
    olsr_printf(1, "(HTTPINFO)socket %s\n", strerror(errno));
    return -1;
  }

  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof(yes)) < 0) {
    olsr_printf(1, "(HTTPINFO)SO_REUSEADDR failed %s\n", strerror(errno));
    close(s);
    return -1;
  }

  /* Bind the socket */

  /* complete the socket structure */
  memset(&sock_in, 0, sizeof(sock_in));
  sock_in.sin_family = AF_INET;
  sock_in.sin_addr.s_addr = httpinfo_listen_ip.v4.s_addr;
  sock_in.sin_port = htons(port);

  /* bind the socket to the port number */
  if (bind(s, (struct sockaddr *)&sock_in, sizeof(sock_in)) == -1) {
    olsr_printf(1, "(HTTPINFO) bind failed %s\n", strerror(errno));
    close(s);
    return -1;
  }

  /* show that we are willing to listen */
  if (listen(s, 1) == -1) {
    olsr_printf(1, "(HTTPINFO) listen failed %s\n", strerror(errno));
    close(s);
    return -1;
  }

  return s;
}

/**
 *Do initialization here
 *
 *This function is called by the my_init
 *function in uolsrd_plugin.c
 */
int
olsrd_plugin_init(void)
{
  /* Get start time */
  gettimeofday(&start_time, NULL);

  /* set up HTTP socket */
  http_socket = get_http_socket(http_port != 0 ? http_port : DEFAULT_TCP_PORT);

  if (http_socket < 0) {
    fprintf(stderr, "(HTTPINFO) could not initialize HTTP socket\n");
    exit(0);
  }

  /* Register socket */
  add_olsr_socket(http_socket, &parse_http_request, NULL, NULL, SP_PR_READ);

  return 1;
}

/* Non reentrant - but we are not multithreaded anyway */
static void
parse_http_request(int fd, void *data __attribute__ ((unused)), unsigned int flags __attribute__ ((unused)))
{
  struct sockaddr_in pin;
  struct autobuf body_abuf = { 0, 0, NULL };
  socklen_t addrlen;
  char *addr;
  char header_buf[MAX_HTTPREQ_SIZE];
  char req_type[11];
  char filename[251];
  char http_version[11];
  int client_socket;
  size_t header_length = 0;
  size_t c = 0;
  int r = 1;
#ifdef linux
  struct timeval timeout = { 0, 200 };
#endif

  if (outbuffer_count >= MAX_CLIENTS) {
    olsr_printf(1, "(HTTPINFO) maximum number of connection reached\n");
    return;
  }

  addrlen = sizeof(struct sockaddr_in);
  client_socket = accept(fd, (struct sockaddr *)&pin, &addrlen);
  if (client_socket == -1) {
    olsr_printf(1, "(HTTPINFO) accept: %s\n", strerror(errno));
    goto close_connection;
  }

#ifdef linux
  if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
    olsr_printf(1, "(HTTPINFO)SO_RCVTIMEO failed %s\n", strerror(errno));
    goto close_connection;
  }

  if (setsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
    olsr_printf(1, "(HTTPINFO)SO_SNDTIMEO failed %s\n", strerror(errno));
    goto close_connection;
  }
#endif
  if (!check_allowed_ip(allowed_nets, (union olsr_ip_addr *)&pin.sin_addr.s_addr)) {
    struct ipaddr_str strbuf;
    olsr_printf(0, "HTTP request from non-allowed host %s!\n",
                olsr_ip_to_string(&strbuf, (union olsr_ip_addr *)&pin.sin_addr.s_addr));
    goto close_connection;
  }

  addr = inet_ntoa(pin.sin_addr);

  memset(header_buf, 0, sizeof(header_buf));

  while ((r = recv(client_socket, &header_buf[c], 1, 0)) > 0 && (c < sizeof(header_buf) - 1)) {
    c++;

    if ((c > 3 && !strcmp(&header_buf[c - 4], "\r\n\r\n")) || (c > 1 && !strcmp(&header_buf[c - 2], "\n\n")))
      break;
  }

  if (r < 0) {
    olsr_printf(1, "(HTTPINFO) Failed to recieve data from client!\n");
    stats.err_hits++;
    goto close_connection;
  }

  /* Get the request */
  if (sscanf(header_buf, "%10s %250s %10s\n", req_type, filename, http_version) != 3) {
    /* Try without HTTP version */
    if (sscanf(header_buf, "%10s %250s\n", req_type, filename) != 2) {
      olsr_printf(1, "(HTTPINFO) Error parsing request %s!\n", header_buf);
      stats.err_hits++;
      goto close_connection;
    }
  }

  olsr_printf(1, "Request: %s\nfile: %s\nVersion: %s\n\n", req_type, filename, http_version);
  abuf_init(&body_abuf, 102400);

  if (!strcmp(req_type, "POST")) {
#ifdef ADMIN_INTERFACE
    int i = 0;
    while (dynamic_files[i].filename) {
      printf("POST checking %s\n", dynamic_files[i].filename);
      if (FILENREQ_MATCH(filename, dynamic_files[i].filename)) {
        uint32_t param_size;

        stats.ok_hits++;

        param_size = recv(client_sockets[curr_clients], header_buf, sizeof(header_buf) - 1, 0);

        header_buf[param_size] = '\0';
        printf("Dynamic read %d bytes\n", param_size);

        //memcpy(body, dynamic_files[i].data, static_bin_files[i].data_size);
        body_length += dynamic_files[i].process_data_cb(header_buf, param_size, &body_buf[body_length], sizeof(body_buf) - body_length);
        header_length = build_http_header(HTTP_OK, true, body_length, header_buf, sizeof(header_buf));
        goto send_http_data;
      }
      i++;
    }
#endif
    /* We only support GET */
    abuf_puts(&body_abuf, HTTP_400_MSG);
    stats.ill_hits++;
    header_length = build_http_header(HTTP_BAD_REQ, true, body_abuf.len, header_buf, sizeof(header_buf));
  } else if (!strcmp(req_type, "GET")) {
    int i = 0;

    for (i = 0; static_bin_files[i].filename; i++) {
      if (FILENREQ_MATCH(filename, static_bin_files[i].filename)) {
        break;
      }
    }

    if (static_bin_files[i].filename) {
      stats.ok_hits++;
      abuf_memcpy(&body_abuf, static_bin_files[i].data, static_bin_files[i].data_size);
      header_length = build_http_header(HTTP_OK, false, body_abuf.len, header_buf, sizeof(header_buf));
      goto send_http_data;
    }

    i = 0;
    while (static_txt_files[i].filename) {
      if (FILENREQ_MATCH(filename, static_txt_files[i].filename)) {
        break;
      }
      i++;
    }

    if (static_txt_files[i].filename) {
      stats.ok_hits++;
      abuf_puts(&body_abuf, static_txt_files[i].data);
      header_length = build_http_header(HTTP_OK, false, body_abuf.len, header_buf, sizeof(header_buf));
      goto send_http_data;
    }

    i = 0;
    if (strlen(filename) > 1) {
      while (tab_entries[i].filename) {
        if (FILENREQ_MATCH(filename, tab_entries[i].filename)) {
          break;
        }
        i++;
      }
    }

    if (tab_entries[i].filename) {
#ifdef NETDIRECT
      header_length = build_http_header(HTTP_OK, true, body_length, header_buf, sizeof(header_buf));
      r = send(client_sockets[curr_clients], header_buf, header_length, 0);
      if (r < 0) {
        olsr_printf(1, "(HTTPINFO) Failed sending data to client!\n");
        goto close_connection;
      }
      netsprintf_error = 0;
      netsprintf_direct = 1;
#endif
      abuf_appendf(&body_abuf,
                 "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n" "<head>\n"
                 "<meta http-equiv=\"Content-type\" content=\"text/html; charset=ISO-8859-1\">\n"
                 "<title>olsr.org httpinfo plugin</title>\n" "<link rel=\"icon\" href=\"favicon.ico\" type=\"image/x-icon\">\n"
                 "<link rel=\"shortcut icon\" href=\"favicon.ico\" type=\"image/x-icon\">\n"
                 "<link rel=\"stylesheet\" type=\"text/css\" href=\"httpinfo.css\">\n" "</head>\n"
                 "<body bgcolor=\"#ffffff\" text=\"#000000\">\n"
                 "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" width=\"%d\">\n"
                 "<tbody><tr bgcolor=\"#ffffff\">\n" "<td align=\"left\" height=\"69\" valign=\"middle\" width=\"80%%\">\n"
                 "<font color=\"black\" face=\"timesroman\" size=\"6\">&nbsp;&nbsp;&nbsp;<a href=\"http://www.olsr.org/\">olsr.org OLSR daemon</a></font></td>\n"
                 "<td height=\"69\" valign=\"middle\" width=\"20%%\">\n"
                 "<a href=\"http://www.olsr.org/\"><img border=\"0\" src=\"/logo.gif\" alt=\"olsrd logo\"></a></td>\n" "</tr>\n"
                 "</tbody>\n" "</table>\n", FRAMEWIDTH);

      build_tabs(&body_abuf, i);
      build_frame(&body_abuf, "Current Routes", "routes", FRAMEWIDTH, tab_entries[i].build_body_cb);

      stats.ok_hits++;

      abuf_appendf(&body_abuf,
                 "</table>\n" "<div id=\"footer\">\n" "<center>\n" "(C)2005 Andreas T&oslash;nnesen<br/>\n"
                 "<a href=\"http://www.olsr.org/\">http://www.olsr.org</a>\n" "</center>\n" "</div>\n" "</body>\n" "</html>\n");

#ifdef NETDIRECT
      netsprintf_direct = 1;
      goto close_connection;
#else
      header_length = build_http_header(HTTP_OK, true, body_abuf.len, header_buf, sizeof(header_buf));
      goto send_http_data;
#endif
    }

    stats.ill_hits++;
    abuf_puts(&body_abuf, HTTP_404_MSG);
    header_length = build_http_header(HTTP_BAD_FILE, true, body_abuf.len, header_buf, sizeof(header_buf));
  } else {
    /* We only support GET */
    abuf_puts(&body_abuf, HTTP_404_MSG);
    stats.ill_hits++;
    header_length = build_http_header(HTTP_BAD_REQ, true, body_abuf.len, header_buf, sizeof(header_buf));
  }

send_http_data:
  if (header_length + body_abuf.len > 0) {
    outbuffer[outbuffer_count] = olsr_malloc(header_length + body_abuf.len, "http output buffer");
    outbuffer_size[outbuffer_count] = header_length + body_abuf.len;
    outbuffer_written[outbuffer_count] = 0;
    outbuffer_socket[outbuffer_count] = client_socket;

    memcpy(outbuffer[outbuffer_count], header_buf, header_length);
    if (body_abuf.len > 0) {
      memcpy((outbuffer[outbuffer_count]) + header_length, body_abuf.buf, body_abuf.len);
    }
    outbuffer_count++;

    if (outbuffer_count == 1) {
      writetimer_entry = olsr_start_timer(100, 0, OLSR_TIMER_PERIODIC, &httpinfo_write_data, NULL, 0);
    }
  }
  abuf_free(&body_abuf);
  return;

close_connection:
  abuf_free(&body_abuf);
  close(client_socket);
}

static void
httpinfo_write_data(void *foo __attribute__ ((unused))) {
  fd_set set;
  int result, i, j, max;
  struct timeval tv;

  FD_ZERO(&set);
  max = 0;
  for (i=0; i<outbuffer_count; i++) {
    /* prevent warning in win32 */
    FD_SET((unsigned int)outbuffer_socket[i], &set);
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
      result = write(outbuffer_socket[i], outbuffer[i] + outbuffer_written[i], outbuffer_size[i] - outbuffer_written[i]);
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

int
build_http_header(http_header_type type, bool is_html, uint32_t msgsize, char *buf, uint32_t bufsize)
{
  time_t currtime;
  const char *h;
  int size;

  switch (type) {
  case HTTP_BAD_REQ:
    h = HTTP_400;
    break;
  case HTTP_BAD_FILE:
    h = HTTP_404;
    break;
  default:
    /* Defaults to OK */
    h = HTTP_200;
    break;
  }
  size = snprintf(buf, bufsize, "%s", h);

  /* Date */
  time(&currtime);
  size += strftime(&buf[size], bufsize - size, "Date: %a, %d %b %Y %H:%M:%S GMT\r\n", localtime(&currtime));

  /* Server version */
  size += snprintf(&buf[size], bufsize - size, "Server: %s %s %s\r\n", PLUGIN_NAME, PLUGIN_VERSION, HTTP_VERSION);

  /* connection-type */
  size += snprintf(&buf[size], bufsize - size, "Connection: closed\r\n");

  /* MIME type */
  size += snprintf(&buf[size], bufsize - size, "Content-type: text/%s\r\n", is_html ? "html" : "plain");

  /* Content length */
  if (msgsize > 0) {
    size += snprintf(&buf[size], bufsize - size, "Content-length: %i\r\n", msgsize);
  }

  /* Cache-control
   * No caching dynamic pages
   */
  size += snprintf(&buf[size], bufsize - size, "Cache-Control: no-cache\r\n");

  if (!is_html) {
    size += snprintf(&buf[size], bufsize - size, "Accept-Ranges: bytes\r\n");
  }
  /* End header */
  size += snprintf(&buf[size], bufsize - size, "\r\n");

  olsr_printf(1, "HEADER:\n%s", buf);

  return size;
}

static void
build_tabs(struct autobuf *abuf, int active)
{
  int tabs = 0;

  abuf_appendf(abuf,
      "<table align=\"center\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" width=\"%d\">\n"
      "<tr bgcolor=\"#ffffff\"><td>\n" "<ul id=\"tabnav\">\n", FRAMEWIDTH);
  for (tabs = 0; tab_entries[tabs].tab_label; tabs++) {
    if (!tab_entries[tabs].display_tab) {
      continue;
    }
    abuf_appendf(abuf, "<li><a href=\"%s\"%s>%s</a></li>\n", tab_entries[tabs].filename,
               tabs == active ? " class=\"active\"" : "", tab_entries[tabs].tab_label);
  }
  abuf_appendf(abuf, "</ul>\n" "</td></tr>\n" "<tr><td>\n");
}

/*
 * destructor - called at unload
 */
void
olsr_plugin_exit(void)
{
  struct allowed_net *a, *next;
  if (http_socket >= 0) {
    CLOSE(http_socket);
  }

  for (a = allowed_nets; a != NULL; a = next) {
    next = a->next;

    free(a);
  }
}

static void
section_title(struct autobuf *abuf, const char *title)
{
  abuf_appendf(abuf,
                  "<h2>%s</h2>\n" "<table width=\"100%%\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\">\n",
                  title);
}

static void
build_frame(struct autobuf *abuf, const char *title __attribute__ ((unused)), const char *the_link
            __attribute__ ((unused)), int width __attribute__ ((unused)), build_body_callback frame_body_cb)
{
  abuf_puts(abuf, "<div id=\"maintable\">\n");
  frame_body_cb(abuf);
  abuf_puts(abuf, "</div>\n");
}

static void
fmt_href(struct autobuf *abuf, const char *const ipaddr)
{
  abuf_appendf(abuf, "<a href=\"http://%s:%d/all\">", ipaddr, http_port);
}

static void
build_ip_txt(struct autobuf *abuf, const bool print_link, const char *const ipaddrstr, const int prefix_len)
{
  if (print_link) {
    fmt_href(abuf, ipaddrstr);
  }

  abuf_puts(abuf, ipaddrstr);
  /* print ip address or ip prefix ? */
  if (prefix_len != -1 && prefix_len != olsr_cnf->maxplen) {
    abuf_appendf(abuf, "/%d", prefix_len);
  }

  if (print_link) {             /* Print the link only if there is no prefix_len */
    abuf_puts(abuf, "</a>");
  }
}

static void
build_ipaddr_link(struct autobuf *abuf, const bool want_link, const union olsr_ip_addr *const ipaddr,
                  const int prefix_len)
{
  struct ipaddr_str ipaddrstr;
  const struct hostent *const hp =
#ifndef WIN32
    resolve_ip_addresses ? gethostbyaddr(ipaddr, olsr_cnf->ipsize,
                                         olsr_cnf->ip_version) :
#endif
    NULL;
  /* Print the link only if there is no prefix_len and ip_version is AF_INET */
  const int print_link = want_link && (prefix_len == -1 || prefix_len == olsr_cnf->maxplen) && (olsr_cnf->ip_version == AF_INET);
  olsr_ip_to_string(&ipaddrstr, ipaddr);

  abuf_puts(abuf, "<td>");
  build_ip_txt(abuf, print_link, ipaddrstr.buf, prefix_len);
  abuf_puts(abuf, "</td>");

  if (resolve_ip_addresses) {
    if (hp) {
      abuf_puts(abuf, "<td>(");
      if (print_link) {
        fmt_href(abuf, ipaddrstr.buf);
      }
      abuf_puts(abuf, hp->h_name);
      if (print_link) {
        abuf_puts(abuf, "</a>");
      }
      abuf_puts(abuf, ")</td>");
    } else {
      abuf_puts(abuf, "<td/>");
    }
  }
}

#define build_ipaddr_with_link(buf, ipaddr, plen) \
          build_ipaddr_link((buf), true, (ipaddr), (plen))
#define build_ipaddr_no_link(buf, ipaddr, plen) \
          build_ipaddr_link((buf), false, (ipaddr), (plen))

static void
build_route(struct autobuf *abuf, const struct rt_entry *rt)
{
  struct lqtextbuffer lqbuffer;

  abuf_puts(abuf, "<tr>");
  build_ipaddr_with_link(abuf, &rt->rt_dst.prefix, rt->rt_dst.prefix_len);
  build_ipaddr_with_link(abuf, &rt->rt_best->rtp_nexthop.gateway, -1);

  abuf_appendf(abuf, "<td>%d</td>", rt->rt_best->rtp_metric.hops);
  abuf_appendf(abuf, "<td>%s</td>",
             get_linkcost_text(rt->rt_best->rtp_metric.cost, true, &lqbuffer));
  abuf_appendf(abuf, "<td>%s</td></tr>\n",
             if_ifwithindex_name(rt->rt_best->rtp_nexthop.iif_index));
}

static void
build_routes_body(struct autobuf *abuf)
{
  struct rt_entry *rt;
  const char *colspan = resolve_ip_addresses ? " colspan=\"2\"" : "";
  section_title(abuf, "OLSR Routes in Kernel");
  abuf_appendf(abuf,
             "<tr><th%s>Destination</th><th%s>Gateway</th><th>Metric</th><th>ETX</th><th>Interface</th></tr>\n",
             colspan, colspan);

  /* Walk the route table */
  OLSR_FOR_ALL_RT_ENTRIES(rt) {
    build_route(abuf, rt);
  } OLSR_FOR_ALL_RT_ENTRIES_END(rt);

  abuf_puts(abuf, "</table>\n");
}

static void
build_config_body(struct autobuf *abuf)
{
  const struct olsr_if *ifs;
  const struct plugin_entry *pentry;
  const struct plugin_param *pparam;
  struct ipaddr_str mainaddrbuf;

  abuf_appendf(abuf, "Version: %s (built on %s on %s)\n<br>", olsrd_version, build_date, build_host);
  abuf_appendf(abuf, "OS: %s\n<br>", OS);

  {
    const time_t currtime = time(NULL);

    abuf_strftime(abuf, "System time: <em>%a, %d %b %Y %H:%M:%S</em><br>",
                            localtime(&currtime));
  }

  {
    struct timeval now, uptime;
    int hours, mins, days;
    gettimeofday(&now, NULL);
    timersub(&now, &start_time, &uptime);

    days = uptime.tv_sec / 86400;
    uptime.tv_sec %= 86400;
    hours = uptime.tv_sec / 3600;
    uptime.tv_sec %= 3600;
    mins = uptime.tv_sec / 60;
    uptime.tv_sec %= 60;

    abuf_puts(abuf, "Olsrd uptime: <em>");
    if (days) {
      abuf_appendf(abuf, "%d day(s) ", days);
    }
    abuf_appendf(abuf, "%02d hours %02d minutes %02d seconds</em><br/>\n", hours, mins, (int)uptime.tv_sec);
  }

  abuf_appendf(abuf, "HTTP stats(ok/dyn/error/illegal): <em>%d/%d/%d/%d</em><br>\n", stats.ok_hits,
             stats.dyn_hits, stats.err_hits, stats.ill_hits);

  abuf_puts(abuf,
             "Click <a href=\"/cfgfile\">here</a> to <em>generate a configuration file for this node</em>.\n");

  abuf_puts(abuf, "<h2>Variables</h2>\n");

  abuf_puts(abuf, "<table width=\"100%%\" border=\"0\">\n<tr>");

  abuf_appendf(abuf, "<td>Main address: <strong>%s</strong></td>\n",
             olsr_ip_to_string(&mainaddrbuf, &olsr_cnf->main_addr));
  abuf_appendf(abuf, "<td>IP version: %d</td>\n", olsr_cnf->ip_version == AF_INET ? 4 : 6);
  abuf_appendf(abuf, "<td>Debug level: %d</td>\n", olsr_cnf->debug_level);
  abuf_appendf(abuf, "<td>FIB Metrics: %s</td>\n", FIB_METRIC_TXT[olsr_cnf->fib_metric]);

  abuf_puts(abuf, "</tr>\n<tr>\n");

  abuf_appendf(abuf, "<td>Pollrate: %0.2f</td>\n", olsr_cnf->pollrate);
  abuf_appendf(abuf, "<td>TC redundancy: %d</td>\n", olsr_cnf->tc_redundancy);
  abuf_appendf(abuf, "<td>MPR coverage: %d</td>\n", olsr_cnf->mpr_coverage);
  abuf_appendf(abuf, "<td>NAT threshold: %f</td>\n", olsr_cnf->lq_nat_thresh);

  abuf_puts(abuf, "</tr>\n<tr>\n");

  abuf_appendf(abuf, "<td>Fisheye: %s</td>\n", olsr_cnf->lq_fish ? "Enabled" : "Disabled");
  abuf_appendf(abuf, "<td>TOS: 0x%04x</td>\n", olsr_cnf->tos);
  abuf_appendf(abuf, "<td>RtTable: 0x%04x/%d</td>\n", olsr_cnf->rt_table, olsr_cnf->rt_table);
  abuf_appendf(abuf, "<td>RtTableDefault: 0x%04x/%d</td>\n", olsr_cnf->rt_table_default,
             olsr_cnf->rt_table_default);
  abuf_appendf(abuf, "<td>RtTableTunnel: 0x%04x/%d</td>\n", olsr_cnf->rt_table_tunnel,
             olsr_cnf->rt_table_tunnel);
  abuf_appendf(abuf, "<td>Willingness: %d %s</td>\n", olsr_cnf->willingness,
             olsr_cnf->willingness_auto ? "(auto)" : "");

  if (olsr_cnf->lq_level == 0) {
    abuf_appendf(abuf, "</tr>\n<tr>\n" "<td>Hysteresis: %s</td>\n",
               olsr_cnf->use_hysteresis ? "Enabled" : "Disabled");
    if (olsr_cnf->use_hysteresis) {
      abuf_appendf(abuf, "<td>Hyst scaling: %0.2f</td>\n", olsr_cnf->hysteresis_param.scaling);
      abuf_appendf(abuf, "<td>Hyst lower/upper: %0.2f/%0.2f</td>\n", olsr_cnf->hysteresis_param.thr_low,
                 olsr_cnf->hysteresis_param.thr_high);
    }
  }

  abuf_appendf(abuf, "</tr>\n<tr>\n" "<td>LQ extension: %s</td>\n",
             olsr_cnf->lq_level ? "Enabled" : "Disabled");
  if (olsr_cnf->lq_level) {
    abuf_appendf(abuf, "<td>LQ level: %d</td>\n" "<td>LQ aging: %f</td>\n", olsr_cnf->lq_level,
               olsr_cnf->lq_aging);
  }
  abuf_puts(abuf, "</tr></table>\n");

  abuf_puts(abuf, "<h2>Interfaces</h2>\n");
  abuf_puts(abuf, "<table width=\"100%%\" border=\"0\">\n");
  for (ifs = olsr_cnf->interfaces; ifs != NULL; ifs = ifs->next) {
    const struct interface *const rifs = ifs->interf;
    abuf_appendf(abuf, "<tr><th colspan=\"3\">%s</th>\n", ifs->name);
    if (!rifs) {
      abuf_puts(abuf, "<tr><td colspan=\"3\">Status: DOWN</td></tr>\n");
      continue;
    }

    if (olsr_cnf->ip_version == AF_INET) {
      struct ipaddr_str addrbuf, maskbuf, bcastbuf;
      abuf_appendf(abuf, "<tr>\n" "<td>IP: %s</td>\n" "<td>MASK: %s</td>\n" "<td>BCAST: %s</td>\n" "</tr>\n",
                 ip4_to_string(&addrbuf, rifs->int_addr.sin_addr), ip4_to_string(&maskbuf, rifs->int_netmask.sin_addr),
                 ip4_to_string(&bcastbuf, rifs->int_broadaddr.sin_addr));
    } else {
      struct ipaddr_str addrbuf, maskbuf;
      abuf_appendf(abuf, "<tr>\n" "<td>IP: %s</td>\n" "<td>MCAST: %s</td>\n" "<td></td>\n" "</tr>\n",
                 ip6_to_string(&addrbuf, &rifs->int6_addr.sin6_addr), ip6_to_string(&maskbuf, &rifs->int6_multaddr.sin6_addr));
    }
    abuf_appendf(abuf, "<tr>\n" "<td>MTU: %d</td>\n" "<td>WLAN: %s</td>\n" "<td>STATUS: UP</td>\n" "</tr>\n",
               rifs->int_mtu, rifs->is_wireless ? "Yes" : "No");
  }
  abuf_puts(abuf, "</table>\n");

  abuf_appendf(abuf, "<em>Olsrd is configured to %s if no interfaces are available</em><br>\n",
             olsr_cnf->allow_no_interfaces ? "run even" : "halt");

  abuf_puts(abuf, "<h2>Plugins</h2>\n");
  abuf_puts(abuf, "<table width=\"100%%\" border=\"0\"><tr><th>Name</th><th>Parameters</th></tr>\n");
  for (pentry = olsr_cnf->plugins; pentry; pentry = pentry->next) {
    abuf_appendf(abuf, "<tr><td>%s</td>\n" "<td><select>\n" "<option>KEY, VALUE</option>\n", pentry->name);

    for (pparam = pentry->params; pparam; pparam = pparam->next) {
      abuf_appendf(abuf, "<option>\"%s\", \"%s\"</option>\n", pparam->key, pparam->value);
    }
    abuf_puts(abuf, "</select></td></tr>\n");

  }
  abuf_puts(abuf, "</table>\n");

  section_title(abuf, "Announced HNA entries");
  if (olsr_cnf->hna_entries) {
    struct ip_prefix_list *hna;
    abuf_puts(abuf, "<tr><th>Network</th></tr>\n");
    for (hna = olsr_cnf->hna_entries; hna; hna = hna->next) {
      struct ipaddr_str netbuf;
      abuf_appendf(abuf, "<tr><td>%s/%d</td></tr>\n", olsr_ip_to_string(&netbuf, &hna->net.prefix),
                 hna->net.prefix_len);
    }
  } else {
    abuf_puts(abuf, "<tr><td></td></tr>\n");
  }
  abuf_puts(abuf, "</table>\n");
}

static void
build_neigh_body(struct autobuf *abuf)
{
  struct neighbor_entry *neigh;
  struct link_entry *the_link = NULL;
  const char *colspan = resolve_ip_addresses ? " colspan=\"2\"" : "";

  section_title(abuf, "Links");

  abuf_appendf(abuf,
             "<tr><th%s>Local IP</th><th%s>Remote IP</th><th>Hysteresis</th>",
             colspan, colspan);
  if (olsr_cnf->lq_level > 0) {
    abuf_puts(abuf, "<th>LinkCost</th>");
  }
  abuf_puts(abuf, "</tr>\n");

  /* Link set */
  OLSR_FOR_ALL_LINK_ENTRIES(the_link) {
    abuf_puts(abuf, "<tr>");
    build_ipaddr_with_link(abuf, &the_link->local_iface_addr, -1);
    build_ipaddr_with_link(abuf, &the_link->neighbor_iface_addr, -1);
    abuf_appendf(abuf, "<td>%0.2f</td>", the_link->L_link_quality);
    if (olsr_cnf->lq_level > 0) {
      struct lqtextbuffer lqbuffer1, lqbuffer2;
      abuf_appendf(abuf, "<td>(%s) %s</td>", get_link_entry_text(the_link, '/', &lqbuffer1),
                 get_linkcost_text(the_link->linkcost, false, &lqbuffer2));
    }
    abuf_puts(abuf, "</tr>\n");
  } OLSR_FOR_ALL_LINK_ENTRIES_END(the_link);

  abuf_puts(abuf, "</table>\n");

  section_title(abuf, "Neighbors");
  abuf_appendf(abuf,
             "<tr><th%s>IP Address</th><th>SYM</th><th>MPR</th><th>MPRS</th><th>Willingness</th><th>2 Hop Neighbors</th></tr>\n",
             colspan);
  /* Neighbors */
  OLSR_FOR_ALL_NBR_ENTRIES(neigh) {

    struct neighbor_2_list_entry *list_2;
    int thop_cnt;
    abuf_puts(abuf, "<tr>");
    build_ipaddr_with_link(abuf, &neigh->neighbor_main_addr, -1);
    abuf_appendf(abuf,
               "<td>%s</td>" "<td>%s</td>" "<td>%s</td>"
               "<td>%d</td>", (neigh->status == SYM) ? "YES" : "NO", neigh->is_mpr ? "YES" : "NO",
               olsr_lookup_mprs_set(&neigh->neighbor_main_addr) ? "YES" : "NO", neigh->willingness);

    abuf_puts(abuf, "<td><select>\n" "<option>IP ADDRESS</option>\n");

    for (list_2 = neigh->neighbor_2_list.next, thop_cnt = 0; list_2 != &neigh->neighbor_2_list; list_2 = list_2->next, thop_cnt++) {
      struct ipaddr_str strbuf;
      abuf_appendf(abuf, "<option>%s</option>\n",
                 olsr_ip_to_string(&strbuf, &list_2->neighbor_2->neighbor_2_addr));
    }
    abuf_appendf(abuf, "</select> (%d)</td></tr>\n", thop_cnt);
  } OLSR_FOR_ALL_NBR_ENTRIES_END(neigh);

  abuf_puts(abuf, "</table>\n");
}

static void
build_topo_body(struct autobuf *abuf)
{
  struct tc_entry *tc;
  const char *colspan = resolve_ip_addresses ? " colspan=\"2\"" : "";

  section_title(abuf, "Topology Entries");
  abuf_appendf(abuf, "<tr><th%s>Destination IP</th><th%s>Last Hop IP</th>",
             colspan, colspan);
  if (olsr_cnf->lq_level > 0) {
    abuf_puts(abuf, "<th>Linkcost</th>");
  }
  abuf_puts(abuf, "</tr>\n");

  OLSR_FOR_ALL_TC_ENTRIES(tc) {
    struct tc_edge_entry *tc_edge;
    OLSR_FOR_ALL_TC_EDGE_ENTRIES(tc, tc_edge) {
      if (tc_edge->edge_inv) {
        abuf_puts(abuf, "<tr>");
        build_ipaddr_with_link(abuf, &tc_edge->T_dest_addr, -1);
        build_ipaddr_with_link(abuf, &tc->addr, -1);
        if (olsr_cnf->lq_level > 0) {
          struct lqtextbuffer lqbuffer1, lqbuffer2;
          abuf_appendf(abuf, "<td>(%s) %s</td>\n",
                     get_tc_edge_entry_text(tc_edge, '/', &lqbuffer1), get_linkcost_text(tc_edge->cost, false, &lqbuffer2));
        }
        abuf_puts(abuf, "</tr>\n");
      }
    } OLSR_FOR_ALL_TC_EDGE_ENTRIES_END(tc, tc_edge);
  } OLSR_FOR_ALL_TC_ENTRIES_END(tc);

  abuf_puts(abuf, "</table>\n");
}

static void
build_mid_body(struct autobuf *abuf)
{
  int idx;
  const char *colspan = resolve_ip_addresses ? " colspan=\"2\"" : "";

  section_title(abuf, "MID Entries");
  abuf_appendf(abuf, "<tr><th%s>Main Address</th><th>Aliases</th></tr>\n", colspan);

  /* MID */
  for (idx = 0; idx < HASHSIZE; idx++) {
    struct mid_entry *entry;
    for (entry = mid_set[idx].next; entry != &mid_set[idx]; entry = entry->next) {
      int mid_cnt;
      struct mid_address *alias;
      abuf_puts(abuf, "<tr>");
      build_ipaddr_with_link(abuf, &entry->main_addr, -1);
      abuf_puts(abuf, "<td><select>\n<option>IP ADDRESS</option>\n");

      for (mid_cnt = 0, alias = entry->aliases; alias != NULL; alias = alias->next_alias, mid_cnt++) {
        struct ipaddr_str strbuf;
        abuf_appendf(abuf, "<option>%s</option>\n", olsr_ip_to_string(&strbuf, &alias->alias));
      }
      abuf_appendf(abuf, "</select> (%d)</td></tr>\n", mid_cnt);
    }
  }

  abuf_puts(abuf, "</table>\n");
}

static void
build_nodes_body(struct autobuf *abuf)
{
  build_neigh_body(abuf);
  build_topo_body(abuf);
  build_mid_body(abuf);
}

static void
build_all_body(struct autobuf *abuf)
{
  build_config_body(abuf);
  build_routes_body(abuf);
  build_neigh_body(abuf);
  build_topo_body(abuf);
  build_mid_body(abuf);
}

static void
build_about_body(struct autobuf *abuf)
{
  abuf_appendf(abuf,
                  "<strong>" PLUGIN_NAME " version " PLUGIN_VERSION "</strong><br/>\n" "by Andreas T&oslash;nnesen (C)2005.<br/>\n"
                  "Compiled "
#ifdef ADMIN_INTERFACE
                  "<em>with experimental admin interface</em> "
#endif
                  "%s at %s<hr/>\n" "This plugin implements a HTTP server that supplies\n"
                  "the client with various dynamic web pages representing\n"
                  "the current olsrd status.<br/>The different pages include:\n"
                  "<ul>\n<li><strong>Configuration</strong> - This page displays information\n"
                  "about the current olsrd configuration. This includes various\n"
                  "olsr settings such as IP version, MID/TC redundancy, hysteresis\n"
                  "etc. Information about the current status of the interfaces on\n"
                  "which olsrd is configured to run is also displayed. Loaded olsrd\n"
                  "plugins are shown with their plugin parameters. Finally all local\n"
                  "HNA entries are shown. These are the networks that the local host\n"
                  "will anounce itself as a gateway to.</li>\n"
                  "<li><strong>Routes</strong> - This page displays all routes currently set in\n"
                  "the kernel <em>by olsrd</em>. The type of route is also displayed(host\n" "or HNA).</li>\n"
                  "<li><strong>Links/Topology</strong> - This page displays all information about\n"
                  "links, neighbors, topology, MID and HNA entries.</li>\n"
                  "<li><strong>All</strong> - Here all the previous pages are displayed as one.\n"
                  "This is to make all information available as easy as possible(for example\n"
                  "for a script) and using as few resources as possible.</li>\n"
#ifdef ADMIN_INTERFACE
                  "<li><strong>Admin</strong> - This page is highly experimental(and unsecure)!\n"
                  "As of now it is not working at all but it provides a impression of\n"
                  "the future possibilities of httpinfo. This is to be a interface to\n"
                  "changing olsrd settings in realtime. These settings include various\n"
                  "\"basic\" settings and local HNA settings.</li>\n"
#endif
                  "<li><strong>About</strong> - this help page.</li>\n</ul>" "<hr/>\n" "Send questions or comments to\n"
                  "<a href=\"mailto:olsr-users@olsr.org\">olsr-users@olsr.org</a> or\n"
                  "<a href=\"mailto:andreto-at-olsr.org\">andreto-at-olsr.org</a><br/>\n"
                  "Official olsrd homepage: <a href=\"http://www.olsr.org/\">http://www.olsr.org</a><br/>\n", build_date,
                  build_host);
}

static void
build_cfgfile_body(struct autobuf *abuf)
{
  abuf_puts(abuf,
             "\n\n" "<strong>This is a automatically generated configuration\n"
             "file based on the current olsrd configuration of this node.<br/>\n" "<hr/>\n" "<pre>\n");
  olsrd_write_cnf_autobuf(abuf, olsr_cnf);

  abuf_puts(abuf, "</pre>\n<hr/>\n");

#if 0
  printf("RETURNING %d\n", size);
#endif
}

static int
check_allowed_ip(const struct allowed_net *const my_allowed_nets, const union olsr_ip_addr *const addr)
{
  const struct allowed_net *alln;
  for (alln = my_allowed_nets; alln != NULL; alln = alln->next) {
    if (ip_in_net(addr, &alln->prefix)) {
      return 1;
    }
  }
  return 0;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
