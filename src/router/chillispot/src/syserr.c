/* 
 * Syslog functions.
 *
 * Copyright (c) 2006, Jens Jakobsen 
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 *   Neither the names of copyright holders nor the names of its contributors
 *   may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * Copyright (C) 2003, 2004 Mondru AB.
 * 
 * The contents of this file may be used under the terms of the GNU
 * General Public License Version 2, provided that the above copyright
 * notice and this permission notice is included in all copies or
 * substantial portions of the software.
 * 
 */

#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "syserr.h"


void sys_err(int pri, char *fn, int ln, int en, char *fmt, ...) {
  va_list args;
  char buf[SYSERR_MSGSIZE];

  va_start(args, fmt);
  vsnprintf(buf, SYSERR_MSGSIZE, fmt, args);
  va_end(args);
  buf[SYSERR_MSGSIZE-1] = 0; /* Make sure it is null terminated */
  if (en)
    syslog(pri, "%s: %d: %d (%s) %s", fn, ln, en, strerror(en), buf);
  else
    syslog(pri, "%s: %d: %s", fn, ln, buf);
}

void sys_errpack(int pri, char *fn, int ln, int en, struct sockaddr_in *peer,
		 void *pack, unsigned len, char *fmt, ...) {
  
  va_list args;
  char buf[SYSERR_MSGSIZE];
  char buf2[SYSERR_MSGSIZE];
  int n;
  int pos;
  
  va_start(args, fmt);
  vsnprintf(buf, SYSERR_MSGSIZE, fmt, args);
  va_end(args);
  buf[SYSERR_MSGSIZE-1] = 0;

  snprintf(buf2, SYSERR_MSGSIZE, "Packet from %s:%u, length: %d, content:",
	   inet_ntoa(peer->sin_addr),
	   ntohs(peer->sin_port),
	   len);
  buf2[SYSERR_MSGSIZE-1] = 0;
  pos = strlen(buf2);
  for(n=0; n<len; n++) {
    if ((pos+4)<SYSERR_MSGSIZE) {
      sprintf((buf2+pos), " %02hhx", ((unsigned char*)pack)[n]);
      pos += 3;
    }
  }
  buf2[pos] = 0;
  
  if (en)
    syslog(pri, "%s: %d: %d (%s) %s. %s", fn, ln, en, strerror(en), buf, buf2);
  else
    syslog(pri, "%s: %d: %s. %s", fn, ln, buf, buf2);

}
