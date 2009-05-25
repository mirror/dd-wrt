/*
 *  Copyright (C) 2006 Luca Deri <deri@ntop.org>
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

#define BASE_ID           180
#define URL_MAX_LEN        32

#define MAX_BYTES_SENT     64
#define MAX_BYTES_RCVD     64

/* RFC 2616 */
/* Request */
#define GET_URL            "GET /"
#define POST_URL           "POST /"
#define HEAD_URL           "HEAD /"
/* Note the other methods (PUT, DELETE, TRACE, CONNECT)
   have not been implemented as they are used very seldom */

/* Response */
#define HTTP_1_0_URL       "HTTP/1.0 "
#define HTTP_1_1_URL       "HTTP/1.1 "

#ifndef min
#define min(a ,b) ((a > b) ? b: a)
#endif

static V9TemplateId httpPlugin_template[] = {
  { BASE_ID,   URL_MAX_LEN, "HTTP_URL", "HTTP URL" },
  { BASE_ID+1, 2, "HTTP_RET_CODE", "HTTP return code (e.g. 200, 304...)" },
  { 0, 0, NULL, NULL }
};

struct plugin_info {
  char http_url[URL_MAX_LEN+1];
  u_int16_t ret_code;
};

/* *********************************************** */

static PluginInfo httpPlugin; /* Forward */

/* ******************************************* */

void httpPlugin_init(int argc, char *argv[]) {
  traceEvent(TRACE_INFO, "Initialized HTTP plugin");
}

/* *********************************************** */

/* Handler called whenever an incoming packet is received */

static void httpPlugin_packet(u_char new_bucket, void *pluginData,
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
  PluginInformation *info;
  struct plugin_info *pinfo;

  // traceEvent(TRACE_INFO, "httpPlugin_packet(%d)", payloadLen);

  if(new_bucket) {
    info = (PluginInformation*)malloc(sizeof(PluginInformation));
    if(info == NULL) {
      	traceEvent(TRACE_ERROR, "Not enough memory?");
	return; /* Not enough memory */
    }

    info->pluginPtr  = (void*)&httpPlugin;
    pluginData = info->pluginData = malloc(sizeof(struct plugin_info));

    if(info->pluginData == NULL) {
      traceEvent(TRACE_ERROR, "Not enough memory?");
      free(info);
      return; /* Not enough memory */
    } else
      memset(info->pluginData, 0, sizeof(struct plugin_info));

    info->next = bkt->plugin;
    bkt->plugin = info;
  }

  pinfo = (struct plugin_info*)pluginData;

  if(payloadLen > 0) {
    char *method;
    
    //traceEvent(TRACE_INFO, "==> [%d][%d]'%s'", bkt->bytesSent, bkt->bytesRcvd, payload);

    if((!strncmp((char*)payload, GET_URL, strlen(GET_URL)))) method = GET_URL;
    else if((!strncmp((char*)payload, POST_URL, strlen(POST_URL)))) method = POST_URL;
    else if((!strncmp((char*)payload, HTTP_1_0_URL, strlen(HTTP_1_0_URL)))) method = HTTP_1_0_URL;
    else if((!strncmp((char*)payload, HTTP_1_1_URL, strlen(HTTP_1_1_URL)))) method = HTTP_1_1_URL;
    else method = NULL;

    if(method) {
      char url[URL_MAX_LEN+1];
      int i, displ;
      
      if((method == GET_URL) || (method == POST_URL)) {
	/* We need to export this flow now */
	if(pinfo->http_url[0] != '\0') {
	  exportBucket(bkt, 0);
	  resetBucketStats(bkt, h, len, sport, dport, payload, payloadLen);
	  memset(pinfo, 0, sizeof(struct plugin_info));	  
	}

	displ = 1;
      } else
	displ = 0;

      strncpy(url, (char*)&payload[strlen(method)-displ], 
	      min(URL_MAX_LEN, (payloadLen-(strlen(method)-displ))));

      url[URL_MAX_LEN] = '\0';
      for(i=0; i<URL_MAX_LEN; i++) 
	if((url[i] == ' ') 
	   || (url[i] == '\r')
	   || (url[i] == '\n')) {
	  url[i] = '\0';
	  break;
	}

      if(displ == 1)
	memcpy(pinfo->http_url, url, strlen(url));
      else
	pinfo->ret_code = atoi(url);
    }
  }
}

/* *********************************************** */

/* Handler called when the flow is deleted (after export) */

static void httpPlugin_delete(HashBucket* bkt, void *pluginData) {
  if(pluginData != NULL)
    free(pluginData);
}

/* *********************************************** *
   
/* Handler called at startup when the template is read */

static V9TemplateId* httpPlugin_get_template(char* template_name) {
  int i;

  for(i=0; httpPlugin_template[i].templateId != 0; i++) {
    if(!strcmp(template_name, httpPlugin_template[i].templateName)) {
      return(&httpPlugin_template[i]);
    }
  }

  return(NULL); /* Unknown */
}

/* *********************************************** */

/* Handler called whenever a flow attribute needs to be exported */

static int httpPlugin_export(void *pluginData, V9TemplateId *theTemplate,
			     int direction /* 0 = src->dst, 1 = dst->src */,
			     HashBucket *bkt, char *outBuffer,
			     u_int* outBufferBegin, u_int* outBufferMax) {
  int i;

  for(i=0; httpPlugin_template[i].templateId != 0; i++) {
    if(theTemplate->templateId == httpPlugin_template[i].templateId) {
      if((*outBufferBegin)+httpPlugin_template[i].templateLen > (*outBufferMax))
	return(-2); /* Too long */

      if(pluginData) {
	struct plugin_info *info = (struct plugin_info *)pluginData;

	switch(httpPlugin_template[i].templateId) {
	case BASE_ID:
	  memcpy(&outBuffer[*outBufferBegin], info->http_url, URL_MAX_LEN);
	  // traceEvent(TRACE_INFO, "==> URL='%s'", info->http_url);
	  break;
	case BASE_ID+1:
	  copyInt16(info->ret_code, outBuffer, outBufferBegin, outBufferMax);
	  // traceEvent(TRACE_INFO, "==> RetCode='%d'", info->ret_code);
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

static V9TemplateId* httpPlugin_conf() {
  return(httpPlugin_template);
}

/* *********************************************** */

/* Plugin entrypoint */
static PluginInfo httpPlugin = {
  "HTTP Protocol Dissector",
  "0.1",
  "Handle HTTP protocol",
  "L.Deri <deri@ntop.org>",
  0 /* not always enabled */, 1, /* enabled */
  httpPlugin_init,
  httpPlugin_conf,
  httpPlugin_delete,
  httpPlugin_packet,
  httpPlugin_get_template,
  httpPlugin_export
};

/* *********************************************** */

/* Plugin entry fctn */
#ifdef MAKE_STATIC_PLUGINS
PluginInfo* httpPluginEntryFctn(void)
#else
  PluginInfo* PluginEntryFctn(void)
#endif
{
  return(&httpPlugin);
}
 
