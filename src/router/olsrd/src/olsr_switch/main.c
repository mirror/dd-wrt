
/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2005, Andreas Tonnesen(andreto@olsr.org)
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

/* olsrd host-switch daemon */

#include "olsr_host_switch.h"
#include "link_rules.h"
#include "ohs_cmd.h"
#include "ipcalc.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#ifdef WIN32
#undef errno
#define errno WSAGetLastError()
#undef strerror
#define strerror(x) StrError(x)
#define close(x) closesocket(x)
#else
#include <sys/wait.h>
#endif

static int srv_socket;

#define OHS_BUFSIZE 1500
static uint8_t data_buffer[OHS_BUFSIZE];

struct ohs_connection *ohs_conns;

//static int ip_version;
//int ipsize;
static struct olsrd_config olsr_cnf_data;
struct olsrd_config *olsr_cnf = &olsr_cnf_data;

uint32_t logbits;

/* local functions */
static int ohs_init_new_connection(int);

static int ohs_route_data(struct ohs_connection *);

static int ohs_init_connect_sockets(void);

static int ohs_configure(void);

#if !defined WIN32
static void ohs_listen_loop(void) __attribute__ ((noreturn));
#else
static void ohs_listen_loop(void);
#endif

#ifdef WIN32
int __stdcall
ohs_close(unsigned long signal __attribute__ ((unused)))
#else
void
ohs_close(int signal __attribute__ ((unused)))
#endif
{
  printf("OHS: exit\n");

  close(srv_socket);

  exit(0);
}

struct ohs_connection *
get_client_by_addr(const union olsr_ip_addr *adr)
{
  struct ohs_connection *oc;
  for (oc = ohs_conns; oc != NULL; oc = oc->next) {
    if (ipequal(adr, &oc->ip_addr)) {
      return oc;
    }
  }
  return NULL;
}

static int
ohs_init_new_connection(int s)
{
  struct ohs_connection *oc;
  int i;

  if (logbits & LOG_CONNECT) {
    printf("ohs_init_new_connection\n");
  }
  /* Create new client node */
  oc = calloc(1, sizeof(struct ohs_connection));
  if (!oc) {
    OHS_OUT_OF_MEMORY("New connection");
  }

  oc->socket = s;
  oc->links = NULL;
  oc->rx = 0;
  oc->tx = 0;
  oc->linkcnt = 0;

  // hack alert: WSAEventSelect makes sockets non-blocking, so the
  // recv() may return without having read anything on Windows; hence
  // re-try for 2 seconds on Windows; shouldn't harm Linux et al.

  /* Get "fake IP" */
  for (i = 0; i < 20; i++) {
    /* Win32 needs that cast. */
    if (recv(oc->socket, (void *)&oc->ip_addr, olsr_cnf->ipsize, 0) == (int)olsr_cnf->ipsize) {
      break;
    }
#if defined WIN32
    Sleep(100);
#endif
  }

  if (i == 20) {
    printf("Failed to fetch IP address! (%s)\n", strerror(errno));
    return -1;
  }

  if (logbits & LOG_CONNECT) {
    struct ipaddr_str addrstr;
    printf("IP: %s\n", olsr_ip_to_string(&addrstr, &oc->ip_addr));
  }
  if (get_client_by_addr(&oc->ip_addr)) {
    if (logbits & LOG_CONNECT) {
      struct ipaddr_str addrstr;
      printf("IP: %s DUPLICATE! Disconecting client!\n", olsr_ip_to_string(&addrstr, &oc->ip_addr));
    }
    close(s);
    free(oc);
    return -1;
  }

  /* Queue */
  oc->next = ohs_conns;
  ohs_conns = oc;
  return 1;
}

int
ohs_delete_connection(struct ohs_connection *oc)
{
  if (!oc) {
    return -1;
  }
  /* Close the socket */
  close(oc->socket);

  if (logbits & LOG_CONNECT) {
    struct ipaddr_str addrstr;
    printf("Removing entry %s\n", olsr_ip_to_string(&addrstr, &oc->ip_addr));
  }
  /* De-queue */
  if (oc == ohs_conns) {
    ohs_conns = ohs_conns->next;
  } else {
    struct ohs_connection *curr_entry = ohs_conns->next;
    struct ohs_connection *prev_entry = ohs_conns;

    while (curr_entry != NULL) {
      if (curr_entry == oc) {
        prev_entry->next = curr_entry->next;
        break;
      }
      prev_entry = curr_entry;
      curr_entry = curr_entry->next;
    }
  }
  ohs_delete_all_related_links(oc);

  free(oc);
  return 0;
}

static int
ohs_route_data(struct ohs_connection *oc)
{
  struct ohs_connection *ohs_cs;
  ssize_t len;
  int cnt = 0;

  oc->tx++;
  /* Read data */
  if ((len = recv(oc->socket, data_buffer, OHS_BUFSIZE, 0)) <= 0)
    return -1;

  if (logbits & LOG_FORWARD) {
    struct ipaddr_str addrstr;
    printf("Received %ld bytes from %s\n", (long)len, olsr_ip_to_string(&addrstr, &oc->ip_addr));
  }
  /* Loop trough clients */
  for (ohs_cs = ohs_conns; ohs_cs; ohs_cs = ohs_cs->next) {
    /* Check that the link is active open */
    if (ohs_check_link(oc, &ohs_cs->ip_addr) && oc->socket != ohs_cs->socket) {
      ssize_t sent;

      /* Send link addr */
      if (send(ohs_cs->socket, (const void *)&oc->ip_addr, olsr_cnf->ipsize, 0) != (int)olsr_cnf->ipsize) {
        printf("Error sending link address!\n");
      }
      /* Send data */
      if (logbits & LOG_FORWARD) {
        struct ipaddr_str addrstr, addrstr2;
        printf("Sending %d bytes %s=>%s\n", (int)len, olsr_ip_to_string(&addrstr, &oc->ip_addr),
               olsr_ip_to_string(&addrstr2, &ohs_cs->ip_addr));
      }

      sent = send(ohs_cs->socket, data_buffer, len, 0);
      if (sent != len) {
        printf("Error sending(buf %d != sent %d)\n", (int)len, (int)sent);
      }
      ohs_cs->rx++;
      cnt++;
    }
  }
  return cnt;
}

static int
ohs_init_connect_sockets(void)
{
  uint32_t yes = 1;
  struct sockaddr_in sin;

  printf("Initiating socket TCP port %d\n", OHS_TCP_PORT);

  if ((srv_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Could not initialize socket(%d): %s\n", srv_socket, strerror(errno));
    exit(0);
  }

  if (setsockopt(srv_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof(yes)) < 0) {
    printf("SO_REUSEADDR failed for socket: %s\n", strerror(errno));
    close(srv_socket);
    exit(0);
  }

  /* complete the socket structure */
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(OHS_TCP_PORT);

  /* bind the socket to the port number */
  if (bind(srv_socket, (struct sockaddr *)&sin, sizeof(sin)) == -1) {
    printf("bind failed for socket: %s\n", strerror(errno));
    close(srv_socket);
    exit(0);
  }

  /* show that we are willing to listen */
  if (listen(srv_socket, 5) == -1) {
    printf("listen failed for socket: %s\n", strerror(errno));
    close(srv_socket);
    exit(0);
  }
  return 1;
}

static int
ohs_configure(void)
{

  return 1;
}

static void
accept_handler(void)
{
  struct sockaddr_in pin;
  socklen_t addrlen = sizeof(pin);
  int s;

  memset(&pin, 0, sizeof(pin));

  if ((s = accept(srv_socket, (struct sockaddr *)&pin, &addrlen)) < 0) {
    printf("accept failed socket: %s\n", strerror(errno));
  } else {
    /* Create new node */
    ohs_init_new_connection(s);
  }
}

static void
stdin_handler(void)
{
  ohs_parse_command();
}

static void
read_handler(struct ohs_connection *con)
{
  if (ohs_route_data(con) < 0)
    ohs_delete_connection(con);
}

static void
ohs_listen_loop(void)
{
#if !defined WIN32
  int n;
  fd_set ibits;
  int fn_stdin = fileno(stdin);

  while (1) {
    int high;

    struct ohs_connection *ohs_cs;

    high = 0;
    FD_ZERO(&ibits);

    /* Add server socket */
    high = srv_socket;
    FD_SET(srv_socket, &ibits);

    if (fn_stdin > high)
      high = fn_stdin;

    FD_SET(fn_stdin, &ibits);

    /* Add clients */
    for (ohs_cs = ohs_conns; ohs_cs; ohs_cs = ohs_cs->next) {
      if (ohs_cs->socket > high)
        high = ohs_cs->socket;

      FD_SET(ohs_cs->socket, &ibits);
    }

    /* block */
    n = select(high + 1, &ibits, 0, 0, NULL);

    if (n == 0)
      continue;

    /* Did somethig go wrong? */
    if (n < 0) {
      if (errno == EINTR)
        continue;

      printf("Error select: %s", strerror(errno));
      continue;
    }

    /* Check server socket */
    if (FD_ISSET(srv_socket, &ibits))
      accept_handler();

    /* Loop trough clients */
    ohs_cs = ohs_conns;
    while (ohs_cs) {
      struct ohs_connection *ohs_tmp = ohs_cs;
      ohs_cs = ohs_cs->next;

      if (FD_ISSET(ohs_tmp->socket, &ibits))
        read_handler(ohs_tmp);
    }

    if (FD_ISSET(fn_stdin, &ibits))
      stdin_handler();

  }
#else
  HANDLE Objects[2];
  WSANETWORKEVENTS NetEvents;
  struct ohs_connection *Walker, *TmpWalker;
  unsigned int Res;

  Objects[0] = GetStdHandle(STD_INPUT_HANDLE);
  Objects[1] = WSACreateEvent();

  if (WSAEventSelect(srv_socket, Objects[1], FD_ACCEPT) == SOCKET_ERROR) {
    fprintf(stderr, "WSAEventSelect failed (1): %s\n", strerror(errno));
    return;
  }

  while (1) {
    for (Walker = ohs_conns; Walker != NULL; Walker = Walker->next) {
      if (WSAEventSelect(Walker->socket, Objects[1], FD_READ | FD_CLOSE) == SOCKET_ERROR) {
        fprintf(stderr, "WSAEventSelect failed (2): %s\n", strerror(errno));
        Sleep(1000);
        continue;
      }
    }

    Res = WaitForMultipleObjects(2, Objects, FALSE, INFINITE);

    if (Res == WAIT_FAILED) {
      fprintf(stderr, "WaitForMultipleObjects failed: %s\n", strerror(GetLastError()));
      Sleep(1000);
      continue;
    }

    if (Res == WAIT_OBJECT_0)
      stdin_handler();

    else if (Res == WAIT_OBJECT_0 + 1) {
      if (WSAEnumNetworkEvents(srv_socket, Objects[1], &NetEvents) == SOCKET_ERROR)
        fprintf(stderr, "WSAEnumNetworkEvents failed (1): %s\n", strerror(errno));

      else {
        if ((NetEvents.lNetworkEvents & FD_ACCEPT) != 0)
          accept_handler();
      }

      for (Walker = ohs_conns; Walker != NULL; Walker = TmpWalker) {
        TmpWalker = Walker->next;

        if (WSAEnumNetworkEvents(Walker->socket, Objects[1], &NetEvents) == SOCKET_ERROR)
          fprintf(stderr, "WSAEnumNetworkEvents failed (2): %s\n", strerror(errno));

        else {
          if ((NetEvents.lNetworkEvents & (FD_READ | FD_CLOSE)) != 0)
            read_handler(Walker);
        }
      }
    }
  }

#endif
}

int
main(void)
{

#ifdef WIN32
  WSADATA WsaData;

  if (WSAStartup(0x0202, &WsaData)) {
    fprintf(stderr, "Could not initialize WinSock.\n");
    exit(EXIT_FAILURE);
  }

  SetConsoleCtrlHandler(ohs_close, true);

#else
  signal(SIGINT, ohs_close);
  signal(SIGTERM, ohs_close);

  /* Avoid zombie children */
  signal(SIGCHLD, SIG_IGN);
#endif

  printf("olsrd host-switch daemon version %s starting\n", OHS_VERSION);

  logbits = LOG_DEFAULT;
  olsr_cnf->ip_version = AF_INET;
  olsr_cnf->ipsize = sizeof(struct in_addr);

  srand((unsigned int)time(NULL));

  ohs_set_olsrd_path(OHS_DEFAULT_OLSRD_PATH);

  ohs_init_connect_sockets();

  ohs_configure();

  printf("OHS command interpreter reading from STDIN\n");
  printf("\n> ");
  fflush(stdout);

  ohs_listen_loop();

  ohs_close(0);

  return 1;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
