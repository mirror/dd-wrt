/**
 * libcache.c
 *
Copyright (c) 2017 William Guglielmo <william@deselmo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 *
 */

#ifndef __KERNEL__
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#define LOCK_INIT(a)
#define LOCK_BEGIN(a)
#define LOCK_END(a)
#else
#include <asm/byteorder.h>
#include <linux/in.h>
#include <linux/in6.h>
#include <asm/errno.h>

#define LOCK_INIT(a) spin_lock_init(&(a)->lock)
#define LOCK_BEGIN(a) spin_lock_bh(&(a)->lock)
#define LOCK_END(a) spin_unlock_bh(&(a)->lock)
#endif

#include "ndpi_api.h"
#include "libcache.h"

#ifndef NDPI_API_VERSION
#define ndpi_malloc(a) malloc(a)
#define ndpi_calloc(a,b) calloc(a,b)
#define ndpi_free(a) free(a)
#endif

#define member_size(type, member) sizeof(((type *)0)->member)

// https://en.wikipedia.org/wiki/Jenkins_hash_function
static uint32_t jenkins_one_at_a_time_hash(const void * _key, size_t length) {
  const uint8_t* key = _key;
  size_t i = 0;
  uint32_t hash = 0;
  while (i != length) {
    hash += key[i++];
    hash += hash << 10;
    hash ^= hash >> 6;
  }
  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;
  return hash;
}


struct cache {
#ifdef __KERNEL__
  spinlock_t lock;
#endif
  uint32_t size;
  uint32_t max_size;
  uint32_t hash_size;
  cache_item_compare item_compare;
  cache_item_hash    item_hash;
  struct cache_entry *head;
  struct cache_entry *tail;
  struct cache_entry *map[]; // last field!
};

struct cache_entry {
  struct cache_entry *prev;
  struct cache_entry *next;
  struct cache_entry *item_next;
  uint16_t item_size,item_hash;
  uint8_t item[]; // last field!
};

typedef struct cache_entry cache_entry_t;

static void cache_touch_entry(cache_t cache, cache_entry_t *entry) {

    /* remove from list */
    if(!entry->prev) return; // first element
    entry->prev->next = entry->next;
    if(entry->next) 
        entry->next->prev = entry->prev;
      else { /* last entry */
        cache->tail = entry->prev;
	cache->tail->next = NULL;
      }
    /* insert to head */
    entry->prev = NULL;
    entry->next = cache->head;
    cache->head->prev = entry;
    cache->head = entry;
}

static cache_entry_t *cache_entry_new(size_t item_size) {
  return (cache_entry_t *) ndpi_calloc(sizeof(cache_entry_t)+item_size, 1);
}

static cache_entry_t **cache_find_entry(cache_t cache, 
		uint32_t hash, void *item, size_t item_size) {
		
  cache_entry_t **prev = &cache->map[hash];
  cache_entry_t *entry = *prev;

  while(entry) {
    if(cache->item_compare) {
	if(!cache->item_compare(entry->item,entry->item_size,item, item_size))
		break;
    } else {
        if(item_size == entry->item_size &&
           !memcmp(entry->item, item, item_size)) break;
    }
    prev = &entry->item_next;
    entry = *prev;
  }
  return prev;
}

cache_t cache_new(uint32_t cache_max_size,uint32_t cache_hash_size) {
  cache_t cache;
  if(!cache_max_size || cache_max_size < 2 ||
      cache_max_size >= (1 << (member_size(cache_entry_t,item_size)*8))) {
    return NULL;
  }
  if(!cache_hash_size) {
      if(cache_max_size < 256) cache_hash_size=7;
        if(cache_max_size < 1024) cache_hash_size=31;
          else if(cache_max_size < 4096) cache_hash_size=127;
            else if(cache_max_size < 16384) cache_hash_size=511;
              else cache_hash_size = 2047;
  }
  if(!(cache_hash_size & 1)) cache_hash_size++; // better if there is an odd number

  cache = (cache_t) ndpi_calloc(1,sizeof(struct cache) + cache_hash_size * sizeof(cache_entry_t *));
  if(!cache)
    return NULL;
  LOCK_INIT(cache);
  cache->size = 0;
  cache->max_size = cache_max_size;
  cache->hash_size = cache_hash_size;
  cache->item_hash = jenkins_one_at_a_time_hash;
  return cache;
}

cache_result cache_add(cache_t cache, void *item, size_t item_size) {
  uint32_t hash;
  cache_result ret = CACHE_NO_ERROR;

  if(!cache || !item || !item_size || !cache->max_size) {
    return CACHE_INVALID_INPUT;
  }

  LOCK_BEGIN(cache);
  hash = cache->item_hash(item, item_size) % cache->hash_size;
  do {
      cache_entry_t *entry, *new_entry;
      entry = *cache_find_entry(cache, hash, item, item_size);
      if(entry) {
          cache_touch_entry(cache, entry);
          break;
      }

      new_entry = cache_entry_new(item_size);
      if(!new_entry) {
          ret = CACHE_MALLOC_ERROR;
          break;
      }
      memcpy(&new_entry->item[0], item, item_size);
      new_entry->item_size = item_size;
      new_entry->item_hash = hash;

      new_entry->next = cache->head;
      if(cache->head)
    	  cache->head->prev = new_entry;
        else
    	  cache->tail = new_entry;
      cache->head = new_entry;

      new_entry->item_next = cache->map[hash];
      cache->map[hash] = new_entry;

      if(cache->max_size > cache->size) {
        cache->size++;
      } else {

        cache_entry_t *tail = cache->tail, **prev_entry;

        prev_entry = &cache->map[tail->item_hash];
        entry = *prev_entry;
        while(entry) {
            if(entry == tail) {
    	        *prev_entry = entry->item_next;
            
    	        /* We remove TAIL entry and cache size > 1 -> not HEAD entry */
    	        entry->prev->next = NULL;
    	        cache->tail = entry->prev; 
 
    	        ndpi_free(entry);
		tail = NULL;
      	        break;
            }
            prev_entry = &entry->item_next;
            entry = *prev_entry;
        }
	if(tail)
	    ret = CACHE_REMOVE_NOT_FOUND;
      }
  } while(0);
  LOCK_END(cache);
  return ret;

}

void *cache_contains(cache_t cache, void *item, size_t item_size) {
  uint32_t hash;
  cache_entry_t *entry;

  if(!cache || !item || !item_size || !cache->max_size) {
    return NULL;
  }

  LOCK_BEGIN(cache);
  hash = cache->item_hash(item, item_size) % cache->hash_size;
  entry = *cache_find_entry(cache, hash, item, item_size);
  if(entry)
        cache_touch_entry(cache, entry);

  LOCK_END(cache);
  return entry ? entry->item: NULL;
}

cache_result cache_remove(cache_t cache, void *item, size_t item_size) {
  uint32_t hash;
  cache_entry_t *entry,**prev=NULL;

  if(!cache || !item || !item_size || !cache->max_size) {
    return CACHE_INVALID_INPUT;
  }

  LOCK_BEGIN(cache);
  hash = cache->item_hash(item, item_size) % cache->hash_size;
  prev = cache_find_entry(cache, hash, item, item_size);
  entry = *prev;
  if(!entry) {
      LOCK_END(cache);
      return CACHE_REMOVE_NOT_FOUND;
  }

  *prev = entry->item_next;

  if(entry->prev) {
	entry->prev->next = entry->next;
  } else {
	cache->head = entry->next;
	if(cache->head) cache->head->prev = NULL;
  }
  if(entry->next) {
	entry->next->prev = entry->prev;
  } else {
	cache->tail = entry->prev;
	if(cache->tail) cache->tail->next = NULL;
  }
  ndpi_free(entry);
  cache->size--;
  LOCK_END(cache);
  return CACHE_NO_ERROR;
}

void cache_free(cache_t cache) {
  cache_entry_t *entry,*next;
  if(!cache) {
    return;
  }
  LOCK_BEGIN(cache);
  for(entry = cache->head; entry; entry = next) {
    next = entry->next;
    ndpi_free(entry);
  }
  LOCK_END(cache);

  ndpi_free(cache);

  return;
}

#ifndef __KERNEL__
void cache_dump(cache_t cache) {
  unsigned int i;
  cache_entry_t *entry,*entry2;
  if(!cache) {
    return;
  }
  printf("== size:%d ",cache->size);
  printf("head:%s %d ",(char *)cache->head->item,cache->head->item_hash);
  printf("tail:%s %d\n",(char *)cache->tail->item,cache->tail->item_hash);

  for(i=0,entry2 = cache->head; i < cache->hash_size; i++) {
    for(entry = cache->map[i]; entry; entry = entry->item_next) {
      printf("%d: %s %s",
		     i, entry == cache->head ? "H" : (entry == cache->tail ? "T":" "),
		    	 (char *)entry->item);
      if(entry2) printf("\t%u:%s %s",
		     entry2->item_hash,
		     cache->map[entry2->item_hash] == entry2 ? "*":" ",
		     (char *)entry2->item
		     );
      else { printf("%s:%d BUG\n",__func__,__LINE__); abort(); }
      printf("\n");
      if(entry2) entry2 = entry2->next;
    }
  }
  printf("==\n");
  return;
}
#endif
