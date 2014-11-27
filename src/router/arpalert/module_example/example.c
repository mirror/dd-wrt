/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: example.c 690 2008-03-31 18:36:43Z  $
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>

#ifdef __FreeBSD__
#   define ETHER_ADDR_OCTET octet
#else
#   define ETHER_ADDR_OCTET ether_addr_octet
#endif

#include "../api/arpalert.h"

// context
int numb;
char *prefix;

// init module and context
void mod_load(char *config){
	char *p, *conf;
	int context = 0;
	char *args[50];
	int count = 0;
	int i = 0;
	
	// default conf
	prefix = "";
	numb = 0;
	
	// parse config
	conf = strdup(config);
	p = conf;
	while(*p != 0){
		if(context == 0 && *p != ' ' && *p != '\t'){
			context = 1;
			args[count] = p;
			count++;
			if(count == 50) break;
		}
		
		else if(context == 1 && ( *p == ' ' || *p == '\t')){
			context = 0;
			*p = 0;
		}
		
		p++;
	}
	
	// apply config
	i = 0;
	while(i < count){
		if(strcmp("prefix", args[i]) == 0){
			i ++;
			if(i < count) prefix = strdup(args[i]);
		}
		else if(strcmp("init", args[i]) == 0){
			i++;
			if(i < count) numb = atoi(args[i]);
		}
		i++;
	}

	// free temp
	free(conf);
}

// alert launched
void mod_alert(int type, int nargs, void **data){
	struct ether_addr *mac; 
	struct in_addr ip;

	mac = (struct ether_addr *)data[1];
	ip.s_addr = (*(struct in_addr *)data[2]).s_addr;
	numb++;

	printf("%s[%d]: type=%d nargs=%d port=%s mac="
	       "%02x:%02x:%02x:%02x:%02x:%02x ip=%s\n",
			 prefix, numb, type, nargs, (char *)data[0], 
			 mac->ETHER_ADDR_OCTET[0],
			 mac->ETHER_ADDR_OCTET[1],
			 mac->ETHER_ADDR_OCTET[2],
			 mac->ETHER_ADDR_OCTET[3],
			 mac->ETHER_ADDR_OCTET[4],
			 mac->ETHER_ADDR_OCTET[5],
	       inet_ntoa(ip));
}

// init module and context
void mod_unload(void){
	printf("MODULE CLOSED AFTER %d ALERTS\n", numb);
	free(prefix);
}
