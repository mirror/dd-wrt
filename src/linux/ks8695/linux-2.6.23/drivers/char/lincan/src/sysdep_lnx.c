/* sysdep_lnx.c
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

#ifdef CAN_ENABLE_VME_SUPPORT
#include "ca91c042.h"
/* Modified version of ca91c042 driver can be found in
 * components/comm/contrib directory. */
#endif

#ifndef IRQF_SHARED
#define IRQF_SHARED SA_SHIRQ
#endif  /*IRQF_SHARED*/

/**
 * can_checked_malloc - memory allocation with registering of requested blocks
 * @size: size of the requested block
 *
 * The function is used in the driver initialization phase to catch possible memory
 * leaks for future driver finalization or case, that driver initialization fail.
 * 
 * Return Value: pointer to the allocated memory or NULL in the case of fail
 */
void *can_checked_malloc(size_t size)
{
	struct mem_addr *mem_new;
	void *address_p;
	
	address_p=kmalloc(size,GFP_KERNEL);
	if(address_p == NULL) {
		CANMSG("can_checked_malloc: out of the memory\n");
		return NULL;
	}

#ifdef DEBUG_MEM
	DEBUGMSG("can_checked_malloc: allocated %d bytes at %p, mem_head=%p\n",
			(int)size, address_p, mem_head);
#endif

	mem_new=(struct mem_addr *)kmalloc(sizeof(struct mem_addr),GFP_KERNEL);
	if (mem_new == NULL) {
		CANMSG("can_checked_malloc: memory list allocation error.\n");
		kfree(address_p);
		return NULL;
	}
	mem_new->next=mem_head;
	mem_new->address=address_p;
	mem_new->size=size;
	mem_head=mem_new;

	return address_p;
}

/**
 * can_checked_free - free memory allocated by  can_checked_malloc()
 * @address_p: pointer to the memory block
 */
int can_checked_free(void *address_p)
{
	struct mem_addr **mem_pptr;
	struct mem_addr *mem_del=NULL;

#ifdef DEBUG_MEM
	DEBUGMSG("can_checked_free %p, mem_head=%p\n", address_p, mem_head);
#endif

	for(mem_pptr = &mem_head; (mem_del = *mem_pptr); mem_pptr = &mem_del->next) {
		if (mem_del->address != address_p)
			continue;
		*mem_pptr=mem_del->next;
		kfree(mem_del);
		kfree(address_p);
		return 0;
	}
	
	CANMSG("can_checked_free: address %p not found on the mem list\n", address_p);
	
	kfree(address_p);
	return -1;
}


/**
 * can_del_mem_list - check for stale memory allocations at driver finalization
 *
 * Checks, if there are still some memory blocks allocated and releases memory
 * occupied by such blocks back to the system
 */
int can_del_mem_list(void)
{
	struct mem_addr *mem;

#ifdef DEBUG_MEM
	DEBUGMSG("can_del_mem_list, mem_head=%p\n", mem_head);
#endif
	if(mem_head == NULL) {
		CANMSG("can_del_mem_list: no entries on the list - OK\n");
		return 0;
	}

	while((mem=mem_head) != NULL) {
		mem_head=mem->next;
		CANMSG("can_del_mem_list: deleting %p with size %d\n",
			mem->address, (int)mem->size);
		kfree(mem->address);
		kfree(mem);
	}
	
	return 0;
}

/**
 * can_request_io_region - request IO space region
 * @start: the first IO port address
 * @n: number of the consecutive IO port addresses
 * @name: name/label for the requested region
 *
 * The function hides system specific implementation of the feature.
 *
 * Return Value: returns positive value (1) in the case, that region could
 *	be reserved for the driver. Returns zero (0) if there is collision with
 *	other driver or region cannot be taken for some other reason.
 */
int can_request_io_region(unsigned long start, unsigned long n, const char *name)
{
    #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0))
	if(check_region(start,n)) return 0;
	request_region(start,n,name);
	return 1;
    #else
	return (request_region(start,n,name))?1:0;
    #endif
}

/**
 * can_release_io_region - release IO space region
 * @start: the first IO port address
 * @n: number of the consecutive IO port addresses
 */
void can_release_io_region(unsigned long start, unsigned long n)
{
	release_region(start,n);
}

/**
 * can_request_mem_region - request memory space region
 * @start: the first memory port physical address
 * @n: number of the consecutive memory port addresses
 * @name: name/label for the requested region
 *
 * The function hides system specific implementation of the feature.
 *
 * Return Value: returns positive value (1) in the case, that region could
 *	be reserved for the driver. Returns zero (0) if there is collision with
 *	other driver or region cannot be taken for some other reason.
 */
int can_request_mem_region(unsigned long start, unsigned long n, const char *name)
{
    #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0))
	return 1;
    #else
	return (request_mem_region(start,n,name))?1:0;
    #endif
}

/**
 * can_release_mem_region - release memory space region
 * @start: the first memory port physical address
 * @n: number of the consecutive memory port addresses
 */
void can_release_mem_region(unsigned long start, unsigned long n)
{
    #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0))
	return;
    #else
	release_mem_region(start,n);
    #endif
}

#ifndef CAN_WITH_RTL

/**
 * can_default_irq_dispatch - the first level interrupt dispatch handler
 * @irq: interrupt vector number, this value is system specific
 * @dev_id: driver private pointer registered at time of request_irq() call.
 *	The CAN driver uses this pointer to store relationship of interrupt
 *	to chip state structure - @struct canchip_t
 * @regs: system dependent value pointing to registers stored in exception frame
 * 
 * File: src/setup.c
 */
can_irqreturn_t can_default_irq_dispatch(CAN_IRQ_HANDLER_ARGS(irq_number, dev_id))
{
	int retval;
	struct canchip_t *chip=(struct canchip_t *)dev_id;

	retval=chip->chipspecops->irq_handler(irq_number, chip);
	return CAN_IRQ_RETVAL(retval);
}

/**
 * can_chip_setup_irq - attaches chip to the system interrupt processing
 * @chip: pointer to CAN chip structure
 *
 * Return Value: returns negative number in the case of fail
 */
int can_chip_setup_irq(struct canchip_t *chip)
{
	if(chip==NULL)
		return -1;
	if(!chip->chipspecops->irq_handler)
		return 0;
	if(chip->flags & CHIP_IRQ_CUSTOM)
		return 1;
			
	if ((chip->flags & CHIP_IRQ_VME) == 0) {
		if (request_irq(chip->chip_irq,can_default_irq_dispatch,IRQF_SHARED,DEVICE_NAME,chip))
			return -1;
		else {
			DEBUGMSG("Registered interrupt %d\n",chip->chip_irq);
			chip->flags |= CHIP_IRQ_SETUP;
		}
	} else {
#ifdef CAN_ENABLE_VME_SUPPORT
		if (chip->chip_irq < 1 || chip->chip_irq > 255) {
			CANMSG("Bad irq parameter. (1 <= irq <= 255).\n");
			return -EINVAL;
		}
		
		request_vmeirq(chip->chip_irq, can_default_irq_dispatch, chip);
		DEBUGMSG("Registered VME interrupt vector %d\n",chip->chip_irq);
		chip->flags |= CHIP_IRQ_SETUP;
#endif
	}
	return 1;
}


/**
 * can_chip_free_irq - unregisters chip interrupt handler from the system
 * @chip: pointer to CAN chip structure
 */
void can_chip_free_irq(struct canchip_t *chip)
{
	if((chip->flags & CHIP_IRQ_SETUP) && (chip->chip_irq>=0)) {
		if(chip->flags & CHIP_IRQ_CUSTOM)
			return;

		if ((chip->flags & CHIP_IRQ_VME) == 0)
			free_irq(chip->chip_irq, chip);
		else { 
#ifdef CAN_ENABLE_VME_SUPPORT
			free_vmeirq(chip->chip_irq);
#endif
		}
			chip->flags &= ~CHIP_IRQ_SETUP;
	}
}

#endif /*CAN_WITH_RTL*/
