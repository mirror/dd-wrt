/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: sens_timeouts.c 690 2008-03-31 18:36:43Z  $
 *
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>

#include "arpalert.h"
#include "data.h"
#include "sens_timeouts.h"
#include "log.h"
#include "loadconfig.h"
#include "func_time.h"

/* hash table size ; this number must be primary number */
#define HASH_SIZE 4096

// hash function
#define SENS_TIMEOUT_HASH(x, y) ({ \
   U_INT32_T a, b, c; \
   a = (*(U_INT16_T*)&(x)->ETHER_ADDR_OCTET[0]) ^ \
	    (*(U_INT16_T*)&(x)->ETHER_ADDR_OCTET[4]); \
   b = a + ( ( (x)->ETHER_ADDR_OCTET[2] ^ \
	    (x)->ETHER_ADDR_OCTET[3] ) << 16 ); \
   c = a ^ ( b >> 12 ); \
   a = ((U_INT16_T)(y)) ^ ( ((U_INT32_T)(y)) >> 20 ) ^ \
	    ( ((U_INT8_T)(y)) >> 12 ); \
   ( a ^ c ) & 0xfff; \
})

extern int errno;

// structurs
struct tmouts {
	struct ether_addr mac;
	struct in_addr ip_d;
	struct timeval last;
	struct capt *idcap;
	struct tmouts *prev_hash;
	struct tmouts *next_hash;
	struct tmouts *next_chain;
	struct tmouts *prev_chain;
};

// data allocation
#define MAX_DATA 2000
struct tmouts tmouts_table[MAX_DATA];

// hash
struct tmouts tmout_h[HASH_SIZE];

// root of free nodes chain
struct tmouts __free_start;
struct tmouts *free_start = &__free_start;

// root of used nodes chain
struct tmouts __used_start;
struct tmouts *used_start = &__used_start;

// data_init
void sens_timeout_init(void) {
	int i;
	struct tmouts *gen;

	// set NULL pointers in tmout_h table and init
	memset(&tmout_h, 0, HASH_SIZE * sizeof(struct tmouts));
	i = 0;
	while(i < HASH_SIZE){
		gen = &tmout_h[i];
		gen->prev_hash = gen;
		gen->next_hash = gen;
		i++;
	}
	
	// generate free nodes list
	free_start->next_chain = free_start;
	free_start->prev_chain = free_start;
	i = 0;
	while(i < MAX_DATA){
		gen = &tmouts_table[i];
		gen->next_chain = free_start->next_chain;
		gen->prev_chain = free_start;
		free_start->next_chain->prev_chain = gen;
		free_start->next_chain = gen;
		i++;
	}

	// set used chain
	used_start->next_chain = used_start;
	used_start->prev_chain = used_start;
}

// add new timeout
void sens_timeout_add(struct ether_addr *mac, struct in_addr ipb,
                      struct capt *idcap){
	struct tmouts *new_tmout;
	struct tmouts *base;
	int hash;

	// get free timeout node
	if(free_start->next_chain == free_start){
		logmsg(LOG_WARNING,
		       "[%s %d] Timeouts table full: more than %d timeouts used",
		       __FILE__, __LINE__, MAX_DATA);
		return;
	}

	// init values
	new_tmout = free_start->next_chain;
	DATA_CPY(&new_tmout->mac, mac);
	new_tmout->idcap = idcap;
	new_tmout->ip_d.s_addr = ipb.s_addr;
	new_tmout->last.tv_sec = current_t.tv_sec +
	                         config[CF_ANTIFLOOD_INTER].valeur.integer;
	new_tmout->last.tv_usec = current_t.tv_usec;

	// delete entrie fron free list
	new_tmout->prev_chain->next_chain = new_tmout->next_chain;
	new_tmout->next_chain->prev_chain = new_tmout->prev_chain;

	// add entrie in hash 
	hash = SENS_TIMEOUT_HASH(mac, ipb.s_addr);
	base = &tmout_h[hash];
	base->next_hash->prev_hash = new_tmout;
	new_tmout->next_hash = base->next_hash;
	base->next_hash = new_tmout;
	new_tmout->prev_hash = base;
	
	// add timeout at the end of chain list
	used_start->prev_chain->next_chain = new_tmout;
	new_tmout->prev_chain = used_start->prev_chain;
	used_start->prev_chain = new_tmout;
	new_tmout->next_chain = used_start;
}

// check if entry is present in timeout hash
int sens_timeout_exist(struct ether_addr *mac, struct in_addr ipb,
                       struct capt *idcap){
	int hash;
	struct tmouts *base;
	struct tmouts *find;

	// if timeout entrie exist: is not expired
	hash = SENS_TIMEOUT_HASH(mac, ipb.s_addr);
	base = &tmout_h[hash];
	find = base->next_hash;

	while(find != base){
		if(find->ip_d.s_addr == ipb.s_addr &&
		   find->idcap == idcap &&
		   DATA_CMP(&(find->mac), mac) == DATA_EQUAL){
			return(TRUE);
		}
		find = find->next_hash;
	}
	return(FALSE);
}

// delete timeouts expires
void sens_timeout_clean(void) {
	struct tmouts *look;
	struct tmouts *look_next;

	look = used_start->next_chain;

	while(look != used_start){ 

		// if entry expires
		if(time_comp(&current_t, &(look->last)) == BIGEST){

			// get next
			look_next = look->next_chain;

			// delete from hash
			look->prev_hash->next_hash = look->next_hash;
			look->next_hash->prev_hash = look->prev_hash;
		
			// delete from used list
			look->prev_chain->next_chain = look->next_chain;
			look->next_chain->prev_chain = look->prev_chain;
		
			// move the structur in free
			free_start->next_chain->prev_chain = look;
			look->next_chain = free_start->next_chain;
			free_start->next_chain = look;
			look->prev_chain = free_start;

			// set next
			look = look_next;
		}

		// else quit (the entry are sorted by expiration date)
		else {
			return;
		}
	}
}

// get next timeout
void *sens_timeout_next(struct timeval *tv){
	struct tmouts *look;
	
	look = used_start->next_chain;
	if(look != used_start){
		tv->tv_sec = look->last.tv_sec;
		tv->tv_usec = look->last.tv_usec;
	}

	else {
		tv->tv_sec = -1;
	}
	
	return sens_timeout_clean;
}

