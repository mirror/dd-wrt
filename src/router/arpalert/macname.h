/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: macname.h 690 2008-03-31 18:36:43Z  $
 *
 */

#ifndef __MACNAME_H__
#define __MACNAME_H__

#include <sys/socket.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#ifdef __FreeBSD__
#   define ETHER_ADDR_OCTET octet
#else
#   define ETHER_ADDR_OCTET ether_addr_octet
#endif
#if (__NetBSD__ || __FreeBSD__ || __OpenBSD__ || __sun )
#    include <net/if_dl.h>
#endif
#if (__sun)
#   define U_INT32_T uint32_t
#   define U_INT16_T uint16_t
#   define U_INT8_T uint8_t
#else
#   define U_INT32_T u_int32_t
#   define U_INT16_T u_int16_t
#   define U_INT8_T u_int8_t
#endif

// init data
void macname_init(void);

/* load oui.txt list
 * @param mode: MACNAME_TEST: test loading
 *              MACNAME_LOAD: load data
 *
 * @return:  0: ok
 *          -1: error
 */
#define MACNAME_TEST 0
#define MACNAME_LOAD 1
#define MACNAME_RELOAD 2
int macname_load(int mode);

/* return the vendor mac name
 * @param mac : the mac
 * @return : return NULL if vendor not found
 */
char *get_vendor(struct ether_addr *mac);

// reload mac database
void macname_reload(void);

#endif
