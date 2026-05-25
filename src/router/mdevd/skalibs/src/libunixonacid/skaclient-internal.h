/* ISC license. */

#ifndef SKACLIENT_INTERNAL_H
#define SKACLIENT_INTERNAL_H

#include <sys/types.h>

#include <skalibs/kolbak.h>
#include <skalibs/unixmessage.h>
#include <skalibs/skaclient.h>

extern int skaclient_init (skaclient *, int, char *, size_t, char *, size_t, char *, size_t, char *, size_t, kolbak_closure *, size_t, char const *, size_t) ;
extern int skaclient_start_cb (unixmessage const *, skaclient_cbdata *) ;

#endif
