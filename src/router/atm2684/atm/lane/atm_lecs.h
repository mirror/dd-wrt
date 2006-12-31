/*
 * ATM connection 
 *
 * $Id: atm.h,v 1.2 1995/11/15 08:27:11 carnil Exp $
 *
 */
#ifndef ATM_LECS_H
#define ATM_LECS_H

/* System includes needed for types */
#include <sys/types.h>

/* Codepoints */
#define CONFIGURATION_DIRECT    0x01
#define CONTROL_DIRECT          0x01
#define CONTROL_DISTRIBUTE      0x01
#define DATA_DIRECT_802_3       0x02
#define DATA_DIRECT_802_5       0x03
#define MULTICAST_SEND_802_3    0x04
#define MULTICAST_FORWARD_802_3 0x04
#define MULTICAST_SEND_802_5    0x05
#define MULTICAST_FORWARD_802_5 0x05

/* Global function prototypes */
int atm_create_socket(unsigned char codepoint, const unsigned char *our_addr);

#endif /* ATM_LECS_H */

