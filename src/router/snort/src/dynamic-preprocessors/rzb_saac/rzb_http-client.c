#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include <rzb_collection_api.h>
#include "rzb_http-client.h"
#include "rzb_debug.h"
#include "rzb_http-fileinfo.h"
#include "rzb_http-server.h"
#include "rzb_http.h"

#include "sf_snort_plugin_api.h"
#include "sfPolicyUserData.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcre.h>

typedef struct {
   pcre       *re;
   pcre_extra *pe;
} pcrestruct;

// Ensure the pcre enum lines up with the pcre strings array
enum { PCRE_EOH, PCRE_URL, PCRE_HOST, PCRE_COOKIE, PCRE_UA } http_pcre_enum;
#define NUM_HTTP_PCRES PCRE_UA+1
pcrestruct http_pcre_structs[NUM_HTTP_PCRES];
char *http_pcre_strings[] = {
                               "\\n\\r?\\n",
                               "^(GET|POST)\\s+([^\\s]+)\\s+HTTP/1\\.[01]\\s*$",
                               "^Host:\\s*([^\\r\\n]+)",
                               "^Cookie:\\s*([^\\r\\n]+)",
                               "^User-Agent:\\s*([^\\r\\n]+)"
                            };

int init_HTTP_PCREs(void) {
   const char *error;
   int erroffset;

   int i;

   for(i = 0; i < NUM_HTTP_PCRES; i++) {
//      /*DEBUGOUT((D_CLIENT | D_INFO),*/printf("Initializing PCRE %d: %s\n", i, http_pcre_strings[i]);//);

      http_pcre_structs[i].re = pcre_compile(http_pcre_strings[i], PCRE_CASELESS | PCRE_DOTALL | PCRE_MULTILINE, &error, &erroffset, NULL);

      if(http_pcre_structs[i].re == NULL) {
         printf("Failed to compile pcre regex %d (%s): %s\n", i, http_pcre_strings[i], error);
         return(-1);
      }

      http_pcre_structs[i].pe = pcre_study(http_pcre_structs[i].re, 0, &error);

      if(error != NULL) {
         printf("Failed to study pcre regex %d /%s/: %s\n", i, http_pcre_strings[i], error);
         return(-1);
      }
   }

   return 1;
}

// < 0 for error.  >= 0 for len of extracted string
int extractHTTPClientHeaderVal(const u_int8_t *buf, u_int32_t size, int pcreidx, int substringnum, char *valuebuf, int valuelen) {
   int result;
   int ovector[9];
   int ovecsize = 9;
   const char *tmpstring;

//   printf("Searching for pcre %d (%s)\n", pcreidx, http_pcre_strings[pcreidx]);

   result = pcre_exec(http_pcre_structs[pcreidx].re, http_pcre_structs[pcreidx].pe,
                      (const char *)buf, size, 0, 0, ovector, ovecsize);

   if(result < 0 || result == PCRE_ERROR_NOMATCH) {
//      printf("pcre not found\n");
      return(-1); // We need to find the URL or this isn't a valid request
   }

   if(valuebuf) {
      result = pcre_get_substring((const char *)buf, ovector, result, substringnum, &tmpstring);
      if(result < 0) {
//         printf("unable to extract substring\n");
         return(-2);
      }

      strncpy(valuebuf, tmpstring, valuelen);
      valuebuf[valuelen-1] = '\0';

      pcre_free_substring(tmpstring);
      return(strlen(valuebuf));
   }

   return(0);
}


int ParseClientRequest(const u_int8_t *payload, u_int32_t payload_size, WEB_ENTRY* webentry) {

   u_int32_t offset_eoh = 0;
   int result;

   DEBUGOUT((D_CLIENT | D_INFO), printf("ParseClientRequest enter\n"));

   if(payload == NULL) {
      DEBUGOUT(D_CRITICAL, printf("ParseClientRequest payload is NULL.  wtf.\n"));
      return(-1);
   }

   if(payload_size < 15) {
      return(-1);
   }

   // I get the sneaking suspicion that eventually I'm going to realize that I still
   // need to keep track of HEAD, OPTION, etc because some jackass is going to desynch me by
   // injecting such requests into the stream so when I receive file data it won't line up
   // correctly.  I really should just rob the code from http_inspect here.

   // Find the end of the HTTP headers
   // XXX This code is pretty useless here unless I get an offset for the end of headers
   result = extractHTTPClientHeaderVal(payload, payload_size, PCRE_EOH, 0, NULL, 0);
   offset_eoh = /*(result >= 0) ? result :*/ payload_size;

   // Get the URL
   result = extractHTTPClientHeaderVal(payload, offset_eoh, PCRE_URL, 2, webentry->url, sizeof(webentry->url));

   // We need a URL (also validates this is a valid request)
   if(result < 0) {
      printf("Unable to extract URL\n");
      return(-1);
   }

   // The remaining headers are optional (PCRE_HOST, PCRE_COOKIE, PCRE_UA)
   result = extractHTTPClientHeaderVal(payload, offset_eoh, PCRE_HOST, 1, webentry->host, sizeof(webentry->host));
   if(result < 0) {
//      printf("Unable to extract Host header\n");
      webentry->host[0] = '\0';
   }

   result = extractHTTPClientHeaderVal(payload, offset_eoh, PCRE_COOKIE, 1, webentry->cookie, sizeof(webentry->cookie));
   if(result < 0) {
//      printf("Unable to extract Cookie header\n");
      webentry->cookie[0] = '\0';
   }

   result = extractHTTPClientHeaderVal(payload, offset_eoh, PCRE_UA, 1, webentry->user_agent, sizeof(webentry->user_agent));
   if(result < 0) {
//      printf("Unable to extract User-Agent header\n");
      webentry->user_agent[0] = '\0';
   }

   return(1);
}

int ProcessFromClient(SFSnortPacket *sp) {
   RULEDATA *ruledata;

   WEB_ENTRY webentry;

   int result;
   FILEINFO *fileinfo;

   DEBUGOUT((D_CLIENT | D_INFO), printf("ProcessFromClient enter\n"));
   DEBUGOUT((D_PACKET | D_WARN), prettyprint(sp->payload, sp->payload_size));

   ruledata = _dpd.streamAPI->get_application_data(sp->stream_session_ptr, SAAC_HTTP);

   if(!ruledata) {
      DEBUGOUT((D_CLIENT | D_DEBUG), printf("ProcessFromClient: adding new rule data\n"));
      ruledata = calloc(1, sizeof(RULEDATA));
      if(!ruledata) {
         DEBUGOUT(D_CRITICAL, printf("ProcessFromClient: ruledata malloc failed\n"));
         return(-1);
      }

      _dpd.streamAPI->set_application_data(sp->stream_session_ptr, SAAC_HTTP, ruledata, &free);
      ruledata->sid = NRTSID;
      ruledata->streaminfoidx = INVALIDSTREAMIDX;
      ruledata->state = WAITINGFORRESPONSEHEADER;

   } else if(ruledata->sid != NRTSID) {
      DEBUGOUT(D_CRITICAL, printf("ProcessFromClient: Not our data! (sid %d/0x%08x)\n", ruledata->sid, ruledata->sid));
      return(-1);
   } else if(IsStreamIgnored(ruledata)) {
      DEBUGOUT((D_SERVER | D_WARN), printf("ProcessFromClient: stream is ignored\n"));
      return(-1);
   }

   fileinfo = calloc(1, sizeof(FILEINFO));

   // Set all counts and sizes to 0, all strings to empty, and pointers to NULL
   // memset(fileinfo, '\0', sizeof(FILEINFO));

   result = ParseClientRequest(sp->payload, sp->payload_size, &webentry);
   DEBUGOUT((D_CLIENT | D_INFO), printf("return from ParseClientRequest() was %d\n", result));

   if(result <= 0) {
      free(fileinfo);
      return(-1);
   }

   // Copy URL and Host header out of webentry into fileinfo
   snprintf(fileinfo->url, sizeof(fileinfo->url), "%s", webentry.url);
   fileinfo->url[sizeof(fileinfo->url) - 1] = 0;
   snprintf(fileinfo->hostname, sizeof(fileinfo->hostname), "%s", webentry.host);
   fileinfo->hostname[sizeof(fileinfo->hostname) - 1] = 0;

   // Now store what we know about this request
   fileinfo->saddr = sp->ip4_header->source;
   fileinfo->daddr = sp->ip4_header->destination;

   // Add address info to webentry
   webentry.src_ip.ip.ipv4 = sp->ip4_header->source;
   webentry.src_ip.family = AF_INET;
   webentry.dst_ip.ip.ipv4 = sp->ip4_header->destination;
   webentry.dst_ip.family = AF_INET;

   // Now send our webentry as an Intel Nugget!
   if(rzb_collection.sendWebTrack(&webentry) == R_FAIL) {
      printf("Failed to send web track info!\n");
      // Not making this fatal error
   }

   DEBUGOUT((D_CLIENT | D_DEBUG), DumpFileInfo(fileinfo));

   result = AddFileInfoListElem(ruledata, fileinfo);

   DEBUGOUT((D_CLIENT | D_INFO), printf("return from StoreFileData() was %d\n", result));

   if(result < 0) {
      DEBUGOUT(D_CRITICAL, printf("AddFileInfoListElem failed!\n"));
      free(fileinfo);
      return(-1);
   }

   DEBUGOUT((D_CLIENT | D_WARN), DumpFileInfoList(ruledata));

//   _dpd.alertAdd(GENERATOR_NRT, DST_PORT_MATCH,
//                 1, 0, 3, DST_PORT_MATCH_STR, 0);

   return(0);
}

