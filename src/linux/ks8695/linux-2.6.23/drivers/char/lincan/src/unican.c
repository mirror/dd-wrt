/* unican.c
 * Linux CAN-bus device driver.
 * Written for new CAN driver version by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */ 

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/unican_cl2.h"
#include "../include/setup.h"

#define UNICAN_PCI_VENDOR  0xFA3C
#define UNICAN_PCI_ID      0x0101

static void unican_delay(long msdelay)
{
    #ifdef CAN_WITH_RTL
	if(!rtl_rt_system_is_idle()) {
		rtl_delay(1000000l*msdelay);
	}else
    #endif /*CAN_WITH_RTL*/
	{
		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout((msdelay*HZ)/1000+1);
	}

}

/* * * unican Chip Functionality * * */

int unican_enable_configuration(struct canchip_t *chip)
{
	return 0;
}

int unican_disable_configuration(struct canchip_t *chip)
{
	return 0;
}

/**
 * unican_chip_config: - can chip configuration
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 * File: src/unican.c
 */
int unican_chip_config(struct canchip_t *chip)
{
	int ret;
	sCAN_CARD *chipext = (sCAN_CARD *)chip->chip_data;

	unican_delay(10);
	
	/* disable all card interrupts */
	ret = cl2_int_mode(chipext, INT_MODE_ALL*0);
	if(ret != CL2_OK) {
		CANMSG("disable interrupts by cl2_iit_mode returned %d\n",ret);
		return -ENODEV;
	}
	unican_delay(1);

        if (chip->baudrate == 0)
                chip->baudrate=1000000;
		
	ret = chip->chipspecops->baud_rate(chip,chip->baudrate,chip->clock,0,75,0);
	if(ret < 0){
		CANMSG("can not set baudrate\n");
		return ret;
	}
	
	unican_delay(2);
	/* set interrupt inhibit time to 1 ms */
	ret = cl2_set_iit(chipext, 10);
	if(ret != CL2_OK) {
		CANMSG("cl2_set_iit returned %d\n",ret);
		return -ENODEV;
	}
	unican_delay(1);

	/* enable start interrupt inhibit time command */
	ret = cl2_iit_mode(chipext, 1);
	if(ret != CL2_OK) {
		CANMSG("cl2_iit_mode returned %d\n",ret);
		return -ENODEV;
	}
	unican_delay(1);
	
	/* enable all card interrupts */
	ret = cl2_int_mode(chipext, INT_MODE_ALL);
	if(ret != CL2_OK) {
		CANMSG("cl2_iit_mode returned %d\n",ret);
		return -ENODEV;
	}
	unican_delay(1);

	/* generate interrupt command */
	cl2_gen_interrupt(chipext);


	return 0;
}

/**
 * unican_extended_mask: - setup of extended mask for message filtering
 * @chip: pointer to chip state structure
 * @code: can message acceptance code
 * @mask: can message acceptance mask
 *
 * Return Value: negative value reports error.
 * File: src/unican.c
 */
int unican_extended_mask(struct canchip_t *chip, unsigned long code, unsigned  long mask)
{
	return 0;
}

/**
 * unican_baud_rate: - set communication parameters.
 * @chip: pointer to chip state structure
 * @rate: baud rate in Hz
 * @clock: frequency of sja1000 clock in Hz (ISA osc is 14318000)
 * @sjw: synchronization jump width (0-3) prescaled clock cycles
 * @sampl_pt: sample point in % (0-100) sets (TSEG1+1)/(TSEG1+TSEG2+2) ratio
 * @flags: fields %BTR1_SAM, %OCMODE, %OCPOL, %OCTP, %OCTN, %CLK_OFF, %CBP
 *
 * Return Value: negative value reports error.
 * File: src/unican.c
 */
int unican_baud_rate(struct canchip_t *chip, int rate, int clock, int sjw,
							int sampl_pt, int flags)
{
	int ret;
	sCAN_CARD *chipext = (sCAN_CARD *)chip->chip_data;
	int bt_val;

	switch (rate) {
		case 5000:   bt_val = CL2_BITRATE_5K; break;
		case 10000:  bt_val = CL2_BITRATE_10K; break;
		case 20000:  bt_val = CL2_BITRATE_20K; break;
		case 50000:  bt_val = CL2_BITRATE_50K; break;
		case 100000: bt_val = CL2_BITRATE_100K; break;
		case 125000: bt_val = CL2_BITRATE_125K; break;
		case 200000: bt_val = CL2_BITRATE_200K; break;
		case 250000: bt_val = CL2_BITRATE_250K; break;
		case 500000: bt_val = CL2_BITRATE_500K; break;
		case 800000: bt_val = CL2_BITRATE_800K; break;
		case 1000000:bt_val = CL2_BITRATE_1M; break;
		default: return -EINVAL;
	}
	
	ret=cl2_set_bitrate(chipext,bt_val);
	if(ret == CL2_COMMAND_BUSY) return -EBUSY;
	if(ret != CL2_OK) return -EINVAL;
	unican_delay(2);
	
	return 0;
}

/**
 * unican_read: - reads and distributes one or more received messages
 * @chip: pointer to chip state structure
 * @obj: pinter to CAN message queue information
 *
 * This is rewritten cl2_receive_data function. The direct use of CL2
 * function would require one more message data copy to reformat message
 * data into different structure layout. Other way is to rewrite CL2 sources.
 * No of these solutions is perfect.
 *
 * File: src/unican.c
 */
void unican_read(struct canchip_t *chip, struct msgobj_t *obj) {
	sCAN_CARD *chipext = (sCAN_CARD *)chip->chip_data;
	__u16 *ptr16;
	__u16 u;
	unsigned long timestamp;
	int i;

	do {
		ptr16 = (__u16*)chipext->rxBufPtr;
		u = unican_readw(ptr16++);
		if ( !(u & CL2_MESSAGE_VALID) ) break; /* No more messages in the queue */

		obj->rx_msg.id = ((__u32)(u & 0xFF00 )) << 16;
		u = unican_readw(ptr16++);
		obj->rx_msg.id |= ((__u32)( u & 0x00FF )) << 16;
		obj->rx_msg.id |= (__u32)( u & 0xFF00 );
		u = unican_readw(ptr16++);
		obj->rx_msg.id |= (__u32)( u & 0x00FF );


		u >>= 8;

		if ( u & CL2_EXT_FRAME ) {	/* 2.0B frame */
			obj->rx_msg.id >>= 3;
			obj->rx_msg.flags = MSG_EXT;
		} else {			/* 2.0A frame */
			obj->rx_msg.id >>= 21;
			obj->rx_msg.flags = 0;
		}

		/*if ( !(u & (CL2_REMOTE_FRAME<<8)) ) 
			obj->rx_msg.flags |= MSG_RTR;*/

		obj->rx_msg.length = ( (u >> 4) & 0x000F );
		if(obj->rx_msg.length > CAN_MSG_LENGTH) obj->rx_msg.length = CAN_MSG_LENGTH;

		for ( i = 0; i < obj->rx_msg.length; ) {
			u = unican_readw(ptr16++);
			obj->rx_msg.data[i++] = (__u8)( u );
			obj->rx_msg.data[i++] = (__u8)( u >> 8 );
		}
		if ( obj->rx_msg.length & 0x01 ) {	/* odd */
			timestamp = ( (unican_readw(ptr16++) & 0x00FF) | (u & 0xFF00) );
		} else {				/* even */
			u = unican_readw(ptr16++);
			timestamp = (u << 8) | (u >> 8);
		}
		unican_writew(0x000,(__u16*)chipext->rxBufPtr);

	       #ifdef CAN_MSG_VERSION_2
		obj->rx_msg.timestamp.tv_sec = 0;
		obj->rx_msg.timestamp.tv_usec = timestamp;
               #else /* CAN_MSG_VERSION_2 */
		obj->rx_msg.timestamp = timestamp;
               #endif /* CAN_MSG_VERSION_2 */
	       
		/* increment rx-buffer pointer */
		if ( (chipext->rxBufBase + chipext->rxBufSize*16 ) <= (chipext->rxBufPtr += 16) ) {
			chipext->rxBufPtr = chipext->rxBufBase;
		}

		canque_filter_msg2edges(obj->qends, &obj->rx_msg);

	} while (1);
}

/**
 * unican_pre_read_config: - prepares message object for message reception
 * @chip: pointer to chip state structure
 * @obj: pointer to message object state structure
 *
 * Return Value: negative value reports error.
 *	Positive value indicates immediate reception of message.
 * File: src/unican.c
 */
int unican_pre_read_config(struct canchip_t *chip, struct msgobj_t *obj)
{
	return 0;
}

#define MAX_TRANSMIT_WAIT_LOOPS 10
/**
 * unican_pre_write_config: - prepares message object for message transmission
 * @chip: pointer to chip state structure
 * @obj: pointer to message object state structure
 * @msg: pointer to CAN message
 *
 * Return Value: negative value reports error.
 * File: src/unican.c
 */
int unican_pre_write_config(struct canchip_t *chip, struct msgobj_t *obj, 
							struct canmsg_t *msg)
{
	return 0;
}

/**
 * unican_send_msg: - initiate message transmission
 * @chip: pointer to chip state structure
 * @obj: pointer to message object state structure
 * @msg: pointer to CAN message
 *
 * This function is called after unican_pre_write_config() function,
 * which prepares data in chip buffer.
 * Return Value: negative value reports error.
 * File: src/unican.c
 */
int unican_send_msg(struct canchip_t *chip, struct msgobj_t *obj, 
							struct canmsg_t *msg)
{
	return 0;
}

/**
 * unican_check_tx_stat: - checks state of transmission engine
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 *	Positive return value indicates transmission under way status.
 *	Zero value indicates finishing of all issued transmission requests.
 * File: src/unican.c
 */
int unican_check_tx_stat(struct canchip_t *chip)
{
	return 0;
}

/**
 * unican_set_btregs: -  configures bitrate registers
 * @chip: pointer to chip state structure
 * @btr0: bitrate register 0
 * @btr1: bitrate register 1
 *
 * Return Value: negative value reports error.
 * File: src/unican.c
 */
int unican_set_btregs(struct canchip_t *chip, unsigned short btr0, 
							unsigned short btr1)
{
	int ret;
	sCAN_CARD *chipext = (sCAN_CARD *)chip->chip_data;
	int bt_val;

	bt_val=btr0 | (btr1<<8);
	ret=cl2_set_bitrate(chipext,bt_val);
	if(ret == CL2_COMMAND_BUSY) return -EBUSY;
	if(ret != CL2_OK) return -EINVAL;

	return 0;
}

/**
 * unican_stop_chip: -  starts chip message processing
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 * File: src/unican.c
 */
int unican_start_chip(struct canchip_t *chip)
{
	return 0;
}

/**
 * unican_stop_chip: -  stops chip message processing
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 * File: src/unican.c
 */
int unican_stop_chip(struct canchip_t *chip)
{
	return 0;
}

/**
 * unican_attach_to_chip: - attaches to the chip, setups registers and state
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 * File: src/sja1000p.c
 */
int unican_attach_to_chip(struct canchip_t *chip)
{
	return 0;
}

/**
 * unican_release_chip: - called before chip structure removal if %CHIP_ATTACHED is set
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 * File: src/sja1000p.c
 */
int unican_release_chip(struct canchip_t *chip)
{
	sCAN_CARD *chipext = (sCAN_CARD *)chip->chip_data;

	unican_stop_chip(chip);
	cl2_clear_interrupt(chipext);

	return 0;
}

/**
 * unican_remote_request: - configures message object and asks for RTR message
 * @chip: pointer to chip state structure
 * @obj: pointer to message object structure
 *
 * Return Value: negative value reports error.
 * File: src/unican.c
 */
int unican_remote_request(struct canchip_t *chip, struct msgobj_t *obj)
{
	CANMSG("unican_remote_request not implemented\n");
	return -ENOSYS;
}

/**
 * unican_standard_mask: - setup of mask for message filtering
 * @chip: pointer to chip state structure
 * @code: can message acceptance code
 * @mask: can message acceptance mask
 *
 * Return Value: negative value reports error.
 * File: src/unican.c
 */
int unican_standard_mask(struct canchip_t *chip, unsigned short code,
		unsigned short mask)
{
	CANMSG("unican_standard_mask not implemented\n");
	return -ENOSYS;
}

/**
 * unican_clear_objects: - clears state of all message object residing in chip
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 * File: src/unican.c
 */
int unican_clear_objects(struct canchip_t *chip)
{
	CANMSG("unican_clear_objects not implemented\n");
	return -ENOSYS;
}

/**
 * unican_config_irqs: - tunes chip hardware interrupt delivery
 * @chip: pointer to chip state structure
 * @irqs: requested chip IRQ configuration
 *
 * Return Value: negative value reports error.
 * File: src/unican.c
 */
int unican_config_irqs(struct canchip_t *chip, short irqs)
{

	CANMSG("unican_config_irqs not implemented\n");
	return -ENOSYS;
}

/**
 * unican_irq_write_handler: - part of ISR code responsible for transmit events
 * @chip: pointer to chip state structure
 * @obj: pointer to attached queue description
 *
 * The main purpose of this function is to read message from attached queues
 * and transfer message contents into CAN controller chip.
 * This subroutine is called by
 * unican_irq_write_handler() for transmit events.
 * File: src/unican.c
 */
void unican_irq_write_handler(struct canchip_t *chip, struct msgobj_t *obj)
{
	int cmd;
	sCAN_CARD *chipext = (sCAN_CARD *)chip->chip_data;
	__u16 *ptr16 = (__u16*)chipext->rxBufPtr;
	__u16 u;
	unsigned long timestamp=0;
	unsigned long cobid;
	int i;
	int len;

       #if 0
	if(obj->tx_slot){
		/* Do local transmitted message distribution if enabled */
		if (processlocal){
			obj->tx_slot->msg.flags |= MSG_LOCAL;
			canque_filter_msg2edges(obj->qends, &obj->tx_slot->msg);
		}
		/* Free transmitted slot */
		canque_free_outslot(obj->qends, obj->tx_qedge, obj->tx_slot);
		obj->tx_slot=NULL;
	}
       #endif

	if ( chipext->asyncTxBufSize==0 ) {
		canque_notify_inends(obj->tx_qedge, CANQUEUE_NOTIFY_ERRTX_PREP);
		return;	/* No asynchronous queue configured */
	}

	do {
		ptr16 = (__u16*)chipext->asyncTxBufPtr;
		if(unican_readw(ptr16) & CL2_MESSAGE_VALID)
			return;		/* No free space in asynchronous Tx queue */

		cmd=canque_test_outslot(obj->qends, &obj->tx_qedge, &obj->tx_slot);
		if(cmd<0)
			return;		/* No more messages to send */
		

		cobid = obj->tx_slot->msg.id;
		
		if ( (obj->tx_slot->msg.flags & MSG_EXT) ) {	/* 2.0B frame */
			cobid <<= 3;
		} else {					/* 2.0A frame */
			cobid <<= 5+16;
		}
		ptr16++;
		u = ((cobid>>16) & 0x00FF ) + (cobid & 0xFF00);
		unican_writew(u,ptr16++);

		len = obj->tx_slot->msg.length;
		if(len > CAN_MSG_LENGTH)
			len = CAN_MSG_LENGTH;
		u = (len << 12) | (cobid & 0x00FF);
		
   		if ( !(obj->tx_slot->msg.flags & MSG_RTR) ) 
			u |= CL2_REMOTE_FRAME<<8;
   		if ( obj->tx_slot->msg.flags & MSG_EXT ) 
			u |= CL2_EXT_FRAME<<8;

		unican_writew(u,ptr16++);

		for ( i = 0; i < len-1; )	{
			u = obj->tx_slot->msg.data[i++];
			u |= ((__u16)obj->tx_slot->msg.data[i]<<8); i++;
			unican_writew(u,ptr16++);
		}
		if(i == len) {
			unican_writew(timestamp,ptr16);
		} else {
			u = obj->tx_slot->msg.data[i++];
			u |= ((timestamp & 0x00FF)<<8);
			unican_writew(u,ptr16++);
			unican_writew(timestamp & 0x00FF, ptr16);
		}

		u = ((cobid>>16) & 0xFF00) | CL2_MESSAGE_VALID;
		unican_writew(u,(__u16*)chipext->asyncTxBufPtr);

		if ( (chipext->asyncTxBufBase + chipext->asyncTxBufSize*16) <= 
						(chipext->asyncTxBufPtr += 16) ) {
			chipext->asyncTxBufPtr = chipext->asyncTxBufBase;
		}


		/* Do local transmitted message distribution if enabled. */
		/* This code should not be called directly there, because it breaks strict
		   behavior of queues if O_SYNC is set. */
		if (processlocal){
			obj->tx_slot->msg.flags |= MSG_LOCAL;
			canque_filter_msg2edges(obj->qends, &obj->tx_slot->msg);
		}
		/* Free transmitted slot */
		canque_free_outslot(obj->qends, obj->tx_qedge, obj->tx_slot);
		obj->tx_slot=NULL;
	
	}while(1);
	
	return;

}

void unican_irq_sync_activities(struct canchip_t *chip, struct msgobj_t *obj)
{
	while(!can_msgobj_test_and_set_fl(obj,TX_LOCK)) {

		if(can_msgobj_test_and_clear_fl(obj,TX_REQUEST)) {
			unican_irq_write_handler(chip, obj);
		}

		/*if(can_msgobj_test_and_clear_fl(obj,FILTCH_REQUEST)) {
			unican_irq_update_filter(chip, obj);
		}*/

		can_msgobj_clear_fl(obj,TX_LOCK);
		if(can_msgobj_test_fl(obj,TX_REQUEST))
			continue;
		if(can_msgobj_test_fl(obj,FILTCH_REQUEST) && !obj->tx_slot)
			continue;
		break;
	}
}


#define MAX_RETR 10

/**
 * unican_irq_handler: - interrupt service routine
 * @irq: interrupt vector number, this value is system specific
 * @chip: pointer to chip state structure
 * 
 * Interrupt handler is activated when state of CAN controller chip changes,
 * there is message to be read or there is more space for new messages or
 * error occurs. The receive events results in reading of the message from
 * CAN controller chip and distribution of message through attached
 * message queues.
 * File: src/unican.c
 */
int unican_irq_handler(int irq, struct canchip_t *chip)
{
	sCAN_CARD *chipext = (sCAN_CARD *)chip->chip_data;
	struct msgobj_t *obj=chip->msgobj[0];
	__u16 status;
	__u16 error;

	if(!(chip->flags&CHIP_CONFIGURED)) {
		CANMSG("unican_irq_handler: called for non-configured device\n");
		return CANCHIP_IRQ_NONE;
	}

	if (cl2_get_status(chipext, &status) == CL2_NO_REQUEST) {
		/* Reenable interrupts generation, this has to be even there, 
		 * because irq_accept disables interrupts
		 */
		cl2_gen_interrupt(chipext);
		return CANCHIP_IRQ_NONE;
	}

	cl2_clear_interrupt(chipext);


	if(status & CL2_CARD_ERROR) {
		cl2_get_error(chipext, &error);
		CANMSG("unican_irq_handler: card status=0x%04x error=0x%04x \n",status,error);
	}
	if(status & CL2_ASYNC_QUEUE_EMPTY) {

	}
	if(status & CL2_SYNC_QUEUE_EMPTY) {
		can_msgobj_set_fl(obj,TX_REQUEST);

		/* calls unican_irq_write_handler synchronized with other invocations */
		unican_irq_sync_activities(chip, obj);

	}
	if(status & CL2_DATA_IN_RBUF) {
		unican_read(chip, obj);
	}

	/* Reenable interrupts generation */
	cl2_gen_interrupt(chipext);

	return CANCHIP_IRQ_HANDLED;
}


/**
 * unican_irq_accept: - fast irq accept routine, blocks further interrupts
 * @irq: interrupt vector number, this value is system specific
 * @chip: pointer to chip state structure
 * 
 * This routine only accepts interrupt reception and stops further
 * incoming interrupts, but does not handle situation causing interrupt.
 * File: src/unican.c
 */
int unican_irq_accept(int irq, struct canchip_t *chip)
{
	sCAN_CARD *chipext = (sCAN_CARD *)chip->chip_data;

	cl2_clear_interrupt(chipext);

	return CANCHIP_IRQ_ACCEPTED;
}

/*void unican_do_tx_timeout(unsigned long data)
{
	struct msgobj_t *obj=(struct msgobj_t *)data;
	
}*/

/**
 * unican_wakeup_tx: - wakeups TX processing
 * @chip: pointer to chip state structure
 * @obj: pointer to message object structure
 *
 * Return Value: negative value reports error.
 * File: src/unican.c
 */
int unican_wakeup_tx(struct canchip_t *chip, struct msgobj_t *obj)
{
	can_preempt_disable();

	can_msgobj_set_fl(obj,TX_REQUEST);

	/* calls unican_irq_write_handler synchronized with other invocations
	  from kernel and IRQ context */
	unican_irq_sync_activities(chip, obj);

	can_preempt_enable();

	return 0;
}


/* * * unican Board Functionality * * */

#define IO_RANGE 0x1000

/**
 * unican_request_io: - reserve io or memory range for can board
 * @candev: pointer to candevice/board which asks for io. Field @io_addr
 *	of @candev is used in most cases to define start of the range
 *
 * Return Value: The function returns zero on success or %-ENODEV on failure
 * File: src/unican.c
 */
int unican_request_io(struct candevice_t *candev)
{
        can_ioptr_t remap_addr;
	if (!can_request_mem_region(candev->io_addr,IO_RANGE,DEVICE_NAME " - unican")) {
		CANMSG("Unable to request IO-memory: 0x%lx\n",candev->io_addr);
		return -ENODEV;
	}
	if ( !( remap_addr = ioremap( candev->io_addr, IO_RANGE ) ) ) {
		CANMSG("Unable to access I/O memory at: 0x%lx\n", candev->io_addr);
		can_release_mem_region(candev->io_addr,IO_RANGE);
		return -ENODEV;
	
	}
	can_base_addr_fixup(candev, remap_addr);
	DEBUGMSG("Registered IO-memory: 0x%lx - 0x%lx\n", candev->io_addr, candev->io_addr + IO_RANGE - 1);
	return 0;
}

/**
 * unican_elease_io - free reserved io memory range
 * @candev: pointer to candevice/board which releases io
 *
 * Return Value: The function always returns zero
 * File: src/unican.c
 */
int unican_release_io(struct candevice_t *candev)
{
	iounmap(candev->dev_base_addr);
	can_release_mem_region(candev->io_addr,IO_RANGE);
	return 0;
}

/**
 * unican_reset - hardware reset routine
 * @candev: Pointer to candevice/board structure
 *
 * Return Value: The function returns zero on success or %-ENODEV on failure
 * File: src/unican.c
 */
int unican_reset(struct candevice_t *candev)
{
	int ret;
	int i;
	struct canchip_t *chip = candev->chip[0];
	sCAN_CARD *chipext;
	

	if(chip->chip_data == NULL) {
		chip->chip_data = can_checked_malloc(sizeof(sCAN_CARD));
		if(!chip->chip_data) return -ENOMEM;
		memset(chip->chip_data,0,sizeof(sCAN_CARD));
		ret = cl2_init_card(chip->chip_data,(void*)chip->chip_base_addr,chip->chip_irq);
		if(ret != CL2_OK){
			CANMSG("cl2_init_card returned %d\n",ret);
			return -ENODEV;
		}
	}
	
	chipext = (sCAN_CARD *)chip->chip_data;
		
	i = 0;
	/* reset and test whether the card is present */
	do {
		cl2_reset_card(chipext);
		unican_delay(10);
		i++;
		ret = cl2_test_card(chipext);
	} while((ret != CL2_OK)&&(i<10));

	if(ret != CL2_OK) {
		CANMSG("card check failed %d\n",ret);
		return -ENODEV;
	}
	
	/* start card firmware */
	ret = cl2_start_firmware(chipext);
	if(ret != CL2_OK){
		CANMSG("cl2_start_firmware returned %d\n",ret);
		return -ENODEV;
	}
	
        unican_delay(100);

	return 0;
}

/**
 * unican_init_hw_data - Initialize hardware cards
 * @candev: Pointer to candevice/board structure
 *
 * Return Value: The function always returns zero
 * File: src/unican.c
 */
int unican_init_hw_data(struct candevice_t *candev) 
{
	candev->res_addr=0;
	candev->nr_82527_chips=0;
	candev->nr_sja1000_chips=0;
	candev->nr_all_chips=1;
	candev->flags |= CANDEV_PROGRAMMABLE_IRQ*0;

	return 0;
}

/**
 * unican_init_chip_data - Initialize chips
 * @candev: Pointer to candevice/board structure
 * @chipnr: Number of the CAN chip on the hardware card
 *
 * Return Value: The function always returns zero
 * File: src/unican.c
 */
int unican_init_chip_data(struct candevice_t *candev, int chipnr)
{
	struct canchip_t *chip = candev->chip[chipnr];
	chip->chip_type = "unican";
	chip->chip_base_addr = 0;
	chip->clock = 10000000;
	chip->int_clk_reg = 0x0;
	chip->int_bus_reg = 0x0;
	chip->max_objects = 1;
	chip->chip_base_addr=candev->dev_base_addr;
			
	CANMSG("initializing unican chip operations\n");
	chip->chipspecops->chip_config=unican_chip_config;
	chip->chipspecops->baud_rate=unican_baud_rate;
	chip->chipspecops->standard_mask=unican_standard_mask;
	chip->chipspecops->extended_mask=unican_extended_mask;
	chip->chipspecops->message15_mask=unican_extended_mask;
	chip->chipspecops->clear_objects=unican_clear_objects;
	chip->chipspecops->config_irqs=unican_config_irqs;
	chip->chipspecops->pre_read_config=unican_pre_read_config;
	chip->chipspecops->pre_write_config=unican_pre_write_config;
	chip->chipspecops->send_msg=unican_send_msg;
	chip->chipspecops->check_tx_stat=unican_check_tx_stat;
	chip->chipspecops->wakeup_tx=unican_wakeup_tx;
	chip->chipspecops->remote_request=unican_remote_request;
	chip->chipspecops->enable_configuration=unican_enable_configuration;
	chip->chipspecops->disable_configuration=unican_disable_configuration;
	chip->chipspecops->set_btregs=unican_set_btregs;
	chip->chipspecops->attach_to_chip=unican_attach_to_chip;
	chip->chipspecops->release_chip=unican_release_chip;
	chip->chipspecops->start_chip=unican_start_chip;
	chip->chipspecops->stop_chip=unican_stop_chip;
	chip->chipspecops->irq_handler=unican_irq_handler;
	chip->chipspecops->irq_accept=unican_irq_accept;

	return 0;
}

/**
 * unican_init_obj_data - Initialize message buffers
 * @chip: Pointer to chip specific structure
 * @objnr: Number of the message buffer
 *
 * Return Value: The function always returns zero
 * File: src/unican.c
 */
int unican_init_obj_data(struct canchip_t *chip, int objnr)
{
	struct msgobj_t *obj=chip->msgobj[objnr];
	obj->obj_base_addr=chip->chip_base_addr;
	/*obj->tx_timeout.function=unican_do_tx_timeout;
	obj->tx_timeout.data=(unsigned long)obj;*/
	return 0;
}

/**
 * unican_program_irq - program interrupts
 * @candev: Pointer to candevice/board structure
 *
 * Return value: The function returns zero on success or %-ENODEV on failure
 * File: src/unican.c
 */
int unican_program_irq(struct candevice_t *candev)
{
	return 0;
}

int unican_register(struct hwspecops_t *hwspecops)
{
	hwspecops->request_io = unican_request_io;
	hwspecops->release_io = unican_release_io;
	hwspecops->reset = unican_reset;
	hwspecops->init_hw_data = unican_init_hw_data;
	hwspecops->init_chip_data = unican_init_chip_data;
	hwspecops->init_obj_data = unican_init_obj_data;
	hwspecops->write_register = NULL;
	hwspecops->read_register = NULL;
	hwspecops->program_irq = unican_program_irq;
	return 0;
}


/* Unicontrols PCI board specific functions */

#ifdef CAN_ENABLE_PCI_SUPPORT

int unican_pci_request_io(struct candevice_t *candev)
{
        can_ioptr_t remap_addr;

    #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))
	if(pci_request_region(candev->sysdevptr.pcidev, 0, "unican_pci") != 0){
		CANMSG("Request of Unican PCI range failed\n");
		return -ENODEV;
	}
    #else /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/
	if(pci_request_regions(candev->sysdevptr.pcidev, "kv_pcican") != 0){
		CANMSG("Request of Unican PCI range failed\n");
		return -ENODEV;
	}
    #endif /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/

	candev->io_addr=pci_resource_start(candev->sysdevptr.pcidev,0);
	candev->res_addr=candev->io_addr;

	if ( !( remap_addr = ioremap( candev->io_addr, IO_RANGE ) ) ) {
		CANMSG("Unable to access I/O memory at: 0x%lx\n", candev->io_addr);
	    #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))
		pci_release_region(candev->sysdevptr.pcidev, 0);
	    #else /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/
		pci_release_regions(candev->sysdevptr.pcidev);
	    #endif /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/
		return -ENODEV;
	
	}
	can_base_addr_fixup(candev, remap_addr);
	DEBUGMSG("Registered IO-memory: 0x%lx - 0x%lx\n", candev->io_addr, candev->io_addr + IO_RANGE - 1);
	DEBUGMSG("VMA: dev_base_addr: 0x%lx chip_base_addr: 0x%lx\n", 
		can_ioptr2ulong(candev->dev_base_addr),
		can_ioptr2ulong(candev->chip[0]->chip_base_addr));

	return 0;
}


int unican_pci_release_io(struct candevice_t *candev)
{
	iounmap(candev->dev_base_addr);
    #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))
	pci_release_region(candev->sysdevptr.pcidev, 0);
    #else /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/
	pci_release_regions(candev->sysdevptr.pcidev);
    #endif /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/
	return 0;
}


int unican_pci_init_hw_data(struct candevice_t *candev)
{
	struct pci_dev *pcidev = NULL;

	do {
		pcidev = pci_find_device(UNICAN_PCI_VENDOR, UNICAN_PCI_ID, pcidev);
		if(pcidev == NULL) return -ENODEV;
	} while(can_check_dev_taken(pcidev));
	
	if (pci_enable_device (pcidev)){
		printk(KERN_CRIT "Setup of Unican PCI failed\n");
		return -EIO;
	}
	candev->sysdevptr.pcidev=pcidev;
	
	if(!(pci_resource_flags(pcidev,0)&IORESOURCE_MEM)){
		printk(KERN_CRIT "Unican PCI region 0 is not MEM\n");
		return -EIO;
	}
	candev->io_addr=pci_resource_start(pcidev,0);
	candev->res_addr=candev->io_addr;
	candev->dev_base_addr=NULL;
	
	/*candev->flags |= CANDEV_PROGRAMMABLE_IRQ;*/

	candev->nr_82527_chips=0;
	candev->nr_sja1000_chips=0;
	candev->nr_all_chips=1;

	return 0;
}


int unican_pci_init_chip_data(struct candevice_t *candev, int chipnr)
{
	int ret;
	candev->chip[chipnr]->chip_irq=candev->sysdevptr.pcidev->irq;
	ret = unican_init_chip_data(candev, chipnr);
	candev->chip[chipnr]->flags |= CHIP_IRQ_PCI;
	return ret;
}

int unican_pci_register(struct hwspecops_t *hwspecops)
{
	hwspecops->request_io = unican_pci_request_io;
	hwspecops->release_io = unican_pci_release_io;
	hwspecops->reset = unican_reset;
	hwspecops->init_hw_data = unican_pci_init_hw_data;
	hwspecops->init_chip_data = unican_pci_init_chip_data;
	hwspecops->init_obj_data = unican_init_obj_data;
	hwspecops->write_register = NULL;
	hwspecops->read_register = NULL;
	hwspecops->program_irq = unican_program_irq;
	return 0;
}

#endif /*CAN_ENABLE_PCI_SUPPORT*/

#ifdef CAN_ENABLE_VME_SUPPORT

#include "unican_vme.c"

#endif /*CAN_ENABLE_VME_SUPPORT*/
