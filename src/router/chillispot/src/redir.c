/*
 * HTTP redirection functions.
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
 * Copyright (C) 2004, 2005 Mondru AB.
 *
 * The contents of this file may be used under the terms of the GNU
 * General Public License Version 2, provided that the above copyright
 * notice and this permission notice is included in all copies or
 * substantial portions of the software.
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <errno.h>
#include <stdarg.h>

#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>

#include <sys/ipc.h>
#include <sys/msg.h>

#include <time.h>
#include <sys/time.h>

#include <signal.h>

#include "syserr.h"
#include "radius.h"
#include "radius_wispr.h"
#include "radius_chillispot.h"
#include "redir.h"
#include "md5.h"
#include "../config.h"

static int optionsdebug = 0; /* TODO: Should be changed to instance */

static int keep_going = 1;   /* OK as global variable for child process */

static int termstate = REDIR_TERM_INIT;    /* When we were terminated */

char credits[] =
"<H1>ChilliSpot " VERSION "</H1><p>Copyright 2002-2005 Mondru AB</p><p> "
"ChilliSpot is an Open Source captive portal or wireless LAN access point "
"controller developed by the community at "
"<a href=\"http://www.chillispot.org\">www.chillispot.org</a> and licensed "
"under the GPL.</p><p>ChilliSpot acknowledges all community members, "
"especially those mentioned at "
"<a href=\"http://www.chillispot.org/credits.html\">http://www.chillispot.org/credits.html</a>.";


/* Termination handler for clean shutdown */
static void redir_termination( int signum) {
  if (optionsdebug) printf("Terminating redir client!\n");
  keep_going = 0;
}

/* Alarm handler for ensured shutdown */
static void redir_alarm( int signum) {
  sys_err(LOG_WARNING, __FILE__, __LINE__, 0,
	  "Client process timed out: %d", termstate);
  exit(0);
}


/* Generate a 16 octet random challenge */
static int redir_challenge(unsigned char *dst) {
  FILE *file;


  if ((file = fopen("/dev/urandom", "r")) == NULL) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "fopen(/dev/urandom, r) failed");
    return -1;
  }
  
  if (fread(dst, 1, REDIR_MD5LEN, file) != REDIR_MD5LEN) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "fread() failed");
    return -1;
  }
  
  fclose(file);
  return 0;
}


/* Convert 32+1 octet ASCII hex string to 16 octet unsigned char */
static int redir_hextochar(char *src, unsigned char * dst) {

  char x[3];
  int n;
  int y;
  
  for (n=0; n< REDIR_MD5LEN; n++) {
    x[0] = src[n*2+0];
    x[1] = src[n*2+1];
    x[2] = 0;
    if (sscanf (x, "%2x", &y) != 1) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0, "HEX conversion failed!");
      return -1;
    }
    dst[n] = (unsigned char) y;
  }

  return 0;
}

/* Convert 16 octet unsigned char to 32+1 octet ASCII hex string */
static int redir_chartohex(unsigned char *src, char *dst) {

  char x[3];
  int n;
  
  for (n=0; n<REDIR_MD5LEN; n++) {
    snprintf(x, 3, "%.2x", src[n]);
    dst[n*2+0] = x[0];
    dst[n*2+1] = x[1];
  }
  dst[REDIR_MD5LEN*2] = 0;
  return 0;
}


/* Encode src as urlencoded and place null terminated result in dst */
static int redir_urlencode( char *src, int srclen, char *dst, int dstsize) {

  char x[3];
  int n;
  int i = 0;
  
  for (n=0; n<srclen; n++) {
    if ((('A' <= src[n]) && (src[n] <= 'Z')) ||
	(('a' <= src[n]) && (src[n] <= 'z')) ||
	(('0' <= src[n]) && (src[n] <= '9')) ||
	('-' == src[n]) ||
	('_' == src[n]) ||
	('.' == src[n]) ||
	('!' == src[n]) ||
	('~' == src[n]) ||
	('*' == src[n]) ||
	('\'' == src[n]) ||
	('(' == src[n]) ||
	(')' == src[n])) {
      if (i<dstsize-1) {
	dst[i++] = src[n];
      }
    }
    else {
      snprintf(x, 3, "%.2x", src[n]);
      if (i<dstsize-3) {
	dst[i++] = '%';
	dst[i++] = x[0];
	dst[i++] = x[1];
      }
    }
  }
  dst[i] = 0;
  return 0;
}

/* Decode urlencoded src and place null terminated result in dst */
static int redir_urldecode(  char *src, int srclen, char *dst, int dstsize) {

  char x[3];
  int n = 0;
  int i = 0;
  unsigned int c;

  while (n<srclen) {
    if (src[n] == '%') {
      if ((n+2) < srclen) {
	x[0] = src[n+1];
	x[1] = src[n+2];
	x[2] = 0;
	c = '_';
	sscanf(x, "%x", &c);
	if (i<(dstsize-1)) dst[i++] = c; 
      }
      n += 3;
    }
    else {
      if (i<(dstsize-1)) dst[i++] = src[n];
      n++;
    }
  }
  dst[i] = 0;
  return 0;
}

/* Concatenate src to dst and place result dst */
static int redir_stradd(char *dst, int dstsize, char *fmt, ...) {
  va_list args;
  char buf[REDIR_MAXBUFFER];

  va_start(args, fmt);
  vsnprintf(buf, REDIR_MAXBUFFER, fmt, args);
  va_end(args);
  buf[REDIR_MAXBUFFER-1] = 0; /* Make sure it is null terminated */

  if ((strlen(dst) + strlen(buf)) > dstsize-1) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0, "redir_stradd() failed");
    return -1;
  }

  strcpy(dst + strlen(dst), buf);
  return 0;
}


/* Make an XML Reply */
static int redir_xmlreply(struct redir_t *redir, struct redir_conn_t *conn, 
			  int res, long int timeleft, char* hexchal, 
			  char* reply, char* redirurl,
			  char *dst, int dstsize) {
  char mid[REDIR_MAXBUFFER];

  snprintf(dst, dstsize,
	   "<!--\r\n"
	   "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
	   "<WISPAccessGatewayParam\r\n"
	   "  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\r\n"
	   "  xsi:noNamespaceSchemaLocation=\"http://www.acmewisp.com/WISPAccessGatewayParam.xsd\""
	   ">\r\n");
  dst[dstsize-1] = 0;

  switch (res) {
  case REDIR_ALREADY:
    redir_stradd(dst, dstsize, "<AuthenticationPollReply>\r\n");
    redir_stradd(dst, dstsize, "<MessageType>140</MessageType>\r\n");
    redir_stradd(dst, dstsize, "<ResponseCode>102</ResponseCode>\r\n");
    redir_stradd(dst, dstsize, 
		 "<ReplyMessage>Already logged on</ReplyMessage>\r\n");
    redir_stradd(dst, dstsize, "</AuthenticationPollReply>\r\n");
    break;
  case REDIR_FAILED_REJECT:
    redir_stradd(dst, dstsize, "<AuthenticationPollReply>\r\n");
    redir_stradd(dst, dstsize, "<MessageType>140</MessageType>\r\n");
    redir_stradd(dst, dstsize, "<ResponseCode>100</ResponseCode>\r\n");
    if (reply) {
      redir_stradd(dst, dstsize, "<ReplyMessage>%s</ReplyMessage>\r\n", reply);
    }
    else {
      redir_stradd(dst, dstsize, 
		   "<ReplyMessage>Invalid Password</ReplyMessage>\r\n");
    }
    redir_stradd(dst, dstsize, "</AuthenticationPollReply>\r\n");
    break;
  case REDIR_FAILED_OTHER:
    redir_stradd(dst, dstsize, "<AuthenticationPollReply>\r\n");
    redir_stradd(dst, dstsize, "<MessageType>140</MessageType>\r\n");
    redir_stradd(dst, dstsize, "<ResponseCode>102</ResponseCode>\r\n");
    if (reply) {
      redir_stradd(dst, dstsize, "<ReplyMessage>%s</ReplyMessage>\r\n", reply);
    }
    else {
      redir_stradd(dst, dstsize, 
		   "<ReplyMessage>Radius error</ReplyMessage>\r\n");
    }
    redir_stradd(dst, dstsize, "</AuthenticationPollReply>\r\n");
    break;
  case REDIR_SUCCESS:
    redir_stradd(dst, dstsize, "<AuthenticationPollReply>\r\n");
    redir_stradd(dst, dstsize, "<MessageType>140</MessageType>\r\n");
    redir_stradd(dst, dstsize, "<ResponseCode>50</ResponseCode>\r\n");
    if (reply) {
      redir_stradd(dst, dstsize, "<ReplyMessage>%s</ReplyMessage>\r\n", reply);
    }
    redir_stradd(dst, dstsize,
		 "<LogoffURL>http://%s:%d/logoff</LogoffURL>\r\n",
		 inet_ntoa(redir->addr), redir->port);
    if (redirurl) {
      redir_stradd(dst, dstsize,
		   "<RedirectionURL>%s</RedirectionURL>\r\n", redirurl);
    }
    redir_stradd(dst, dstsize, "</AuthenticationPollReply>\r\n");
    break;
  case REDIR_LOGOFF:
    redir_stradd(dst, dstsize, "<LogoffReply>\r\n");
    redir_stradd(dst, dstsize, "<MessageType>130</MessageType>\r\n");
    redir_stradd(dst, dstsize, "<ResponseCode>150</ResponseCode>\r\n");
    redir_stradd(dst, dstsize, "</LogoffReply>\r\n");
    break;
  case REDIR_NOTYET:
    redir_stradd(dst, dstsize, "<Redirect>\r\n");
    redir_stradd(dst, dstsize, "<AccessProcedure>1.0</AccessProcedure>\r\n");
    if (redir->radiuslocationid) {
      redir_stradd(dst, dstsize, 
		   "<AccessLocation>%s</AccessLocation>\r\n",
		   redir->radiuslocationid);
    }
    if (redir->radiuslocationname) {
      redir_stradd(dst, dstsize, 
	       "<LocationName>%s</LocationName>\r\n",
	       redir->radiuslocationname);
    }
    redir_stradd(dst, dstsize, 
		 "<LoginURL>%s?res=smartclient&uamip=%s&uamport=%d&challenge=%s</LoginURL>\r\n",
		 redir->url, inet_ntoa(redir->addr), redir->port, hexchal);
    redir_stradd(dst, dstsize, 
		 "<AbortLoginURL>http://%s:%d/abort</AbortLoginURL>\r\n",
		 inet_ntoa(redir->addr), redir->port);
    redir_stradd(dst, dstsize, "<MessageType>100</MessageType>\r\n");
    redir_stradd(dst, dstsize, "<ResponseCode>0</ResponseCode>\r\n");
    redir_stradd(dst, dstsize, "</Redirect>\r\n");
    break;
  case REDIR_ABORT_ACK:
    redir_stradd(dst, dstsize, "<AbortLoginReply>\r\n");
    redir_stradd(dst, dstsize, "<MessageType>150</MessageType>\r\n");
    redir_stradd(dst, dstsize, "<ResponseCode>151</ResponseCode>\r\n");
    redir_stradd(dst, dstsize, "</AbortLoginReply>\r\n");
    break;
  case REDIR_ABORT_NAK:
    redir_stradd(dst, dstsize, "<AbortLoginReply>\r\n");
    redir_stradd(dst, dstsize, "<MessageType>150</MessageType>\r\n");
    redir_stradd(dst, dstsize, "<ResponseCode>50</ResponseCode>\r\n");
    redir_stradd(dst, dstsize,
		 "<LogoffURL>http://%s:%d/logoff</LogoffURL>\r\n",
		 inet_ntoa(redir->addr), redir->port);
    redir_stradd(dst, dstsize, "</AbortLoginReply>\r\n");
    break;
  case REDIR_STATUS:
    redir_stradd(dst, dstsize, "<AuthenticationPollReply>\r\n");
    redir_stradd(dst, dstsize, "<MessageType>140</MessageType>\r\n");
    if (conn->authenticated != 1) {
      redir_stradd(dst, dstsize, "<ResponseCode>150</ResponseCode>\r\n");
      redir_stradd(dst, dstsize, 
		   "<ReplyMessage>Not logged on</ReplyMessage>\r\n");
    }
    else {
      redir_stradd(dst, dstsize, "<ResponseCode>50</ResponseCode>\r\n");
      redir_stradd(dst, dstsize,
		   "<ReplyMessage>Already logged on</ReplyMessage>\r\n");
    }
    redir_stradd(dst, dstsize, "</AuthenticationPollReply>\r\n");

    if (conn->authenticated == 1) {
      struct timeval timenow;
      uint32_t sessiontime;
      gettimeofday(&timenow, NULL);
      sessiontime = timenow.tv_sec - conn->start_time.tv_sec;
      sessiontime += (timenow.tv_usec - conn->start_time.tv_usec) / 1000000;
      redir_stradd(dst, dstsize, "<SessionStatus>\r\n");
      if (timeleft) {
	redir_stradd(dst, dstsize, "<SessionTimeLeft>%d</SessionTimeLeft>\r\n",
		     timeleft);
      }
      redir_stradd(dst, dstsize, "<Acct-Session-Time>%d</Acct-Session-Time>\r\n", sessiontime);
      redir_stradd(dst, dstsize, "<Start-Time>%d</Start-Time>\r\n" , conn->start_time);
      redir_stradd(dst, dstsize, "<Acct-Input-Octets>%d</Acct-Input-Octets>\r\n",
		   conn->input_octets);
      redir_stradd(dst, dstsize, "<Acct-Output-Octets>%d</Acct-Output-Octets>\r\n",
		   conn->output_octets);
      redir_stradd(dst, dstsize, "<Session-Timeout>%d</Session-Timeout>\r\n",
		   conn->sessiontimeout);
      redir_stradd(dst, dstsize, "<ChilliSpot-Max-Input-Octets>%d</ChilliSpot-Max-Input-Octets>\r\n",
		   conn->maxinputoctets);

      redir_stradd(dst, dstsize, "<ChilliSpot-Max-Output-Octets>%d</ChilliSpot-Max-Output-Octets>\r\n",
		   conn->maxoutputoctets);
      redir_stradd(dst, dstsize, "<ChilliSpot-Max-Total-Octets>%d</ChilliSpot-Max-Total-Octets>\r\n",
		   conn->maxtotaloctets);
      redir_stradd(dst, dstsize,
		   "<LogoffURL>http://%s:%d/logoff</LogoffURL>\r\n",
		   inet_ntoa(redir->addr), redir->port);
      redir_stradd(dst, dstsize,
		   "<StatusURL>http://%s:%d/status</StatusURL>\r\n",
		   inet_ntoa(redir->addr), redir->port);
      redir_stradd(dst, dstsize, "</SessionStatus>\r\n");
    }
    break;
  default:
    sys_err(LOG_ERR, __FILE__, __LINE__, 0, "Unknown res in switch");
    return -1;
  }
  
  redir_stradd(dst, dstsize, "</WISPAccessGatewayParam>\r\n");
  redir_stradd(dst, dstsize, "-->\r\n");
  return 0;
}

/* Make an HTTP redirection reply and send it to the client */
static int redir_reply(struct redir_t *redir, int fd, 
		       struct redir_conn_t *conn, int res, 
		       long int timeleft,
		       char* hexchal, char* uid, char* userurl, char* reply, 
		       char* redirurl, uint8_t *hismac) {
  char buffer[REDIR_MAXBUFFER];
  char xmlreply[REDIR_MAXBUFFER];
  char *resp = NULL;
  
  buffer[0] = 0;
  xmlreply[0] = 0;

  (void) redir_xmlreply(redir, conn, res, timeleft, hexchal, reply, redirurl,
			xmlreply, sizeof(xmlreply));
  
  switch (res) {
  case REDIR_ALREADY:
    resp = "already";
    break;
  case REDIR_FAILED_REJECT:
  case REDIR_FAILED_OTHER:
    resp = "failed";
    break;
  case REDIR_SUCCESS:
    resp = "success";
    break;
  case REDIR_LOGOFF:
    resp = "logoff";
    break;
  case REDIR_NOTYET:
    resp = "notyet";
    break;
  case REDIR_ABORT_ACK:
    resp = "logoff";
    break;
  case REDIR_ABORT_NAK:
    resp = "already";
    break;
  case REDIR_ABOUT:
    break;
  case REDIR_STATUS:
    if (conn->authenticated == 1)
      resp = "already";
    else
      resp = "notyet";
    break;
  default:
    sys_err(LOG_ERR, __FILE__, __LINE__, 0, "Unknown res in switch");
    return -1;
  }

  if (resp) {
    snprintf(buffer, sizeof(buffer), 
	     "HTTP/1.0 302 Moved Temporarily\r\n"
	     "Location: %s?res=%s&uamip=%s&uamport=%d", 
	     redir->url, resp, inet_ntoa(redir->addr), redir->port);
    buffer[sizeof(buffer)-1] = 0;
  }
  else {
    snprintf(buffer, sizeof(buffer), "HTTP/1.0 200 OK\r\n");
    buffer[sizeof(buffer)-1] = 0;
  }

  if (hexchal) {
    redir_stradd(buffer, sizeof(buffer), "&challenge=%s", hexchal);
  }
  
  if (uid) {
    char mid2[REDIR_MAXBUFFER];
    mid2[0] = 0;
    (void)redir_urlencode(uid, strlen(uid), mid2, sizeof(mid2));
    redir_stradd(buffer, sizeof(buffer), "&uid=%s", mid2);
  }
  
  if (timeleft) {
    redir_stradd(buffer, sizeof(buffer), "&timeleft=%ld", timeleft);
  }
  
  if (userurl) {
    char mid2[REDIR_MAXBUFFER];
    mid2[0] = 0;
    (void)redir_urlencode(userurl, strlen(userurl), mid2, sizeof(mid2));
    redir_stradd(buffer, sizeof(buffer), "&userurl=%s", mid2);
  }

  if (reply) {
    char mid2[REDIR_MAXBUFFER];
    mid2[0] = 0;
    (void)redir_urlencode(reply, strlen(reply), mid2, sizeof(mid2));
    redir_stradd(buffer, sizeof(buffer), "&reply=%s", mid2);
  }

  if (redir->radiusnasid) {
    char mid2[REDIR_MAXBUFFER];
    mid2[0] = 0;
    (void)redir_urlencode(redir->radiusnasid, strlen(redir->radiusnasid),
		    mid2, sizeof(mid2));
    redir_stradd(buffer, sizeof(buffer), "&nasid=%s", mid2);
  }

  if (redirurl) {
    char mid2[REDIR_MAXBUFFER];
    mid2[0] = 0;
    (void)redir_urlencode(redirurl, strlen(redirurl), 
		    mid2, sizeof(mid2));
    redir_stradd(buffer, sizeof(buffer), "&redirurl=%s", mid2);
  }

  if (hismac) {
    char mac[REDIR_MACSTRLEN+1];
    char mid2[REDIR_MAXBUFFER];
    mid2[0] = 0;
    snprintf(mac, REDIR_MACSTRLEN+1, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
	     hismac[0], hismac[1],
	     hismac[2], hismac[3],
	     hismac[4], hismac[5]);
    (void)redir_urlencode(mac, strlen(mac), mid2, sizeof(mid2));
    redir_stradd(buffer, sizeof(buffer), "&mac=%s", mid2);
  }
  
  redir_stradd(buffer, sizeof(buffer), 
	       "\r\n"
	       "Content-type: text/html"
	       "\r\n\r\n"
	       "<HTML>\r\n"
	       "<HEAD><TITLE>ChilliSpot</TITLE></HEAD>");
  redir_stradd(buffer, sizeof(buffer), credits);

  if (resp) {
    redir_stradd(buffer, sizeof(buffer), 
		 "<BODY><H2>Browser error!</H2>"
		 "Browser does not support redirects!</BODY>\r\n");
    redir_stradd(buffer, sizeof(buffer), xmlreply);
  }
  redir_stradd(buffer, sizeof(buffer), "</HTML>\r\n");

  if (optionsdebug) printf("redir_reply: Sending http reply: %s\n",
			   buffer);
  
  if (send(fd, buffer, strlen(buffer), 0) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno, "send() failed!");
    return -1;
  }
  return 0;
}

/* Allocate new instance of redir */
int redir_new(struct redir_t **redir,
	      struct in_addr *addr, int port) {
  
  struct sockaddr_in address;
  int optval = 1;
  int n = 0;

  /* Set up address */
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = addr->s_addr;
  address.sin_port = htons(port);
#if defined(__FreeBSD__)  || defined (__APPLE__)
  address.sin_len = sizeof (struct sockaddr_in);
#endif

  if (!(*redir = calloc(1, sizeof(struct redir_t)))) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno, "calloc() failed");
    return EOF;
  }

  (*redir)->addr = *addr;
  (*redir)->port = port;
  (*redir)->starttime = 0;
  
  if (((*redir)->fd  = socket(AF_INET ,SOCK_STREAM ,0)) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno, "socket() failed");
    return -1;
  }

  /* TODO: FreeBSD
  if (setsockopt((*redir)->fd, SOL_SOCKET, SO_REUSEPORT,
		 &optval, sizeof(optval))) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno, "setsockopt() failed");
    close((*redir)->fd);
    return -1;
  }
  */

  if (setsockopt((*redir)->fd, SOL_SOCKET, SO_REUSEADDR,
		 &optval, sizeof(optval))) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno, "setsockopt() failed");
    close((*redir)->fd);
    return -1;
  }
  
  while (bind((*redir)->fd, (struct sockaddr *)&address, sizeof(address))) {
    if ((EADDRINUSE == errno) && (10 > n++)) {
      sys_err(LOG_WARNING, __FILE__, __LINE__, 0, 
	      "UAM port already in use. Waiting for retry.");
      if (sleep(30)) { /* In case we got killed */
	close((*redir)->fd);
	return -1;
      }
    }
    else {
      sys_err(LOG_ERR, __FILE__, __LINE__, errno, "bind() failed");
      close((*redir)->fd);
      return -1;
    }
  }
  
  if (listen((*redir)->fd, REDIR_MAXLISTEN)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno, "listen() failed");
    close((*redir)->fd);
    return -1;
  }
  FILE *fp=fopen("/tmp/.chilli_id","rb");
  if (fp!=NULL)
    {
    int msgid;
    int count = fread(&msgid,4,1,fp);
    if (count>0)
	{
	if (msgctl(msgid, IPC_RMID, NULL)) {
	    sys_err(LOG_ERR, __FILE__, __LINE__, errno, "msgctl() failed");
	 }
	
	}
    fclose(fp);
    }
  
  if (((*redir)->msgid = msgget(IPC_PRIVATE, 0)) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno, "msgget() failed");
    sys_err(LOG_ERR, __FILE__, __LINE__, 0, 
	    "Most likely your computer does not have System V IPC installed");
    close((*redir)->fd);
    return -1;
  }
  fp=fopen("/tmp/.chilli_id","wb");
  fwrite(&(*redir)->msgid,4,1,fp);
  fclose(fp);
  
  return 0;
}


/* Free instance of redir */
int redir_free(struct redir_t *redir) {
  if (close(redir->fd)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno, "close() failed");
  }

  if (msgctl(redir->msgid, IPC_RMID, NULL)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno, "msgctl() failed");
  }
  
  free(redir);
  return 0;
}


/* Set redir parameters */
void redir_set(struct redir_t *redir, int debug,
	       char *url, char *homepage, char* secret,
	       struct in_addr *radiuslisten, 
	       struct in_addr *radiusserver0, struct in_addr *radiusserver1,
	       uint16_t radiusauthport, uint16_t radiusacctport,
	       char* radiussecret, char* radiusnasid,
	       struct in_addr *radiusnasip, char* radiuscalled,
	       char* radiuslocationid, char* radiuslocationname,
	       int radiusnasporttype) {

  optionsdebug = debug; /* TODO: Do not change static variable from instance */
  
  redir->debug = debug;
  redir->url = url;
  redir->homepage = homepage;
  redir->secret = secret;
  redir->radiusserver0 = *radiusserver0;
  redir->radiusserver1 = *radiusserver1;
  redir->radiusauthport = radiusauthport;
  redir->radiusacctport = radiusacctport;
  redir->radiussecret  = radiussecret;
  redir->radiusnasid  = radiusnasid;
  redir->radiusnasip  = *radiusnasip;
  redir->radiuscalled  = radiuscalled;
  redir->radiuslocationid  = radiuslocationid;
  redir->radiuslocationname  = radiuslocationname;
  redir->radiusnasporttype = radiusnasporttype;
  return;
}


/* Get the path of an HTTP request */
static int redir_getpath(struct redir_t *redir, char *src, char *dst, int dstsize) {

  char *p1;
  char *p2;
  char *p3;
  char *peol;
  int dstlen = 0;

  if (!(peol = strstr(src, "\n"))) /* End of the first line */
    return -1;

  if (!strncmp("GET ", src, 4)) {
    p1 = src + 4;
  }
  else if (!strncmp("HEAD ", src, 5)) {
    p1 = src + 5;
  }
  else {
    return -1;
  }

  while (*p1 == ' ') p1++; /* Advance through additional white space */
  
  if (*p1 == '/')
    p1++;
  else
    return -1;

  /* The path ends with a ? or a space */
  p2 = strstr(p1, "?");
  p3 = strstr(p1, " ");

  if ((p2 == NULL) && (p3 == NULL))  /* Not found at all */
    return -1;

  if ((p2 >= peol) && (p3 >= peol)) /* Not found on first line */
    return -1;

  if (p2 && !p3) {
    dstlen = p2-p1;
  } 
  else if (!p2 && p3) {
    dstlen = p3-p1;
  } 
  else if (p3>p2)
    dstlen = p2-p1;
  else
    dstlen = p3-p1;

  if (dstlen>=dstsize)
    return -1;

  strncpy(dst, p1, dstlen);
  dst[dstlen] = 0;

  /*printf("The path is: %s\n", dst); */

  return 0;

}

/* Get the url of an HTTP request */
static int redir_geturl(struct redir_t *redir, char *src, char *dst, int dstsize) {

  char *p1;
  char *p3;
  char *peol;
  char *path;
  int pathlen;
  char *host;
  int hostlen;

  dst[0] = 0; /* Null terminate in case of error return */
   
  if (!(peol = strstr(src, "\r\n"))) /* End of the line */
    return -1;

  /* HTTP Request can be 
     GET and HEAD: OK
     POST, PUT, DELETE, TRACE, CONNECT: Not OK
  */

  if (!strncmp("GET ", src, 4)) {
    p1 = src + 4;
  }
  else if (!strncmp("HEAD ", src, 5)) {
    p1 = src + 5;
  }
  else {
    return -1;
  }

  while (*p1 == ' ') p1++; /* Advance through additional white space */
  
  p3 = strstr(p1, " ");   /* The path ends with a space */

  if ((p3 == NULL) || (p3 >= peol))  /* Not found at all or at first line */
    return -1;

  path = p1;
  pathlen = p3-p1;

  if (!(p1 = strstr(p3, "\r\nHost:")))
    return -1;

  p1 += 7;
  while (*p1 == ' ') p1++; /* Advance through additional white space */
  
  if (!(peol = strstr(p1, "\r\n"))) /* End of the line */
    return -1;

  hostlen = peol-p1;
  host = p1;

  if ((7 + hostlen + pathlen)>=dstsize) {
    return -1;
  }

  strncpy(dst, "http://", 7);
  strncpy(dst+7, host, hostlen);
  strncpy(dst+7+hostlen, path, pathlen);
  dst[7 + hostlen + pathlen] = 0;

  if (optionsdebug) printf("Userurl: %s\n", dst);

  return 0;

}


/* Get a parameter of an HTTP request. Parameter is url decoded */
/* TODO: Should be merged with other parsers */
static int redir_getparam(struct redir_t *redir, char *src, 
		   char *param,
		   char *dst, int dstsize) {

  char *p1;
  char *p2;
  char *p3;
  char *peol;
  int dstlen = 0;
  char sstr[255];
  char mid[255];
  int len = 0;

  /* printf("Looking for: %s\n", param); TODO */

  if (!(peol = strstr(src, "\n"))) /* End of the first line */
    return -1;


  if (strncmp("GET ", src, 4)) {
    return -1;
  }

  strncpy(sstr, param, sizeof(sstr));
  sstr[sizeof(sstr)-1] = 0;
  strncat(sstr, "=", sizeof(sstr));
  sstr[sizeof(sstr)-1] = 0;

  if (!(p1 = strstr(src, sstr)))
    return -1;
  
  p1 += strlen(sstr);

  /* The parameter ends with a & or a space */
  p2 = strstr(p1, "&");
  p3 = strstr(p1, " ");

  /*printf("p1:\n%s\n\np2\n%s\n\np3:%s\n\n", p1, p2, p3);*/
  
  if ((p2 == NULL) && (p3 == NULL))  /* Not found at all */
    return -1;

  if ((p2 >= peol) && (p3 >= peol)) /* Not found on first line */
    return -1;

  if (p2 && !p3) {
    len = p2-p1;
  } 
  else if (!p2 && p3) {
    len = p3-p1;
  } 
  else if (p3>p2)
    len = p2-p1;
  else
    len = p3-p1;

  (void)redir_urldecode(p1, len, dst, dstsize);

  /*printf("The parameter is: %s\n", dst);*/

  return 0;

}

/* Read the an HTTP request from a client */
static int redir_getreq(struct redir_t *redir, int fd, struct redir_conn_t *conn) {

  int maxfd = 0;	        /* For select() */
  fd_set fds;			/* For select() */
  struct timeval idleTime;	/* How long to select() */
  int status;
  char buffer[REDIR_MAXBUFFER];
  int buflen = 0;
  int recvlen = 0;
  char path[REDIR_MAXBUFFER];
  char resp[REDIR_MAXBUFFER];
  char mid[REDIR_MAXBUFFER];
  int i;

  maxfd = fd;

  memset(buffer, 0, sizeof(buffer));
  memset(path, 0, sizeof(path));
  
  /* Read whatever the client send to us */
  while ((redir->starttime + REDIR_HTTP_MAX_TIME) > time(NULL)) {
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    idleTime.tv_sec = 0;
    idleTime.tv_usec = REDIR_HTTP_SELECT_TIME;

    switch (status = select(maxfd + 1, &fds, NULL, NULL, &idleTime)) {
    case -1:
      sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	      "select() returned -1!");
      return -1;
    case 0:
      break; 
    default:
      break;
    }
  
    if ((status > 0) && FD_ISSET(fd, &fds)) {
      if ((recvlen = 
	   recv(fd, buffer+buflen, sizeof(buffer)-1 - buflen, 0)) < 0) {
	if (errno != ECONNRESET)
	  sys_err(LOG_ERR, __FILE__, __LINE__, errno, "recv() failed!");
	return -1;
      }
      buflen += recvlen;
      buffer[buflen] = 0;
      if (strstr(buffer, "\n")) break; /* Only interested in first line */
    }
  }
  
  for (i = 0; i<buflen; i++) if (buffer[i] == 0) buffer[i] = 0x0a; /* TODO: Hack to make Flash work */

  if (buflen <= 0) {
    if (optionsdebug) printf("No HTTP request received!\n");
    return -1;
  }

  if (redir_getpath(redir, buffer, path, sizeof(path))) {
    if (optionsdebug) printf("Could not parse path!\n");
    return -1;
  }

  if (!redir_getparam(redir, buffer, "userurl",
		     conn->userurl, sizeof(conn->userurl))) {
    if (optionsdebug) printf("User URL: %s!\n", conn->userurl);
  }
  
  if ((!strcmp(path, "logon")) || (!strcmp(path, "login"))) {
    if (redir_getparam(redir, buffer, "username", 
		       conn->username, sizeof(conn->username))) {
      if (optionsdebug) printf("No username found!\n");
      return -1;
    }
    
    if (!redir_getparam(redir, buffer, "response",
			resp, sizeof(resp))) {
      (void)redir_hextochar(resp, conn->chappassword);
      conn->chap = 1;
      conn->password[0] = 0;
    }
    else if (!redir_getparam(redir, buffer, "password",
			     resp, sizeof(resp))) {
      (void)redir_hextochar(resp, conn->password);
      conn->chap = 0;
      conn->chappassword[0] = 0;

    } else {
      if (optionsdebug) printf("No password found!\n");
      return -1;
    }

    conn->type = REDIR_LOGIN;
    return 0;
  }
  else if ((!strcmp(path, "logoff")) || (!strcmp(path, "logout"))) {
    conn->type = REDIR_LOGOUT;
    return 0;
  } 
  else if (!strncmp(path, "msdownload", 10)) {
    conn->type = REDIR_MSDOWNLOAD;
    return 0;
  }
  else if (!strcmp(path, "prelogin")) {
    conn->type = REDIR_PRELOGIN;
    return 0;
  }
  else if (!strcmp(path, "abort")) {
    conn->type = REDIR_ABORT;
    return 0;
  }
  else if (!strcmp(path, "about")) {
    conn->type = REDIR_ABOUT;
    return 0;
  }
  else if (!strcmp(path, "status")) {
    conn->type = REDIR_STATUS;
    return 0;
  }
  else {
    if (redir_geturl(redir, buffer, conn->userurl, sizeof(conn->userurl))) {
      if (optionsdebug) printf("Could not parse URL!\n");
      return -1;
    }
    return 0;
  }
}

/* Radius callback when access accept/reject/challenge has been received */
static int redir_cb_radius_auth_conf(struct radius_t *radius,
			      struct radius_packet_t *pack,
			      struct radius_packet_t *pack_req, void *cbp) {
  
  struct radius_attr_t *interimattr = NULL;
  struct radius_attr_t *stateattr = NULL;
  struct radius_attr_t *classattr = NULL;
  struct radius_attr_t *attr = NULL;
  char attrs[RADIUS_ATTR_VLEN+1];
  struct tm stt;
  int tzhour, tzmin;
  char *tz;
  int result;
  struct redir_conn_t *conn = (struct redir_conn_t*) cbp;


  if (optionsdebug)
    printf("Received access request confirmation from radius server\n");
  
  if (!conn) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "No peer protocol defined");
    return 0;
  }
  
  if (!pack) { /* Timeout */
    sys_err(LOG_ERR, __FILE__, __LINE__, 0, "Radius request timed out");
    conn->response = REDIR_FAILED_OTHER;
    return 0;
  }

  /* We expect ACCESS-ACCEPT, ACCESS-REJECT (or ACCESS-CHALLENGE) */
  if ((pack->code != RADIUS_CODE_ACCESS_REJECT) && 
      (pack->code != RADIUS_CODE_ACCESS_CHALLENGE) &&
      (pack->code != RADIUS_CODE_ACCESS_ACCEPT)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0, 
	    "Unknown radius access reply code %d", pack->code);
    conn->response = REDIR_FAILED_OTHER;
    return 0;
  }

  /* Reply message (might be present in both ACCESS-ACCEPT and ACCESS-REJECT */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_REPLY_MESSAGE, 0, 0, 0)) {
    memcpy(conn->replybuf, attr->v.t, attr->l-2);
    conn->replybuf[attr->l-2] = 0;
    conn->reply = conn->replybuf;
  }
  else {
    conn->replybuf[0] = 0;
    conn->reply = NULL;
  }
  
  /* ACCESS-ACCEPT */
  if (pack->code != RADIUS_CODE_ACCESS_ACCEPT) {
    conn->response = REDIR_FAILED_REJECT;
    return 0;
  }

  /* Get Service Type */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_SERVICE_TYPE, 0, 0, 0)) {
    if(ntohl(attr->v.i) == RADIUS_SERVICE_TYPE_CHILLISPOT_AUTHORIZE_ONLY) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Chillispot-Authorize-Only Service-Type in Access-Accept");
      conn->response = REDIR_FAILED_REJECT;
      return 0;
    }
  }

  /* State */
  if (!radius_getattr(pack, &stateattr, RADIUS_ATTR_STATE, 0, 0, 0)) {
    conn->statelen = stateattr->l-2;
    memcpy(conn->statebuf, stateattr->v.t, stateattr->l-2);
  }
  else {
    conn->statelen = 0;
  }
  
  /* Class */
  if (!radius_getattr(pack, &classattr, RADIUS_ATTR_CLASS, 0, 0, 0)) {
    conn->classlen = classattr->l-2;
    memcpy(conn->classbuf, classattr->v.t, classattr->l-2);
  }
  else {
    conn->classlen = 0;
  }


  /* Session timeout */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_SESSION_TIMEOUT,
		      0, 0, 0)) {
    conn->sessiontimeout = ntohl(attr->v.i);
  }
  else {
    conn->sessiontimeout = 0;
  }

  /* Idle timeout */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_IDLE_TIMEOUT,
		      0, 0, 0)) {
    conn->idletimeout = ntohl(attr->v.i);
  }
  else {
    conn->idletimeout = 0;
  }

  /* Filter ID */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_FILTER_ID,
		      0, 0, 0)) {
    conn->filteridlen = attr->l-2;
    memcpy(conn->filteridbuf, attr->v.t, attr->l-2);
    conn->filteridbuf[attr->l-2] = 0;
    conn->filterid = conn->filteridbuf;
  }
  else {
    conn->filteridlen = 0;
    conn->filteridbuf[0] = 0;
    conn->filterid = NULL;
  }

  /* Interim interval */
  if (!radius_getattr(pack, &interimattr, RADIUS_ATTR_ACCT_INTERIM_INTERVAL, 
		      0, 0, 0)) {
    conn->interim_interval = ntohl(interimattr->v.i);
    if (conn->interim_interval < 60) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Received too small radius Acct-Interim-Interval value: %d. Disabling interim accounting",
	      conn->interim_interval);
      conn->interim_interval = 0;
    } 
    else if (conn->interim_interval < 600) {
      sys_err(LOG_WARNING, __FILE__, __LINE__, 0,
	      "Received small radius Acct-Interim-Interval value: %d",
	      conn->interim_interval);
    }
  }
  else {
    conn->interim_interval = 0;
  }


  /* Redirection URL */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_WISPR,
		      RADIUS_ATTR_WISPR_REDIRECTION_URL, 0)) {
    conn->redirurllen = attr->l-2;
    memcpy(conn->redirurlbuf, attr->v.t, attr->l-2);
    conn->redirurlbuf[attr->l-2] = 0;
    conn->redirurl = conn->redirurlbuf;
  }
  else {
    conn->redirurllen = 0;
    conn->redirurlbuf[0] = 0;
    conn->redirurl = NULL;
  }

  /* Bandwidth up */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_WISPR,
		      RADIUS_ATTR_WISPR_BANDWIDTH_MAX_UP, 0)) {
    conn->bandwidthmaxup = ntohl(attr->v.i);
  }
  else {
    conn->bandwidthmaxup = 0;
  }

  /* Bandwidth down */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_WISPR,
		      RADIUS_ATTR_WISPR_BANDWIDTH_MAX_DOWN, 0)) {
    conn->bandwidthmaxdown = ntohl(attr->v.i);
  }
  else {
    conn->bandwidthmaxdown = 0;
  }

#ifdef RADIUS_ATTR_CHILLISPOT_BANDWIDTH_MAX_UP
  /* Bandwidth up */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_CHILLISPOT,
		      RADIUS_ATTR_CHILLISPOT_BANDWIDTH_MAX_UP, 0)) {
    conn->bandwidthmaxup = ntohl(attr->v.i) * 1000;
  }
#endif

#ifdef RADIUS_ATTR_CHILLISPOT_BANDWIDTH_MAX_DOWN
  /* Bandwidth down */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_CHILLISPOT,
		      RADIUS_ATTR_CHILLISPOT_BANDWIDTH_MAX_DOWN, 0)) {
    conn->bandwidthmaxdown = ntohl(attr->v.i) * 1000;
  }
#endif

  /* Max input octets */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_CHILLISPOT,
		      RADIUS_ATTR_CHILLISPOT_MAX_INPUT_OCTETS, 0)) {
    conn->maxinputoctets = ntohl(attr->v.i);
  }
  else {
    conn->maxinputoctets = 0;
  }

  /* Max output octets */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_CHILLISPOT,
		      RADIUS_ATTR_CHILLISPOT_MAX_OUTPUT_OCTETS, 0)) {
    conn->maxoutputoctets = ntohl(attr->v.i);
  }
  else {
    conn->maxoutputoctets = 0;
  }

  /* Max total octets */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_CHILLISPOT,
		      RADIUS_ATTR_CHILLISPOT_MAX_TOTAL_OCTETS, 0)) {
    conn->maxtotaloctets = ntohl(attr->v.i);
  }
  else {
    conn->maxtotaloctets = 0;
  }

  /* Session-Terminate-Time */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_WISPR,
		      RADIUS_ATTR_WISPR_SESSION_TERMINATE_TIME, 0)) {
    struct timeval timenow;
    gettimeofday(&timenow, NULL);
    memcpy(attrs, attr->v.t, attr->l-2);
    attrs[attr->l-2] = 0;
    memset(&stt, 0, sizeof(stt));
    result = sscanf(attrs, "%d-%d-%dT%d:%d:%d %d:%d",
		    &stt.tm_year, &stt.tm_mon, &stt.tm_mday,
		    &stt.tm_hour, &stt.tm_min, &stt.tm_sec,
		    &tzhour, &tzmin);
    if (result == 8) { /* Timezone */
      /* tzhour and tzmin is hours and minutes east of GMT */
      /* timezone is defined as seconds west of GMT. Excludes DST */
      stt.tm_year -= 1900;
      stt.tm_mon  -= 1;
      stt.tm_hour -= tzhour; /* Adjust for timezone */
      stt.tm_min  -= tzmin;  /* Adjust for timezone */
      /*      stt.tm_hour += daylight;*/
      /*stt.tm_min  -= (timezone / 60);*/
      tz = getenv("TZ");
      setenv("TZ", "", 1); /* Set environment to UTC */
      tzset();
      conn->sessionterminatetime = mktime(&stt);
      if (tz) 
			setenv("TZ", tz, 1); 
      else
			unsetenv("TZ");
      tzset();
    }
    else if (result >= 6) { /* Local time */
      tzset();
      stt.tm_year -= 1900;
      stt.tm_mon  -= 1;
      stt.tm_isdst = -1; /*daylight;*/
      conn->sessionterminatetime = mktime(&stt);
    } 
    else {
      conn->sessionterminatetime = 0;
      sys_err(LOG_WARNING, __FILE__, __LINE__, 0,
	      "Illegal WISPr-Session-Terminate-Time received: %s", attrs);
    }
    if ((conn->sessionterminatetime) && 
	(timenow.tv_sec > conn->sessionterminatetime)) {
      conn->response = REDIR_FAILED_OTHER;
      sys_err(LOG_WARNING, __FILE__, __LINE__, 0,
	      "WISPr-Session-Terminate-Time in the past received: %s", attrs);
      return 0;
    }
  }
  else {
    conn->sessionterminatetime = 0;
  }
  
  conn->response = REDIR_SUCCESS;
  return 0;
  
}

/* Send radius Access-Request and wait for answer */
static int redir_radius(struct redir_t *redir, struct in_addr *addr,
		 struct redir_conn_t *conn) {
  struct radius_t *radius;      /* Radius client instance */
  struct radius_packet_t radius_pack;
  int maxfd = 0;	        /* For select() */
  fd_set fds;			/* For select() */
  struct timeval idleTime;	/* How long to select() */
  int status;
  unsigned char chap_password[REDIR_MD5LEN + 1];
  unsigned char chap_challenge[REDIR_MD5LEN];
  unsigned char user_password[REDIR_MD5LEN+1];

  MD5_CTX context;

  char mac[REDIR_MACSTRLEN+1];
  char url[REDIR_URL_LEN];
  int n;

  if (radius_new(&radius,
		 &redir->radiuslisten, 0, 0,
		 NULL, 0, NULL, NULL, NULL)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Failed to create radius");
    return -1;
  }

  if (radius->fd > maxfd)
    maxfd = radius->fd;

  radius_set(radius, 0,
	     &redir->radiusserver0, &redir->radiusserver1,
	     redir->radiusauthport, redir->radiusacctport,
	     redir->radiussecret);
  
  radius_set_cb_auth_conf(radius, redir_cb_radius_auth_conf);

  radius_default_pack(radius, &radius_pack, RADIUS_CODE_ACCESS_REQUEST);
  
  radius_addattr(radius, &radius_pack, RADIUS_ATTR_USER_NAME, 0, 0, 0,
		 (uint8_t*) conn->username, strlen(conn->username));

  if (redir->secret) {
    /* Get MD5 hash on challenge and uamsecret */
    MD5Init(&context);
    MD5Update(&context, conn->uamchal, REDIR_MD5LEN);
    MD5Update(&context, (uint8_t*) redir->secret, strlen(redir->secret));
    MD5Final(chap_challenge, &context);
  }
  else {
    memcpy(chap_challenge, conn->uamchal, REDIR_MD5LEN);
  }
  
  if (conn->chap == 0) {
    for (n=0; n<REDIR_MD5LEN; n++) 
      user_password[n] = conn->password[n] ^ chap_challenge[n];
    user_password[REDIR_MD5LEN] = 0;
    radius_addattr(radius, &radius_pack, RADIUS_ATTR_USER_PASSWORD, 0, 0, 0,
		   user_password, strlen((char*)user_password));
  }
  else if (conn->chap == 1) {
    chap_password[0] = 0; /* Chap ident */
    memcpy(chap_password +1, conn->chappassword, REDIR_MD5LEN);
    radius_addattr(radius, &radius_pack, RADIUS_ATTR_CHAP_CHALLENGE, 0, 0, 0,
		   chap_challenge, REDIR_MD5LEN);
    radius_addattr(radius, &radius_pack, RADIUS_ATTR_CHAP_PASSWORD, 0, 0, 0,
		   chap_password, REDIR_MD5LEN+1);
  }

  radius_addattr(radius, &radius_pack, RADIUS_ATTR_NAS_IP_ADDRESS, 0, 0,
		 ntohl(redir->radiusnasip.s_addr), NULL, 0); /* WISPr_V1.0 */
  radius_addattr(radius, &radius_pack, RADIUS_ATTR_SERVICE_TYPE, 0, 0,
		 RADIUS_SERVICE_TYPE_LOGIN, NULL, 0); /* WISPr_V1.0 */
  radius_addattr(radius, &radius_pack, RADIUS_ATTR_FRAMED_IP_ADDRESS, 0, 0,
		 ntohl(conn->hisip.s_addr), NULL, 0); /* WISPr_V1.0 */

  /* Include his MAC address */
  snprintf(mac, REDIR_MACSTRLEN+1, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
	   conn->hismac[0], conn->hismac[1],
	   conn->hismac[2], conn->hismac[3],
	   conn->hismac[4], conn->hismac[5]);
  
  radius_addattr(radius, &radius_pack, RADIUS_ATTR_CALLING_STATION_ID, 0, 0, 0,
		 (uint8_t*) mac, REDIR_MACSTRLEN);
  
  if (redir->radiuscalled)
    radius_addattr(radius, &radius_pack, RADIUS_ATTR_CALLED_STATION_ID, 0, 0, 0,
		   (uint8_t*) redir->radiuscalled, strlen(redir->radiuscalled)); /* WISPr_V1.0 */

  if (redir->radiusnasid)
    radius_addattr(radius, &radius_pack, RADIUS_ATTR_NAS_IDENTIFIER, 0, 0, 0,
		   (uint8_t*) redir->radiusnasid, 
		   strlen(redir->radiusnasid)); /* WISPr_V1.0 */


  radius_addattr(radius, &radius_pack, RADIUS_ATTR_ACCT_SESSION_ID, 0, 0, 0,
		 (uint8_t*) conn->sessionid, REDIR_SESSIONID_LEN-1);


  radius_addattr(radius, &radius_pack, RADIUS_ATTR_NAS_PORT_TYPE, 0, 0,
		 redir->radiusnasporttype, NULL, 0);

  radius_addattr(radius, &radius_pack, RADIUS_ATTR_NAS_PORT, 0, 0,
		 conn->nasport, NULL, 0);
  
  radius_addattr(radius, &radius_pack, RADIUS_ATTR_MESSAGE_AUTHENTICATOR, 
		 0, 0, 0, NULL, RADIUS_MD5LEN);

  if (redir->radiuslocationid)
    radius_addattr(radius, &radius_pack, RADIUS_ATTR_VENDOR_SPECIFIC,
		   RADIUS_VENDOR_WISPR, RADIUS_ATTR_WISPR_LOCATION_ID, 0,
		   (uint8_t*) redir->radiuslocationid,
		   strlen(redir->radiuslocationid));

  if (redir->radiuslocationname)
    radius_addattr(radius, &radius_pack, RADIUS_ATTR_VENDOR_SPECIFIC,
		   RADIUS_VENDOR_WISPR, RADIUS_ATTR_WISPR_LOCATION_NAME, 0,
		   (uint8_t*) redir->radiuslocationname, 
		   strlen(redir->radiuslocationname));

  snprintf(url, REDIR_URL_LEN, "http://%s:%d/logoff",
	   inet_ntoa(redir->addr), redir->port);
  url[REDIR_URL_LEN] = 0;
  
  radius_addattr(radius, &radius_pack, RADIUS_ATTR_VENDOR_SPECIFIC,
		 RADIUS_VENDOR_WISPR, RADIUS_ATTR_WISPR_LOGOFF_URL, 0,
		 (uint8_t*) url, strlen(url));

  radius_req(radius, &radius_pack, conn);

  while ((redir->starttime + REDIR_RADIUS_MAX_TIME) > time(NULL)) {

    FD_ZERO(&fds);
    if (radius->fd != -1) FD_SET(radius->fd, &fds);
    if (radius->proxyfd != -1) FD_SET(radius->proxyfd, &fds);
    
    idleTime.tv_sec = 0;
    idleTime.tv_usec = REDIR_RADIUS_SELECT_TIME;
    radius_timeleft(radius, &idleTime);

    switch (status = select(maxfd + 1, &fds, NULL, NULL, &idleTime)) {
    case -1:
      sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	      "select() returned -1!");
      break;  
    case 0:
      if (optionsdebug) printf("Select returned 0\n");
      radius_timeout(radius);
      break; 
    default:
      break;
    }

    if (status > 0) {
      if ((radius->fd != -1) && FD_ISSET(radius->fd, &fds) && 
	  radius_decaps(radius) < 0) {
	sys_err(LOG_ERR, __FILE__, __LINE__, 0,
		"radius_ind() failed!");
      }
      
      if ((radius->proxyfd != -1) && FD_ISSET(radius->proxyfd, &fds) && 
	  radius_proxy_ind(radius) < 0) {
	sys_err(LOG_ERR, __FILE__, __LINE__, 0,
		"radius_proxy_ind() failed!");
      }
      
    }
  
    if (conn->response) {
      radius_free(radius);
      return 0;
    }
    
  }
  return 0;

}


/* redir_accept() does the following:
 1) forks a child process
 2) Accepts the tcp connection 
 3) Analyses a HTTP get request
 4) GET request can be one of the following:
    a) Logon request with username and challenge response
       - Does a radius request
       - If OK send result to parent and redirect to welcome page
       - Else redirect to error login page
    b) Logoff request
       - Send logoff request to parent
       - Redirect to login page?
    c) Request for another server
       - Redirect to login server.

 Incoming requests are identified only by their IP address. No MAC
 address information is obtained. The main security problem is denial
 of service attacks by malicious hosts sending logoff requests for
 clients. This can be prevented by checking incoming packets for
 matching MAC and src IP addresses.
*/

int redir_accept(struct redir_t *redir) {

  int new_socket;
  struct sockaddr_in address;
  int addrlen = sizeof(address);

  int bufsize = REDIR_MAXBUFFER;
  char buffer[bufsize];
  int buflen;
  int status;
  char hexchal[1+(2*REDIR_MD5LEN)];
  unsigned char challenge[REDIR_MD5LEN];
  struct redir_msg_t msg;
  int state = 0;

  struct redir_conn_t conn;

  struct sigaction act, oldact;
  struct itimerval itval;

  /* Close of socket */
  void redir_close(){
    if (!keep_going) shutdown(new_socket, SHUT_RDWR);
    close(new_socket);
    return;
  }
  
  void redir_memcopy(int msg_type){
    redir_challenge(challenge);
    (void)redir_chartohex(challenge, hexchal);
    msg.type = msg_type;
    msg.addr = address.sin_addr;
    memcpy(msg.uamchal, challenge, REDIR_MD5LEN);
    return;
  }
  
  memset(&conn, 0, sizeof(conn));
  memset(&msg, 0, sizeof(msg));

  if ((new_socket = accept(redir->fd, (struct sockaddr *)&address, 
			   (socklen_t*) &addrlen)) 
      < 0) {
    if (errno != ECONNABORTED)
      sys_err(LOG_ERR, __FILE__, __LINE__, errno, "accept() failed!");
    return 0;
  }


  /* This forks a new process. The child really should close all
     unused file descriptors and free memory allocated. This however
     is performed when the process exits, so currently we don't
     care */
  

  if ((status = fork()) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "fork() returned -1!");
    return 0;
  }


  if (status > 0) { /* Parent */
    close(new_socket);
    return 0; 
  }

  memset(&act, 0, sizeof(act));
  act.sa_handler = redir_termination;
  sigaction(SIGTERM, &act, &oldact);
  sigaction(SIGINT, &act, &oldact);
  act.sa_handler = redir_alarm;
  sigaction(SIGALRM, &act, &oldact);

  memset(&itval, 0, sizeof(itval));
  itval.it_interval.tv_sec = REDIR_MAXTIME; 
  itval.it_interval.tv_usec = 0; 
  itval.it_value.tv_sec = REDIR_MAXTIME;
  itval.it_value.tv_usec = 0; 
  if (setitimer(ITIMER_REAL, &itval, NULL)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "setitimer() failed!");
  }


  redir->starttime = time(NULL);

  if (fcntl(new_socket, F_SETFL, O_NONBLOCK)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno, "fcntl() failed");
    exit(0);
  }

  termstate = REDIR_TERM_GETREQ;
  if (optionsdebug) printf("Calling redir_getreq()\n");

  if (redir_getreq(redir, new_socket, &conn)) {
    if (optionsdebug) printf("Error calling get_req. Terminating\n");
    exit(0);
  }

  termstate = REDIR_TERM_GETSTATE;
  if (optionsdebug) printf("Calling cb_getstate()\n");

  if (!redir->cb_getstate) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "No cb_getstate() defined!");
    exit(0);
  }

  state = redir->cb_getstate(redir, &address.sin_addr, &conn);

  termstate = REDIR_TERM_PROCESS;
  if (optionsdebug) printf("Processing received request\n");

  if (conn.type == REDIR_LOGIN) { 
    
    /* Was client was already logged on? */
    if (state == 1) {
      if (optionsdebug) printf("redir_accept: Already logged on\n");
      redir_reply(redir, new_socket, &conn, REDIR_ALREADY, 0, 
		  NULL, NULL, conn.userurl, NULL,
		  NULL, conn.hismac);
      redir_close();
      exit(0);
    }

    /* Did the challenge expire? */
    if ((conn.uamtime + REDIR_CHALLENGETIMEOUT2) < time(NULL)) {
      if (optionsdebug) printf("redir_accept: Challenge expired: %d : %d\n",
			       conn.uamtime, time(NULL));
      redir_memcopy(REDIR_CHALLENGE);      
      if (msgsnd(redir->msgid, (struct msgbuf*) &msg, 
		 sizeof(struct redir_msg_t), 0) < 0) {
	sys_err(LOG_ERR, __FILE__, __LINE__, errno, "msgsnd() failed!");
	exit(0);
      }

      redir_reply(redir, new_socket, &conn, REDIR_FAILED_OTHER, 0,
		  hexchal, NULL, conn.userurl, NULL,
		  NULL, conn.hismac);
      redir_close();
      exit(0);
    }

    termstate = REDIR_TERM_RADIUS;
    if (optionsdebug) printf("Calling radius\n");

    if (optionsdebug) printf("redir_accept: Sending redius request\n");
    redir_radius(redir, &address.sin_addr, &conn);

    termstate = REDIR_TERM_REPLY;
    if (optionsdebug) printf("Received radius reply\n");

    if (conn.response == REDIR_SUCCESS) { /* Radius-Accept */
      redir_reply(redir, new_socket, &conn, conn.response, conn.sessiontimeout,
                  NULL, conn.username, conn.userurl, conn.reply,
                  conn.redirurl, conn.hismac);
      
      msg.type = REDIR_LOGIN;
      strncpy(msg.username, conn.username, sizeof(msg.username));
      msg.username[sizeof(msg.username)-1] = 0;
      msg.statelen = conn.statelen;
      memcpy(msg.statebuf, conn.statebuf, conn.statelen);
      msg.classlen = conn.classlen;
      memcpy(msg.classbuf, conn.classbuf, conn.classlen);
      msg.sessiontimeout = conn.sessiontimeout;
      msg.idletimeout = conn.idletimeout;
      msg.interim_interval = conn.interim_interval;
      msg.addr = address.sin_addr;
      msg.bandwidthmaxup = conn.bandwidthmaxup;
      msg.bandwidthmaxdown = conn.bandwidthmaxdown;
      msg.maxinputoctets = conn.maxinputoctets;
      msg.maxoutputoctets = conn.maxoutputoctets;
      msg.maxtotaloctets = conn.maxtotaloctets;
      msg.sessionterminatetime = conn.sessionterminatetime;
      msg.filteridlen = conn.filteridlen;
      strncpy(msg.filteridbuf, conn.filteridbuf, sizeof(msg.filteridbuf));
      
      if (msgsnd(redir->msgid, (struct msgbuf*) &msg,
		 sizeof(struct redir_msg_t), 0) < 0) {
	sys_err(LOG_ERR, __FILE__, __LINE__, errno, "msgsnd() failed!");
	exit(0);
      }
    }
    else {
      redir_memcopy(REDIR_CHALLENGE);      
      if (msgsnd(redir->msgid, (struct msgbuf*) &msg, 
		 sizeof(struct redir_msg_t), 0) < 0) {
	sys_err(LOG_ERR, __FILE__, __LINE__, errno, "msgsnd() failed!");
	exit(0);
      }

      redir_reply(redir, new_socket, &conn, conn.response, 0,
		  hexchal, NULL, conn.userurl, conn.reply, 
		  NULL, conn.hismac);
      redir_close();
      exit(0);
    }    

    redir_close();
    exit(0); /* Terminate the client */
  }
  else if (conn.type == REDIR_LOGOUT) {
    redir_memcopy(REDIR_LOGOUT); 
    if (msgsnd(redir->msgid, (struct msgbuf*) &msg, 
	       sizeof(struct redir_msg_t), 0) < 0) {
      sys_err(LOG_ERR, __FILE__, __LINE__, errno, "msgsnd() failed!");
      exit(0);
    }

    redir_reply(redir, new_socket, &conn, REDIR_LOGOFF, 0, 
		hexchal, NULL, conn.userurl, NULL, 
		NULL, conn.hismac);
    redir_close();    
    exit(0); /* Terminate the client */
  }
  else if (conn.type == REDIR_PRELOGIN) {
    redir_memcopy(REDIR_CHALLENGE);
    if (msgsnd(redir->msgid, (struct msgbuf*) &msg, 
	       sizeof(struct redir_msg_t), 0) < 0) {
      sys_err(LOG_ERR, __FILE__, __LINE__, errno, "msgsnd() failed!");
      exit(0);
    }

    if (state == 1) {
      redir_reply(redir, new_socket, &conn, REDIR_ALREADY, 0, 
		  NULL, NULL, conn.userurl, NULL,
		  NULL, conn.hismac);
    }
    else {
      redir_reply(redir, new_socket, &conn, REDIR_NOTYET, 0, 
		  hexchal, NULL, conn.userurl, NULL,
		  NULL, conn.hismac);
    }
    redir_close();
    exit(0);
  }
  else if (conn.type == REDIR_ABORT) {
    if (state == 1) {
      redir_reply(redir, new_socket, &conn, REDIR_ABORT_NAK, 0,
		  NULL, NULL, conn.userurl, NULL, 
		  NULL, conn.hismac);
    }
    else {
      redir_memcopy(REDIR_ABORT);
      if (msgsnd(redir->msgid, (struct msgbuf*) &msg, 
		 sizeof(struct redir_msg_t), 0) < 0) {
	sys_err(LOG_ERR, __FILE__, __LINE__, errno, "msgsnd() failed!");
	exit(0);
      }
      redir_reply(redir, new_socket, &conn, REDIR_ABORT_ACK, 0, 
		  hexchal, NULL, conn.userurl, NULL,
		  NULL, conn.hismac);
    }
    redir_close();
    exit(0);
  }
  else if (conn.type == REDIR_ABOUT) {
    redir_reply(redir, new_socket, &conn, REDIR_ABOUT, 0, 
		NULL, NULL, NULL, NULL,
		NULL, NULL);
    redir_close();
    exit(0);
  }
  else if (conn.type == REDIR_STATUS) {
    struct timeval timenow;
    uint32_t sessiontime;
    uint32_t timeleft;
    gettimeofday(&timenow, NULL);
    sessiontime = timenow.tv_sec - conn.start_time.tv_sec;
    sessiontime += (timenow.tv_usec - conn.start_time.tv_usec) / 1000000;
    if (conn.sessiontimeout)
      timeleft = conn.sessiontimeout - sessiontime;
    else
      timeleft = 0;
    redir_reply(redir, new_socket, &conn, REDIR_STATUS, timeleft,
		NULL, NULL, NULL, NULL,
		NULL, NULL);
    redir_close();
    exit(0);
  }
  else if (conn.type == REDIR_MSDOWNLOAD) {
    buflen = snprintf(buffer, bufsize,
		      "HTTP/1.0 403 Forbidden\r\n\r\n");
    

    send(new_socket, buffer, buflen, 0);
    
    redir_close();
    exit(0);
  }

  /* It was not a request for a known path. It must be an original request */

  if (optionsdebug) printf("redir_accept: Original request\n");

  /* Did the challenge expire? */
  if ((conn.uamtime + REDIR_CHALLENGETIMEOUT1) < time(NULL)) {
    redir_memcopy(REDIR_CHALLENGE);
    strncpy(msg.userurl, conn.userurl, sizeof(msg.userurl));
    msg.userurl[sizeof(msg.userurl)-1] = 0;
    if (msgsnd(redir->msgid, (struct msgbuf*) &msg, 
	       sizeof(struct redir_msg_t), 0) < 0) {
      sys_err(LOG_ERR, __FILE__, __LINE__, errno, "msgsnd() failed!");
      exit(0);
    }
  }
  else {
    (void)redir_chartohex(conn.uamchal, hexchal);
  }
  
  if (redir->homepage) {
    buflen = snprintf(buffer, bufsize,
		      "HTTP/1.0 302 Moved Temporarily\r\n"
		      "Location: "
		      "%s\r\n"
		      "Content-type: text/html"
		      "\r\n\r\n"
		      "<HTML>"
		      "<HEAD><TITLE>302 Moved Temporarily</TITLE></HEAD>"
		      "<BODY><H1>Browser error!</H1>"
		      "Browser does not support redirects!</BODY></HTML>",
		      redir->homepage);
    buffer[bufsize-1] = 0;
    if (buflen>bufsize) buflen = bufsize;

    if (optionsdebug) printf("redir_reply: Sending http reply: %s\n",
			     buffer);

    send(new_socket, buffer, buflen, 0);
  }
  else if (state == 1) {
    redir_reply(redir, new_socket, &conn, REDIR_ALREADY, 0,
		NULL, NULL, conn.userurl, NULL,
		NULL, conn.hismac);
  }
  else {
    redir_reply(redir, new_socket, &conn, REDIR_NOTYET, 0, 
		hexchal, NULL, conn.userurl, NULL, 
		NULL, conn.hismac);
  }

  redir_close();
  exit(0);

  /*  close(redir->fd);*/
}


/* Set callback to determine state information for the connection */
int redir_set_cb_getstate(struct redir_t *redir,
  int (*cb_getstate) (struct redir_t *redir, struct in_addr *addr,
		      struct redir_conn_t *conn)) {

  redir->cb_getstate = cb_getstate;
  return 0;
}
