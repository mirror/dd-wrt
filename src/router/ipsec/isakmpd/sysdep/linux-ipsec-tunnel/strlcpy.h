/* $Id: strlcpy.h,v 1.1.2.1 2004/03/23 15:38:59 nikki Exp $ */

#ifndef _BSD_STRLCPY_H
#define _BSD_STRLCPY_H

#ifndef HAVE_STRLCPY
#include <sys/types.h>
size_t strlcpy(char *dst, const char *src, size_t siz);
#endif /* !HAVE_STRLCPY */

#endif /* _BSD_STRLCPY_H */
