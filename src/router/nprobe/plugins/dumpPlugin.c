/*
 *  Copyright (C) 2004-05 Luca Deri <deri@ntop.org>
 *
 *  		          http://www.ntop.org/
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

#define PATH_LEN      256
#define BASE_ID       100
#define BASE_PATH  "/tmp"

#if !defined(WIN32)
#define CONST_PATH_SEP                    '/'
#else
#define CONST_PATH_SEP                    '\\'
#endif

struct plugin_info {
  FILE *fd;
  char *file_path;
};

static V9TemplateId dumpPlugin_template[] = {
  { BASE_ID, PATH_LEN, "DUMP_PATH", "Path where dumps will be saved" },
  { 0, 0, NULL, NULL }
};

/* *********************************************** */

static PluginInfo dumpPlugin; /* Forward */

/* ******************************************* */

void mkdir_p(char *path) {
  int i, rc;

  if(path == NULL) {
    traceEvent(TRACE_WARNING, "mkdir(null) skipped");
    return;
  }

#ifdef WIN32
  revertSlash(path, 0);
#endif

  /* Start at 1 to skip the root */
  for(i=1; path[i] != '\0'; i++)
    if(path[i] == CONST_PATH_SEP) {
      path[i] = '\0';
      rc = mkdir(path, 0755 /* everyone */);
      path[i] = CONST_PATH_SEP;
    }

  mkdir(path, 0755 /* everyone */);
}

/* *********************************************** */

void dumpPlugin_init(int argc, char *argv[]) {
  traceEvent(TRACE_INFO, "Initialized dump plugin\n");
}

/* *********************************************** */

static void dumpPlugin_packet(u_char new_bucket, void* pluginData,
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
    /* The file has not yet been created */
    char buf[32], buf1[32], filePath[PATH_LEN], dirPath[PATH_LEN];
    time_t now = time(NULL);
    FILE *fd;
    struct tm t;
    char *prefix;
    
    info = (PluginInformation*)malloc(sizeof(PluginInformation));
    if(info == NULL) {
      traceEvent(TRACE_ERROR, "Not enough memory?");
      return; /* Not enough memory */
    }

    info->pluginPtr  = (void*)&dumpPlugin;
    pluginData = info->pluginData = malloc(sizeof(struct plugin_info));
    
    if(info->pluginData == NULL) {	
      traceEvent(TRACE_ERROR, "Not enough memory?");
      free(info);
      return; /* Not enough memory */
    }

    info->next = bkt->plugin;      
    bkt->plugin = info;

#ifdef DEBUG
    traceEvent(TRACE_INFO, "dumpPlugin_create called.\n");
#endif

    strftime(dirPath, sizeof(dirPath), "/%G/%b/%e/%H/%M/", localtime_r(&now, &t));

    if(     (bkt->sport == 25)  || (bkt->dport == 25)) prefix = "smtp";
    else if((bkt->sport == 110) || (bkt->dport == 110)) prefix = "pop";
    else if((bkt->sport == 143) || (bkt->dport == 143)) prefix = "imap";
    else if((bkt->sport == 220) || (bkt->dport == 220)) prefix = "imap3";
    else prefix = "data";

    snprintf(filePath, sizeof(filePath), "%s/%s/%s:%d_%s:%d-%u.%s",
	     BASE_PATH, dirPath,
	     _intoa(bkt->src, buf, sizeof(buf)), (int)bkt->sport,
	     _intoa(bkt->dst, buf1, sizeof(buf1)), (int)bkt->dport,
	     (unsigned int)now, prefix);

    fd = fopen(filePath, "w+");

    if(fd == NULL) {
      char fullPath[256];

      /* Maybe the directory has not been created yet */
      snprintf(fullPath, sizeof(fullPath), "%s/%s", BASE_PATH, dirPath);
      mkdir_p(fullPath);

      fd = fopen(filePath, "w+");
    }

    if(fd != NULL) {
      struct plugin_info* infos = (struct plugin_info*)pluginData;

#ifdef DEBUG
      traceEvent(TRACE_INFO, "Saving flow into %s", filePath);
#endif
      infos->fd = fd;
      infos->file_path = strdup(filePath);
    }
  }
  
  if((payload == NULL) || (payloadLen == 0)) return; /* Nothing to save */

  if(pluginData != NULL)
    (void)fwrite(payload, payloadLen, 1, ((struct plugin_info *)pluginData)->fd);
}

/* *********************************************** */

static void dumpPlugin_delete(HashBucket* bkt, void *pluginData) {
#ifdef DEBUG
  traceEvent(TRACE_INFO, "dumpPlugin_delete called.\n");
#endif

  if(pluginData != NULL) {
    struct plugin_info *info = (struct plugin_info*)pluginData;
#ifdef DEBUG
    char buf[256], buf1[256];

    traceEvent(TRACE_INFO, "Flow [%s:%d -> %s:%d] terminated.\n",
	       _intoa(bkt->src, buf, sizeof(buf)), (int)bkt->sport,
	       _intoa(bkt->dst, buf1, sizeof(buf1)), (int)bkt->dport);
#endif

    fclose(info->fd);
    if(info->file_path != NULL) free(info->file_path);
    free(info);
  }
}

/* *********************************************** */

static V9TemplateId* dumpPlugin_get_template(char* template_name) {
    int i;

    for(i=0; dumpPlugin_template[i].templateId != 0; i++) {
    if(!strcmp(template_name, dumpPlugin_template[i].templateName)) {
      return(&dumpPlugin_template[i]);
    }
  }

  return(NULL); /* Unknown */
}

/* *********************************************** */

static int dumpPlugin_export(void *pluginData, V9TemplateId *theTemplate, int direction,
			     HashBucket *bkt, char *outBuffer,
			     u_int* outBufferBegin, u_int* outBufferMax) {
  int i;

  for(i=0; dumpPlugin_template[i].templateId != 0; i++) {
    if(theTemplate->templateId == dumpPlugin_template[i].templateId) {
      if((*outBufferBegin)+dumpPlugin_template[i].templateLen > (*outBufferMax))
	return(-2); /* Too long */

      if(pluginData) {
	struct plugin_info *info = (struct plugin_info *)pluginData;

	switch(dumpPlugin_template[i].templateId) {
	case BASE_ID:
	  copyLen((u_char*)info->file_path, dumpPlugin_template[i].templateLen,
		  outBuffer, outBufferBegin, outBufferMax);
	  traceEvent(TRACE_INFO, "file_path: %s", info->file_path);
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

static V9TemplateId* dumpPlugin_conf() {
  return(dumpPlugin_template);
}

/* *********************************************** */

static PluginInfo dumpPlugin = {
  "dump",
  "0.1",
  "save flows into files",
  "L.Deri <deri@ntop.org>",
  0 /* not always enabled */, 1, /* enabled */
  dumpPlugin_init,
  dumpPlugin_conf,
  dumpPlugin_delete,
  dumpPlugin_packet,
  dumpPlugin_get_template,
  dumpPlugin_export
};

/* *********************************************** */

/* Plugin entry fctn */
#ifdef MAKE_STATIC_PLUGINS
PluginInfo* dumpPluginEntryFctn(void)
#else
     PluginInfo* PluginEntryFctn(void)
#endif
{
  return(&dumpPlugin);
}

