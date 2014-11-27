/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: macname.c 690 2008-03-31 18:36:43Z  $
 *
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#include "loadconfig.h"
#include "macname.h"
#include "func_str.h"
#include "log.h"

extern int errno;

#define BUFFER_SIZE 1024
#define HASH_SIZE   4096

#define MACNAME_HASH(x, y, z) \
	((((U_INT8_T)(y) << 4 ) ^ ((U_INT8_T)(z))) & 0xfff); \

struct macname {
	U_INT8_T octet[3];
	char * name;
	struct macname *next;
	struct macname *prev;
};

struct macname macname_base[HASH_SIZE];

// init data
void macname_init(void){
	bzero(macname_base, 
	      sizeof(struct macname) * HASH_SIZE);
}

/* load mac vendor name database
 * @param mode: MACNAME_TEST: test loading file
 *              MACNAME_LOAD: load file
 *
 * @return:  0 load is ok
 *          -1 load error
 */
int macname_load(int mode){
	FILE *file;
	char buf[BUFFER_SIZE];
	char *args[4];
	U_INT8_T octet[3];
	int ligne;
	int i;
	int hash;
	int ret;
	struct macname *base;
	struct macname *next;
	struct macname *nextnext;
	struct macname *libre;

	// free and init/reinit data 
	if(mode == MACNAME_LOAD){
		i = 0;
		while(i < HASH_SIZE){
			base = &macname_base[i];
			next = base->next;
			if(next != NULL){
				while(next != base){
					nextnext = next->next;
					if(next->name != NULL){
						free(next->name);
					}
					free(next);
					next = nextnext;
				}
			}
			base->next = base;
			base->prev = base;

			i++;
		}
	}

	// check option avalaibility
	if(config[CF_MACCONV_FILE].valeur.string == NULL ||
	   config[CF_MACCONV_FILE].valeur.string[0] == 0){
		config[CF_LOG_VENDOR].valeur.integer = FALSE;
		config[CF_ALERT_VENDOR].valeur.integer = FALSE;
		config[CF_MOD_VENDOR].valeur.integer = FALSE;
		return 0;
	}

	if(config[CF_LOG_VENDOR].valeur.integer == FALSE &&
	   config[CF_ALERT_VENDOR].valeur.integer == FALSE &&
	   config[CF_MOD_VENDOR].valeur.integer == FALSE){
		return 0;
	}

	// open file
	file = fopen(config[CF_MACCONV_FILE].valeur.string, "r");
	if(file == NULL){
		logmsg(LOG_ERR, "[%s %d] fopen[%d]: %s (%s)",
		       __FILE__, __LINE__,
		       errno, strerror(errno),
		       config[CF_MACCONV_FILE].valeur.string);
		return -1;
	}

	// lines counter
	ligne = 0;

	// line format:
	// /^[0-9A-F]{2}-[0-9A-F]{2}-[0-9A-F]{2}   (hex)\t\t.*\n$/
	// 09-9A-AF   (hex)		Vendor Name

	// for each line ..
	while(feof(file) == 0){
		fgets(buf, BUFFER_SIZE, file);
		ligne ++;

		// check validity of line begin (mac)
		i = 0;
		while(i < 9){
			switch(i){
				case 0:
				case 1:
				case 3:
				case 4:
				case 6:
				case 7:
					if((buf[i] < '0' || buf[i] > '9') &&
					   (buf[i] < 'A' || buf[i] > 'F')){
						goto end_of_mac_read;
					}
					break;
				
				case 2:
				case 5:
					if(buf[i] != '-'){
						goto end_of_mac_read;
					}
					break;
			}
			i++;
		}

		// validity of line end (blabla)
		if(memcmp("   (hex)\t\t", &buf[8], 10) != 0){
			goto end_of_mac_read;
		}

		// decode
		args[0] = &buf[0];
		buf[2] = 0;
		args[1] = &buf[3];
		buf[5] = 0;
		args[2] = &buf[6];
		buf[8] = 0;
		args[3] = &buf[18];
		i = 18;
		while(buf[i] != '\r' && buf[i] != '\n' && buf[i] != 0){
			i++;
		}
		buf[i] = 0;

		// convert hexa string to int
		i = 0;
		while(i < 3){
			ret = strhex_to_int(args[i]);
			if(ret == -1){
				logmsg(LOG_ERR,
				       "file \"%s\", line %d: mac conversion error",
				       config[CF_MACCONV_FILE].valeur.string,
				       ligne);
				return -1;
			}
			octet[i] = strhex_to_int(args[i]);
			i++;
		}

		// insert data
		if(mode == MACNAME_LOAD){
			// load
			#ifdef DEBUG
			logmsg(LOG_DEBUG, 
			       "load %s:%s:%s => %s",
			       args[0], args[1], args[2],
			       args[3]);
			#endif

			// allocate memory for new data
			libre = (struct macname *)malloc(sizeof(struct macname));
			if(libre == NULL){
				logmsg(LOG_ERR, "[%s %i] malloc[%d]: %s",
				       __FILE__, __LINE__,
				       errno, strerror(errno));
				exit(1);
			}

			// set data
			libre->octet[0] = octet[0];
			libre->octet[1] = octet[1];
			libre->octet[2] = octet[2];
			libre->name = strdup(args[3]);

			// insert data
			hash = MACNAME_HASH(octet[0], octet[1], octet[2]);
			base = &macname_base[hash];
			libre->prev = base->prev;
			base->prev->next = libre;
			libre->next = base;
			base->prev = libre;
		}

		end_of_mac_read: ;
	}	

	// close file
	if(fclose(file) == EOF){
		logmsg(LOG_ERR, "[%s %d] fclose[%d]: %s",
		       __FILE__, __LINE__, errno, strerror(errno));
		exit(1);
	}														 

	return 0;
}

// reload database
void macname_reload(void){
	if(macname_load(MACNAME_TEST) == 0){
		macname_load(MACNAME_LOAD);
	}
}

char *get_vendor(struct ether_addr *mac){
	int hash;
	struct macname *base;
	struct macname *next;

	hash = MACNAME_HASH(mac->ETHER_ADDR_OCTET[0],
	                    mac->ETHER_ADDR_OCTET[1],
	                    mac->ETHER_ADDR_OCTET[2]);
	base = &macname_base[hash];
	next = base->next;
	while(next != base){
		if(next->octet[0] == mac->ETHER_ADDR_OCTET[0] &&
		   next->octet[1] == mac->ETHER_ADDR_OCTET[1] &&
		   next->octet[2] == mac->ETHER_ADDR_OCTET[2]){
			return next->name;
		}
		next = next->next;
	}

	return NULL;
}

