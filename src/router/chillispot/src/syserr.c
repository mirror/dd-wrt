/* 
 * Syslog functions.
 * Copyright (C) 2003, 2004 Mondru AB.
 * 
 * The contents of this file may be used under the terms of the GNU
 * General Public License Version 2, provided that the above copyright
 * notice and this permission notice is included in all copies or
 * substantial portions of the software.
 * 
 */

#include <stdarg.h>
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
