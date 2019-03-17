#ifndef _nfs_h_
#define _nfs_h_
#include <jansson.h>

struct nfs_share {
	char mp[32];
	char sd[64];
	char access_perms[4];
	char allowed[64];
	struct nfs_share *next;
};

struct nfs_share *getnfsshares(void);
struct nfs_share *getnfsshare(char *mp, char *sd, char *access_perms, char *allowed);

#endif
