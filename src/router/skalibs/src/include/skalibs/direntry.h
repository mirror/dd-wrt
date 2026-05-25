/* ISC license. */

#ifndef SKALIBS_DIRENTRY_H
#define SKALIBS_DIRENTRY_H

#include <dirent.h>

typedef struct dirent direntry, *direntry_ref ;

extern void dir_close (DIR *) ;
extern int dir_fd (DIR *) ;  /* Solaris doesn't have dirfd() */

#endif
