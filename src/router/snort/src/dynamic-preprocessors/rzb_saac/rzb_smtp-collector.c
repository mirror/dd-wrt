#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "sf_snort_packet.h"
#include "sf_dynamic_preprocessor.h"
#include "sfPolicyUserData.h"

#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <uuid/uuid.h>

#include <rzb_collection_api.h>
#include "rzb_smtp-collector.h"
#include "rzb_debug.h"

#define SAAC_SMTP 6825

#ifndef RULE_MATCH
   #define RULE_MATCH 1
#endif

#ifndef RULE_NOMATCH
   #define RULE_NOMATCH -1
#endif

#define SMTPDUMPERID 2525

#define DISPLAY_DEMO_OUTPUT

#define SMTPCAP_INITSIZE 30000
#define SMTPCAP_MAXSIZE 15000000

typedef struct {
   u_int32_t sid;
   u_int32_t totalsize;
   u_int32_t storedsize;
   u_int8_t *clientdata;
} smtpcapture;


void smtpdumper_freedata(smtpcapture *sessiondata) {

   //printf("SMTPDUMP smtpdumper_freedata enter\n");

   if(!sessiondata) {
      //printf("SMTPDUMP sessiondata is NULL!\n");
      return;
   }

   if(sessiondata->clientdata) {
      free(sessiondata->clientdata);
   } else {
      //printf("SMTPDUMP sessiondata->clientdata is NULL!\n");
   }

   free(sessiondata);
}

void smtpdumper_calldetection(void *dataptr) {

   BLOCK_META_DATA *mdata = NULL;

   smtpcapture *smtpcapturedata = (smtpcapture*)dataptr;

   //printf("SMTPDUMP smtpdumper_calldetection enter\n");

   if(!dataptr) {
      //printf("SMTPDUMP dataptr is NULL!\n");
      return;
   }

   if(smtpcapturedata->clientdata) {

//      printf("SMTPDUMP Calling sendData() with the following data (%d bytes):\n\n", ((smtpcapture*)(dataptr))->storedsize);
#ifdef DISPLAY_DEMO_OUTPUT
      prettyprint(smtpcapturedata->clientdata, smtpcapturedata->storedsize);
      printf("\n\n");
#endif

      mdata = calloc(1, sizeof(*mdata));
      if(mdata == NULL) return;

      // Fill in the required fields
      mdata->timestamp = (unsigned int)time(NULL);
      mdata->data = smtpcapturedata->clientdata;
      mdata->size = smtpcapturedata->storedsize;
//      mdata->src_ip = 0x01010101;
//      mdata->dst_ip = 0x02020202;
      mdata->ip_proto = 6;
      mdata->src_port = 25;
      mdata->dst_port = 8000;

      uuid_copy(mdata->datatype, MAIL_CAPTURE);

      rzb_collection.sendData(mdata);

   } else {
      //printf("SMTPDUMP dataptr->clientdata is NULL!\n");
   }

   //printf("SMTPDUMP Freeing session data\n");
   // Data is freed by sendData; we just need to clear out the rest of the structure.
   // We can accomplish this by setting clientdata to NULL so we don't do the doublefree
   smtpcapturedata->clientdata = NULL;
   smtpdumper_freedata(smtpcapturedata);

}


/* detection functions */
int smtpdumpereval(SFSnortPacket *sp) {
   const u_int8_t *cursor_normal, *end_of_payload = 0;
//   Packet *sp = (Packet *) p;

   smtpcapture *sessiondata = NULL;
   u_int8_t *tmpdataptr; // For realloc()s

   u_int32_t incoming_data_size = 0;

   //printf("SMTPDUMP smtpdumpereval enter\n");

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   sessiondata = _dpd.streamAPI->get_application_data(sp->stream_session_ptr, SAAC_SMTP);

   //printf("SMTPDUMP sessiondata = %p\n", sessiondata);

   if(sessiondata) {
      if(sessiondata->sid != SMTPDUMPERID) {
         printf("SMTPDUMP Someone else's data!\n");
         return RULE_NOMATCH;
      }

      if(sessiondata->storedsize >= SMTPCAP_MAXSIZE) {
         printf("SMTPDUMP Already have SMTPCAP_MAXSIZE(%d) bytes of data\n", SMTPCAP_MAXSIZE);
         return RULE_NOMATCH;
      }
   } else {

      sessiondata = (smtpcapture*)calloc(1, sizeof(smtpcapture));

      if(!sessiondata) {
         printf("SMTPDUMP sessiondata malloc failed!\n");
         return RULE_NOMATCH;
      }

      sessiondata->sid = SMTPDUMPERID;
      sessiondata->clientdata = (u_int8_t*)malloc(SMTPCAP_INITSIZE);

      if(!sessiondata->clientdata) {
         printf("SMTPDUMP sessiondata->clientdata malloc failed!\n");
         smtpdumper_freedata(sessiondata);
         return RULE_NOMATCH;
      }

      sessiondata->totalsize = SMTPCAP_INITSIZE;
      sessiondata->storedsize = 0;

      //printf("SMTPDUMP storing rule data\n");

      _dpd.streamAPI->set_application_data(sp->stream_session_ptr, SAAC_SMTP, sessiondata, &smtpdumper_calldetection);
      //printf("SMTPDUMP stored rule data\n");
   }

   cursor_normal = sp->payload;
   end_of_payload = sp->payload + sp->payload_size;

   incoming_data_size = sp->payload_size; //end_of_payload - cursor_normal;

   //printf("SMTPDUMP incoming_data_size = %d\n", incoming_data_size);

   // Check if we have enough room for the incoming data
   if(incoming_data_size > (sessiondata->totalsize - sessiondata->storedsize)) {
      // We've previously ensured we are not already overcapped on data

      //printf("SMTPDUMP reallocating to %d bytes\n", sessiondata->totalsize * 2);

      // Double our amount of storage
      tmpdataptr = realloc(sessiondata->clientdata, sessiondata->totalsize * 2);

      if(!tmpdataptr) {
         // If there is not enough available memory, realloc() returns a null pointer  and sets errno to [ENOMEM].
         if(errno == ENOMEM) {
            smtpdumper_freedata(sessiondata);
            return(RULE_NOMATCH);
         } else {
            printf("SMTPDUMP realloc() failed but I dunno wtf\n");
            smtpdumper_freedata(sessiondata);
            return(RULE_NOMATCH);
         }
      }

      sessiondata->clientdata = tmpdataptr;
      sessiondata->totalsize *= 2;

      //printf("SMTPDUMP totalsize is now %d\n", sessiondata->totalsize);
   }

   // We have enough room, so store the data
   //printf("SMTPDUMP storing %d bytes at %p\n", incoming_data_size, &((sessiondata->clientdata)[sessiondata->storedsize]));
   memcpy(&((sessiondata->clientdata)[sessiondata->storedsize]), cursor_normal, incoming_data_size);
   sessiondata->storedsize += incoming_data_size;
   //printf("SMTPDUMP stored size is now %d\n", sessiondata->storedsize);

   return RULE_NOMATCH;
}

