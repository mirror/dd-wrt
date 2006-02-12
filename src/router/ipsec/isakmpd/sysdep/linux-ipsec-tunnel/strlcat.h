/* $Id: strlcat.h,v 1.1.2.1 2004/03/23 15:38:59 nikki Exp $ */

#ifndef _BSD_STRLCAT_H
#define _BSD_STRLCAT_H

#ifndef HAVE_STRLCAT
#include <sys/types.h>
size_t strlcat(char *dst, const char *src, size_t siz);
#endif /* !HAVE_STRLCAT */

#endif /* _BSD_STRLCAT_H */
