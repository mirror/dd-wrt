/* setup.c
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
#include "../include/setup.h"
#include "../include/finish.h"

int init_hwspecops(struct candevice_t *candev, int *irqnum_p);
int init_device_struct(int card, int *chan_param_idx_p, int *irq_param_idx_p);
int init_chip_struct(struct candevice_t *candev, int chipnr, int irq, long baudrate, long clock);
int init_obj_struct(struct candevice_t *candev, struct canchip_t *hostchip, int objnr);

/**
 * can_base_addr_fixup - relocates board physical memory addresses to the CPU accessible ones
 * @candev: pointer to the previously filled device/board, chips and message objects structures
 * @new_base: @candev new base address
 *
 * This function adapts base addresses of all structures of one board
 * to the new board base address.
 * It is required for translation between physical and virtual address mappings.
 * This function is prepared to simplify board specific xxx_request_io() function
 * for memory mapped devices.
 */
int can_base_addr_fixup(struct candevice_t *candev, can_ioptr_t new_base)
{
	long offs;
	int i, j;
	
	offs=new_base-candev->dev_base_addr;
	candev->dev_base_addr=new_base;
	for(i=0;i<candev->nr_all_chips;i++){
		candev->chip[i]->chip_base_addr += offs;
		for(j=0;j<candev->chip[i]->max_objects;j++)
			candev->chip[i]->msgobj[j]->obj_base_addr += offs;
	}
	return 0;
}

/**
 * can_check_dev_taken - checks if bus device description is already taken by driver
 * @anydev:	pointer to bus specific Linux device description 
 *
 * Returns: Returns 1 if device is already used by LinCAN driver, 0 otherwise.
 */
int can_check_dev_taken(void *anydev)
{
	int board_nr;
	struct candevice_t *candev;
	void *boarddev;

	for (board_nr=hardware_p->nr_boards; board_nr--; ) {
		if((candev=hardware_p->candevice[board_nr])==NULL)
			continue;
		boarddev=candev->sysdevptr.anydev;
		if(boarddev == anydev)
			return 1;
	}
	
	return 0;
}


/**
 * register_obj_struct - registers message object into global array
 * @obj: the initialized message object being registered
 * @minorbase: wanted minor number, if (-1) automatically selected
 *
 * Return Value: returns negative number in the case of fail
 */
int register_obj_struct(struct msgobj_t *obj, int minorbase)
{
	static int next_minor=0;
	int i;
	
	if(minorbase>=0)
		next_minor=minorbase;
	if(next_minor>=MAX_TOT_MSGOBJS)
		next_minor=0;
	i=next_minor;
	do{
		if(objects_p[i]==NULL){
			objects_p[i]=obj;
			obj->minor=i;
			next_minor=i+1;
			return 0;
		}
		if(++i >= MAX_TOT_MSGOBJS) i=0;
	}while(i!=next_minor);
	obj->minor=-1;
	return -1;
}


/**
 * register_chip_struct - registers chip into global array
 * @chip: the initialized chip structure being registered
 * @minorbase: wanted minor number base, if (-1) automatically selected
 *
 * Return Value: returns negative number in the case of fail
 */
int register_chip_struct(struct canchip_t *chip, int minorbase)
{
	static int next_chip_slot=0;
	int i;
	
	if(next_chip_slot>=MAX_TOT_CHIPS)
		next_chip_slot=0;
	i=next_chip_slot;
	do{
		if(chips_p[i]==NULL){
                	chips_p[i]=chip;

			next_chip_slot=i+1;
			return 0;
		}
		if(++i >= MAX_TOT_CHIPS) i=0;
	}while(i!=next_chip_slot);
	return -1;
}



/**
 * init_hw_struct - initializes driver hardware description structures
 *
 * The function init_hw_struct() is used to initialize the hardware structure.
 *
 * Return Value: returns negative number in the case of fail
 */
int init_hw_struct(void)
{
	int i=0;
	int irq_param_idx=0;
	int chan_param_idx=0;

	hardware_p->nr_boards=0;
	while ( (hw[i] != NULL) & (i < MAX_HW_CARDS) ) {
		hardware_p->nr_boards++;

		if (init_device_struct(i, &chan_param_idx, &irq_param_idx)) {
			CANMSG("Error initializing candevice_t structures.\n");
			return -ENODEV;
		}
		i++;
	}

	return 0;
}

/**
 * init_device_struct - initializes single CAN device/board
 * @card: index into @hardware_p HW description
 * @chan_param_idx_p: pointer to the index into arrays of the CAN channel parameters
 * @irq_param_idx_p: pointer to the index into arrays of the per CAN channel IRQ parameters
 *
 * The function builds representation of the one board from parameters provided
 * in the module parameters arrays: 
 *	@hw[card] .. hardware type,
 *	@io[card] .. base IO address,
 *	@baudrate[chan_param_idx] .. per channel baudrate,
 *	@minor[chan_param_idx] .. optional specification of requested channel minor base,
 *	@irq[irq_param_idx] .. one or more board/chips IRQ parameters.
 * The indexes are advanced after consumed parameters if the registration is successful.
 *
 * The hardware specific operations of the device/board are initialized by call to
 * init_hwspecops() function. Then board data are initialized by board specific 
 * init_hw_data() function. Then chips and objects representation is build by
 * init_chip_struct() function. If all above steps are successful, chips and
 * message objects are registered into global arrays. 
 *
 * Return Value: returns negative number in the case of fail
 */
int init_device_struct(int card, int *chan_param_idx_p, int *irq_param_idx_p)
{
	struct candevice_t *candev;
	int ret;
	int irqnum;
	int chipnr;
	long bd;
	int irqsig=-1;
	long clock;
	
	candev=(struct candevice_t *)can_checked_malloc(sizeof(struct candevice_t));
	if (candev==NULL)
		return -ENOMEM;

        memset(candev, 0, sizeof(struct candevice_t));

	hardware_p->candevice[card]=candev;
	candev->candev_idx=card;

	candev=candev;

	candev->hwname=hw[card];
	candev->io_addr=io[card];
	candev->dev_base_addr=(can_ioptr_t)io[card];
	clock=clockfreq[card];

	candev->hwspecops=(struct hwspecops_t *)can_checked_malloc(sizeof(struct hwspecops_t));
	if (candev->hwspecops==NULL)
		goto error_nomem;

	memset(candev->hwspecops, 0, sizeof(struct hwspecops_t));

	if (init_hwspecops(candev, &irqnum))
		goto error_nodev;

	if (candev->hwspecops->init_hw_data(candev))
		goto error_nodev;

	/* Alocate and initialize the chip structures */
	for (chipnr=0; chipnr < candev->nr_all_chips; chipnr++) {

		if(chipnr<irqnum)
			irqsig=irq[*irq_param_idx_p+chipnr];
		
		bd=baudrate[*chan_param_idx_p+chipnr];
		if(!bd) bd=baudrate[0];
	
		if ((ret=init_chip_struct(candev, chipnr, irqsig, bd*1000, clock*1000)))
			goto error_chip;
	}
	


	for (chipnr=0; chipnr < candev->nr_all_chips; chipnr++) {
		int m=minor[*chan_param_idx_p+chipnr];
		struct canchip_t *chip=candev->chip[chipnr];
		int objnr;

		register_chip_struct(chip, m);
		
		for (objnr=0; objnr<chip->max_objects; objnr++) {
			register_obj_struct(chip->msgobj[objnr], m);
			if(m>=0) m++;
		}
	}

	*irq_param_idx_p += irqnum;
	*chan_param_idx_p += candev->nr_all_chips;

	return 0;

    error_nodev:
	ret=-ENODEV;
    error_chip:
	candevice_done(candev);
	goto error_both;

    error_nomem:
	ret=-ENOMEM;

    error_both:
	hardware_p->candevice[card]=NULL;
	can_checked_free(candev);
	return ret;
    	
}

/**
 * init_chip_struct - initializes one CAN chip structure
 * @candev: pointer to the corresponding CAN device/board
 * @chipnr: index of the chip in the corresponding device/board structure
 * @irq: chip IRQ number or (-1) if not appropriate
 * @baudrate: baudrate in the units of 1Bd
 * @clock: optional chip base clock frequency in 1Hz step
 *
 * Chip structure is allocated and chip specific operations are filled by 
 * call to board specific init_chip_data() which calls chip specific
 * fill_chipspecops(). The message objects are generated by 
 * calls to init_obj_struct() function.
 *
 * Return Value: returns negative number in the case of fail
 */
int init_chip_struct(struct candevice_t *candev, int chipnr, int irq, long baudrate, long clock)
{
	struct canchip_t *chip;
	int objnr;
	int ret;

	candev->chip[chipnr]=(struct canchip_t *)can_checked_malloc(sizeof(struct canchip_t));
	if ((chip=candev->chip[chipnr])==NULL)
		return -ENOMEM;

        memset(chip, 0, sizeof(struct canchip_t));

	chip->write_register=candev->hwspecops->write_register;
	chip->read_register=candev->hwspecops->read_register;

	chip->chipspecops=can_checked_malloc(sizeof(struct chipspecops_t));
	if (chip->chipspecops==NULL)
		return -ENOMEM;
	memset(chip->chipspecops,0,sizeof(struct chipspecops_t));

	chip->chip_idx=chipnr;
	chip->hostdevice=candev;
	chip->chip_irq=irq;
	chip->baudrate=baudrate;
	chip->clock=clock;
	chip->flags=0x0;

	if(candev->hwspecops->init_chip_data(candev,chipnr)<0)
		return -ENODEV;

	for (objnr=0; objnr<chip->max_objects; objnr++) {
		ret=init_obj_struct(candev, chip, objnr);
		if(ret<0) return ret;
	}

	return 0;
}


/**
 * init_obj_struct - initializes one CAN message object structure
 * @candev: pointer to the corresponding CAN device/board
 * @hostchip: pointer to the chip containing this object
 * @objnr: index of the builded object in the chip structure
 *
 * The function initializes message object structure and allocates and initializes
 * CAN queue chip ends structure.
 *
 * Return Value: returns negative number in the case of fail
 */
int init_obj_struct(struct candevice_t *candev, struct canchip_t *hostchip, int objnr)
{
	struct canque_ends_t *qends;
	struct msgobj_t *obj;
	int ret;

	obj=(struct msgobj_t *)can_checked_malloc(sizeof(struct msgobj_t));
	hostchip->msgobj[objnr]=obj;
	if (obj == NULL) 
		return -ENOMEM;

        memset(obj, 0, sizeof(struct msgobj_t));
	obj->minor=-1;

	atomic_set(&obj->obj_used,0);
	INIT_LIST_HEAD(&obj->obj_users);
	init_timer(&obj->tx_timeout);

	qends = (struct canque_ends_t *)can_checked_malloc(sizeof(struct canque_ends_t));
	if(qends == NULL) return -ENOMEM;
	memset(qends, 0, sizeof(struct canque_ends_t));
	obj->hostchip=hostchip;
	obj->object=objnr+1;
	obj->qends=qends;
	obj->tx_qedge=NULL;
	obj->tx_slot=NULL;
	obj->obj_flags = 0x0;

	ret=canqueue_ends_init_chip(qends, hostchip, obj);
	if(ret<0) return ret;

	ret=candev->hwspecops->init_obj_data(hostchip,objnr);
	if(ret<0) return ret;
	
	return 0;
}


/**
 * init_hwspecops - finds and initializes board/device specific operations
 * @candev: pointer to the corresponding CAN device/board
 * @irqnum_p: optional pointer to the number of interrupts required by board
 *
 * The function searches board @hwname in the list of supported boards types.
 * The board type specific board_register() function is used to initialize
 * @hwspecops operations.
 *
 * Return Value: returns negative number in the case of fail
 */
int init_hwspecops(struct candevice_t *candev, int *irqnum_p)
{
	const struct boardtype_t *brp;
	
	brp = boardtype_find(candev->hwname);
	
	if(!brp) {
		CANMSG("Sorry, hardware \"%s\" is currently not supported.\n",candev->hwname);
		return -EINVAL;
	}
	
	if(irqnum_p)
		*irqnum_p=brp->irqnum;
	brp->board_register(candev->hwspecops);

	return 0;
}
