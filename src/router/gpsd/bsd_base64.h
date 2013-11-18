/*
 * This file is Copyright (c)2010 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 */
#ifndef _BSD_BASE64_H
#define _BSD_BASE64_H

#ifndef HAVE___B64_NTOP
# ifndef HAVE_B64_NTOP
int b64_ntop(unsigned char const *src, size_t srclength, char *target,
    size_t targsize);
int b64_pton(char const *src, unsigned char *target, size_t targsize);
# endif /* !HAVE_B64_NTOP */
# define __b64_ntop b64_ntop
# define __b64_pton b64_pton
#endif /* HAVE___B64_NTOP */

#endif /* _BSD_BASE64_H */
