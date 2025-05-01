/*
 * Compatability glue for systems lacking the if_nametoindex and
 * if_indextoname functions.
 *
 * The file 'if_index.c' was taken verbatimly from the GNU C Library
 * version 2.1 (990920) and is Copyright (C) 1997, 1998, 1999 Free
 * Software Foundation, Inc.
 */

#define _BITS_LIBC_LOCK_H 1
#define __libc_lock_define_initialized(a,b)
#define __libc_lock_lock(a)
#define __libc_lock_unlock(a)
#define __ioctl ioctl
#define __set_errno(a) {errno = (a);}
#define __socket socket
#define internal_function

#include <features.h>
#if __GLIBC_MINOR__ < 1
/* Return a list of all interfaces and their indices.  */

struct if_nameindex
  {
    unsigned int if_index;	/* 1, 2, ... */
    char *if_name;		/* null terminated name: "eth0", ... */
  };
#endif

#include "if_index.c"
