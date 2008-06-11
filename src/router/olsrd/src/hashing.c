
/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 *
 * * Redistributions of source code must retain the above copyright 
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright 
 *   notice, this list of conditions and the following disclaimer in 
 *   the documentation and/or other materials provided with the 
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its 
 *   contributors may be used to endorse or promote products derived 
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#include "olsr_protocol.h"
#include "hashing.h"
#include "defs.h"

/*
 * Taken from lookup2.c by Bob Jenkins.  (http://burtleburtle.net/bob/c/lookup2.c).
 * --------------------------------------------------------------------
 * lookup2.c, by Bob Jenkins, December 1996, Public Domain.
 * You can use this free for any purpose.  It has no warranty.
 * --------------------------------------------------------------------
 */

#define __jhash_mix(a, b, c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

static inline olsr_u32_t
jenkins_hash(const olsr_u8_t * k, olsr_u32_t length)
{
  /* k: the key
   * length: length of the key
   * initval: the previous hash, or an arbitrary value
   */
  olsr_u32_t a, b, c, len;

  /* Set up the internal state */
  len = length;
  a = b = 0x9e3779b9;		/* the golden ratio; an arbitrary value */
  c = 0;			/* the previous hash value */

  /* handle most of the key */
  while (len >= 12) {
    a += (k[0] + ((olsr_u32_t) k[1] << 8) + ((olsr_u32_t) k[2] << 16)
	  + ((olsr_u32_t) k[3] << 24));
    b += (k[4] + ((olsr_u32_t) k[5] << 8) + ((olsr_u32_t) k[6] << 16)
	  + ((olsr_u32_t) k[7] << 24));
    c += (k[8] + ((olsr_u32_t) k[9] << 8) + ((olsr_u32_t) k[10] << 16)
	  + ((olsr_u32_t) k[11] << 24));

    __jhash_mix(a, b, c);

    k += 12;
    len -= 12;
  }

  c += length;
  switch (len) {
  case 11:
    c += ((olsr_u32_t) k[10] << 24);
  case 10:
    c += ((olsr_u32_t) k[9] << 16);
  case 9:
    c += ((olsr_u32_t) k[8] << 8);
    /* the first byte of c is reserved for the length */
  case 8:
    b += ((olsr_u32_t) k[7] << 24);
  case 7:
    b += ((olsr_u32_t) k[6] << 16);
  case 6:
    b += ((olsr_u32_t) k[5] << 8);
  case 5:
    b += k[4];
  case 4:
    a += ((olsr_u32_t) k[3] << 24);
  case 3:
    a += ((olsr_u32_t) k[2] << 16);
  case 2:
    a += ((olsr_u32_t) k[1] << 8);
  case 1:
    a += k[0];
  }
  __jhash_mix(a, b, c);

  return c;
}


/**
 * Hashing function. Creates a key based on an IP address.
 * @param address the address to hash
 * @return the hash(a value in the (0 to HASHMASK-1) range)
 */
olsr_u32_t
olsr_ip_hashing(const union olsr_ip_addr *address)
{
  olsr_u32_t hash;

  switch (olsr_cnf->ip_version) {
  case AF_INET:
    hash = jenkins_hash((const olsr_u8_t *)&address->v4,
                        sizeof(olsr_u32_t));
    break;
  case AF_INET6:
    hash = jenkins_hash((const olsr_u8_t *)&address->v6,
			sizeof(struct in6_addr));
    break;
  default:
    hash = 0;
    break;

  }
  return hash & HASHMASK;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * End:
 */
