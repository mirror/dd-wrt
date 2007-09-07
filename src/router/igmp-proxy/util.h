/*
 * Copyright 2007, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of Broadcom Corporation.                            
 *
 * $Id$
 */


#define EQUAL(s1,s2)	(strcmp((s1),(s2)) == 0)
extern int      log_level;

// this header file is used by most of igmp files and the following need
// to be included 
// if the new source file uses memory allocation funtions
#ifdef USE_DMALLOC
#include "dmalloc.h"
#endif
/*
 * Log support
 */
#ifdef DEBUG
#define LOG(x) debug x
#else
#define LOG(x)
#endif
// #define LOG_INFO 1
#define	LOG_DETAIL	2
// #define LOG_DEBUG 3


#define UPSTREAM    4
#define DOWNSTREAM  5

typedef struct _interface_list_t {
    char            ifl_name[IFNAMSIZ];
    struct sockaddr ifl_addr;
    struct _interface_list_t *ifl_next;
    short           ifl_index;	/* lo=1, eth0=2,usb=3,br0=4,nas=5 and etc */
    int type;
} interface_list_t;

unsigned short  in_cksum(unsigned short *addr, int len);
interface_list_t *get_interface_list(short af, short flags, short unflags);
void            free_interface_list(interface_list_t * ifl);
short           get_interface_flags(char *ifname);
short           set_interface_flags(char *ifname, short flags);
int             get_interface_mtu(char *ifname);
int             mrouter_onoff(int sockfd, int onoff);
/* FILE-CSTYLED */
