/*
 *  Copyright (C) 2005 Luca Deri <deri@ntop.org>
 *
 *  		       http://www.ntop.org/
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

#define BASE_ID           150

static V9TemplateId rtpPlugin_template[] = {
  { BASE_ID,   4, "RTP_FIRST_SSRC",   "First flow RTP Sync Source ID" },
  { BASE_ID+1, 4, "RTP_FIRST_TS",     "First flow RTP timestamp" },
  { BASE_ID+2, 4, "RTP_LAST_SSRC",    "Last flow RTP Sync Source ID" },
  { BASE_ID+3, 4, "RTP_LAST_TS",      "Last flow RTP timestamp" },
  { BASE_ID+4, 4, "RTP_IN_JITTER",        "RTP Jitter (ms * 1000)" },
  { BASE_ID+5, 4, "RTP_OUT_JITTER",       "RTP Jitter (ms * 1000)" },
  { BASE_ID+6, 4, "RTP_IN_PKT_LOST",      "Packet lost in stream" },
  { BASE_ID+7, 4, "RTP_OUT_PKT_LOST",     "Packet lost in stream" },
  { BASE_ID+8, 1, "RTP_OUT_PAYLOAD_TYPE", "RTP payload type" },
  { BASE_ID+9, 4, "RTP_IN_MAX_DELTA",     "Max delta (ms*100) between consecutive pkts" },
  { BASE_ID+10, 4, "RTP_OUT_MAX_DELTA",    "Max delta (ms*100) between consecutive pkts" },
  { 0,   0, NULL, NULL }
};

/* ************************************ */

struct rtp_header {
  u_int8_t version, payload_type;
  u_int16_t sequence_num;
  u_int32_t timestamp, sync_source_id;
};

/* *********************************************** */

struct plugin_info {
  u_int8_t  payloadType;
  u_int32_t src2dstInitialTimestamp, dst2srcInitialTimestamp;
  u_int32_t src2dstPrevTimestamp, dst2srcPrevTimestamp;
  u_int32_t src2dstTransit, dst2srcTransit;
  double    src2dstJitter, dst2srcJitter;
  u_int32_t src2dstInitialSeqNumber, dst2srcInitialSeqNumber;
  u_int32_t src2dstLastSeqNumber, dst2srcLastSeqNumber;
  struct timeval src2dstLastTs, dst2srcLastTs; /* private */
  u_int32_t src2dstMaxDelta, dst2srcMaxDelta;
};

/* *********************************************** */

static PluginInfo rtpPlugin; /* Forward */

/* ****************************************************** */

u_int32_t ms100TimeDiff(struct timeval end, struct timeval begin) {
  if((end.tv_sec == 0) && (end.tv_usec == 0))
    return(0);
  else
    return((end.tv_sec-begin.tv_sec)*100000+(end.tv_usec-begin.tv_usec)/10);
}

/* ******************************************* */

void rtpPlugin_init(int argc, char *argv[]) {
  traceEvent(TRACE_INFO, "Initialized RTP plugin\n");
}

/* *********************************************** */
/* Code taken from ethereal (gtk/rtp_analysis.c) */

/*
 * RTP Payload types
 * Table B.2 / H.225.0
 * Also RFC 1890, and
 *
 *	http://www.iana.org/assignments/rtp-parameters
 */
#define PT_PCMU		0	/* RFC 1890 */
#define PT_1016		1	/* RFC 1890 */
#define PT_G721		2	/* RFC 1890 */
#define PT_GSM		3	/* RFC 1890 */
#define PT_G723		4	/* From Vineet Kumar of Intel; see the Web page */
#define PT_DVI4_8000	5	/* RFC 1890 */
#define PT_DVI4_16000	6	/* RFC 1890 */
#define PT_LPC		7	/* RFC 1890 */
#define PT_PCMA		8	/* RFC 1890 */
#define PT_G722		9	/* RFC 1890 */
#define PT_L16_STEREO	10	/* RFC 1890 */
#define PT_L16_MONO	11	/* RFC 1890 */
#define PT_QCELP	12	/* Qualcomm Code Excited Linear Predictive coding? */
#define PT_CN		13	/* RFC 3389 */
#define PT_MPA		14	/* RFC 1890, RFC 2250 */
#define PT_G728		15	/* RFC 1890 */
#define PT_DVI4_11025	16	/* from Joseph Di Pol of Sun; see the Web page */
#define PT_DVI4_22050	17	/* from Joseph Di Pol of Sun; see the Web page */
#define PT_G729		18
#define PT_CN_OLD	19	/* Payload type reserved (old version Comfort Noise) */
#define PT_CELB		25	/* RFC 2029 */
#define PT_JPEG		26	/* RFC 2435 */
#define PT_NV		28	/* RFC 1890 */
#define PT_H261		31	/* RFC 2032 */
#define PT_MPV		32	/* RFC 2250 */
#define PT_MP2T		33	/* RFC 2250 */
#define PT_H263		34	/* from Chunrong Zhu of Intel; see the Web page */


typedef struct _key_value {
  u_int32_t  key;
  u_int32_t  value;
} key_value;


/* RTP sampling clock rates for fixed payload types as defined in
   http://www.iana.org/assignments/rtp-parameters */
static const key_value clock_map[] = {
  {PT_PCMU,       8000},
  {PT_1016,       8000},
  {PT_G721,       8000},
  {PT_GSM,        8000},
  {PT_G723,       8000},
  {PT_DVI4_8000,  8000},
  {PT_DVI4_16000, 16000},
  {PT_LPC,        8000},
  {PT_PCMA,       8000},
  {PT_G722,       8000},
  {PT_L16_STEREO, 44100},
  {PT_L16_MONO,   44100},
  {PT_QCELP,      8000},
  {PT_CN,         8000},
  {PT_MPA,        90000},
  {PT_G728,       8000},
  {PT_G728,       8000},
  {PT_DVI4_11025, 11025},
  {PT_DVI4_22050, 22050},
  {PT_G729,       8000},
  {PT_CN_OLD,     8000},
  {PT_CELB,       90000},
  {PT_JPEG,       90000},
  {PT_NV,         90000},
  {PT_H261,       90000},
  {PT_MPV,        90000},
  {PT_MP2T,       90000},
  {PT_H263,       90000},
};

#define NUM_CLOCK_VALUES	(sizeof clock_map / sizeof clock_map[0])

static u_int32_t get_clock_rate(u_int32_t key)
{
  size_t i;

  for (i = 0; i < NUM_CLOCK_VALUES; i++) {
    if (clock_map[i].key == key)
      return clock_map[i].value;
  }
  return 1;
}

/* *********************************************** */

/* Handler called whenever an incoming packet is received */

static void rtpPlugin_packet(u_char new_bucket, void *pluginData,
			     HashBucket* bkt,
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
			     u_char *payload, int payloadLen) {
  u_char payload_type;
  u_int32_t clock_rate;

  if((payload == NULL) || (payloadLen < 12 /* RTP header len */)) return;

  if(new_bucket /* This bucket has been created recently */) {
    /*
     * RTP Payload types
     * Table B.2 / H.225.0
     * Also RFC 1890, and
     *
     *	http://www.iana.org/assignments/rtp-parameters
     */
    payload_type = payload[1] & 0x7F;

    if(1) {
      traceEvent(TRACE_INFO, "==> Called rtpPlugin_packet(%d)[%02X][%02X]",
		 new_bucket, payload[0] & 0xFF, payload[1] & 0xFF);
    }

    /* Check whether this is an RTP flow */
    if((bkt->proto == 17)
       && ((payload[0] & 0xFF) == 0x80)   /* RTP magic byte[1] */
       && ((payload_type <= 34 /* PT_H263 */))) {
      PluginInformation *info;


      traceEvent(TRACE_INFO, "==> Found RTP flow");

      info = (PluginInformation*)malloc(sizeof(PluginInformation));
      if(info == NULL) {
	traceEvent(TRACE_ERROR, "Not enough memory?");
	return; /* Not enough memory */
      }

      info->pluginPtr  = (void*)&rtpPlugin;
      pluginData = info->pluginData = (struct plugin_info*)malloc(sizeof(struct plugin_info));

      if(info->pluginData == NULL) {
	traceEvent(TRACE_ERROR, "Not enough memory?");
	free(info);
	return; /* Not enough memory */
      } else {
	/* Reset fields */
	memset(pluginData, 0, sizeof(struct plugin_info));

	info->next = bkt->plugin;
	bkt->plugin = info;
      }
    }
  }

  /*
    See chapter A.8 of http://www.faqs.org/rfcs/rfc3550.html
    for more information about jitter calculation
  */
  if(pluginData != NULL) {
    struct plugin_info *info = (struct plugin_info*)pluginData;
    struct rtp_header *hdr = (struct rtp_header*)payload;
    u_int32_t ts = ntohl(hdr->timestamp), ssrc = ntohl(hdr->sync_source_id);
    double transit;

    /*
    {

  	current_time = (double)pinfo->fd->rel_secs + (double) pinfo->fd->rel_usecs/1000000;
	current_diff = fabs (current_time - (statinfo->time) - ((double)(rtpinfo->info_timestamp)-(double)(statinfo->timestamp))/clock_rate);
	current_jitter = statinfo->jitter + (current_diff - statinfo->jitter)/16;
    }
    */

    info->payloadType = hdr->payload_type & 0x7F;
    clock_rate = get_clock_rate(info->payloadType);

    if(bkt->sport == sport) {
      /* src -> dst */

      if(info->src2dstInitialSeqNumber == 0)
	transit = 0;
      else {
	transit =
	  fabs(((double)h->ts.tv_sec+(double)h->ts.tv_usec/1000000)-
	       ((double)info->src2dstLastTs.tv_sec+(double)info->src2dstLastTs.tv_usec/1000000)
	       - (double)(ts-info->src2dstPrevTimestamp)/clock_rate);
      }

      info->src2dstPrevTimestamp = ts;

      if(info->src2dstInitialSeqNumber == 0) {
	info->src2dstInitialTimestamp = ts;
	info->src2dstInitialSeqNumber = ssrc;
      } else {
	u_int32_t delta =  ms100TimeDiff(h->ts, info->src2dstLastTs);
	if(delta > info->src2dstMaxDelta) info->src2dstMaxDelta = delta;
      }

      memcpy(&info->src2dstLastTs, &h->ts, sizeof(struct timeval));

      info->src2dstLastSeqNumber = ssrc;

      info->src2dstJitter += (transit - info->src2dstJitter)/16;
      info->src2dstTransit = transit;

      /*
	traceEvent(TRACE_INFO, "[%d -> %d] -> [transit=%.2f][jitter=%.2f ms]",
	sport, dport, transit, info->src2dstJitter*1000);      
      */
    } else {
      /* dst -> src */

      if(info->dst2srcInitialSeqNumber == 0)
	transit = 0;
      else {
	transit =
	  fabs(((double)h->ts.tv_sec+(double)h->ts.tv_usec/1000000)-
	       ((double)info->dst2srcLastTs.tv_sec+(double)info->dst2srcLastTs.tv_usec/1000000)
	       - (double)(ts-info->dst2srcPrevTimestamp)/clock_rate);
      }

      info->dst2srcPrevTimestamp = ts;

      if(info->dst2srcInitialSeqNumber == 0) {
	info->dst2srcInitialTimestamp = ts;
	info->dst2srcInitialSeqNumber = ssrc;
      } else {
	u_int32_t delta =  ms100TimeDiff(h->ts, info->dst2srcLastTs);
	if(delta > info->dst2srcMaxDelta) info->dst2srcMaxDelta = delta;
      }

      memcpy(&info->dst2srcLastTs, &h->ts, sizeof(struct timeval));

      info->dst2srcLastSeqNumber = ssrc;

      info->dst2srcJitter += (transit - info->dst2srcJitter)/16;
      info->dst2srcTransit = transit;

      /*
	traceEvent(TRACE_INFO, "[%d -> %d] -> [transit=%.2f][jitter=%.2f ms]",
	sport, dport, transit, info->dst2srcJitter*1000);
      */
    }
  }
}

/* *********************************************** */

/* Handler called when the flow is deleted (after export) */

static void rtpPlugin_delete(HashBucket* bkt, void *pluginData) {

  if(pluginData != NULL) {
    struct plugin_info *info = (struct plugin_info*)pluginData;

#ifdef SIP_DEBUG
    char buf[256], buf1[256];

    traceEvent(TRACE_INFO, "RTP  '%s:%d'->'%s:%d'",
	       _intoa(bkt->src, buf, sizeof(buf)), info->rtp_src_port,
	       _intoa(bkt->dst, buf1, sizeof(buf1)), info->rtp_dst_port);
#endif

    free(info);
  }
}

/* *********************************************** */

/* Handler called at startup when the template is read */

static V9TemplateId* rtpPlugin_get_template(char* template_name) {
  int i;

  for(i=0; rtpPlugin_template[i].templateId != 0; i++) {
    if(!strcmp(template_name, rtpPlugin_template[i].templateName)) {
      return(&rtpPlugin_template[i]);
    }
  }

  return(NULL); /* Unknown */
}

/* *********************************************** */

/* Handler called whenver a flow attribute needs to be exported */

static int rtpPlugin_export(void *pluginData, V9TemplateId *theTemplate,
			    int direction /* 0 = src->dst, 1 = dst->src */,
			    HashBucket *bkt, char *outBuffer,
			    u_int* outBufferBegin, u_int* outBufferMax) {
  int i;

  for(i=0; rtpPlugin_template[i].templateId != 0; i++) {
    if(theTemplate->templateId == rtpPlugin_template[i].templateId) {
      if((*outBufferBegin)+rtpPlugin_template[i].templateLen > (*outBufferMax))
	return(-2); /* Too long */

      if(pluginData) {
	struct plugin_info *info = (struct plugin_info *)pluginData;
	u_int32_t jitter;
	int32_t pktLost;

	float pktLoss;

	switch(rtpPlugin_template[i].templateId) {
	case BASE_ID:
	  copyInt32(direction == 0 ? info->src2dstInitialSeqNumber : info->dst2srcInitialSeqNumber,
		    outBuffer, outBufferBegin, outBufferMax);
	  if(traceMode) traceEvent(TRACE_INFO, "RTP_FIRST_SSRC: %u",
				   direction == 0 ? info->src2dstInitialSeqNumber : info->dst2srcInitialSeqNumber);
	  break;
	case BASE_ID+1:
	  copyInt32(direction == 1 ? info->src2dstInitialTimestamp : info->dst2srcInitialTimestamp,
		    outBuffer, outBufferBegin, outBufferMax);
	  if(traceMode) traceEvent(TRACE_INFO, "RTP_FIRST_TS: %u",
				   direction == 1 ? info->src2dstInitialTimestamp : info->dst2srcInitialTimestamp);
	  break;
	case BASE_ID+2:
	  copyInt32(direction == 0 ? info->src2dstLastSeqNumber : info->dst2srcLastSeqNumber,
		    outBuffer, outBufferBegin, outBufferMax);
	  if(traceMode) traceEvent(TRACE_INFO, "RTP_FIRST_SSRC: %u",
				   direction == 0 ? info->src2dstLastSeqNumber : info->dst2srcLastSeqNumber);
	  break;
	case BASE_ID+3:
	  copyInt32(direction == 0 ? info->src2dstPrevTimestamp : info->dst2srcPrevTimestamp,
		    outBuffer, outBufferBegin, outBufferMax);
	  if(traceMode) traceEvent(TRACE_INFO, "RTP_FIRST_TS: %u",
				   direction == 0 ? info->src2dstPrevTimestamp : info->dst2srcPrevTimestamp);
	  break;
	case BASE_ID+4:
	  jitter = info->src2dstJitter*1000000;
	  copyInt32(jitter, outBuffer, outBufferBegin, outBufferMax);
	  if(traceMode) traceEvent(TRACE_INFO, "RTP_IN_JITTER: %.2f ms", (float)jitter/1000);
	  break;
	case BASE_ID+5:
	  jitter = info->dst2srcJitter*1000000;
	  copyInt32(jitter, outBuffer, outBufferBegin, outBufferMax);
	  if(traceMode) traceEvent(TRACE_INFO, "RTP_OUT_JITTER: %.2f ms", (float)jitter/1000);
	  break;
	case BASE_ID+6:
	  if(bkt->pktSent > 0) {
	    pktLost = info->src2dstLastSeqNumber+1 - info->src2dstInitialSeqNumber;
	    pktLost -= bkt->pktSent; if(pktLost < 0) pktLost = 0;
	    pktLoss = (float)(pktLost*10000)/(float)bkt->pktSent;
	  } else {
	    pktLost = 0; pktLoss= 0;
	  }
	  copyInt32((u_int32_t)pktLost, outBuffer, outBufferBegin, outBufferMax);
	  if(traceMode) traceEvent(TRACE_INFO, "RTP_IN_PKT_LOST: %.1f%% [%u pkts]",
				   pktLoss/100, pktLost);
	  break;
	case BASE_ID+7:
	  if(bkt->pktRcvd > 0) {
	    pktLost = info->dst2srcLastSeqNumber+1 - info->dst2srcInitialSeqNumber;
	    pktLost -= bkt->pktRcvd; if(pktLost < 0) pktLost = 0;
	    pktLoss = (float)(pktLost*10000)/(float)bkt->pktRcvd;
	  } else {
	    pktLost = 0; pktLoss= 0;
	  }
	  copyInt32((u_int32_t)pktLost, outBuffer, outBufferBegin, outBufferMax);
	  if(traceMode) traceEvent(TRACE_INFO, "RTP_OUT_PKT_LOST: %.1f%% [%u pkts]",
				   pktLoss/100, pktLost);
	  break;
	case BASE_ID+8:
	  copyInt8(info->payloadType, outBuffer, outBufferBegin, outBufferMax);
	  if(traceMode) traceEvent(TRACE_INFO, "RTP_PAYLOAD_TYPE: %u", info->payloadType);
	  break;
	case BASE_ID+9:
	  copyInt32(info->src2dstMaxDelta,
		   outBuffer, outBufferBegin, outBufferMax);
	  if(traceMode) traceEvent(TRACE_INFO, "RTP_IN_MAX_DELTA: %.2f ms",
				   (float)info->src2dstMaxDelta/100);
	  break;
	case BASE_ID+10:
	  copyInt32(info->dst2srcMaxDelta,
		   outBuffer, outBufferBegin, outBufferMax);
	  if(traceMode) traceEvent(TRACE_INFO, "RTP_OUT_MAX_DELTA: %.2f ms",
				   (float)info->dst2srcMaxDelta/100);
	  break;
	default:
	  return(-1); /* Not handled */
	}

	return(0);
      }
    }
  }

  return(-1); /* Not handled */
}

/* *********************************************** */

static V9TemplateId* rtpPlugin_conf() {
  return(rtpPlugin_template);
}

/* *********************************************** */

/* Plugin entrypoint */
static PluginInfo rtpPlugin = {
  "RTP",
  "0.2",
  "Handle RTP protocols",
  "L.Deri <deri@ntop.org>",
  0 /* not always enabled */, 1, /* enabled */
  rtpPlugin_init,
  rtpPlugin_conf,
  rtpPlugin_delete,
  rtpPlugin_packet,
  rtpPlugin_get_template,
  rtpPlugin_export
};


/* *********************************************** */

/* Plugin entry fctn */
#ifdef MAKE_STATIC_PLUGINS
PluginInfo* rtpPluginEntryFctn(void)
#else
  PluginInfo* PluginEntryFctn(void)
#endif
{
  return(&rtpPlugin);
}

