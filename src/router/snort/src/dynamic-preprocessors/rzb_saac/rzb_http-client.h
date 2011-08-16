#ifndef NRT_CLIENT_H
#define NRT_CLIENT_H

#include <rzb_collection_api.h>

#include "sf_snort_packet.h"

int ParseClientRequest(const u_int8_t *, u_int32_t, WEB_ENTRY*);
int ProcessFromClient(SFSnortPacket *);
int init_HTTP_PCREs(void);

#endif

