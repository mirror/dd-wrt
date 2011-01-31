#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "packets.h"

/* Extern func's prototypes found from packets.h */

struct tag{
  struct tag *next;
  struct tag *prev;
  uint32_t tag;
  uint32_t cache_id;
}; 

static struct tag *tags = NULL;

static struct tag *search_tag(uint32_t tag){
  struct tag *ptag = tags;
  while(ptag != NULL){
    if(tag == ptag->tag)
      return ptag;
  ptag = ptag->next;
  }
  return NULL;
}

static struct tag *search_id(uint32_t cache_id){
  struct tag *ptag = tags;
  while(ptag != NULL){
    if(cache_id == ptag->cache_id)
      return ptag;
  ptag = ptag->next;
  }
  return NULL;
}

static uint32_t get_tag(void){
  static uint32_t tag = 0;
  tag = (tag + 1) %  UINT_MAX;
  return tag;
}

uint32_t new_tag(uint32_t cache_id){
  struct tag *new;
  new = search_id(cache_id);
  if(new != NULL)
    return new->tag;
  new = malloc(sizeof(struct tag));
  memset(new,0,sizeof(struct tag));
  new->tag = get_tag();
  while(search_tag(new->tag) != NULL)
    new->tag = get_tag();
  new->next = tags;
  if(tags != NULL)
    tags->prev = new;
  tags = new;
  new->cache_id = cache_id;
  return new->tag;
}

int remove_tag(uint32_t tag){
  struct tag *ptag = search_tag(tag);
  if(ptag == NULL)
    return 0;
  printf("mpcd: tag_list.c: removing tag %u from the tag_list.\n",tag);
  if(ptag->prev != NULL)
    ptag->prev->next = ptag->next;
  else
    tags = ptag->next;
  if(ptag->next != NULL)
    ptag->next->prev = ptag->prev;
  free(ptag);
  return 1;
}


