/* 
 * IP hash functions.
 * Copyright (C) 2003, 2004, 2005 Mondru AB.
 * 
 * The contents of this file may be used under the terms of the GNU
 * General Public License Version 2, provided that the above copyright
 * notice and this permission notice is included in all copies or
 * substantial portions of the software.
 * 
 */


#ifndef _IPHASH_H
#define _IPHASH_H

#include "ippool.h"

/* IP hash functions are used to generate a hash table of IP addresses.
   The functions build on ippool.c.
   ippool_getip() is used to check if an address is in the hash table. */

/* Create new address pool */
extern 
int iphash_new(struct ippool_t **this, struct ippoolm_t *list, int listsize);

/* Delete existing address pool */
extern int iphash_free(struct ippool_t *this);


#endif	/* !_IPHASH_H */
