/* constants.h
 * Header file for the Linux CAN-bus driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

/* Device name as it will appear in /proc/devices */
#define DEVICE_NAME "can"

/* Branch of the driver */
#define CAN_DRV_BRANCH (('L'<<24)|('I'<<16)|('N'<<8)|'C')

/* Version of the driver */
#define CAN_DRV_VER_MAJOR 0
#define CAN_DRV_VER_MINOR 3
#define CAN_DRV_VER_PATCH 3
#define CAN_DRV_VER ((CAN_DRV_VER_MAJOR<<16) | (CAN_DRV_VER_MINOR<<8) | CAN_DRV_VER_PATCH)

/* Default driver major number, see /usr/src/linux/Documentation/devices.txt */
#define CAN_MAJOR 91

/* Definition of the maximum number of concurrent supported hardware boards,
 * chips per board, total number of chips, interrupts and message objects.
 * Obviously there are no 32 different interrupts, but each chip can have its
 * own interrupt so we have to check for it MAX_IRQ == MAX_TOT_CHIPS times.
 */
#define MAX_HW_CARDS 8
#define MAX_HW_CHIPS 4
#define MAX_TOT_CHIPS (MAX_HW_CHIPS*MAX_HW_CARDS)
#define MAX_TOT_CHIPS_STR 32	/* must be explicit for MODULE_PARM */
#define MAX_IRQ 32
#define MAX_MSGOBJS 32
#define MAX_TOT_MSGOBJS (MAX_TOT_CHIPS*MAX_MSGOBJS)
#define MAX_BUF_LENGTH 64*10
//#define MAX_BUF_LENGTH 4


/* These flags can be used for the msgobj_t structure flags data entry */
#define MSGOBJ_OPENED_b		   0
#define MSGOBJ_TX_REQUEST_b	   1
#define MSGOBJ_TX_LOCK_b           2
#define MSGOBJ_IRQ_REQUEST_b       3
#define MSGOBJ_WORKER_WAKE_b       4
#define MSGOBJ_FILTCH_REQUEST_b    5
#define MSGOBJ_RX_MODE_b           6
#define MSGOBJ_RX_MODE_EXT_b       7
#define MSGOBJ_TX_PENDING_b        8

#define MSGOBJ_OPENED              (1<<MSGOBJ_OPENED_b)
#define MSGOBJ_TX_REQUEST          (1<<MSGOBJ_TX_REQUEST_b)
#define MSGOBJ_TX_LOCK             (1<<MSGOBJ_TX_LOCK_b)
#define MSGOBJ_IRQ_REQUEST         (1<<MSGOBJ_IRQ_REQUEST_b)
#define MSGOBJ_WORKER_WAKE         (1<<MSGOBJ_WORKER_WAKE_b)
#define MSGOBJ_FILTCH_REQUEST      (1<<MSGOBJ_FILTCH_REQUEST_b)
#define MSGOBJ_RX_MODE             (1<<MSGOBJ_RX_MODE_b)
#define MSGOBJ_RX_MODE_EXT         (1<<MSGOBJ_RX_MODE_EXT_b)
#define MSGOBJ_TX_PENDING          (1<<MSGOBJ_TX_PENDING_b)

#define can_msgobj_test_fl(obj,obj_fl) \
  test_bit(MSGOBJ_##obj_fl##_b,&(obj)->obj_flags)
#define can_msgobj_set_fl(obj,obj_fl) \
  set_bit(MSGOBJ_##obj_fl##_b,&(obj)->obj_flags)
#define can_msgobj_clear_fl(obj,obj_fl) \
  clear_bit(MSGOBJ_##obj_fl##_b,&(obj)->obj_flags)
#define can_msgobj_test_and_set_fl(obj,obj_fl) \
  test_and_set_bit(MSGOBJ_##obj_fl##_b,&(obj)->obj_flags)
#define can_msgobj_test_and_clear_fl(obj,obj_fl) \
  test_and_clear_bit(MSGOBJ_##obj_fl##_b,&(obj)->obj_flags)


/* These flags can be used for the canchip_t structure flags data entry */
#define CHIP_ATTACHED    (1<<0)  /* chip is attached to HW, release_chip() has to be called */
#define CHIP_CONFIGURED  (1<<1)  /* chip is configured and prepared for communication */
#define CHIP_SEGMENTED   (1<<2)  /* segmented access, ex: i82527 with 16 byte window*/
#define CHIP_IRQ_SETUP   (1<<3)  /* IRQ handler has been set */
#define CHIP_IRQ_PCI     (1<<4)  /* chip is on PCI board and uses PCI interrupt  */
#define CHIP_IRQ_VME     (1<<5)  /* interrupt is VME bus and requires VME bridge */
#define CHIP_IRQ_CUSTOM  (1<<6)  /* custom interrupt provided by board or chip code */
#define CHIP_IRQ_FAST    (1<<7)  /* interrupt handler only schedules postponed processing */

#define CHIP_MAX_IRQLOOP 1000

/* System independent defines of IRQ handled state */
#define CANCHIP_IRQ_NONE     0
#define CANCHIP_IRQ_HANDLED  1
#define CANCHIP_IRQ_ACCEPTED 2
#define CANCHIP_IRQ_STUCK    3

/* These flags can be used for the candevices_t structure flags data entry */
#define CANDEV_PROGRAMMABLE_IRQ (1<<0)
#define CANDEV_IO_RESERVED	(1<<1)

/* Next flags are specific for struct canuser_t applications connection */
#define CANUSER_RTL_CLIENT      (1<<0)
#define CANUSER_RTL_MEM         (1<<1)
#define CANUSER_DIRECT          (1<<2)


enum timing_BTR1 {
	MAX_TSEG1 = 15,
	MAX_TSEG2 = 7
};

/* Flags for baud_rate function */
#define BTR1_SAM (1<<1)

#endif
