/* canmsg.h - common kernel-space and user-space CAN message structure
 * Linux CAN-bus device driver.
 * Written by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#ifndef _CANMSG_T_H
#define _CANMSG_T_H

#ifdef __KERNEL__

#include <linux/time.h>
#include <linux/types.h>

#else /* __KERNEL__ */

#include <inttypes.h>
#include <sys/time.h>
#include <sys/types.h>

#endif /* __KERNEL__ */

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * CAN_MSG_VERSION_2 enables new canmsg_t layout compatible with
 * can4linux project from http://www.port.de/
 * 
 */
#define CAN_MSG_VERSION_2

/* Number of data bytes in one CAN message */
#define CAN_MSG_LENGTH 8

#ifdef CAN_MSG_VERSION_2

typedef struct timeval canmsg_tstamp_t ;

typedef unsigned long canmsg_id_t;

/**
 * struct canmsg_t - structure representing CAN message
 * @flags:  message flags
 *      %MSG_RTR .. message is Remote Transmission Request,
 *	%MSG_EXT .. message with extended ID, 
 *      %MSG_OVR .. indication of queue overflow condition,
 *	%MSG_LOCAL .. message originates from this node.
 * @cob:    communication object number (not used)
 * @id:     ID of CAN message
 * @timestamp: not used
 * @length: length of used data
 * @data:   data bytes buffer
 *
 * Header: canmsg.h
 */
struct canmsg_t {
	int             flags;
	int             cob;
	canmsg_id_t     id;
	canmsg_tstamp_t timestamp;
	unsigned short  length;
	unsigned char   data[CAN_MSG_LENGTH];
};

#else /*CAN_MSG_VERSION_2*/
#ifndef PACKED
#define PACKED __attribute__((packed))
#endif
/* Old, deprecated version of canmsg_t structure */
struct canmsg_t {
	short		flags;
	int		cob;
	canmsg_id_t	id;
	unsigned long	timestamp;
	unsigned int	length;
	unsigned char	data[CAN_MSG_LENGTH];
} PACKED;
#endif /*CAN_MSG_VERSION_2*/

typedef struct canmsg_t canmsg_t;

/**
 * struct canfilt_t - structure for acceptance filter setup
 * @flags:  message flags
 *      %MSG_RTR .. message is Remote Transmission Request,
 *	%MSG_EXT .. message with extended ID, 
 *      %MSG_OVR .. indication of queue overflow condition,
 *	%MSG_LOCAL .. message originates from this node.
 *	there are corresponding mask bits
 *	%MSG_RTR_MASK, %MSG_EXT_MASK, %MSG_LOCAL_MASK.
 *	%MSG_PROCESSLOCAL enables local messages processing in the
 *	combination with global setting
 * @queid:  CAN queue identification in the case of the multiple
 *	    queues per one user (open instance)
 * @cob:    communication object number (not used)
 * @id:     selected required value of cared ID id bits
 * @mask:   select bits significand for the comparation;
 *          1 .. take care about corresponding ID bit, 0 .. don't care
 *
 * Header: canmsg.h
 */
struct canfilt_t {
	int		flags;
	int		queid;
	int		cob;
	canmsg_id_t	id;
	canmsg_id_t	mask;
};

typedef struct canfilt_t canfilt_t;

/* Definitions to use for canmsg_t and canfilt_t flags */
#define MSG_RTR   (1<<0)
#define MSG_OVR   (1<<1)
#define MSG_EXT   (1<<2)
#define MSG_LOCAL (1<<3)
/* If you change above lines, check canque_filtid2internal function */

/* Additional definitions used for canfilt_t only */
#define MSG_FILT_MASK_SHIFT   8
#define MSG_RTR_MASK   (MSG_RTR<<MSG_FILT_MASK_SHIFT)
#define MSG_EXT_MASK   (MSG_EXT<<MSG_FILT_MASK_SHIFT)
#define MSG_LOCAL_MASK (MSG_LOCAL<<MSG_FILT_MASK_SHIFT)
#define MSG_PROCESSLOCAL (MSG_OVR<<MSG_FILT_MASK_SHIFT)

/* Can message ID mask */
#define MSG_ID_MASK ((1l<<29)-1)

#ifdef __cplusplus
} /* extern "C"*/
#endif

#endif /*_CANMSG_T_H*/
