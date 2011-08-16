#ifndef NRT_FILEINFO_H
#define NRT_FILEINFO_H

#include <rzb_collection_api.h>
#include "rzb_http.h"

enum filereadstatus { ERROR = -1, WAITINGFORRESPONSEHEADER = 1, SERVERRETURNNOT200, SKIPTONEXTRESPONSE, WAITINGFORDATA, IGNORESTREAM };

typedef struct _RULEDATA {
   u_int32_t sid;
   u_int32_t streaminfoidx;
   enum filereadstatus state;
} RULEDATA;

typedef struct _FILEINFO {
   char url[URLLEN];
   char hostname[HOSTNAMELEN];
   struct in_addr saddr;
   struct in_addr daddr;
   unsigned int filesize;
   unsigned int amountstored;
   unsigned int bufferindex;
   unsigned char *filedata;
   unsigned char md5[16];
   int alert;
} FILEINFO;

struct FILEINFOLISTELEM {
   FILEINFO *fileinfo;
   struct FILEINFOLISTELEM *next;
};

int AddFileInfoListElem(RULEDATA *, FILEINFO *);
void DumpFileInfo(FILEINFO *);
int DumpFileInfoList(RULEDATA *);
void FreeFileInfo(FILEINFO *);

int DeleteFileInfoListHead(RULEDATA *);
FILEINFO *PopFileInfo(RULEDATA *);
void FreeFileInfoList(RULEDATA *);
void FreeNRTStreamData(void *);

extern int numfileinfostructsinuse;

extern struct FILEINFOLISTELEM *fileinfolist[NUMSTREAMSTOTRACK];
#endif

