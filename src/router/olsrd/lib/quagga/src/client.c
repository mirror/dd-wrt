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

/* -------------------------------------------------------------------------
 * File               : client.c
 * Description        : zebra client functions
 * ------------------------------------------------------------------------- */

#define HAVE_SOCKLEN_T

#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include "defs.h"
#include "olsr.h"
#include "log.h"
#include "routing_table.h"
#include "common/autobuf.h"

#include "common.h"
#include "quagga.h"
#include "packet.h"
#include "client.h"

static void zclient_connect(void);

static void
zclient_connect(void)
{
  int ret;

  union {
    struct sockaddr_in sin;
    struct sockaddr_un sun;
  } sockaddr;

  if (close(zebra.sock) < 0)
    olsr_exit("QUAGGA: Could not close socket", EXIT_FAILURE);

  zebra.sock = socket(zebra.port ? AF_INET : AF_UNIX, SOCK_STREAM, 0);

  if (zebra.sock < 0)
    olsr_exit("QUAGGA: Could not create socket", EXIT_FAILURE);

  memset(&sockaddr, 0, sizeof sockaddr);

  if (zebra.port) {
    sockaddr.sin.sin_family = AF_INET;
    sockaddr.sin.sin_port = htons(zebra.port);
    sockaddr.sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    ret = connect(zebra.sock, (struct sockaddr *)&sockaddr.sin, sizeof sockaddr.sin);
  } else {
    sockaddr.sun.sun_family = AF_UNIX;
    strscpy(sockaddr.sun.sun_path, zebra.sockpath, sizeof(sockaddr.sun.sun_path));
    ret = connect(zebra.sock, (struct sockaddr *)&sockaddr.sun, sizeof sockaddr.sun);
  }

  if (ret < 0)
    zebra.status &= ~STATUS_CONNECTED;
  else
    zebra.status |= STATUS_CONNECTED;

}

void
zclient_reconnect(void)
{
  struct rt_entry *tmp;

  zclient_connect();
  if (!(zebra.status & STATUS_CONNECTED))
    return;                     // try again next time

  zebra_hello(ZEBRA_HELLO);
  if (zebra.options & OPTION_EXPORT) {
    OLSR_FOR_ALL_RT_ENTRIES(tmp) {
      zebra_addroute(tmp);
    }
    OLSR_FOR_ALL_RT_ENTRIES_END(tmp);
  }
  zebra_redistribute(ZEBRA_REDISTRIBUTE_ADD);

}

int
zclient_write(unsigned char *options)
{
  unsigned char *pnt;
  uint16_t len;
  int ret;

  if (!(zebra.status & STATUS_CONNECTED))
    return 0;

  pnt = options;
  memcpy(&len, pnt, sizeof len);

  len = ntohs(len);

  do {
    ret = write(zebra.sock, pnt, len);
    if (ret < 0) {
      if ((errno == EINTR) || (errno == EAGAIN)) {
        errno = 0;
        ret = 0;
        continue;
      } else {
        OLSR_PRINTF(1, "(QUAGGA) Disconnected from zebra.\n");
        zebra.status &= ~STATUS_CONNECTED;
        /* TODO: Remove HNAs added from redistribution */
        free(options);
        return -1;
      }
    }
    pnt = pnt + ret;
  }
  while ((len -= ret));
  free(options);

  return 0;
}

unsigned char *
zclient_read(ssize_t * size)
{
  unsigned char *buf = NULL;
  ssize_t bufsize = 0;
  uint16_t offset = 0;
  int socket_flags_saved = fcntl(zebra.sock, F_GETFL);

  /* initialise size */
  *size = 0;

  /* set non-blocking on the socket */
  (void) fcntl(zebra.sock, F_SETFL, socket_flags_saved | O_NONBLOCK);

  /* read complete packets */
  do {
    ssize_t bytes_received;
    uint16_t packet_length;

    /* (re)allocate buffer */
    if (*size == bufsize) {
      ssize_t start = bufsize;
      bufsize += AUTOBUFCHUNK;
      buf = olsr_realloc(buf, bufsize, "QUAGGA: grow read buffer");
      memset(&buf[start], 0, AUTOBUFCHUNK);
    }

    /* read from socket */
    bytes_received = read(zebra.sock, &buf[*size], bufsize - *size);

    /* handle invalid packet */
    if (!bytes_received) {
      goto error_out;
    }

    /* handle no data available */
    if (bytes_received < 0) {
      /* handle disconnect */
#if EWOULDBLOCK == EAGAIN
      if (errno != EAGAIN) { // oops - we got disconnected
#else
      if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) { // oops - we got disconnected
#endif
        OLSR_PRINTF(1, "(QUAGGA) Disconnected from zebra\n");
        zebra.status &= ~STATUS_CONNECTED;
        /* TODO: Remove HNAs added from redistribution */
      }

      goto error_out;
    }

    *size += bytes_received;

    /* detect zebra packet fragmentation */
    while (*size >= (ssize_t) (offset + sizeof(packet_length))) {
      packet_length = ntohs(*((uint16_t *)(void *) &buf[offset]));
      offset += packet_length;
    }
  } while (*size != offset);

  out: (void) fcntl(zebra.sock, F_SETFL, socket_flags_saved);
  return buf;

  error_out: free(buf);
  buf = NULL;
  *size = 0;
  goto out;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
