/* 
 * IP address pool functions.
 * Copyright (C) 2003, 2004 Mondru AB.
 * 
 * The contents of this file may be used under the terms of the GNU
 * General Public License Version 2, provided that the above copyright
 * notice and this permission notice is included in all copies or
 * substantial portions of the software.
 * 
 */

#ifndef _IPPOOL_H
#define _IPPOOL_H

/* Assuming that the address space is fragmented we need a hash table
   in order to return the addresses.

   The list pool should provide for both IPv4 and IPv6 addresses.

   When initialising a new address pool it should be possible to pass
   a string of CIDR format networks: "10.0.0.0/24 10.15.0.0/20" would
   translate to 256 addresses starting at 10.0.0.0 and 1024 addresses
   starting at 10.15.0.0. 

   The above also applies to IPv6 which can be specified as described
   in RFC2373.
*/

#define IPPOOL_NOIP6

#define IPPOOL_NONETWORK   0x01
#define IPPOOL_NOBROADCAST 0x02
#define IPPOOL_NOGATEWAY   0x04

#define IPPOOL_STATSIZE 0x10000

struct ippoolm_t;                /* Forward declaration */

struct ippool_t {
  int listsize;                  /* Total number of addresses */
  int allowdyn;                  /* Allow dynamic IP address allocation */
  int allowstat;                 /* Allow static IP address allocation */
  struct in_addr stataddr;       /* Static address range network address */
  struct in_addr statmask;       /* Static address range network mask */
  struct ippoolm_t *member;      /* Listsize array of members */
  int hashsize;                  /* Size of hash table */
  int hashlog;                   /* Log2 size of hash table */
  int hashmask;                  /* Bitmask for calculating hash */
  struct ippoolm_t **hash;       /* Hashsize array of pointer to member */
  struct ippoolm_t *firstdyn;    /* Pointer to first free dynamic member */
  struct ippoolm_t *lastdyn;     /* Pointer to last free dynamic member */
  struct ippoolm_t *firststat;   /* Pointer to first free static member */
  struct ippoolm_t *laststat;    /* Pointer to last free static member */
};

struct ippoolm_t {
#ifndef IPPOOL_NOIP6
  struct in6_addr addr;          /* IP address of this member */
#else
  struct in_addr addr;           /* IP address of this member */
#endif
  int inuse;                     /* 0=available; 1= dynamic; 2 = static */
  struct ippoolm_t *nexthash;    /* Linked list part of hash table */
  struct ippoolm_t *prev, *next; /* Linked list of free dynamic or static */
  void *peer;                    /* Pointer to peer protocol handler */
};

/* The above structures require approximately 20+4 = 24 bytes for
   each address (IPv4). For IPv6 the corresponding value is 32+4 = 36
   bytes for each address. */

/* Hash an IP address using code based on Bob Jenkins lookupa */
extern unsigned long int ippool_hash4(struct in_addr *addr);

/* Create new address pool */
extern int ippool_new(struct ippool_t **this, char *dyn,  char *stat, 
		      int allowdyn, int allowstat, int flags);

/* Delete existing address pool */
extern int ippool_free(struct ippool_t *this);

/* Find an IP address in the pool */
extern int ippool_getip(struct ippool_t *this, struct ippoolm_t **member,
		 struct in_addr *addr);

/* Get an IP address. If addr = 0.0.0.0 get a dynamic IP address. Otherwise
   check to see if the given address is available */
extern int ippool_newip(struct ippool_t *this, struct ippoolm_t **member,
			struct in_addr *addr, int statip);

/* Return a previously allocated IP address */
extern int ippool_freeip(struct ippool_t *this, struct ippoolm_t *member);

/* Get net and mask based on ascii string */
extern int ippool_aton(struct in_addr *addr, struct in_addr *mask,
		       char *pool, int number);


#ifndef IPPOOL_NOIP6
extern unsigned long int ippool_hash6(struct in6_addr *addr);
extern int ippool_getip6(struct ippool_t *this, struct in6_addr *addr);
extern int ippool_returnip6(struct ippool_t *this, struct in6_addr *addr);
#endif

#endif	/* !_IPPOOL_H */
