/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: data.h 690 2008-03-31 18:36:43Z  $
 *
 */

#ifndef __DATA_H
#define __DATA_H

#include <pcap.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#ifdef __FreeBSD__
#   define ETHER_ADDR_OCTET octet
#else
#   define ETHER_ADDR_OCTET ether_addr_octet
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

#define NOT_EXIST            0x00
#define ALLOW                0X01
#define DENY                 0x02
#define APPEND               0x04

#define IP_CHANGE            0
#define UNKNOWN_ADDRESS      1 
#define BLACK_LISTED         2
#define NEW                  3
#define UNAUTH_RQ            4
#define RQ_ABUS              5
#define MAC_ERROR            6 
#define FLOOD                7 
#define NEW_MAC              8
#define MAC_CHANGE           9

// chain devices and idcap
struct capt {
	struct capt *next;
	pcap_t *pcap;
	char *device;
	struct ether_addr mac;
};

struct data_pack {
	struct ether_addr mac;
	// NOT_EXIST, ALLOW, DENY, APPEND 
	int flag;
	struct in_addr ip;
	struct timeval timestamp;
	struct timeval lastalert[7];
	int request;
	/// bit field used for set detect exception
	U_INT32_T alerts;
	struct capt *cap_id;
	
	// chain
	struct data_pack *next_chain;
	struct data_pack *prev_chain;
	struct data_pack *next_mac;
	struct data_pack *prev_mac;
	struct data_pack *next_ip;
	struct data_pack *prev_ip;
};

// set ip_change              0: 1st bit
#define SET_IP_CHANGE(a)      a |= 0x00000001
#define ISSET_IP_CHANGE(a)    (a & 0x00000001)
// set black_listed           2: 3rd bit
#define SET_BLACK_LISTED(a)   a |= 0x00000004
#define ISSET_BLACK_LISTED(a) (a & 0x00000004)
// set unauthorized_request   4: 5th bit
#define SET_UNAUTH_RQ(a)      a |= 0x00000010
#define ISSET_UNAUTH_RQ(a)    (a & 0x00000010)
// set rq_abus                5: 6th bit
#define SET_RQ_ABUS(a)        a |= 0x00000050
#define ISSET_RQ_ABUS(a)      (a & 0x00000050)
// set mac_error              6: 7th bit
#define SET_MAC_ERROR(a)      a |= 0x00000040
#define ISSET_MAC_ERROR(a)    (a & 0x00000040)
// set mac_change             9: 10th bit
#define SET_MAC_CHANGE(a)     a |= 0x00000200
#define ISSET_MAC_CHANGE(a)   (a & 0x00000200)

// initialize data system
void data_init(void);

// clear all datas
void data_reset(void);

// call a dump of all datas
void data_rqdump(void);

// launch data dump
void data_dump(void);

// compare 2 mac adresses
// return 0 if mac are equals
// data_cmp(data_mac *, data_mac *)
#define DATA_EQUAL 0
#define DATA_CMP(a, b) memcmp(a, b, sizeof(struct ether_addr))

// copy mac
#define DATA_CPY(a, b) memcpy(a, b, sizeof(struct ether_addr))

// add data in database with field
void data_add_field(struct ether_addr *mac, int status,
                    struct in_addr, U_INT32_T,
                    struct capt *idcap);

// add data in database with detection time
void data_add_time(struct ether_addr *mac, int status,
                   struct in_addr ip, struct capt *idcap,
                   struct timeval *tv);

// update data in database with field
void data_update_field(struct ether_addr *mac, int status,
                       struct in_addr ip,
                       U_INT32_T field, struct capt *idcap);
				
// add data to database
struct data_pack *data_add(struct ether_addr *mac,
                           int status, struct in_addr,
                           struct capt *idcap);

// timeout indexation
void index_timeout(struct data_pack *);

// force ip indexation
void index_ip(struct data_pack *);

// delete ip indexation
void unindex_ip(struct in_addr ip, struct capt *idcap);
		  
// check if data exist
// return NULL if not exist
struct data_pack *data_exist(struct ether_addr *,
                             struct capt *idcap);

// check if ip exist
// return NULL if not exist
struct data_pack *data_ip_exist(struct in_addr ip,
                                struct capt *idcap);

// return next timeout and function to call for data section
void *data_next(struct timeval *tv);

#endif
