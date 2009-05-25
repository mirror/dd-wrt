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


/* ************************************************************************

History:

1.0    [06/02]        Initial release
1.3    [07/02]        First public release
2.0    [01/03]        Full rewrite (see manual)
2.0.1  [02/03]        Added -P
2.0.2  [02/03]        Added -C parameter to -P for storing flows in compressed format
2.0.3  [02/03]        Added -E for specifying the NetFlow engine
2.2.90 [06/03]        Added nFlow-rewised/NetFlowv9
2.2.91 [09/03]        Major code rewrite after debugging at University of Texas
2.2.94 [09/03]        Improved IPv6 support
3.0    [01/04]        Stable release
3.2    [28/05]        Removed nFlow support

************************************************************************ */

#include "nprobe.h"

#define MAX_NUM_COLLECTORS          8
#define MAX_NUM_OPTIONS            48

#define DISPLAY_TIME               30

#define BLANK_SPACES               "                     "
#define TCP_PROTOCOL               0x06

/* #define HASH_DEBUG */

/* #define TIME_PROTECTION */

#ifdef linux
#define CHECKSUM
#endif

#ifdef CHECKSUM
#include "checksum.c"
static char wrongChecksum = 0;
#endif

#define MAX_SAMPLE_RATE    ((u_short)-1)

#ifdef WIN32
static char* printAvailableInterfaces(int index);
#endif

/* *************************************** */

#define DEFAULT_TEMPLATE_ID                            257

/*
#define OPTION_TEMPLATE "%SYSTEM_ID %SAMPLING_INTERVAL %SAMPLING_ALGORITHM %TOTAL_BYTES_EXP %TOTAL_EXP_PKTS_SENT %TOTAL_FLOWS_EXP %FLOW_ACTIVE_TIMEOUT %FLOW_INACTIVE_TIMEOUT"
*/

#define OPTION_TEMPLATE "%SYSTEM_ID %TOTAL_FLOWS_EXP %TOTAL_EXP_PKTS_SENT"

#define DEFAULT_OPTION_TEMPLATE_ID (DEFAULT_TEMPLATE_ID+1)
#define TEMPLATE_PACKETS_DELTA 10

/* *********** Globals ******************* */

u_int sampleRate;
u_int _idx = 0, hashDebug = 0, minFlowSize;
u_int maxNumActiveFlows = (u_int)-1, droppedPktsTooManyFlows = 0;
u_int bucketsAllocated=0;
u_int numExports, totFlows, idTemplate = DEFAULT_TEMPLATE_ID;
u_char *textFormat = NULL;
u_long numAdds=0;
struct timeval initialSniffTime, lastExportTime;
u_short flowExportDelay, scanCycle=30 /* sec */;
#ifdef USE_SYSLOG
char nprobeId[255+1];
#endif
char *pcapFile, *flowLockFile = NULL;
u_char ignoreTcpUdpSrcPort, ignoreTcpUdpDstPort;
u_char ignoreIpSrcAddress, ignoreIpDstAddress;
u_char ignoreTos, reflectorMode;


pcap_t *pcapPtr;
int datalink;
char *tmpDev = NULL, *netFilter = NULL;
char *addr=NULL, *port=NULL, *lcdDevice;
char *bindAddr = NULL, *bindPort = NULL;
u_int16_t inputInterfaceIndex = 0;
char *dirPath = NULL;
u_char compressFlows = (u_char)-1, useNetFlow, computeFingerprint;
FILE *flowFd = NULL;
u_int64_t totExports = 0;
char *stringTemplate, shutdownInProgress;
HashBucket *exportQueue;

/* Export Options */
u_char netFlowVersion;
short minNumFlowsPerPacket;
NetFlow5Record theV5Flow;
V9FlowHeader theV9Header;
int numFlows;
unsigned char *buffer = NULL;
u_short templatePacketsDelta = TEMPLATE_PACKETS_DELTA;
IpV4Fragment *fragmentsList = NULL;
u_int32_t bufferLen, purgedBucketsLen=0;
u_int32_t exportBucketsLen=0, fragmentListLen=0;
u_int32_t totBytesExp, totExpPktSent, totFlowExp;
struct sockaddr_in sockIn;
u_short packetsBeforeSendingTemplates = 0;

/* Threads */
pthread_mutex_t exportMutex, purgedBucketsMutex, hashMutex[MAX_HASH_MUTEXES];
ConditionalVariable  exportQueueCondvar;
pthread_t dequeueThread, walkHashThread;

/* V9 Templates */
#define TEMPLATE_LIST_LEN   32
V9TemplateId *v9TemplateList[TEMPLATE_LIST_LEN];
char templateBuffer[NETFLOW_MAX_BUFFER_LEN];
int templateBufBegin = 0, templateBufMax = NETFLOW_MAX_BUFFER_LEN;
int  numTemplateFieldElements;
 /* approximate # of flows that the template takes up */
u_short templateFlowSize;

/* V9 Options */
V9TemplateId *v9OptionTemplateList[TEMPLATE_LIST_LEN];
char optionTemplateBuffer[NETFLOW_MAX_BUFFER_LEN];
int optionTemplateBufBegin = 0, optionTemplateBufMax = NETFLOW_MAX_BUFFER_LEN;
int numOptionTemplateFieldElements;
/* approximate # of flows that the template takes up */
u_short optionTemplateFlowSize;

/* Collectors addresses */
u_char useIpV6;
CollectorAddress netFlowDest[MAX_NUM_COLLECTORS];
u_int8_t numCollectors;

/* Stats */
Counter totalPkts, totalBytes;
Counter totalTCPPkts, totalTCPBytes;
Counter totalUDPPkts, totalUDPBytes;
Counter totalICMPPkts, totalICMPBytes;

time_t  lastSample;
Counter currentPkts, currentBytes;
Counter currentTCPPkts, currentTCPBytes;
Counter currentUDPPkts, currentUDPBytes;
Counter currentICMPPkts, currentICMPBytes;

#ifdef CHECKSUM
char *checksum;
#endif

/* ****************************************************** */

/* Forward */
static int  exportBucketToNetflow(HashBucket *myBucket, int direction);
static int  exportBucketToNetflowV5(HashBucket *myBucket, int direction);
static int  exportBucketToNetflowV9(HashBucket *myBucket, int direction);
static void checkExportQueuedFlows(int forceExport);
static void printStats();
static void shutdown_nprobe();

/* ****************************************************** */

#ifndef WIN32
void cleanup(int signo) {
  static u_char statsPrinted = 0;

  if(!statsPrinted) {
    struct pcap_stat pcapStat;

    if(pcap_stats(pcapPtr, &pcapStat) >= 0) {
      traceEvent(TRACE_INFO, "Packet stats: "
		 "%u pkts rcvd/%u pkts dropped [%.1f%%]\n",
		 pcapStat.ps_recv, pcapStat.ps_drop,
		 pcapStat.ps_recv > 0 ?
		 (float)(pcapStat.ps_drop*100)/(float)pcapStat.ps_recv : 0);
    }

    statsPrinted = 1;
  }

  shutdown_nprobe();
  exit(0);
}
#endif

/* ****************************************************** */

#ifndef WIN32
void brokenPipe(int signo) {
#ifdef DEBUG
  traceEvent(TRACE_WARNING, "Broken pipe (socket %d closed) ?\n", currSock);
#endif
  signal(SIGPIPE, brokenPipe);
}
#endif

/* ****************************************************** */

void dummyProcesssPacket(u_char *_deviceId,
			 const struct pcap_pkthdr *h,
			 const u_char *p) {
  ;
}

/* ****************************************************** */

void processPacket(u_char *_deviceId,
		   const struct pcap_pkthdr *h,
		   const u_char *p) {
  struct ether_header ehdr;
  u_int caplen = h->caplen, length = h->len, offset;
  u_short eth_type, off=0, numPkts = 1;
  u_int8_t flags = 0, proto = 0;
  struct ip ip;
  struct ip6_hdr ipv6;
  struct tcphdr tp;
  struct udphdr up;
  struct icmp icmpPkt;
  unsigned char *payload = NULL;
  int payloadLen = 0; /* Do not set it to unsigned */
  u_char fingerprint[FINGERPRINT_LEN+1];
  IpAddress src, dst;
  u_char isFragment = 0;
#ifdef DEBUG
  traceEvent(TRACE_INFO, ".");
#endif

  if(caplen >= sizeof(struct ether_header)) {
    u_int plen, hlen=0;
    u_short sport, dport, numMplsLabels = 0;
    u_char mplsLabels[MAX_NUM_MPLS_LABELS][MPLS_LABEL_LEN];

    totalPkts++,   totalBytes   += length;
    currentPkts++, currentBytes += length;

    switch(datalink) {
    case DLT_ANY: /* Linux 'any' device */
      eth_type = DLT_ANY;
      memset(&ehdr, 0, sizeof(struct ether_header));
      break;
    default:
      memcpy(&ehdr, p, sizeof(struct ether_header));
      eth_type = ntohs(ehdr.ether_type);
      break;
    }

    if((eth_type == ETHERTYPE_IP)
       || (eth_type == ETHERTYPE_IPV6)
       || (eth_type == ETHERTYPE_VLAN) /* Courtesy of  Mikael Cam <mca@mgn.net> - 2002/08/28 */
       || (eth_type == ETHERTYPE_MPLS)
       || (eth_type == DLT_NULL)
       || (eth_type == DLT_ANY)
       || (eth_type == 16385 /* MacOSX loopback */)
       || (eth_type == 16390 /* MacOSX loopback */)
       ) {
      u_int ehshift = 0;
      u_short vlanId = 0;
      u_int estimatedLen=0;

      if(eth_type == ETHERTYPE_MPLS) {
	char bos; /* bottom_of_stack */

	memset(mplsLabels, 0, sizeof(mplsLabels));
	ehshift = sizeof(struct ether_header);
	bos = 0;
	while(bos == 0) {	  
	  memcpy(&mplsLabels[numMplsLabels], p+ehshift, MPLS_LABEL_LEN);

	  bos = (mplsLabels[numMplsLabels][2] & 0x1), ehshift += 4, numMplsLabels++;
	  if((ehshift > caplen) || (numMplsLabels >= MAX_NUM_MPLS_LABELS))
	    return; /* bad packet */	  
	}
	eth_type = ETHERTYPE_IP;
      } else if((eth_type == ETHERTYPE_IP) || (eth_type == ETHERTYPE_IPV6))
	ehshift = sizeof(struct ether_header);
      else if(eth_type == ETHERTYPE_VLAN) {
	Ether80211q qType;

	ehshift = sizeof(struct ether_vlan_header);
	memcpy(&qType, p+sizeof(struct ether_header), sizeof(Ether80211q));
	vlanId = ntohs(qType.vlanId) & 0xFFF;
	eth_type = ntohs(qType.protoType);
	/* printf("VlanId: %d\n", vlanId); <<<== NOT USED YET */
      } else if(eth_type == DLT_ANY) {
	ehshift = sizeof(AnyHeader);
	eth_type = ntohs(((AnyHeader*)p)->protoType);
      } else
	ehshift = NULL_HDRLEN;

      if(eth_type == ETHERTYPE_IP) {
	memcpy(&ip, p+ehshift, sizeof(struct ip));
	if(ip.ip_v != 4) return; /* IP v4 only */
	estimatedLen = ehshift+htons(ip.ip_len);
	hlen = (u_int)ip.ip_hl * 4;

	src.ipVersion = 4, dst.ipVersion = 4;
	if(ignoreIpSrcAddress)
	  src.ipType.ipv4 = 0; /* 0.0.0.0 */
	else
	  src.ipType.ipv4 = ntohl(ip.ip_src.s_addr);

	if(ignoreIpDstAddress)
	  dst.ipType.ipv4 = 0; /* 0.0.0.0 */
	else 
	  dst.ipType.ipv4 = ntohl(ip.ip_dst.s_addr);

	proto = ip.ip_p;
	isFragment = (ntohs(ip.ip_off) & 0x3fff) ? 1 : 0;

	off = ntohs(ip.ip_off);
      } else if(eth_type == ETHERTYPE_IPV6) {
	memcpy(&ipv6, p+ehshift, sizeof(struct ip6_hdr));
	if(((ipv6.ip6_vfc >> 4) & 0x0f) != 6) return; /* IP v6 only */
	estimatedLen = sizeof(struct ip6_hdr)+ehshift+htons(ipv6.ip6_plen);
	hlen = sizeof(struct ip6_hdr);

	src.ipVersion = 6, dst.ipVersion = 6;

	if(ignoreIpSrcAddress) 
	  memset(&src.ipType.ipv6, 0, sizeof(struct in6_addr));
	else 
	  memcpy(&src.ipType.ipv6, &ipv6.ip6_src, sizeof(struct in6_addr));
	
	if(ignoreIpDstAddress) 
	  memset(&dst.ipType.ipv6, 0, sizeof(struct in6_addr));
	 else 
	  memcpy(&dst.ipType.ipv6, &ipv6.ip6_dst, sizeof(struct in6_addr));

	proto = ipv6.ip6_nxt; /* next header (protocol) */
      } else
	return; /* Anything else that's not IPv4/v6 */

      plen = length-ehshift;
      if(caplen > estimatedLen) caplen = estimatedLen;
      sport = dport = 0; /* default */
      offset = ehshift+hlen;

      switch(proto) {
      case IPPROTO_TCP:
	if(plen < (hlen+sizeof(struct tcphdr))) return; /* packet too short */
	memcpy(&tp, p+offset, sizeof(struct tcphdr));
	if(!ignoreTcpUdpSrcPort) sport = ntohs(tp.th_sport);
	if(!ignoreTcpUdpDstPort) dport = ntohs(tp.th_dport);
	flags = tp.th_flags;
	payloadLen = caplen - offset - (tp.th_off * 4);
	if(payloadLen > 0)
	  payload = (unsigned char*)(p+offset+(tp.th_off * 4));
	else {
	  payloadLen = 0;
	  payload    = NULL;
	}

	if(computeFingerprint && (eth_type == ETHERTYPE_IP) /* no IPv6 */) {
	  int WIN=0, MSS=-1, WS=-1, S=0, N=0, D=0, T=0;
	  int ttl;
	  char WSS[3], _MSS[5];
	  struct tcphdr *tcp = (struct tcphdr *)(p+offset);
	  u_char *tcp_opt = (u_char *)(tcp + 1);
	  u_char *tcp_data = (u_char *)((int)tcp + tp.th_off * 4);
	  int tcpUdpLen = ntohs(ip.ip_len) - hlen;

	  if(tp.th_flags & TH_SYN) {  /* only SYN or SYN-2ACK packets */
	    if(tcpUdpLen > 0) {
	      /* don't fragment bit is set */
	      if(ntohs(ip.ip_off) & IP_DF) D = 1;

	      WIN = ntohs(tp.th_win);  /* TCP window size */

	      if(tcp_data != tcp_opt) { 
		/* there are some tcp_option to be parsed */
		u_char *opt_ptr = tcp_opt;

		while(opt_ptr < tcp_data) {
		  switch(*opt_ptr) {
		  case TCPOPT_EOL:        /* end option: exit */
		    opt_ptr = tcp_data;
		    break;
		  case TCPOPT_NOP:
		    N = 1;
		    opt_ptr++;
		    break;
		  case TCPOPT_SACKOK:
		    S = 1;
		    opt_ptr += 2;
		    break;
		  case TCPOPT_MAXSEG:
		    opt_ptr += 2;
		    MSS = ntohs(ptohs(opt_ptr));
		    opt_ptr += 2;
		    break;
		  case TCPOPT_WSCALE:
		    opt_ptr += 2;
		    WS = *opt_ptr;
		    opt_ptr++;
		    break;
		  case TCPOPT_TIMESTAMP:
		    T = 1;
		    opt_ptr++;
		    opt_ptr += (*opt_ptr - 1);
		    break;
		  default:
		    opt_ptr++;
		    if(*opt_ptr > 0) opt_ptr += (*opt_ptr - 1);
		    break;
		  }
		}
	      }

	      if(WS == -1) sprintf(WSS, "WS");
	      else snprintf(WSS, sizeof(WSS), "%02d", WS);

	      if(MSS == -1) sprintf(_MSS, "_MSS");
	      else snprintf(_MSS, sizeof(_MSS), "%04X", MSS);

	      snprintf(fingerprint, sizeof(fingerprint),
		       "%04X%s%02X%s%d%d%d%d%c%02X",
		       WIN, _MSS, ttl = ttlPredictor(ip.ip_ttl),
		       WSS , S, N, D, T,
		       (tp.th_flags & TH_ACK) ? 'A' : 'S', tcpUdpLen);
	    }
	  }
	}
	break;

      case IPPROTO_UDP:
	if(plen < (hlen+sizeof(struct udphdr))) return; /* packet too short */
	memcpy(&up, p+offset, sizeof(struct udphdr));
	if(!ignoreTcpUdpSrcPort) sport = ntohs(up.uh_sport);
	if(!ignoreTcpUdpDstPort) dport = ntohs(up.uh_dport);
	payloadLen = caplen - offset;
	if(payloadLen > 0)
	  payload = (unsigned char*)(p+offset+sizeof(struct udphdr));
	else {
	  payloadLen = 0;
	  payload    = NULL;
	}
	break;
      case IPPROTO_ICMP:
	if(plen < (hlen+sizeof(struct icmp))) return; /* packet too short */
	memcpy(&icmpPkt, p+offset, sizeof(struct icmp));
	payloadLen = caplen - offset;
	if(!(ignoreTcpUdpSrcPort || ignoreTcpUdpDstPort)) 
	  sport = 0, dport = (icmpPkt.icmp_type << 8) + icmpPkt.icmp_code;
	if(payloadLen > 0)
	  payload = (unsigned char*)(p+offset+sizeof(struct icmp));
	else {
	  payloadLen = 0;
	  payload    = NULL;
	}
	break;
      }
      
      /* ************************************************ */

      /* Is this is a fragment ?
	 NOTE: IPv6 doesn't have the concept of fragments
      */
      if(isFragment) {
	u_short fragmentOffset = (off & 0x1FFF)*8,
	  fragmentId = ntohs(ip.ip_id);
	IpV4Fragment *list = fragmentsList, *prev = NULL;

	while(list != NULL) {
	  if((list->src == src.ipType.ipv4)
	     && (list->dst == dst.ipType.ipv4)
	     && (list->fragmentId == fragmentId))
	    break;
	  else {
	    if((h->ts.tv_sec-list->firstSeen) > 30 /* sec */) {
	      /* Purge expired fragment */
	      IpV4Fragment *next = list->next;

	      if(prev == NULL)
		fragmentsList = next;
	      else
		prev->next = next;

	      free(list);
	      fragmentListLen--;
	      list = next;
	    } else {
	      prev = list;
	      list = list->next;
	    }
	  }
	}

	if(list == NULL) {
	  /* Fragment not found */
	  IpV4Fragment *frag = (IpV4Fragment*)malloc(sizeof(IpV4Fragment));

	  /* We have enough memory */
	  if(frag != NULL) {
	    memset(frag, 0, sizeof(IpV4Fragment));
	    frag->next = fragmentsList;
	    fragmentsList = frag;
	    frag->src = src.ipType.ipv4, frag->dst = dst.ipType.ipv4;
	    frag->fragmentId = fragmentId;
	    frag->firstSeen = h->ts.tv_sec;
	    list = frag, prev = NULL;;
	    fragmentListLen++;
	  }
	}

	if(list != NULL) {
	  if(fragmentOffset == 0)
	    list->sport = sport, list->dport = dport;

	  list->len += plen, list->numPkts++;

	  if(!(off & IP_MF)) {
	    /* last fragment->we know the total data size */
	    IpV4Fragment *next = list->next;
	    sport = list->sport, dport = list->dport;
	    plen = list->len, numPkts = list->numPkts;

	    /* We can now free the fragment */
	    if(prev == NULL)
	      fragmentsList = next;
	    else
	      prev->next = next;

	    fragmentListLen--;
	    free(list);
	  } else {
	    /* More fragments: we'll handle the packet later */
	    return;
	  }
	}
      }

      /* ************************************************ */

      if(ignoreTcpUdpSrcPort || ignoreTcpUdpDstPort || ignoreIpSrcAddress || ignoreIpDstAddress) {
	payloadLen = 0;
	payload    = NULL;
      }

#ifdef DEBUG
      {
	char buf[256], buf1[256];

	printf("%2d) %s:%d -> %s:%d [len=%d][payloadLen=%d]\n",
	       ip.ip_p, _intoa(ip.ip_src.s_addr, buf, sizeof(buf)), sport,
	       _intoa(ip.ip_dst.s_addr, buf1, sizeof(buf1)), dport,
	       plen, payloadLen);
      }
#endif

      addPktToHash(proto, isFragment, numPkts,
		   ignoreTos ? 0 : ip.ip_tos,
		   vlanId, &ehdr, src, sport, dst, dport, plen,
		   h->ts, flags,
		   (proto == IPPROTO_ICMP) ? icmpPkt.icmp_type : 0,
		   numMplsLabels, mplsLabels,
		   computeFingerprint ? fingerprint : NULL,
		   h, p, payload, payloadLen);
    }
#ifdef DEBUG
    else {
      if(traceMode)
	traceEvent(TRACE_WARNING, "Unknown ethernet type: 0x%X (%d)",
		   eth_type, eth_type);
    }
#endif
  }
}

/* ****************************************************** */

static void exportBucket(HashBucket *myBucket) {
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

      snprintf(tmpTime, sizeof(tmpBuf), "%lu.flow", (unsigned long)theTime);

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
    rc = exportBucketToNetflow(myBucket, 0 /* src -> dst */);

    if(rc > 0)
      totFlows++;
  }

#ifdef ENABLE_PLUGINS
  pluginCallback(DELETE_FLOW_CALLBACK, myBucket, NULL, NULL, NULL, 0);
#endif

  if(myBucket->src2dstPayload != NULL) {
    free(myBucket->src2dstPayload);
    myBucket->src2dstPayload = NULL;
  }

  /* *********************** */

  if(myBucket->bytesRcvd > 0) {
    if((myBucket->proto != TCP_PROTOCOL) || (myBucket->bytesRcvd >= minFlowSize)) {
      rc = exportBucketToNetflow(myBucket, 1 /* dst -> src */);

      if(rc > 0)
	totFlows++;
    }

    if(myBucket->dst2srcPayload != NULL) {
      free(myBucket->dst2srcPayload);
      myBucket->dst2srcPayload = NULL;
    }
  }

  if(myBucket->mplsInfo != NULL) {
    free(myBucket->mplsInfo);
    myBucket->mplsInfo = NULL;
  }
}

/* ****************************************************** */

void queueBucketToExport(HashBucket *myBucket) {
  pthread_mutex_lock(&exportMutex);
  addToList(myBucket, &exportQueue);
  exportBucketsLen++;
  pthread_mutex_unlock(&exportMutex);
  signalCondvar(&exportQueueCondvar);
}

/* ****************************************************** */

void* dequeueBucketToExport(void* notUsed) {
  while(1) {
    if(exportQueue == NULL) {
      if(!shutdownInProgress)
	waitCondvar(&exportQueueCondvar);
      else
	break;
    }

    if(exportQueue != NULL) {
      HashBucket *myBucket;

      /* Remove bucket from list */
      pthread_mutex_lock(&exportMutex);
      myBucket = getListHead(&exportQueue);
      exportBucketsLen--;
      pthread_mutex_unlock(&exportMutex);

      /* Export bucket */
      exportBucket(myBucket);

      /* Recycle bucket */
      pthread_mutex_lock(&purgedBucketsMutex);
      addToList(myBucket, &purgedBuckets);
      purgedBucketsLen++;
      pthread_mutex_unlock(&purgedBucketsMutex);
    }
  }

  traceEvent(TRACE_INFO, "Export thread terminated [exportQueue=%x]\n", exportQueue);
  return(NULL);
}

/* ****************************************************** */

/*
  From the tests carried on, the very best approach
  is to have a periodic thread that scans for expired
  flows.
*/
void* hashWalker(void* notUsed) {
  u_int numSlots = 0;

  /* Wait until all the data structures have been allocated */
  while(theHash == NULL) ntop_sleep(1);

  for(;shutdownInProgress == 0;) {
    walkHash(0);
    if(++numSlots >= hashSize) {
      int activeBuckets = bucketsAllocated-(purgedBucketsLen+exportBucketsLen);
      int freeBucketsThreshold = activeBuckets*.1; /* 10% of activeBuckets */

      if(purgedBucketsLen > freeBucketsThreshold) {
	/* Too many buckets: let's free some of them */
	while((purgedBucketsLen > 0) && (freeBucketsThreshold > 0)) {
	  HashBucket *bkt;

	  /* Get the head */
	  pthread_mutex_lock(&purgedBucketsMutex);
	  bkt = getListHead(&purgedBuckets);
	  purgedBucketsLen--, bucketsAllocated--;
	  pthread_mutex_unlock(&purgedBucketsMutex);

	  /* Free the head */
	  free(bkt);
	  freeBucketsThreshold--;
	}
      }

      printStats();
      numSlots = 0;
      ntop_sleep(scanCycle);
    }
  }

  traceEvent(TRACE_INFO, "Hash walker thread terminated\n");
  return(NULL);
}

/* ****************************************************** */

void probeVersion() {
  printf("Welcome to nprobe v.%s for %s\n"
	 "Built on %s\n"
	 "Copyright 2002-04 by Luca Deri <deri@ntop.org>\n",
	 version, osName, buildDate);
}

/* ******************************************************** */

void usage() {
  char buf[16];

  probeVersion();
  printf("\n");

  printf("Usage: nprobe -n <host:port> [-i <interface>] [-t <dump timeout>]\n"
	 "              [-d <idle timeout>] [-l <send timeout>] [-s <scan cycle>]\n"
	 "              [-p <level>] [-f <filter>] [-a] [-b] [-G]\n"
	 "              "
#ifndef CHECKSUM
	 "[-P <path>] [-D <format>] "
#endif
	 "[-u <device index>] [-v] [-C <flow lock file>]"
	 "\n              "
#ifdef USE_SYSLOG
	 "[-I <probe name>] "
#endif
	 "[-w <hash size>] [-e <flow delay>]\n"
	 "              [-z <min flow size>] [-M <max num active flows>]"
#ifdef CHECKSUM
	 "\n              [-c <firmware checksum>] "
#endif
	 "[-R <payload Len>]"
	 "\n              [-x <payload policy>] [-N <key>] [-E <engine>]"
	 "\n              [-m <min # flows>] [-r <dump file>] [-q <host:port>]"
	 "\n              [-S <sample rate>] [-A <AS list>] [-g <PID file>]"
	 "\n              [-T <Flow Template>] [-U <Flow Template Id>]"
	 "\n              [-o <v9 Templ. Export Policy>]"
	 "\n\n"
	 );

  printf("-n <host:port>     | Address of the NetFlow collector(s). Multiple\n");
  printf("                   | collectors can be defined using multiple -n flags.\n");
  printf("                   | In this case flows will be sent in round robin mode.\n");
  printf("                   | or to all defined collectors if the -a flag is used.\n");
  printf("                   | Note that you can specify both IPv4 and IPv6 addresses.\n");
#ifndef WIN32
  printf("-i <interface>     | Interface name from which packets are captured\n");
#else
  printf("-i <interface>     | Index of the interface from which packets are captured\n");
#endif
  printf("-t <dump timeout>  | It specifies the maximum (seconds) flow lifetime\n"
	 "                   | [default=%d]\n", lifetimeTimeout);
  printf("-d <idle timeout>  | It specifies the maximum (seconds) flow idle lifetime\n"
         "                   | [default=%d]\n", idleTimeout);
  printf("-l <send timeout>  | It specifies how long expired flows queued in nprobe for delivery\n"
	 "                   | are emitted [default=%d]\n", sendTimeout);
  printf("-s <scan cycle>    | It specifies how often (seconds) expired flows are emitted\n"
	 "                   | [default=%d]\n", scanCycle);
  printf("-p <level>         | It specifies the flow aggregation level. Available levels:\n"
	 "                   | 0 - IP address: TCP/UDP port numbers are ignored and set to 0\n"
	 "                   | 1 - Ports: IP addresses are ignored and set to 0.0.0.0\n"
	 "                   | 2 - IP Protocol: IP addresses/ports/tos/AS are ignored\n"
	 "                   |     and set to 0.0.0.0/0/0/0\n");
  printf("-f <BPF filter>    | BPF filter used to select captured packets\n"
	 "                   | [default=no filter]\n");
  printf("-a                 | If several collectors are defined, this option gives the\n");
  printf("                   | ability to send all collectors all the flows. If the flag\n");
  printf("                   | is omitted collectors are selected in round robin.\n");
  printf("-b                 | Verbose output (used to tune nProbe parameters: debug only)\n");
#ifndef WIN32
  printf("-G                 | Start as daemon.\n");
#endif
  printf("-P <path>          | Directory where dump files will be stored\n");
  printf("-D <format>        | <format>: flows are saved using the specified text format (see below).\n"
	 "                   | b       : binary uncompressed NetFlow flow format\n"
	 "                   | c       : binary compressed (gzip) NetFlow flow format\n"
	 "                   | <format>: text format (see below)\n"
	 "                   | Example: -D b. Note: this flag has no effect without -P.\n");
  printf("-u <device index>  | Index of the input device used in the emitted flows\n");
  printf("                   | If you have multiple interfaces you can have multiple\n");
  printf("                   | nProbe instances and distinguish the emitted flows\n"
	 "                   | using this index. [default=%d]\n", inputInterfaceIndex);
  printf("-v                 | Prints the program version.\n");
  printf("-C <flow lock>     | If the flow lock file is present no flows are\n"
	 "                   | emitted. This facility is useful to implement\n"
	 "                   | high availability by means of a daemon that\n"
	 "                   | can create a lock file when this instance\n"
	 "                   | is in standby.\n");
  printf("-h                 | Prints this help.\n");
#ifdef USE_SYSLOG
  printf("-I <probe name>    | Log to syslog as <probe name> [default=stdout]\n");
#endif
  printf("-w <hash size>     | Flows hash size [default=%d]\n", hashSize);
  printf("-e <flow delay>    | Delay (in ms) between two flow exports [default=%d]\n",
	 flowExportDelay);

  if(minFlowSize == 0)
    strcpy(buf, "unlimited");
  else
    sprintf(buf, "%u", minFlowSize);
  printf("-z <min flow size> | Minimum TCP flow size (in bytes). If a TCP flow is shorter\n"
	 "                   | than the specified size the flow is not\n"
	 "                   | emitted [default=%s]\n", buf);


  printf("-M <max num flows> | Limit the number of active flows. This is useful if you want\n"
         "                   | to limit the memory/CPU allocated to nProbe in case of non\n"
	 "                   | well-behaved applications (e.g. worms, DOS) [default=%u]\n",
	 maxNumActiveFlows);
#ifdef CHECKSUM
  printf("-c <fw checksum>   | Firmware checksum value used to identify non-genuine firmware.\n");
#endif

  printf("-R <payload Len>   | Specify the max payload length [default: %d bytes]\n",
	 maxPayloadLen);
  printf("-x <payload policy>| Specify the max payload export policy. The format is:\n"
	 "                   | TCP:UDP:ICMP:OTHER where all parameters can se set to:\n"
	 "                   | 0: no payload for the selected protocol\n"
	 "                   | 1: payload for the selected protocol\n"
	 "                   | 2: payload only for TCP sessions with SYN flag [TCP only]\n"
	 "                   | Example -x 2:0:0:0 [default=%d:%d:%d:%d]\n",
	 tcpPayloadExport, udpPayloadExport, icmpPayloadExport, otherPayloadExport);

  printf("-E <engine>        | Specify the engine type and id. The format is\n"
	 "                   | engineType:engineId. [default=%d:%d]\n",
	 engineType, engineId);
  printf("-m <min # flows>   | Minimum number of flows per packet unless an expired flow\n"
	 "                   | is queued for too long (see -l) [default=%d for v5, dynamic for v9]\n",
	 V5FLOWS_PER_PAK);
  printf("-r <dump file>     | Read packets from the dump file (pcap format).\n");
  printf("-q <host:port>     | Specifies the address:port of the flow sender. This option\n"
	 "                   | is useful for hosts with multiple interfaces\n"
	 "                   | or if flows must be emitted from a static port\n");
  printf("-S <sample rate>   | Rate at which captured packets are processed.\n"
	 "                   | Default: 0 [no sampling]\n");
  printf("-A <AS list>       | File containing the list of known ASs.\n");
  printf("-g <PID file>      | Put the PID in the specified file\n");
  printf("-T <Flow Template> | Specify the NetFlowV9 template (see below).\n");
  printf("-U <Flow Temp. Id> | Specify the NetFlowV9 template identifier [default: %d]\n", idTemplate);
  printf("-o <v9 Temp. Exp>  | Specify how many NetFlowV9 pkts are exported between template exports [default: %d]\n",
	 templatePacketsDelta);

#ifdef WIN32
  (void)printAvailableInterfaces(-1);
#endif
  printf("\nNetFlowV9 format [-T]"
	 "\n----------------"
	 "\nThe following options can be used to specify the format:\n");
  printf("  %%%s\t%s\n",  "BYTES", "");
  printf("  %%%s\t%s\n",  "PKTS", "");
  printf("  %%%s\t%s\n",  "PROT", "");
  printf("  %%%s\t%s\n",  "TOS", "");
  printf("  %%%s\t%s\n",  "TCP_FLAGS", "");
  printf("  %%%s\t%s\n",  "L4_SRC_PORT", "");
  printf("  %%%s\t%s\n",  "IP_SRC_ADDR", "");
  printf("  %%%s\t%s\n",  "SRC_MASK", "");
  printf("  %%%s\t%s\n",  "INPUT_SNMP", "");
  printf("  %%%s\t%s\n",  "L4_DST_PORT", "");
  printf("  %%%s\t%s\n",  "IP_DST_ADDR", "");
  printf("  %%%s\t%s\n",  "DST_MASK", "");
  printf("  %%%s\t%s\n",  "OUTPUT_SNMP", "");
  printf("  %%%s\t%s\n",  "IP_NEXT_HOP", "");
  printf("  %%%s\t%s\n",  "SRC_AS", "");
  printf("  %%%s\t%s\n",  "DST_AS", "");
  printf("  %%%s\t%s\n",  "BGP_NEXT_HOP", "");
  printf("  %%%s\t%s\n",  "MUL_DPKTS", "");
  printf("  %%%s\t%s\n",  "MUL_DOCTETS", "");
  printf("  %%%s\t%s\n",  "LAST_SWITCHED", "");
  printf("  %%%s\t%s\n",  "FIRST_SWITCHED", "");
  printf("  %%%s\t%s\n",  "IPV6_SRC_ADDR", "");
  printf("  %%%s\t%s\n",  "IPV6_DST_ADDR", "");
  printf("  %%%s\t%s\n",  "IPV6_SRC_MASK", "");
  printf("  %%%s\t%s\n",  "IPV6_DST_MASK", "");
  printf("  %%%s\t%s\n",  "FLOW_LABEL", "");
  printf("  %%%s\t%s\n",  "ICMP_TYPE", "");
  printf("  %%%s\t%s\n",  "IGMP_TYPE", "");
  printf("  %%%s\t%s\n",  "SAMPLING_INTERVAL", "");
  printf("  %%%s\t%s\n",  "SAMPLING_ALGORITHM", "");
  printf("  %%%s\t%s\n",  "FLOW_ACTIVE_TIMEOUT", "");
  printf("  %%%s\t%s\n",  "FLOW_INACTIVE_TIMEOUT", "");
  printf("  %%%s\t%s\n",  "ENGINE_TYPE", "");
  printf("  %%%s\t%s\n",  "ENGINE_ID", "");
  printf("  %%%s\t%s\n",  "TOTAL_BYTES_EXP", "");
  printf("  %%%s\t%s\n",  "TOTAL_EXP_PKTS_SENT", "");
  printf("  %%%s\t%s\n",  "TOTAL_FLOWS_EXP", "");
  printf("  %%%s\t%s\n",  "SRC_MAC", "");
  printf("  %%%s\t%s\n",  "DST_MAC", "");
  printf("  %%%s\t%s\n",  "SRC_VLAN", "");
  printf("  %%%s\t%s\n",  "DST_VLAN", "");
  printf("  %%%s\t%s\n",  "IP_PROTOCOL_VERSION", "");
  printf("  %%%s\t%s\n",  "DIRECTION", "");
  printf("  %%%s\t%s\n",  "IPV6_NEXT_HOP", "");
  printf("  %%%s\t%s\n",  "BPG_IPV6_NEXT_HOP", "");
  printf("  %%%s\t%s\n",  "IPV6_OPTION_HEADERS", "");
  printf("  %%%s\t%s\n",  "MPLS_LABEL_1", "");
  printf("  %%%s\t%s\n",  "MPLS_LABEL_2", "");
  printf("  %%%s\t%s\n",  "MPLS_LABEL_3", "");
  printf("  %%%s\t%s\n",  "MPLS_LABEL_4", "");
  printf("  %%%s\t%s\n",  "MPLS_LABEL_5", "");
  printf("  %%%s\t%s\n",  "MPLS_LABEL_6", "");
  printf("  %%%s\t%s\n",  "MPLS_LABEL_7", "");
  printf("  %%%s\t%s\n",  "MPLS_LABEL_8", "");
  printf("  %%%s\t%s\n",  "MPLS_LABEL_9", "");
  printf("  %%%s\t%s\n",  "MPLS_LABEL_10", "");
  printf("  %%%s\t%s\n",  "FRAGMENTED", "");
  printf("  %%%s\t%s\n",  "FINGERPRINT", "");
  printf("  %%%s\t%s\n",  "NW_LATENCY_SEC", "");
  printf("  %%%s\t%s\n",  "NW_LATENCY_NSEC", "");
  printf("  %%%s\t%s\n",  "APPL_LATENCY_SEC", "");
  printf("  %%%s\t%s\n",  "APPL_LATENCY_NSEC", "");
  printf("  %%%s\t%s\n",  "PAYLOAD", "");

  printf("\nExample: nprobe -T \"%%LAST_SWITCHED %%FIRST_SWITCHED %%BYTES %%PKTS %%INPUT_SNMP %%OUTPUT_SNMP %%IP_SRC_ADDR %%IP_DST_ADDR %%PROT %%TOS %%L4_SRC_PORT %%L4_DST_PORT\"\n");

  printf("\nDump flow format [-D]"
	 "\n----------------"
	 "\nThe following options can be used to specify the format:\n");
  printf("  %%sa\tSource IP Address (IPv4/6)\n");
  printf("  %%da\tDestination IP Address  (IPv4/6)\n");
  printf("  %%in\tInput interface index\n");
  printf("  %%ou\tOutput interface index\n");
  printf("  %%dp\tPackets sent in Duration (ms between 1st & last packet in this flow)\n");
  printf("  %%do\tOctets sent in Duration (ms between 1st & last packet in this flow)\n");
  printf("  %%rp\tPackets rcvd in Duration (ms between 1st & last packet in this flow)\n");
  printf("  %%ro\tOctets rcvd in Duration (ms between 1st & last packet in this flow)\n");
  printf("  %%fr\tSysUptime at start of flow\n");
  printf("  %%la\tSysUptime at last packet of the flow\n");
  printf("  %%sP\tTCP/UDP source port number (.e.g, FTP, Telnet, etc., or equivalent)\n");
  printf("  %%dP\tTCP/UDP destination port number (.e.g, FTP, Telnet, etc., or equivalent)\n");
  printf("  %%tf\tCumulative OR of tcp flags\n");
  printf("  %%pr\tIP protocol, e.g., 6=TCP, 17=UDP, etc...\n");
  printf("  %%ds\tdst peer/origin Autonomous System\n");
  printf("  %%ss\tsource peer/origin Autonomous System\n");
  printf("  %%ts\tIP Type-of-Service\n");
  printf("  %%fm\t0=no fragmented pkts, 1=fragmented pkts\n");
  printf("  %%fp\tHost fingerprint\n");
  printf("  %%vt\tVLAN Tag    (802.1q only)\n");
  printf("  %%mh\tMPLS header (MPLS traffic only)\n");
  printf("  %%ns\tTCP NetworkLatency: Current seconds since 0000 UTC 1970\n");
  printf("  %%nn\tTCP NetworkLatency: Residual nanoseconds since 0000 UTC 1970\n");
  printf("  %%ps\tApplicationLatency: Current seconds since 0000 UTC 1970\n");
  printf("  %%pn\tApplicationLatency: Residual nanoseconds since 0000 UTC 1970\n");
  printf("  %%if\tICMP flags\n");
  printf("  %%pl\tpayloadLen\n");
  printf("  %%pa\tpayload\n");

  printf("\n");
  exit(0);
}

/* ****************************************************** */

static void printStats() {
  struct pcap_stat pcapStat;
  time_t now = time(NULL), nowDiff;
  char buf[32];

  nowDiff = now-initialSniffTime.tv_sec;

  /* Wait at least 10 seconds */
  if((nowDiff < 10) || (totalPkts == 0)) return;

  traceEvent(TRACE_INFO, "Average traffic: [%.1f pkt/sec][%s/sec]",
	     (float)totalPkts/nowDiff,
	     formatTraffic((float)(8*totalBytes)/(float)nowDiff, 1, buf));

  nowDiff = now-lastSample;
  traceEvent(TRACE_INFO, "Current traffic: [%.1f pkt/sec][%s/sec]",
	     (float)currentPkts/nowDiff,
	     formatTraffic((float)(8*currentBytes)/(float)nowDiff, 1, buf));
  lastSample = now;
  currentBytes = currentPkts = 0;

  traceEvent(TRACE_INFO, "Current flow export rate: [%.1f flows/sec]",
	     (float)totFlows/nowDiff);
  totFlows = 0;

  traceEvent(TRACE_INFO, "Buckets: [active=%d][allocated=%d][free=%d][toBeExported=%d][frags=%d]",
	     bucketsAllocated-(purgedBucketsLen+exportBucketsLen),
	     bucketsAllocated, purgedBucketsLen, exportBucketsLen,
	     fragmentListLen);

  traceEvent(TRACE_INFO, "Fragment queue: [len=%d]", fragmentListLen);

  if(traceMode) {
    traceEvent(TRACE_INFO, "Num Packets: %u (max bucket search: %d)",
	       totalPkts, maxBucketSearch);
  } else {
    static u_int lastMaxBucketSearch = 5; /* Don't bother with values < 5 */

    if(maxBucketSearch > lastMaxBucketSearch) {
      traceEvent(TRACE_INFO, "Max bucket search: %d slots (for better "
		 "performance a larger value for -w)",
		 maxBucketSearch);
      lastMaxBucketSearch = maxBucketSearch;
    }
  }

  maxBucketSearch = 0; /* reset */

  if(pcap_stats(pcapPtr, &pcapStat) >= 0) {
    if(traceMode) {
      traceEvent(TRACE_INFO, "%u pkts rcvd/%u pkts dropped",
		 pcapStat.ps_recv, pcapStat.ps_drop);
    }
  }
}

/* ****************************************************** */

int resolveIpV4Address(char *addr, int port) {
  struct hostent *hostAddr;
  struct in_addr dstAddr;

  if((hostAddr = gethostbyname(addr)) == NULL) {
    traceEvent(TRACE_ERROR, "Unable to resolve address '%s'\n", addr);
    return(-1);
  }

  memset(&netFlowDest[numCollectors], 0, sizeof(CollectorAddress));
  memcpy(&dstAddr.s_addr, hostAddr->h_addr_list[0], hostAddr->h_length);
  netFlowDest[numCollectors].sockFd = -1;
  netFlowDest[numCollectors].isV6 = 0;
  netFlowDest[numCollectors].u.v4Address.sin_addr.s_addr = dstAddr.s_addr;
  netFlowDest[numCollectors].u.v4Address.sin_family      = AF_INET;
  netFlowDest[numCollectors].u.v4Address.sin_port        = (int)htons(port);

  return(0);
}

/* ****************************************************** */

#ifndef IPV4_ONLY

int resolveIpV6Address(char *addr, int port, int *isIpV6Address) {
  int errnum;
  struct addrinfo hints, *res;

  if((useIpV6 == 0) || strstr(addr, ".")) {
    (*isIpV6Address) = 0;
    return(resolveIpV4Address(addr, port));
  }

  (*isIpV6Address) = 0;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  errnum = getaddrinfo(addr, NULL, &hints, &res);
  if(errnum != 0) {
    traceEvent(TRACE_INFO, "Unable to resolve address '%s' [error=%d]\n",
	       addr, errnum);
    return(-1);
  }

  memset(&netFlowDest[numCollectors], 0, sizeof(CollectorAddress));
  netFlowDest[numCollectors].isV6 = 1;
  netFlowDest[numCollectors].u.v6Address.sin6_family      = res->ai_family;
  netFlowDest[numCollectors].u.v6Address.sin6_flowinfo    = 0;
  netFlowDest[numCollectors].u.v6Address.sin6_port        = (int)htons(port);
  memcpy(&netFlowDest[numCollectors].u.v6Address.sin6_addr, 
	 res->ai_addr, res->ai_addrlen);
  freeaddrinfo(res);
  return(0);
}

#endif

/* ****************************************************** */

#define PROTO_UDP_URL       "udp://"
#define PROTO_TCP_URL       "tcp://"
#define PROTO_SCTP_URL      "sctp://"

int initNetFlow(char* addr, int port) {
  int sockopt = 1, rc, isIpV6Address = 0;
  char *address;
  u_char transport = TRANSPORT_UDP;

  if(numCollectors >= MAX_NUM_COLLECTORS) {
    traceEvent(TRACE_INFO,
	       "Unable to define further collector address "
	       "(max %d collectors allowed)\n", MAX_NUM_COLLECTORS);
    return(-1);
  }

  if(strncmp(addr, PROTO_UDP_URL, strlen(PROTO_UDP_URL)) == 0)
    transport = TRANSPORT_UDP, address = &addr[strlen(PROTO_UDP_URL)];
  else if(strncmp(addr, PROTO_TCP_URL, strlen(PROTO_TCP_URL)) == 0)
    transport = TRANSPORT_TCP, address = &addr[strlen(PROTO_TCP_URL)];
  else if(strncmp(addr, PROTO_SCTP_URL, strlen(PROTO_SCTP_URL)) == 0) {
#ifdef HAVE_SCTP
    transport = TRANSPORT_SCTP;
#else
    traceEvent(TRACE_ERROR, "SCTP isn't supported on your system. Using UDP.");
    transport = TRANSPORT_UDP;
#endif
    address = &addr[strlen(PROTO_SCTP_URL)];
  } else
    transport = TRANSPORT_UDP, address = addr;

#ifdef IPV4_ONLY
  rc = resolveIpV4Address(address, port);
#else
  if(useIpV6) {
    rc = resolveIpV6Address(address, port, &isIpV6Address);
    if(!isIpV6Address) useIpV6 = 0;
  } else
    rc = resolveIpV4Address(address, port);
#endif

  if(rc != 0)  return(-1);

#ifndef linux
#ifdef IPV4_ONLY
    if(transport == TRANSPORT_UDP)
      netFlowDest[numCollectors].sockFd = socket(AF_INET, SOCK_DGRAM, 0);
    else if(transport == TRANSPORT_TCP)
      netFlowDest[numCollectors].sockFd = socket(AF_INET, SOCK_STREAM, 0);
#ifdef HAVE_SCTP
    else if(transport == TRANSPORT_SCTP)
      netFlowDest[numCollectors].sockFd = socket(AF_INET,
						 SOCK_SEQPACKET, IPPROTO_SCTP);
#endif
#else /* !IPV4_ONLY */

    if(useIpV6) {
      if(transport == TRANSPORT_UDP)
	netFlowDest[numCollectors].sockFd = socket(AF_INET6, SOCK_DGRAM, 0);
      else if(transport == TRANSPORT_TCP)
	netFlowDest[numCollectors].sockFd = socket(AF_INET6, SOCK_STREAM, 0);
#ifdef HAVE_SCTP
      else if(transport == TRANSPORT_SCTP)
	netFlowDest[numCollectors].sockFd = socket(AF_INET6, SOCK_SEQPACKET,
						   IPPROTO_SCTP);
#endif
    }

    if(netFlowDest[numCollectors].sockFd == -1) {
      useIpV6 = 0; /* No IPv6 ? */

      if(transport == TRANSPORT_UDP)
	netFlowDest[numCollectors].sockFd = socket(AF_INET, SOCK_DGRAM, 0);
      else if(transport == TRANSPORT_TCP)
	netFlowDest[numCollectors].sockFd = socket(AF_INET, SOCK_STREAM, 0);
#ifdef HAVE_SCTP
      else if(transport == TRANSPORT_SCTP)
	netFlowDest[numCollectors].sockFd = socket(AF_INET, SOCK_SEQPACKET,
						   IPPROTO_SCTP);
#endif
    }
#endif
#else /* linux */
    if(sockIn.sin_addr.s_addr == 0) {
      if(transport == TRANSPORT_UDP)
	netFlowDest[numCollectors].sockFd = socket(AF_INET, SOCK_DGRAM, 0);
      else if(transport == TRANSPORT_TCP)
	netFlowDest[numCollectors].sockFd = socket(AF_INET, SOCK_STREAM, 0);
#ifdef HAVE_SCTP
      else if(transport == TRANSPORT_SCTP)
	netFlowDest[numCollectors].sockFd = socket(AF_INET, SOCK_SEQPACKET, 
						   IPPROTO_SCTP);
#endif
    } else {
      int tmp = 1;

      if(transport != TRANSPORT_UDP) {
	transport = TRANSPORT_UDP;
	traceEvent(TRACE_WARNING, 
		   "Unable to use a transport different from UDP");
	traceEvent(TRACE_WARNING, "when -q is used. Reverting to UDP.");
      }

      netFlowDest[numCollectors].sockFd = socket(PF_INET, SOCK_RAW,
						 IPPROTO_UDP);

      if(netFlowDest[numCollectors].sockFd == -1) {
	traceEvent(TRACE_ERROR, "Fatal error while creating socket (%s).",
		   strerror(errno));
	if((getuid () && geteuid ()) || setuid (0)) {
	  traceEvent(TRACE_ERROR, "You probably need superuser capabilities. "
		     "Please try again.");
	}
	exit(-1);
      }

      transport = TRANSPORT_UDP_RAW;
      /* Tell Linux that we specify the IP header */
      setsockopt(netFlowDest[numCollectors].sockFd, 0, IP_HDRINCL, 
		 &tmp, sizeof(tmp));
    }
#endif

    setsockopt(netFlowDest[numCollectors].sockFd, SOL_SOCKET, SO_REUSEADDR,
	       (char *)&sockopt, sizeof(sockopt));

  if(netFlowDest[numCollectors].sockFd == -1) {
    traceEvent(TRACE_INFO, "Fatal error while creating socket (%s).", 
	       strerror(errno));
    exit(-1);
  }

  if(transport == TRANSPORT_TCP) {
    int rc;

    traceEvent(TRACE_INFO, "Connecting to %s:%d...", addr, port);

#ifndef IPV4_ONLY
    if(netFlowDest[numCollectors].isV6)
      rc = connect(netFlowDest[numCollectors].sockFd,
		   (struct sockaddr *)&netFlowDest[numCollectors].u.v6Address,
		   sizeof(struct sockaddr_in6));
    else
#endif
      rc = connect(netFlowDest[numCollectors].sockFd,
		   (struct sockaddr *)&netFlowDest[numCollectors].u.v4Address,
		   sizeof(struct sockaddr_in));

    if(rc == -1) {
      traceEvent(TRACE_ERROR, "Connection failed with remote peer [%s]. "
		 "Leaving.\n", strerror(errno));
      close(netFlowDest[numCollectors].sockFd);
      exit(-1);
    }
  }

  netFlowDest[numCollectors].transport = transport;
  numCollectors++;

  if(strstr(address, ":"))
    traceEvent(TRACE_INFO, "Exporting flows towards [%s]:%d using %s", 
	       addr, port,
	       transport == TRANSPORT_UDP ? "UDP" : 
	       (transport == TRANSPORT_TCP ? "TCP" : "SCTP"));
  else
    traceEvent(TRACE_INFO, "Exporting flows towards %s:%d using %s",
	       addr, port,
	       transport == TRANSPORT_UDP ? "UDP" :
	       (transport == TRANSPORT_TCP ? "TCP" : "SCTP"));

  return(0);
}

/* ******************************************* */

#define MAX_LOCK_CHECK_FREQUENCY   10 /* sec */

int is_locked_send() {
  static u_char show_message = 1;
  static time_t last_check = 0;
  static int last_returned_value = 0;
  time_t now = time(NULL);

  /* Avoid checking the lock file too often */
  if((now-last_check) < MAX_LOCK_CHECK_FREQUENCY)
    return(last_returned_value);

  if(flowLockFile != NULL) {
    struct stat buf;
   
     last_check = now;
    /* The lock file exists so no flows will be sent */
    if(stat(flowLockFile, &buf) == 0) {
      if(show_message) {
	traceEvent(TRACE_WARNING, 
		   "Lock file is present: no flows will be emitted.");
	show_message = 0;
      }
      return(last_returned_value = 1);
    }
  }

  show_message = 1;   
  return(last_returned_value = 0); /* Not locked */
}

/* ******************************************* */

static int send_buffer(int s, const void *msg, size_t len, 
		       int flags, const struct sockaddr *to, socklen_t tolen) {

  if(is_locked_send())
    return(len); /* Emulate succesful send */
  else
    return(sendto(s, msg, len, flags, to, tolen));
}

/* ****************************************************** */

#ifdef linux

#define BUFFER_SIZE 1500

/*
 * Checksum routine for Internet Protocol family headers (C Version)
 *
 * Borrowed from DHCPd
 */

static u_int32_t in_cksum(unsigned char *buf, 
			  unsigned nbytes, u_int32_t sum) {
  int i;

  /* Checksum all the pairs of bytes first... */
  for (i = 0; i < (nbytes & ~1U); i += 2) {
    sum += (u_int16_t) ntohs(*((u_int16_t *)(buf + i)));
    /* Add carry. */
    if (sum > 0xFFFF)
      sum -= 0xFFFF;
  }

  /* If there's a single byte left over, checksum it, too.   Network
     byte order is big-endian, so the remaining byte is the high byte. */
  if (i < nbytes) {
#ifdef DEBUG_CHECKSUM_VERBOSE
    debug ("sum = %x", sum);
#endif
    sum += buf [i] << 8;
    /* Add carry. */
    if (sum > 0xFFFF)
      sum -= 0xFFFF;
  }

  return sum;
}

/* ******************************************* */

static u_int32_t wrapsum (u_int32_t sum) {
  sum = ~sum & 0xFFFF;
  return htons(sum);
}

/* ******************************************* */

static int send_raw_socket(int sock, const void *dataBuffer,
			 int dataBufferLen, struct sockaddr_in *dest) {
  if(is_locked_send())
    return(dataBufferLen); /* Emulate succesful send */
  else {
    static int ipHdrId = 0;
    int rc;
    char buffer[BUFFER_SIZE];
    unsigned int buffer_size = BUFFER_SIZE, headerLen;
    struct iphdr *ip_header;
    struct udphdr *udp_header;

    ip_header = (struct iphdr*) buffer;
    ip_header->ihl = 5;
    ip_header->version = 4;
    ip_header->tos = 0;
    ip_header->tot_len = htons(buffer_size);
    ip_header->id = htons(ipHdrId++);
    ip_header->ttl = 64;
    ip_header->frag_off = 0x40;
    ip_header->protocol = IPPROTO_UDP;
    ip_header->check = wrapsum(in_cksum((unsigned char *)ip_header, 
					sizeof(struct iphdr), 0));
    ip_header->daddr = dest->sin_addr.s_addr;
    ip_header->saddr = sockIn.sin_addr.s_addr;

    udp_header = (struct udphdr*) (ip_header + 1);
    udp_header->uh_sport = sockIn.sin_port;
    udp_header->uh_dport = dest->sin_port;
    udp_header->uh_ulen = htons(sizeof(struct udphdr)+dataBufferLen);
    udp_header->uh_sum  = 0; /* It must be 0 to compute the checksum */

    headerLen = sizeof(struct iphdr)+sizeof(struct udphdr);
    if(dataBufferLen > (BUFFER_SIZE-headerLen))
      dataBufferLen = BUFFER_SIZE-headerLen-1;
    memcpy(&buffer[headerLen], dataBuffer, dataBufferLen);

    buffer_size = headerLen+dataBufferLen;
    ip_header->tot_len  = htons(buffer_size);

    /*
      http://www.cs.nyu.edu/courses/fall01/G22.2262-001/class11.htm
      http://www.ietf.org/rfc/rfc0761.txt
      http://www.ietf.org/rfc/rfc0768.txt
    */
    udp_header->uh_sum  = wrapsum(in_cksum((unsigned char *)udp_header, sizeof(struct udphdr),
					   in_cksum((unsigned char *)dataBuffer, dataBufferLen,
						    in_cksum((unsigned char *)&ip_header->saddr,
							     2*sizeof(ip_header->saddr),
							     IPPROTO_UDP + ntohs(udp_header->uh_ulen)))));
    rc = send_buffer(sock, buffer, buffer_size, 0,
		(struct sockaddr*)dest,
		sizeof(struct sockaddr_in));

    /*
      printf("buff %d [rc=%d][dataBufferLen=%d]\n",
      buffer_size, rc, dataBufferLen);
    */

    return(rc > 0 ? (rc-headerLen) : rc);
  }
}

#endif /* linux */

/* ****************************************************** */

void reopenSocket(CollectorAddress *collector) {
  int rc, sockopt = 1;

  traceEvent(TRACE_WARNING,
	     "Attempting to reopen the socket. Please wait....");

  close(collector->sockFd), collector->sockFd = -1;

  if(collector->transport == TRANSPORT_TCP)
    collector->sockFd = socket(AF_INET, SOCK_STREAM, 0);
#ifdef HAVE_SCTP
  else if(collector->transport == TRANSPORT_SCTP)
    collector->sockFd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
#endif

  if(collector->sockFd == -1) {
    traceEvent(TRACE_ERROR, 
	       "Fatal error while creating socket (%s). Trying again later.",
	       strerror(errno));
    return;
  }

  setsockopt(collector->sockFd, SOL_SOCKET, SO_REUSEADDR, 
	     (char *)&sockopt, sizeof(sockopt));

  if(collector->transport == TRANSPORT_TCP) {
#ifndef IPV4_ONLY
    if(collector->isV6)
      rc = connect(collector->sockFd,
		   (struct sockaddr *)&collector->u.v6Address,
		   sizeof(struct sockaddr_in6));
    else
#endif
      rc = connect(collector->sockFd,
		   (struct sockaddr *)&collector->u.v4Address,
		   sizeof(struct sockaddr_in));

    if(rc == -1)
      traceEvent(TRACE_ERROR, 
		 "Connection failed with remote peer [%s]. "
		 "Trying again later.\n", strerror(errno));
    else {
      /* Peer reconnected */
      /*
	 NOTE
	 When a peer is reconnected the template should be resent
	 only to it. However in order to keep the code simple, the
	 template is resent to everyone.
      */
      /* Force the probe to resend the template */
      packetsBeforeSendingTemplates = 0; 
    }
  }

  collector->flowSequence = 0;
}

/* ****************************************************** */

static int sendFlowData(CollectorAddress *collector, char *buffer,
			int bufferLength, int sequenceIncrement) {
  int rc;
  u_int32_t flow_sequence;
  struct timeval now;

#ifdef DEMO_MODE
  if(collector->flowSequence > MAX_DEMO_FLOWS) return(0);
#endif

  errno = 0;

  /* traceEvent(TRACE_INFO, "sendFlowData: len=%d\n", bufferLength); */

  /*
     We need to fill the sequence number according to the collector
     sequence.
  */

  /* traceEvent(TRACE_INFO, "**** flowSequence=%d", 
     collector->flowSequence); */

  flow_sequence = htonl(collector->flowSequence);
  memcpy((char*)&buffer[netFlowVersion == 9 ? 12 /* version+count+sysUptime+unis_secs */
			: 16 /* version+count+sysUptime+unis_secs+unis_nsecs */],
	 &flow_sequence, 4);

  /*
    This delay is used to slow down export rate as some
    collectors might not be able to catch up with nProbe
  */
  if(flowExportDelay > 0) {
#ifndef WIN32
    struct timespec timeout;
#endif
    u_int32_t msDiff;

    gettimeofday(&now, NULL);
    msDiff = msTimeDiff(now, collector->lastExportTime);

    if(msDiff < flowExportDelay) {
      msDiff = flowExportDelay - msDiff;

#ifdef DEBUG
      traceEvent(TRACE_INFO, "Sleeping for %d msec", msDiff);
#endif

#ifndef WIN32
      timeout.tv_sec = 0;
      timeout.tv_nsec = 1000*msDiff;
      while((nanosleep(&timeout, &timeout) == -1) && (errno == EINTR))
	; /* Do nothing */
#else
      waitForNextEvent(msDiff);
#endif
    }
  }

  if(collector->transport == TRANSPORT_TCP)
    rc = send(collector->sockFd, buffer, bufferLength, 0);
  else {
    if(collector->isV6 == 0) {
#ifdef linux
      if(collector->transport == TRANSPORT_UDP_RAW)
	rc = send_raw_socket(collector->sockFd, buffer, bufferLength,
			     &collector->u.v4Address);
      else
#endif
	rc = send_buffer(collector->sockFd, buffer, bufferLength,
			 0, (struct sockaddr *)&collector->u.v4Address,
			 sizeof(collector->u.v4Address));
    }
#ifndef IPV4_ONLY
    else
      rc = send_buffer(collector->sockFd, buffer, bufferLength,
		       0, (struct sockaddr *)&collector->u.v6Address,
		       sizeof(collector->u.v6Address));
#endif
  }

  /*
    Note that on NetFlow v9 the sequence number is
    incremented per NetFlow packet sent and not per
    flow sent as for previous versions.
  */
  collector->flowSequence += sequenceIncrement;

  if(flowExportDelay > 0)
    collector->lastExportTime.tv_sec = now.tv_sec, 
      collector->lastExportTime.tv_usec = now.tv_usec;

  if((rc == -1) && (errno == EPIPE /* Broken pipe */)) {
    traceEvent(TRACE_WARNING, "Socket %d disconnected.", collector->sockFd);
    reopenSocket(collector);
  }

  return(rc);
}

/* ****************************************************** */

void sendNetFlow(void *buffer, u_int32_t bufferLength,
		 u_char lastFlow, int sequenceIncrement,
		 u_char broadcastToAllCollectors) {
  u_int32_t rc = 0;
  static u_short collectorId = 0;

#ifdef TIME_PROTECTION
  {
    struct tm expireDate;

#define EXPIRE_DAY    30
#define EXPIRE_MONTH  10
#define EXPIRE_YEAR   2004

    memset(&expireDate, 0, sizeof(expireDate));
    expireDate.tm_mday = EXPIRE_DAY;
    expireDate.tm_mon  = EXPIRE_MONTH-1;
    expireDate.tm_year = EXPIRE_YEAR-1900;

    if(time(NULL) > mktime(&expireDate)) {
      traceEvent(TRACE_ERROR, "Sorry: this copy of nProbe is expired.\n");
      exit(0);
    }
  }
#endif

#ifdef DEBUG
  traceEvent(TRACE_INFO, "==>> sendNetFlow(%d)", bufferLength);
#endif

  if(numCollectors == 0) return;

  if(reflectorMode || broadcastToAllCollectors) {
    /* Send all the flows to all collectors */
    int i;

    for(i = 0; i<numCollectors; i++) {
      if(shutdownInProgress) break;

      rc = sendFlowData(&netFlowDest[i], buffer, bufferLength, 
			sequenceIncrement);

      if(rc != bufferLength)
	traceEvent(TRACE_INFO, "Error while exporting flows (%s)", 
		   strerror(errno));
#ifdef DEBUG
      else {
	char addrbuf[INET6_ADDRSTRLEN];

	totBytesExp += rc, totExpPktSent++;

	if(netFlowDest[i].isV6 == 0)
	  traceEvent(TRACE_INFO, "Sent flow packet to %s",
		     inet_ntoa(netFlowDest[i].u.v4Address.sin_addr));
	else
	  traceEvent(TRACE_INFO, "Sent flow packet to [%s]",
		     inet_ntop(AF_INET6, (void *)&(netFlowDest[i].u.v6Address),
			       addrbuf, sizeof (addrbuf)));
      }
#endif /* DEBUG */
    }
  } else {
    /* Send flows to all collectors in round robin */
    rc = sendFlowData(&netFlowDest[collectorId], buffer, 
		      bufferLength, sequenceIncrement);

    /* Switch to next collector */
    collectorId = (collectorId + 1) % numCollectors;
  }

  if(rc != bufferLength)
    traceEvent(TRACE_WARNING, "Error while exporting flows (%s)", 
	       strerror(errno));
}

/* ****************************************************** */

void sendNetFlowV5(NetFlow5Record *theV5Flow, u_char lastFlow) {
  int len;

  if(theV5Flow->flowHeader.count == 0) return;

  if(traceMode)
    traceEvent(TRACE_INFO, "Sending %d flows (NetFlowV5 format)",
	       ntohs(theV5Flow->flowHeader.count));

  len = (ntohs(theV5Flow->flowHeader.count)*sizeof(struct flow_ver5_rec)
	 +sizeof(struct flow_ver5_hdr));

  if(flowFd && (textFormat == NULL)) {
#ifdef HAVE_ZLIB_H
    if(compressFlows == 1) {
      char outBuf[2000];
      uLongf outBufLen = sizeof(outBuf);

      if(compress(outBuf, &outBufLen, (const Bytef*)theV5Flow, len) == Z_OK) {
	fprintf(flowFd, "%04d", (int)outBufLen);
	(void)fwrite((const  void*)outBuf, outBufLen, 1, flowFd);
	if(traceMode) traceEvent(TRACE_INFO, "Compression %d->%d [%d %% size]",
				 len, outBufLen, (outBufLen*100)/len);

	sendNetFlow(outBuf, outBufLen, lastFlow,
		    ntohs(theV5Flow->flowHeader.count), 0);
      } else {
	traceEvent(TRACE_ERROR, "Unable to compress flow (len=%d)", len);
      }
    } else
#endif
      {
	fprintf(flowFd, "%04d", len);
	(void)fwrite((const  void*)theV5Flow, len, 1, flowFd);
      }
  }

  sendNetFlow((char *)theV5Flow, len, lastFlow, 
	      ntohs(theV5Flow->flowHeader.count), 0);
}

/* ****************************************************** */

void initNetFlowV5Header(NetFlow5Record *theV5Flow) {
  memset(&theV5Flow->flowHeader, 0, sizeof(theV5Flow->flowHeader));

  theV5Flow->flowHeader.version        = htons(5);
  theV5Flow->flowHeader.sysUptime      = htonl(msTimeDiff(actTime, 
							  initialSniffTime));
  theV5Flow->flowHeader.unix_secs      = htonl(actTime.tv_sec);
  theV5Flow->flowHeader.unix_nsecs     = htonl(actTime.tv_usec/1000);
  /* NOTE: theV5Flow->flowHeader.flow_sequence 
     will be filled by sendFlowData */
  theV5Flow->flowHeader.engine_type    = htons(engineType);
  theV5Flow->flowHeader.engine_id      = htons(engineId);
  theV5Flow->flowHeader.sampleRate     = htons(sampleRate);
}

/* ****************************************************** */

void initNetFlowV9Header(V9FlowHeader *v9Header) {
  memset(v9Header, 0, sizeof(V9FlowHeader));
  v9Header->version        = htons(9);
  v9Header->sysUptime      = htonl(msTimeDiff(actTime, initialSniffTime));
  v9Header->unix_secs      = htonl(time(NULL));
  v9Header->sourceId       = htons(engineType); /* CHECK */
}

/* ****************************************************** */

void printHash() {
  u_int i;

  for(i = 0; i<hashSize; i++) {
    if(theHash[i] != NULL)
      printf("theHash[%4d]\n", i);
  }
}

/* ****************************************************** */

void dumpBuffer(char *buffer, int bufferLength) {
  int i;

  if(bufferLength > 512) bufferLength = 512;

  for(i=0; i<bufferLength; i++) {
    if(!(i % 8)) printf("\n");
    printf("%3d[%02x] ", i, buffer[i] & 0xFF );
  }

  printf("\n");
}

/* ****************************************************** */

static int padding(int len) {
  int module = len % 4;

  if(module == 0)
    return(0);
  else  
    return(4 - module);
}

/* ****************************************************** */

void sendNetFlowV9(u_char lastFlow, u_char sendTemplate, 
		   u_char sendOnlyTheTemplate) {
  V9FlowSet flowSet;
  char flowBuffer[1514 /* Ethernet MTU */ - 42 /* Ethernet+IP+UDP header */];
  int bufLen = 0, len, pad;

  /* NOTE: theV9Header.flow_sequence will be filled by sendFlowData */
  memcpy(&flowBuffer[bufLen], &theV9Header, sizeof(theV9Header)); 
  bufLen += sizeof(theV9Header);

  /*
    NOTE:
    In order to keep things simple, whenever there are multiple
    collectors in round robin and the template needs to be sent out
    it is sent alone (i.e. without incuding flows) to all the collectors.

    If there is just one collector, the template also contains flows
    up to the MTU size.
  */
  if(sendTemplate) {
    V9Template templateDef;
    V9OptionTemplate optionTemplateDef;
    char tmpBuffer[256];
    int flowBufBegin, flowBufMax, numElements, optionTemplateId = idTemplate+1;

    templateDef.templateFlowset = htons(0);
    len = sizeof(V9Template)+templateBufBegin;
    pad = padding(len); len += pad;
    templateDef.fieldCount = htons(numTemplateFieldElements);
    templateDef.flowsetLen = htons(len);
    templateDef.templateId = htons(idTemplate);

    memcpy(&flowBuffer[bufLen], &templateDef, sizeof(V9Template)); 
    bufLen += sizeof(V9Template);
    memcpy(&flowBuffer[bufLen], templateBuffer, templateBufBegin); 
    bufLen += templateBufBegin;
    bufLen += pad;

    /* Options Template */
    optionTemplateDef.templateFlowset = htons(1); /* 1 = option template */
    len = sizeof(V9OptionTemplate)+optionTemplateBufBegin;
    pad = padding(len); len += pad;
    optionTemplateDef.flowsetLen     = htons(len);
    optionTemplateDef.templateId     = htons(optionTemplateId);
    optionTemplateDef.optionScopeLen = htons(4 /* SystemId=2 + SystemLen=2 */);
    optionTemplateDef.optionLen      = htons(4 /* each field is 4 bytes */ 
					     * (numOptionTemplateFieldElements-1 /* 1=interface hack */));

    memcpy(&flowBuffer[bufLen], &optionTemplateDef, sizeof(V9OptionTemplate)); 
    bufLen += sizeof(V9OptionTemplate);
    memcpy(&flowBuffer[bufLen], optionTemplateBuffer, optionTemplateBufBegin); 
    bufLen += optionTemplateBufBegin;
    bufLen += pad;

    /* Options DataRecord */
    flowBufBegin = 0, flowBufMax = sizeof(tmpBuffer);
    flowPrintf(v9OptionTemplateList, tmpBuffer, &flowBufBegin, &flowBufMax,
	       &numElements, 0, NULL, 0, 0, 1);

    len = flowBufBegin+sizeof(flowSet);
    pad = padding(len); len += pad;
    flowSet.templateId = htons(optionTemplateId);
    flowSet.flowsetLen = htons(len);

    memcpy(&flowBuffer[bufLen], &flowSet, sizeof(flowSet)); 
    bufLen += sizeof(flowSet);
    memcpy(&flowBuffer[bufLen], tmpBuffer, flowBufBegin); 
    bufLen += flowBufBegin;
    bufLen += pad;
  }

  if(!sendOnlyTheTemplate) {
    /* Fill the PDU with records up to the MTU size */
    flowSet.templateId = htons(idTemplate);
    len = bufferLen+4;
    pad = padding(len); len += pad;
    flowSet.flowsetLen = htons(len);
    memcpy(&flowBuffer[bufLen], &flowSet, sizeof(flowSet)); 
    bufLen += sizeof(flowSet);

    if((bufLen+bufferLen) >= sizeof(flowBuffer)) {
      traceEvent(TRACE_WARNING, 
		 "Internal error: too many NetFlow flows per packet (see -m)");
      bufferLen = sizeof(flowBuffer)-bufLen-1;
    }

    memcpy(&flowBuffer[bufLen], buffer, bufferLen); bufLen += bufferLen;
    bufLen += pad;
    sendNetFlow(&flowBuffer, bufLen, 0, 1, 0);
  } else
    sendNetFlow(&flowBuffer, bufLen, 0, 1, 1);

  bufferLen = 0;
}

/* ****************************************************** */

static void checkNetFlowExport(int forceExport) {
  int emitFlow, deltaFlows, flowExpired;

  if(numFlows == 0) return;

  if((netFlowVersion == 9)
     && (numCollectors > 1) && (!reflectorMode) /* Round-robin mode */
     && (packetsBeforeSendingTemplates == 0) /* It's time to send the template */
     ) {
    initNetFlowV9Header(&theV9Header);

    theV9Header.count = htons(3);
    sendNetFlowV9(0, 1, 1);
    deltaFlows = 0, packetsBeforeSendingTemplates = 
      numCollectors*templatePacketsDelta;
  } else {
    if((netFlowVersion == 9) && (packetsBeforeSendingTemplates == 0))
      deltaFlows = templateFlowSize;
    else
      deltaFlows = 0;
  }

  emitFlow = ((deltaFlows+numFlows) >= minNumFlowsPerPacket)
    || (forceExport && shutdownInProgress);
  gettimeofday(&actTime, NULL);
  flowExpired = lastExportTime.tv_sec && 
    (actTime.tv_sec > (lastExportTime.tv_sec+sendTimeout));

  if(forceExport || emitFlow || flowExpired) {
    if(netFlowVersion == 5) {
      initNetFlowV5Header(&theV5Flow);
      theV5Flow.flowHeader.count = htons(numFlows);
      sendNetFlowV5(&theV5Flow, 0);
    } else {
      initNetFlowV9Header(&theV9Header);

      if(deltaFlows > 0)
	theV9Header.count = htons(4);
      else
	theV9Header.count = htons(1);

      sendNetFlowV9(0, deltaFlows > 0 ? 1 : 0, 0);

      if(packetsBeforeSendingTemplates == 0)
	packetsBeforeSendingTemplates = templatePacketsDelta;
      else
	packetsBeforeSendingTemplates--;
    }

    totFlowExp += numFlows;
    numFlows = 0, totExports++, numExports++;
    lastExportTime.tv_sec = actTime.tv_sec, 
      lastExportTime.tv_usec = actTime.tv_usec;
  }
}

/* ****************************************************** */

static void checkExportQueuedFlows(int forceExport) {
  checkNetFlowExport(forceExport);
}

/* ****************************************************** */

static int exportBucketToNetflowV5(HashBucket *myBucket, int direction) {
  if(direction == 0 /* src -> dst */) {
    if(myBucket->pktSent == 0) return(0); /* Nothing to export */
    theV5Flow.flowRecord[numFlows].srcaddr   = htonl(myBucket->src.ipType.ipv4);
    theV5Flow.flowRecord[numFlows].dstaddr   = htonl(myBucket->dst.ipType.ipv4);
    theV5Flow.flowRecord[numFlows].dPkts     = htonl(myBucket->pktSent);
    theV5Flow.flowRecord[numFlows].dOctets   = htonl(myBucket->bytesSent);
    theV5Flow.flowRecord[numFlows].First     = htonl(msTimeDiff(myBucket->firstSeenSent, initialSniffTime));
    theV5Flow.flowRecord[numFlows].Last      = htonl(msTimeDiff(myBucket->lastSeenSent, initialSniffTime));
    theV5Flow.flowRecord[numFlows].srcport   = htons(myBucket->sport);
    theV5Flow.flowRecord[numFlows].dstport   = htons(myBucket->dport);
    theV5Flow.flowRecord[numFlows].tos       = myBucket->src2dstTos;
    theV5Flow.flowRecord[numFlows].src_as    = htons(ip2AS(myBucket->src)); // htons(srcAS);
    theV5Flow.flowRecord[numFlows].dst_as    = htons(ip2AS(myBucket->dst)); // htons(dstAS);
    theV5Flow.flowRecord[numFlows].tcp_flags = myBucket->src2dstTcpFlags;
  } else {
    if(myBucket->pktRcvd == 0) return(0); /* Nothing to export */
    theV5Flow.flowRecord[numFlows].srcaddr   = htonl(myBucket->dst.ipType.ipv4);
    theV5Flow.flowRecord[numFlows].dstaddr   = htonl(myBucket->src.ipType.ipv4);
    theV5Flow.flowRecord[numFlows].dPkts     = htonl(myBucket->pktRcvd);
    theV5Flow.flowRecord[numFlows].dOctets   = htonl(myBucket->bytesRcvd);
    theV5Flow.flowRecord[numFlows].First     = htonl(msTimeDiff(myBucket->firstSeenRcvd, initialSniffTime));
    theV5Flow.flowRecord[numFlows].Last      = htonl(msTimeDiff(myBucket->lastSeenRcvd, initialSniffTime));
    theV5Flow.flowRecord[numFlows].srcport   = htons(myBucket->dport);
    theV5Flow.flowRecord[numFlows].dstport   = htons(myBucket->sport);
    theV5Flow.flowRecord[numFlows].tos       = myBucket->dst2srcTos;
    theV5Flow.flowRecord[numFlows].src_as    = htons(ip2AS(myBucket->dst)); // htons(dstAS);
    theV5Flow.flowRecord[numFlows].dst_as    = htons(ip2AS(myBucket->src)); // htons(srcAS);
    theV5Flow.flowRecord[numFlows].tcp_flags = myBucket->dst2srcTcpFlags;
  }

  theV5Flow.flowRecord[numFlows].input     = htons(inputInterfaceIndex);
  theV5Flow.flowRecord[numFlows].output    = htons(255 /* unknown device */);
  theV5Flow.flowRecord[numFlows].prot      = myBucket->proto;
  return(1);
}

/* ****************************************************** */

static int exportBucketToNetflowV9(HashBucket *myBucket, int direction) {
  int flowBufBegin = bufferLen, flowBufMax = NETFLOW_MAX_BUFFER_LEN;
  int numElements;

  if(direction == 0 /* src -> dst */) {
    if(myBucket->pktSent == 0) return(0); /* Nothing to export */
  } else {
    if(myBucket->pktRcvd == 0) return(0); /* Nothing to export */
  }

  flowPrintf(v9TemplateList, buffer, &flowBufBegin, &flowBufMax,
	     &numElements, 0, myBucket, direction, 0, 0);

  bufferLen = flowBufBegin;
  return(1);
}

/* ****************************************************** */

static int exportBucketToNetflow(HashBucket *myBucket, int direction) {
  int rc = 0;

  if(netFlowVersion == 5) {
    if(myBucket->src.ipVersion == 4)
      rc = exportBucketToNetflowV5(myBucket, direction);
    else {
      static char msgPrinted = 0;

      if(!msgPrinted) {
	traceEvent(TRACE_INFO, "Unable to export IPv6 flow using NetFlowV5. Dropped.");
	msgPrinted = 1;
      }
    }
  } else
    rc = exportBucketToNetflowV9(myBucket, direction);

  if(rc) {
    if(traceMode)
      printFlow(myBucket, direction);

    if(flowFd && textFormat)
      nprintf(flowFd, textFormat, myBucket, direction);

    numFlows++, totFlows++;
    checkNetFlowExport(0);
  }

  return(rc);
}

/* ****************************************************** */

void parseOptions(int argc, char* argv[]) {
  int _argc;
  char *_argv[MAX_NUM_OPTIONS], *theItem;
  char line[256];
  FILE *fd;
  int opt, i, opt_n = 0;
  u_char mandatoryParamOk=0;
#ifndef WIN32
  u_char becomeDaemon = 0;
#endif

  memset(&sockIn, 0, sizeof(sockIn));

  /* Set defaults */
  netFlowVersion = 5; /* NetFlow v5 */
  ignoreTcpUdpSrcPort = ignoreTcpUdpDstPort = 0;
  ignoreIpSrcAddress = ignoreIpDstAddress = ignoreTos = 0;
  ignoreAS = 0;
  numCollectors = 0;
  hashSize = HASH_SIZE;
  gettimeofday(&initialSniffTime, NULL);
  pcapFile = NULL;
  reflectorMode = 0;
  minFlowSize = 0;
  traceMode = 0;
  flowExportDelay = 0;
  engineType = 0, engineId = 0;
  useNetFlow = 0xFF;
  computeFingerprint = 0;
  tcpPayloadExport = 2, udpPayloadExport = icmpPayloadExport = otherPayloadExport = 0;
  stringTemplate = NULL;
#ifdef CHECKSUM
  checksum = NULL;
#else
  dirPath = NULL;
  compressFlows = 0;
#endif
  textFormat = NULL;
  maxPayloadLen = 0;
  bufferLen = 0;
  numFlows = 0;
  minNumFlowsPerPacket = -1;
  lastExportTime.tv_sec = 0, lastExportTime.tv_usec = 0;
  sampleRate = 0;

  if((argc == 2) && (argv[1][0] != '-')) {
    char *tok, cont=1;

    fd = fopen(argv[1], "r");

    if(fd == NULL) {
      traceEvent(TRACE_ERROR, "Unable to read config. file %s", argv[1]);
      exit(-1);
    }

    _argc = 0;
    _argv[_argc++] = strdup("nprobe");

    while(cont && fgets(line, sizeof(line), fd)) {
      /* printf("line='%s'\n", line); */

      tok = strtok(line, "=");

      while(tok != NULL) {
	int len;

	if(_argc >= MAX_NUM_OPTIONS) {
	  traceEvent(TRACE_ERROR, "Command line too long");
	  cont = 0; break;
	}
	len = strlen(tok)-1;
	if(tok[len] == '\n') tok[len] = '\0';
	/* printf("_argv[%d]='%s'\n", _argc, tok);  */
	_argv[_argc++] = strdup(tok);
	tok = strtok(NULL, "\n");
      }
    }

    fclose(fd);
  } else {
    if(argc >= MAX_NUM_OPTIONS)
      _argc = MAX_NUM_OPTIONS-1;
    else
      _argc = argc;

    /* Copy arguments */
    for(i=0; i<_argc; i++) {
      _argv[i] = strdup(argv[i]);
    }
  }

#ifdef IPV4_ONLY
  useIpV6 = 0;
#else
  useIpV6 = 1;
#endif

  optarg = NULL;
  while((opt = getopt(_argc, _argv,
		      "A:ab"
#ifdef CHECKSUM
		      "c:"
#endif
		      "C:d:D:e:E:f:g:hi:I:l:"
		      "M:m:N:n:o:p:P:q:r:R:s:S:t:T:u:U:w:x:vz:"
#ifndef WIN32
		      "G"
#endif
		      "k"
		      )) != EOF) {
    switch (opt) {
    case 'A':
      readASs(optarg);
      break;
    case 'a':
      reflectorMode = 1;
      break;
    case 'b':
      traceMode = 1;
      break;
#ifdef CHECKSUM
    case 'c':
      checksum = strdup(optarg);
      break;
#endif
    case 'C':
      flowLockFile = strdup(optarg);
      break;
    case 'P':
      dirPath = strdup(optarg);
      break;
    case 'D':
      if(optarg[0] == 'b') compressFlows = 0;
      else if(optarg[0] == 'c') compressFlows = 1;
      else textFormat = strdup(optarg);
      break;
    case 'd':
      idleTimeout = atoi(optarg);
      break;
    case 'E':
      theItem = strtok(optarg, ":");
      if(theItem == NULL) {
	traceEvent(TRACE_WARNING,
		   "WARNING: Wrong engine specified (-E flag): see help.");
      } else {
	engineType = (u_short)atoi(theItem);
	theItem = strtok(NULL, ":");

	if(theItem == NULL) {
	  traceEvent(TRACE_WARNING,
		     "WARNING: Wrong engine specified (-E flag): see help.");
	} else
	  engineId = (u_short)atoi(theItem);
      }
      break;
    case 'e':
      flowExportDelay = atoi(optarg);
      if(flowExportDelay > 1000) {
	traceEvent(TRACE_INFO, "Maximum flow export delay is 1000 ms");
	flowExportDelay = 1000;
      }
      break;
    case 'g':
      fd = fopen(optarg, "w");
      if(fd != NULL) {
	fprintf(fd, "%lu\n", (unsigned long)getpid());
	fclose(fd);
      } else
	traceEvent(TRACE_ERROR, "Unable to store PID in file %s", optarg);
      break;
    case 'f':
      if((optarg[0] == '\"') || (optarg[0] == '\'')) {
	netFilter = strdup(&optarg[1]);
	netFilter[strlen(netFilter)-2] = '\0';
      } else {
	netFilter = strdup(optarg);
      }
      break;
    case 'h':
      usage();
      return;
    case 'i':
#ifdef WIN32
      tmpDev = printAvailableInterfaces(atoi(optarg));
#else
      tmpDev = strdup(optarg);
#endif
      break;
    case 'm':
      minNumFlowsPerPacket = atoi(optarg);
      break;
    case 'p':
      switch(atoi(optarg)) {
      case 0: 
	ignoreTcpUdpSrcPort = ignoreTcpUdpDstPort = 1; 
	computeFingerprint = 0; 
	break;
      case 1:
	ignoreIpSrcAddress = ignoreIpDstAddress = 1;
	computeFingerprint = 0;
	break;
      case 2: 
	ignoreTcpUdpSrcPort = ignoreTcpUdpDstPort = 
	  ignoreIpSrcAddress = ignoreIpDstAddress = ignoreTos = ignoreAS = 1;
	computeFingerprint = 0; break;
      default:
	traceEvent(TRACE_WARNING, "Sorry: the -p parameter is out of range");
      }
      break;
    case 'r':
      pcapFile = strdup(optarg);
      traceEvent(TRACE_INFO, "Reading packets from file %s", pcapFile);
      break;
    case 'R':
      maxPayloadLen = atoi(optarg);
      if(maxPayloadLen > MAX_PAYLOAD_LEN) {
	maxPayloadLen = MAX_PAYLOAD_LEN;
	traceEvent(TRACE_WARNING, "WARNING: payload limited to %d bytes", 
		   maxPayloadLen);
      }
      break;
    case 's':
      scanCycle = atoi(optarg);
      break;
#ifndef WIN32
    case 'G':
      becomeDaemon = 1;
      break;
#endif
    case 'l':
      sendTimeout = atoi(optarg);
      break;
    case 'M':
      maxNumActiveFlows = (u_int)atoi(optarg);
      break;
    case 'S':
      sampleRate = atoi(optarg);
      if(sampleRate > MAX_SAMPLE_RATE) {
	sampleRate = MAX_SAMPLE_RATE;
	traceEvent(TRACE_WARNING,
		   "WARNING: sample rate set to %d [range 0:%d]",
		   MAX_SAMPLE_RATE, MAX_SAMPLE_RATE);
      } else if(sampleRate == 1)
	sampleRate = 0; /* 1:1 is no sampling */
      break;
    case 't':
      lifetimeTimeout = atoi(optarg);
      break;
    case 'u':
      inputInterfaceIndex = atoi(optarg);
      break;
    case 'z':
      minFlowSize = (u_int)atoi(optarg);
      break;
    case 'v':
      probeVersion();
      exit(0);
    case 'w':
      hashSize = atoi(optarg);
      if(hashSize < HASH_SIZE) {
	hashSize = HASH_SIZE;
	traceEvent(TRACE_INFO, "Minimum hash size if %d.", hashSize);
      }
      break;
    case 'x':
      if(sscanf(optarg, "%d:%d:%d:%d",
		(int*)&tcpPayloadExport, (int*)&udpPayloadExport,
		(int*)&icmpPayloadExport, (int*)&otherPayloadExport)) {
	if(tcpPayloadExport > 2) {
	  tcpPayloadExport = 0;
	  traceEvent(TRACE_WARNING, 
		     "WARNING: wrong value for -x TCP value [range 0:2]");
	}

	if(udpPayloadExport > 1) {
	  udpPayloadExport = 0;
	  traceEvent(TRACE_WARNING,
		     "WARNING: wrong value for -x UDP value [range 0:1]");
	}

	if(icmpPayloadExport > 1) {
	  icmpPayloadExport = 0;
	  traceEvent(TRACE_WARNING, 
		     "WARNING: wrong value for -x ICMP value [range 0:1]");
	}

	if(otherPayloadExport > 1) {
	  otherPayloadExport = 0;
	  traceEvent(TRACE_WARNING,
		     "WARNING: wrong value for -x OTHER value [range 0:1]");
	}
      } else
	traceEvent(TRACE_INFO, "Wrong format for -x. See -h for more info");
      break;
#ifdef USE_SYSLOG
    case 'I':
      {
	u_int len = sizeof(nprobeId);

	if(len >= (sizeof(nprobeId)-1)) len = sizeof(nprobeId);
	strncpy(nprobeId, optarg, len);
	nprobeId[len] = '\0';
	useSyslog = 1;
      }
      break;
#endif
    case 'n':
      {
	int idx = strlen(optarg)-1;
	char *port = NULL, *addr = NULL;

	opt_n = 1;

	/*
	  IPv6 addresses should be delimited by square brackets
	  according to RFC 2732.
	*/

	while(idx > 0) {
	  if(optarg[idx] == ':') {
	    optarg[idx] = '\0';
	    addr = optarg, port = &optarg[idx+1];
	    break;
	  } else
	    idx--;
	}

	if(port == NULL)
	  usage();
	else {
	  if(initNetFlow(addr, atoi(port)) == 0)
	    mandatoryParamOk++;
	}
      }
      break;
    case 'o':
      templatePacketsDelta = (u_short)atoi(optarg);
      break;
    case 'q':
      {
	if(opt_n == 1) {
	  traceEvent(TRACE_ERROR, 
		     "You need to specify the -q option before the -n option."
		     " Please try again.");
	  exit(0);
	}

	bindAddr = strtok(optarg, ":");
	if(bindAddr != NULL) {
	  bindAddr = strdup(bindAddr);
	  bindPort = strtok(NULL, ":");
	  if(bindPort == NULL)
	    usage();
	  else
	    bindPort = strdup(bindPort);
	} else
	  usage();

	if(bindAddr != NULL) {
	  memset(&sockIn, 0, sizeof(sockIn));
	  /*
	    FreeBSD only
	    sockIn.sin_len = sizeof(struct sockaddr_in);
	  */
#ifdef IPV4_ONLY
	  sockIn.sin_family = AF_INET;
#else
	  sockIn.sin_family = AF_INET6;
#endif

	  if(bindPort)
	    sockIn.sin_port   = (int)htons((unsigned short int)atoi(bindPort));

	  if(!inet_aton(bindAddr, &sockIn.sin_addr)) {
	    traceEvent(TRACE_ERROR, "Unable to convert address '%s'. "
		       "Not binding to a particular interface", bindAddr);
	    sockIn.sin_addr.s_addr = INADDR_ANY;
	  }

	  /*
	    If we ask to bind to IPv4 via -q then we
	    implicitly ask to use IPv4
	  */
	  if(strstr(bindAddr, ":") == NULL)
	    useIpV6 = 0;
	}
      }
      break;

    case 'T':
      stringTemplate = strdup(optarg);
      netFlowVersion = 9; /* NetFlow v9 */
      if(useNetFlow == 0xFF) useNetFlow = 1;
      break;

    case 'U':
      idTemplate = atoi(optarg);
      netFlowVersion = 9; /* NetFlow v9 */
      if(useNetFlow == 0xFF) useNetFlow = 1;
      break;

    default:
      usage();
      break;
    }
  }

  if(useNetFlow == 0xFF) useNetFlow = 1;

  if(netFlowVersion == 5) {
    if(minNumFlowsPerPacket == -1)
      minNumFlowsPerPacket = V5FLOWS_PER_PAK; /* Default */

    if(minNumFlowsPerPacket > V5FLOWS_PER_PAK) {
      traceEvent(TRACE_WARNING, 
		 "Sorry: the min # of flows per packet cannot be set over %d",
		 V5FLOWS_PER_PAK);
      minNumFlowsPerPacket = V5FLOWS_PER_PAK;
    }
  }

  if(traceMode) traceEvent(TRACE_INFO, "Tracing enabled");

  if((dirPath != NULL) 
     && (textFormat == NULL) && (compressFlows == (u_char)-1)) {
    traceEvent(TRACE_WARNING,
	       "-P can be specified only with -D. Ignoring -P value [%s].",
	       dirPath);
    free(dirPath);
    dirPath = NULL;
  }

  if((dirPath == NULL) && (textFormat != NULL)) {
    traceEvent(TRACE_WARNING, 
	       "-P must be specified when -D is used. Ignoring -D value [%s].",
	       textFormat);
    free(textFormat);
    textFormat = NULL;
  }

  if(dirPath) {
    struct stat statbuf;

    if((stat(dirPath, &statbuf) != 0)
       || (!(statbuf.st_mode & S_IFDIR)) /* It's not a directory */
       || (!(statbuf.st_mode & S_IWRITE)) /* It's not writable    */
       ) {
      traceEvent(TRACE_ERROR, 
		 "Sorry, the path you specified with -P is invalid.");
      traceEvent(TRACE_ERROR, 
		 "Make sure the directory exists and it's writable.");
      exit(-1);
    }

    flowFd = NULL;
    mandatoryParamOk = 1; /* -P can substitute -n */
  }

  if(mandatoryParamOk == 0) {
    traceEvent(TRACE_WARNING, "-n parameter is missing. "
	       "127.0.0.1:2055 will be used.\n");
    initNetFlow("127.0.0.1", 2055);
  }

  for(i=0; i<_argc; i++)
    free(_argv[i]);

#ifndef WIN32
  if(becomeDaemon) 
    daemonize();
#endif

}

/* ****************************************************** */

char *printPayloadValue(u_char payloadExportType) {
  switch(payloadExportType) {
  case 0:
    return("no payload");
    break;
  case 1:
    return("full payload");
    break;
  case 2:
    return("payload only with SYN set");
    break;
  default:
    return("??");
  }
}

/* ****************************************************** */

void shutdown_nprobe() {
  static u_char once = 0;
  u_int i;

  if(once) return; else once = 1;

  traceEvent(TRACE_INFO, "nProbe is shutting down...\n");

  shutdownInProgress = 1;
  signalCondvar(&exportQueueCondvar);
  ntop_sleep(1);

  traceEvent(TRACE_INFO, "Exporting pending buckets...\n");
  for(i=0;i<hashSize; i++) {
    walkHash(1);
  }

  traceEvent(TRACE_INFO, "Flushing queued flows...\n");
  dequeueBucketToExport(NULL);
  checkExportQueuedFlows(1);

  traceEvent(TRACE_INFO, "Freeing memory...\n");

  for(i = 0; i<numCollectors; i++)
    close(netFlowDest[i].sockFd);

  pcap_close(pcapPtr);
  free(theHash);
  if(tmpDev != NULL) free(tmpDev);
  if(buffer != NULL) free(buffer);

  for(i=0; i<2; i++) {
    HashBucket *list;

    if(i == 0)
      list = purgedBuckets;
    else
      list = exportQueue;

    while(list != NULL) {
      HashBucket *nextEntry = list->next;

      if(list->src2dstPayload != NULL) free(list->src2dstPayload);
      if(list->dst2srcPayload != NULL) free(list->dst2srcPayload);
      free(list);
      bucketsAllocated--;
      list = nextEntry;
    }
  }

  while(fragmentsList != NULL) {
    IpV4Fragment *next = fragmentsList->next;
    free(fragmentsList);
    fragmentsList = next;
  }

#ifdef DEBUG
  if(bucketsAllocated > 0)
    traceEvent(TRACE_INFO, "WARNING ===> bucketsAllocated: %d\n", 
	       bucketsAllocated);
#endif

#ifndef WIN32
  if(useSyslog)
    closelog();
#endif

#ifdef ENABLE_PLUGINS
  termPlugins();
#endif

  traceEvent(TRACE_INFO, "nProbe terminated.\n");
  exit(0);
}

/* ******************************************* */

int openDevice(char ebuf[], int printErrors) {
  if(pcapFile != NULL) {
    pcapPtr = pcap_open_offline(pcapFile, ebuf);
    if(pcapPtr == NULL)
      return(-1);
  } else {
    if(tmpDev == NULL) {
#ifdef WIN32
      tmpDev = printAvailableInterfaces(0);
#else
      tmpDev = pcap_lookupdev(ebuf);
#endif
      if(tmpDev == NULL) {
	if(printErrors)
	  traceEvent(TRACE_ERROR,
		     "Unable to locate default interface (%s)\n", ebuf);
	return(-1);
      } else {
	char *_tmpDev = strdup(tmpDev);
	tmpDev = _tmpDev;
      }
    }

    pcapPtr = pcap_open_live(tmpDev,
#ifdef ENABLE_PLUGINS
			     1600
#else
			     DEFAULT_SNAPLEN
#endif
			     , 1 /* promiscous */, 100 /* ms */, ebuf);

    if(pcapPtr == NULL)  {
      if(printErrors)
	traceEvent(TRACE_ERROR, "Unable to open interface %s.\n", tmpDev);

#ifndef WIN32
      if((getuid () && geteuid ()) || setuid (0)) {
	if(printErrors) {
	  traceEvent(TRACE_ERROR,
		     "nProbe opens the network interface "
		     "in promiscuous mode, ");
	  traceEvent(TRACE_ERROR, "so it needs root permission "
		     "to run. Quitting...");
	}
      }
#endif
      return(-1);
    }
  }

  datalink = pcap_datalink(pcapPtr);

  /* ************************ */

  if(netFilter != NULL) {
    struct bpf_program fcode;
    struct in_addr netmask;

    netmask.s_addr = htonl(0xFFFFFF00);

    if((pcap_compile(pcapPtr, &fcode, netFilter, 1, netmask.s_addr) < 0)
       || (pcap_setfilter(pcapPtr, &fcode) < 0)) {
      if(printErrors)
	traceEvent(TRACE_ERROR,
		   "Unable to set filter %s. Filter ignored.\n", netFilter);
      /* return(-1); */
    } else {
      if(printErrors)
	traceEvent(TRACE_INFO, "Packet capture filter set to \"%s\"",
		   netFilter);
    }

    free(netFilter);
  }

  return(0);
}

/* ****************************************************** */

void restoreInterface(char ebuf[]) {
  if(pcapFile == NULL) {
    int rc = -1;

    traceEvent(TRACE_INFO,
	       "Error while capturing packets: %s",
	       pcap_geterr(pcapPtr));
    traceEvent(TRACE_INFO, "Waiting until the interface comes back...");

    while(rc == -1) {
      ntop_sleep(1);
      rc = openDevice(ebuf, 0);
    }
    traceEvent(TRACE_INFO, "The interface is now awailable again.");
  }
}

/* ****************************************************** */

int
#ifdef WIN32
nprobe_main
#else
main
#endif
(int argc, char *argv[]) {
  char ebuf[PCAP_ERRBUF_SIZE];
  u_int i, mallocSize;

#ifdef WIN32
  int optind=0;
#endif

#ifdef DEMO_MODE
  printf("*************************************************************\n");
  printf("* NOTE: This is a DEMO version limited to %d flows export. *\n",
	 MAX_DEMO_FLOWS);
  printf("*************************************************************\n\n");
#endif
#ifdef TIME_PROTECTION
  printf("*************************************************************\n");
  printf("* NOTE: This is a DEMO version operational until %02d/%02d/%02d.*\n",
	 EXPIRE_DAY, EXPIRE_MONTH, EXPIRE_YEAR);
  printf("*************************************************************\n\n");
#endif

#ifdef WIN32
  initWinsock32();
#endif

  parseOptions(argc, argv);

  traceEvent(TRACE_INFO, "Welcome to nprobe v.%s for %s", version, osName);

#ifndef WIN32
  if(useSyslog)
    openlog(nprobeId, LOG_PID ,LOG_DAEMON);
#endif

  if(bindAddr != NULL) {
#ifndef linux
    if(numCollectors != 1) {
      traceEvent(TRACE_ERROR, 
		 "You cannot specify multiple collectors when '-q' is used.");
      exit(-1);
    }

    if(bind(netFlowDest[1].sockFd, (struct sockaddr *)&sockIn, 
	    sizeof(sockIn)) < 0) {
      traceEvent(TRACE_ERROR, "unable to bind to %s:%s (%s).",
		 bindAddr, bindPort, strerror(errno));
      traceEvent(TRACE_ERROR, "Is another instance of nProbe running?");
    }
#endif

    free(bindAddr);
    free(bindPort);
  }

  theHash = NULL;
  bufferLen = 0;
  shutdownInProgress = 0;
  totBytesExp = totExpPktSent = totFlowExp = 0;

  createCondvar(&exportQueueCondvar);
  pthread_mutex_init(&exportMutex, NULL);
  pthread_mutex_init(&purgedBucketsMutex, NULL);

  for(i=0; i<MAX_HASH_MUTEXES; i++)
    pthread_mutex_init(&hashMutex[i], NULL);

  pthread_create(&dequeueThread, NULL, dequeueBucketToExport, NULL);
  pthread_create(&walkHashThread, NULL, hashWalker, NULL);

  buffer = (unsigned char*)malloc(NETFLOW_MAX_BUFFER_LEN);
  if(netFlowVersion == 9) {
    u_int i, flowLen, optionTemplateFlowSize = 0;
    
    compileTemplate(stringTemplate, v9TemplateList, TEMPLATE_LIST_LEN);
    flowPrintf(v9TemplateList, templateBuffer, 
	       &templateBufBegin, &templateBufMax,
	       &numTemplateFieldElements, 1, NULL, 0, 0, 0);

    /* 
       Optimization for NetFlow v9
       Discard fields that are not needed
    */
    if(strstr(stringTemplate, "%IP_SRC_ADDR") == NULL) ignoreIpSrcAddress = 1;
    if(strstr(stringTemplate, "%IP_DST_ADDR") == NULL) ignoreIpDstAddress = 1;
    if(strstr(stringTemplate, "%L4_SRC_PORT") == NULL) ignoreTcpUdpSrcPort = 1;
    if(strstr(stringTemplate, "%L4_DST_PORT") == NULL) ignoreTcpUdpDstPort = 1;
    if(strstr(stringTemplate, "%PAYLOAD") == NULL)     maxPayloadLen = 0;
    
    compileTemplate(OPTION_TEMPLATE, v9OptionTemplateList, TEMPLATE_LIST_LEN);
    flowPrintf(v9OptionTemplateList, optionTemplateBuffer,
	       &optionTemplateBufBegin, &optionTemplateBufMax,
	       &numOptionTemplateFieldElements, 1, NULL, 0, 0, 1);
    
    flowLen = 0;
    for(i=0; i<TEMPLATE_LIST_LEN; i++) {
      if(v9TemplateList[i] != NULL)
	flowLen += v9TemplateList[i]->templateLen;
      else
	break;
    }

    if(flowLen > 0) {
    for(i=0; i<TEMPLATE_LIST_LEN; i++) {
      if(v9OptionTemplateList[i] != NULL)
	optionTemplateFlowSize += v9OptionTemplateList[i]->templateLen;
      else
	break;
    }
    
    templateFlowSize = (((8+templateBufBegin)+(8+optionTemplateBufBegin)
			 +(4+optionTemplateFlowSize))/flowLen) 
      + 1 /* Just to be safe */;
    
    if(minNumFlowsPerPacket == -1) {
      /*
	  As with NetFlow v5, we suppose that a UDP packet can fit up to 1440
	  bytes (alias NETFLOW_MAX_BUFFER_LEN) of payload for NetFlow flows.
      */
      minNumFlowsPerPacket = (NETFLOW_MAX_BUFFER_LEN/flowLen)-1;
      traceEvent(TRACE_INFO, "Each flow is %d bytes length", flowLen);
      traceEvent(TRACE_INFO, "The # packets per flow has been set to %d", 
		 minNumFlowsPerPacket);
    } else {
      if((minNumFlowsPerPacket*flowLen) >= NETFLOW_MAX_BUFFER_LEN) {
	traceEvent(TRACE_WARNING, 
		   "Too many flows per packet specified using -m.");
	  minNumFlowsPerPacket = (NETFLOW_MAX_BUFFER_LEN/flowLen)-1;
	  traceEvent(TRACE_INFO, "The # packets per flow has been set to %d", 
		     minNumFlowsPerPacket);
	}
    }
    } else {
      netFlowVersion = 5;
      traceEvent(TRACE_INFO, "The flow size is zero. Switching back to v5");
    }
  }
  

  if(buffer == NULL) {
    traceEvent(TRACE_ERROR, "not enough memory\n");
    exit(-1);
  }

  exportQueue = NULL;

#ifndef WIN32
  signal(SIGTERM, cleanup);
  signal(SIGINT,  cleanup);
  signal(SIGPIPE, brokenPipe);
#endif

  /* pcap-based sniffing */
  mallocSize = sizeof(HashBucket*)*hashSize;
  theHash = (HashBucket**)calloc(1, mallocSize);
  if(theHash == NULL) {
    traceEvent(TRACE_ERROR, "not enough memory\n");
    exit(-1);
  }

  purgedBuckets = NULL;

  if((openDevice(ebuf, 1) == -1) || (pcapPtr == NULL)) {
    traceEvent(TRACE_ERROR,
	       "Unable to open interface %s (%s)\n",
	       tmpDev == NULL ? "<none>" : tmpDev, ebuf);
    exit(-1);
  }

#ifdef CHECKSUM
  {
    char *checksum;
    FILE *fd = fopen("/etc/nprobe/checksum", "r");

    if(fd == NULL)
      checksum = NULL;
    else {
      checksum = (char*)malloc(64);
      fgets(checksum, 64, fd);
      checksum[32] = '\0';
      fclose(fd);
    }

    /* Check only on the first device */
    if(checkMac("eth0" /* tmpDev */, checksum) != 0) {
      traceEvent(TRACE_ERROR,
		 "INFO: Wrong firmware checksum %s.\n", checksum);
      wrongChecksum = 1;
    }

    if(checksum != NULL) free(checksum);

    if(wrongChecksum) {
      traceEvent(TRACE_ERROR, "INFO: This copy of nProbe has not been designed for this box.\n");
      traceEvent(TRACE_ERROR, "INFO: You're probably running a bad firmware version.\n");
      traceEvent(TRACE_ERROR, "INFO: nProbe will now run in demo mode with export\n");
      traceEvent(TRACE_ERROR, "INFO: limited to %d flows.\n", MAX_DEMO_FLOWS);
    }
  }
#endif

  totalPkts = 0, totalBytes = 0, totalTCPPkts = 0, totalTCPBytes = 0;
  totalUDPPkts = 0, totalUDPBytes = 0, totalICMPPkts = 0, totalICMPBytes = 0;
  currentPkts = 0, currentBytes = 0, currentTCPPkts = 0, currentTCPBytes = 0;
  currentUDPPkts = 0, currentUDPBytes = 0,
    currentICMPPkts = 0, currentICMPBytes = 0;
  lastSample = time(NULL);

  traceEvent(TRACE_INFO, "The flows hash has %d buckets", hashSize);
  traceEvent(TRACE_INFO, "Flows older than %d seconds will be exported", lifetimeTimeout);
  traceEvent(TRACE_INFO, "Flows inactive for at least %d seconds will be exported", idleTimeout);
  traceEvent(TRACE_INFO, "Expired flows will be checked every %d seconds", scanCycle);
  traceEvent(TRACE_INFO, "Expired flows will not be queued for more than %d seconds", sendTimeout);

  if((engineType != 0) || (engineId != 0))
    traceEvent(TRACE_INFO, 
	       "Exported flows with engineType=%d and engineId=%d",
	       engineType, engineId);

  if(minFlowSize != 0)
    traceEvent(TRACE_INFO,
	       "TCP flows shorter than %u bytes will not be emitted",
	       minFlowSize);

  if(ignoreTcpUdpSrcPort)
    traceEvent(TRACE_INFO, "UDP/TCP src ports will be ignored and set to 0.");
  
  if(ignoreTcpUdpDstPort)
    traceEvent(TRACE_INFO, "UDP/TCP dst ports will be ignored and set to 0.");

  if(flowExportDelay > 0)
    traceEvent(TRACE_INFO, 
	       "The minimum intra-flow delay is of at least %d ms", 
	       flowExportDelay);

  if(flowLockFile != NULL) 
    traceEvent(TRACE_INFO,
	       "No flows will be sent if the lock file '%s' is present", 
	       flowLockFile);

  if(numCollectors > 1) {
    if(reflectorMode)
      traceEvent(TRACE_INFO, "All flows will be sent to all defined "
		 "collectors (NetFlow reflector mode)");
    else
      traceEvent(TRACE_INFO, "Flows will be sent to the defined collectors "
		 "in round robin.");
  }

  traceEvent(TRACE_INFO, "Flows will be emitted in NetFlow v%d format", 
	     (int)netFlowVersion);

  if(maxPayloadLen) {
    traceEvent(TRACE_INFO, "Max payload length set to %d bytes",
	       maxPayloadLen);
    traceEvent(TRACE_INFO, "Payload export policy (-x) for TCP:   %s",
	       printPayloadValue(tcpPayloadExport));
    traceEvent(TRACE_INFO, "Payload export policy (-x) for UDP:   %s",
	       printPayloadValue(udpPayloadExport));
    traceEvent(TRACE_INFO, "Payload export policy (-x) for ICMP:  %s",
	       printPayloadValue(icmpPayloadExport));
    traceEvent(TRACE_INFO, "Payload export policy (-x) for OTHER: %s",
	       printPayloadValue(otherPayloadExport));
  }

  if(sampleRate > 0)
    traceEvent(TRACE_INFO, "Sampling packets at 1:%d rate", sampleRate);

  if(pcapFile == NULL)
    traceEvent(TRACE_INFO, "Capturing packets from interface %s", tmpDev);

#ifdef ENABLE_PLUGINS
  initPlugins();
#endif

  if(sampleRate == 0) {
    while(!shutdownInProgress) {
      int rc = pcap_loop(pcapPtr, -1, processPacket, NULL);

      if(pcapFile && (rc == 0))
	break; /* No more packets to read from dump file */
      else if(rc == -1) {
	if(!shutdownInProgress) {
	  traceEvent(TRACE_ERROR, 
		     "Error while reading packets: '%s'",
		     pcap_geterr(pcapPtr));
	  pcap_close(pcapPtr);
	  restoreInterface(ebuf);
	}
      }
    }
  } else {
    u_short packetToGo = sampleRate;
    int rc;

    while(1) {
      if(packetToGo > 0) {
	rc = pcap_dispatch(pcapPtr, 1, dummyProcesssPacket, NULL);
	packetToGo--;
#ifdef DEBUG
	traceEvent(TRACE_INFO, "Discarded packet [%d packets to go]",
		   packetToGo);
#endif
      } else {
#ifdef DEBUG
	traceEvent(TRACE_INFO, "Processing packet");
#endif
	rc = pcap_dispatch(pcapPtr, 1, processPacket, NULL);
	packetToGo = sampleRate;
      }

      if(rc == -1) {
	if(!shutdownInProgress) {
	  traceEvent(TRACE_ERROR, "Error while reading packets: '%s'",
		     pcap_geterr(pcapPtr));
	  pcap_close(pcapPtr);
	  restoreInterface(ebuf);
	}
      } else if((rc == 0) && (pcapFile != NULL)) {
	/* Captured file is over */
	break;
      }
    } /* while */
  }

  if(pcapFile)
    traceEvent(TRACE_INFO, "No more packets to read. Sleeping...\n");
  
  shutdown_nprobe();

  return(0);
}

/* ******************************** */

#ifdef WIN32
static char* printAvailableInterfaces(int index) {
  char ebuf[PCAP_ERRBUF_SIZE];
  char *tmpDev = pcap_lookupdev(ebuf), *ifName;
  int ifIdx=0, defaultIdx = -1, numInterfaces = 0;
  u_int i;
  char intNames[32][256];

  if(tmpDev == NULL) {
    traceEvent(TRACE_INFO, "Unable to locate default interface (%s)", ebuf);
    exit(-1);
  }

  ifName = tmpDev;

  if(index == -1) printf("\n\nAvailable interfaces:\n");

  if(!isWinNT()) {
    for(i=0;; i++) {
      if(tmpDev[i] == 0) {
	if(ifName[0] == '\0')
	  break;
	else {
	  if(index == -1) {
	    numInterfaces++;
	    printf("\t[index=%d] '%s'\n", ifIdx, ifName);
	  }

	  if(ifIdx < 32) {
	    strcpy(intNames[ifIdx], ifName);
	    if(defaultIdx == -1) {
	      if(strncmp(intNames[ifIdx], "PPP", 3) /* Avoid to use the PPP interface */
		 && strncmp(intNames[ifIdx], "ICSHARE", 6)) {
		/* Avoid to use the internet sharing interface */
		defaultIdx = ifIdx;
	      }
	    }
	  }
	  ifIdx++;
	  ifName = &tmpDev[i+1];
	}
      }
    }

    tmpDev = intNames[defaultIdx];
  } else {
    /* WinNT/2K */
    static char tmpString[128];
    int i, j,ifDescrPos = 0;
    unsigned short *ifName; /* UNICODE */
    char *ifDescr;

    ifName = (unsigned short *)tmpDev;

    while(*(ifName+ifDescrPos) || *(ifName+ifDescrPos-1))
      ifDescrPos++;
    ifDescrPos++;	/* Step over the extra '\0' */
    ifDescr = (char*)(ifName + ifDescrPos); /* cast *after* addition */

    while(tmpDev[0] != '\0') {
      for(j=0, i=0; !((tmpDev[i] == 0) && (tmpDev[i+1] == 0)); i++) {
	if(tmpDev[i] != 0)
	  tmpString[j++] = tmpDev[i];
      }

      tmpString[j++] = 0;
      if(index == -1) {
	printf("\t[index=%d] '%s'\n", ifIdx, ifDescr);
	ifDescr += strlen(ifDescr)+1;
	numInterfaces++;
      }

      tmpDev = &tmpDev[i+3];
      strcpy(intNames[ifIdx++], tmpString);
      defaultIdx = 0;
    }

    if(defaultIdx != -1)
      tmpDev = intNames[defaultIdx]; /* Default */
  }

  if(index == -1) {
    if(numInterfaces == 0) {
      traceEvent(TRACE_WARNING, "no interfaces available! This application cannot");
      traceEvent(TRACE_WARNING, "work make sure that winpcap is installed properly");
      traceEvent(TRACE_WARNING, "and that you have network interfaces installed.");
    }
    return(NULL);
  } else if((index < 0) || (index > ifIdx)) {
    traceEvent(TRACE_ERROR, "Index=%d out of range\n", index);
    exit(-1);
  } else
    return(intNames[index]);
}
#endif /* WIN32 */

