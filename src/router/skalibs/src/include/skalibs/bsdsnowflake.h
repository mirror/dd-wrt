/* ISC license. */

#ifndef SKALIBS_BSDSNOWFLAKE_H
#define SKALIBS_BSDSNOWFLAKE_H

 /*
   Like skalibs/nonposix.h, this header is supposed to be included
   *before* system headers.
   Unlike skalibs/nonposix.h, though, it does not define ftms that
   enable non-portable behaviour; it just attempts to work around
   blatant brokenness, things that actually ARE defined by POSIX
   but where OSes just ignore the spec.
   The BSDs are experts at this, hence the name.
 */


#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__bsdi__) || defined(__DragonFly__)

 /* BSDs: the eponym.
    Sometimes you just need to recognize who they are and what they do. */

#ifndef SKALIBS_BSD_SUCKS
#define SKALIBS_BSD_SUCKS
#endif

 /* Other times you just need to say you're a BSD so they deign
    to define symbols like EOVERFLOW. Give me my POSIX symbols,
    pretty please? */

#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#endif


#if defined(__APPLE__) && defined(__MACH__)

 /* MacOS: needs this for full SUSv3 conformance. That's how you
    can tell it's really a BSD inside. With additional layers of crap. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#ifndef _DARWIN_C_SOURCE
#define _DARWIN_C_SOURCE
#endif

#endif /* __APPLE__ && __MACH__ */

#endif /* SKALIBS_BSDSNOWFLAKE_H */
