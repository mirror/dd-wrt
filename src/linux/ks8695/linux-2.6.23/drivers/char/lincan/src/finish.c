/* finish.c - finalization of the driver operation
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
#include "../include/devcommon.h"
#include "../include/finish.h"
#include "../include/setup.h"


/**
 * msgobj_done - destroys one CAN message object
 * @obj: pointer to CAN message object structure
 */
void msgobj_done(struct msgobj_t *obj)
{
	int delayed=0;
	if(obj->qends) {
		delayed=canqueue_ends_done_chip(obj->qends);
		if(delayed < 0)
			CANMSG("msgobj_done: problem with chip queue ends\n");
	}

	if((obj->hostchip) && (obj->object>0)) {
		if(obj->hostchip->msgobj[obj->object-1] == obj)
			obj->hostchip->msgobj[obj->object-1]=NULL;
		else
			CANMSG("msgobj_done: not registered in the canchip_t\n");
		obj->hostchip=NULL;
	}
	
	if((obj->minor>=0)) {
		if(objects_p[obj->minor] == obj)
			objects_p[obj->minor] = NULL;
		else
			CANMSG("msgobj_done: not registered as minor\n");
	}
	
	del_timer_sync(&obj->tx_timeout);

	if(obj->qends) {
		/*delayed free could be required there in the future,
		  actual use patter cannot generate such situation*/
		if(!delayed) {
			can_checked_free(obj->qends);
		}
	}
	obj->qends=NULL;
}


/**
 * canchip_done - destroys one CAN chip representation
 * @chip: pointer to CAN chip structure
 */
void canchip_done(struct canchip_t *chip)
{

	int i;
	struct msgobj_t *obj;

	if(chip->flags & CHIP_ATTACHED)
		chip->chipspecops->release_chip(chip);

	if((chip->hostdevice) && (chip->chip_idx>=0)) {
		if(chip->hostdevice->chip[chip->chip_idx] == chip)
			chip->hostdevice->chip[chip->chip_idx] = NULL;
		else
			CANMSG("canchip_done: not registered in hostdevice\n");
	}

	can_chip_free_irq(chip);
		
	can_synchronize_irq(chip->chip_irq);
	
  	for(i=0; i<chip->max_objects; i++){
		if((obj=chip->msgobj[i])==NULL)
			continue;
		msgobj_done(obj);
		can_checked_free(obj);
	}
	
	can_checked_free(chip->chipspecops);
	chip->chipspecops=NULL;

}

/**
 * candevice_done - destroys representation of one CAN device/board
 * @candev: pointer to CAN device/board structure
 */
void candevice_done(struct candevice_t *candev)
{
	int i;
	struct canchip_t *chip;
	
  	for(i=0; i<candev->nr_all_chips; i++){
		if((chip=candev->chip[i])==NULL)
			continue;
		canchip_done(chip);
		can_checked_free(chip);
	
	}
	if(candev->flags & CANDEV_IO_RESERVED) {
		candev->hwspecops->release_io(candev);
		candev->flags &= ~CANDEV_IO_RESERVED;
	}
	can_checked_free(candev->hwspecops);
	candev->hwspecops=NULL;
}

/**
 * candevice_done - destroys representation of all CAN devices/boards
 * @canhw: pointer to the root of all CAN hardware representation
 */
void canhardware_done(struct canhardware_t *canhw)
{
	int i;
	struct candevice_t *candev;
	
  	for(i=0; i<canhw->nr_boards; i++){
		if((candev=canhw->candevice[i])==NULL)
			continue;
		candevice_done(candev);
		can_checked_free(candev);
	}

}
