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


/* ********************** */

#define MAX_NUM_NETWORKS 16

#define CONST_NETWORK_ENTRY                 0
#define CONST_NETMASK_ENTRY                 1
#define CONST_BROADCAST_ENTRY               2
#define CONST_INVALIDNETMASK                -1


/* ********************************************** */

#ifdef WIN32
#define nprobe_sleep(a /* sec */) waitForNextEvent(1000*a /* ms */)
extern unsigned long waitForNextEvent(unsigned long ulDelay /* ms */);
extern void initWinsock32();
extern short isWinNT();
#define close(fd) closesocket(fd)
#else
int nprobe_sleep(int secs);
#endif

extern void traceEvent(int eventTraceLevel, char* file, int line, char * format, ...);
extern void daemonize();
/*
#ifndef WIN32
extern int snprintf(char *string, size_t maxlen, const char *format, ...);
#endif
*/
extern void checkHostFingerprint(char *fingerprint, char *osName, int osNameLen);
extern u_short ip2AS(IpAddress ip);
extern void readASs(char *path);
extern void nprintf(FILE *stream, char *fmt, HashBucket *theFlow, int direction);
extern V9TemplateId ver9_templates[];
extern void printTemplateInfo(V9TemplateId *templates);
extern void dumpPluginHelp();
extern void enablePlugins();
extern void flowPrintf(V9TemplateId **templateList, char *outBuffer,
		       u_int *outBufferBegin, u_int *outBufferMax,
		       int *numElements, char buildTemplate,
		       HashBucket *theFlow, int direction,
		       int addTypeLen, int optionTemplate);
extern void compileTemplate(char *_fmt, V9TemplateId **templateList, int templateElements);
extern double toMs(struct timeval theTime);
extern u_int32_t msTimeDiff(struct timeval end, struct timeval begin);
extern unsigned int ntop_sleep(unsigned int secs);
extern HashBucket* getListHead(HashBucket **list);
extern void addToList(HashBucket *bkt, HashBucket **list);
extern void parseLocalAddressLists(char* _addresses);
extern unsigned short isLocalAddress(struct in_addr *addr);
extern uint32_t str2addr(char *address);
extern char* etheraddr_string(const u_char *ep, char *buf);
extern void setPayloadLength(int len);
extern u_int16_t ifIdx(HashBucket *theFlow, int direction, int inputIf);

extern void copyInt8(u_int8_t t8, char *outBuffer, u_int *outBufferBegin, u_int *outBufferMax);
extern void copyInt16(u_int16_t _t16, char *outBuffer, u_int *outBufferBegin, u_int *outBufferMax);
extern void copyInt32(u_int32_t _t32, char *outBuffer, u_int *outBufferBegin, u_int *outBufferMax);
extern void copyLen(u_char *str, int strLen, char *outBuffer, u_int *outBufferBegin, u_int *outBufferMax);

extern int32_t gmt2local(time_t t);
extern void resetBucketStats(HashBucket* bkt,
			     const struct pcap_pkthdr *h, 
			     u_int len,
			     u_short sport, u_short dport,
			     u_char *payload, int payloadLen);
/* nprobe.c */
extern char *stringTemplate;
extern int exportBucketToNetflow(HashBucket *myBucket, int direction, u_char free_memory);
