#ifdef HAVE_CONFIG_H
#include "autoconf.h"
#endif /* HAVE_CONFIG_H */

#include "osdep.h"

static const char rcsid[] =
"$Id: in_cksum.c,v 1.6 2013/08/20 18:37:19 michaels Exp $";


/*
 * Copied from W. R. Stevens UNP book.  R.I.P.
 */

uint16_t
in_cksum(uint16_t *addr, int len)
{
   int      nleft = len;
   uint32_t sum    = 0;
   uint16_t *w     = addr;
   uint16_t answer = 0;

   /*
    * Our algorithm is simple, using a 32 bit accumulator (sum), we add
    * sequential 16 bit words to it, and at the end, fold back all the
    * carry bits from the top 16 bits into the lower 16 bits.
    */
   while (nleft > 1)  {
      sum += *w++;
      nleft -= 2;
   }

   /* mop up an odd byte, if necessary */
   if (nleft == 1) {
      *(unsigned char *)(&answer) = *(unsigned char *)w ;
      sum                        += answer;
   }

   /* add back carry outs from top 16 bits to low 16 bits */
   sum    = (sum >> 16) + (sum & 0xffff);   /* add hi 16 to low 16. */
   sum   += (sum >> 16);                    /* add carry.           */
   answer = ~sum;                           /* truncate to 16 bits. */

   return answer;
}
