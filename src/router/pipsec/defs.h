/* $Id: defs.h,v 1.3 1999/04/09 23:30:02 beyssac Exp $ */

#ifndef _DEFS_H
#define _DEFS_H

#if defined(__FreeBSD__)
#define HAVE_SA_LEN
#define NEED_IPID_SWAP
#define NEED_IPLEN_FIX
#define HAVE_MD5
#endif

#endif
