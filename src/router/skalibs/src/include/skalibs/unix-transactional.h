/* ISC license. */

#ifndef SKALIBS_UNIX_TRANSACTIONAL_H
#define SKALIBS_UNIX_TRANSACTIONAL_H

#include <stddef.h>
#include <sys/stat.h>
#include <sys/uio.h>

#include <skalibs/gccattributes.h>
#include <skalibs/direntry.h>
#include <skalibs/stralloc.h>

 /* Transactional/reliable filesystem operations */

extern int opengetlnclose (char const *, stralloc *, int) ;

extern int open2_at (int, char const *, int) ;
extern int open3_at (int, char const *, int, unsigned int) ;
extern int access_at (int, char const *, int, unsigned int) ;
extern DIR *opendir_at (int, char const *) ;
extern int stat_at (int, char const *, struct stat *) ;
extern int lstat_at (int, char const *, struct stat *) ;

extern int open_readat (int, char const *) ;
extern int open_readatb (int, char const *) ;
extern int open_writeat (int, char const *) ;
extern int open_writeatb (int, char const *) ;
extern int open_truncat (int, char const *) ;
extern int open_truncatb (int, char const *) ;
extern int open_appendat (int, char const *) ;
extern int open_appendatb (int, char const *) ;
extern int openc_readat (int, char const *) ;
extern int openc_readatb (int, char const *) ;
extern int openc_writeat (int, char const *) ;
extern int openc_writeatb (int, char const *) ;
extern int openc_truncat (int, char const *) ;
extern int openc_truncatb (int, char const *) ;
extern int openc_appendat (int, char const *) ;
extern int openc_appendatb (int, char const *) ;

extern ssize_t openreadnclose_at (int, char const *, char *, size_t) ;
extern int openslurpclose_at (int, char const *, stralloc *) ;
extern int opengetlnclose_at (int, char const *, stralloc *, int) ;

extern size_t openwritenclose_at (int, char const *, char const *, size_t) ;
extern size_t openwritevnclose_at (int, char const *, struct iovec const *, unsigned int) ;

extern int atomic_rm_rf (char const *) ;
extern int atomic_rm_rf_tmp (char const *, stralloc *) ;
extern int atomic_symlink (char const *, char const *, char const *) gccattr_deprecated ;
extern int atomic_symlink4 (char const *, char const *, char *, size_t) ;

#endif
