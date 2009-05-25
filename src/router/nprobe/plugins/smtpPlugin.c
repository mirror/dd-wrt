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

#define BASE_ID           185
#define ADDRESS_MAX_LEN    32

#define MAIL_FROM         "MAIL From:<"
#define RCPT_TO           "RCPT To:<"
#define RESET             "RESET"

#ifndef min
#define min(a ,b) ((a > b) ? b: a)
#endif

static V9TemplateId smtpPlugin_template[] = {
  { BASE_ID,   ADDRESS_MAX_LEN, "SMTP_MAIL_FROM", "Mail sender" },
  { BASE_ID+1, 2, "SMTP_RCPT_TO", "Mail recipient" },
  { 0, 0, NULL, NULL }
};

struct plugin_info {
  char mail_from[ADDRESS_MAX_LEN+1];
  char rcpt_to[ADDRESS_MAX_LEN+1];
};

/* *********************************************** */

static PluginInfo smtpPlugin; /* Forward */

/* ******************************************* */

void smtpPlugin_init(int argc, char *argv[]) {
  traceEvent(TRACE_INFO, "Initialized SMTP plugin");
}

/* *********************************************** */

/* Handler called whenever an incoming packet is received */

static void smtpPlugin_packet(u_char new_bucket, void *pluginData,
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

  // traceEvent(TRACE_INFO, "smtpPlugin_packet(%d)", payloadLen);

  if(new_bucket) {
    info = (PluginInformation*)malloc(sizeof(PluginInformation));
    if(info == NULL) {
      traceEvent(TRACE_ERROR, "Not enough memory?");
      return; /* Not enough memory */
    }

    info->pluginPtr  = (void*)&smtpPlugin;
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

    if((!strncasecmp((char*)payload, MAIL_FROM, strlen(MAIL_FROM)))) method = MAIL_FROM;
    else if((!strncasecmp((char*)payload, RCPT_TO, strlen(RCPT_TO)))) method = RCPT_TO;
    else if((!strncasecmp((char*)payload, RESET, strlen(RESET)))) method = RESET;
    else method = NULL;

    if(method) {
      char address[ADDRESS_MAX_LEN+1];
      int i;
      
      if(method == RESET) {
	/* We need to export this flow now */
	exportBucket(bkt, 0);
	resetBucketStats(bkt, h, len, sport, dport, payload, payloadLen);
	memset(pinfo, 0, sizeof(struct plugin_info));	  
	return;
      }
      
      strncpy(address, (char*)&payload[strlen(method)-1], 
	      min(ADDRESS_MAX_LEN, (payloadLen-(strlen(method)-1))));

      address[ADDRESS_MAX_LEN] = '\0';
      for(i=0; i<ADDRESS_MAX_LEN; i++) 
	if((address[i] == ' ') 
	   || (address[i] == '\r')
	   || (address[i] == '\n')) {
	  address[i] = '\0';
	  break;
	}

      if(method == MAIL_FROM)
	memcpy(pinfo->mail_from, address, strlen(address));
      else if(method == RCPT_TO)
	memcpy(pinfo->rcpt_to, address, strlen(address));
    }
  }
}

/* *********************************************** */

/* Handler called when the flow is deleted (after export) */

static void smtpPlugin_delete(HashBucket* bkt, void *pluginData) {
  if(pluginData != NULL)
    free(pluginData);
}

/* *********************************************** *
   
/* Handler called at startup when the template is read */

static V9TemplateId* smtpPlugin_get_template(char* template_name) {
  int i;

  for(i=0; smtpPlugin_template[i].templateId != 0; i++) {
    if(!strcmp(template_name, smtpPlugin_template[i].templateName)) {
      return(&smtpPlugin_template[i]);
    }
  }

  return(NULL); /* Unknown */
}

/* *********************************************** */

/* Handler called whenever a flow attribute needs to be exported */

static int smtpPlugin_export(void *pluginData, V9TemplateId *theTemplate,
			     int direction /* 0 = src->dst, 1 = dst->src */,
			     HashBucket *bkt, char *outBuffer,
			     u_int* outBufferBegin, u_int* outBufferMax) {
  int i;

  for(i=0; smtpPlugin_template[i].templateId != 0; i++) {
    if(theTemplate->templateId == smtpPlugin_template[i].templateId) {
      if((*outBufferBegin)+smtpPlugin_template[i].templateLen > (*outBufferMax))
	return(-2); /* Too long */

      if(pluginData) {
	struct plugin_info *info = (struct plugin_info *)pluginData;

	switch(smtpPlugin_template[i].templateId) {
	case BASE_ID:
	  memcpy(&outBuffer[*outBufferBegin], info->mail_from, ADDRESS_MAX_LEN);
	  // traceEvent(TRACE_INFO, "==> MAIL_FROM='%s'", info->mail_from);
	  break;
	case BASE_ID+1:
	  memcpy(&outBuffer[*outBufferBegin], info->rcpt_to, ADDRESS_MAX_LEN);
	  // traceEvent(TRACE_INFO, "==> RCPT_TO='%s'", info->rcpt_to);
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

static V9TemplateId* smtpPlugin_conf() {
  return(smtpPlugin_template);
}

/* *********************************************** */

/* Plugin entrypoint */
static PluginInfo smtpPlugin = {
  "SMTP Protocol Dissector",
  "0.1",
  "Handle SMTP protocol",
  "L.Deri <deri@ntop.org>",
  0 /* not always enabled */, 1, /* enabled */
  smtpPlugin_init,
  smtpPlugin_conf,
  smtpPlugin_delete,
  smtpPlugin_packet,
  smtpPlugin_get_template,
  smtpPlugin_export
};

/* *********************************************** */

/* Plugin entry fctn */
#ifdef MAKE_STATIC_PLUGINS
PluginInfo* smtpPluginEntryFctn(void)
#else
  PluginInfo* PluginEntryFctn(void)
#endif
{
  return(&smtpPlugin);
}
 
