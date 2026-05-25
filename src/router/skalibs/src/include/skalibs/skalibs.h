/* ISC license. */

#ifndef SKALIBS_H
#define SKALIBS_H

/*
   This header is heavy! It includes everything in skalibs except:
   - skalibs/config.h: package configuration
   - skalibs/sysdeps.h: system-dependent feature test macros
   - skalibs/nonposix.h: pre-system headers definitions for POSIX extensions
   - skalibs/bsdsnowflake.h: pre-system headers BSD-specific workarounds
   - skalibs/nsig.h: SKALIBS_NSIG definition, requires nonposix
   - skalibs/posixishard.h: post-system headers workarounds for conformance failures
*/

#include <skalibs/cplz.h>
#include <skalibs/posixplz.h>
#include <skalibs/envexec.h>
#include <skalibs/stddjb.h>
#include <skalibs/stdcrypto.h>
#include <skalibs/random.h>
#include <skalibs/datastruct.h>
#include <skalibs/unixonacid.h>
#include <skalibs/playnice.h>

#endif
