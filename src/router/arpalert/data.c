/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: data.c 690 2008-03-31 18:36:43Z  $
 *
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <arpa/inet.h>

#include "arpalert.h"
#include "data.h"
#include "log.h"
#include "loadconfig.h"
#include "func_time.h"
#include "func_str.h"

// hash table size (must be a primary number)
#define HASH_SIZE 4096

// actual number of datas
unsigned int data_size;

extern int errno;

//unsigned int data_mac_hash(data_mac *mac);
// return un resultat sur 12 bits
#define DATA_MAC_HASH(x) ({ \
   U_INT32_T a, b; \
	a = (*(U_INT16_T*)&(x)->ETHER_ADDR_OCTET[0]) ^ \
	    (*(U_INT16_T*)&(x)->ETHER_ADDR_OCTET[4]); \
	b = a + ( ( (x)->ETHER_ADDR_OCTET[2] ^ \
	    (x)->ETHER_ADDR_OCTET[3] ) << 16 ); \
	( a ^ ( b >> 12 ) ) & 0xfff; \
})

//unsigned int data_ip_hash(data_mac *mac);
// return un resultat sur 12 bits
#define DATA_IP_HASH(x) ({ \
	U_INT32_T a, b, c; \
	a = (U_INT16_T)(x) & 0xfff; \
	b = (U_INT32_T)(x) >> 20; \
	c = (U_INT8_T)(x) >> 12; \
	a ^ b ^ c; \
})

// hash mac table base
struct data_pack data_mac_tab[HASH_SIZE];

// hash ip table base
struct data_pack data_ip_tab[HASH_SIZE];

// dump mask
int dump_mask;

// next dump
struct timeval dump_time;
// last dump
struct timeval last_dump;
// next cleaning
struct timeval clean_time;

// contain base index of timeouts
struct data_pack __timeouts;
struct data_pack *timeouts = &__timeouts;

// init memory
void data_init(void){
	int i;
	struct data_pack *data;

	// init data structurs
	memset(data_mac_tab, 0, HASH_SIZE * sizeof(struct data_element *));
	memset(data_ip_tab, 0, HASH_SIZE * sizeof(struct data_element *));

	i = 0;
	while(i < HASH_SIZE){
		data = &data_mac_tab[i];
		data->next_mac = data;
		data->prev_mac = data;
		data = &data_ip_tab[i];
		data->next_ip = data;
		data->prev_ip = data;
		i++;
	}
	
	data_size = 0;

	// init timeout chain
	timeouts->next_chain = timeouts;
	timeouts->prev_chain = timeouts;

	// set next dump
	dump_time.tv_sec = -1;
	// set last dump
	last_dump.tv_sec = -1;

	// compute mask of allowed data dump
	dump_mask = config[CF_DMPBL].valeur.integer * DENY +
	            config[CF_DMPWL].valeur.integer * ALLOW +
	            config[CF_DMPAPP].valeur.integer * APPEND;
}

// index timeouts
void index_timeout(struct data_pack *data){

	// set actual time
	data->timestamp.tv_sec = current_t.tv_sec;
	data->timestamp.tv_usec = current_t.tv_usec;

	// unindex
	data->prev_chain->next_chain = data->next_chain;
	data->next_chain->prev_chain = data->prev_chain;

	// reindex
	timeouts->prev_chain->next_chain = data;
	data->prev_chain = timeouts->prev_chain;
	data->next_chain = timeouts;
	timeouts->prev_chain = data;
}

// allow to dump data
void data_rqdump(void){
	dump_time.tv_sec = last_dump.tv_sec +
	                   config[CF_DUMP_INTER].valeur.integer;
	dump_time.tv_usec = last_dump.tv_usec;
	if(dump_time.tv_sec < 0){
			dump_time.tv_sec = 0;
			dump_time.tv_usec = 0;
	}
}

// free memory
void data_reset(void){
	struct data_pack *del;
	struct data_pack *delnext;
	struct data_pack *base;
	int step;

	// free all nodes
	step = 0;
	while(step < HASH_SIZE){
		base = &data_mac_tab[step];
		delnext = base->next_mac;
		while(delnext != base){
			del = delnext;
			delnext = delnext->next_mac;
			free(del);
		}
		step++;
	}

	// reinit structurs
	step = 0;
	while(step < HASH_SIZE){
		base = &data_mac_tab[step];
		base->next_mac = base;
		base->prev_mac = base;
		base = &data_ip_tab[step];
		base->next_ip = base;
		base->prev_ip = base;
		step++;
	}
}

// update mac address in hash
void data_update_field(struct ether_addr *mac, int status,
                    struct in_addr ip,
                    U_INT32_T field, struct capt *idcap){
	struct data_pack *datap;
	#ifdef DEBUG 
	char buf[MAC_SIZE];
	#endif
	
	// check if this mac exists
	datap = data_exist(mac, idcap);

	if(datap == NULL){
		datap = data_add(mac, status, ip, idcap);
	} else {
		datap->alerts = field;
		datap->flag = status;
		unindex_ip(ip, idcap);
		datap->ip.s_addr = ip.s_addr;
		index_ip(datap);
		#ifdef DEBUG
		MAC_TO_STR(*mac, buf);
		logmsg(LOG_DEBUG, "[%s %d %s] address %s updated",
		       __FILE__, __LINE__, __FUNCTION__, buf);
		#endif
	}
}

// add mac address in hash
void data_add_field(struct ether_addr *mac, int status,
                    struct in_addr ip,
                    U_INT32_T field, struct capt *idcap){
	struct data_pack *datap;
	
	datap = data_add(mac, status, ip, idcap);
	datap->alerts = field;
}

// add mac address in hash whith discover time
void data_add_time(struct ether_addr *mac, int status,
                   struct in_addr ip, struct capt *idcap,
                   struct timeval *tv){
	struct data_pack *datap;

	datap = data_add(mac, status, ip, idcap);
	datap->timestamp.tv_sec = tv->tv_sec;
	datap->timestamp.tv_usec = tv->tv_usec;
}


// add ip in index
void index_ip(struct data_pack *to_index){
	struct data_pack *find;
	int hash;

	// delete hash entry
	to_index->prev_ip->next_ip = to_index->next_ip;
	to_index->next_ip->prev_ip = to_index->prev_ip;

	// calculate ip hash and index data
	hash = DATA_IP_HASH(to_index->ip.s_addr);
	find = &data_ip_tab[hash];
	to_index->prev_ip = find;
	to_index->next_ip = find->next_ip;
	find->next_ip->prev_ip = to_index;
	find->next_ip = to_index;
}

// delete indexed ip
void unindex_ip(struct in_addr ip, struct capt *idcap){
	struct data_pack *next;
	struct data_pack *base;
	int hash;
		  
	// calculate ip hash
	hash = DATA_IP_HASH(ip.s_addr);
	base = &data_ip_tab[hash];

	// find entry
	next = base->next_ip;
	while(next != base &&
	      next->ip.s_addr != ip.s_addr &&
	      next->cap_id == idcap) {
		next = next->next_ip;
	}

	// delete hash entry
	if(next != base){
		next->prev_ip->next_ip = next->next_ip;
		next->next_ip->prev_ip = next->prev_ip;
		next->prev_ip = next;
		next->next_ip = next;
	}
}

// clean all detected mac adresses
// emergency procedure
void data_flush(void){
	struct data_pack *base;
	struct data_pack *clean;
	struct data_pack *next;
	int step;

	// parse hash table
	step = 0;
	while(step < HASH_SIZE){
		base = &data_mac_tab[step];
		clean = base->next_mac;
		while(clean != base){
			if(clean->flag == APPEND){

				// get next
				next = clean->next_mac;

				// unindex ip hash
				clean->prev_ip->next_ip = clean->next_ip;
				clean->next_ip->prev_ip = clean->prev_ip;

				// unindex mac hash
				clean->prev_mac->next_mac = clean->next_mac;
				clean->next_mac->prev_mac = clean->prev_mac;

				// unindex in timeout list
				clean->prev_chain->next_chain = clean->next_chain;
				clean->next_chain->prev_chain = clean->prev_chain;

				// remove data
				free(clean);
				data_size--;

				// next
				clean = next;

			} else {
				// get next data
				clean = clean->next_mac;
			}
		}
		step++;
	}
	data_rqdump();
}

// add mac address in hash
struct data_pack *data_add(struct ether_addr *mac, int status,
                           struct in_addr ip, struct capt *idcap){
	struct data_pack *add;
	struct data_pack *libre;
	int mac_hash;
	#ifdef DEBUG 
	char buf[MAC_SIZE];
	#endif

	add = data_exist(mac, idcap);
	if (add != NULL)
		return add;

	if(data_size >= config[CF_MAXENTRY].valeur.integer){
		logmsg(LOG_ERR, "memory up to %i entries: flushing data",
		       config[CF_MAXENTRY].valeur.integer);
		data_flush();
	}
	
	// allocate memory for new data
	libre = (struct data_pack *)malloc(sizeof(struct data_pack));
	if(libre == NULL){
		logmsg(LOG_ERR, "[%s %i] malloc[%d]: %s",
		       __FILE__, __LINE__, 
		       errno, strerror(errno));
		exit(1);
	}

	// make data structur
	memset(libre, 0, sizeof(struct data_pack));
	DATA_CPY(&libre->mac, mac);
	libre->flag = status;
	libre->ip.s_addr = ip.s_addr;
	libre->cap_id = idcap;
	data_size++;

	// calculate mac hash and add data
	mac_hash = DATA_MAC_HASH(mac);
	add = &data_mac_tab[mac_hash];
	libre->prev_mac = add;
	libre->next_mac = add->next_mac;
	add->next_mac->prev_mac = libre;
	add->next_mac = libre;
	
	// index time
	libre->next_chain = libre;
	libre->prev_chain = libre;
	index_timeout(libre);

	// index ip
	libre->next_ip = libre;
	libre->prev_ip = libre;
	index_ip(libre);
	
	#ifdef DEBUG
	MAC_TO_STR(mac[0], buf);
	logmsg(LOG_DEBUG, "[%s %d %s] address %s add in hashs",
	       __FILE__, __LINE__, __FUNCTION__, buf);
	#endif

	return(libre);
}

// dump hash data
void data_dump(void){
	struct data_pack *base;
	struct data_pack *dump;
	int step;
	int fp;
	int len;
	char msg[128]; //mac(17) + ip(15) + interface(?) + \n + \0

	// set new times
	last_dump.tv_sec = current_t.tv_sec;
	last_dump.tv_usec = current_t.tv_usec;
	dump_time.tv_sec = -1;

	// if no data dump file
	if(config[CF_LEASES].valeur.string == NULL ||
	   config[CF_LEASES].valeur.string[0] == 0) {
		return;
	}

	#ifdef DEBUG
	logmsg(LOG_DEBUG, "[%s %d %s] running datadump",
	       __FILE__, __LINE__, __FUNCTION__);
	#endif		

	// open file
	fp = open(config[CF_LEASES].valeur.string,
	          O_WRONLY | O_CREAT | O_TRUNC, 
	          S_IRWXO  | S_IRWXG | S_IRWXU);

	// error check
	if(fp == -1){
		logmsg(LOG_ERR, "[%s %i] open[%d]: %s (%s)",
		       __FILE__, __LINE__, 
		       errno, strerror(errno),
		       config[CF_LEASES].valeur.string);
		exit(1);
	}

	// parse hash table
	step = 0;
	while(step < HASH_SIZE){
		base = &data_mac_tab[step];
		dump = base->next_mac;
		while(dump != base){
			// dump
			if( ( dump_mask & dump->flag) != 0 ){
				//if(dump->ip.s_addr != 0){
					len = snprintf(msg, 128,
					               "%02x:%02x:%02x:%02x:%02x:%02x %s %s %u %u\n",
					               dump->mac.ETHER_ADDR_OCTET[0],
					               dump->mac.ETHER_ADDR_OCTET[1],
					               dump->mac.ETHER_ADDR_OCTET[2],
					               dump->mac.ETHER_ADDR_OCTET[3],
					               dump->mac.ETHER_ADDR_OCTET[4],
					               dump->mac.ETHER_ADDR_OCTET[5], 
					               inet_ntoa(dump->ip),
					               dump->cap_id->device,
										(unsigned int)dump->timestamp.tv_sec,
										(unsigned int)dump->timestamp.tv_usec);
					write(fp, msg, len);
				//}
			}

			// get next data
			dump = dump->next_mac;
		}

		step ++;
	}

	// close file
	if(close(fp) == -1){
		logmsg(LOG_ERR, "[%s %i] close[%d]: %s",
		       __FILE__, __LINE__, 
		       errno, strerror(errno));
		exit(1);
	}
}

// clean discored mac too old
void data_clean(void){
	struct timeval old_t;
	struct data_pack *clean;
	struct data_pack *clean_next;

	#ifdef DEBUG
	logmsg(LOG_DEBUG,
	       "[%s %d %s] running data clean",
	       __FILE__, __LINE__, __FUNCTION__);
	#endif

	// calculate the time that correspond at current timeout limit
	old_t.tv_sec = current_t.tv_sec - config[CF_TOOOLD].valeur.integer;
	old_t.tv_usec = current_t.tv_usec;

	// get first element
	clean = timeouts->next_chain;
	while(clean != timeouts){
	
		if(time_comp(&old_t, &(clean->timestamp)) == BIGEST){

			// get next
			clean_next = clean->next_chain;
	
			if(clean->flag == APPEND){
				// unindex ip hash
				clean->prev_ip->next_ip = clean->next_ip;
				clean->next_ip->prev_ip = clean->prev_ip;

				// unindex mac hash
				clean->prev_mac->next_mac = clean->next_mac;
				clean->next_mac->prev_mac = clean->prev_mac;

				// unindex in timeout list
				clean->prev_chain->next_chain = clean->next_chain;
				clean->next_chain->prev_chain = clean->prev_chain;
				
				// can dump database
				data_rqdump();

				// delete data
				free(clean);
			}

			// set next
			clean = clean_next;
		}

		// if the firsts times are not biggest
		// quit beaucause the timeout are ordoned
		// by time
		else {
			return;
		}
	}
}

// get ip in ip hash
struct data_pack *data_ip_exist(struct in_addr ip,
                                struct capt *idcap){
	struct data_pack *base;
	struct data_pack *find;
	int hash;

	// calculate hash
	hash = DATA_IP_HASH(ip.s_addr);
	base = &data_ip_tab[hash];
	find = base->next_ip;
	while(find != base){
		if(find->cap_id == idcap &&
		   find->ip.s_addr == ip.s_addr){
			return find;
		}
		find = find->next_ip;
	}
	return NULL;
}

// get mac in hash
struct data_pack *data_exist(struct ether_addr *mac,
                             struct capt *idcap){
	struct data_pack *base;
	struct data_pack *find;
	int hash;

	// calculate hash
	hash = DATA_MAC_HASH(mac);
	base = &data_mac_tab[hash];
	find = base->next_mac;
	while(find != base){
		if(find->cap_id == idcap &&
		   DATA_CMP(&find->mac, mac) == 0){
			return find;
		}
		find = find->next_mac;
	}
	return(NULL);
}

// return date of next timeout
void *data_next(struct timeval *tv){
	struct timeval next_clean;
	next_clean.tv_sec = timeouts->next_chain->timestamp.tv_sec +
	                    config[CF_TOOOLD].valeur.integer;
	next_clean.tv_usec = timeouts->next_chain->timestamp.tv_usec;

	// if timeout is not set and dump is not set
	if(timeouts->next_chain == timeouts &&
	   dump_time.tv_sec == -1){
		tv->tv_sec = -1;
		return NULL;
	}
	
	// if timeout is set and dump is not set
	else if(timeouts->next_chain != timeouts &&
	        dump_time.tv_sec == -1){
		tv->tv_sec = next_clean.tv_sec;
		tv->tv_usec = next_clean.tv_usec;
		return data_clean;
	}

	// if timeout is not set and dump is set
	else if(timeouts->next_chain == timeouts &&
	        dump_time.tv_sec != -1){
		tv->tv_sec = dump_time.tv_sec;
		tv->tv_usec = dump_time.tv_usec;
		return data_dump;
	}

	// if timeout and dump are sets
	// next_clean time lowest, then clen
	else if(time_comp(&dump_time,
	                  &next_clean) == BIGEST){
		tv->tv_sec = next_clean.tv_sec;
		tv->tv_usec = next_clean.tv_usec;
		return data_clean;

	// else dump
	} else {
		tv->tv_sec = dump_time.tv_sec;
		tv->tv_usec = dump_time.tv_usec;
		return data_dump;
	}
}

