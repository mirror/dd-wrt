/* can.h
 * Header file for the Linux CAN-bus driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#ifndef _CAN_DRVAPI_T_H
#define _CAN_DRVAPI_T_H

#ifdef __KERNEL__

#include <linux/time.h>
#include <linux/types.h>
#include <linux/ioctl.h>

#else /* __KERNEL__ */

#include <inttypes.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#endif /* __KERNEL__ */

#include "./canmsg.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CAN ioctl magic number */
#define CAN_IOC_MAGIC 'd'

typedef unsigned long bittiming_t;
typedef unsigned short channel_t;

/**
 * struct can_baudparams_t - datatype for calling CONF_BAUDPARAMS IOCTL 
 * @flags: reserved for additional flags for chip configuration, should be written -1 or 0
 * @baudrate: baud rate in Hz
 * @sjw: synchronization jump width (0-3) prescaled clock cycles
 * @sampl_pt: sample point in % (0-100) sets (TSEG1+1)/(TSEG1+TSEG2+2) ratio
 * 
 * The structure is used to configure new set of parameters into CAN controller chip.
 * If default value of some field should be preserved, fill field by value -1.
 */
struct can_baudparams_t {
	long flags;
	long baudrate;
	long sjw;
	long sample_pt;
};

#define CAN_IOCTL_FLAG_RX_FIFO_FULL	1<<0
#define CAN_IOCTL_FLAG_TX_FIFO_FULL	1<<1
#define CAN_IOCTL_FLAG_ERR_WARNING	1<<2
#define CAN_IOCTL_FLAG_DATA_OVERRUN	1<<3
#define CAN_IOCTL_FLAG_UNUSED		1<<4
#define CAN_IOCTL_FLAG_ERR_PASSIVE	1<<5
#define CAN_IOCTL_FLAG_ARBIT_LOST	1<<6
#define CAN_IOCTL_FLAG_BUS_ERROR	1<<7

#define LED_REGISTER 0x101
#define DATA_LED 0x10
#define ERROR_LED 0x01
#define ERROR_LED_FLASHING 0x02

/* CAN ioctl functions */
#define CAN_DRV_QUERY _IO(CAN_IOC_MAGIC, 0)
#define CAN_DRV_QRY_BRANCH    0	/* returns driver branch value - "LINC" for LinCAN driver */
#define CAN_DRV_QRY_VERSION   1	/* returns driver version as (major<<16) | (minor<<8) | patch */
#define CAN_DRV_QRY_MSGFORMAT 2	/* format of canmsg_t structure */

#define CMD_START _IOW(CAN_IOC_MAGIC, 1, channel_t)
#define CMD_STOP _IOW(CAN_IOC_MAGIC, 2, channel_t)
//#define CMD_RESET 3

#define CONF_BAUD _IOW(CAN_IOC_MAGIC, 4, bittiming_t)
//#define CONF_ACCM
//#define CONF_XTDACCM
//#define CONF_TIMING
//#define CONF_OMODE
#define CONF_FILTER _IOW(CAN_IOC_MAGIC, 8, unsigned char)

//#define CONF_FENABLE
//#define CONF_FDISABLE

#define STAT _IO(CAN_IOC_MAGIC, 9)
#define CANQUE_FILTER _IOW(CAN_IOC_MAGIC, 10, struct canfilt_t)
#define CANQUE_FLUSH  _IO(CAN_IOC_MAGIC, 11)
#define CONF_BAUDPARAMS  _IOW(CAN_IOC_MAGIC, 11, struct can_baudparams_t)
#define CANRTR_READ  _IOWR(CAN_IOC_MAGIC, 12, struct canmsg_t)
#define GET_CAN_STATUS _IOR(CAN_IOC_MAGIC, 13, int)

#ifdef __cplusplus
} /* extern "C"*/
#endif

#endif /*_CAN_DRVAPI_T_H*/
