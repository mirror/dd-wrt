/*
 *  Copyright (C) 2002-06 Luca Deri <deri@ntop.org>
 *
 *  			  http://www.ntop.org/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "nprobe.h"

#ifdef sun
extern char *strtok_r(char *, const char *, char **);
#endif

#ifdef WIN32
#define strtok_r(a, b, c) strtok(a, b)
#endif

/* ********************** */

static u_int32_t localNetworks[MAX_NUM_NETWORKS][3]; /* [0]=network, [1]=mask, [2]=broadcast */
static u_int32_t numLocalNetworks;

/* ********************** */

extern struct timeval initialSniffTime;
extern pthread_mutex_t traceEventMutex;
extern u_char useDynamicInterfaceIdx;

/* ******************************************* */

void traceEvent(int eventTraceLevel, char* file, int line, char * format, ...) {
#ifdef WIN32
  int beginFileIdx;
#endif
  va_list va_ap;

  va_start (va_ap, format);

  /* Fix courtesy of "Burton M. Strauss III" <BStrauss@acm.org> */
  if(eventTraceLevel <= traceLevel) {
    char buf[BUF_SIZE];
    char theDate[32];
    time_t theTime = time(NULL);

    /* We have two paths - one if we're logging, one if we aren't
     *   Note that the no-log case is those systems which don't support it (WIN32),
     *                                those without the headers !defined(USE_SYSLOG)
     *                                those where it's parametrically off...
     */

    memset(buf, 0, BUF_SIZE);
    strftime(theDate, 32, "%d/%b/%Y %H:%M:%S", localtime(&theTime));

#if defined(WIN32) || !defined(USE_SYSLOG)

#ifndef DEBUG
#ifdef WIN32
    for(beginFileIdx=strlen(file)-1; beginFileIdx>1; beginFileIdx--)
      if(file[beginFileIdx-1] == '\\') break;
    printf("%s [%s:%4d] ", theDate, &file[beginFileIdx], line);
#else
    printf("%s [%s:%d] ", theDate, file, line);
#endif

#else
    printf("%s ", theDate);
#endif

#if defined(WIN32)
    /* Windows lacks vsnprintf */
    vsprintf(buf, format, va_ap);
#else /* WIN32 - vsnprintf */
    vsnprintf(buf, BUF_SIZE-1, format, va_ap);
#endif /* WIN32 - vsnprintf */

    printf("%s%s", buf, (format[strlen(format)-1] != '\n') ? "\n" : "");
    fflush(stdout);
#else /* !WIN32 */

    vsnprintf(buf, BUF_SIZE-1, format, va_ap);

#if defined(USE_SYSLOG)
    if(useSyslog)
      syslog(eventTraceLevel, "[%s:%4d] %s", file, line, buf);
#endif

    if(!useSyslog) {
      if(traceMode) {
	printf("%s [%s:%4d] ", theDate, file, line);
      } else
	printf("%s ", theDate);

      if(eventTraceLevel == 0 /* TRACE_ERROR */) printf("ERROR: ");
      else if(eventTraceLevel == 1 /* TRACE_WARNING */) printf("WARNING: ");

      printf("%s%s", buf, (format[strlen(format)-1] != '\n') ? "\n" : "");
      fflush(stdout);
    }
#endif /* WIN32 || !USE_SYSLOG */
  }

  va_end (va_ap);
}

/* ************************************ */

#ifdef WIN32
unsigned long waitForNextEvent(unsigned long ulDelay /* ms */) {
  unsigned long ulSlice = 1000L; /* 1 Second */

  while(ulDelay > 0L) {
    if(ulDelay < ulSlice)
      ulSlice = ulDelay;
    Sleep(ulSlice);
    ulDelay -= ulSlice;
  }

  return ulDelay;
}

/* ******************************* */

void initWinsock32() {
  WORD wVersionRequested;
  WSADATA wsaData;
  int err;

  wVersionRequested = MAKEWORD(2, 0);
  err = WSAStartup( wVersionRequested, &wsaData );
  if( err != 0 ) {
    /* Tell the user that we could not find a usable */
    /* WinSock DLL.                                  */
    traceEvent(TRACE_ERROR, "FATAL ERROR: unable to initialise Winsock 2.x.");
    exit(-1);
  }
}

/* ******************************** */

short isWinNT() {
  DWORD dwVersion;
  DWORD dwWindowsMajorVersion;

  dwVersion=GetVersion();
  dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
  if(!(dwVersion >= 0x80000000 && dwWindowsMajorVersion >= 4))
    return 1;
  else
    return 0;
}

/* ****************************************************** */
/*
int snprintf(char *string, size_t maxlen, const char *format, ...) {
  int ret=0;
  va_list args;

  va_start(args, format);
  vsprintf(string,format,args);
  va_end(args);
  return ret;
}
*/
#endif /* Win32 */

/* ****************************************************** */

void checkHostFingerprint(char *fingerprint, char *osName, int osNameLen) {
  FILE *fd = NULL;
  char *WIN, *MSS, *WSS, *ttl, *flags;
  int S, N, D, T, done = 0;
  char *strtokState;

  osName[0] = '\0';
  strtokState = NULL;
  WIN = strtok_r(fingerprint, ":", &strtokState);
  MSS = strtok_r(NULL, ":", &strtokState);
  ttl = strtok_r(NULL, ":", &strtokState);
  WSS = strtok_r(NULL, ":", &strtokState);
  S = atoi(strtok_r(NULL, ":", &strtokState));
  N = atoi(strtok_r(NULL, ":", &strtokState));
  D = atoi(strtok_r(NULL, ":", &strtokState));
  T = atoi(strtok_r(NULL, ":", &strtokState));
  flags = strtok_r(NULL, ":", &strtokState);

  fd = fopen("etter.passive.os.fp", "r");

  if(fd) {
    char line[384];
    char *b, *d, *ptr;

    while((!done) && fgets(line, sizeof(line)-1, fd)) {
      if((line[0] == '\0') || (line[0] == '#') || (strlen(line) < 30)) continue;
      line[strlen(line)-1] = '\0';

      strtokState = NULL;
      ptr = strtok_r(line, ":", &strtokState); if(ptr == NULL) continue;
      if(strcmp(ptr, WIN)) continue;
      b = strtok_r(NULL, ":", &strtokState); if(b == NULL) continue;
      if(strcmp(MSS, "_MSS") != 0) {
	if(strcmp(b, "_MSS") != 0) {
	  if(strcmp(b, MSS)) continue;
	}
      }

      ptr = strtok_r(NULL, ":", &strtokState); if(ptr == NULL) continue;
      if(strcmp(ptr, ttl)) continue;

      d = strtok_r(NULL, ":", &strtokState); if(d == NULL) continue;
      if(strcmp(WSS, "WS") != 0) {
	if(strcmp(d, "WS") != 0) {
	  if(strcmp(d, WSS)) continue;
	}
      }

      ptr = strtok_r(NULL, ":", &strtokState); if(ptr == NULL) continue;
      if(atoi(ptr) != S) continue;
      ptr = strtok_r(NULL, ":", &strtokState); if(ptr == NULL) continue;
      if(atoi(ptr) != N) continue;
      ptr = strtok_r(NULL, ":", &strtokState); if(ptr == NULL) continue;
      if(atoi(ptr) != D) continue;
      ptr = strtok_r(NULL, ":", &strtokState); if(ptr == NULL) continue;
      if(atoi(ptr) != T) continue;
      ptr = strtok_r(NULL, ":", &strtokState); if(ptr == NULL) continue;
      if(strcmp(ptr, flags)) continue;

      /* NOTE
	 strlen(srcHost->fingerprint) is 29 as the fingerprint length is so
	 Example: 0212:_MSS:80:WS:0:1:0:0:A:LT
      */

      snprintf(osName, osNameLen, "%s", &line[29]);
      done = 1;
    }

    fclose(fd);
  }
}

/* *************************************************** */

/* Courtesy of Andreas Pfaller <apfaller@yahoo.com.au> */

typedef struct IPNode {
  struct IPNode *b[2];
  u_short as;
} IPNode;

IPNode *asHead = NULL;
u_long asMem = 0, asCount=0;

/* *************************************************** */

/* Courtesy of Andreas Pfaller <apfaller@yahoo.com.au> */

static u_int32_t xaton(char *s) {
  u_int32_t a, b, c, d;

  if(4!=sscanf(s, "%d.%d.%d.%d", &a, &b, &c, &d))
    return 0;
  return((a&0xFF)<<24)|((b&0xFF)<<16)|((c&0xFF)<<8)|(d&0xFF);
}

/* ******************************************************************* */

static void addNodeInternal(u_int32_t ip, int prefix, u_short as) {
  IPNode *p1 = asHead;
  IPNode *p2 = NULL;
  int i, b;

  for(i=0; i<prefix; i++) {
    b=(ip>>(31-i)) & 0x1;
    if(!p1->b[b]) {
      if(!(p2=malloc(sizeof(IPNode)))) {
	traceEvent(TRACE_ERROR, "Not enough memory?");
	return;
      }
      memset(p2, 0, sizeof(IPNode));
      asMem += sizeof(IPNode);
      p1->b[b]=p2;
    }
    else
      p2=p1->b[b];

    p1=p2;
  }

  if(p2->as == 0)
    p2->as = as;
}

/* ******************************************************************* */

u_short ip2AS(IpAddress ip) {
  if(ignoreAS || ip.ipVersion != 4)
    return(0);
  else {
    IPNode *p=asHead;
    int i, b;
    u_short as=0;

    i=0;
    while(p != NULL) {
      if(p->as != 0) as = p->as;
      b = (ip.ipType.ipv4 >> (31-i)) & 0x1;
      p = p->b[b];
      i++;
    }

#ifdef DEBUG
    {
      char buf[64];
      struct in_addr addr;

      addr.s_addr = ip.ipType.ipv4;
      traceEvent(TRACE_INFO, "%s: %d AS",  _intoaV4(addr.s_addr, buf, sizeof(buf)), as);
    }
#endif

    return(as);
  }
}

/* ************************************ */

void readASs(char *path) {
  if(ignoreAS || (path == NULL))
    return;
  else {
    FILE *fd;
    u_char useGz;

    traceEvent(TRACE_INFO, "Attempting to read AS table from file %s", path);

#ifdef HAVE_ZLIB_H
    if(!strcmp(&path[strlen(path)-3], ".gz")) {
      useGz = 1;
      fd = gzopen(path, "r");
    } else
#endif
      {
	useGz = 0;
	fd = fopen(path, "r");
      }

    if(fd != NULL) {
      asHead = malloc(sizeof(IPNode));

      if(asHead == NULL) {
	traceEvent(TRACE_ERROR, "Not enough memory?");
	return;
      }

      memset(asHead, 0, sizeof(IPNode));
      asHead->as = 0;
      asMem += sizeof(IPNode);

      while(1) {
	char buff[256];
	char *strTokState, *as, *ip, *prefix;

#ifdef HAVE_ZLIB_H
	if(useGz) {
	  if(gzeof(fd)) break;
	} else
#endif
	  {
	    if(feof(fd)) break;
	  }

#ifdef HAVE_ZLIB_H
	if(useGz) {
	  if(gzgets(fd, buff, sizeof(buff)) == NULL) continue;
	} else
#endif
	  {
	    if(fgets(buff, sizeof(buff), fd) == NULL) continue;
	  }

	if((as = strtok_r(buff, ":", &strTokState)) == NULL)  continue;
	if((ip = strtok_r(NULL, "/", &strTokState)) == NULL)  continue;
	if((prefix = strtok_r(NULL, "\n", &strTokState)) == NULL)  continue;

	addNodeInternal(xaton(ip), atoi(prefix), atoi(as));
	asCount++;
      }

#ifdef HAVE_ZLIB_H
      if(useGz)
	gzclose(fd);
      else
#endif
	fclose(fd);

      traceEvent(TRACE_INFO, "Read %d ASs [Used %d KB of memory]", asCount, asMem/1024);
    } else
      traceEvent(TRACE_ERROR, "Unable to read file %s", path);

    if(asCount > 0)
      ignoreAS = 0;
    else
      ignoreAS = 1;
  }
}

/* ******************************************** */

void nprintf(FILE *stream, char *fmt, HashBucket *theFlow, int direction) {
  char buf[256], c;
  int i;

  for(; *fmt; fmt++) {	/* scan format string characters */
    switch(*fmt) {
    case '%':	        /* special format follows */
      switch(c = *++fmt) {
      case '%':		/* print a percent character */
	putc('%', stream);
	break;
      case 'd':
	switch(*++fmt) {
	case 'a':
	  /* Destination IP Address(IPv4/6) */
	  fprintf(stream, "%s", direction == 0 ? _intoa(theFlow->dst, buf, sizeof(buf)):_intoa(theFlow->src, buf, sizeof(buf)));
	  break;
	case 'o':
	  /* Destination Octets (sent) */
	  fprintf(stream, "%lu", direction == 0 ? theFlow->bytesSent : theFlow->bytesRcvd);
	  break;
	case 'p':
	  /* Destination Packets (sent) */
	  fprintf(stream, "%lu", direction == 0 ? theFlow->pktSent : theFlow->pktRcvd);
	  break;
	case 'P':
	  /* Destination Port */
	  fprintf(stream, "%d", direction == 0 ? theFlow->dport : theFlow->sport);
	  break;
	case 's':
	  /* Destination AS */
	  fprintf(stream, "%d", direction == 0 ? ip2AS(theFlow->src) : ip2AS(theFlow->dst));
	  break;
	}
	break;
      case 'f':
	switch(*++fmt) {
	case 'r':
	  /* SysUptime at start of flow */
	  fprintf(stream, "%.3f", direction == 0 ? toMs(theFlow->firstSeenSent) : toMs(theFlow->firstSeenRcvd));
	  break;
	case 'm':
	  /* Fragmented Packets */
	  fprintf(stream, "%d", direction == 0 ? fragmentedPacketSrc2Dst(theFlow) : fragmentedPacketDst2Src(theFlow));
	  break;
	case 'p':
	  /* Host Fingerprint */
	  for(i=0; i<FINGERPRINT_LEN; i++)
	    fprintf(stream, "%c",
		    (direction == 0 ? theFlow->src2dstFingerprint : theFlow->dst2srcFingerprint)[i]);
	  break;
	}
	break;
      case 'i':
	switch(*++fmt) {
	case 'f':
	  /* ICMP Flags */
	  fprintf(stream, "%d", direction == 0 ? theFlow->src2dstIcmpFlags : theFlow->dst2srcIcmpFlags);
	  break;
	case 'n':
	  /* Input interface index */
	  fprintf(stream, "%d", ifIdx(theFlow, direction, 1));
	  break;
	}
	break;

      case 'l':
	switch(*++fmt) {
	case 'a':
	  /* SysUptime at end of flow */
	  fprintf(stream, "%.3f", direction == 0 ? toMs(theFlow->lastSeenSent) : toMs(theFlow->lastSeenRcvd));
	  break;
	}
	break;
      case 'm':
	switch(*++fmt) {
	case 'd':
	  /* Destination Mac Address */
	  fprintf(stream, "%s", direction == 0 ? etheraddr_string(theFlow->dstMacAddress, buf)
		  : etheraddr_string(theFlow->srcMacAddress, buf));
	  break;
	case 'h':
	  /* MPLS header */
	  fprintf(stream, "%d", 0); /* dummy */
	  break;
	case 's':
	  /* Source Mac Address */
	  fprintf(stream, "%s", direction == 0 ? etheraddr_string(theFlow->srcMacAddress, buf)
		  : etheraddr_string(theFlow->dstMacAddress, buf));
	  break;
	}
	break;
      case 'n':
	switch(*++fmt) {
	case 'n':
	  /* TCP Nw Latency(usec) */
	  if(nwLatencyComputed(theFlow)) {
	    fprintf(stream, "%ld", (long int)theFlow->nwLatency.tv_usec*1000);
	  } else
	    fprintf(stream, "0");
	  break;
	case 's':
	  /* TCP Nw Latency(sec) */
	  if(nwLatencyComputed(theFlow)) {
	    fprintf(stream, "%lu", (long unsigned int)theFlow->nwLatency.tv_sec);
	  } else
	    fprintf(stream, "0");
	  break;
	}
	break;
      case 'o':
	switch(*++fmt) {
	case 'u':
	  /* Output interface index */
	  fprintf(stream, "%d", ifIdx(theFlow, direction, 0));
	  break;
	}
	break;
      case 'p':
	switch(*++fmt) {
	case 'a':
	  /* Payload */
	  if(direction == 0) {
	    int i; for(i=0; i<theFlow->src2dstPayloadLen; i++) fprintf(stream, "%c", theFlow->src2dstPayload[i]);
	  } else {
	    int i; for(i=0; i<theFlow->dst2srcPayloadLen; i++) fprintf(stream, "%c", theFlow->dst2srcPayload[i]);
	  }
	  break;
	case 'l':
	  /* Payload Len */
	  fprintf(stream, "%d", direction == 0 ? theFlow->src2dstPayloadLen : theFlow->dst2srcPayloadLen);
	  break;
	case 'r':
	  /* IP Protocol */
	  fprintf(stream, "%d", theFlow->proto);
	  break;
	case 'n':
	  /* Appl Latency(nsec) */
	  if(applLatencyComputed(theFlow)) {
	    fprintf(stream, "%lu", (long unsigned int)(direction == 0 ? theFlow->src2dstApplLatency.tv_usec*1000 :
						       theFlow->dst2srcApplLatency.tv_usec*1000));
	  } else
	    fprintf(stream, "0");
	  break;
	case 's':
	  /* Appl Latency(sec) */
	  if(applLatencyComputed(theFlow)) {
	    fprintf(stream, "%lu", (long unsigned int)(direction == 0 ? theFlow->src2dstApplLatency.tv_sec :
						       theFlow->dst2srcApplLatency.tv_sec));
	  } else
	    fprintf(stream, "0");
	  break;
	}
	break;
      case 'r':
	switch(*++fmt) {
	case 'o':
	  /* Destination Octets */
	  fprintf(stream, "%lu", (netFlowVersion == 5) ? 0 : (direction == 0 ? theFlow->bytesRcvd : theFlow->bytesSent));
	  break;
	case 'p':
	  /* Destination Packets */
	  fprintf(stream, "%lu", (netFlowVersion == 5) ? 0 : (direction == 0 ? theFlow->pktRcvd : theFlow->pktSent));
	  break;
	}
	break;
      case 's':
	switch(*++fmt) {
	case 'a':
	  /* Source IP Address(IPv4/6) */
	  fprintf(stream, "%s", direction == 0 ? _intoa(theFlow->src, buf, sizeof(buf)):
		  _intoa(theFlow->dst, buf, sizeof(buf)));
	  break;
	case 'P':
	  /* Source Port */
	  fprintf(stream, "%d", direction == 0 ? theFlow->sport : theFlow->dport);
	  break;
	case 's':
	  /* Source AS */
	  fprintf(stream, "%d", direction == 0 ? ip2AS(theFlow->dst) : ip2AS(theFlow->src));
	  break;
	}
	break;
      case 't':
	switch(*++fmt) {
	case 'f':
	  /* TCP Flags */
	  fprintf(stream, "%d", direction == 0 ? theFlow->src2dstTcpFlags : theFlow->dst2srcTcpFlags);
	  break;
	case 's':
	  /* Type of Service */
	  fprintf(stream, "%d", direction == 0 ? theFlow->src2dstTos : theFlow->dst2srcTos);
	  break;
	}
	break;
      case 'v':
	switch(*++fmt) {
	case 't':
	  /* VLAN tag */
	  fprintf(stream, "%d", 0); /* dummy */
	  break;
	}
	break;
      default:
	traceEvent(TRACE_ERROR, "Unknown pattern \"%%%c\" found on template (see -D option)", c);
	break;
      }
      break;
    default:
      putc(*fmt, stream);
    }
  }

  putc('\n', stream);
}

/* ********* NetFlow v9/IPFIX ***************************** */

/*
  Cisco Systems NetFlow Services Export Version 9

  http://www.faqs.org/rfcs/rfc3954.html
*/

V9TemplateId ver9_templates[] = {
  /* { 0,  0, "NOT_USED", "" }, */
  { 1,  4, "IN_BYTES", "Incoming flow bytes" },
  { 1,  0, "SYSTEM_ID", "" }, /* Hack for options template */
  { 2,  4, "IN_PKTS", "Incoming flow packets" },
  { 2,  0, "INTERFACE_ID", "" }, /* Hack for options template */
  { 3,  4, "FLOWS", "Number of flows" },
  { 3,  0, "LINE_CARD", "" }, /* Hack for options template */
  { 4,  1, "PROTOCOL", "IP protocol byte" },
  { 4,  0, "NETFLOW_CACHE", "" }, /* Hack for options template */
  { 5,  1, "SRC_TOS", "Type of service byte" },
  { 5,  0, "TEMPLATE_ID", "" }, /* Hack for options template */
  { 6,  1, "TCP_FLAGS", "Cumulative of all flow TCP flags" },
  { 7,  2, "L4_SRC_PORT", "IPv4 source port" },
  { 8,  4, "IPV4_SRC_ADDR", "IPv4 source address" },
  { 9,  1, "SRC_MASK", "Source subnet mask (/<bits>)" },
  { 10,  2, "INPUT_SNMP", "Input interface SNMP idx" },
  { 11,  2, "L4_DST_PORT", "IPv4 destination port" },
  { 12,  4, "IPV4_DST_ADDR", "IPv4 destination address" },
  { 13,  1, "DST_MASK", "Dest subnet mask (/<bits>)" },
  { 14,  2, "OUTPUT_SNMP", "Output interface SNMP idx" },
  { 15,  4, "IPV4_NEXT_HOP", "IPv4 next hop address" },
  { 16,  2, "SRC_AS", "Source BGP AS" },
  { 17,  2, "DST_AS", "Destination BGP AS" },
  /*
  { 18,  4, "BGP_IPV4_NEXT_HOP", "" },
  { 19,  4, "MUL_DST_PKTS", "" },
  { 20,  4, "MUL_DST_BYTES", "" },
  */
  { 21,  4, "LAST_SWITCHED", "SysUptime (msec) of the last flow pkt" },
  { 22,  4, "FIRST_SWITCHED", "SysUptime (msec) of the first flow pkt" },
  { 23,  4, "OUT_BYTES", "Outgoing flow bytes" },
  { 24,  4, "OUT_PKTS", "Outgoing flow packets" },
  /* { 25,  0, "RESERVED", "" }, */
  /* { 26,  0, "RESERVED", "" }, */
  { 27,  16, "IPV6_SRC_ADDR", "IPv4 source address" },
  { 28,  16, "IPV6_DST_ADDR", "IPv4 destination address" },
  { 29,  1, "IPV6_SRC_MASK", "IPv4 source mask" },
  { 30,  1, "IPV6_DST_MASK", "IPv4 destination mask" },
  /* { 31,  3, "IPV6_FLOW_LABEL", "" }, */
  { 32,  2, "ICMP_TYPE", "ICMP Type * 256 + ICMP code" },
  /* { 33,  1, "MUL_IGMP_TYPE", "" }, */
  { 34,  4, "SAMPLING_INTERVAL", "Sampling rate" },
  { 35,  1, "SAMPLING_ALGORITHM", "Sampling type (deterministic/random)" },
  { 36,  2, "FLOW_ACTIVE_TIMEOUT", "Activity timeout of flow cache entries" },
  { 37,  2, "FLOW_INACTIVE_TIMEOUT", "Inactivity timeout of flow cache entries" },
  { 38,  1, "ENGINE_TYPE", "Flow switching engine" },
  { 39,  1, "ENGINE_ID", "Id of the flow switching engine" },
  { 40,  4, "TOTAL_BYTES_EXP", "Total bytes exported" },
  { 41,  4, "TOTAL_PKTS_EXP", "Total flow packets exported" },
  { 42,  4, "TOTAL_FLOWS_EXP", "Total number of exported flows" },
  /* { 43,  0, "RESERVED", "" }, */
  /* { 44,  0, "RESERVED", "" }, */
  /* { 45,  0, "RESERVED", "" }, */
  /* { 46,  0, "RESERVED", "" }, i*/
  /* { 47,  0, "RESERVED", "" }, */
  /* { 48,  0, "RESERVED", "" }, */
  /* { 49,  0, "RESERVED", "" }, */
  /* { 50,  0, "RESERVED", "" }, */
  /* { 51,  0, "RESERVED", "" }, */
  /* { 52,  0, "RESERVED", "" }, */
  /* { 53,  0, "RESERVED", "" }, */
  /* { 54,  0, "RESERVED", "" }, */
  /* { 55,  0, "RESERVED", "" }, */
  { 56,  6, "IN_SRC_MAC", "Source MAC Address" }, /* new */
  { 57,  6, "OUT_DST_MAC", "Destination MAC Address" }, /* new */
  { 58,  2, "SRC_VLAN", "Source VLAN" },
  { 59,  2, "DST_VLAN", "Destination VLAN" },
  { 60,  1, "IP_PROTOCOL_VERSION", "[4=IPv4][6=IPv6]" },
  { 61,  1, "DIRECTION", "[0=ingress][1=egress] flow" },
  /*
  { 62,  1, "IPV6_NEXT_HOP", "IPv4 next hop address" },
  { 63,  16, "BPG_IPV6_NEXT_HOP", "" },
  { 64,  16, "IPV6_OPTION_HEADERS", "" },
  */
  /* { 65,  0, "RESERVED", "" }, */
  /* { 66,  0, "RESERVED", "" }, */
  /* { 67,  0, "RESERVED", "" }, */
  /* { 68,  0, "RESERVED", "" }, */
  /* { 69,  0, "RESERVED", "" }, */
  { 70,  3, "MPLS_LABEL_1",  "MPLS label at position 1" },
  { 71,  3, "MPLS_LABEL_2",  "MPLS label at position 2" },
  { 72,  3, "MPLS_LABEL_3",  "MPLS label at position 3" },
  { 73,  3, "MPLS_LABEL_4",  "MPLS label at position 4" },
  { 74,  3, "MPLS_LABEL_5",  "MPLS label at position 5" },
  { 75,  3, "MPLS_LABEL_6",  "MPLS label at position 6" },
  { 76,  3, "MPLS_LABEL_7",  "MPLS label at position 7" },
  { 77,  3, "MPLS_LABEL_8",  "MPLS label at position 8" },
  { 78,  3, "MPLS_LABEL_9",  "MPLS label at position 9" },
  { 79,  3, "MPLS_LABEL_10", "MPLS label at position 10" },

  { 80,  6, "IN_DST_MAC", "Source MAC Address" },
  { 81,  6, "OUT_SRC_MAC", "Destination MAC Address" },

  /*
    ntop Extensions

    IMPORTANT
    if you change/add constants here/below make sure
    you change them into ntop too.
  */

  { 90,  1, "FRAGMENTED", "1=some flow packets are fragmented" },
  { 91,  FINGERPRINT_LEN, "FINGERPRINT", "TCP fingerprint" },
  { 92,  4, "NW_LATENCY_SEC", "Network latency (sec)" },
  { 93,  4, "NW_LATENCY_USEC", "Network latency (usec)" },
  { 94,  4, "APPL_LATENCY_SEC", "Application latency (sec)" },
  { 95,  4, "APPL_LATENCY_USEC", "Application latency (sec)" },
  { IN_PAYLOAD_ID,  0 /* The length is set at runtime */, "IN_PAYLOAD", "Initial payload bytes" },
  { OUT_PAYLOAD_ID,  0 /* The length is set at runtime */, "OUT_PAYLOAD", "Initial payload bytes" },
  { 98,  4, "ICMP_FLAGS", "Cumulative of all flow ICMP types" },

  { 0,  1, "PAD1", "" },
  { 0,  2, "PAD2", "" },
  { 0,   0, NULL, NULL }
};

/* ******************************************** */

void printTemplateInfo(V9TemplateId *templates) {
  int j = 0;

  while(templates[j].templateName != NULL) {
    if((templates[j].templateLen > 0)
       || (templates[j].templateId == IN_PAYLOAD_ID)
       || (templates[j].templateId == OUT_PAYLOAD_ID)) {
      /* This is not a dummy template */
      printf("[%3d] %%%-22s\t%s\n",
	     templates[j].templateId,
	     templates[j].templateName,
	     templates[j].templateDescr);
    }

    j++;
  }
}

/* ******************************************** */

void setPayloadLength(int len) {
  int i = 0;

  while(ver9_templates[i].templateName != NULL) {
    if((ver9_templates[i].templateId == IN_PAYLOAD_ID)
       || (ver9_templates[i].templateId == OUT_PAYLOAD_ID)) {
      ver9_templates[i].templateLen = len;
      break;
    }
    i++;
  }
}

/* ******************************************** */

void copyInt8(u_int8_t t8, char *outBuffer,
	      u_int *outBufferBegin, u_int *outBufferMax) {
  if((*outBufferBegin)+sizeof(t8) < (*outBufferMax)) {
    memcpy(&outBuffer[(*outBufferBegin)], &t8, sizeof(t8));
    (*outBufferBegin) += sizeof(t8);
  }
}

/* ******************************************** */

void copyInt16(u_int16_t _t16, char *outBuffer,
	       u_int *outBufferBegin, u_int *outBufferMax) {
  u_int16_t t16 = htons(_t16);

  if((*outBufferBegin)+sizeof(t16) < (*outBufferMax)) {
    memcpy(&outBuffer[(*outBufferBegin)], &t16, sizeof(t16));
    (*outBufferBegin) += sizeof(t16);
  }
}

/* ******************************************** */

void copyInt32(u_int32_t _t32, char *outBuffer,
	       u_int *outBufferBegin, u_int *outBufferMax) {
  u_int32_t t32 = htonl(_t32);

  if((*outBufferBegin)+sizeof(t32) < (*outBufferMax)) {
#ifdef DEBUG
    char buf1[32];

    printf("(8) %s\n", _intoaV4(_t32, buf1, sizeof(buf1)));
#endif

    memcpy(&outBuffer[(*outBufferBegin)], &t32, sizeof(t32));
    (*outBufferBegin) += sizeof(t32);
  }
}

/* ******************************************** */

void copyLen(u_char *str, int strLen, char *outBuffer,
	     u_int *outBufferBegin, u_int *outBufferMax) {
  if((*outBufferBegin)+strLen < (*outBufferMax)) {
    memcpy(&outBuffer[(*outBufferBegin)], str, strLen);
    (*outBufferBegin) += strLen;
  }
}

/* ******************************************** */

static void copyIpV6(struct in6_addr ipv6, char *outBuffer,
		     u_int *outBufferBegin, u_int *outBufferMax) {
  copyLen((u_char*)&ipv6, sizeof(ipv6), outBuffer,
	  outBufferBegin, outBufferMax);
}

/* ******************************************** */

static void copyMac(u_char *macAddress, char *outBuffer,
		    u_int *outBufferBegin, u_int *outBufferMax) {
  copyLen(macAddress, 6 /* lenght of mac address */,
	  outBuffer, outBufferBegin, outBufferMax);
}

/* ******************************************** */

static void copyMplsLabel(struct mpls_labels *mplsInfo, int labelId,
			  char *outBuffer, u_int *outBufferBegin,
			  u_int *outBufferMax) {
  if(mplsInfo == NULL) {
    int i;

    for(i=0; (i < 3) && (*outBufferBegin < *outBufferMax); i++) {
      outBuffer[*outBufferBegin] = 0;
      (*outBufferBegin)++;
    }
  } else {
    if(((*outBufferBegin)+MPLS_LABEL_LEN) < (*outBufferMax)) {
      memcpy(outBuffer, mplsInfo->mplsLabels[labelId-1], MPLS_LABEL_LEN);
      (*outBufferBegin) += MPLS_LABEL_LEN;
    }
  }
}

/* ****************************************************** */

static void exportPayload(HashBucket *myBucket, int direction,
			  V9TemplateId *theTemplate,
			  char *outBuffer, u_int *outBufferBegin,
			  u_int *outBufferMax) {
  if(maxPayloadLen > 0) {
    u_char thePayload[MAX_PAYLOAD_LEN];
    int len;

    if(direction == 0)
      len = myBucket->src2dstPayloadLen;
    else
      len = myBucket->dst2srcPayloadLen;

    /*
      u_int16_t t16;

      t16 = theTemplate->templateId;
      copyInt16(t16, outBuffer, outBufferBegin, outBufferMax);
      t16 = maxPayloadLen;
      copyInt16(t16, outBuffer, outBufferBegin, outBufferMax);
    */

    memset(thePayload, 0, maxPayloadLen);
    if(len > maxPayloadLen) len = maxPayloadLen;
    memcpy(thePayload, direction == 0 ? myBucket->src2dstPayload : myBucket->dst2srcPayload, len);

    copyLen(thePayload, maxPayloadLen, outBuffer, outBufferBegin, outBufferMax);
  }
}

/* ******************************************** */

u_int16_t ifIdx(HashBucket *myBucket, int direction, int inputIf) {
  u_char *mac;
  u_int16_t idx;

  if(num_src_mac_export > 0) {
    int i = 0;

    for(i = 0; i<num_src_mac_export; i++)
      if((((inputIf == 1) && (direction == 0)) || ((inputIf == 0) && (direction == 1)))
	 && (memcmp(myBucket->srcMacAddress, mac_if_match[i].mac_address, 6) == 0))
        return(mac_if_match[i].interface_id);
      else if((((inputIf == 0) && (direction == 0)) || ((inputIf == 1) && (direction == 1)))
	      && (memcmp(myBucket->dstMacAddress, mac_if_match[i].mac_address, 6) == 0))
        return(mac_if_match[i].interface_id);
  }

  if(direction == 0 /* src -> dst */) {
    /* Source */

    if(inputIf) {
      if(inputInterfaceIndex != (u_int16_t)-1)
	return(inputInterfaceIndex);
    } else {
      if(outputInterfaceIndex != (u_int16_t)-1)
	return(outputInterfaceIndex);
    }
    /* else dynamic */
  } else {
    /* Destination */
    if(inputIf) {
      if(outputInterfaceIndex != (u_int16_t)-1)
	return(outputInterfaceIndex);
    } else {
      if(inputInterfaceIndex != (u_int16_t)-1)
	return(inputInterfaceIndex);
    }
    /* else dynamic */
  }

  /* Calculate the input/output interface using
     the last two MAC address bytes */
  if(direction == 0 /* src -> dst */) {
    if(inputIf)
      mac = &(myBucket->srcMacAddress[4]);
    else
      mac = &(myBucket->dstMacAddress[4]);
  } else {
    if(inputIf)
      mac = &(myBucket->dstMacAddress[4]);
    else
      mac = &(myBucket->srcMacAddress[4]);
  }

  idx = (mac[0] * 256) + mac[1];

  return(idx);
}

/* ******************************************** */

static void handleTemplate(V9TemplateId *theTemplate,
			   char *outBuffer, u_int *outBufferBegin,
			   u_int *outBufferMax,
			   char buildTemplate, int *numElements,
			   HashBucket *theFlow, int direction,
			   int addTypeLen, int optionTemplate) {
  if(buildTemplate || addTypeLen) {
    u_int16_t t16;

    t16 = theTemplate->templateId;
    copyInt16(t16, outBuffer, outBufferBegin, outBufferMax);
    t16 = theTemplate->templateLen;
    copyInt16(t16, outBuffer, outBufferBegin, outBufferMax);
  }

  if(!buildTemplate) {
    if(theTemplate->templateLen == 0)
      ; /* Nothign to do: all fields have zero length */
    else {
      // traceEvent(TRACE_INFO, "[%s][%d]", theTemplate->templateName, theTemplate->templateLen);

      switch(theTemplate->templateId) {
      case 1:
	copyInt32(direction == 0 ? theFlow->bytesSent : theFlow->bytesRcvd,
		  outBuffer, outBufferBegin, outBufferMax);
	break;
      case 2:
	copyInt32(direction == 0 ? theFlow->pktSent : theFlow->pktRcvd,
		  outBuffer, outBufferBegin, outBufferMax);
	break;
      case 4:
	copyInt8((u_int8_t)theFlow->proto, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 5:
	copyInt8(direction == 0 ? theFlow->src2dstTos : theFlow->dst2srcTos,
		 outBuffer, outBufferBegin, outBufferMax);
	break;
      case 6:
	copyInt8(direction == 0 ? theFlow->src2dstTcpFlags : theFlow->dst2srcTcpFlags,
		 outBuffer, outBufferBegin, outBufferMax);
	break;
      case 7:
	copyInt16(direction == 0 ? theFlow->sport : theFlow->dport,
		  outBuffer, outBufferBegin, outBufferMax);
	break;
      case 8:
	copyInt32(direction == 0 ? theFlow->src.ipType.ipv4 : theFlow->dst.ipType.ipv4,
		  outBuffer, outBufferBegin, outBufferMax);
	break;
      case 9: /* SRC_MASK */
	copyInt8(0, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 10: /* INPUT_SNMP */
	copyInt16(ifIdx(theFlow, direction, 1), outBuffer, outBufferBegin, outBufferMax);
	break;
      case 11:
	copyInt16(direction == 0 ? theFlow->dport : theFlow->sport,
		  outBuffer, outBufferBegin, outBufferMax);
	break;
      case 12:
	copyInt32(direction == 0 ? theFlow->dst.ipType.ipv4 : theFlow->src.ipType.ipv4,
		  outBuffer, outBufferBegin, outBufferMax);
	break;
      case 13: /* DST_MASK */
	copyInt8(0, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 14: /* OUTPUT_SNMP */
	copyInt16(ifIdx(theFlow, direction, 0), outBuffer, outBufferBegin, outBufferMax);
	break;
      case 15: /* IPV4_NEXT_HOP */
	copyInt32(0, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 16:
	copyInt16(direction == 0 ? ip2AS(theFlow->src) : ip2AS(theFlow->dst),
		  outBuffer, outBufferBegin, outBufferMax);
	break;
      case 17:
	copyInt16(direction == 0 ? ip2AS(theFlow->dst) : ip2AS(theFlow->src),
		  outBuffer, outBufferBegin, outBufferMax);
	break;
      case 21:
	copyInt32(direction == 0 ? msTimeDiff(theFlow->lastSeenSent, initialSniffTime)
		  : msTimeDiff(theFlow->lastSeenRcvd, initialSniffTime),
		  outBuffer, outBufferBegin, outBufferMax);
	break;
      case 22:
	copyInt32(direction == 0 ? msTimeDiff(theFlow->firstSeenSent, initialSniffTime)
		  : msTimeDiff(theFlow->firstSeenRcvd, initialSniffTime),
		  outBuffer, outBufferBegin, outBufferMax);
	break;
      case 23:
	copyInt32(direction == 0 ? theFlow->bytesRcvd : theFlow->bytesSent,
		  outBuffer, outBufferBegin, outBufferMax);
	break;
      case 24:
	copyInt32(direction == 1 ? theFlow->pktRcvd : theFlow->pktSent,
		  outBuffer, outBufferBegin, outBufferMax);
	break;
      case 27:
	copyIpV6(direction == 0 ? theFlow->src.ipType.ipv6 : theFlow->dst.ipType.ipv6,
		 outBuffer, outBufferBegin, outBufferMax);
	break;
      case 28:
	copyIpV6(direction == 0 ? theFlow->dst.ipType.ipv6 : theFlow->src.ipType.ipv6,
		 outBuffer, outBufferBegin, outBufferMax);
	break;
      case 29:
      case 30:
	{
	  IpAddress addr;

	  memset(&addr, 0, sizeof(addr));
	  copyIpV6(addr.ipType.ipv6, outBuffer, outBufferBegin, outBufferMax);
	}
	break;
      case 32:
	copyInt16(direction == 0 ? theFlow->src2dstIcmpType : theFlow->dst2srcIcmpType,
		  outBuffer, outBufferBegin, outBufferMax);
	break;
      case 34: /* SAMPLING INTERVAL */
	copyInt32(1 /* 1:1 = no sampling */, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 35: /* SAMPLING ALGORITHM */
	copyInt8(0x01 /* 1=Deterministic Sampling, 0x02=Random Sampling */,
		 outBuffer, outBufferBegin, outBufferMax);
	break;
      case 36: /* FLOW ACTIVE TIMEOUT */
	copyInt16(lifetimeTimeout, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 37: /* FLOW INACTIVE TIMEOUT */
	copyInt16(idleTimeout, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 38:
	copyInt8((u_int8_t)engineType, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 39:
	copyInt8((u_int8_t)engineId, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 40: /* TOTAL_BYTES_EXP */
	copyInt32(totBytesExp, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 41: /* TOTAL_PKTS_EXP */
	copyInt32(totExpPktSent, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 42: /* TOTAL_FLOWS_EXP */
	copyInt32(totFlowExp, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 56: /* IN_SRC_MAC */
	copyMac(theFlow->srcMacAddress, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 57: /* OUT_DST_MAC */
	copyMac(theFlow->dstMacAddress, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 58: /* SRC_VLAN */
	/* no break */
      case 59: /* DST_VLAN */
	copyInt16(theFlow->vlanId, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 60: /* IP_PROTOCOL_VERSION */
	copyInt8(useIpV6 == 1 ? 6 : 4, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 61: /* Direction */
	copyInt8(direction, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 70: /* MPLS: label 1 */
	copyMplsLabel(theFlow->mplsInfo, 1, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 71: /* MPLS: label 2 */
	copyMplsLabel(theFlow->mplsInfo, 2, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 72: /* MPLS: label 3 */
	copyMplsLabel(theFlow->mplsInfo, 3, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 73: /* MPLS: label 4 */
	copyMplsLabel(theFlow->mplsInfo, 4, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 74: /* MPLS: label 5 */
	copyMplsLabel(theFlow->mplsInfo, 5, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 75: /* MPLS: label 6 */
	copyMplsLabel(theFlow->mplsInfo, 6, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 76: /* MPLS: label 7 */
	copyMplsLabel(theFlow->mplsInfo, 7, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 77: /* MPLS: label 8 */
	copyMplsLabel(theFlow->mplsInfo, 8, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 78: /* MPLS: label 9 */
	copyMplsLabel(theFlow->mplsInfo, 9, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 79: /* MPLS: label 10 */
	copyMplsLabel(theFlow->mplsInfo, 10, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 80: /* IN_DST_MAC */
	copyMac(theFlow->dstMacAddress, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 81: /* OUT_SRC_MAC */
	copyMac(theFlow->srcMacAddress, outBuffer, outBufferBegin, outBufferMax);
	break;

	/* ************************************ */

	/* nFlow Extensions */
      case 90:
	copyInt8(direction == 0 ? fragmentedPacketSrc2Dst(theFlow) :
		 fragmentedPacketSrc2Dst(theFlow),
		 outBuffer, outBufferBegin, outBufferMax);
	break;
      case 91:
	copyLen(direction == 0 ? theFlow->src2dstFingerprint : theFlow->dst2srcFingerprint,
		FINGERPRINT_LEN,
		outBuffer, outBufferBegin, outBufferMax);
	break;
      case 92:
	copyInt32(nwLatencyComputed(theFlow) ? theFlow->nwLatency.tv_sec : 0,
		  outBuffer, outBufferBegin, outBufferMax);
	break;
      case 93:
	copyInt32(nwLatencyComputed(theFlow) ? theFlow->nwLatency.tv_usec : 0,
		  outBuffer, outBufferBegin, outBufferMax);
	break;
      case 94:
	copyInt32(applLatencyComputed(theFlow) ? (direction == 0 ? theFlow->src2dstApplLatency.tv_sec
						  : theFlow->dst2srcApplLatency.tv_sec) : 0,
		  outBuffer, outBufferBegin, outBufferMax);
	break;
      case 95:
	copyInt32(applLatencyComputed(theFlow) ?
		  (direction == 0 ? theFlow->src2dstApplLatency.tv_usec :
		   theFlow->dst2srcApplLatency.tv_usec) : 0,
		  outBuffer, outBufferBegin, outBufferMax);
	break;
      case IN_PAYLOAD_ID:
	exportPayload(theFlow, 0, theTemplate, outBuffer, outBufferBegin, outBufferMax);
	break;
      case OUT_PAYLOAD_ID:
	exportPayload(theFlow, 1, theTemplate, outBuffer, outBufferBegin, outBufferMax);
	break;
      case 98:
        copyInt32(direction == 0 ? theFlow->src2dstIcmpFlags : theFlow->dst2srcIcmpFlags,
                  outBuffer, outBufferBegin, outBufferMax);
        break;
      default:
	if(checkPluginExport(theTemplate, direction, theFlow,
			     outBuffer, outBufferBegin, outBufferMax) != 0) {
	  u_char null_data[128] = { 0 };
	/*
	  This flow is the one we like, however we need
	  to store some values anyway, so we put an empty value
	*/


	  copyLen(null_data, theTemplate->templateLen,
		  outBuffer, outBufferBegin, outBufferMax);
	}
      }
    }

#ifdef DEBUG
    traceEvent(TRACE_INFO, "name=%s/Id=%d/len=%d\n",
	       theTemplate->templateName, theTemplate->templateId, *outBufferBegin);
#endif
  }

  (*numElements) = (*numElements)+1;
  return;
}

/* ******************************************** */

void flowPrintf(V9TemplateId **templateList, char *outBuffer,
		u_int *outBufferBegin, u_int *outBufferMax,
		int *numElements, char buildTemplate,
		HashBucket *theFlow, int direction,
		int addTypeLen, int optionTemplate) {
  int idx = 0;

  (*numElements) = 0;

  while(templateList[idx] != NULL) {
    handleTemplate(templateList[idx], outBuffer, outBufferBegin, outBufferMax,
		   buildTemplate, numElements,
		   theFlow, direction, addTypeLen,
		   optionTemplate);
    idx++;
  }
}

/* ******************************************** */

void compileTemplate(char *_fmt, V9TemplateId **templateList, int templateElements) {
  int idx=0, endIdx, i, templateIdx, len = strlen(_fmt);
  char fmt[1024], tmpChar, found;

  templateIdx = 0;
  snprintf(fmt, sizeof(fmt), "%s", _fmt);

  while((idx < len) && (fmt[idx] != '\0')) {	/* scan format string characters */
    switch(fmt[idx]) {
    case '%':	        /* special format follows */
      endIdx = ++idx;
      while(fmt[endIdx] != '\0') {
	if((fmt[endIdx] == ' ') || (fmt[endIdx] == '%'))
	  break;
	else
	  endIdx++;
      }

      if((endIdx == (idx+1)) && (fmt[endIdx] == '\0')) return;
      tmpChar = fmt[endIdx]; fmt[endIdx] = '\0';

      i = 0, found = 0;

      while(ver9_templates[i].templateName != NULL) {
	if(strcmp(&fmt[idx], ver9_templates[i].templateName) == 0) {
	  templateList[templateIdx++] = &ver9_templates[i];
	  found = 1;
	  break;
	}

	i++;
      }

      if(!found) {
	if((templateList[templateIdx] = getPluginTemplate(&fmt[idx])) != NULL)
	  templateIdx++;
	else
	  traceEvent(TRACE_WARNING, "Unable to locate template '%s'. Discarded.", &fmt[idx]);
      }

      if(templateIdx >= (templateElements-1)) {
	traceEvent(TRACE_WARNING, "Unable to add further template elements (%d).", templateIdx);
	break;
      }

      fmt[endIdx] = tmpChar;
      if(tmpChar == '%')
	idx = endIdx;
      else
	idx = endIdx+1;
      break;
    default:
      idx++;
      break;
    }
  }

  templateList[templateIdx] = NULL;
}

/* ******************************************** */

double toMs(struct timeval theTime) {
  return(theTime.tv_sec+(float)theTime.tv_usec/1000000);
}

/* ****************************************************** */

u_int32_t msTimeDiff(struct timeval end, struct timeval begin) {
  if((end.tv_sec == 0) && (end.tv_usec == 0))
    return(0);
  else
    return((end.tv_sec-begin.tv_sec)*1000+(end.tv_usec-begin.tv_usec)/1000);
}

/* ****************************************************** */

#ifndef WIN32
int createCondvar(ConditionalVariable *condvarId) {
  int rc;

  rc = pthread_mutex_init(&condvarId->mutex, NULL);
  rc = pthread_cond_init(&condvarId->condvar, NULL);
  condvarId->predicate = 0;

  return(rc);
}

/* ************************************ */

void deleteCondvar(ConditionalVariable *condvarId) {
  pthread_mutex_destroy(&condvarId->mutex);
  pthread_cond_destroy(&condvarId->condvar);
}

/* ************************************ */

int waitCondvar(ConditionalVariable *condvarId) {
  int rc;

  if((rc = pthread_mutex_lock(&condvarId->mutex)) != 0)
    return rc;

  while(condvarId->predicate <= 0) {
    rc = pthread_cond_wait(&condvarId->condvar, &condvarId->mutex);
  }

  condvarId->predicate--;

  rc = pthread_mutex_unlock(&condvarId->mutex);

  return rc;
}
/* ************************************ */

int signalCondvar(ConditionalVariable *condvarId, int broadcast) {
  int rc;

  rc = pthread_mutex_lock(&condvarId->mutex);

  condvarId->predicate++;

  rc = pthread_mutex_unlock(&condvarId->mutex);
  if(broadcast)
    rc = pthread_cond_broadcast(&condvarId->condvar);
  else
    rc = pthread_cond_signal(&condvarId->condvar);

  return rc;
}

#undef sleep /* Used by ntop_sleep */

#else /* WIN32 */

/* ************************************ */

int createCondvar(ConditionalVariable *condvarId) {
  condvarId->condVar = CreateEvent(NULL,  /* no security */
				   TRUE , /* auto-reset event (FALSE = single event, TRUE = broadcast) */
				   FALSE, /* non-signaled initially */
				   NULL); /* unnamed */
  InitializeCriticalSection(&condvarId->criticalSection);
  return(1);
}

/* ************************************ */

void deleteCondvar(ConditionalVariable *condvarId) {
  CloseHandle(condvarId->condVar);
  DeleteCriticalSection(&condvarId->criticalSection);
}

/* ************************************ */

int waitCondvar(ConditionalVariable *condvarId) {
  int rc;
#ifdef DEBUG
  traceEvent(CONST_TRACE_INFO, "Wait (%x)...", condvarId->condVar);
#endif
  EnterCriticalSection(&condvarId->criticalSection);
  rc = WaitForSingleObject(condvarId->condVar, INFINITE);
  LeaveCriticalSection(&condvarId->criticalSection);

#ifdef DEBUG
  traceEvent(CONST_TRACE_INFO, "Got signal (%d)...", rc);
#endif

  return(rc);
}

/* ************************************ */

/* NOTE: broadcast is currently ignored */
int signalCondvar(ConditionalVariable *condvarId, int broadcast) {
#ifdef DEBUG
  traceEvent(CONST_TRACE_INFO, "Signaling (%x)...", condvarId->condVar);
#endif
  return((int)PulseEvent(condvarId->condVar));
}

#define sleep(a /* sec */) waitForNextEvent(1000*a /* ms */)

#endif /* WIN32 */

/* ******************************************* */

unsigned int ntop_sleep(unsigned int secs) {
  unsigned int unsleptTime = secs, rest;

  while((rest = sleep(unsleptTime)) > 0)
    unsleptTime = rest;

  return(secs);
}

/* ******************************************* */

HashBucket* getListHead(HashBucket **list) {
  HashBucket *bkt = *list;

  (*list) = bkt->next;

  return(bkt);
}

/* ******************************************* */

void addToList(HashBucket *bkt, HashBucket **list) {
  bkt->next = *list;
  (*list) = bkt;
}

/* **************************************** */

#ifndef WIN32

void detachFromTerminal(int doChdir) {
  if(doChdir) (void)chdir("/");
  setsid();  /* detach from the terminal */

  fclose(stdin);
  fclose(stdout);
  /* fclose(stderr); */

  /*
   * clear any inherited file mode creation mask
   */
  umask (0);

  /*
   * Use line buffered stdout
   */
  /* setlinebuf (stdout); */
  setvbuf(stdout, (char *)NULL, _IOLBF, 0);
}

/* **************************************** */

void daemonize(void) {
  int childpid;

  signal(SIGHUP, SIG_IGN);
  signal(SIGCHLD, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);

  if((childpid = fork()) < 0)
    traceEvent(TRACE_ERROR, "INIT: Occurred while daemonizing (errno=%d)", errno);
  else {
#ifdef DEBUG
    traceEvent(TRACE_INFO, "DEBUG: after fork() in %s (%d)",
	       childpid ? "parent" : "child", childpid);
#endif
    if(!childpid) { /* child */
      traceEvent(TRACE_INFO, "INIT: Bye bye: I'm becoming a daemon...");
      detachFromTerminal(1);
    } else { /* father */
      traceEvent(TRACE_INFO, "INIT: Parent process is exiting (this is normal)");
      exit(0);
    }
  }
}

#endif /* WIN32 */

/* ****************************************

   Address management

   **************************************** */

static int int2bits(int number) {
  int bits = 8;
  int test;

  if((number > 255) || (number < 0))
    return(CONST_INVALIDNETMASK);
  else {
    test = ~number & 0xff;
    while (test & 0x1)
      {
	bits --;
	test = test >> 1;
      }
    if(number != ((~(0xff >> bits)) & 0xff))
      return(CONST_INVALIDNETMASK);
    else
      return(bits);
  }
}

/* ********************** */

static int dotted2bits(char *mask) {
  int		fields[4];
  int		fields_num, field_bits;
  int		bits = 0;
  int		i;

  fields_num = sscanf(mask, "%d.%d.%d.%d",
		      &fields[0], &fields[1], &fields[2], &fields[3]);
  if((fields_num == 1) && (fields[0] <= 32) && (fields[0] >= 0))
    {
#ifdef DEBUG
      traceEvent(CONST_TRACE_INFO, "DEBUG: dotted2bits (%s) = %d", mask, fields[0]);
#endif
      return(fields[0]);
    }
  for (i=0; i < fields_num; i++)
    {
      /* We are in a dotted quad notation. */
      field_bits = int2bits (fields[i]);
      switch (field_bits)
	{
	case CONST_INVALIDNETMASK:
	  return(CONST_INVALIDNETMASK);

	case 0:
	  /* whenever a 0 bits field is reached there are no more */
	  /* fields to scan                                       */
	  /* In this case we are in a bits (not dotted quad) notation */
	  return(bits /* fields[0] - L.Deri 08/2001 */);

	default:
	  bits += field_bits;
	}
    }
  return(bits);
}

/* ********************** */

void parseLocalAddressLists(char* _addresses) {
  char *address, *addresses, *strTokState;

  numLocalNetworks = 0;

  if((_addresses == NULL) || (_addresses[0] == '\0'))
    return;

  addresses = strdup(_addresses);
  address = strtok_r(addresses, ",", &strTokState);

  while(address != NULL) {
    char *mask = strchr(address, '/');

    if(mask == NULL) {
      traceEvent(TRACE_WARNING, "Empty mask '%s' - ignoring entry", address);
    } else {
      u_int32_t network, networkMask, broadcast;
      int bits, a, b, c, d;

      mask[0] = '\0';
      mask++;
      bits = dotted2bits (mask);

      if(sscanf(address, "%d.%d.%d.%d", &a, &b, &c, &d) != 4) {
	address = strtok_r(NULL, ",", &strTokState);
	continue;
      }

      if(bits == CONST_INVALIDNETMASK) {
        traceEvent(TRACE_WARNING, "netmask '%s' not valid - ignoring entry", mask);
	/* malformed netmask specification */
	address = strtok_r(NULL, ",", &strTokState);
	continue;
      }

      network = ((a & 0xff) << 24) + ((b & 0xff) << 16) + ((c & 0xff) << 8) + (d & 0xff);
      /* Special case the /32 mask - yeah, we could probably do it with some fancy
         u long long stuff, but this is simpler...
         Burton Strauss <Burton@ntopsupport.com> Jun2002
      */
      if(bits == 32) {
	networkMask = 0xffffffff;
      } else {
	networkMask = 0xffffffff >> bits;
	networkMask = ~networkMask;
      }

      if((network & networkMask) != network)  {
	/* malformed network specification */

	traceEvent(TRACE_WARNING, "%d.%d.%d.%d/%d is not a valid network - correcting mask",
		   a, b, c, d, bits);
	/* correcting network numbers as specified in the netmask */
	network &= networkMask;

	a = (int) ((network >> 24) & 0xff);
	b = (int) ((network >> 16) & 0xff);
	c = (int) ((network >>  8) & 0xff);
	d = (int) ((network >>  0) & 0xff);

	/*
	  traceEvent(CONST_TRACE_NOISY, "Assuming %d.%d.%d.%d/%d [0x%08x/0x%08x]",
	  a, b, c, d, bits, network, networkMask);
	*/
      }

      broadcast = network | (~networkMask);

      a = (int) ((network >> 24) & 0xff);
      b = (int) ((network >> 16) & 0xff);
      c = (int) ((network >>  8) & 0xff);
      d = (int) ((network >>  0) & 0xff);

      traceEvent(TRACE_INFO, "Adding %d.%d.%d.%d/%d to the local network list",
		 a, b, c, d, bits);

      /* NOTE: entries are saved in network byte order for performance reasons */
      localNetworks[numLocalNetworks][CONST_NETWORK_ENTRY]   = htonl(network);
      localNetworks[numLocalNetworks][CONST_NETMASK_ENTRY]   = htonl(networkMask);
      localNetworks[numLocalNetworks][CONST_BROADCAST_ENTRY] = htonl(broadcast);
      numLocalNetworks++;
    }

    address = strtok_r(NULL, ",", &strTokState);
  }

  free(addresses);
}

/* ************************************************ */

#undef DEBUG

unsigned short isLocalAddress(struct in_addr *addr) {
  int i;
#ifdef DEBUG
  char buf[64];
#endif

  /* If unset all the addresses are local */
  if(numLocalNetworks == 0) return(1);

  for(i=0; i<numLocalNetworks; i++)
    if((addr->s_addr & localNetworks[i][CONST_NETMASK_ENTRY])
       == localNetworks[i][CONST_NETWORK_ENTRY]) {
#ifdef DEBUG
      traceEvent(TRACE_INFO, "%s is local",
		 _intoaV4(ntohl(addr->s_addr), buf, sizeof(buf)));
#endif
      return 1;
    }

#ifdef DEBUG
  traceEvent(TRACE_INFO, "%s is NOT local",
	     _intoaV4(ntohl(addr->s_addr), buf, sizeof(buf)));
#endif
  return(0);
}

/* ************************************************ */

/* Utility function */
uint32_t str2addr(char *address) {
  int a, b, c, d;

  if(sscanf(address, "%d.%d.%d.%d", &a, &b, &c, &d) != 4) {
    return(0);
  } else
    return(((a & 0xff) << 24) + ((b & 0xff) << 16) + ((c & 0xff) << 8) + (d & 0xff));
}

/* ************************************************ */

static char hex[] = "0123456789ABCDEF";

char* etheraddr_string(const u_char *ep, char *buf) {
  u_int i, j;
  char *cp;

  cp = buf;
  if ((j = *ep >> 4) != 0)
    *cp++ = hex[j];
  else
    *cp++ = '0';

  *cp++ = hex[*ep++ & 0xf];

  for(i = 5; (int)--i >= 0;) {
    *cp++ = ':';
    if ((j = *ep >> 4) != 0)
      *cp++ = hex[j];
    else
      *cp++ = '0';

    *cp++ = hex[*ep++ & 0xf];
  }

  *cp = '\0';
  return (buf);
}

/* ************************************ */

/*
 * Returns the difference between gmt and local time in seconds.
 * Use gmtime() and localtime() to keep things simple.
 *
 * Code inherited from tcpdump
 *
 */
int32_t gmt2local(time_t t) {
  register int dt, dir;
  register struct tm *gmt, *loc;
  struct tm sgmt;

  if(t == 0)
    t = time(NULL);
  gmt = &sgmt;
  *gmt = *gmtime(&t);
  loc = localtime(&t);
  dt = (loc->tm_hour - gmt->tm_hour) * 60 * 60 +
    (loc->tm_min - gmt->tm_min) * 60;

  /*
   * If the year or julian day is different, we span 00:00 GMT
   * and must add or subtract a day. Check the year first to
   * avoid problems when the julian day wraps.
   */
  dir = loc->tm_year - gmt->tm_year;
  if(dir == 0)
    dir = loc->tm_yday - gmt->tm_yday;
  dt += dir * 24 * 60 * 60;

  return (dt);
}

/* *********************************************************** */

void resetBucketStats(HashBucket* bkt,
		      const struct pcap_pkthdr *h,
		      u_int len, 
		      u_short sport, u_short dport,
		      u_char *payload, int payloadLen) {
  if(bkt->sport == sport) {
    bkt->bytesSent = len, bkt->pktSent = 1, bkt->bytesRcvd = bkt->pktRcvd = 0;
    memcpy(&bkt->firstSeenSent, &h->ts, sizeof(struct timeval));
    memcpy(&bkt->lastSeenSent, &h->ts, sizeof(struct timeval));
    memset(&bkt->firstSeenRcvd, 0, sizeof(struct timeval));
    memset(&bkt->lastSeenRcvd, 0, sizeof(struct timeval));
  } else {
    bkt->bytesSent = bkt->pktSent = 0, bkt->bytesRcvd = len, bkt->pktRcvd = 1;
    memset(&bkt->firstSeenRcvd, 0, sizeof(struct timeval));
    memset(&bkt->lastSeenRcvd, 0, sizeof(struct timeval));
    memcpy(&bkt->firstSeenSent, &h->ts, sizeof(struct timeval));
    memcpy(&bkt->lastSeenRcvd, &h->ts, sizeof(struct timeval));
  }

  if(bkt->src2dstPayload) { free(bkt->src2dstPayload);  bkt->src2dstPayload = NULL;  }
  if(bkt->dst2srcPayload) { free(bkt->dst2srcPayload); bkt->dst2srcPayload = NULL; }  
  setPayload(bkt, h, payload, payloadLen, bkt->sport == sport ? 0 : 1);
}
