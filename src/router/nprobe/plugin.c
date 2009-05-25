/*
 *  Copyright (C) 2004-06 Luca Deri <deri@ntop.org>
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

/* *********************************************** */

static u_short numDeleteFlowFctn, numPacketFlowFctn;
static PluginInfo *plugins[MAX_NUM_PLUGINS+1] = { NULL };
static u_int num_active_plugins;

u_short num_plugins_enabled = 0;

#ifdef MAKE_STATIC_PLUGINS
extern PluginInfo* sipPluginEntryFctn(void);
extern PluginInfo* rtpPluginEntryFctn(void);
extern PluginInfo* dumpPluginEntryFctn(void);
/*
extern PluginInfo* l7PluginEntryFctn(void);
extern PluginInfo* classIdPluginEntryFctn(void);
*/
extern PluginInfo* httpPluginEntryFctn(void);
extern PluginInfo* smtpPluginEntryFctn(void);
#else
static char *pluginDirs[] = { "./plugins", NULL };
#endif

/* *********************************************** */

static void loadPlugin(char *dirName, char *pluginName);

/* *********************************************** */

void initPlugins(int argc, char* argv[]) {
  int i;
#ifndef MAKE_STATIC_PLUGINS
  int idp = 0;
#ifndef WIN32
  char dirPath[256];
  struct dirent* dp;
  DIR* directoryPointer=NULL;
#endif
#endif

  /* ******************************** */

  /* Register plugins */

  num_active_plugins = 0;
#ifdef MAKE_STATIC_PLUGINS
  traceEvent(TRACE_INFO, "Initializing static plugins.\n");

  loadPlugin(NULL, "sipPlugin");
  loadPlugin(NULL, "rtpPlugin");
  /* loadPlugin(NULL, "dumpPlugin");  */
  /*
  loadPlugin(NULL, "l7Plugin");
  loadPlugin(NULL, "classIdPlugin");
  */
  loadPlugin(NULL, "httpPlugin");
  loadPlugin(NULL, "smtpPlugin");
  loadPlugin(NULL, "classIdPlugin");
#else /* MAKE_STATIC_PLUGINS */
  traceEvent(TRACE_INFO, "Loading plugins.\n");

  for(idp = 0; pluginDirs[idp] != NULL; idp++ ){
    snprintf(dirPath, sizeof(dirPath), "%s", pluginDirs[idp]);
    directoryPointer = opendir(dirPath);

    if(directoryPointer != NULL)
      break;
  }

  if(directoryPointer == NULL) {
    traceEvent(TRACE_WARNING, "Unable to find plugins directory. nProbe will work without plugins!");
  } else {
    traceEvent(TRACE_INFO, "Looking for plugins in %s", dirPath);

    while((dp = readdir(directoryPointer)) != NULL) {
      if(dp->d_name[0] == '.')
	continue;
      else if(strcmp(&dp->d_name[strlen(dp->d_name)-strlen(".so")], ".so"))
	continue;
      loadPlugin(dirPath, dp->d_name);
    }

    closedir(directoryPointer);
  }

#endif /* MAKE_STATIC_PLUGINS */

  /* ******************************** */

  numDeleteFlowFctn = numPacketFlowFctn = 0;

  i=0;
  while(plugins[i] != NULL) {
    if(plugins[i]->enabled || plugins[i]->always_enabled) {
      traceEvent(TRACE_INFO, "-> %s", plugins[i]->name);
      if(plugins[i]->initFctn != NULL) plugins[i]->initFctn(argc, argv);
      if(plugins[i]->deleteFlowFctn != NULL) numDeleteFlowFctn++;
      if(plugins[i]->packetFlowFctn != NULL) numPacketFlowFctn++;
    }

    i++;
  }

  traceEvent(TRACE_INFO, "%d plugin(s) loaded [%d delete][%d packet].\n",
	     i, numDeleteFlowFctn, numPacketFlowFctn);
}

/* *********************************************** */

void termPlugins() {
  traceEvent(TRACE_INFO, "Terminate plugins.\n");
}

/* *********************************************** */

void dumpPluginHelp() {
  int i = 0;

  while(plugins[i] != NULL) {
   V9TemplateId *templates = plugins[i]->pluginFlowConf();

   if(templates && (templates[0].templateName != NULL)) {
     printf("\nPlugin %s templates:\n", plugins[i]->name);
     printTemplateInfo(templates);
   }

    i++;
  }
}

/* *********************************************** */

void pluginCallback(u_char callbackType, HashBucket* bkt,
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
  int i = 0;

  switch(callbackType) {
  case CREATE_FLOW_CALLBACK:
    while(plugins[i] != NULL) {
      switch(callbackType) {
      case CREATE_FLOW_CALLBACK:
	if((plugins[i]->enabled)
	   && (plugins[i]->packetFlowFctn != NULL)) {
	  plugins[i]->packetFlowFctn(1 /* new flow */,
				     NULL, bkt,
				     proto, isFragment,
				     numPkts, tos,
				     vlanId, ehdr,
				     src, sport,
				     dst, dport,
				     len, flags, icmpType,
				     numMplsLabels,
				     mplsLabels, fingerprint,
				     h, p, payload, payloadLen);
	}
	break;
      }

      i++;
    }
    break;

  case DELETE_FLOW_CALLBACK:
    if(bkt->plugin != NULL) {
      PluginInformation *plugin = bkt->plugin, *next;

      while(plugin != NULL) {
	if(plugin->pluginPtr->deleteFlowFctn != NULL) {
	  plugin->pluginPtr->deleteFlowFctn(bkt, plugin->pluginData);
	  next = plugin->next;
	  free(plugin);
	  bkt->plugin = next;
	  plugin = next;
	}
      }
    }
    break;

  case PACKET_CALLBACK:
    if(bkt->plugin != NULL) {
      PluginInformation *plugin = bkt->plugin;

      while(plugin != NULL) {
	if(plugin->pluginPtr->packetFlowFctn != NULL) {
	  plugin->pluginPtr->packetFlowFctn(0 /* existing flow */,
					    plugin->pluginData,
					    bkt,
					    proto, isFragment,
					    numPkts, tos,
					    vlanId, ehdr,
					    src, sport,
					    dst, dport,
					    len, flags, icmpType,
					    numMplsLabels,
					    mplsLabels, fingerprint,
					    h, p, payload, payloadLen);
	}

	plugin = plugin->next;
      }
    }
    break;

  default:
    return; /* Unknown callback */
  }
}

/* *********************************************** */

V9TemplateId* getPluginTemplate(char* template_name) {
  int i=0;

  while(plugins[i] != NULL) {
    if(plugins[i]->getPluginTemplateFctn != NULL) {
      V9TemplateId *rc = plugins[i]->getPluginTemplateFctn(template_name);

      if(rc != NULL) return(rc);
    }

    i++;
  }

  return(NULL); /* Unknown */
}

/* *********************************************** */

int checkPluginExport(V9TemplateId *theTemplate, /* Template being export */
		      int direction,             /* 0 = src->dst, 1 = dst->src */
		      HashBucket *bkt,           /* The flow bucket being export */
		      char *outBuffer,           /* Buffer where data will be exported */
		      u_int *outBufferBegin,     /* Index of the slot (0..outBufferMax) where data will be insert */
		      u_int *outBufferMax        /* Length of outBuffer */
		      ) {
  if(bkt->plugin != NULL) {
    PluginInformation *plugin = bkt->plugin;

    while(plugin != NULL) {
      if(plugin->pluginPtr->checkPluginExportFctn != NULL) {
	int rc = plugin->pluginPtr->checkPluginExportFctn(plugin->pluginData, theTemplate, direction, bkt,
							  outBuffer, outBufferBegin, outBufferMax);

	if(rc == 0) return(0);
      }

      plugin = plugin->next;
    }
  }

  return(-1); /* Not handled */
}

/* *********************************************** */

static void loadPlugin(char *dirName, char *pluginName){
    char pluginPath[256];
#ifndef WIN32
    void *pluginPtr = NULL;
    void *pluginEntryFctnPtr;
#endif
    PluginInfo* pluginInfo;
#ifndef WIN32
    PluginInfo* (*pluginJumpFunc)();
#endif

    snprintf(pluginPath, sizeof(pluginPath), "%s/%s", dirName != NULL ? dirName : ".", pluginName);

#ifndef MAKE_STATIC_PLUGINS

  pluginPtr = (void*)dlopen(pluginPath, RTLD_NOW /* RTLD_LAZY */); /* Load the library */

  if(pluginPtr == NULL) {
    traceEvent(TRACE_WARNING, "Unable to load plugin '%s'", pluginPath);
    traceEvent(TRACE_WARNING, "Message is '%s'", dlerror());
    return;
  } else
    traceEvent(TRACE_INFO, "Loaded '%s'", pluginPath);

  pluginEntryFctnPtr = (void*)dlsym(pluginPtr, "PluginEntryFctn");

  if(pluginEntryFctnPtr == NULL) {
#ifdef WIN32
    traceEvent(TRACE_WARNING, "Unable to locate plugin '%s' entry function [%li]",
           pluginPath, GetLastError());
#else
    traceEvent(TRACE_WARNING, "Unable to locate plugin '%s' entry function [%s]",
           pluginPath, dlerror());
#endif /* WIN32 */
    return;
  }

  pluginJumpFunc = (PluginInfo*(*)())pluginEntryFctnPtr;
  pluginInfo = pluginJumpFunc();

#else /* MAKE_STATIC_PLUGINS */

  if(strcmp(pluginName, "sipPlugin") == 0)
    pluginInfo = sipPluginEntryFctn();
  else if(strcmp(pluginName, "rtpPlugin") == 0)
    pluginInfo = rtpPluginEntryFctn();
  else if(strcmp(pluginName, "dumpPlugin") == 0)
    pluginInfo = dumpPluginEntryFctn();
  /*
 else if(strcmp(pluginName, "l7Plugin") == 0)
    pluginInfo = l7PluginEntryFctn();
  else if(strcmp(pluginName, "classIdPlugin") == 0)
    pluginInfo = classIdPluginEntryFctn();
  */
  else if(strcmp(pluginName, "httpPlugin") == 0)
    pluginInfo = httpPluginEntryFctn();
  else if(strcmp(pluginName, "smtpPlugin") == 0)
    pluginInfo = smtpPluginEntryFctn();
  else
    pluginInfo = NULL;

#endif /* MAKE_STATIC_PLUGINS */

  if (pluginInfo != NULL)
      plugins[num_active_plugins++] = pluginInfo; /* FIX : add PluginInfo to the list */
}

/* *************************** */

void enablePlugins() {
  int i = 0, found = 0;

  while(plugins[i] != NULL) {
    if(stringTemplate == NULL)
      found = 0;
    else {
      if(plugins[i]->enabled && (!plugins[i]->always_enabled)) {
	V9TemplateId *templates = plugins[i]->pluginFlowConf();

	found = 0;

	if(templates && (templates[0].templateName != NULL)) {
	  int j = 0;

	  while(templates[j].templateName != NULL) {
	    char *str;

	    if((str = strstr(stringTemplate, templates[j].templateName)) != NULL) {
	      found = 1;
	      break;
	    }

	    j++;
	  }
	}
      }
    }

    if((!found) && (!plugins[i]->always_enabled)) {
      traceEvent(TRACE_INFO, "Disabling plugin %s (v9 template is not using it)",
		 plugins[i]->name);
      plugins[i]->enabled = 0;
    } else {
      traceEvent(TRACE_INFO, "Enabling plugin %s", plugins[i]->name);
      plugins[i]->enabled = 1;
   }

    i++;
  }
}
