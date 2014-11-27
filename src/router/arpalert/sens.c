/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: sens.c 690 2008-03-31 18:36:43Z  $
 *
 */

#include "config.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "arpalert.h"
#include "loadconfig.h"
#include "data.h"
#include "sens.h"
#include "capture.h"
#include "log.h"
#include "func_str.h"

#if (__sun)
#    define INADDR_NONE ((in_addr_t)(-1))
#endif

// hash table size ; this number must be primary number
#define HASH_SIZE 4096

/* debug: */
// #define DEBUG 1

/* HACHAGE */
#define SENS_HASH(x, y, z) ({ \
	U_INT32_T a, b, c; \
	a = (*(U_INT16_T*)&(x)->ETHER_ADDR_OCTET[0]) ^ \
	    (*(U_INT16_T*)&(x)->ETHER_ADDR_OCTET[4]); \
	b = a + ( ( (x)->ETHER_ADDR_OCTET[2] ^ \
	            (x)->ETHER_ADDR_OCTET[3] ) << 16 ); \
	c = a ^ ( b >> 12 ); \
	a = ((U_INT16_T)(y)) ^ ( ((U_INT32_T)(y)) >> 20 ) ^ \
	                       ( ((U_INT8_T)(y)) >> 12 ); \
	b = ((U_INT16_T)(z)) ^ ( ((U_INT32_T)(z)) >> 20 ) ^ \
	                       ( ((U_INT8_T)(z)) >> 12 ); \
	( a ^ b ^ c ) & 0xfff; \
})

#define BUF_SIZE           1024
#define MAX_ARGS           150
#define MAX_CONTEXT_ARGS   20
#define MAC_ADRESS_MAX_LEN 17
#define IP_ADRESS_MAX_LEN  15
#define MASK_MAX_LEN       2

// masks 
#define END_OF_MASKS 0x00000001

extern int errno;

// conv binary mask to ip style mask
const U_INT32_T dec_to_bin[33] = {
	0x00000000,
	0x00000080,
	0x000000c0,
	0x000000e0,
	0x000000f0,
	0x000000f8,
	0x000000fc,
	0x000000fe,
	0x000000ff,
	0x000080ff,
	0x0000c0ff,
	0x0000e0ff,
	0x0000f0ff,
	0x0000f8ff,
	0x0000fcff,
	0x0000feff,
	0x0000ffff,
	0x0080ffff,
	0x00c0ffff,
	0x00e0ffff,
	0x00f0ffff,
	0x00f8ffff,
	0x00fcffff,
	0x00feffff,
	0x00ffffff,
	0x80ffffff,
	0xc0ffffff,
	0xe0ffffff,
	0xf0ffffff,
	0xf8ffffff,
	0xfcffffff,
	0xfeffffff,
	0xffffffff
};

/* structures */
struct pqt {
	struct ether_addr mac;
	struct in_addr ip_d;
	struct in_addr mask;
	struct capt *idcap;
	struct pqt *next;
};

/* hash */
struct pqt *pqt_h[HASH_SIZE];

/* mask list */
struct in_addr used_masks[33];

/* load data aor acls
 * @param status:
 *        SENS_TEST: test file
 *        SENS_LOAD: load file
 * @return:   0 if is ok
 *           -1 if is not ok
 */
int sens_init(int status) {
	FILE *fd;
	char buf[BUF_SIZE];
	char *parse;
	int i, j;
	char sort_tmp;
	signed char list_mask[33];
	char *str_ip;
	char *str_mask;
	struct in_addr ip;
	U_INT32_T mask;
	struct in_addr binmask;
	struct ether_addr mac;
	struct capt *idcap = NULL;
	char *args[MAX_ARGS];
	int arg_c;
	int arg_n;
	int *arg;
	int ligne = 0;
	int context;

	// init memory
	memset(&pqt_h, 0, HASH_SIZE * sizeof(struct pqt *));
	memset(&list_mask, -1, 33);

	if(config[CF_AUTHFILE].valeur.string == NULL ||
	   config[CF_AUTHFILE].valeur.string[0] == 0) {
		return 0;
	}

	// open file
	fd = fopen(config[CF_AUTHFILE].valeur.string, "r");
	if(fd == NULL){
		logmsg(LOG_ERR, "[%s %i] fopen[%d]: %s (%s)",
		       __FILE__, __LINE__,
		       errno, strerror(errno),
		       config[CF_AUTHFILE].valeur.string);
		return -1;
	}

	// read lines
	while(feof(fd) == 0){
		fgets(buf, BUF_SIZE, fd);
		ligne++;

		arg_c = 0;
		arg_n = MAX_CONTEXT_ARGS;
		arg = &arg_n;
		bzero(args, sizeof(char *) * MAX_ARGS);
		parse = buf;
		context = 0;
		while(*parse != 0){
			// mac / interface identification
			if(*parse == '['){
				*parse = ' ';
				arg = &arg_c;
			}
			else if(*parse == ']'){
				*parse = ' ';
				arg = &arg_n;
			}

			// nul character
			if(*parse == ' ' || *parse == '\t'){
				*parse = 0;
				context = 1;
			}

			// last caracter => quit
			else if( *parse=='#' || *parse=='\n' || *parse=='\r'){
				*parse = 0;
				break;
			}

			// other caracters
			else if(context == 1){
				args[*arg] = parse;
				(*arg)++;
				// exceed args hard limit
				if(arg_c == MAX_CONTEXT_ARGS || arg_n == MAX_ARGS){
					logmsg(LOG_ERR,
					       "file: \"%s\", line %d: "
					       "exceed args hard limit (%d)",
					       config[CF_AUTHFILE].valeur.string,
							 ligne, MAX_ARGS);
					return -1;
				}
				context = 0;
			}
			parse++;
		}

		// context sanity check
		if(arg_c > 0 && arg_c < 2){
			logmsg(LOG_ERR,
			       "file: \"%s\", line %d: insufficient arguments",
			       config[CF_AUTHFILE].valeur.string,
			       ligne);
			return -1;
		}

		// change context
		if(arg_c > 0){
		
			// set interface
			idcap = cap_get_interface(args[1]);
			if(idcap == NULL){
				logmsg(LOG_ERR,
				       "file: \"%s\", line %d: unknown interfaces: %s",
				       config[CF_AUTHFILE].valeur.string,
				       ligne, args[1]);
				exit(1);
			}

			// set mac address
			if(str_to_mac(args[0], &mac) == -1){
				logmsg(LOG_ERR,
				       "file: \"%s\", line %d: mac addess error",
				       config[CF_AUTHFILE].valeur.string,
				       ligne);
				return -1;
			}
		}

		// add address
		i = MAX_CONTEXT_ARGS;
		while(i < arg_n){
			parse = args[i];
			str_ip = parse;
			str_mask = NULL;
			while(*parse != 0){
				if(*parse == '/'){
					*parse = 0;
					parse++;
					str_mask = parse;
					break;
				}
				parse++;
			}

			// ip
			ip.s_addr = inet_addr(str_ip);
			if(ip.s_addr == INADDR_NONE){
				logmsg(LOG_ERR,
				       "file: \"%s\", line %d: ip adress error: %s",
				       config[CF_AUTHFILE].valeur.string,
				       ligne, str_ip);
				return -1;
			}

			// network address validation
			mask = atoi(str_mask);
			binmask.s_addr = dec_to_bin[mask];
			if( (ip.s_addr & binmask.s_addr) != ip.s_addr){
				logmsg(LOG_ERR,
				       "file: \"%s\", line %d: incorrect value %s/%u",
				       config[CF_AUTHFILE].valeur.string,
				       ligne, str_ip, str_mask);
				return -1;
			}

			// add this network value in hash
			if(status == SENS_LOAD){
				sens_add(&mac, ip, binmask, idcap);
			}

			i++;
		}
	}

	// close
	if(fclose(fd) == EOF){
		logmsg(LOG_ERR, "[%s %d] fclose[%d]: %s",
		       __FILE__, __LINE__, errno, strerror(errno));
		exit(1);
	}

	// end of function if only file is tested
	if(status == SENS_TEST){
		return 0;
	}
	
	// sort list_mask
	// algo bubble sort
	i = 0;
	while(i < 32){
		j = 32;
		while(j > i){
			if(list_mask[j] > list_mask[j-1]){
				sort_tmp = list_mask[j-1];
				list_mask[j-1] = list_mask[j];
				list_mask[j] = sort_tmp;
			}
			j--;
		}
		// convert decimal mask to binary mask
		if(list_mask[i] != -1){
			used_masks[i].s_addr = dec_to_bin[(u_char)list_mask[i]];
		} else {
			used_masks[i].s_addr = END_OF_MASKS;
		}
		i++;
	}

	return 0;
}

// add data to hash
void sens_add(struct ether_addr *mac, struct in_addr ipb,
              struct in_addr mask, struct capt *idcap){
	u_int h;
	struct pqt *mpqt;

	mpqt = (struct pqt *)malloc(sizeof(struct pqt));
	if(mpqt == NULL){
		logmsg(LOG_ERR, "[%s %d] malloc[%d]: %s",
		       __FILE__, __LINE__, errno, strerror(errno));
		exit(1);
	}
	DATA_CPY(&mpqt->mac, mac);
	mpqt->ip_d = ipb;
	mpqt->mask = mask;

	// calculate hash
	h = SENS_HASH(mac, ipb.s_addr, mask.s_addr);
	// find a free space
	mpqt->next = pqt_h[h];
	pqt_h[h] = mpqt;
}

void sens_free(void){
	int i;
	struct pqt *free_pqt;
	struct pqt *current_pqt;

	i = 0;
	while(i < HASH_SIZE){
		current_pqt = pqt_h[i];
		while(current_pqt != NULL){
			free_pqt = current_pqt;
			current_pqt = current_pqt->next;
			free(free_pqt);
		}
		pqt_h[i] = NULL;
		i++;
	}
}

void sens_reload(void){
	int retval;

	retval = sens_init(SENS_TEST);
	if(retval == -1){
		logmsg(LOG_ERR,
		       "errors in file \"%s\": not reload",
		       config[CF_AUTHFILE].valeur.string);
		return;
	}
	sens_free();
	sens_init(SENS_LOAD);
}

int sens_exist(struct ether_addr *mac,
               struct in_addr ipb, struct capt *idcap){
	u_int h;
	struct pqt *spqt;
	struct in_addr *masks = &used_masks[0];
	struct in_addr ip;

	// test all masks
	while(masks->s_addr != END_OF_MASKS){
		
		// apply mask
		ip.s_addr = ipb.s_addr & masks->s_addr;

		// get data in hash
		h = SENS_HASH(mac, ip.s_addr, masks->s_addr);
		spqt = pqt_h[h];

		// find data
		while(spqt != NULL){
			if(spqt->ip_d.s_addr == ip.s_addr &&
			   spqt->mask.s_addr == masks->s_addr &&
			   DATA_CMP(&spqt->mac, mac) == 0 ){
				return(TRUE);
			}
			spqt = spqt->next;
		}
		masks++;
	}
	return(FALSE);
}

