/*
 *  Copyright (C) 2002-05 Luca Deri <deri@ntop.org>
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

/* ********************************** */

extern HashBucket **theHash;
extern u_char traceMode, ignoreAS;
#ifndef WIN32
extern int useSyslog;
#endif
extern int traceLevel;
extern u_short maxPayloadLen;
extern u_short idleTimeout, lifetimeTimeout, sendTimeout;
extern u_int maxBucketSearch, hashSize;
extern struct timeval actTime;
extern u_short engineType, engineId;
extern u_char tcpPayloadExport, udpPayloadExport, icmpPayloadExport, otherPayloadExport;

/* ********************************** */

#define MAX_NUM_PLUGINS             8

#define CREATE_FLOW_CALLBACK        1
#define DELETE_FLOW_CALLBACK        2
#define PACKET_CALLBACK             3

extern V9TemplateId* getPluginTemplate(char* template_name);
extern int checkPluginExport(V9TemplateId *theTemplate, int direction,
			     HashBucket *theFlow, char *outBuffer,
			     u_int* outBufferBegin, u_int* outBufferMax);
typedef V9TemplateId* (*PluginConf)();
typedef void (*PluginInitFctn)(int argc, char *argv[]);
typedef void (*PluginFctn)(HashBucket*, void*);
typedef void (*PluginPacketFctn)(u_char new_bucket, void*, HashBucket*,
				 u_short proto, u_char isFragment,
				 u_short numPkts, u_char tos,
				 u_short vlanId, struct ether_header *ehdr,
				 IpAddress *src, u_short sport,
				 IpAddress *dst, u_short dport,
				 u_int len, u_int8_t flags, u_int8_t icmpType,
				 u_short numMplsLabels,
				 u_char mplsLabels[MAX_NUM_MPLS_LABELS][MPLS_LABEL_LEN],
				 char *fingerprint,
				 const struct pcap_pkthdr *h, const u_char *p,
				 u_char *payload, int payloadLen);
typedef V9TemplateId* (*PluginGetPluginTemplateFctn)(char* template_name);
typedef int (*PluginCheckPluginExportFctn)(void*, V9TemplateId *theTemplate, int direction,
					   HashBucket *theFlow, char *outBuffer,
					   u_int* outBufferBegin, u_int* outBufferMax);

typedef struct pluginInfo {
  char *name, *version, *descr, *author;
  u_char always_enabled, enabled;
  PluginInitFctn initFctn;
  PluginConf pluginFlowConf;
  PluginFctn deleteFlowFctn;
  PluginPacketFctn packetFlowFctn;
  PluginGetPluginTemplateFctn getPluginTemplateFctn;
  PluginCheckPluginExportFctn checkPluginExportFctn;
} PluginInfo;

extern PluginInfo* PluginEntryFctn(void);

/* ********************************** */
extern char* _intoa(IpAddress addr, char* buf, u_short bufLen);
extern char* _intoaV4(unsigned int addr, char* buf, u_short bufLen);
extern char* formatTraffic(float numBits, int bits, char *buf);
extern u_char ttlPredictor(u_char x);
extern char* proto2name(u_short proto);
extern void setPayload(HashBucket *bkt, const struct pcap_pkthdr *h,
		       u_char *payload, int payloadLen, int direction);
extern void updateApplLatency(u_short proto, HashBucket *bkt,
			      int direction, struct timeval *stamp,
			      u_int8_t icmpType, u_int8_t icmpCode);
extern void updateTcpFlags(HashBucket *bkt, int direction,
			   struct timeval *stamp, u_int8_t flags,
			   char *fingerprint, u_char tos);
extern int cmpIpAddress(IpAddress src, IpAddress dst);
extern void addPktToHash(u_short proto, u_char isFragment, u_short numPkts,
			 u_char tos, u_short vlanId, struct ether_header *ehdr,
			 IpAddress src, u_short sport,
			 IpAddress dst, u_short dport, u_int len,
			 u_int8_t flags,
			 u_int8_t icmpType, u_int8_t icmpCode,
			 u_short numMplsLabels,
			 u_char mplsLabels[MAX_NUM_MPLS_LABELS][MPLS_LABEL_LEN],
			 char *fingerprint, struct pcap_pkthdr *h, u_char *p,
			 u_char *payload, int payloadLen);
extern void printICMPflags(u_int32_t flags, char *icmpBuf, int icmpBufLen);
extern void printFlow(HashBucket *theFlow, int direction);
extern int isFlowExpired(HashBucket *myBucket, time_t theTime);
extern void printBucket(HashBucket *myBucket);
extern void walkHash(int flushHash);

/* nprobe.c or nprobe_mod.c */
extern void queueBucketToExport(HashBucket *myBucket);

/* plugin.c */
extern u_short num_plugins_enabled;
extern void initPlugins();
extern void termPlugins();
extern void pluginCallback(u_char callbackType, HashBucket* bucket,
			   u_short proto, u_char isFragment,
			   u_short numPkts, u_char tos,
			   u_short vlanId, struct ether_header *ehdr,
			   IpAddress *src, u_short sport,
			   IpAddress *dst, u_short dport,
			   u_int len, u_int8_t flags, u_int8_t icmpType,
			   u_short numMplsLabels,
			   u_char mplsLabels[MAX_NUM_MPLS_LABELS][MPLS_LABEL_LEN],
			   char *fingerprint,
			   const struct pcap_pkthdr *h, const u_char *p,
			   u_char *payload, int payloadLen);
