#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include <rzb_collection_api.h>
#include "rzb_http-client.h"
#include "rzb_http-server.h"
#include "rzb_debug.h"
#include "rzb_http-fileinfo.h"
#include "rzb_http.h"

#include "sf_snort_plugin_api.h"
#include "sfPolicyUserData.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>

int SkipToEndOfHTTPHeader(const u_int8_t **in_cursor, const u_int8_t *end_of_data) {
   const u_int8_t *cursor = *in_cursor;

   while(cursor < end_of_data) {
      while(cursor < end_of_data && *cursor++ != '\n');

      if(cursor[0] == '\r' && cursor[1] == '\n') {
         cursor += 2;
         *in_cursor = cursor;
         return(1);
      } else if(cursor[0] == '\n') {
         cursor++;
         *in_cursor = cursor;
         return(1);
      }
   }

   return(-1);
}


enum filereadstatus ReadFileData(const u_int8_t **in_cursor, const u_int8_t *end_of_data, FILEINFO *fileinfo) {
   const u_int8_t *cursor = *in_cursor;

   u_int32_t amounttoalloc;
   u_int32_t bytesavailable;

   const u_int8_t *end_of_file;
   u_int8_t *filedataptr;

   if(cursor >= end_of_data)
      return(ERROR);

   // Make sure we have somewhere to store our data
   if((fileinfo->filedata) == NULL) {
      // ZDNOTE Need to limit the amount of memory that will be allocated at a time.  This may involve some
      // ZDNOTE changes to the FILEINFO struct.
      // ZDNOTE amounttoalloc = (fileinfo->filesize < MAXFILEALLOCCHUNK) ? fileinfo->filesize : MAXFILEALLOCCHUNK;
      if(fileinfo->filesize > 100000000 /*ULONG_MAX*/) {
         // ZDNOTE this will also trip on files for which we don't have a Content-Length header
         DEBUGOUT((D_FILE | D_DEBUG), printf("ReadFileData filesize is >100M!  Bailing!\n"));
         return(ERROR);
      }

      amounttoalloc = fileinfo->filesize;

      fileinfo->filedata = calloc(amounttoalloc, 1);

      if((fileinfo->filedata) == NULL) {
         printf("ReadFileData unable to allocate file contents buffer!\n");
         return(ERROR);
      }

      fileinfo->amountstored = 0;
      fileinfo->bufferindex = 0;
   }

   end_of_file = cursor + (fileinfo->filesize - fileinfo->amountstored);
   if(end_of_file > end_of_data) {
      end_of_file = end_of_data;
   }

   bytesavailable = end_of_file - cursor;

   // ZDNOTE Need to verify there is enough space left in the buffer before copy

   filedataptr = &((fileinfo->filedata)[fileinfo->bufferindex]);

   while(cursor < end_of_file) {
      *filedataptr++ = *cursor++;
   }

   *in_cursor = cursor;
   fileinfo->amountstored += bytesavailable;
   fileinfo->bufferindex += bytesavailable; // ZDNOTE again, check buffer size

   DEBUGOUT((D_FILE | D_DEBUG), printf("Saved %d bytes. (%d/%d total)\n", bytesavailable, fileinfo->amountstored, fileinfo->filesize));

   if(fileinfo->amountstored == fileinfo->filesize)
      return(WAITINGFORRESPONSEHEADER);
   else if(fileinfo->amountstored < fileinfo->filesize)
      return(WAITINGFORDATA);
   else
      return(ERROR);
}


int CallDetectionFunction(FILEINFO *fileinfo) {

   BLOCK_META_DATA *mdata = NULL;
   const unsigned char *tmp;

   // Init the metadata structure
   if((mdata = calloc(1, sizeof(*mdata))) == NULL) {
      perror("Error allocating mdata\n");
      return -1;
   }

   // Fill in the required fields
   mdata->timestamp = (unsigned int)time(NULL);
   mdata->data = fileinfo->filedata;
   mdata->size = fileinfo->filesize;
   mdata->src_ip.ip.ipv4 = fileinfo->saddr;
   mdata->src_ip.family = AF_INET;
   mdata->dst_ip.ip.ipv4 = fileinfo->daddr;
   mdata->dst_ip.family = AF_INET;
   mdata->ip_proto = 6;
   mdata->src_port = 25;
   mdata->dst_port = 8000;
   tmp = rzb_collection.file_type_lookup(fileinfo->filedata, fileinfo->filesize);
   uuid_copy(mdata->datatype, tmp);

//   DEBUGOUT((D_DETECT | D_INFO), printf("CallDetectionFunction enter\n"));

   // ZDNOTE Dunno what to do, so we're just going to...
   printf("Calling detection function with following file information:\n");
   DumpFileInfo(fileinfo);

   rzb_collection.sendData(mdata);

   fileinfo->filedata = NULL;
   fileinfo->filesize = 0;

   return(0);
}


enum filereadstatus ProcessServerHeader(const u_int8_t **in_cursor, const u_int8_t *end_of_data, FILEINFO *fileinfo) {
   const u_int8_t *cursor = *in_cursor;

   if(cursor + 15 >= end_of_data) {
      DEBUGOUT(D_CRITICAL, printf("ProcessServerHeader not enough data!\n"));
      return(ERROR);
   }

   // Check for HTTP/1.[01] header
   if( (strncasecmp((const char *)cursor, "http/1.", 7) != 0) || (cursor[7] != '0' && cursor[7] != '1'))
   {
      DEBUGOUT(D_CRITICAL, printf("ProcessServerHeader not a valid HTTP version\n"));
      return(ERROR);
   }

   cursor += 8;

   while(cursor < end_of_data && *cursor == ' ')
      cursor++;

   if(cursor + 6 >= end_of_data) {
      DEBUGOUT(D_CRITICAL, printf("ProcessServerHeader not enough data 2!\n"));
      return(ERROR);
   }

   if( memcmp(cursor, "200", 3) != 0)
   {
//      DEBUGOUT((D_FILE | D_DEBUG), printf("Unhandled response code: %c%c%c%c%c\n", cursor[-2], cursor[-1], cursor[0], cursor[1], cursor[2]));
//      DEBUGOUT((D_SERVER | D_WARN), printf("ProcessServerHeader not 200 response (is %c%c%d)\n", *(cursor-3), *(cursor-2), *(cursor-1)));
      DEBUGOUT((D_SERVER | D_WARN), printf("ProcessServerHeader not 200 response (is %c%c%d)\n", *cursor, *(cursor+1), *(cursor+2)));
      *in_cursor = cursor;
      return(SERVERRETURNNOT200);   // ZDNOTE We really need to handle other codes to skip over data
   }

   cursor += 3;
   // ZDNOTE Don't know if it matters, but we're not caring about the response message

   // Now, we're going to see if we can find a Content-Length header.
   // By definition, it has to be at the start of a line.  So, we're just going
   // To look for newlines and every time we find one, see if we're now looking
   // at Content-Length:
   while(cursor < end_of_data) {
      while(cursor < end_of_data && *cursor++ != '\n'); // Find next newline

      // No Content-Length: header.
      if(cursor + 16 >= end_of_data) {
         DEBUGOUT((D_SERVER | D_EMERG), printf("No content-length header\n"));
         //SkipToEndOfHTTPHeader(&cursor, end_of_data);
         fileinfo->filesize = UINT_MAX;
         break; //return(WAITINGFORDATA); // ZDNOTE bug if header spans packets.  INHTTPHEADERS state??
      }
      if( strncasecmp((const char *)cursor, "content-length:", 15) == 0 )
      {
         cursor += 15;
         if(cursor + 10 <= end_of_data) {
            fileinfo->filesize = strtoul((char *)cursor, (char**)(&cursor), 10); // ignores preceeding whitespace
         }

         DEBUGOUT((D_SERVER | D_DEBUG), printf("Found content-length.  Filesize = %d\n", fileinfo->filesize));

         SkipToEndOfHTTPHeader(&cursor, end_of_data);
         break;

      } else if(cursor[0] == '\r' && cursor[1] == '\n') {
         cursor += 2;
         break;
      } else if(cursor[0] == '\n') {
         cursor++;
         break;
      }
   }

   *in_cursor = cursor;

   return(WAITINGFORDATA);
}


int ProcessFromServer(SFSnortPacket *sp) {
   RULEDATA *ruledata;

   int result;

   const u_int8_t *cursor = sp->payload;
   const u_int8_t *end_of_data;

   FILEINFO *currentfile;

//   u_int32_t remaining_data = 0;

   DEBUGOUT((D_SERVER | D_INFO), printf("ProcessFromServer enter\n"));
   DEBUGOUT((D_PACKET | D_WARN), prettyprint(sp->payload, sp->payload_size));

   ruledata = _dpd.streamAPI->get_application_data(sp->stream_session_ptr, SAAC_HTTP);

   if(!ruledata) {
      DEBUGOUT((D_SERVER | D_DEBUG), printf("ProcessFromServer: no rule data!\n"));
      return(-1);
   } else if(ruledata->sid != NRTSID) {
      DEBUGOUT((D_SERVER | D_WARN), printf("Not our data! (sid %d/0x%08x)\n", ruledata->sid, ruledata->sid));
      return(-1);
   } else if(IsStreamIgnored(ruledata)) {
      DEBUGOUT((D_SERVER | D_WARN), printf("ProcessFromServer: stream is ignored\n"));
      return(-1);
   }


   if(fileinfolist[ruledata->streaminfoidx] == NULL) {
      printf("Craptacular, the fileinfolist is NULL, ruledata->streaminfoidx = %d\n", ruledata->streaminfoidx);
      DEBUGOUT(D_CRITICAL, printf("ProcessFromServer fileinfolist[ruledata->streaminfoidx] is NULL!\n"));
      return(-1);
   }

   currentfile = (fileinfolist[ruledata->streaminfoidx])->fileinfo;

   if(currentfile == NULL) {
      DEBUGOUT(D_CRITICAL, printf("ProcessFromServer head fileinfo is NULL!\n"));
      return(-1);
   }

   cursor = sp->payload;
//   dataremaining = sp->dsize;
   end_of_data = sp->payload + sp->payload_size;

   while(cursor < end_of_data && !IsStreamIgnored(ruledata)) {
      switch(ruledata->state) {
         case WAITINGFORRESPONSEHEADER:
            // We're currently waiting for the server to answer our request
            // ProcessServerHeader moves the cursor to the beginning of the response body
            // ...unless the header bridges packets.  This will be a bug.  ZDNOTE
            result = ProcessServerHeader(&cursor, end_of_data, currentfile);

            DEBUGOUT((D_SERVER | D_INFO), printf("return from ProcessServerResponse() was %d\n", result));
            DEBUGOUT((D_SERVER | D_WARN), DumpFileInfo(currentfile));

            switch(result) {
               case WAITINGFORDATA:
                  // Successfully processed header, now waiting for data
                  ruledata->state = WAITINGFORDATA;
               break;

               case SERVERRETURNNOT200:
               case IGNORESTREAM:
               case ERROR:
               default:
                  DEBUGOUT(D_CRITICAL, printf("ProcessServerHeader() unhandled response code (%d)\n", result));
                  IgnoreStream(ruledata);
                  //cursor = end_of_data;
               break;
            }
         break;

         case WAITINGFORDATA:
            result = ReadFileData(&cursor, end_of_data, currentfile);

            switch(result) {
               case WAITINGFORDATA:
                  // Nothing's changed regarding state
               break;

               case WAITINGFORRESPONSEHEADER:

                  DEBUGOUT((D_DEBUG | D_SERVER), printf("WE HAVE A COMPLETE FILE! ruledata=%p, streaminfoidx=%d\n", ruledata, ruledata->streaminfoidx));
                  DEBUGOUT((D_DEBUG | D_SERVER), DumpFileInfoList(ruledata));

                  // This means we got all of our data.  Call the detection function.
                  CallDetectionFunction(currentfile);

                  // Get the current file off of the stack
                  PopFileInfo(ruledata);

                  // And grab the next file on the list
                  if(fileinfolist[ruledata->streaminfoidx])
                     currentfile = (fileinfolist[ruledata->streaminfoidx])->fileinfo;
                  else
                     currentfile = NULL; // ZDNOTE hm....

                  IgnoreStream(ruledata); // POC1 for now we're ignoring pipelining

                  //cursor = end_of_data;
                  //ruledata->state = IGNORESTREAM;
               break;

               default:
                  DEBUGOUT(D_CRITICAL, printf("ProcessFromServer Unhandled response from ReadFileData (%d)\n", result));
                  IgnoreStream(ruledata);
                  //cursor = end_of_data;
               break;
            }

            break;

         case SKIPTONEXTRESPONSE:
            // Read data, skipping until we find a server response.
            // We can totally cheat if we know a content length.
//         break;

         default:
            DEBUGOUT(D_CRITICAL, printf("ProcessFromServer Unhandled ruledate state (%d). Bailing.\n", ruledata->state));
            IgnoreStream(ruledata);
            //cursor = end_of_data;
         break;
      }
   }

//   _dpd.alertAdd(GENERATOR_NRT, DST_PORT_MATCH,
//                 1, 0, 3, DST_PORT_MATCH_STR, 0);

   if(IsStreamIgnored(ruledata))
      return(-1);
   else
      return(0);
}


// Partially debug / hackery, partially something we'll probably want to keep
void IgnoreStream(RULEDATA *ruledata) {

   if(ruledata == NULL)
      return;

   DEBUGOUT((D_DEBUG | D_SERVER), printf("Clearing streaminfoidx %d (%p)\n", ruledata->streaminfoidx, ruledata));

   // Set state to ignore and clear out the list
   ruledata->state = IGNORESTREAM;

   FreeFileInfoList(ruledata);

//   if(ruledata->streaminfoidx == INVALIDSTREAMIDX) {
//      DEBUGOUT((D_DEBUG | D_SERVER), printf("   INVALIDSTREAMIDX, exiting\n"));
//      return;
//   }
//
//   while(fileinfolist[ruledata->streaminfoidx]) {
//      DEBUGOUT((D_DEBUG | D_SERVER), printf("   popping %s\n", (fileinfolist[ruledata->streaminfoidx])->fileinfo->url));
//
//      DeleteFileInfoListHead(ruledata);
////      printf("ZDNOTE MEMORY LEAK! Setting pointer to NULL.\n");
////      fileinfolist[ruledata->streaminfoidx] = NULL;
//   }
//
//   ruledata->streaminfoidx = INVALIDSTREAMIDX;
}

int IsStreamIgnored(RULEDATA *ruledata) {
   if(ruledata == NULL || ruledata->state == IGNORESTREAM || ruledata->streaminfoidx == INVALIDSTREAMIDX)
      return(1);

   return(0);
}

