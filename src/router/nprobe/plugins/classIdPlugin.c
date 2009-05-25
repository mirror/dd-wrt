/*
 *  Copyright (C) 2006 Luca Deri <deri@ntop.org>
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

#include "pcre.h"

#define CLASSID_RULES_CONF   "classId_rules.conf"
#define CLASSID_GROUPS_CONF  "classId_groups.conf"
#define BASE_ID              168

static V9TemplateId class_id_plugin_template[] = {
  { BASE_ID,   2, "CLASS_ID", "Traffic Class Identifier" },
  { BASE_ID+1, 2, "RULE_ID",  "Traffic Rule Identifier" },
  { 0, 0, NULL, NULL }
};

/* *********************************************** */

struct rule_info {
  u_short class_id, rule_id;
  pcre *proto_regex;
  struct rule_info *next;
};

static struct rule_info *rule_root;
static u_int num_patterns;

struct plugin_info {
  struct rule_info *rule_ptr;
};

#define CONST_PATTERN_EXTENSION   ".pat"
#define MAX_BYTES_SENT            1024
#define MAX_BYTES_RCVD            1024

/* *********************************************** */

static PluginInfo classIdPlugin; /* Forward */

/* ******************************************* */

static char add_rule(u_short rule_id, u_short class_id, char *pattern) {
  struct rule_info *info = (struct rule_info*)malloc(sizeof(struct rule_info));
  const char *error;
  int erroffset, len;

  if(pattern[0] == '\"') pattern++;
  
  len = strlen(pattern);
  while(len > 0) {
    if(pattern[len-1] == '\"') {
      pattern[len-1] = '\0';
      len--;
    } else
      break;
  }

  //traceEvent(TRACE_WARNING, "[rule_id=%d][class_id=%d][%s]", rule_id, class_id, pattern);

  if(info == NULL) {
    traceEvent(TRACE_WARNING, "Not enough memory while loading pattern");
    return(0);
  }

  info->class_id = class_id;
  info->rule_id  = rule_id;
  info->proto_regex = pcre_compile(pattern,   /* the pattern */
				   0,          /* default options */
				   &error,     /* for error message */
				   &erroffset, /* for error offset */
				   NULL);      /* use default character tables */

  if(info->proto_regex == NULL) {
    traceEvent(TRACE_WARNING, "Invalid pattern (%s) specified [error=%d][offset=%d]", 
	       pattern, error, erroffset);
    free(info);
    return(0);
  } else
    traceEvent(TRACE_INFO, "Added pattern '%s' [rule_id=%d][class_id=%d]",
	       pattern, rule_id, class_id);

  info->next = rule_root;
  rule_root = info;
  return(1);
}

/* ******************************************* */

void classIdPlugin_init(int argc, char *argv[]) {
  FILE *class_id_rules_fd, *class_id_groups_fd;
  char rules_buffer[512], groups_buffer[512];

  traceEvent(TRACE_INFO, "Initialized ClassId plugin");

  rule_root = NULL;
  num_patterns = 0;

  class_id_rules_fd  = fopen(CLASSID_RULES_CONF, "r");
  class_id_groups_fd = fopen(CLASSID_GROUPS_CONF, "r");

  if(!class_id_rules_fd) {
    traceEvent(TRACE_ERROR, "Unable to read %s file", CLASSID_RULES_CONF);
    return;
  }

  if(!class_id_groups_fd) {
    traceEvent(TRACE_ERROR, "Unable to read %s file", CLASSID_GROUPS_CONF);
    return;
  }

  while((!feof(class_id_rules_fd))
	&& (fgets(rules_buffer, sizeof(rules_buffer), class_id_rules_fd) != NULL)) {
    if((rules_buffer[0] != '#')
       && (rules_buffer[0] != ' ') && (rules_buffer[0] != '\n')
       && (rules_buffer[0] != '\r') && (rules_buffer[0] != '\t')) {
      char *rules_id_str, *rules_pattern_str, found;

      rules_buffer[strlen(rules_buffer)-1] = '\0';

      rules_id_str = strtok(rules_buffer, "\t");
      if(rules_id_str) rules_pattern_str = strtok(NULL, "\t"); else rules_pattern_str = NULL;

      //traceEvent(TRACE_INFO, "[%s][%s]", rules_id_str, rules_pattern_str);

      if(rules_id_str && rules_pattern_str) {
	char *item = NULL, *groups_class_id = NULL, *groups_rule_id = NULL;
	u_short rule_id = atoi(rules_id_str);
	/* 1               1,2,3                   "All Webmail Users" */

	rewind(class_id_groups_fd);
	found = 0;
	while((!found) 
	      && (!feof(class_id_groups_fd))
	      && (fgets(groups_buffer, sizeof(groups_buffer), class_id_groups_fd) != NULL)) {
	  if((groups_buffer[0] != '#')
	     && (groups_buffer[0] != ' ') && (groups_buffer[0] != '\n')
	     && (groups_buffer[0] != '\r') && (groups_buffer[0] != '\t')) {
	    groups_class_id = strtok(groups_buffer, "\t");
	    if(groups_class_id) groups_rule_id = strtok(NULL, "\t"); else groups_rule_id = NULL;

	    if(groups_class_id && groups_class_id) {
	      // traceEvent(TRACE_INFO, "-> [%s][%s][%s][rule_id=%d]", groups_class_id, groups_class_id, groups_rule_id, rule_id);
	      item = strtok(groups_rule_id, ",");
	      while(item != NULL) {
		if(atoi(item) == rule_id) {
		  // traceEvent(TRACE_INFO, "+> [%s][rule_id=%d]", item, rule_id);
		  found = 1;
		  break;
		} else
		  item = strtok(NULL, ",");
	      }
	    }
	  }
	}

	if(item != NULL) {
	  u_int16_t class_id = atoi(groups_class_id);

	  traceEvent(TRACE_INFO, "Loading [rule_id=%d][class_id=%d][pattern='%s']",
		     rule_id, class_id, rules_pattern_str);
	  if(add_rule(rule_id, class_id, rules_pattern_str))
	    num_patterns++;
	} else {
	  traceEvent(TRACE_WARNING, "Unable to find class_id for rule_id %d [pattern='%s']",
		     rule_id, rules_pattern_str);
	}
      }
    }
  }

  fclose(class_id_groups_fd);
  fclose(class_id_rules_fd);

  traceEvent(TRACE_INFO, "Loaded %d patterns", num_patterns);
}

/* *********************************************** */

static struct rule_info* protocolMatch(u_char *payload, int payloadLen) {
  struct rule_info *scanner = rule_root;

  ///traceEvent(TRACE_INFO, "protocolMatch(%d)", payloadLen);

  while(scanner) {
    // traceEvent(TRACE_INFO, "protocolMatch(%s, %d)", scanner->rule_name, payloadLen);

    int rc = pcre_exec(scanner->proto_regex, /* the compiled pattern */
		       NULL,                 /* no extra data - we didn't study the pattern */
		       (char*)payload,       /* the subject string */
		       payloadLen,           /* the length of the subject */
		       0,                    /* start at offset 0 in the subject */
		       PCRE_PARTIAL,         /* default options */
		       NULL,                 /* output vector for substring information */
		       0);                   /* number of elements in the output vector */

    if(rc >= 0) {
      // traceEvent(TRACE_INFO, "Found match [class_id=%d][rule_id=%d]", scanner->class_id, scanner->rule_id);
      return(scanner);
    } else {
      //traceEvent(TRACE_ERROR, "pcre_exec returned %d [len=%d][%s]", rc, payloadLen, payload);
    }

    scanner = scanner->next;
  }

  return(NULL);
}

/* *********************************************** */

/* Handler called whenever an incoming packet is received */

static void classIdPlugin_packet(u_char new_bucket, void *pluginData,
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

  //traceEvent(TRACE_INFO, "classIdPlugin_packet(%d)", payloadLen);
  
  if(new_bucket) {
    info = (PluginInformation*)malloc(sizeof(PluginInformation));
    if(info == NULL) return; /* Not enough memory */

    info->pluginPtr  = (void*)&classIdPlugin;
    pluginData = info->pluginData = malloc(sizeof(struct plugin_info));

    if(info->pluginData == NULL) {
      free(info);
      return; /* Not enough memory */
    } else
      memset(info->pluginData, 0, sizeof(struct plugin_info));

    info->next = bkt->plugin;
    bkt->plugin = info;
  }

  pinfo = (struct plugin_info*)pluginData;

  if((payloadLen > 0)
     && (pinfo->rule_ptr == NULL)
     && (bkt->bytesSent < MAX_BYTES_SENT) && (bkt->bytesRcvd < MAX_BYTES_RCVD)) {
    struct rule_info *rule_ptr = protocolMatch(payload, payloadLen);

    if(rule_ptr != NULL) {
      // traceEvent(TRACE_INFO, "==> Found '%d' class id", class_id);
      pinfo->rule_ptr = rule_ptr;
    }
  }
}

/* *********************************************** */

/* Handler called when the flow is deleted (after export) */

static void classIdPlugin_delete(HashBucket* bkt, void *pluginData) {
  if(pluginData != NULL)
    free(pluginData);
}

/* *********************************************** *

/* Handler called at startup when the template is read */

static V9TemplateId* classIdPlugin_get_template(char* template_name) {
  int i;

  for(i=0; class_id_plugin_template[i].templateId != 0; i++) {
    if(!strcmp(template_name, class_id_plugin_template[i].templateName)) {
      return(&class_id_plugin_template[i]);
    }
  }

  return(NULL); /* Unknown */
}

/* *********************************************** */

/* Handler called whenever a flow attribute needs to be exported */

static int classIdPlugin_export(void *pluginData, V9TemplateId *theTemplate,
			   int direction /* 0 = src->dst, 1 = dst->src */,
			   HashBucket *bkt, char *outBuffer,
			   u_int* outBufferBegin, u_int* outBufferMax) {
  int i;

  for(i=0; class_id_plugin_template[i].templateId != 0; i++) {
    if(theTemplate->templateId == class_id_plugin_template[i].templateId) {
      if((*outBufferBegin)+class_id_plugin_template[i].templateLen > (*outBufferMax))
	return(-2); /* Too long */

      if(pluginData) {
	struct plugin_info *info = (struct plugin_info *)pluginData;
	u_int16_t val;
	
	switch(class_id_plugin_template[i].templateId) {
	case BASE_ID:
	  val = ((info != NULL) && (info->rule_ptr != NULL)) ? info->rule_ptr->class_id : 0;
	  //traceEvent(TRACE_INFO, "++ classId=%d", val);
	  copyInt16(val, outBuffer, outBufferBegin, outBufferMax);
	  break;
	case BASE_ID+1:
	  val = ((info != NULL) && (info->rule_ptr != NULL)) ? info->rule_ptr->rule_id : 0;
	  //traceEvent(TRACE_INFO, "++ ruleId=%d", val);
	  copyInt16(val, outBuffer, outBufferBegin, outBufferMax);
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

static V9TemplateId* classIdPlugin_conf() {
  return(class_id_plugin_template);
}

/* *********************************************** */

/* Plugin entry point */
static PluginInfo classIdPlugin = {
  "Netflow Class Id Implementation",
  "0.1",
  "Handle ClassId implementation",
  "L.Deri <deri@ntop.org>",
  0 /* not always enabled */, 1, /* enabled */
  classIdPlugin_init,
  classIdPlugin_conf,
  classIdPlugin_delete,
  classIdPlugin_packet,
  classIdPlugin_get_template,
  classIdPlugin_export
};

/* *********************************************** */

/* Plugin entry fctn */
#ifdef MAKE_STATIC_PLUGINS
PluginInfo* classIdPluginEntryFctn(void)
#else
     PluginInfo* PluginEntryFctn(void)
#endif
{
  return(&classIdPlugin);
}

