/* can_devrtl.c - CAN message queues functions for the RT-Linux
 * Linux CAN-bus device driver.
 * New CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#ifdef CAN_WITH_RTL

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/can_queue.h"
#include "../include/main.h"
#include "../include/setup.h"

#include <rtl_malloc.h>

#ifdef CAN_ENABLE_VME_SUPPORT
#include "ca91c042.h"
/* Modified version of ca91c042 driver can be found in
 * components/comm/contrib directory. */
#endif

can_spinlock_t can_irq_manipulation_lock;

unsigned int can_rtl_isr( unsigned int irq_num, struct pt_regs *r )
{
	struct canchip_t *chip;
	struct candevice_t *candev;
	int board_nr;
	int chip_nr;
	int irq2linux=0;
	int ret;
	pthread_t thread=NULL;

	DEBUGMSG("can_rtl_isr invoked for irq %d\n",irq_num);
	
	/* I hate next loop, but RT-Linux does not provide context to ISR */
	for (board_nr=hardware_p->nr_boards; board_nr--; ) {
		if((candev=hardware_p->candevice[board_nr])==NULL)
			continue;
		for(chip_nr=candev->nr_all_chips; chip_nr--; ) {
			if((chip=candev->chip[chip_nr])==NULL)
				continue;
			if(chip->chip_irq!=irq_num) continue;

			if(chip->chipspecops->irq_accept)
				ret=chip->chipspecops->irq_accept(chip->chip_irq,chip);

			set_bit(MSGOBJ_IRQ_REQUEST_b,&chip->pend_flags);
			set_bit(MSGOBJ_WORKER_WAKE_b,&chip->pend_flags);
			if(chip->flags & CHIP_IRQ_PCI)
				irq2linux=1;
#ifdef CAN_ENABLE_VME_SUPPORT
			if (chip->flags & CHIP_IRQ_VME)
                                tundra_rtl_ack_irq_vector(irq_num);
#endif
			if(!chip->worker_thread) continue;
			thread=chip->worker_thread;
			pthread_kill(thread,RTL_SIGNAL_WAKEUP);
		}
	}

        /* The following lines are commented out because of it is not
         * possible to share level activated (PCI) IRQs between Linux
         * and RT-Linux. */
/* 	if(irq2linux) */
/* 		rtl_global_pend_irq(irq_num); */

	/*if(thread) rtl_reschedule_thread(thread);*/

	rtl_schedule();

	return 0;
}



/*
RTL_MARK_READY(pthread_self())
RTL_MARK_SUSPENDED(pthread_self());
return rtl_schedule();
can_enable_irq
can_disable_irq 
rtl_critical( state )
rtl_end_critical( state )
rtl_request_global_irq( irq, isr ); 
rtl_free_global_irq( irq )
*/

void * can_chip_worker_thread(void *arg)
{
	struct canchip_t *chip = (struct canchip_t *) arg;
	struct msgobj_t *obj;
	int ret, i;
	int loop_cnt;
	rtl_irqstate_t flags;
	
	if(!chip) return 0;
	
	
	if (!(chip->flags & CHIP_CONFIGURED)){
		if (chip->chipspecops->chip_config(chip))
			CANMSG("Error configuring chip.\n");
		else
			chip->flags |= CHIP_CONFIGURED; 

		if((chip->msgobj[0])!=NULL)
			if (chip->chipspecops->pre_read_config(chip,chip->msgobj[0])<0)
				CANMSG("Error initializing chip for receiving\n");
				
	} /* End of chip configuration */
	set_bit(MSGOBJ_IRQ_REQUEST_b,&chip->pend_flags);
	

	while (1) {
		DEBUGMSG("Worker thread for chip %d active\n",chip->chip_idx);
		if(test_and_clear_bit(MSGOBJ_IRQ_REQUEST_b,&chip->pend_flags)){
			DEBUGMSG("IRQ_REQUEST processing ...\n");
			loop_cnt = 100;
			if(chip->chipspecops->irq_handler) do{
				ret=chip->chipspecops->irq_handler(chip->chip_irq,chip);
			}while(ret && --loop_cnt);
			continue;
		}
		if(test_and_clear_bit(MSGOBJ_TX_REQUEST_b,&chip->pend_flags)){
			DEBUGMSG("TX_REQUEST processing ...\n");
			for(i=0;i<chip->max_objects;i++){
				if((obj=chip->msgobj[i])==NULL)
					continue;
				if(can_msgobj_test_fl(obj,TX_REQUEST)) {
					DEBUGMSG("Calling wakeup_tx\n");
					chip->chipspecops->wakeup_tx(chip, obj);
				}
				if(can_msgobj_test_fl(obj,FILTCH_REQUEST)) {
					DEBUGMSG("Calling filtch_rq\n");
					if(chip->chipspecops->filtch_rq)
						chip->chipspecops->filtch_rq(chip, obj);
				}
			}
			continue;
		}

		/*re-enable chip IRQ, I am not sure, if this is required,
		  but it seems to not work without that */
		if(chip->chip_irq>=0) {
			if ((chip->flags & CHIP_IRQ_VME) == 0) can_enable_irq(chip->chip_irq);
		    #ifdef CAN_ENABLE_VME_SUPPORT
		      #if 0
			else tundra_rtl_enable_pci_irq();
		      #endif
			/* FIXME: Bad practice. Doesn't work with more
			 * than one card.
			 *
			 * irq_accept added to the LinCAN driver now,
			 * and above workaround should not be required.
			 * Enable rtl_hard_enable_irq() at line 
			 * ca91c042.c:1045
			 */
		    #endif /*CAN_ENABLE_VME_SUPPORT*/

		}

                rtl_no_interrupts (flags);
		RTL_MARK_SUSPENDED(pthread_self());
		if(test_and_clear_bit(MSGOBJ_WORKER_WAKE_b,&chip->pend_flags)){
			RTL_MARK_READY(pthread_self());
                        rtl_restore_interrupts (flags);
			continue;
		}
                rtl_restore_interrupts (flags);
		rtl_schedule();

	}
	return 0;
}


int can_chip_setup_irq(struct canchip_t *chip)
{
	int ret;
        struct sched_param sched_param;
        pthread_attr_t attrib;
	pthread_attr_t *attrib_p=NULL;
	
	if(chip==NULL)
		return -1;
	
	if(can_rtl_priority>=0){
		pthread_attr_init(&attrib);
		sched_param.sched_priority = can_rtl_priority;
		pthread_attr_setschedparam(&attrib, &sched_param);
		/* pthread_attr_setschedpolicy(&attrib, SCHED_FIFO); */
		attrib_p=&attrib;
	}
	
	if(chip->chipspecops->irq_handler && !(chip->flags & CHIP_IRQ_CUSTOM)){
		int (*my_request_irq)(unsigned int vector, unsigned int (*rtl_handler)(unsigned int irq, struct pt_regs *regs));
#ifdef CAN_ENABLE_VME_SUPPORT
		if ((chip->flags & CHIP_IRQ_VME) != 0)
			my_request_irq = rtl_request_vmeirq;
		else
#endif
			my_request_irq = rtl_request_irq;

		if (my_request_irq(chip->chip_irq,can_rtl_isr))
			return -1;
		else {
			DEBUGMSG("Registered interrupt %d\n",chip->chip_irq);
			chip->flags |= CHIP_IRQ_SETUP;
		}
	}
        ret=pthread_create(&chip->worker_thread, attrib_p, can_chip_worker_thread, chip);
	if(ret<0) chip->worker_thread=NULL;
	
	return ret;
}


void can_chip_free_irq(struct canchip_t *chip)
{
	if(chip->worker_thread)
		pthread_delete_np(chip->worker_thread);
	if((chip->flags & CHIP_IRQ_SETUP) && (chip->chip_irq>=0)
	    && !(chip->flags & CHIP_IRQ_CUSTOM)) {
		int (*my_free_irq)(unsigned int vector);
#ifdef CAN_ENABLE_VME_SUPPORT
		if ((chip->flags & CHIP_IRQ_VME) != 0)
			my_free_irq = rtl_free_vmeirq;
		else
#endif
			my_free_irq = rtl_free_irq;
		my_free_irq(chip->chip_irq);
		chip->flags &= ~CHIP_IRQ_SETUP;
	}
}


#endif /*CAN_WITH_RTL*/
