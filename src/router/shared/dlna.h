#ifndef _dlna_h_
#define _dlna_h_
#include <jansson.h>

#define TYPE_AUDIO 0x1
#define TYPE_VIDEO 0x2
#define TYPE_IMAGES 0x4

struct dlna_share {
	char mp[32];
	char sd[64];
	int types;
	struct dlna_share *next;
};

struct dlna_share *getdlnashares(void);
struct dlna_share *getdlnashare(char *mp, char *sd, int types);

#endif
