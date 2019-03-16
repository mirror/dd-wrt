#ifndef _rsync_h_
#define _rsync_h_
#include <jansson.h>

struct rsync_share {
	char mp[32];
	char sd[64];
	char label[64];
	struct rsync_share *next;
};

struct rsync_share *getrsyncshares(void);
struct rsync_share *getrsyncshare(char *mp, char *sd, char *label);

#endif
