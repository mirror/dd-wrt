/* ioctl.c
 * Linux CAN-bus device driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#ifdef CAN_WITH_RTL

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"

#include <rtl_posixio.h>
#include "../include/ioctl.h"
#include "../include/can_iortl.h"


int can_ioctl_rtl_posix(struct rtl_file *fptr, unsigned int cmd, unsigned long arg)
{
	struct canuser_t *canuser =
		(struct canuser_t *)can_get_rtl_file_private_data(fptr);
	int i=0;
	unsigned short channel=0;
	unsigned btr0=0, btr1=0;
	struct msgobj_t *obj;
	struct canchip_t *chip;
	struct canque_ends_t *qends;
	
	if(!canuser || (canuser->magic != CAN_USER_MAGIC)){
		CANMSG("can_ioctl_: bad canuser magic\n");
		return -ENODEV;
	}
	
	obj = canuser->msgobj;
	if (obj == NULL) {
		CANMSG("Could not assign buffer structure\n");
		return -1;
	}

	qends = canuser->qends;
	chip = obj->hostchip;
	if (chip == NULL) {
		CANMSG("Device is not correctly configured.\n");
		CANMSG("Please reload the driver.\n");
		return -1;
	}

	switch (cmd) {
		case CAN_DRV_QUERY: {
			return can_ioctl_query(canuser, arg);
		}
		case STAT: {
			for (i=0x0; i<0x100; i++)
				CANMSG("0x%x is 0x%x\n",i,can_read_reg(chip,i));
			break;
		}
		case CMD_START: {
			if (chip->chipspecops->start_chip(chip))
				return -1;
			break;
		}
		case CMD_STOP: {
			if (chip->chipspecops->stop_chip(chip))
				return -1;
			break;
		}
		case CANQUE_FLUSH: {
			canque_flush(canuser->rx_edge0);
			break;
		}
		case CONF_FILTER: {
			if(canuser->rx_edge0){
				canque_set_filt(canuser->rx_edge0, arg, ~0, 0);
			}

			break;
		}
		
		case CANQUE_FILTER: {
			struct canfilt_t canfilt=*(struct canfilt_t *)arg;
			if(canuser->rx_edge0){
				canque_set_filt(canuser->rx_edge0, canfilt.id, canfilt.mask, canfilt.flags);
			}
			break;
		}

		case CONF_BAUD: {
			channel = arg & 0xff;
			btr0 = (arg >> 8) & 0xff;
			btr1 = (arg >> 16) & 0xff;

			if (chip->chipspecops->set_btregs(chip, btr0, btr1)) {
				CANMSG("Error setting bit timing registers\n");
				return -1;
			}
			break;
		}
		default: {
			CANMSG("Not a valid ioctl command\n");
			return -EINVAL;
		}
		
	}

	return 0;
}

#endif /*CAN_WITH_RTL*/
