#ifndef _FTW_H
#define	_FTW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <features.h>
#include <sys/stat.h>

#define FTW_F   1
#define FTW_D   2
#define FTW_DNR 3
#define FTW_NS  4
#define FTW_SL  5
#define FTW_DP  6
#define FTW_SLN 7

#define FTW_PHYS  1
#define FTW_MOUNT 2
#define FTW_CHDIR 4
#define FTW_DEPTH 8

#ifdef _GNU_SOURCE
#define FTW_ACTIONRETVAL 16
#define FTW_CONTINUE 0
#define FTW_STOP 1
#define FTW_SKIP_SUBTREE 2
#define FTW_SKIP_SIBLINGS 3
#endif

struct FTW {
	int base;
	int level;
};

int ftw(const char *, int (*)(const char *, const struct stat *, int), int);
int nftw(const char *, int (*)(const char *, const struct stat *, int, struct FTW *), int, int);

#if defined(_LARGEFILE64_SOURCE)
#define ftw64 ftw
#define nftw64 nftw
#endif

#ifdef __cplusplus
}
#endif

#endif
