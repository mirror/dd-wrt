/*
 *  Copyright (C) 2002-04 Luca Deri <deri@ntop.org>
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

/* ****************************** */

HashBucket **theHash, *purgedBuckets;
u_char traceMode, ignoreAS;
#ifndef WIN32
int useSyslog = 0;
#endif
int traceLevel = 2;
u_short maxPayloadLen;
u_short idleTimeout = DUMP_TIMEOUT;
u_short lifetimeTimeout = 4*DUMP_TIMEOUT;
u_short sendTimeout   = DUMP_TIMEOUT;
u_int maxBucketSearch, hashSize;
struct timeval actTime;
u_short engineType, engineId;
u_char tcpPayloadExport, udpPayloadExport, icmpPayloadExport, otherPayloadExport;

/* Extern */
extern pthread_mutex_t purgedBucketsMutex, hashMutex[MAX_HASH_MUTEXES];
extern u_int bucketsAllocated, maxNumActiveFlows, droppedPktsTooManyFlows;
extern u_int32_t purgedBucketsLen;
extern char *pcapFile;
extern u_char calculateJitter;
extern char *dirPath;
extern FILE *flowFd;
extern u_int minFlowSize, totFlows;

#define TCP_PROTOCOL               0x06

/* ****************************** */

/*
 * A faster replacement for inet_ntoa().
 */
char* _intoaV4(unsigned int addr, char* buf, u_short bufLen) {
  char *cp, *retStr;
  u_int byte;
  int n;

  cp = &buf[bufLen];
  *--cp = '\0';

  n = 4;
  do {
    byte = addr & 0xff;
    *--cp = byte % 10 + '0';
    byte /= 10;
    if (byte > 0) {
      *--cp = byte % 10 + '0';
      byte /= 10;
      if (byte > 0)
	*--cp = byte + '0';
    }
    *--cp = '.';
    addr >>= 8;
  } while (--n > 0);

  /* Convert the string to lowercase */
  retStr = (char*)(cp+1);

  return(retStr);
}

/* ****************************** */

char* _intoa(IpAddress addr, char* buf, u_short bufLen) {
  if(addr.ipVersion == 4)
    return(_intoaV4(addr.ipType.ipv4, buf, bufLen));
  else {
    char *ret;
#ifdef IPV4_ONLY
    ret = "???";
#else
    int len;

    ret = (char*)inet_ntop(AF_INET6, &addr.ipType.ipv6, &buf[1], bufLen-2);
    len = strlen(ret);
    buf[0] = '[';
    buf[len+1] = ']';
    buf[len+2] = '\0';
    ret = buf;
#endif

    return(ret);
  }
}

/* ****************************************************** */

char* formatTraffic(float numBits, int bits, char *buf) {
  char unit;

  if(bits)
    unit = 'b';
  else
    unit = 'B';

  if(numBits < 1024) {
    snprintf(buf, 32, "%lu %c", (unsigned long)numBits, unit);
  } else if (numBits < 1048576) {
    snprintf(buf, 32, "%.0f K%c", (float)(numBits)/1024, unit);
  } else {
    float tmpMBits = ((float)numBits)/1048576;

    if(tmpMBits < 1024) {
      snprintf(buf, 32, "%.0f M%c", tmpMBits, unit);
    } else {
      tmpMBits /= 1024;

      if(tmpMBits < 1024) {
	snprintf(buf, 32, "%.0f G%c", tmpMBits, unit);
      } else {
	snprintf(buf, 32, "%.0f T%c", (float)(tmpMBits)/1024, unit);
      }
    }
  }

  return(buf);
}

/* ******************************************************** */

/*
  Fingerprint code courtesy of ettercap
  http://ettercap.sourceforge.net
*/
u_char ttlPredictor(u_char x) {		
  /* coded by awgn <awgn@antifork.org> */
  /* round the TTL to the nearest power of 2 (ceiling) */
  u_char i = x, j = 1, c = 0;
  
  do {
    c += i & 1;
    j <<= 1;
  } while(i >>= 1);

  if(c == 1)
    return(x);
  else
    return(j ? j : 0xff);
}

/* ******************************************************** */

char* proto2name(u_short proto) {
  static char protoName[8];

  switch(proto) {
  case IPPROTO_TCP:    return("TCP");
  case IPPROTO_UDP:    return("UDP");
  case IPPROTO_ICMP:   return("ICMP");
  case IPPROTO_ICMPV6: return("ICMPV6");
  case 2:              return("IGMP");
  case 47:             return("GRE");
  default:
    snprintf(protoName, sizeof(protoName), "%d", proto);
    return(protoName);
  }
}

/* ****************************************************** */

void setPayload(HashBucket *bkt, const struct pcap_pkthdr *h, 
		u_char *payload, int payloadLen,  int direction) {

  if(payloadLen > 0) {
    int diff;

    if(direction == 0) {
      if(bkt->src2dstPayload == NULL)
	bkt->src2dstPayload = (u_char*)malloc(sizeof(char)*(maxPayloadLen+1));

      if(bkt->src2dstPayload != NULL) {
	diff = maxPayloadLen-bkt->src2dstPayloadLen;
	
	if(diff > 0) {
	  if(diff > payloadLen) diff = payloadLen;
	  memcpy(&bkt->src2dstPayload[bkt->src2dstPayloadLen], payload, diff);
	  bkt->src2dstPayloadLen += diff;
	}
      } else
	traceEvent(TRACE_ERROR, "Not enough memory?");
    } else {
      if(bkt->dst2srcPayload == NULL)
	bkt->dst2srcPayload = (u_char*)malloc(sizeof(char)*(maxPayloadLen+1));

      if(bkt->dst2srcPayload != NULL) {
	diff = maxPayloadLen-bkt->dst2srcPayloadLen;

	if(diff > 0) {
	  if(diff > payloadLen) diff = payloadLen;
	  memcpy(&bkt->dst2srcPayload[bkt->dst2srcPayloadLen], payload, diff);
	  bkt->dst2srcPayloadLen += diff;
	}
      } else
	traceEvent(TRACE_ERROR, "Not enough memory?");
    }

    /* Jitter Calculation */
  }
}

/* ************************************************* */

void updateApplLatency(u_short proto, HashBucket *bkt,
		       int direction, struct timeval *stamp,
		       u_int8_t icmpType, u_int8_t icmpCode) {

  if(!applLatencyComputed(bkt)) {
    /*
      src ---------> dst -+
      | Application
      | Latency
      <--------      -+

      NOTE:
      1. Application latency is calculated as the time passed since the first
      packet sent the first packet on the opposite direction is received.
      2. Application latency is calculated only on the first packet

    */

    if(direction  == 0) {
      /* src->dst */
      if(bkt->src2dstApplLatency.tv_sec == 0)
	bkt->src2dstApplLatency.tv_sec = stamp->tv_sec, bkt->src2dstApplLatency.tv_usec = stamp->tv_usec;

      if(bkt->dst2srcApplLatency.tv_sec != 0) {
	bkt->dst2srcApplLatency.tv_sec  = bkt->src2dstApplLatency.tv_sec-bkt->dst2srcApplLatency.tv_sec;

	if((bkt->src2dstApplLatency.tv_usec-bkt->dst2srcApplLatency.tv_usec) < 0) {
	  bkt->dst2srcApplLatency.tv_usec = 1000000 + bkt->src2dstApplLatency.tv_usec - bkt->dst2srcApplLatency.tv_usec;
	  if(bkt->dst2srcApplLatency.tv_usec > 1000000) bkt->dst2srcApplLatency.tv_usec = 1000000;
	  bkt->dst2srcApplLatency.tv_sec--;
	} else
	  bkt->dst2srcApplLatency.tv_usec = bkt->src2dstApplLatency.tv_usec-bkt->dst2srcApplLatency.tv_usec;

	bkt->src2dstApplLatency.tv_sec = 0, bkt->src2dstApplLatency.tv_usec = 0;
	NPROBE_FD_SET(FLAG_APPL_LATENCY_COMPUTED, &(bkt->flags));
      }
    } else {
      /* dst -> src */
      if(bkt->dst2srcApplLatency.tv_sec == 0)
	bkt->dst2srcApplLatency.tv_sec = stamp->tv_sec, bkt->dst2srcApplLatency.tv_usec = stamp->tv_usec;

      if(bkt->src2dstApplLatency.tv_sec != 0) {
	bkt->src2dstApplLatency.tv_sec  = bkt->dst2srcApplLatency.tv_sec-bkt->src2dstApplLatency.tv_sec;

	if((bkt->dst2srcApplLatency.tv_usec-bkt->src2dstApplLatency.tv_usec) < 0) {
	  bkt->src2dstApplLatency.tv_usec = 1000000 + bkt->dst2srcApplLatency.tv_usec - bkt->src2dstApplLatency.tv_usec;
	  if(bkt->src2dstApplLatency.tv_usec > 1000000) bkt->src2dstApplLatency.tv_usec = 1000000;
	  bkt->src2dstApplLatency.tv_sec--;
	} else
	  bkt->src2dstApplLatency.tv_usec = bkt->dst2srcApplLatency.tv_usec-bkt->src2dstApplLatency.tv_usec;

	bkt->dst2srcApplLatency.tv_sec = 0, bkt->dst2srcApplLatency.tv_usec = 0;
	NPROBE_FD_SET(FLAG_APPL_LATENCY_COMPUTED, &(bkt->flags));
      }
    }

#if 0
    if(applLatencyComputed(bkt)) {
      char buf[32], buf1[32];

      if(bkt->src2dstApplLatency.tv_sec || bkt->src2dstApplLatency.tv_usec)
	printf("[Appl: %.2f ms (%s->%s)]", (float)(bkt->src2dstApplLatency.tv_sec*1000
						   +(float)bkt->src2dstApplLatency.tv_usec/1000),
	       _intoa(bkt->src, buf, sizeof(buf)), _intoa(bkt->dst, buf1, sizeof(buf1)));
      else
	printf("[Appl: %.2f ms (%s->%s)]", (float)(bkt->dst2srcApplLatency.tv_sec*1000
						   +(float)bkt->dst2srcApplLatency.tv_usec/1000),
	       _intoa(bkt->dst, buf, sizeof(buf)), _intoa(bkt->src, buf1, sizeof(buf1))
	       );
    }
#endif
  }

  if(proto == IPPROTO_ICMP) {
    u_int8_t val = (256 * icmpType) + icmpCode;

    if(direction == 0)
      bkt->src2dstIcmpType = val;
    else
      bkt->dst2srcIcmpType = val;
  }
}

/* ****************************************************** */

void updateTcpFlags(HashBucket *bkt, int direction,
		    struct timeval *stamp, u_int8_t flags,
		    char *fingerprint, u_char tos) {
  if(direction  == 0)
    bkt->src2dstTos |= tos;
  else
    bkt->dst2srcTos |= tos;

  if(!nwLatencyComputed(bkt)) {
    if(flags == TH_SYN) {
      bkt->nwLatency.tv_sec = stamp->tv_sec;
      bkt->nwLatency.tv_usec = stamp->tv_usec;
    } else if(flags == TH_ACK) {
      if(bkt->nwLatency.tv_sec == 0) {
	/* We missed the SYN flag */
	NPROBE_FD_SET(FLAG_NW_LATENCY_COMPUTED,   &(bkt->flags));
	NPROBE_FD_SET(FLAG_APPL_LATENCY_COMPUTED, &(bkt->flags)); /* We cannot calculate it as we have
								     missed the 3-way handshake */
	return;
      }

      if(((direction  == 0)    && (bkt->src2dstTcpFlags != TH_SYN))
	 || ((direction  == 1) && (bkt->dst2srcTcpFlags != TH_SYN)))
	return; /* Wrong flags */

      if(stamp->tv_sec >= bkt->nwLatency.tv_sec) {
	bkt->nwLatency.tv_sec = stamp->tv_sec-bkt->nwLatency.tv_sec;

	if((stamp->tv_usec - bkt->nwLatency.tv_usec) < 0) {
	  bkt->nwLatency.tv_usec = 1000000 + stamp->tv_usec - bkt->nwLatency.tv_usec;
	  if(bkt->nwLatency.tv_usec > 1000000) bkt->nwLatency.tv_usec = 1000000;
	  bkt->nwLatency.tv_sec--;
	} else
	  bkt->nwLatency.tv_usec = stamp->tv_usec-bkt->nwLatency.tv_usec;

	bkt->nwLatency.tv_sec /= 2;
	bkt->nwLatency.tv_usec /= 2;
      } else
	bkt->nwLatency.tv_sec = 0, bkt->nwLatency.tv_usec = 0;

#if 0
      printf("[Net: %.1f ms]",
	     (float)(bkt->nwLatency.tv_sec*1000+(float)bkt->nwLatency.tv_usec/1000));
#endif

      NPROBE_FD_SET(FLAG_NW_LATENCY_COMPUTED, &(bkt->flags));
      updateApplLatency(IPPROTO_TCP, bkt, direction, stamp, 0, 0);
    }
  } else {
    /* Nw latency computed */
    if(!applLatencyComputed(bkt)) {
      /*
	src ---------> dst -+
	| Application
	| Latency
	<--------      -+

	NOTE:
	1. Application latency is calculated as the time passed since the first
	packet sent after the 3-way handshake until the first packet on
	the opposite direction is received.
	2. Application latency is calculated only on the first packet
      */
      
      updateApplLatency(IPPROTO_TCP, bkt, direction, stamp, 0, 0);
    }
  }

  if(fingerprint != NULL) {
    if((direction == 0) && (bkt->src2dstFingerprint[0] == '\0'))
      memcpy(bkt->src2dstFingerprint, fingerprint, FINGERPRINT_LEN);
    else if((direction == 1) && (bkt->dst2srcFingerprint[0] == '\0'))
      memcpy(bkt->dst2srcFingerprint, fingerprint, FINGERPRINT_LEN);
  }
}

/* ****************************************************** */

/*
   1 - equal
   0 - different
*/
int cmpIpAddress(IpAddress src, IpAddress dst) {
  if(src.ipVersion != dst.ipVersion) return(0);

  if(src.ipVersion == 4) {
    return(src.ipType.ipv4 == dst.ipType.ipv4 ? 1 : 0);
  } else {
    return(!memcmp(&src.ipType.ipv6, &dst.ipType.ipv6, sizeof(struct in6_addr)));
  }
}

/* ****************************************************** */

void addPktToHash(u_short proto, u_char isFragment,
		  u_short numPkts, u_char tos,
		  u_short vlanId, struct ether_header *ehdr,
		  IpAddress src, u_short sport,
		  IpAddress dst, u_short dport,
		  u_int  len, u_int8_t flags, 
		  u_int8_t icmpType, u_int8_t icmpCode,
		  u_short numMplsLabels,
		  u_char mplsLabels[MAX_NUM_MPLS_LABELS][MPLS_LABEL_LEN],
		  char *fingerprint,
		  struct pcap_pkthdr *h, u_char *p,
		  u_char *payload, int payloadLen) {
  u_int32_t n=0, mutexIdx, idx; /* (src+dst+sport+dport) % hashSize; */
  HashBucket *bkt;
  u_int32_t srcHost=0, dstHost=0;

  if(shutdownInProgress) return;

  if(src.ipVersion == 4) {
    srcHost = src.ipType.ipv4, dstHost = dst.ipType.ipv4;
  } else {
    srcHost = src.ipType.ipv6.s6_addr32[0]+src.ipType.ipv6.s6_addr32[1]
      +src.ipType.ipv6.s6_addr32[2]+src.ipType.ipv6.s6_addr32[3];
    dstHost = dst.ipType.ipv6.s6_addr32[0]+dst.ipType.ipv6.s6_addr32[1]
      +dst.ipType.ipv6.s6_addr32[2]+dst.ipType.ipv6.s6_addr32[3];
  }

  idx = vlanId+srcHost+dstHost+sport+dport;
  idx %= hashSize;

  if(pcapFile == NULL) /* Live capture */
    actTime.tv_sec = h->ts.tv_sec, actTime.tv_usec = h->ts.tv_usec;

#if 0
  {
    char buf[256], buf1[256];

    printf("[%4s] %s:%d -> %s:%d [len=%u][payloadLen=%d][idx=%d]\n",
	   proto2name(proto),
	   _intoa(src, buf, sizeof(buf)), (int)sport,
	   _intoa(dst, buf1, sizeof(buf1)), (int)dport,
	   len, payloadLen, idx);
  }
#endif

  mutexIdx = idx % MAX_HASH_MUTEXES;
  pthread_mutex_lock(&hashMutex[mutexIdx]);

  bkt = theHash[idx];

  while(bkt != NULL) {
#ifdef ENABLE_MAGIC
    if(bkt->magic != 67) {
      printf("Error: magic error detected (%d)", bkt->magic);
    }
#endif

    if((bkt->proto == proto)
       && (bkt->vlanId == vlanId)
       && ((cmpIpAddress(bkt->src, src)
	    && cmpIpAddress(bkt->dst, dst)
	    && (bkt->sport == sport)
	    && (bkt->dport == dport))
	   || (cmpIpAddress(bkt->src, dst)
	       && cmpIpAddress(bkt->dst, src)
	       && (bkt->sport == dport)
	       && (bkt->dport == sport)))) {
      if(bkt->src.ipType.ipv4 == src.ipType.ipv4) {
	bkt->bytesSent += len, bkt->pktSent += numPkts;
	bkt->lastSeenSent.tv_sec = h->ts.tv_sec, bkt->lastSeenSent.tv_usec = h->ts.tv_usec;
	if(isFragment) NPROBE_FD_SET(FLAG_FRAGMENTED_PACKET_SRC2DST, &(bkt->flags));
	if(proto == IPPROTO_TCP)
	  updateTcpFlags(bkt, 0, &h->ts, flags, fingerprint, tos);
	else if((proto == IPPROTO_UDP) || (proto == IPPROTO_ICMP))
	  updateApplLatency(proto, bkt, 0, &h->ts, icmpType, icmpCode);

	setPayload(bkt, h, payload, payloadLen, 0);
	bkt->src2dstTcpFlags |= flags; /* Do not move this line before updateTcpFlags(...) */
      } else {
	bkt->bytesRcvd += len, bkt->pktRcvd += numPkts;
	if((bkt->firstSeenRcvd.tv_sec == 0) && (bkt->firstSeenRcvd.tv_usec == 0))
	  bkt->firstSeenRcvd.tv_sec = h->ts.tv_sec, bkt->firstSeenRcvd.tv_usec = h->ts.tv_usec;
	bkt->lastSeenRcvd.tv_sec = h->ts.tv_sec, bkt->lastSeenRcvd.tv_usec = h->ts.tv_usec;
	if(isFragment) NPROBE_FD_SET(FLAG_FRAGMENTED_PACKET_DST2SRC, &(bkt->flags));
	if(proto == IPPROTO_TCP)
	  updateTcpFlags(bkt, 1, &h->ts, flags, fingerprint, tos);
	else if((proto == IPPROTO_UDP) || (proto == IPPROTO_ICMP))
	  updateApplLatency(proto, bkt, 1, &h->ts, icmpType, icmpCode);

	setPayload(bkt, h, payload, payloadLen, 1);
	bkt->dst2srcTcpFlags |= flags; /* Do not move this line before updateTcpFlags(...) */
      }

      pluginCallback(PACKET_CALLBACK, bkt,
		     proto,  isFragment, numPkts,  tos,
		     vlanId, ehdr, &src,  sport,
		     &dst,  dport, len,
		     flags,  icmpType, numMplsLabels,
		     mplsLabels, fingerprint,
		     h, p, payload, payloadLen);

      pthread_mutex_unlock(&hashMutex[mutexIdx]);
      return;
    } else {
      /* Bucket not found yet */
      n++;
      bkt = bkt->next;
    }
  } /* while */

  if(n > maxBucketSearch) {
    maxBucketSearch = n;
    /* traceEvent(TRACE_INFO, "maxBucketSearch=%d\n", maxBucketSearch); */
  }

  pthread_mutex_unlock(&hashMutex[mutexIdx]);

#ifdef DEBUG_EXPORT
  printf("Adding new bucket\n");
#endif

  pthread_mutex_lock(&purgedBucketsMutex);
  if(purgedBuckets != NULL) {
    bkt = getListHead(&purgedBuckets);
    purgedBucketsLen--;
  } else {
    if(bucketsAllocated >= maxNumActiveFlows) {
      static u_char msgSent = 0;

      if(!msgSent) {
	traceEvent(TRACE_WARNING, "WARNING: too many (%u) active flows [limit=%u] (see -M)",
		   bucketsAllocated, maxNumActiveFlows);
	msgSent = 1;
      }
      droppedPktsTooManyFlows++;
      pthread_mutex_unlock(&purgedBucketsMutex);
      return;
    }

    bkt = (HashBucket*)malloc(sizeof(HashBucket));
    if(bkt == NULL) {
      	traceEvent(TRACE_ERROR, "NULL bkt (not enough memory?)\n");
      pthread_mutex_unlock(&purgedBucketsMutex);
      return;
    }
    bucketsAllocated++;
  }
  pthread_mutex_unlock(&purgedBucketsMutex);

  memset(bkt, 0, sizeof(HashBucket)); /* Reset bucket */
#ifdef ENABLE_MAGIC
  bkt->magic = 67;
#endif
  memcpy(&bkt->src, &src, sizeof(IpAddress));
  memcpy(&bkt->dst, &dst, sizeof(IpAddress));
  bkt->proto = proto, bkt->vlanId = vlanId;
  bkt->sport = sport, bkt->dport = dport;
  memcpy(bkt->srcMacAddress, (char *)ESRC(ehdr), 6);
  memcpy(bkt->dstMacAddress, (char *)EDST(ehdr), 6);
  bkt->firstSeenSent.tv_sec = bkt->lastSeenSent.tv_sec = h->ts.tv_sec,
    bkt->firstSeenSent.tv_usec = bkt->lastSeenSent.tv_usec = h->ts.tv_usec;
  bkt->firstSeenRcvd.tv_sec = bkt->lastSeenRcvd.tv_sec = 0,
    bkt->firstSeenRcvd.tv_usec = bkt->lastSeenRcvd.tv_usec = 0;
  bkt->bytesSent += len, bkt->pktSent += numPkts;
  if(isFragment) NPROBE_FD_SET(FLAG_FRAGMENTED_PACKET_SRC2DST, &(bkt->flags));
  if(proto == IPPROTO_TCP)
    updateTcpFlags(bkt, 0, &h->ts, flags, fingerprint, tos);
  else if((proto == IPPROTO_UDP) || (proto == IPPROTO_ICMP))
    updateApplLatency(proto, bkt, 0, &h->ts, icmpType, icmpCode);

  setPayload(bkt, h, payload, payloadLen, 0);
  bkt->src2dstTcpFlags |= flags;

  if(numMplsLabels > 0) {
    bkt->mplsInfo = malloc(sizeof(struct mpls_labels));
    bkt->mplsInfo->numMplsLabels = numMplsLabels;
    memcpy(bkt->mplsInfo->mplsLabels, mplsLabels,
	   MAX_NUM_MPLS_LABELS*MPLS_LABEL_LEN);
  }

  pluginCallback(CREATE_FLOW_CALLBACK, bkt,
		 proto,  isFragment, numPkts,  tos,
		 vlanId, ehdr, &src,  sport,
		 &dst,  dport, len,
		 flags,  icmpType, numMplsLabels,
		 mplsLabels, fingerprint,
		 h, p, payload, payloadLen);

  /* Put the bucket on top of the list */
  pthread_mutex_lock(&hashMutex[mutexIdx]);
  addToList(bkt, &theHash[idx]);
  pthread_mutex_unlock(&hashMutex[mutexIdx]);

#ifdef DEBUG_EXPORT
  printf("Bucket added (tot=%d)\n", bucketsAdded);
#endif

if(traceMode == 2) {
    char buf[256], buf1[256];

    traceEvent(TRACE_INFO, "New Flow: [%s] %s:%d -> %s:%d\n",
	       proto2name(proto), _intoa(src, buf, sizeof(buf)), sport,
	       _intoa(dst, buf1, sizeof(buf1)), dport);
   }
}

/* ****************************************************** */

void printICMPflags(u_int32_t flags, char *icmpBuf, int icmpBufLen) {
  snprintf(icmpBuf, icmpBufLen, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
	   NPROBE_FD_ISSET(NPROBE_ICMP_ECHOREPLY, &flags)     ? "[ECHO REPLY]" : "",
	   NPROBE_FD_ISSET(NPROBE_ICMP_UNREACH, &flags)       ? "[UNREACH]": "",
	   NPROBE_FD_ISSET(NPROBE_ICMP_SOURCEQUENCH, &flags)  ? "[SOURCE_QUENCH]": "",
	   NPROBE_FD_ISSET(NPROBE_ICMP_REDIRECT, &flags)      ? "[REDIRECT]": "",
	   NPROBE_FD_ISSET(NPROBE_ICMP_ECHO, &flags)          ? "[ECHO]": "",
	   NPROBE_FD_ISSET(NPROBE_ICMP_ROUTERADVERT, &flags)  ? "[ROUTERADVERT]": "",
	   NPROBE_FD_ISSET(NPROBE_ICMP_ROUTERSOLICIT, &flags) ? "[ROUTERSOLICIT]": "",
	   NPROBE_FD_ISSET(NPROBE_ICMP_TIMXCEED, &flags)      ? "[TIMXCEED]": "",
	   NPROBE_FD_ISSET(NPROBE_ICMP_PARAMPROB, &flags)     ? "[PARAMPROB]": "",
	   NPROBE_FD_ISSET(NPROBE_ICMP_TSTAMP, &flags)        ? "[TIMESTAMP]": "",
	   NPROBE_FD_ISSET(NPROBE_ICMP_TSTAMPREPLY, &flags)   ? "[TIMESTAMP REPLY]": "",
	   NPROBE_FD_ISSET(NPROBE_ICMP_IREQ, &flags)          ? "[INFO REQ]": "",
	   NPROBE_FD_ISSET(NPROBE_ICMP_IREQREPLY, &flags)     ? "[INFO REPLY]": "",
	   NPROBE_FD_ISSET(NPROBE_ICMP_MASKREQ , &flags)      ? "[MASK REQ]": "",
	   NPROBE_FD_ISSET(NPROBE_ICMP_MASKREPLY, &flags)     ? "[MASK REPLY]": "");
}

/* ****************************************************** */

void printFlow(HashBucket *theFlow, int direction) {
  char buf[256] = {0}, buf1[256] = {0}, latBuf[32] = {0}, *fragmented = "";
  char icmpBuf[128] = {0}, applLatBuf[32] = {0}, jitterStr[64] = {0};
  int len, theLen;
  u_char *thePayload;

  if(((direction == 0) && fragmentedPacketSrc2Dst(theFlow))
     || ((direction == 1) && fragmentedPacketDst2Src(theFlow)))
    fragmented = " [FRAGMENT]";

  if(nwLatencyComputed(theFlow)
     && ((theFlow->nwLatency.tv_sec > 0) || (theFlow->nwLatency.tv_usec > 0))) {
    snprintf(latBuf, sizeof(latBuf), " [N: %.2f ms]",
	     (float)(theFlow->nwLatency.tv_sec*1000+(float)theFlow->nwLatency.tv_usec/1000));
  }

  if(applLatencyComputed(theFlow)) {
    if((direction == 0) && (theFlow->src2dstApplLatency.tv_sec || theFlow->src2dstApplLatency.tv_usec))
      snprintf(applLatBuf, sizeof(applLatBuf), " [A: %.2f ms]",
	       (float)(theFlow->src2dstApplLatency.tv_sec*1000
		       +(float)theFlow->src2dstApplLatency.tv_usec/1000));
    else if((direction == 1) && (theFlow->dst2srcApplLatency.tv_sec || theFlow->dst2srcApplLatency.tv_usec))
      snprintf(applLatBuf, sizeof(applLatBuf), " [A: %.2f ms]",
	       (float)(theFlow->dst2srcApplLatency.tv_sec*1000
		       +(float)theFlow->dst2srcApplLatency.tv_usec/1000));
  }

  if(theFlow->proto == IPPROTO_ICMP) {
    if(direction == 0)
      printICMPflags(theFlow->src2dstIcmpFlags, icmpBuf, sizeof(icmpBuf));
    else
      printICMPflags(theFlow->dst2srcIcmpFlags, icmpBuf, sizeof(icmpBuf));
  }

  if(direction == 0) {
    theLen     = theFlow->src2dstPayloadLen;
    thePayload = theFlow->src2dstPayload;
  } else {
    theLen     = theFlow->dst2srcPayloadLen;
    thePayload = theFlow->dst2srcPayload;
  }

  if(theLen >= maxPayloadLen) len = maxPayloadLen; else len = theLen;

  if(direction == 0) {
    traceEvent(TRACE_INFO, "Emitting Flow: [%s] %s:%d -> %s:%d [%d/%d pkt][%d/%d bytes]%s%s%s%s%s\n",
	       proto2name(theFlow->proto), _intoa(theFlow->src, buf, sizeof(buf)), theFlow->sport,
	       _intoa(theFlow->dst, buf1, sizeof(buf1)), theFlow->dport,
	       (int)theFlow->pktSent, (int)theFlow->pktRcvd,
	       (int)theFlow->bytesSent, (int)theFlow->bytesRcvd,
	       latBuf, applLatBuf, jitterStr, icmpBuf, fragmented);
    if(theFlow->src2dstFingerprint[0] != '\0')
      traceEvent(TRACE_INFO, "Fingeprint: '%s'", theFlow->src2dstFingerprint);
  } else {
    traceEvent(TRACE_INFO, "Emitting Flow: [%s] %s:%d -> %s:%d [%d pkt/%d bytes]%s%s%s%s%s\n",
	   proto2name(theFlow->proto), _intoa(theFlow->dst, buf, sizeof(buf)), theFlow->dport,
	   _intoa(theFlow->src, buf1, sizeof(buf1)), theFlow->sport,
	   (int)theFlow->pktRcvd, (int)theFlow->bytesRcvd, latBuf, applLatBuf, jitterStr, icmpBuf, fragmented);
    if(theFlow->dst2srcFingerprint[0] != '\0')
      traceEvent(TRACE_INFO, "Fingeprint: '%s'", theFlow->dst2srcFingerprint);
  }
}

/* ****************************************************** */

int isFlowExpired(HashBucket *myBucket, time_t theTime) {
  if(myBucket->bucket_expired /* Forced expire */
     || ((theTime-myBucket->lastSeenSent.tv_sec)     >= idleTimeout)         /* flow expired: data not sent for a while */
     || ((theTime-myBucket->firstSeenSent.tv_sec) >= lifetimeTimeout)        /* flow expired: flow active but too old   */
     || ((myBucket->pktRcvd > 0)
	 && (((theTime-myBucket->lastSeenRcvd.tv_sec) >= idleTimeout)        /* flow expired: data not sent for a while */
	 || ((theTime-myBucket->firstSeenRcvd.tv_sec) >= lifetimeTimeout)))  /* flow expired: flow active but too old   */
     ) {
    return(1);
  } else {
    /* if(hashDebug) printBucket(myBucket); */
    return(0);
  }
}

/* ****************************************************** */

void printBucket(HashBucket *myBucket) {
  char str[32], str1[32];
  int a = time(NULL)-myBucket->firstSeenSent.tv_sec;
  int b = time(NULL)-myBucket->lastSeenSent.tv_sec;
  int c = myBucket->bytesRcvd ? time(NULL)-myBucket->firstSeenRcvd.tv_sec : 0;
  int d = myBucket->bytesRcvd ? time(NULL)-myBucket->lastSeenRcvd.tv_sec : 0;

#ifdef DEBUG
  if((a > 30) || (b>30) || (c>30) || (d>30))
#endif
    {
      printf("[%4s] %s:%d [%lu pkts] <-> %s:%d [%lu pkts] [FsSent=%d][LsSent=%d][FsRcvd=%d][LsRcvd=%d]\n",
	     proto2name(myBucket->proto),
	     _intoa(myBucket->src, str, sizeof(str)), myBucket->sport, myBucket->pktSent,
	     _intoa(myBucket->dst, str1, sizeof(str1)), myBucket->dport, myBucket->pktRcvd,
	     a, b, c, d);
    }
}

/* ******************************************************** */

void walkHash(int flushHash) {
  static u_int walkIndex = 0, entry_num_bkts;
  u_int mutexIdx = walkIndex % MAX_HASH_MUTEXES;
  HashBucket *myPrevBucket, *myBucket, *myNextBucket;
  time_t now = time(NULL);


#ifdef DEBUG_EXPORT
  printf("begin walkHash(%d)\n", walkIndex);
#endif

  pthread_mutex_lock(&hashMutex[mutexIdx]);
  myPrevBucket = NULL, myBucket = theHash[walkIndex];
  entry_num_bkts = 0;

  while(myBucket != NULL) {
#ifdef ENABLE_MAGIC
    if(myBucket->magic != 67) {
      printf("Error (2): magic error detected (magic=%d)\n", myBucket->magic);
    }
#endif

    if(shutdownInProgress) {
      pthread_mutex_unlock(&hashMutex[mutexIdx]);
      /* return; */
    }

    if(flushHash || isFlowExpired(myBucket, now)) {
#ifdef DEBUG_EXPORT
      printf("Found flow to emit (expired)(idx=%d)\n",walkIndex);
#endif

      myNextBucket = myBucket->next;
      queueBucketToExport(myBucket);

      if(myPrevBucket != NULL)
	myPrevBucket->next = myNextBucket;
      else
	theHash[walkIndex] = myNextBucket;

      myBucket = myNextBucket;
    } else {
      /* Move to the next bucket */
      myPrevBucket = myBucket;
      myBucket = myBucket->next;
    }
  } /* while */

  pthread_mutex_unlock(&hashMutex[mutexIdx]);

  walkIndex = (walkIndex + 1) % hashSize;

#ifdef DEBUG_EXPORT
  printf("end walkHash(%d)\n", walkIndex);
#endif
}

/* ****************************************************** */

void exportBucket(HashBucket *myBucket, u_char free_memory) {
  int rc = 0;
  static char tmpBuf[512]; /* same size as newPath */

  if(dirPath != NULL) {
    time_t theTime = time(NULL);
    static time_t lastTheTime = 0;

    theTime -= (theTime % 60);

    if(lastTheTime != theTime) {
      if(flowFd != NULL) {
	char newPath[512]; /* same size as tmpBuf */
	int len = strlen(tmpBuf)-strlen(TEMP_PREFIX);

	fclose(flowFd);
	strncpy(newPath, tmpBuf, len); newPath[len] = '\0';
	rename(tmpBuf, newPath);
	flowFd = NULL;
      }

      lastTheTime = theTime;
    }

    if(flowFd == NULL) {
      char tmpTime[64];

      snprintf(tmpTime, sizeof(tmpTime), "%lu.flow", (unsigned long)theTime);

      snprintf(tmpBuf, sizeof(tmpBuf), "%s%c%s%s", dirPath,
#ifdef WIN32
	       '\\'
#else
	       '/'
#endif
	       ,tmpTime, TEMP_PREFIX);

      if((flowFd = fopen(tmpBuf, "w+b")) == NULL) {
	traceEvent(TRACE_WARNING, "WARNING: Unable to create file '%s' [errno=%d]\n", tmpBuf, errno);
      }
    }
  }

  if((myBucket->proto != TCP_PROTOCOL) || (myBucket->bytesSent >= minFlowSize)) {
    rc = exportBucketToNetflow(myBucket, 0 /* src -> dst */, free_memory);

    if(rc > 0)
      totFlows++;
  }

  if(free_memory && (myBucket->src2dstPayload != NULL)) {
    free(myBucket->src2dstPayload);
    myBucket->src2dstPayload = NULL;
  }

  /* *********************** */

  if(myBucket->bytesRcvd > 0) {
    if(netFlowVersion == 5) {
      /*
	v9 flows do not need to be exported twice, once per direction
	 as they are bi-directional
      */
      if((myBucket->proto != TCP_PROTOCOL) || (myBucket->bytesRcvd >= minFlowSize)) {
	rc = exportBucketToNetflow(myBucket, 1 /* dst -> src */, free_memory);

	if(rc > 0)
	  totFlows++;
      }
    }

    if(free_memory && (myBucket->dst2srcPayload != NULL)) {
      free(myBucket->dst2srcPayload);
      myBucket->dst2srcPayload = NULL;
    }
  }

  if(free_memory && (myBucket->mplsInfo != NULL)) {
    free(myBucket->mplsInfo);
    myBucket->mplsInfo = NULL;
  }

  if(free_memory)
    pluginCallback(DELETE_FLOW_CALLBACK, myBucket,
		   0, 0,
		   0, 0,
		   0, NULL,
		   NULL, 0,
		   NULL, 0,
		   0,
		   0, 0, 0, NULL, NULL,
		   NULL, NULL, NULL, 0);
}

