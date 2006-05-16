/*
 * Copyright (c) 2001, 2002, 2003, 2004  Netli, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: genhash.c,v 1.1 2004/05/06 13:45:53 vlm Exp $
 */
/*
 * Implementation of a hash data structure.
 * This particular implementation is supposed to be space-efficient
 * particularly in the case of tiny number of hash elements.
 * It also has an aggressive hash buckets expanding technique, which allows
 * to deal with increasing number of elements without a loss of search speed.
 *
 * Generally, one structure of type genhash_t is allocated per hash set.
 * This structure is supposed to hold all information related to the current
 * set, and also holds a tiny number of hash elements, when hash hasn't yet
 * grown up. When the number of elements reaches some point, part of the
 * genhash_t structure is reused to contain the pointers to the actual
 * hash buckets and LRU (Least Recently Used) list's head and tail.
 * Elements which were held inside genhash_t will be moved to the hash buckets.
 * 
 * Said above effectively means two modes of operation: TINY and NORMAL.
 * They can be distinguished by examining the h->numbuckets value, which
 * is 0 for TINY and greater for NORMAL mode.
 * 
 * In the TINY mode we use a lower part of the genhash_t structure
 * (lower 32 bytes from 64 bytes of genhash_t) to hold up to IH_VALUE (4)
 * key/value pairs.
 * 
 * In the NORMAL mode we use the lower part of the genhash_t structure
 * to hold a set of pointers, including a pointer to the hash buckets.
 * We agressively expand hash buckets size when adding new elements
 * to lower the number of key comparisons.
 */

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "genhash.h"

/* 1M entries, 4M RAM */
#define	DEFAULT_MAXIMUM_HASH_BUCKETS_NUMBER	(1024 * 1024)
static int maximum_hash_buckets_number = DEFAULT_MAXIMUM_HASH_BUCKETS_NUMBER;

/*
 * A single hash element structure which binds a value to its key.
 */
typedef struct genhash_el_s {
	void *key;
	void *value;
	struct genhash_el_s *hash_next;
	struct genhash_el_s *hash_prev;
	struct genhash_el_s *lru_prev;
	struct genhash_el_s *lru_next;
	int key_hash;	/* Save hash of the key */
} genhash_el;

#define	IH_VALUES	4  /* Internally held key/value pairs for TINY mode */

/*
 * A hash structure with buckets etc.
 */
struct genhash_s {
	int (*keycmpf) (const void *lkey1, const void *rkey2);  /* compare */
	int (*keyhashf) (const void *key);		/* hash function */
	void (*keydestroyf) (void *key);	/* key destructor */
	void (*valuedestroyf) (void *value);	/* value destructor */

	int numelements;
	int numbuckets;	/* 0 means "use internal" */
	int lru_limit;	/* Must be initialized separately */
	union {
		int tiny_walkel;
		int normal_direction;
	} un1;

	/* 32-byte boundary here */

	union {
		struct _internal_tiny_s {
			void *keys[IH_VALUES];
			void *values[IH_VALUES];
		} _TINY;	/* 32-byte structure */
		struct _internal_normal_s {
			genhash_el *lru_head;	/* LRU list head */
			genhash_el *lru_tail;	/* LRU list tail */
			genhash_el *walkel;	/* Current for genhash_walk() */
			genhash_el **buckets;	/* Hash buckets */
		} _NORMAL;
	} un2;
#define	tiny_walkel	un1.tiny_walkel
#define	normal_direction	un1.normal_direction
#define	tiny_keys	un2._TINY.keys
#define	tiny_values	un2._TINY.values
#define	lru_head	un2._NORMAL.lru_head
#define	lru_tail	un2._NORMAL.lru_tail
#define	walkel		un2._NORMAL.walkel
#define	buckets		un2._NORMAL.buckets
};


static int
_genhash_normal_add(genhash_t *h, genhash_el *el,
	const void *key, const void *value);


genhash_t *
genhash_new(
	int (*keycmpf) (const void *key1, const void *key2),
	int (*keyhashf) (const void *key),
	void (*keydestroyf) (void *key),
	void (*valuedestroyf) (void *value)
) {
	genhash_t *h;

	h = (genhash_t *)malloc(sizeof(genhash_t));
	if (!h)
		return NULL;

	memset(h, 0, sizeof(genhash_t));

	genhash_reinit(h, keycmpf, keyhashf, keydestroyf, valuedestroyf);
  
	return h;
}

int
genhash_reinit(
	genhash_t *h,
	int (*keycmpf) (const void *key1, const void *key2),
	int (*keyhashf) (const void *key),
	void (*keydestroyf) (void *key),
	void (*valuedestroyf) (void *value)
) {

	assert(keycmpf && keyhashf);

	h->keycmpf = keycmpf;
	h->keyhashf = keyhashf;
	h->keydestroyf = keydestroyf;
	h->valuedestroyf = valuedestroyf;
  
	return 0;
}

int
genhash_count(genhash_t *h) {
	if(h) {
		return h->numelements;
	} else {
		return 0;
	}
}


static void
_remove_normal_hash_el(genhash_t *h, genhash_el *el) {
	void *kd_arg;
	void *vd_arg;

	/* Found it; remove from lists and destruct as specified */
	if (el->hash_prev) {
		if((el->hash_prev->hash_next = el->hash_next))
			el->hash_next->hash_prev = el->hash_prev;
		
	} else {
		if((h->buckets[el->key_hash % h->numbuckets] = el->hash_next))
			el->hash_next->hash_prev = NULL;
	}

	/* Remove from LRU list */
	if(el->lru_prev) {
		if((el->lru_prev->lru_next = el->lru_next))
			el->lru_next->lru_prev = el->lru_prev;
		else
			h->lru_tail = el->lru_prev;
	} else {
		if(h->lru_head == el) {
			if((h->lru_head = el->lru_next) == NULL)
				h->lru_tail = NULL;
			else
				h->lru_head->lru_prev = NULL;
		}
	}

	/* Remember key and value */
	kd_arg = el->key;
	vd_arg = el->value;

	free(el);
	h->numelements--;

	/* Remove key and value */
	if (h->keydestroyf)
		h->keydestroyf(kd_arg);
	if (h->valuedestroyf)
		h->valuedestroyf(vd_arg);
}

static inline void
_genhash_normal_el_move2top(genhash_t *h, genhash_el *el) {

	/* Move to the top of the hash bucket */
	if(el->hash_prev) {
		int bucket = el->key_hash % h->numbuckets;

		/* Remove from the current location */
		if((el->hash_prev->hash_next = el->hash_next))
			el->hash_next->hash_prev = el->hash_prev;

		/* Move to the top of the hash bucket */
		if((el->hash_next = h->buckets[bucket]))
			el->hash_next->hash_prev = el;
		h->buckets[bucket] = el;
		el->hash_prev = NULL;
	}

	/* Move to the top of LRU list */
	if(h->lru_limit && el->lru_prev) {

		/* Remove from current location */
		if((el->lru_prev->lru_next = el->lru_next))
			el->lru_next->lru_prev = el->lru_prev;
		else
			h->lru_tail = el->lru_prev;
	
		/* Append to the head */
		el->lru_prev = NULL;
		h->lru_head->lru_prev = el;
		el->lru_next = h->lru_head;
		h->lru_head = el;
	}
}

static int
_expand_hash(genhash_t *h) {
	int newbuckets_count;
	genhash_el **newbuckets;

	/*
	 * Compute a new number of buckets value.
	 */
	if(h->numbuckets) {
		newbuckets_count = h->numbuckets << 2;
		/* Too big hash table */
		if(newbuckets_count > maximum_hash_buckets_number) {
			if(h->numbuckets < maximum_hash_buckets_number) {
				newbuckets_count = maximum_hash_buckets_number;
			} else {
				/* No need to set errno here. */
				return -1;
			}
		}
	} else {
		/* 8 buckets -> 16 bytes of memory (likely, a malloc block) */
		newbuckets_count = IH_VALUES << 1;
		if(newbuckets_count > maximum_hash_buckets_number) {
			if(maximum_hash_buckets_number) {
				newbuckets_count = maximum_hash_buckets_number;
			} else {
				/* Allowed to store only 4 elements */
				errno = EPERM;
				return -1;
			}
		}
	}

	/*
	 * Allocate a new storage for buckets.
	 */
	newbuckets = (genhash_el **)malloc(newbuckets_count
		* sizeof(genhash_el *));
	if(newbuckets) {
		memset(newbuckets, 0, newbuckets_count * sizeof(genhash_el *));
	} else {
		return -1;
	}

	if(h->numbuckets) {
		genhash_el *el;
		int bucket;
		/*
		 * Rehash elements from old h->buckets to newbuckets.
		 * No need to touch LRU pointers and other staff - it is okay.
		 */

		for(el = h->lru_tail; el; el = el->lru_prev) {
			bucket = el->key_hash % newbuckets_count;
			el->hash_prev = NULL;
			if((el->hash_next = newbuckets[bucket]))
				newbuckets[bucket]->hash_prev = el;
			newbuckets[bucket] = el;
		}

		free(h->buckets);
		h->buckets = newbuckets;
		h->numbuckets = newbuckets_count;

		return 0;
	} else {
		/*
		 * Moving from inline tiny storage into buckets.
		 */
		genhash_el *els[IH_VALUES] = { NULL };
		struct _internal_tiny_s tiny_substruct;
		int i;
		int saved_numelements;
		int saved_lru_limit;

		/* Pre-allocate hash elements */
		for(i = 0; i < h->numelements; i++) {
			els[i] = (genhash_el *)malloc(sizeof(genhash_el));
			if(els[i] == NULL) {
				for(i = 0; i < h->numelements; i++)
					if(els[i])
						free(els[i]);
				free(newbuckets);
				return -1;
			}
		}

		/* Save part of the union */
		tiny_substruct = h->un2._TINY;
		/* Re-initialize this part in NORMAL model */
		memset(&h->un2._NORMAL, 0, sizeof(h->un2._NORMAL));

		/* There was no allocated buckets, when in tiny hash mode. */
		h->buckets = newbuckets;
		h->numbuckets = newbuckets_count;

		saved_numelements = h->numelements;
		saved_lru_limit = h->lru_limit;
		h->numelements = 0;
		h->lru_limit = 0;	/* Disable LRU expiration for a while */

		for(i = 0; i < saved_numelements; i++) {
			/*
			 * genhash_normal_add won't fail, if we supply
			 * an already allocated genhash_el *.
			 */
			(void)_genhash_normal_add(h, els[i],
				tiny_substruct.keys[i],
				tiny_substruct.values[i]);
		}

		h->lru_limit = saved_lru_limit;

		return 0;
	}
}

/*
 * Won't return with error if el is provided.
 */
static int
_genhash_normal_add(genhash_t *h, genhash_el *el,
		const void *key, const void *value) {
	genhash_el **bucket;

	if(el == NULL) {
		el = (genhash_el *)malloc(sizeof(genhash_el));
		if(el == NULL) {
			/* Errno will be set by malloc() */
			return -1;
		}
	}

	memset(el, 0, sizeof(genhash_el));

	/* Make it positive */
	el->key_hash = h->keyhashf(key) & 0x7fffffff;
	bucket = &h->buckets[el->key_hash % h->numbuckets];

	//(const void *)
	el->key = key;
	//(const void *)
	el->value = value;

	el->hash_prev = NULL;
	if((el->hash_next = *bucket))
		(*bucket)->hash_prev = el;
	*bucket = el;

	h->numelements++;

	/*
	 * Add to the LRU list.
	 */
	if(h->lru_head) {
		el->lru_next = h->lru_head;
		h->lru_head->lru_prev = el;
		h->lru_head = el;
	} else {
		h->lru_head = el;
		h->lru_tail = el;
	}

	if(h->lru_limit) {
		while((h->numelements > h->lru_limit)
			&& h->lru_head != h->lru_tail)
		  _remove_normal_hash_el(h, h->lru_tail);
	}

	return 0;
}


int
genhash_add(genhash_t *h, const void *key, const void *value) {

	if(key == NULL) {
		errno = EINVAL;
		return -1;
	}

	if(h->numbuckets == 0) {
		/* We have a tiny internally-held set of elements */

		if(h->numelements < IH_VALUES) {
			//(const void *)
			h->tiny_keys[h->numelements] = key;
			//(const void *)
			h->tiny_values[h->numelements] = value;
			h->numelements++;
			return 0;
		}

		if(_expand_hash(h) == -1)
			return -1;

	} else {

		if((h->numelements / h->numbuckets) > 2)
			(void)_expand_hash(h);
	}

	return _genhash_normal_add(h, NULL, key, value);
}

int
genhash_addunique(genhash_t *h, const void *key, const void *value) {
	if(genhash_get(h, key)) {
		errno = EEXIST;
		return -1;
	}
	return genhash_add(h, key, value);
}

void *
genhash_get(genhash_t *h, const void *key) {

	if(h->numbuckets == 0) {
		int i;

		/* hope, h->numelements <= IH_VALUES */
		for(i = 0; i < h->numelements; i++) {
			if(h->keycmpf(h->tiny_keys[i], key) == 0) {
				void *return_value = h->tiny_values[i];
				if(i) {
					/* Switch with the first value */
					void *tmp_key, *tmp_value;
					tmp_key = h->tiny_keys[0];
					tmp_value = h->tiny_values[0];
					h->tiny_keys[0] = h->tiny_keys[i];
					h->tiny_values[0] = h->tiny_values[i];
					h->tiny_keys[i] = tmp_key;
					h->tiny_values[i] = tmp_value;
				}
				return return_value;
			}
		}

	} else {
		genhash_el *walk;
		int bucket = (h->keyhashf(key) & 0x7fffffff)
			% h->numbuckets;

		for(walk = h->buckets[bucket];
			walk; walk = walk->hash_next) {
	
			if (h->keycmpf(walk->key, key) == 0) {
				_genhash_normal_el_move2top(h, walk);
				return walk->value;
			}
		}

	}

	errno = ESRCH;
	return NULL;
}

int
genhash_del(genhash_t *h, const void *key) {

	if(h->numbuckets == 0) {
		int i;

		for(i = 0; i < h->numelements; i++) {
			if(h->keycmpf(h->tiny_keys[i], key) == 0) {
				/* Remember values */
				void *kd_arg = h->tiny_keys[i];
				void *vd_arg = h->tiny_values[i];

				/* Substitute it with the last one */
				h->numelements--;
				/* No harm if overwriting itself */
				h->tiny_keys[i] = h->tiny_keys[h->numelements];
				h->tiny_values[i] = h->tiny_values[h->numelements];

				/* Delete in real */
				if(h->keydestroyf)
					h->keydestroyf(kd_arg);
				if(h->valuedestroyf)
					h->valuedestroyf(vd_arg);

				return 0;
			}
		}

		errno = ESRCH;
		return -1;
	} else {
		genhash_el *walk;
		int bucket;

		if(h->numelements == 0) {
			errno = ESRCH;
			return -1;	/* not found */
		}
	
		bucket = (h->keyhashf(key) & 0x7fffffff)
			% h->numbuckets;
	
		for (walk = h->buckets[bucket];
			walk; walk = walk->hash_next)
			if (h->keycmpf(walk->key, key) == 0)
				break;
	
		if(walk == NULL) {
			errno = ESRCH;
			return -1; /* not found */
		}
	
		_remove_normal_hash_el(h, walk);

		return 0;
	}

	return 0;
}

int
genhash_walk_init(genhash_t *h, int direction) {

	if(h->numbuckets == 0) {
		h->tiny_walkel = 0;
	} else {
		if((h->normal_direction = direction) == 0) {
			/* Most recent first order */
			h->walkel = h->lru_head;
		} else {
			/* Least recent first order */
			h->walkel = h->lru_tail;
		}
	}

	return h->numelements;
}

int
genhash_walk(genhash_t *h, void *key_p, void *val_p) {
	void **key = key_p;
	void **val = val_p;

	if(h->numbuckets == 0) {
		if((h->tiny_walkel >= h->numelements)
			|| (h->tiny_keys[h->tiny_walkel] == NULL))
			return 0;

		if(key)
			*key = h->tiny_keys[h->tiny_walkel];
		if(val)
			*val = h->tiny_values[h->tiny_walkel];
		
		h->tiny_walkel++;
	} else {
		if(h->walkel == NULL)
			/* Already finished */
			return 0;

		if(key)
			*key = h->walkel->key;
		if(val)
			*val = h->walkel->value;

		h->walkel = (h->normal_direction == 0)
			? h->walkel->lru_next
			: h->walkel->lru_prev;
	}

	return 1;
}

int
genhash_iter_init(genhash_iter_t *iter, genhash_t *h, int direction) {

	iter->__hash_ptr = h;
	iter->__direction = direction;

	if(h->numbuckets == 0) {
		iter->__un.__item_number = 0;
	} else {
		if(direction == 0) {
			/* Most recent first order */
			iter->__un.__location = h->lru_head;
		} else {
			/* Least recent first order */
			iter->__un.__location = h->lru_tail;
		}
	}

	return h->numelements;
}

int
genhash_iter(genhash_iter_t *iter, void *key_p, void *val_p) {
	void **key = key_p;
	void **val = val_p;
	genhash_t *h = iter->__hash_ptr;

	if(h->numbuckets == 0) {
		if((iter->__un.__item_number >= h->numelements)
			|| (h->tiny_keys[iter->__un.__item_number] == NULL))
			return 0;

		if(key)
			*key = h->tiny_keys[iter->__un.__item_number];
		if(val)
			*val = h->tiny_values[iter->__un.__item_number];

		iter->__un.__item_number++;
	} else {
		if(iter->__un.__location == NULL)
			/* Already finished */
			return 0;

		if(key)
			*key = ((genhash_el *)iter->__un.__location)->key;
		if(val)
			*val = ((genhash_el *)iter->__un.__location)->value;

		iter->__un.__location = (iter->__direction == 0)
			? ((genhash_el *)iter->__un.__location)->lru_next
			: ((genhash_el *)iter->__un.__location)->lru_prev;
	}


	return 1;
}


int
genhash_set_lru_limit(genhash_t *h, int value) {
	if(h) {
		int prev_limit = h->lru_limit;
		if(value >= 0)
			h->lru_limit = value;
		return prev_limit;
	} else {
		errno = EINVAL;
		return -1;
	}
}
/* Deprecated function */
void genhash_set_limit(genhash_t *, int);
void genhash_set_limit(genhash_t *h, int v) { genhash_set_lru_limit(h, v); }

int
genhash_set_buckets_limit(int value) {
	int prev_limit = maximum_hash_buckets_number;
	if(value > 0) {
		maximum_hash_buckets_number = value;
	}
	return prev_limit;
}

void
genhash_destroy(genhash_t *h) {
	if(h) {
		genhash_empty(h, 1, 1);
		free(h);
	}
}

void
genhash_empty(genhash_t *h, int freekeys, int freevalues) {

	if(h == NULL) return;

	/*
	 * Don't free what could not be freed.
	 */
	if(h->keydestroyf == NULL)
		freekeys = 0;
	if(h->valuedestroyf == NULL)
		freevalues = 0;

	if(h->numbuckets == 0) {
		while(h->numelements > 0) {
			int n = --h->numelements;
			void *kd_arg = h->tiny_keys[n];
			void *vd_arg = h->tiny_values[n];

			if (freekeys)
				h->keydestroyf(kd_arg);
			if (freevalues)
				h->valuedestroyf(vd_arg);
		}
	} else {
		genhash_el *el, *el_next;

		for(el = h->lru_head; el; el = el_next) {
			void *kd_arg = el->key;
			void *vd_arg = el->value;
			el_next = el->lru_next;
			free(el);

			h->numelements --;

			if (freekeys)
				h->keydestroyf(kd_arg);
			if (freevalues)
				h->valuedestroyf(vd_arg);
		}
		free(h->buckets);
		h->lru_head = NULL;
		h->lru_tail = NULL;

		memset(&h->un2, 0, sizeof(h->un2));
		h->numbuckets = 0;
	}
	assert(h->numelements == 0);
}


/*----- Simple hash and compare functions for common data types ------*/

int
hashf_int (const void *key) {
	return (*(const int *)key ^ (*(const int *)key >> 16));
}

int
cmpf_int (const void *key1, const void *key2) {
	return (*(const int *)key1 != *(const int *)key2);
}

int
hashf_void (const void *key) {
	return ((int)key ^ ((int)key >> 16));
}

int
cmpf_void (const void *key1, const void *key2) {
	return (key1 != key2);
}


/*
 * Phong's linear congruential hash
 */
#define dcharhash(h, c) ((h) = 0x63c63cd9*(h) + 0x9c39c33d + (c))

int
hashf_string(const void *keyarg) {
	register const unsigned char *key;
	register unsigned int h;
	register unsigned char c;

	key = keyarg;
	for (h = 0; (c = *key++);)
		dcharhash(h, c);

	return (h);
}

int
cmpf_string(const void *key1, const void *key2) {
	return strcmp((const char *)key1, (const char *)key2);
}

