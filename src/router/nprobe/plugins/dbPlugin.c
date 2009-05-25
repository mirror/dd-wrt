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

/* #define DEBUG */

static V9TemplateId dbPlugin_template[] = {  
  /* Nothing to export into a template */
  { 0, 0, NULL, NULL }
};

/* *********************************************** */

static PluginInfo dbPlugin; /* Forward */

/* *********************************************** */

void dbPlugin_init(int argc, char *argv[]) {
  char *host = NULL, *user = NULL, *pw = NULL;
  int i, save = traceLevel;
  
  traceLevel = 10;
  
#ifdef HAVE_MYSQL

  traceEvent(TRACE_INFO, "Initializing DB plugin\n");

  for(i=0; i<argc; i++)
    if(strncmp(argv[i], MYSQL_OPT, strlen(MYSQL_OPT)) == 0) {
      char arg[256];

      snprintf(arg, sizeof(arg), "%s:", strdup(&argv[i][strlen(MYSQL_OPT)]));
      
      /* <host>:<user>:<pw> */            
      if((host = strtok(arg, ":")) != NULL)
	if((user = strtok(NULL, ":")) != NULL) {
	  pw = strtok(NULL, ":");
	  if(pw == NULL) pw = "";
	}

      if(host && user && pw) {
	traceEvent(TRACE_INFO, "Attemping to connect to database as [%s][%s][%s]", host, user, pw);
	init_database(host, user, pw, "nprobe");
      } else {
	traceEvent(TRACE_WARNING, "Bad format for --mysql=<host>:<user>:<pw> [%s][%s][%s]", host, user, pw);
	traceEvent(TRACE_WARNING, "Database support has been disabled.");
      }
    }
#else
  traceEvent(TRACE_WARNING, "DB support is not enabled (disabled at compile time)");
#endif
  
  traceLevel = save;
}

/* *********************************************** */

static void dbPlugin_packet(u_char new_bucket, void* pluginData,
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
  if(new_bucket) {
    PluginInformation *info;
    
    info = (PluginInformation*)malloc(sizeof(PluginInformation));
    if(info == NULL) {
      traceEvent(TRACE_ERROR, "Not enough memory?");
      return; /* Not enough memory */
    }

    info->pluginPtr  = (void*)&dbPlugin;
    info->pluginData = NULL;
    
    info->next = bkt->plugin;      
    bkt->plugin = info;

#ifdef DEBUG
    traceEvent(TRACE_INFO, "dbPlugin_create called.\n");
#endif
  }
}

/* *********************************************** */

static void dbPlugin_delete(HashBucket* bkt, void *pluginData) {
#ifdef DEBUG
  traceEvent(TRACE_INFO, "dbPlugin_delete called.");
#endif

  if(pluginData != NULL) {
    struct plugin_info *info = (struct plugin_info*)pluginData;
#ifdef DEBUG
    char buf[256], buf1[256];

    traceEvent(TRACE_INFO, "Flow [%s:%d -> %s:%d] terminated.\n",
	       _intoa(bkt->src, buf, sizeof(buf)), (int)bkt->sport,
	       _intoa(bkt->dst, buf1, sizeof(buf1)), (int)bkt->dport);
#endif

    free(info);
  }
}

/* *********************************************** */

static V9TemplateId* dbPlugin_get_template(char* template_name) {
  int i;

  for(i=0; dbPlugin_template[i].templateId != 0; i++) {
    if(!strcmp(template_name, dbPlugin_template[i].templateName)) {
      return(&dbPlugin_template[i]);
    }
  }

  return(NULL); /* Unknown */
}

/* *********************************************** */

static int dbPlugin_export(void *pluginData, V9TemplateId *theTemplate, int direction,
			   HashBucket *bkt, char *outBuffer,
			   u_int* outBufferBegin, u_int* outBufferMax) {

  // traceEvent(TRACE_ERROR, " +++ dbPlugin_export()");

  return(-1); /* Not handled */	  
}

/* *********************************************** */

static V9TemplateId* dbPlugin_conf() {
  return(dbPlugin_template);
}

/* *********************************************** */

static PluginInfo dbPlugin = {
  "db",
  "0.1",
  "Save flows into a database",
  "L.Deri <deri@ntop.org>",
  1 /* always enabled */, 1, /* enabled */
  dbPlugin_init,
  dbPlugin_conf,
  dbPlugin_delete,
  dbPlugin_packet,
  dbPlugin_get_template,
  dbPlugin_export
};

/* *********************************************** */

/* Plugin entry fctn */
#ifdef MAKE_STATIC_PLUGINS
PluginInfo* dbPluginEntryFctn(void)
#else
     PluginInfo* PluginEntryFctn(void)
#endif
{
  return(&dbPlugin);
}

