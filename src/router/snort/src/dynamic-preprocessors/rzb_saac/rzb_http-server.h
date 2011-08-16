#ifndef NRT_SERVER_H
#define NRT_SERVER_H

#include <rzb_collection_api.h>
#include "rzb_http-fileinfo.h"

#include "sf_snort_packet.h"

int ProcessFromServer(SFSnortPacket *);
enum filereadstatus ProcessServerHeader(const u_int8_t **, const u_int8_t *, FILEINFO *);
enum filereadstatus ReadFileData(const u_int8_t **, const u_int8_t *, FILEINFO *);
int SkipToEndOfHTTPHeader(const u_int8_t **, const u_int8_t *);
int CallDetectionFunction(FILEINFO *);

int IsStreamIgnored(RULEDATA *);
void IgnoreStream(RULEDATA *);

#endif

