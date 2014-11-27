/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: maclist.c 696 2008-03-31 18:44:59Z  $
 *
 */

#include "config.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "capture.h"
#include "arpalert.h"
#include "maclist.h"
#include "data.h"
#include "loadconfig.h"
#include "log.h"
#include "func_time.h"
#include "func_str.h"

#if (__sun)
#    define INADDR_NONE ((in_addr_t)(-1))
#endif

#define BUFFER_SIZE 1024
#define MAX_ARGS    20

extern int errno;

#define TEST        1
#define LOAD        2
#define RELOAD      3
/* parse maclist 
 * @file_name: file to parse
 * @level: ALLOW : file contain allow macc addr (white list)
 *         DENY : file contain deny mac addr (black list)
 *         APPEND : load leases file
 * @mode:  TEST : test file and return status
 *         LOAD : load file
 *         RELOAD : load file and replace mac in memory
 *
 * @return: 0  if success
 *          -1 if error
 */
int maclist_file(char *file_name, int level, int mode){
	char buf[BUFFER_SIZE];
	FILE *file;
	char *args[MAX_ARGS];
	int ligne = 0;
	int arg, blank, i;
	char *parse;
	struct ether_addr mac;
	struct in_addr ip;
	U_INT32_T bitfield;
	struct capt *dev;
	struct timeval discover;
	struct timeval comp;

	// open file
	file = fopen(file_name, "r");
	if(file == NULL){
		// the leases file is not found ...
		// maybe does not exist. is not an error
		// just log
		if(errno == ENOENT && level == APPEND){
			logmsg(LOG_NOTICE, "Leases file (%s) not found",
			       file_name);
			return 0;
		}
		logmsg(LOG_ERR, "[%s %d] fopen[%d]: %s (%s)",
		       __FILE__, __LINE__,
		       errno, strerror(errno),
		       file_name);
		return -1;
	}
	
	buf[0] = 0;

	// for each line ..
	while(feof(file) == 0){
		bzero(buf, sizeof(char) * BUFFER_SIZE);
		fgets(buf, BUFFER_SIZE, file);
		ligne ++;

		// parse and return arguments list
		arg = 0;
		bzero(args, sizeof(char *) * MAX_ARGS);
		blank = 1;
		parse = buf;
		while(*parse != 0){
			// caractere nul:
			if(*parse == ' ' || *parse == '\t'){
				*parse = 0;
				blank = 1;
				parse ++;
			}

			// last caracter => quit
			else if( *parse == '#' || *parse == '\n' || *parse == '\r'){
				*parse = 0;
				break;
			}

			// other caracters
			else {
				if(blank == 1){
					args[arg] = parse;
					arg ++;
					// exceed args hard limit
					if(arg == MAX_ARGS) {
						logmsg(LOG_ERR,
						       "file: \"%s\", line %d: "
						       "exceed hard args limit (%d)",
						       file_name, ligne, MAX_ARGS);
						return -1;
					}
					blank = 0;
				}

				parse++;
			}
		}

		// proceed args
		if(arg == 0) continue;
	
		// sanity check
		if(arg < 3){
			logmsg(LOG_ERR,
			       "file: \"%s\", line %d: insufficient arguments",
			       file_name, ligne);
			return -1;
		}

		// first arg: mac addr
		if(str_to_mac(args[0], &mac) == -1){
			logmsg(LOG_ERR,
			       "file: \"%s\", line %d: mac adress error",
			       file_name, ligne);
			return -1;
		}

		// convert string ip to numeric IP
		ip.s_addr = inet_addr(args[1]);
		if(ip.s_addr == INADDR_NONE){
			logmsg(LOG_ERR,
			       "file: \"%s\", line %d: ip adress error: %s",
			       file_name, ligne, args[1]);
			return -1;
		}

		// pointer to interface
		dev = cap_get_interface(args[2]);
		if(dev == NULL){
			logmsg(LOG_ERR,
			       "file: \"%s\", line %d: device \"%s\" not found/used",
			       file_name, ligne, args[2]);
			return -1;
		}

		// if the loaded file are black list or white list
		if(level == ALLOW || level == DENY){

			// check flags
			bitfield = 0;
			i = 3;
			while(i < arg){
				/**/ if(strcmp(args[i], "ip_change") == 0){
					SET_IP_CHANGE(bitfield);
				}
				else if(strcmp(args[i], "black_listed") == 0){
					SET_BLACK_LISTED(bitfield);
				}
				else if(strcmp(args[i], "unauth_rq") == 0){
					SET_UNAUTH_RQ(bitfield);
				}
				else if(strcmp(args[i], "rq_abus") == 0){
					SET_RQ_ABUS(bitfield);
				}
				else if(strcmp(args[i], "mac_error") == 0){
					SET_MAC_ERROR(bitfield);
				}
				else if(strcmp(args[i], "mac_change") == 0){
					SET_MAC_CHANGE(bitfield);
				}
				else {
					logmsg(LOG_ERR,
					       "file: \"%s\", line %d: flag \"%s\" not availaible",
					       file_name, ligne, args[i]);
					return -1;
				}
				i++;
			}
	
			// add data
			if(mode == LOAD){
				data_add_field(&mac, level, ip, bitfield, dev);
			}
			else if(mode == RELOAD){
				data_update_field(&mac, level, ip, bitfield, dev);
			}
		}

		// if the loaded file are leases file
		else if(level == APPEND){
			
			// sanity check
			if(arg < 5){
				logmsg(LOG_ERR,
				       "file: \"%s\", line %d: insufficient arguments "
				       "for leases file",
				       file_name, ligne);
				return -1;
			}

			// get discovering date of packet date
			discover.tv_sec = atoi(args[3]);
			discover.tv_usec = atoi(args[4]);

			if(mode == LOAD){
				// check if time is expired
				comp.tv_sec = discover.tv_sec +
				              config[CF_TOOOLD].valeur.integer;
				comp.tv_usec = discover.tv_usec;
				if(time_comp(&comp, &current_t) == BIGEST){
					// add data
					data_add_time(&mac, level, ip, dev, &discover);
				}
			}
		}
	}

	// close file
	if(fclose(file) == EOF){
		logmsg(LOG_ERR, "[%s %d] fclose[%d]: %s",
		       __FILE__, __LINE__, errno, strerror(errno));
		exit(1);
	}

	// end
	return 0;
}

void maclist_load(void){
	int retval;
	
	// load white list
	if(config[CF_MACLIST].valeur.string != NULL &&
	   config[CF_MACLIST].valeur.string[0] != 0){
		retval = maclist_file(config[CF_MACLIST].valeur.string,
		                      ALLOW, LOAD);
		if(retval == -1){
			exit(1);
		}
	}

	// load black list
	if(config[CF_BLACKLST].valeur.string != NULL &&
	   config[CF_BLACKLST].valeur.string[0] != 0){
		retval = maclist_file(config[CF_BLACKLST].valeur.string,
		                      DENY, LOAD);
		if(retval == -1){
			exit(1);
		}
	}

	// load leases
	if(config[CF_LEASES].valeur.string != NULL &&
	   config[CF_LEASES].valeur.string[0] != 0){
		retval = maclist_file(config[CF_LEASES].valeur.string,
		                      APPEND, LOAD);
		if(retval == -1){
			exit(1);
		}
	}
}

void maclist_reload(void){
	int retval;

	#ifdef DEBUG
	logmsg(LOG_DEBUG, "[%s %d %s] reload maclist",
	       __FILE__, __LINE__, __FUNCTION__);
	#endif

	// test white list
	if(config[CF_MACLIST].valeur.string != NULL &&
	   config[CF_MACLIST].valeur.string[0] != 0){
		retval = maclist_file(config[CF_MACLIST].valeur.string,
		                        ALLOW, TEST);
		if(retval == -1){
			logmsg(LOG_ERR,
			       "errors in file \"%s\": not reload",
			       config[CF_MACLIST].valeur.string);
			return;
		}
	}

	// test black list
	if(config[CF_BLACKLST].valeur.string != NULL &&
	   config[CF_BLACKLST].valeur.string[0] != 0){
		retval = maclist_file(config[CF_BLACKLST].valeur.string,
		                       DENY, TEST);
		if(retval == -1){
			logmsg(LOG_ERR,
			       "errors in file \"%s\": not reload",
			       config[CF_MACLIST].valeur.string);
			return;
		}
	}

	// reload white list
	if(config[CF_MACLIST].valeur.string != NULL &&
	   config[CF_MACLIST].valeur.string[0] != 0){
		retval = maclist_file(config[CF_MACLIST].valeur.string,
		                      ALLOW, RELOAD);
		if(retval == -1){
			exit(1);
		}
	}

	// reload black list
	if(config[CF_BLACKLST].valeur.string != NULL &&
	   config[CF_BLACKLST].valeur.string[0] != 0){
		retval = maclist_file(config[CF_BLACKLST].valeur.string,
		                      DENY, RELOAD);
		if(retval == -1){
			exit(1);
		}
	}
}
