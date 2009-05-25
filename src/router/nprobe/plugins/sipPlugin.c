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

/* #define SIP_DEBUG */

#define SIP_INVITE        "INVITE" /* User Info */
#define SIP_OK            "SIP/2.0 200 Ok" /* Stream Info */

#include "nprobe.h"

#define BASE_ID             130
#define MAX_SIP_STR_LEN      50
#define SIP_CODECS_STR_LEN   32
#define DEFAULT_SIP_PORT   5060

static V9TemplateId sipPlugin_template[] = {
  { BASE_ID,    MAX_SIP_STR_LEN,   "SIP_CALL_ID",       "SIP call-id" },
  { BASE_ID+1,  MAX_SIP_STR_LEN,   "SIP_CALLING_PARTY", "SIP Call initiator" },
  { BASE_ID+2,  MAX_SIP_STR_LEN,   "SIP_CALLED_PARTY",  "SIP Called party" },
  { BASE_ID+3,  SIP_CODECS_STR_LEN,"SIP_RTP_CODECS",    "SIP RTP codecs" },
  { BASE_ID+4,   4,                "SIP_INVITE_TIME",   "SIP SysUptime (msec) of INVITE" },
  { BASE_ID+5,   4,                "SIP_TRYING_TIME",   "SIP SysUptime (msec) of Trying" },
  { BASE_ID+6,   4,                "SIP_RINGING_TIME",  "SIP SysUptime (msec) of RINGING" },
  { BASE_ID+7,   4,                "SIP_OK_TIME",       "SIP SysUptime (msec) of OK" },
  { BASE_ID+8,   4,                "SIP_ACK_TIME",      "SIP SysUptime (msec) of ACK" },
  { BASE_ID+9,   2,                "SIP_RTP_SRC_PORT",  "SIP RTP stream source port" },
  { BASE_ID+10,  2,                "SIP_RTP_DST_PORT",  "SIP RTP stream dest port" },
  { 0,   0, NULL, NULL }
};

/* *********************************************** */

struct plugin_info {
  char sip_call_id[MAX_SIP_STR_LEN];
  char sip_calling_party[MAX_SIP_STR_LEN];
  char sip_called_party[MAX_SIP_STR_LEN];
  char rtp_codecs[SIP_CODECS_STR_LEN];
  struct timeval sip_invite_time, sip_trying_time,
    sip_ringing_time, sip_ok_time, sip_ack_time;
  u_int16_t rtp_src_port, rtp_dst_port;
};

/* *********************************************** */

static PluginInfo sipPlugin; /* Forward */

/* ******************************************* */

void sipPlugin_init(int argc, char *argv[]) {
  traceEvent(TRACE_INFO, "Initialized SIP plugin\n");
}

/* *********************************************** */

/* Handler called whenever an incoming packet is received */

static void sipPlugin_packet(u_char new_bucket, void *pluginData,
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

  if((payload == NULL) || (payloadLen == 0)) return;

  if(new_bucket /* This bucket has been created recently */) {
    /* Check whether this is an RTP or SIP flow */
    if((bkt->proto == 17 /* UDP */)
       && (bkt->sport == DEFAULT_SIP_PORT)
       && (bkt->dport == DEFAULT_SIP_PORT) /* SIP */
       ) {
      PluginInformation *info;

      info = (PluginInformation*)malloc(sizeof(PluginInformation));
      if(info == NULL) {
	traceEvent(TRACE_ERROR, "Not enough memory?");
	return; /* Not enough memory */
      }

      info->pluginPtr  = (void*)&sipPlugin;
      pluginData = info->pluginData = (struct plugin_info*)malloc(sizeof(struct plugin_info));
 
      if(info->pluginData == NULL) {	
	traceEvent(TRACE_ERROR, "Not enough memory?");
	free(info);
	return; /* Not enough memory */
      } else {
	/* Set defaults */
	struct plugin_info *infos = (struct plugin_info*)pluginData;
	
	info->next = bkt->plugin;      
	bkt->plugin = info;
	
	memset(infos, 0, sizeof(struct plugin_info));
      }
    }
  }

  if(pluginData != NULL) {
    char *my_payload, *strtokState, *row;
#ifdef SIP_DEBUG
    char buf[32];
#endif
    char *from = NULL, *to = NULL, *server = NULL, *audio = NULL, *video = NULL;    
    struct plugin_info *info = (struct plugin_info*)pluginData;

    /* Handle your Sip packet here */
    my_payload = malloc(payloadLen+1);

    if(my_payload != NULL) {
      char *rtpmap;
      memcpy(my_payload, payload, payloadLen);
      my_payload[payloadLen] = '\0';

      row = strtok_r((char*)my_payload, "\r\n", &strtokState);

      if(row != NULL) {
	if(strstr(row, "INVITE"))       
	  info->sip_invite_time.tv_sec = h->ts.tv_sec, info->sip_invite_time.tv_usec = h->ts.tv_usec;
	else if(strstr(row, "Trying"))
	  info->sip_trying_time.tv_sec = h->ts.tv_sec, info->sip_trying_time.tv_usec = h->ts.tv_usec;
	else if(strstr(row, "Ringing"))
	  info->sip_ringing_time.tv_sec = h->ts.tv_sec, info->sip_ringing_time.tv_usec = h->ts.tv_usec;
	else if(strstr(row, "OK"))
	  info->sip_ok_time.tv_sec = h->ts.tv_sec, info->sip_ok_time.tv_usec = h->ts.tv_usec;
	else if(strstr(row, "ACK"))
	  info->sip_ack_time.tv_sec = h->ts.tv_sec, info->sip_ack_time.tv_usec = h->ts.tv_usec;

	row = strtok_r(NULL, "\r\n", &strtokState);

	while(row != NULL) {
#ifdef SIP_DEBUG
	  traceEvent(TRACE_INFO, "==> SIP [%d] '%s'", strlen(row), row);
#endif

	  if((from == NULL)
	     && ((!strncmp(row, "From: ", 6))  || (!strncmp(row, "f: ", 3)))) {
	    from = row;
	  } else if((to == NULL)
		    && ((!strncmp(row, "To: ", 4)) || (!strncmp(row, "t: ", 3)))) {
	    to = row;
	  } else if(!strncmp(row, "Call-ID: ", 8)) {
	    strncpy(info->sip_call_id, &row[9], MAX_SIP_STR_LEN);
	  } else if((server == NULL) && (!strncmp(row, "Server: ", 8))) {
	    server = row;
	  } else if((audio == NULL) && (!strncmp(row, "m=audio ", 8))) {
	    audio = row;
	  } else if((video == NULL) && (!strncmp(row, "m=video ", 8))) {
	    video = row;
	  } else if((rtpmap = strstr(row, "=rtpmap:")) != NULL) {
	    char *codec;
	    int i;
	  
	    if(rtpmap[10] == ' ') 
	      codec = &rtpmap[11];
	    else
	      codec = &rtpmap[10];
	  
	    for(i=0; codec[i] != '\0'; i++)
	      if(codec[i] == '/') {
		codec[i] = '\0';
		break;
	      }

	    if(info->rtp_codecs[0] == '\0') {
	      snprintf(info->rtp_codecs, sizeof(info->rtp_codecs)-1, "%s", codec);
	    } else {
	      if(strstr(info->rtp_codecs, codec) == NULL) {
		char tmpStr[SIP_CODECS_STR_LEN];
	      
		snprintf(tmpStr, sizeof(tmpStr)-1, "%s;%s", 
			 info->rtp_codecs, codec);
		strcpy(info->rtp_codecs, tmpStr);
	      }
	    }	 
	  }

	  row = strtok_r(NULL, "\r\n", &strtokState);
	}
      }

      if(server) {
	strtok_r(server, ":", &strtokState);
	server = strtok_r(NULL, ":", &strtokState);
#ifdef SIP_DEBUG
	/* traceEvent(TRACE_INFO, "Server '%s'", server); */
#endif
      }

    if(from && to && (!strncasecmp((char*)my_payload, SIP_INVITE, strlen(SIP_INVITE)))) {
	strtok_r(from, ":", &strtokState);
	strtok_r(NULL, ":\"", &strtokState);
	from = strtok_r(NULL, "\"@>", &strtokState);

	strtok_r(to, ":", &strtokState);
	strtok_r(NULL, "\":", &strtokState);
	to = strtok_r(NULL, "\"@>", &strtokState);
#ifdef SIP_DEBUG
	traceEvent(TRACE_INFO, "'%s'->'%s'", from, to);
#endif

	strncpy(info->sip_calling_party, from, MAX_SIP_STR_LEN);
	strncpy(info->sip_called_party, to, MAX_SIP_STR_LEN);
      }

      if(audio) {
	strtok_r(audio, " ", &strtokState);
	audio = strtok_r(NULL, " ", &strtokState);
#ifdef SIP_DEBUG
	traceEvent(TRACE_INFO, "RTP '%s:%s'", _intoa(*src, buf, sizeof(buf)), audio);
#endif
      
	if(cmpIpAddress(bkt->src, *src)) {
	  /* Direction: src -> dst */
	  if(audio)
	    info->rtp_src_port = atoi(audio);
	} else {
	  /* Direction: dst -> src */
	  if(audio) info->rtp_dst_port = atoi(audio);
	}
      }

      if(video) {
	strtok_r(video, " ", &strtokState);
	video = strtok_r(NULL, " ", &strtokState);
#ifdef SIP_DEBUG
	traceEvent(TRACE_INFO, "RTP '%s:%s'", _intoa(*src, buf, sizeof(buf)), video);
#endif
      }

      free(my_payload);
    } else
      traceEvent(TRACE_ERROR, "Not enough memory?");
  }
}

/* *********************************************** */

/* Handler called when the flow is deleted (after export) */

static void sipPlugin_delete(HashBucket* bkt, void *pluginData) {

  if(pluginData != NULL) {
    struct plugin_info *info = (struct plugin_info*)pluginData;

#ifdef SIP_DEBUG
    char buf[256], buf1[256];

    traceEvent(TRACE_INFO, "SIP: '%s'->'%s'", info->sip_calling_party, info->sip_called_party);
    traceEvent(TRACE_INFO, "RTP  '%s:%d'->'%s:%d'", 
	       _intoa(bkt->src, buf, sizeof(buf)), info->rtp_src_port,
	       _intoa(bkt->dst, buf1, sizeof(buf1)), info->rtp_dst_port);
#endif

    free(info);
  }
}

/* *********************************************** */

/* Handler called at startup when the template is read */

static V9TemplateId* sipPlugin_get_template(char* template_name) {
  int i;

  for(i=0; sipPlugin_template[i].templateId != 0; i++) {
    if(!strcmp(template_name, sipPlugin_template[i].templateName)) {
      return(&sipPlugin_template[i]);
    }
  }

  return(NULL); /* Unknown */
}

/* *********************************************** */

/* Handler called whenver a flow attribute needs to be exported */

static int sipPlugin_export(void *pluginData, V9TemplateId *theTemplate, 
			    int direction /* 0 = src->dst, 1 = dst->src */,
			    HashBucket *bkt, char *outBuffer,
			    u_int* outBufferBegin, u_int* outBufferMax) {
  int i;

  for(i=0; sipPlugin_template[i].templateId != 0; i++) {
    if(theTemplate->templateId == sipPlugin_template[i].templateId) {
      if((*outBufferBegin)+sipPlugin_template[i].templateLen > (*outBufferMax))
	return(-2); /* Too long */

      if(pluginData) {
	struct plugin_info *info = (struct plugin_info *)pluginData;

	switch(sipPlugin_template[i].templateId) {
	case BASE_ID:
	  copyLen((u_char*)info->sip_call_id, sipPlugin_template[i].templateLen,
		  outBuffer, outBufferBegin, outBufferMax);
	  if(traceMode) traceEvent(TRACE_INFO, "sip_call_id: %s", info->sip_call_id);
	  break;
	case BASE_ID+1:
	  copyLen((u_char*)info->sip_calling_party, sipPlugin_template[i].templateLen,
		  outBuffer, outBufferBegin, outBufferMax);
	  if(traceMode) traceEvent(TRACE_INFO, "sip_calling_party: %s", info->sip_calling_party);
	  break;
	case BASE_ID+2:
	  copyLen((u_char*)info->sip_called_party, sipPlugin_template[i].templateLen,
		  outBuffer, outBufferBegin, outBufferMax);
	  if(traceMode) traceEvent(TRACE_INFO, "sip_called_party: %s", info->sip_called_party);
	  break;
	case BASE_ID+3:
	  copyLen((u_char*)info->rtp_codecs, sipPlugin_template[i].templateLen,
		  outBuffer, outBufferBegin, outBufferMax);
	  if(traceMode) traceEvent(TRACE_INFO, "rtp_codecs: %s", info->rtp_codecs);
	  break;
	case BASE_ID+4:
	  copyInt32(msTimeDiff(info->sip_invite_time, initialSniffTime), 
		    outBuffer, outBufferBegin, outBufferMax);
	  if(traceMode) traceEvent(TRACE_INFO, "sip_invite_time: %u",
		     msTimeDiff(info->sip_invite_time, initialSniffTime));
	  break;
	case BASE_ID+5:
	  copyInt32(msTimeDiff(info->sip_trying_time, initialSniffTime), 
		    outBuffer, outBufferBegin, outBufferMax);
	  if(traceMode) traceEvent(TRACE_INFO, "sip_trying_time: %u",
		     msTimeDiff(info->sip_trying_time, initialSniffTime));
	  break;
	case BASE_ID+6:
	  copyInt32(msTimeDiff(info->sip_ringing_time, initialSniffTime),
		    outBuffer, outBufferBegin, outBufferMax);
	  if(traceMode) traceEvent(TRACE_INFO, "sip_ringing_time: %u", 
		     msTimeDiff(info->sip_ringing_time, initialSniffTime));
	  break;
	case BASE_ID+7:
	  copyInt32(msTimeDiff(info->sip_ok_time, initialSniffTime), 
		    outBuffer, outBufferBegin, outBufferMax);
	  if(traceMode) traceEvent(TRACE_INFO, "sip_ok_time: %u", 
		     msTimeDiff(info->sip_ok_time, initialSniffTime));
	  break;
	case BASE_ID+8:
	  copyInt32(msTimeDiff(info->sip_ack_time, initialSniffTime), 
		    outBuffer, outBufferBegin, outBufferMax);
	  if(traceMode) traceEvent(TRACE_INFO, "sip_ack_time: %u", 
		     msTimeDiff(info->sip_ack_time, initialSniffTime));
	  break;
	case BASE_ID+9:
	  copyInt16(direction == 0 ? info->rtp_src_port :
		    info->rtp_dst_port, outBuffer, outBufferBegin, outBufferMax);
	  if(traceMode) traceEvent(TRACE_INFO, "rtp_src_port: %d", info->rtp_src_port);
	  break;
	case BASE_ID+10:
	  copyInt16(direction == 0 ? info->rtp_dst_port : info->rtp_src_port,
		    outBuffer, outBufferBegin, outBufferMax);
	  if(traceMode) traceEvent(TRACE_INFO, "rtp_dst_port: %d", info->rtp_dst_port); 
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

static V9TemplateId* sipPlugin_conf() {
  return(sipPlugin_template);
}

/* *********************************************** */

/* Plugin entrypoint */
static PluginInfo sipPlugin = {
  "SIP",
  "0.2",
  "Handle SIP protocol",
  "L.Deri <deri@ntop.org>",
  0 /* not always enabled */, 1, /* enabled */
  sipPlugin_init,
  sipPlugin_conf,
  sipPlugin_delete,
  sipPlugin_packet,
  sipPlugin_get_template,
  sipPlugin_export
};


/* *********************************************** */

/* Plugin entry fctn */
#ifdef MAKE_STATIC_PLUGINS
PluginInfo* sipPluginEntryFctn(void)
#else
     PluginInfo* PluginEntryFctn(void)
#endif
{
  return(&sipPlugin);
}

