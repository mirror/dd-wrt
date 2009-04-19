
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
#include <stdarg.h>
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
#include "socket_parser.h"
#include "ipcalc.h"
#include "lq_plugin.h"

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
#ifdef __FreeBSD__
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

typedef int (*build_body_callback) (char *, uint32_t);

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

static int build_tabs(char *, uint32_t, int);

static void parse_http_request(int);

static int build_http_header(http_header_type, bool, uint32_t, char *, uint32_t);

static int build_frame(char *, uint32_t, const char *, const char *, int, build_body_callback frame_body_cb);

static int build_routes_body(char *, uint32_t);

static int build_config_body(char *, uint32_t);

static int build_neigh_body(char *, uint32_t);

static int build_topo_body(char *, uint32_t);

static int build_mid_body(char *, uint32_t);

static int build_nodes_body(char *, uint32_t);

static int build_all_body(char *, uint32_t);

static int build_about_body(char *, uint32_t);

static int build_cfgfile_body(char *, uint32_t);

static int check_allowed_ip(const struct allowed_net *const allowed_nets, const union olsr_ip_addr *const addr);

static int build_ip_txt(char *buf, const uint32_t bufsize, const bool want_link, const char *const ipaddrstr, const int prefix_len);

static int build_ipaddr_link(char *buf, const uint32_t bufsize, const bool want_link, const union olsr_ip_addr *const ipaddr,
                             const int prefix_len);
static int section_title(char *buf, uint32_t bufsize, const char *title);

static ssize_t writen(int fd, const void *buf, size_t count);

static struct timeval start_time;
static struct http_stats stats;
static int client_sockets[MAX_CLIENTS];
static int curr_clients;
static int http_socket;

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
  struct sockaddr_in sin;
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
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(port);

  /* bind the socket to the port number */
  if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) == -1) {
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

  curr_clients = 0;
  /* set up HTTP socket */
  http_socket = get_http_socket(http_port != 0 ? http_port : DEFAULT_TCP_PORT);

  if (http_socket < 0) {
    fprintf(stderr, "(HTTPINFO) could not initialize HTTP socket\n");
    exit(0);
  }

  /* Register socket */
  add_olsr_socket(http_socket, &parse_http_request);

  return 1;
}

/* Non reentrant - but we are not multithreaded anyway */
void
parse_http_request(int fd)
{
  struct sockaddr_in pin;
  socklen_t addrlen;
  char *addr;
  char req[MAX_HTTPREQ_SIZE];
  static char body[HTML_BUFSIZE];
  char req_type[11];
  char filename[251];
  char http_version[11];
  unsigned int c = 0;
  int r = 1, size = 0;

  if (curr_clients >= MAX_CLIENTS) {
    return;
  }
  curr_clients++;

  addrlen = sizeof(struct sockaddr_in);
  client_sockets[curr_clients] = accept(fd, (struct sockaddr *)&pin, &addrlen);
  if (client_sockets[curr_clients] == -1) {
    olsr_printf(1, "(HTTPINFO) accept: %s\n", strerror(errno));
    goto close_connection;
  }

  if (!check_allowed_ip(allowed_nets, (union olsr_ip_addr *)&pin.sin_addr.s_addr)) {
    struct ipaddr_str strbuf;
    olsr_printf(0, "HTTP request from non-allowed host %s!\n",
                olsr_ip_to_string(&strbuf, (union olsr_ip_addr *)&pin.sin_addr.s_addr));
    close(client_sockets[curr_clients]);
  }

  addr = inet_ntoa(pin.sin_addr);

  memset(req, 0, sizeof(req));
  memset(body, 0, sizeof(body));

  while ((r = recv(client_sockets[curr_clients], &req[c], 1, 0)) > 0 && (c < sizeof(req) - 1)) {
    c++;

    if ((c > 3 && !strcmp(&req[c - 4], "\r\n\r\n")) || (c > 1 && !strcmp(&req[c - 2], "\n\n")))
      break;
  }

  if (r < 0) {
    olsr_printf(1, "(HTTPINFO) Failed to recieve data from client!\n");
    stats.err_hits++;
    goto close_connection;
  }

  /* Get the request */
  if (sscanf(req, "%10s %250s %10s\n", req_type, filename, http_version) != 3) {
    /* Try without HTTP version */
    if (sscanf(req, "%10s %250s\n", req_type, filename) != 2) {
      olsr_printf(1, "(HTTPINFO) Error parsing request %s!\n", req);
      stats.err_hits++;
      goto close_connection;
    }
  }

  olsr_printf(1, "Request: %s\nfile: %s\nVersion: %s\n\n", req_type, filename, http_version);

  if (!strcmp(req_type, "POST")) {
#ifdef ADMIN_INTERFACE
    int i = 0;
    while (dynamic_files[i].filename) {
      printf("POST checking %s\n", dynamic_files[i].filename);
      if (FILENREQ_MATCH(filename, dynamic_files[i].filename)) {
        uint32_t param_size;

        stats.ok_hits++;

        param_size = recv(client_sockets[curr_clients], req, sizeof(req) - 1, 0);

        req[param_size] = '\0';
        printf("Dynamic read %d bytes\n", param_size);

        //memcpy(body, dynamic_files[i].data, static_bin_files[i].data_size);
        size += dynamic_files[i].process_data_cb(req, param_size, &body[size], sizeof(body) - size);
        c = build_http_header(HTTP_OK, true, size, req, sizeof(req));
        goto send_http_data;
      }
      i++;
    }
#endif
    /* We only support GET */
    strscpy(body, HTTP_400_MSG, sizeof(body));
    stats.ill_hits++;
    c = build_http_header(HTTP_BAD_REQ, true, strlen(body), req, sizeof(req));
  } else if (!strcmp(req_type, "GET")) {
    int i = 0;

    for (i = 0; static_bin_files[i].filename; i++) {
      if (FILENREQ_MATCH(filename, static_bin_files[i].filename)) {
        break;
      }
    }

    if (static_bin_files[i].filename) {
      stats.ok_hits++;
      memcpy(body, static_bin_files[i].data, static_bin_files[i].data_size);
      size = static_bin_files[i].data_size;
      c = build_http_header(HTTP_OK, false, size, req, sizeof(req));
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
      size += snprintf(&body[size], sizeof(body) - size, "%s", static_txt_files[i].data);
      c = build_http_header(HTTP_OK, false, size, req, sizeof(req));
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
      c = build_http_header(HTTP_OK, true, size, req, sizeof(req));
      r = send(client_sockets[curr_clients], req, c, 0);
      if (r < 0) {
        olsr_printf(1, "(HTTPINFO) Failed sending data to client!\n");
        goto close_connection;
      }
      netsprintf_error = 0;
      netsprintf_direct = 1;
#endif
      size +=
        snprintf(&body[size], sizeof(body) - size,
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

      size += build_tabs(&body[size], sizeof(body) - size, i);
      size += build_frame(&body[size], sizeof(body) - size, "Current Routes", "routes", FRAMEWIDTH, tab_entries[i].build_body_cb);

      stats.ok_hits++;

      size +=
        snprintf(&body[size], sizeof(body) - size,
                 "</table>\n" "<div id=\"footer\">\n" "<center>\n" "(C)2005 Andreas T&oslash;nnesen<br/>\n"
                 "<a href=\"http://www.olsr.org/\">http://www.olsr.org</a>\n" "</center>\n" "</div>\n" "</body>\n" "</html>\n");

#ifdef NETDIRECT
      netsprintf_direct = 1;
      goto close_connection;
#else
      c = build_http_header(HTTP_OK, true, size, req, sizeof(req));
      goto send_http_data;
#endif
    }

    stats.ill_hits++;
    strscpy(body, HTTP_404_MSG, sizeof(body));
    c = build_http_header(HTTP_BAD_FILE, true, strlen(body), req, sizeof(req));
  } else {
    /* We only support GET */
    strscpy(body, HTTP_400_MSG, sizeof(body));
    stats.ill_hits++;
    c = build_http_header(HTTP_BAD_REQ, true, strlen(body), req, sizeof(req));
  }

send_http_data:

  r = writen(client_sockets[curr_clients], req, c);
  if (r < 0) {
    olsr_printf(1, "(HTTPINFO) Failed sending data to client!\n");
    goto close_connection;
  }

  r = writen(client_sockets[curr_clients], body, size);
  if (r < 0) {
    olsr_printf(1, "(HTTPINFO) Failed sending data to client!\n");
    goto close_connection;
  }

close_connection:
  close(client_sockets[curr_clients]);
  curr_clients--;

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

static int
build_tabs(char *buf, const uint32_t bufsize, int active)
{
  int size = 0, tabs = 0;

  size +=
    snprintf(&buf[size], bufsize - size,
             "<table align=\"center\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" width=\"%d\">\n"
             "<tr bgcolor=\"#ffffff\"><td>\n" "<ul id=\"tabnav\">\n", FRAMEWIDTH);
  for (tabs = 0; tab_entries[tabs].tab_label; tabs++) {
    if (!tab_entries[tabs].display_tab) {
      continue;
    }
    size +=
      snprintf(&buf[size], bufsize - size, "<li><a href=\"%s\"%s>%s</a></li>\n", tab_entries[tabs].filename,
               tabs == active ? " class=\"active\"" : "", tab_entries[tabs].tab_label);
  }
  size += snprintf(&buf[size], bufsize - size, "</ul>\n" "</td></tr>\n" "<tr><td>\n");
  return size;
}

/*
 * destructor - called at unload
 */
void
olsr_plugin_exit(void)
{
  if (http_socket >= 0) {
    CLOSE(http_socket);
  }
}

static int
section_title(char *buf, uint32_t bufsize, const char *title)
{
  return snprintf(buf, bufsize,
                  "<h2>%s</h2>\n" "<table width=\"100%%\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\">\n",
                  title);
}

static int
build_frame(char *buf, uint32_t bufsize, const char *title __attribute__ ((unused)), const char *link
            __attribute__ ((unused)), int width __attribute__ ((unused)), build_body_callback frame_body_cb)
{
  int size = 0;
  size += snprintf(&buf[size], bufsize - size, "<div id=\"maintable\">\n");
  size += frame_body_cb(&buf[size], bufsize - size);
  size += snprintf(&buf[size], bufsize - size, "</div>\n");
  return size;
}

static int
fmt_href(char *buf, const uint32_t bufsize, const char *const ipaddr)
{
  return snprintf(buf, bufsize, "<a href=\"http://%s:%d/all\">", ipaddr, http_port);
}

static int
build_ip_txt(char *buf, const uint32_t bufsize, const bool print_link, const char *const ipaddrstr, const int prefix_len)
{
  int size = 0;

  if (print_link) {
    size += fmt_href(&buf[size], bufsize - size, ipaddrstr);
  }

  size += snprintf(&buf[size], bufsize - size, "%s", ipaddrstr);
  /* print ip address or ip prefix ? */
  if (prefix_len != -1 && prefix_len != olsr_cnf->maxplen) {
    size += snprintf(&buf[size], bufsize - size, "/%d", prefix_len);
  }

  if (print_link) {             /* Print the link only if there is no prefix_len */
    size += snprintf(&buf[size], bufsize - size, "</a>");
  }
  return size;
}

static int
build_ipaddr_link(char *buf, const uint32_t bufsize, const bool want_link, const union olsr_ip_addr *const ipaddr,
                  const int prefix_len)
{
  int size = 0;
  struct ipaddr_str ipaddrstr;
  const struct hostent *const hp =
#ifndef WIN32
    resolve_ip_addresses ? gethostbyaddr(ipaddr, olsr_cnf->ipsize,
                                         olsr_cnf->ip_version) :
#endif
    NULL;
  /* Print the link only if there is no prefix_len */
  const int print_link = want_link && (prefix_len == -1 || prefix_len == olsr_cnf->maxplen);
  olsr_ip_to_string(&ipaddrstr, ipaddr);

  size += snprintf(&buf[size], bufsize - size, "<td>");
  size += build_ip_txt(&buf[size], bufsize - size, print_link, ipaddrstr.buf, prefix_len);
  size += snprintf(&buf[size], bufsize - size, "</td>");

  if (resolve_ip_addresses) {
    if (hp) {
      size += snprintf(&buf[size], bufsize - size, "<td>(");
      if (print_link) {
        size += fmt_href(&buf[size], bufsize - size, ipaddrstr.buf);
      }
      size += snprintf(&buf[size], bufsize - size, "%s", hp->h_name);
      if (print_link) {
        size += snprintf(&buf[size], bufsize - size, "</a>");
      }
      size += snprintf(&buf[size], bufsize - size, ")</td>");
    } else {
      size += snprintf(&buf[size], bufsize - size, "<td/>");
    }
  }
  return size;
}

#define build_ipaddr_with_link(buf, bufsize, ipaddr, plen) \
          build_ipaddr_link((buf), (bufsize), true, (ipaddr), (plen))
#define build_ipaddr_no_link(buf, bufsize, ipaddr, plen) \
          build_ipaddr_link((buf), (bufsize), false, (ipaddr), (plen))

static int
build_route(char *buf, uint32_t bufsize, const struct rt_entry *rt)
{
  int size = 0;
  struct lqtextbuffer lqbuffer;

  size += snprintf(&buf[size], bufsize - size, "<tr>");
  size += build_ipaddr_with_link(&buf[size], bufsize - size, &rt->rt_dst.prefix, rt->rt_dst.prefix_len);
  size += build_ipaddr_with_link(&buf[size], bufsize - size, &rt->rt_best->rtp_nexthop.gateway, -1);

  size += snprintf(&buf[size], bufsize - size, "<td>%d</td>", rt->rt_best->rtp_metric.hops);
  size +=
    snprintf(&buf[size], bufsize - size, "<td>%s</td>",
             get_linkcost_text(rt->rt_best->rtp_metric.cost, true, &lqbuffer));
  size +=
    snprintf(&buf[size], bufsize - size, "<td>%s</td></tr>\n",
             if_ifwithindex_name(rt->rt_best->rtp_nexthop.iif_index));
  return size;
}

static int
build_routes_body(char *buf, uint32_t bufsize)
{
  int size = 0;
  struct rt_entry *rt;
  const char *colspan = resolve_ip_addresses ? " colspan=\"2\"" : "";
  size += section_title(&buf[size], bufsize - size, "OLSR Routes in Kernel");
  size +=
    snprintf(&buf[size], bufsize - size,
             "<tr><th%s>Destination</th><th%s>Gateway</th><th>Metric</th><th>ETX</th><th>Interface</th></tr>\n",
             colspan, colspan);

  /* Walk the route table */
  OLSR_FOR_ALL_RT_ENTRIES(rt) {
    size += build_route(&buf[size], bufsize - size, rt);
  } OLSR_FOR_ALL_RT_ENTRIES_END(rt);

  size += snprintf(&buf[size], bufsize - size, "</table>\n");

  return size;
}

static int
build_config_body(char *buf, uint32_t bufsize)
{
  int size = 0;
  const struct olsr_if *ifs;
  const struct plugin_entry *pentry;
  const struct plugin_param *pparam;
  struct ipaddr_str mainaddrbuf;

  size += snprintf(&buf[size], bufsize - size, "Version: %s (built on %s on %s)\n<br>", olsrd_version, build_date, build_host);
  size += snprintf(&buf[size], bufsize - size, "OS: %s\n<br>", OS);

  {
    const time_t currtime = time(NULL);
    const int rc = strftime(&buf[size], bufsize - size,
                            "System time: <em>%a, %d %b %Y %H:%M:%S</em><br>",
                            localtime(&currtime));
    if (rc > 0) {
      size += rc;
    }
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

    size += snprintf(&buf[size], bufsize - size, "Olsrd uptime: <em>");
    if (days) {
      size += snprintf(&buf[size], bufsize - size, "%d day(s) ", days);
    }
    size +=
      snprintf(&buf[size], bufsize - size, "%02d hours %02d minutes %02d seconds</em><br/>\n", hours, mins, (int)uptime.tv_sec);
  }

  size +=
    snprintf(&buf[size], bufsize - size, "HTTP stats(ok/dyn/error/illegal): <em>%d/%d/%d/%d</em><br>\n", stats.ok_hits,
             stats.dyn_hits, stats.err_hits, stats.ill_hits);

  size +=
    snprintf(&buf[size], bufsize - size,
             "Click <a href=\"/cfgfile\">here</a> to <em>generate a configuration file for this node</em>.\n");

  size += snprintf(&buf[size], bufsize - size, "<h2>Variables</h2>\n");

  size += snprintf(&buf[size], bufsize - size, "<table width=\"100%%\" border=\"0\">\n<tr>");

  size +=
    snprintf(&buf[size], bufsize - size, "<td>Main address: <strong>%s</strong></td>\n",
             olsr_ip_to_string(&mainaddrbuf, &olsr_cnf->main_addr));
  size += snprintf(&buf[size], bufsize - size, "<td>IP version: %d</td>\n", olsr_cnf->ip_version == AF_INET ? 4 : 6);
  size += snprintf(&buf[size], bufsize - size, "<td>Debug level: %d</td>\n", olsr_cnf->debug_level);
  size +=
    snprintf(&buf[size], bufsize - size, "<td>FIB Metrics: %s</td>\n",
             FIBM_FLAT == olsr_cnf->fib_metric ? CFG_FIBM_FLAT : FIBM_CORRECT ==
             olsr_cnf->fib_metric ? CFG_FIBM_CORRECT : CFG_FIBM_APPROX);

  size += snprintf(&buf[size], bufsize - size, "</tr>\n<tr>\n");

  size += snprintf(&buf[size], bufsize - size, "<td>Pollrate: %0.2f</td>\n", olsr_cnf->pollrate);
  size += snprintf(&buf[size], bufsize - size, "<td>TC redundancy: %d</td>\n", olsr_cnf->tc_redundancy);
  size += snprintf(&buf[size], bufsize - size, "<td>MPR coverage: %d</td>\n", olsr_cnf->mpr_coverage);
  size += snprintf(&buf[size], bufsize - size, "<td>NAT threshold: %f</td>\n", olsr_cnf->lq_nat_thresh);

  size += snprintf(&buf[size], bufsize - size, "</tr>\n<tr>\n");

  size += snprintf(&buf[size], bufsize - size, "<td>Fisheye: %s</td>\n", olsr_cnf->lq_fish ? "Enabled" : "Disabled");
  size += snprintf(&buf[size], bufsize - size, "<td>TOS: 0x%04x</td>\n", olsr_cnf->tos);
  size += snprintf(&buf[size], bufsize - size, "<td>RtTable: 0x%04x/%d</td>\n", olsr_cnf->rttable, olsr_cnf->rttable);
  size +=
    snprintf(&buf[size], bufsize - size, "<td>RtTableDefault: 0x%04x/%d</td>\n", olsr_cnf->rttable_default,
             olsr_cnf->rttable_default);
  size +=
    snprintf(&buf[size], bufsize - size, "<td>Willingness: %d %s</td>\n", olsr_cnf->willingness,
             olsr_cnf->willingness_auto ? "(auto)" : "");

  if (olsr_cnf->lq_level == 0) {
    size +=
      snprintf(&buf[size], bufsize - size, "</tr>\n<tr>\n" "<td>Hysteresis: %s</td>\n",
               olsr_cnf->use_hysteresis ? "Enabled" : "Disabled");
    if (olsr_cnf->use_hysteresis) {
      size += snprintf(&buf[size], bufsize - size, "<td>Hyst scaling: %0.2f</td>\n", olsr_cnf->hysteresis_param.scaling);
      size +=
        snprintf(&buf[size], bufsize - size, "<td>Hyst lower/upper: %0.2f/%0.2f</td>\n", olsr_cnf->hysteresis_param.thr_low,
                 olsr_cnf->hysteresis_param.thr_high);
    }
  }

  size +=
    snprintf(&buf[size], bufsize - size, "</tr>\n<tr>\n" "<td>LQ extension: %s</td>\n",
             olsr_cnf->lq_level ? "Enabled" : "Disabled");
  if (olsr_cnf->lq_level) {
    size +=
      snprintf(&buf[size], bufsize - size, "<td>LQ level: %d</td>\n" "<td>LQ aging: %f</td>\n", olsr_cnf->lq_level,
               olsr_cnf->lq_aging);
  }
  size += snprintf(&buf[size], bufsize - size, "</tr></table>\n");

  size += snprintf(&buf[size], bufsize - size, "<h2>Interfaces</h2>\n");
  size += snprintf(&buf[size], bufsize - size, "<table width=\"100%%\" border=\"0\">\n");
  for (ifs = olsr_cnf->interfaces; ifs != NULL; ifs = ifs->next) {
    const struct interface *const rifs = ifs->interf;
    size += snprintf(&buf[size], bufsize - size, "<tr><th colspan=\"3\">%s</th>\n", ifs->name);
    if (!rifs) {
      size += snprintf(&buf[size], bufsize - size, "<tr><td colspan=\"3\">Status: DOWN</td></tr>\n");
      continue;
    }

    if (olsr_cnf->ip_version == AF_INET) {
      struct ipaddr_str addrbuf, maskbuf, bcastbuf;
      size +=
        snprintf(&buf[size], bufsize - size, "<tr>\n" "<td>IP: %s</td>\n" "<td>MASK: %s</td>\n" "<td>BCAST: %s</td>\n" "</tr>\n",
                 ip4_to_string(&addrbuf, rifs->int_addr.sin_addr), ip4_to_string(&maskbuf, rifs->int_netmask.sin_addr),
                 ip4_to_string(&bcastbuf, rifs->int_broadaddr.sin_addr));
    } else {
      struct ipaddr_str addrbuf, maskbuf;
      size +=
        snprintf(&buf[size], bufsize - size, "<tr>\n" "<td>IP: %s</td>\n" "<td>MCAST: %s</td>\n" "<td></td>\n" "</tr>\n",
                 ip6_to_string(&addrbuf, &rifs->int6_addr.sin6_addr), ip6_to_string(&maskbuf, &rifs->int6_multaddr.sin6_addr));
    }
    size +=
      snprintf(&buf[size], bufsize - size, "<tr>\n" "<td>MTU: %d</td>\n" "<td>WLAN: %s</td>\n" "<td>STATUS: UP</td>\n" "</tr>\n",
               rifs->int_mtu, rifs->is_wireless ? "Yes" : "No");
  }
  size += snprintf(&buf[size], bufsize - size, "</table>\n");

  size +=
    snprintf(&buf[size], bufsize - size, "<em>Olsrd is configured to %s if no interfaces are available</em><br>\n",
             olsr_cnf->allow_no_interfaces ? "run even" : "halt");

  size += snprintf(&buf[size], bufsize - size, "<h2>Plugins</h2>\n");
  size += snprintf(&buf[size], bufsize - size, "<table width=\"100%%\" border=\"0\"><tr><th>Name</th><th>Parameters</th></tr>\n");
  for (pentry = olsr_cnf->plugins; pentry; pentry = pentry->next) {
    size +=
      snprintf(&buf[size], bufsize - size, "<tr><td>%s</td>\n" "<td><select>\n" "<option>KEY, VALUE</option>\n", pentry->name);

    for (pparam = pentry->params; pparam; pparam = pparam->next) {
      size += snprintf(&buf[size], bufsize - size, "<option>\"%s\", \"%s\"</option>\n", pparam->key, pparam->value);
    }
    size += snprintf(&buf[size], bufsize - size, "</select></td></tr>\n");

  }
  size += snprintf(&buf[size], bufsize - size, "</table>\n");

  size += section_title(&buf[size], bufsize - size, "Announced HNA entries");
  if (olsr_cnf->hna_entries) {
    struct ip_prefix_list *hna;
    size += snprintf(&buf[size], bufsize - size, "<tr><th>Network</th></tr>\n");
    for (hna = olsr_cnf->hna_entries; hna; hna = hna->next) {
      struct ipaddr_str netbuf;
      size +=
        snprintf(&buf[size], bufsize - size, "<tr><td>%s/%d</td></tr>\n", olsr_ip_to_string(&netbuf, &hna->net.prefix),
                 hna->net.prefix_len);
    }
  } else {
    size += snprintf(&buf[size], bufsize - size, "<tr><td></td></tr>\n");
  }
  size += snprintf(&buf[size], bufsize - size, "</table>\n");
  return size;
}

static int
build_neigh_body(char *buf, uint32_t bufsize)
{
  struct neighbor_entry *neigh;
  struct link_entry *link = NULL;
  int size = 0;
  const char *colspan = resolve_ip_addresses ? " colspan=\"2\"" : "";

  size += section_title(&buf[size], bufsize - size, "Links");

  size +=
    snprintf(&buf[size], bufsize - size,
             "<tr><th%s>Local IP</th><th%s>Remote IP</th><th>Hysteresis</th>",
             colspan, colspan);
  if (olsr_cnf->lq_level > 0) {
    size += snprintf(&buf[size], bufsize - size, "<th>LinkCost</th>");
  }
  size += snprintf(&buf[size], bufsize - size, "</tr>\n");

  /* Link set */
  OLSR_FOR_ALL_LINK_ENTRIES(link) {
    size += snprintf(&buf[size], bufsize - size, "<tr>");
    size += build_ipaddr_with_link(&buf[size], bufsize, &link->local_iface_addr, -1);
    size += build_ipaddr_with_link(&buf[size], bufsize, &link->neighbor_iface_addr, -1);
    size += snprintf(&buf[size], bufsize - size, "<td>%0.2f</td>", link->L_link_quality);
    if (olsr_cnf->lq_level > 0) {
      struct lqtextbuffer lqbuffer1, lqbuffer2;
      size +=
        snprintf(&buf[size], bufsize - size, "<td>(%s) %s</td>", get_link_entry_text(link, '/', &lqbuffer1),
                 get_linkcost_text(link->linkcost, false, &lqbuffer2));
    }
    size += snprintf(&buf[size], bufsize - size, "</tr>\n");
  } OLSR_FOR_ALL_LINK_ENTRIES_END(link);

  size += snprintf(&buf[size], bufsize - size, "</table>\n");

  size += section_title(&buf[size], bufsize - size, "Neighbors");
  size +=
    snprintf(&buf[size], bufsize - size,
             "<tr><th%s>IP Address</th><th>SYM</th><th>MPR</th><th>MPRS</th><th>Willingness</th><th>2 Hop Neighbors</th></tr>\n",
             colspan);
  /* Neighbors */
  OLSR_FOR_ALL_NBR_ENTRIES(neigh) {

    struct neighbor_2_list_entry *list_2;
    int thop_cnt;
    size += snprintf(&buf[size], bufsize - size, "<tr>");
    size += build_ipaddr_with_link(&buf[size], bufsize, &neigh->neighbor_main_addr, -1);
    size +=
      snprintf(&buf[size], bufsize - size,
               "<td>%s</td>" "<td>%s</td>" "<td>%s</td>"
               "<td>%d</td>", (neigh->status == SYM) ? "YES" : "NO", neigh->is_mpr ? "YES" : "NO",
               olsr_lookup_mprs_set(&neigh->neighbor_main_addr) ? "YES" : "NO", neigh->willingness);

    size += snprintf(&buf[size], bufsize - size, "<td><select>\n" "<option>IP ADDRESS</option>\n");

    for (list_2 = neigh->neighbor_2_list.next, thop_cnt = 0; list_2 != &neigh->neighbor_2_list; list_2 = list_2->next, thop_cnt++) {
      struct ipaddr_str strbuf;
      size +=
        snprintf(&buf[size], bufsize - size, "<option>%s</option>\n",
                 olsr_ip_to_string(&strbuf, &list_2->neighbor_2->neighbor_2_addr));
    }
    size += snprintf(&buf[size], bufsize - size, "</select> (%d)</td></tr>\n", thop_cnt);
  } OLSR_FOR_ALL_NBR_ENTRIES_END(neigh);

  size += snprintf(&buf[size], bufsize - size, "</table>\n");
  return size;
}

static int
build_topo_body(char *buf, uint32_t bufsize)
{
  int size = 0;
  struct tc_entry *tc;
  const char *colspan = resolve_ip_addresses ? " colspan=\"2\"" : "";

  size += section_title(&buf[size], bufsize - size, "Topology Entries");
  size +=
    snprintf(&buf[size], bufsize - size, "<tr><th%s>Destination IP</th><th%s>Last Hop IP</th>",
             colspan, colspan);
  if (olsr_cnf->lq_level > 0) {
    size += snprintf(&buf[size], bufsize - size, "<th>Linkcost</th>");
  }
  size += snprintf(&buf[size], bufsize - size, "</tr>\n");

  OLSR_FOR_ALL_TC_ENTRIES(tc) {
    struct tc_edge_entry *tc_edge;
    OLSR_FOR_ALL_TC_EDGE_ENTRIES(tc, tc_edge) {
      if (tc_edge->edge_inv) {
        size += snprintf(&buf[size], bufsize - size, "<tr>");
        size += build_ipaddr_with_link(&buf[size], bufsize, &tc_edge->T_dest_addr, -1);
        size += build_ipaddr_with_link(&buf[size], bufsize, &tc->addr, -1);
        if (olsr_cnf->lq_level > 0) {
          struct lqtextbuffer lqbuffer1, lqbuffer2;
          size +=
            snprintf(&buf[size], bufsize - size, "<td>(%s) %s</td>\n",
                     get_tc_edge_entry_text(tc_edge, '/', &lqbuffer1), get_linkcost_text(tc_edge->cost, false, &lqbuffer2));
        }
        size += snprintf(&buf[size], bufsize - size, "</tr>\n");
      }
    } OLSR_FOR_ALL_TC_EDGE_ENTRIES_END(tc, tc_edge);
  } OLSR_FOR_ALL_TC_ENTRIES_END(tc);

  size += snprintf(&buf[size], bufsize - size, "</table>\n");

  return size;
}

static int
build_mid_body(char *buf, uint32_t bufsize)
{
  int size = 0;
  int idx;
  const char *colspan = resolve_ip_addresses ? " colspan=\"2\"" : "";

  size += section_title(&buf[size], bufsize - size, "MID Entries");
  size += snprintf(&buf[size], bufsize - size, "<tr><th%s>Main Address</th><th>Aliases</th></tr>\n", colspan);

  /* MID */
  for (idx = 0; idx < HASHSIZE; idx++) {
    struct mid_entry *entry;
    for (entry = mid_set[idx].next; entry != &mid_set[idx]; entry = entry->next) {
      int mid_cnt;
      struct mid_address *alias;
      size += snprintf(&buf[size], bufsize - size, "<tr>");
      size += build_ipaddr_with_link(&buf[size], bufsize, &entry->main_addr, -1);
      size += snprintf(&buf[size], bufsize - size, "<td><select>\n<option>IP ADDRESS</option>\n");

      for (mid_cnt = 0, alias = entry->aliases; alias != NULL; alias = alias->next_alias, mid_cnt++) {
        struct ipaddr_str strbuf;
        size += snprintf(&buf[size], bufsize - size, "<option>%s</option>\n", olsr_ip_to_string(&strbuf, &alias->alias));
      }
      size += snprintf(&buf[size], bufsize - size, "</select> (%d)</td></tr>\n", mid_cnt);
    }
  }

  size += snprintf(&buf[size], bufsize - size, "</table>\n");
  return size;
}

static int
build_nodes_body(char *buf, uint32_t bufsize)
{
  int size = 0;

  size += build_neigh_body(&buf[size], bufsize - size);
  size += build_topo_body(&buf[size], bufsize - size);
  size += build_mid_body(&buf[size], bufsize - size);

  return size;
}

static int
build_all_body(char *buf, uint32_t bufsize)
{
  int size = 0;

  size += build_config_body(&buf[size], bufsize - size);
  size += build_routes_body(&buf[size], bufsize - size);
  size += build_neigh_body(&buf[size], bufsize - size);
  size += build_topo_body(&buf[size], bufsize - size);
  size += build_mid_body(&buf[size], bufsize - size);

  return size;
}

static int
build_about_body(char *buf, uint32_t bufsize)
{
  return snprintf(buf, bufsize,
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

static int
build_cfgfile_body(char *buf, uint32_t bufsize)
{
  int size = 0;

  size +=
    snprintf(&buf[size], bufsize - size,
             "\n\n" "<strong>This is a automatically generated configuration\n"
             "file based on the current olsrd configuration of this node.<br/>\n" "<hr/>\n" "<pre>\n");

#ifdef NETDIRECT
  {
    /* Hack to make netdirect stuff work with
       olsrd_write_cnf_buf
     */
    char tmpBuf[10000];
    size = olsrd_write_cnf_buf(olsr_cnf, tmpBuf, 10000);
    snprintf(&buf[size], bufsize - size, tmpBuf);
  }
#else
  size += olsrd_write_cnf_buf(olsr_cnf, &buf[size], bufsize - size);
#endif

  if (size < 0) {
    size = snprintf(buf, size, "ERROR GENERATING CONFIGFILE!\n");
  }

  size += snprintf(&buf[size], bufsize - size, "</pre>\n<hr/>\n");

#if 0
  printf("RETURNING %d\n", size);
#endif
  return size;
}

static int
check_allowed_ip(const struct allowed_net *const allowed_nets, const union olsr_ip_addr *const addr)
{
  const struct allowed_net *alln;
  for (alln = allowed_nets; alln != NULL; alln = alln->next) {
    if ((addr->v4.s_addr & alln->mask.v4.s_addr) == (alln->net.v4.s_addr & alln->mask.v4.s_addr)) {
      return 1;
    }
  }
  return 0;
}

#if 0

/*
 * In a bigger mesh, there are probs with the fixed
 * bufsize. Because the Content-Length header is
 * optional, the sprintf() is changed to a more
 * scalable solution here.
 */

int
netsprintf(char *str, const char *format, ...)
{
  va_list arg;
  int rv;
  va_start(arg, format);
  rv = vsprintf(str, format, arg);
  va_end(arg);
  if (0 != netsprintf_direct) {
    if (0 == netsprintf_error) {
      if (0 > send(client_sockets[curr_clients], str, rv, 0)) {
        olsr_printf(1, "(HTTPINFO) Failed sending data to client!\n");
        netsprintf_error = 1;
      }
    }
    return 0;
  }
  return rv;
}
#endif

static ssize_t
writen(int fd, const void *buf, size_t count)
{
  size_t bytes_left = count;
  const char *p = buf;
  while (bytes_left > 0) {
    const ssize_t written = write(fd, p, bytes_left);
    if (written == -1) {        /* error */
      if (errno == EINTR) {
        continue;
      }
      return -1;
    }
    /* We wrote something */
    bytes_left -= written;
    p += written;
  }
  return count;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
