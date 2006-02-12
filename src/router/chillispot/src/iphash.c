/* 
 * IP address pool functions.
 * Copyright (C) 2003, 2004, 2005 Mondru AB.
 * 
 * The contents of this file may be used under the terms of the GNU
 * General Public License Version 2, provided that the above copyright
 * notice and this permission notice is included in all copies or
 * substantial portions of the software.
 * 
 */
 
#include <sys/types.h>
#include <netinet/in.h> /* in_addr */
#include <stdlib.h>     /* calloc */
#include <stdio.h>      /* sscanf */

#include "iphash.h"

/* Create new address pool hash */
int iphash_new(struct ippool_t **this, struct ippoolm_t *list, int listsize) {

  int i;

  if (!(*this = calloc(sizeof(struct ippool_t), 1))) {
    /* Failed to allocate memory for iphash */
    return -1;
  }
  
  (*this)->listsize = listsize;
  (*this)->member = list;

  /* Determine log2 of hashsize */
  for ((*this)->hashlog = 0; 
       ((1 << (*this)->hashlog) < listsize);
       (*this)->hashlog++);
  
  /* Determine hashsize */
  (*this)->hashsize = 1 << (*this)->hashlog; /* Fails if mask=0: All Internet*/
  (*this)->hashmask = (*this)->hashsize -1;
  
  /* Allocate hash table */
  if (!((*this)->hash = calloc(sizeof(struct ippoolm_t), (*this)->hashsize))){
    /* Failed to allocate memory for hash members in iphash */
    return -1;
  }
  
  for (i = 0; i<listsize; i++) {
    
    (*this)->member[i].inuse = 1; /* TODO */
    ippool_hashadd(*this, &(*this)->member[i]);
  }

  return 0;
}

/* Delete existing address pool */
int iphash_free(struct ippool_t *this) {
  free(this->hash);
  free(this);
  return 0; /* Always OK */
}
