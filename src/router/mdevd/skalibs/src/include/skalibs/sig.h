/* ISC license. */

#ifndef SKALIBS_SIG_H
#define SKALIBS_SIG_H

#include <sys/types.h>
#include <signal.h>

#include <skalibs/gccattributes.h>

typedef void sig_func (int) ;
typedef sig_func *sig_func_ref ;

extern int sig_catch (int, sig_func_ref) ;
#define sig_restore(sig) sig_catch((sig), SIG_DFL)
#define sig_ignore(sig) sig_catch((sig), SIG_IGN)
extern int sig_altignore (int) ;

extern void sig_restoreto (sigset_t const *, unsigned int) ;

extern void sig_block (int) ;
extern void sig_unblock (int) ;
extern void sig_blocknone (void) ;

extern char const *sig_name (int) ;
extern int sig_number (char const *) ;
extern size_t sig0_scan (char const *, int *) ;

#endif
