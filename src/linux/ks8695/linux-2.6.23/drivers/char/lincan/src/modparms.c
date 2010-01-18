/* mod_parms.c
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
#include "../include/modparms.h"

int parse_mod_parms(void)
{
	int i=0,j=0,irq_needed=0,irq_supplied=0,io_needed=0,io_supplied=0,minor_needed=0,minor_supplied=0;
	const struct boardtype_t *brp;

	if ( (hw[0] == NULL) | (io[0] == -1) ) {
		CANMSG("You must supply your type of hardware, interrupt numbers and io address.\n");
		CANMSG("Example: # insmod lincan.ko hw=pip5 irq=4 io=0x8000\n");
		return -ENODEV;
	}

	while ( (hw[i] != NULL) && (i < MAX_HW_CARDS) ) {
		brp = boardtype_find(hw[i]);
		if(!brp) {
			CANMSG("Sorry, hardware \"%s\" is currently not supported.\n",hw[i]);
			return -EINVAL;
		}
		irq_needed += brp->irqnum;
		i++;
	}

	/* Check wether the supplied number of io addresses is correct. */
	io_needed=i;
	while ( (io[io_supplied] != -1) & (io_supplied<MAX_HW_CARDS) ) 
		io_supplied++;
	if (io_needed != io_supplied) {
		CANMSG("Invalid number of io addresses.\n");
		CANMSG("Supplied hardware needs %d io address(es).\n",io_needed);
		return -EINVAL;
	}

	/* Check wether the supplied number of irq's is correct. */
	while ( (irq[irq_supplied] != -1) & (irq_supplied<MAX_IRQ) )
		irq_supplied++;
	while ( (hw[j] != NULL) && (j<MAX_HW_CARDS) ) {
		if (!strcmp(hw[j],"template"))
			irq_needed = irq_supplied;
		j++;
	}
	if (irq_needed != irq_supplied) {
		CANMSG("Invalid number of interrupts.\n");
		CANMSG("Supplied harware needs %d irq number(s).\n",irq_needed);
		return -EINVAL;
	}

	/* In case minor numbers were assigned check wether the correct number
	 * of minor numbers was supplied.
	 */
	if (minor[0] != -1) {
		minor_needed=irq_needed;
		while ((minor[minor_supplied] != -1) & (minor_supplied<MAX_IRQ))
			minor_supplied++; 
		if (minor_supplied != minor_needed) {
			CANMSG("Invalid number of minor numbers.\n");
			CANMSG("Supplied hardware needs %d minor number(s).\n",minor_needed);
			return -EINVAL;
		}
	}

	return 0;
}
/* list_hw is used when debugging is on to show the hardware layout */
int list_hw(void)
{
	int i=0,j=0,k=0;

	DEBUGMSG("Number of boards : %d\n",hardware_p->nr_boards);

	while ( (hw[i] != NULL) & (i<=MAX_HW_CARDS-1) ) {
		printk(KERN_ERR "\n");
		DEBUGMSG("Hardware         : %s\n",hardware_p->candevice[i]->hwname);
		DEBUGMSG("IO address       : 0x%lx\n",hardware_p->candevice[i]->io_addr);
		DEBUGMSG("Nr. all chips    : %d\n",hardware_p->candevice[i]->nr_all_chips);
		DEBUGMSG("Nr. of i82527    : %d\n",hardware_p->candevice[i]->nr_82527_chips);
		DEBUGMSG("Nr. of sja1000   : %d\n",hardware_p->candevice[i]->nr_sja1000_chips);
		for (j=0; j<hardware_p->candevice[i]->nr_all_chips; j++) {
			DEBUGMSG("Chip%d type       : %s\n", j+1, hardware_p->candevice[i]->chip[j]->chip_type);
			DEBUGMSG("Chip base        : 0x%lx\n",hardware_p->candevice[i]->chip[j]->chip_base_addr);
			DEBUGMSG("Interrupt        : %d\n",hardware_p->candevice[i]->chip[j]->chip_irq);


			for (k=0; k<hardware_p->candevice[i]->chip[j]->max_objects; k++)
				DEBUGMSG("Obj%d: minor: %d base: 0x%lx\n",k,hardware_p->candevice[i]->chip[j]->msgobj[k]->minor,hardware_p->candevice[i]->chip[j]->msgobj[k]->obj_base_addr);

		}
		i++;
	}
	return 0;
}
