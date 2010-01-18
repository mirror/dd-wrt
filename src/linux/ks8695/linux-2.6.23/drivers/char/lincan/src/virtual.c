/* virtual.c
 * Linux CAN-bus device driver.
 * Written for new CAN driver version by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */ 

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"


long virtual_bus_latency(struct msgobj_t *obj)
{
	long latency;
	latency=obj->hostchip->baudrate;
	if(latency){
		latency=(long)HZ*1000/latency;
	}
	return latency;
}


/* * * Virtual Chip Functionality * * */

int virtual_enable_configuration(struct canchip_t *chip)
{
	return 0;
}

int virtual_disable_configuration(struct canchip_t *chip)
{
	return 0;
}

/**
 * virtual_chip_config: - can chip configuration
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 * File: src/virtual.c
 */
int virtual_chip_config(struct canchip_t *chip)
{
	return 0;
}

/**
 * virtual_extended_mask: - setup of extended mask for message filtering
 * @chip: pointer to chip state structure
 * @code: can message acceptance code
 * @mask: can message acceptance mask
 *
 * Return Value: negative value reports error.
 * File: src/virtual.c
 */
int virtual_extended_mask(struct canchip_t *chip, unsigned long code, unsigned  long mask)
{
	return 0;
}

/**
 * virtual_baud_rate: - set communication parameters.
 * @chip: pointer to chip state structure
 * @rate: baud rate in Hz
 * @clock: frequency of sja1000 clock in Hz (ISA osc is 14318000)
 * @sjw: synchronization jump width (0-3) prescaled clock cycles
 * @sampl_pt: sample point in % (0-100) sets (TSEG1+1)/(TSEG1+TSEG2+2) ratio
 * @flags: fields %BTR1_SAM, %OCMODE, %OCPOL, %OCTP, %OCTN, %CLK_OFF, %CBP
 *
 * Return Value: negative value reports error.
 * File: src/virtual.c
 */
int virtual_baud_rate(struct canchip_t *chip, int rate, int clock, int sjw,
							int sampl_pt, int flags)
{
	return 0;
}

/**
 * virtual_read: - reads and distributes one or more received messages
 * @chip: pointer to chip state structure
 * @obj: pinter to CAN message queue information
 *
 * File: src/virtual.c
 */
void virtual_read(struct canchip_t *chip, struct msgobj_t *obj) {

}

/**
 * virtual_pre_read_config: - prepares message object for message reception
 * @chip: pointer to chip state structure
 * @obj: pointer to message object state structure
 *
 * Return Value: negative value reports error.
 *	Positive value indicates immediate reception of message.
 * File: src/virtual.c
 */
int virtual_pre_read_config(struct canchip_t *chip, struct msgobj_t *obj)
{
	return 0;
}

#define MAX_TRANSMIT_WAIT_LOOPS 10
/**
 * virtual_pre_write_config: - prepares message object for message transmission
 * @chip: pointer to chip state structure
 * @obj: pointer to message object state structure
 * @msg: pointer to CAN message
 *
 * Return Value: negative value reports error.
 * File: src/virtual.c
 */
int virtual_pre_write_config(struct canchip_t *chip, struct msgobj_t *obj, 
							struct canmsg_t *msg)
{
	return 0;
}

/**
 * virtual_send_msg: - initiate message transmission
 * @chip: pointer to chip state structure
 * @obj: pointer to message object state structure
 * @msg: pointer to CAN message
 *
 * This function is called after virtual_pre_write_config() function,
 * which prepares data in chip buffer.
 * Return Value: negative value reports error.
 * File: src/virtual.c
 */
int virtual_send_msg(struct canchip_t *chip, struct msgobj_t *obj, 
							struct canmsg_t *msg)
{
	return 0;
}

/**
 * virtual_check_tx_stat: - checks state of transmission engine
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 *	Positive return value indicates transmission under way status.
 *	Zero value indicates finishing of all issued transmission requests.
 * File: src/virtual.c
 */
int virtual_check_tx_stat(struct canchip_t *chip)
{
	return 0;
}

/**
 * virtual_set_btregs: -  configures bitrate registers
 * @chip: pointer to chip state structure
 * @btr0: bitrate register 0
 * @btr1: bitrate register 1
 *
 * Return Value: negative value reports error.
 * File: src/virtual.c
 */
int virtual_set_btregs(struct canchip_t *chip, unsigned short btr0, 
							unsigned short btr1)
{
	return 0;
}

/**
 * virtual_stop_chip: -  starts chip message processing
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 * File: src/virtual.c
 */
int virtual_start_chip(struct canchip_t *chip)
{
	return 0;
}

/**
 * virtual_stop_chip: -  stops chip message processing
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 * File: src/virtual.c
 */
int virtual_stop_chip(struct canchip_t *chip)
{
	return 0;
}

/**
 * virtual_attach_to_chip: - attaches to the chip, setups registers and state
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 * File: src/virtual.c
 */
int virtual_attach_to_chip(struct canchip_t *chip)
{
	return 0;
}

/**
 * virtual_release_chip: - called before chip structure removal if %CHIP_ATTACHED is set
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 * File: src/virtual.c
 */
int virtual_release_chip(struct canchip_t *chip)
{
	virtual_stop_chip(chip);
	return 0;
}

/**
 * virtual_remote_request: - configures message object and asks for RTR message
 * @chip: pointer to chip state structure
 * @obj: pointer to message object structure
 *
 * Return Value: negative value reports error.
 * File: src/virtual.c
 */
int virtual_remote_request(struct canchip_t *chip, struct msgobj_t *obj)
{
	CANMSG("virtual_remote_request not implemented\n");
	return -ENOSYS;
}

/**
 * virtual_standard_mask: - setup of mask for message filtering
 * @chip: pointer to chip state structure
 * @code: can message acceptance code
 * @mask: can message acceptance mask
 *
 * Return Value: negative value reports error.
 * File: src/virtual.c
 */
int virtual_standard_mask(struct canchip_t *chip, unsigned short code,
		unsigned short mask)
{
	CANMSG("virtual_standard_mask not implemented\n");
	return -ENOSYS;
}

/**
 * virtual_clear_objects: - clears state of all message object residing in chip
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 * File: src/virtual.c
 */
int virtual_clear_objects(struct canchip_t *chip)
{
	CANMSG("virtual_clear_objects not implemented\n");
	return -ENOSYS;
}

/**
 * virtual_config_irqs: - tunes chip hardware interrupt delivery
 * @chip: pointer to chip state structure
 * @irqs: requested chip IRQ configuration
 *
 * Return Value: negative value reports error.
 * File: src/virtual.c
 */
int virtual_config_irqs(struct canchip_t *chip, short irqs)
{
	CANMSG("virtual_config_irqs not implemented\n");
	return -ENOSYS;
}

/**
 * virtual_irq_write_handler: - part of ISR code responsible for transmit events
 * @chip: pointer to chip state structure
 * @obj: pointer to attached queue description
 *
 * The main purpose of this function is to read message from attached queues
 * and transfer message contents into CAN controller chip.
 * This subroutine is called by
 * virtual_irq_write_handler() for transmit events.
 * File: src/virtual.c
 */
void virtual_irq_write_handler(struct canchip_t *chip, struct msgobj_t *obj)
{

}

#define MAX_RETR 10

/**
 * virtual_irq_handler: - interrupt service routine
 * @irq: interrupt vector number, this value is system specific
 * @dev_id: driver private pointer registered at time of request_irq() call.
 *	The CAN driver uses this pointer to store relationship of interrupt
 *	to chip state structure - @struct canchip_t
 * @regs: system dependent value pointing to registers stored in exception frame
 * 
 * Interrupt handler is activated when state of CAN controller chip changes,
 * there is message to be read or there is more space for new messages or
 * error occurs. The receive events results in reading of the message from
 * CAN controller chip and distribution of message through attached
 * message queues.
 * File: src/virtual.c
 */
int virtual_irq_handler(int irq, struct canchip_t *chip)
{
	return CANCHIP_IRQ_HANDLED;
}


void virtual_schedule_next(struct msgobj_t *obj)
{
	int cmd;

	can_preempt_disable();

	can_msgobj_set_fl(obj,TX_REQUEST);
	
	while(!can_msgobj_test_and_set_fl(obj,TX_LOCK)){

		can_msgobj_clear_fl(obj,TX_REQUEST);
		
		cmd=canque_test_outslot(obj->qends, &obj->tx_qedge, &obj->tx_slot);
		if(cmd>=0) {
			mod_timer(&obj->tx_timeout,
				jiffies+virtual_bus_latency(obj));
			DEBUGMSG("virtual: scheduled delivery\n");

		} else		
			can_msgobj_clear_fl(obj,TX_LOCK);
		
		if(!can_msgobj_test_fl(obj,TX_REQUEST)) break;
		DEBUGMSG("TX looping in virtual_schedule_next\n");
	}

	can_preempt_enable();
}


void virtual_do_tx_timeout(unsigned long data)
{
	struct msgobj_t *obj=(struct msgobj_t *)data;
	
	if(obj->tx_slot) {
		/* fill CAN message timestamp */
		can_filltimestamp(&obj->tx_slot->msg.timestamp);

		/* Deliver message to edges */
		canque_filter_msg2edges(obj->qends, &obj->tx_slot->msg);
		/* Free transmitted slot */
		canque_free_outslot(obj->qends, obj->tx_qedge, obj->tx_slot);
		obj->tx_slot=NULL;
		DEBUGMSG("virtual: delayed delivery\n");
	}
	can_msgobj_clear_fl(obj,TX_LOCK);

	virtual_schedule_next(obj);
}

/**
 * virtual_wakeup_tx: - wakeups TX processing
 * @chip: pointer to chip state structure
 * @obj: pointer to message object structure
 *
 * Function is responsible for initiating message transmition.
 * It is responsible for clearing of object TX_REQUEST flag
 *
 * Return Value: negative value reports error.
 * File: src/virtual.c
 */
int virtual_wakeup_tx(struct canchip_t *chip, struct msgobj_t *obj)
{
	/* can_msgobj_set_fl(obj,TX_REQUEST); */
	
	struct canque_edge_t *qedge;
	struct canque_slot_t *slot;
	int cmd;

	can_msgobj_clear_fl(obj,TX_REQUEST);

    #ifndef CAN_WITH_RTL
	if(!virtual_bus_latency(obj)) {
    #endif /*CAN_WITH_RTL*/
		/* Ensure delivery of all ready slots */
		while((cmd=canque_test_outslot(obj->qends, &qedge, &slot)) >= 0){
			if(cmd==0) {
				/* fill CAN message timestamp */
				can_filltimestamp(&slot->msg.timestamp);
				
				canque_filter_msg2edges(obj->qends, &slot->msg);
				DEBUGMSG("virtual: direct delivery\n");
			}
			canque_free_outslot(obj->qends, qedge, slot);
		}
    #ifndef CAN_WITH_RTL
	} else {
		virtual_schedule_next(obj);
	}
    #endif /*CAN_WITH_RTL*/

	return 0;
}


/* * * Virtual Board Functionality * * */

/**
 * virtual_request_io: - reserve io or memory range for can board
 * @candev: pointer to candevice/board which asks for io. Field @io_addr
 *	of @candev is used in most cases to define start of the range
 *
 * Return Value: The function returns zero on success or %-ENODEV on failure
 * File: src/virtual.c
 */
int virtual_request_io(struct candevice_t *candev)
{
	return 0;
}

/**
 * virtual_elease_io - free reserved io memory range
 * @candev: pointer to candevice/board which releases io
 *
 * Return Value: The function always returns zero
 * File: src/virtual.c
 */
int virtual_release_io(struct candevice_t *candev)
{
	return 0;
}

/**
 * virtual_reset - hardware reset routine
 * @candev: Pointer to candevice/board structure
 *
 * Return Value: The function returns zero on success or %-ENODEV on failure
 * File: src/virtual.c
 */
int virtual_reset(struct candevice_t *candev)
{
	return 0;
}

/**
 * virtual_init_hw_data - Initialize hardware cards
 * @candev: Pointer to candevice/board structure
 *
 * Return Value: The function always returns zero
 * File: src/virtual.c
 */
int virtual_init_hw_data(struct candevice_t *candev) 
{
	candev->res_addr=0;
	candev->nr_82527_chips=0;
	candev->nr_sja1000_chips=0;
	candev->nr_all_chips=1;
	candev->flags |= CANDEV_PROGRAMMABLE_IRQ*0;

	return 0;
}

#define CHIP_TYPE "virtual"

/**
 * virtual_init_chip_data - Initialize chips
 * @candev: Pointer to candevice/board structure
 * @chipnr: Number of the CAN chip on the hardware card
 *
 * Return Value: The function always returns zero
 * File: src/virtual.c
 */
int virtual_init_chip_data(struct candevice_t *candev, int chipnr)
{
	struct canchip_t *chip = candev->chip[chipnr];
	chip->chip_type = CHIP_TYPE;
	chip->chip_base_addr = 0;
	chip->clock = 10000000;
	chip->int_clk_reg = 0x0;
	chip->int_bus_reg = 0x0;
	chip->max_objects = 1;

	CANMSG("initializing virtual chip operations\n");
	chip->chipspecops->chip_config=virtual_chip_config;
	chip->chipspecops->baud_rate=virtual_baud_rate;
	chip->chipspecops->standard_mask=virtual_standard_mask;
	chip->chipspecops->extended_mask=virtual_extended_mask;
	chip->chipspecops->message15_mask=virtual_extended_mask;
	chip->chipspecops->clear_objects=virtual_clear_objects;
	chip->chipspecops->config_irqs=virtual_config_irqs;
	chip->chipspecops->pre_read_config=virtual_pre_read_config;
	chip->chipspecops->pre_write_config=virtual_pre_write_config;
	chip->chipspecops->send_msg=virtual_send_msg;
	chip->chipspecops->check_tx_stat=virtual_check_tx_stat;
	chip->chipspecops->wakeup_tx=virtual_wakeup_tx;
	chip->chipspecops->remote_request=virtual_remote_request;
	chip->chipspecops->enable_configuration=virtual_enable_configuration;
	chip->chipspecops->disable_configuration=virtual_disable_configuration;
	chip->chipspecops->attach_to_chip=virtual_attach_to_chip;
	chip->chipspecops->release_chip=virtual_release_chip;
	chip->chipspecops->set_btregs=virtual_set_btregs;
	chip->chipspecops->start_chip=virtual_start_chip;
	chip->chipspecops->stop_chip=virtual_stop_chip;
	chip->chipspecops->irq_handler=NULL;
	chip->chipspecops->irq_accept=NULL;

	return 0;
}

/**
 * virtual_init_obj_data - Initialize message buffers
 * @chip: Pointer to chip specific structure
 * @objnr: Number of the message buffer
 *
 * Return Value: The function always returns zero
 * File: src/virtual.c
 */
int virtual_init_obj_data(struct canchip_t *chip, int objnr)
{
	struct msgobj_t *obj=chip->msgobj[objnr];
	obj->obj_base_addr=chip->chip_base_addr;
	obj->tx_timeout.function=virtual_do_tx_timeout;
	obj->tx_timeout.data=(unsigned long)obj;
	return 0;
}

/**
 * virtual_program_irq - program interrupts
 * @candev: Pointer to candevice/board structure
 *
 * Return value: The function returns zero on success or %-ENODEV on failure
 * File: src/virtual.c
 */
int virtual_program_irq(struct candevice_t *candev)
{
	return 0;
}

int virtual_register(struct hwspecops_t *hwspecops)
{
	hwspecops->request_io = virtual_request_io;
	hwspecops->release_io = virtual_release_io;
	hwspecops->reset = virtual_reset;
	hwspecops->init_hw_data = virtual_init_hw_data;
	hwspecops->init_chip_data = virtual_init_chip_data;
	hwspecops->init_obj_data = virtual_init_obj_data;
	hwspecops->write_register = NULL;
	hwspecops->read_register = NULL;
	hwspecops->program_irq = virtual_program_irq;
	return 0;
}
