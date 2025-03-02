/* Utilitity and replacement functions */
#ifndef SMCROUTE_UTIL_H_
#define SMCROUTE_UTIL_H_

#include "config.h"
#include <stdio.h>
#include <string.h>
#include "mroute.h"

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#endif

/* From The Practice of Programming, by Kernighan and Pike */
#ifndef NELEMS
#define NELEMS(array) (sizeof(array) / sizeof(array[0]))
#endif

int pidfile_create(const char *basename, uid_t uid, gid_t gid);

#ifndef HAVE_UTIMENSAT
int utimensat(int dirfd, const char *pathname, const struct timespec ts[2], int flags);
#endif

#ifndef HAVE_STRLCPY
size_t strlcpy(char *dst, const char *src, size_t len);
#endif

#ifndef HAVE_STRLCAT
size_t strlcat(char *dst, const char *src, size_t dsize);
#endif

#ifndef HAVE_TEMPFILE
FILE *tempfile(void);
#endif

int is_range(char *arg);

inline static char *chomp(char *str)
{
	char *p;

	if (!str || strlen(str) < 1) {
		errno = EINVAL;
		return NULL;
	}

	p = str + strlen(str) - 1;
        while (p >= str && *p == '\n')
		*p-- = 0;

	return str;
}

#endif /* SMCROUTE_UTIL_H_ */
