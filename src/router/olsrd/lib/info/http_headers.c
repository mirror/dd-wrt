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

#include "http_headers.h"

#include <assert.h>

void http_header_build_result(unsigned int status, struct autobuf *abuf) {
  assert(abuf);

  /* Status */
  abuf_appendf(abuf, "%s %s\r\n", INFO_HTTP_VERSION, httpStatusToReply(status));

  /* End header */
  abuf_puts(abuf, "\r\n");
}

void http_header_build(const char *plugin_name, unsigned int status, const char *mime, struct autobuf *abuf, int *contentLengthIndex) {
  assert(plugin_name);
  assert(abuf);
  assert(contentLengthIndex);

  /* Status */
  abuf_appendf(abuf, "%s %s\r\n", INFO_HTTP_VERSION, httpStatusToReply(status));

  /* Date */
  {
    time_t currtime;
    char buf[128];

    time(&currtime);
    if (strftime(buf, sizeof(buf), "Date: %a, %d %b %Y %H:%M:%S GMT\r\n", gmtime(&currtime))) {
      abuf_puts(abuf, buf);
    }
  }

  /* Server version */
  abuf_appendf(abuf, "Server: OLSRD %s\r\n", plugin_name);

  /* connection-type */
  abuf_puts(abuf, "Connection: close\r\n");

  /* MIME type */
  if (mime != NULL) {
    abuf_appendf(abuf, "Content-Type: %s\r\n", mime);
  }

  /* CORS data */
  /* No needs to be strict here, access control is based on source IP */
  abuf_puts(abuf, "Access-Control-Allow-Origin: *\r\n");
  abuf_puts(abuf, "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n");
  abuf_puts(abuf, "Access-Control-Allow-Headers: Accept, Origin, X-Requested-With\r\n");
  abuf_puts(abuf, "Access-Control-Max-Age: 1728000\r\n");

  /* Content length */
  abuf_puts(abuf, "Content-Length: ");
  *contentLengthIndex = abuf->len;
  abuf_puts(abuf, "            "); /* 12 spaces reserved for the length (max. 1TB-1), to be filled at the end */
  abuf_puts(abuf, "\r\n");

  /* Cache-control
   * No caching dynamic pages
   */
  abuf_puts(abuf, "Cache-Control: no-cache\r\n");

  /* End header */
  abuf_puts(abuf, "\r\n");
}

void http_header_adjust_content_length(struct autobuf *abuf, int contentLengthIndex, int contentLength) {
  char buf[12 + 1]; /* size must match to number of spaces used (+1 for the terminating byte) */

  assert(abuf);

  memset(buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%d", contentLength);
  buf[sizeof(buf) - 1] = '\0';
  memcpy(&abuf->buf[contentLengthIndex], buf, strlen(buf));
}
