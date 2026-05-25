/* ISC license. */

#ifndef SKALIBS_ENV_H
#define SKALIBS_ENV_H

#include <sys/types.h>

#include <skalibs/gccattributes.h>
#include <skalibs/stralloc.h>

extern size_t env_len (char const *const *) gccattr_pure ;
extern char const *env_get2 (char const *const *, char const *) gccattr_pure ;
extern char const *ucspi_get (char const *) gccattr_pure ;

extern int env_addmodif (stralloc *, char const *, char const *) ;
extern int env_make (char const **, size_t, char const *, size_t) ;
extern int env_string (stralloc *, char const *const *, size_t) ;

extern size_t env_merg (char const **, size_t, char const *const *, char const *, size_t) ;
extern size_t env_merge (char const **, size_t, char const *const *, size_t, char const *, size_t) ;
extern size_t env_mergen (char const **, size_t, char const *const *, size_t, char const *, size_t, size_t) ;
extern size_t env_mergn (char const **, size_t, char const *const *, char const *, size_t, size_t) ;

#define SKALIBS_ENVDIR_VERBATIM 0x01
#define SKALIBS_ENVDIR_NOCHOMP 0x02
#define SKALIBS_ENVDIR_NOCLAMP 0x04

extern int envdir_internal (char const *, stralloc *, unsigned int, char) ;
#define envdir(path, sa) envdir_internal(path, (sa), 0, '\n')
#define envdir_chomp(path, sa) envdir_internal(path, (sa), SKALIBS_ENVDIR_NOCHOMP, '\n')
#define envdir_verbatim_chomp(path, sa) envdir_internal(path, (sa), SKALIBS_ENVDIR_VERBATIM, '\n')
#define envdir_verbatim(path, sa) envdir_internal(path, (sa), SKALIBS_ENVDIR_VERBATIM|SKALIBS_ENVDIR_NOCHOMP, '\n')
extern int env_dump (char const *, mode_t, char const *const *) ;
extern int env_dump4 (char const *, mode_t, char const *const *, int) ;

#endif
