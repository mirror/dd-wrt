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
#include "sf_ip.h"

#include <stdio.h>
#include <stdlib.h>

int numfileinfostructsinuse = 0;

u_int32_t nextfreestreaminfoidx = 0;
struct FILEINFOLISTELEM *fileinfolist[NUMSTREAMSTOTRACK];

void DumpFileInfo(FILEINFO *fileinfo) {

   char srcaddr[INET_ADDRSTRLEN], dstaddr[INET_ADDRSTRLEN];

   if(fileinfo == NULL) {
      DEBUGOUT(D_CRITICAL, printf("DumpFileInfo fileinfo is NULL!\n"));
      return;
   }

   // snort typedefs inet_ntoa to sfip_ntoa.  We use inetaddrs.  wtf.
   sfip_raw_ntop(AF_INET, &fileinfo->saddr, srcaddr, sizeof(srcaddr));
   sfip_raw_ntop(AF_INET, &fileinfo->daddr, dstaddr, sizeof(dstaddr));

   printf("/--- fileinfo start ---\n");
   printf("| url: %s\n", fileinfo->url);
   printf("| hostname: %s\n", fileinfo->hostname);
   printf("| saddr: %s\n", srcaddr);
   printf("| daddr: %s\n", dstaddr);
   printf("| filesize: %d\n", fileinfo->filesize);
   printf("| amountstored: %d\n", fileinfo->amountstored);
   printf("| bufferindex: %d\n", fileinfo->bufferindex);
   printf("| filedata = %p\n", fileinfo->filedata);

   if(fileinfo->filedata != NULL) {
      DEBUGOUT((D_FILE | D_WARN), prettyprint(fileinfo->filedata, fileinfo->filesize));
#ifndef DEBUG
      prettyprint(fileinfo->filedata, (fileinfo->filesize > 256) ? 256 : fileinfo->filesize);
#endif
   }

   printf("\\--- fileinfo end   ---\n");
}


int DumpFileInfoList(RULEDATA *ruledata) {
   struct FILEINFOLISTELEM *tmp;

   printf("DumpFileInfoList, index %d\n", ruledata->streaminfoidx);
   if(ruledata->streaminfoidx == INVALIDSTREAMIDX) {
      printf("Invalid stream index!\n");
      return(-1);
   }

   tmp = fileinfolist[ruledata->streaminfoidx];

   if(tmp == NULL) {
      printf("Head node is NULL!\n");
      return(-1);
   }

   do {
      DumpFileInfo(tmp->fileinfo);
   } while((tmp = tmp->next));

   return(1);
}


void FreeFileInfo(FILEINFO *fileinfo) {
   if(fileinfo) {
      if(fileinfo->filedata) {
         //printf("Freeing file data 0x%08x\n", fileinfo->filedata);
         free(fileinfo->filedata);
         //fileinfo->filedata = NULL;
      }

      free(fileinfo);
      numfileinfostructsinuse--;
   }

//   printf("FreeFileInfo numfileinfostructsinuse=%d\n", numfileinfostructsinuse);
}

void FreeNRTStreamData(void *inptr) {
   RULEDATA *ruledata = (RULEDATA *)inptr;

   printf("Freeing NRT stream data.  Be afraid.  Be very afraid.\n");

   DEBUGOUT((D_CLIENT|D_SERVER|D_DEBUG), printf("FreeNRTStreamData enter\n"));

   if(!ruledata) {
      DEBUGOUT((D_SERVER|D_CLIENT|D_DEBUG), printf("   inptr is NULL, exiting\n"));
      return;
   }

   FreeFileInfoList(ruledata);

   free(ruledata);
}


void FreeFileInfoList(RULEDATA *ruledata) {

   DEBUGOUT((D_CLIENT|D_SERVER|D_DEBUG), printf("FreeFileInfoList enter\n"));

   if(!ruledata) {
      DEBUGOUT((D_SERVER|D_CLIENT|D_DEBUG), printf("   inptr is NULL, exiting\n"));
      return;
   }

   if(ruledata->streaminfoidx != INVALIDSTREAMIDX) {
      while(fileinfolist[ruledata->streaminfoidx]) {
         DEBUGOUT((D_DEBUG | D_SERVER), printf("   deleting %s\n", (fileinfolist[ruledata->streaminfoidx])->fileinfo->url));

         DeleteFileInfoListHead(ruledata);
      }
   }

   ruledata->streaminfoidx = INVALIDSTREAMIDX;
}

int AddFileInfoListElem(RULEDATA *ruledata, FILEINFO *fileinfo) {
   struct FILEINFOLISTELEM *tmp, *addme;

   int i;

   DEBUGOUT((D_FILE | D_INFO), printf("AddFileInfoListElem enter\n"));

   if(ruledata->streaminfoidx == INVALIDSTREAMIDX) {
      if(nextfreestreaminfoidx == OUTOFSTREAMINFOSTORAGE) {
         DEBUGOUT(D_CRITICAL, printf("out of stream storage!\n"));
         return(-1);
      }

      ruledata->streaminfoidx = nextfreestreaminfoidx;
      DEBUGOUT((D_FILE | D_DEBUG), printf("Using next open slot, at index %d\n", nextfreestreaminfoidx));

      // Now let's find the next open index
      i = nextfreestreaminfoidx + 1;
      while(i < NUMSTREAMSTOTRACK) {
         if(fileinfolist[i] == NULL)
            break;
         else
            i++;
      }

      if(i == NUMSTREAMSTOTRACK) {
         i = 0;
         while(i < nextfreestreaminfoidx) {
            if(fileinfolist[i] == NULL)
               break;
            else
               i++;
         }
      }

      // Out of additional storage
      if(i == ruledata->streaminfoidx)
      {
         printf("Out of streaminfo storage\n");
         nextfreestreaminfoidx = OUTOFSTREAMINFOSTORAGE;
      }else
         nextfreestreaminfoidx = i;

      DEBUGOUT((D_FILE | D_DEBUG), printf("nextfreestreaminfoidx = %d\n", nextfreestreaminfoidx));
   }

   DEBUGOUT((D_FILE | D_DEBUG), printf("adding fileinfo at index %d\n", ruledata->streaminfoidx));

   addme = calloc(1, sizeof(*addme));

   if(addme == NULL) {
      DEBUGOUT(D_CRITICAL, printf("Unable to allocate fileinfolistelem!\n"));
      return(-1);
   }

   addme->fileinfo = fileinfo;
   addme->next = '\0';

   tmp = fileinfolist[ruledata->streaminfoidx];

   if(tmp) {
      while(tmp->next) {
         tmp = tmp->next;
      }

      tmp->next = addme;
   } else {
      fileinfolist[ruledata->streaminfoidx] = addme;
   }

   numfileinfostructsinuse++;
//   printf("AddFileInfoListElem numfileinfostructsinuse=%d\n", numfileinfostructsinuse);

   return(1);
}


FILEINFO *PopFileInfo(RULEDATA *ruledata) {
   struct FILEINFOLISTELEM *tmp;
   FILEINFO *fileinfo;

   DEBUGOUT((D_FILE | D_INFO), printf("PopFileInfo enter\n"));

   if(ruledata->streaminfoidx == INVALIDSTREAMIDX) {
      DEBUGOUT(D_CRITICAL, printf("PopFileInfo streaminfoidx is INVALIDSTREAMIDX!\n"));
      return(NULL);
   }

   tmp = fileinfolist[ruledata->streaminfoidx];

   if(tmp == NULL) {
      DEBUGOUT(D_CRITICAL, printf("PopFileInfo fileinfolist entry is NULL!\n"));
      return(NULL);
   }

   // Change the head
   fileinfolist[ruledata->streaminfoidx] = tmp->next;

   // Grab the fileinfo and free the container
   fileinfo = tmp->fileinfo;
   free(tmp);

   DEBUGOUT((D_CLIENT | D_SERVER | D_INFO), printf("PopFileInfo freed fileinfo container at %p\n", tmp));

   return(fileinfo);
}


int DeleteFileInfoListHead(RULEDATA *ruledata) {
   FILEINFO *fileinfo;

   DEBUGOUT((D_FILE | D_INFO), printf("DeleteFileInfoListHead enter\n"));

   fileinfo = PopFileInfo(ruledata);

   DEBUGOUT((D_CLIENT | D_SERVER | D_INFO), printf("freeing fileinfo at %p\n", fileinfo));

   if(fileinfo == NULL)
      return(-1);

   FreeFileInfo(fileinfo);

   return(1);
}



