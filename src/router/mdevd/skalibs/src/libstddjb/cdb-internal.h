/* ISC license. */

#ifndef SKALIBS_CDB_INTERNAL_H
#define SKALIBS_CDB_INTERNAL_H

#include <stdint.h>
#include <sys/uio.h>

#include <skalibs/gccattributes.h>
#include <skalibs/cdb.h>

#define CDB_HASHSTART 5381

extern uint32_t cdb_hashadd (uint32_t, uint8_t) ;
extern uint32_t cdb_hash (char const *, uint32_t) gccattr_pure ;
extern uint32_t cdb_hashv (struct iovec const *, unsigned int) gccattr_pure ;
extern char const *cdb_p (cdb const *, uint32_t, uint32_t) gccattr_pure ;

#endif
