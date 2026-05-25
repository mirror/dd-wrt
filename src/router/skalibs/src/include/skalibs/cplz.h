/* ISC license. */

#ifndef SKALIBS_CPLZ_H
#define SKALIBS_CPLZ_H

 /*
    Tricks to work around misdesigns of C.
 */

#define LAUNDER(p) do { void *_p = p ; asm volatile("" : "+r"(_p) ::) ; _p ; } while (0)

#endif
