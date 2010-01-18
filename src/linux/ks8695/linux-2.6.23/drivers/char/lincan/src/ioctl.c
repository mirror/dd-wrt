/* ioctl.c
 * Linux CAN-bus device driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/ioctl.h"

int can_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int i=0;
	unsigned short channel=0;
	unsigned btr0=0, btr1=0;
	struct canuser_t *canuser = (struct canuser_t*)(file->private_data);
	struct msgobj_t *obj;
	struct canchip_t *chip;
	struct canque_ends_t *qends;
	
	if(!canuser || (canuser->magic != CAN_USER_MAGIC)){
		CANMSG("can_ioctl: bad canuser magic\n");
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
		case GET_CAN_STATUS: {
    			if (copy_to_user((unsigned long *)arg, &chip->canStatus, sizeof(chip->canStatus)))
		       		return -EFAULT;

			// clear canStatus 
			chip->canStatus = 0x0;
#ifdef CAN_USE_LEDS
			chip->canStatusLED = 0x0;
			can_write_reg(chip, chip->canStatusLED, LED_REGISTER);
#endif

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

			/* In- and output buffer re-initialization */
			
			if(canuser->rx_edge0){
				canque_set_filt(canuser->rx_edge0, arg, ~0, 0);
			}

			break;
		}
		
		case CANQUE_FILTER: {
			struct canfilt_t canfilt;
			int ret;
			ret = copy_from_user(&canfilt, (void*)arg, sizeof(struct canfilt_t));
			if(ret)	return -EFAULT;
			if(canuser->rx_edge0){
				canque_set_filt(canuser->rx_edge0, canfilt.id, canfilt.mask, canfilt.flags);
			}
			break;
		}
		
		case CANRTR_READ: {
			int ret;
			struct canmsg_t rtr_msg;
			
			ret = copy_from_user(&rtr_msg, (void*)arg, sizeof(struct canmsg_t));
			if(ret)	return -EFAULT;
			ret = can_ioctl_remote_read(canuser, &rtr_msg, rtr_msg.id, 0);
			if(ret<0) return ret;
			ret = copy_to_user((void*)arg, &rtr_msg, sizeof(struct canmsg_t));
			if(ret)	return -EFAULT;
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
		
		case CONF_BAUDPARAMS: {
			struct can_baudparams_t params;
			int ret;
			
			ret = copy_from_user(&params, (void*)arg, sizeof(struct can_baudparams_t));
			if(ret)	return -EFAULT;

			if(params.flags == -1) params.flags = 0;
			if(params.baudrate == -1) params.baudrate = chip->baudrate;
			if(params.sjw == -1) params.sjw = 0;
			if(params.sample_pt == -1) params.sample_pt = 75;
			i=chip->chipspecops->baud_rate(chip, params.baudrate, chip->clock, params.sjw,
                                                        params.sample_pt, params.flags);
			if(i>=0) chip->baudrate = params.baudrate;
			else {
				CANMSG("Error setting baud parameters\n");
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
