/* unican_vme.c
 * Linux CAN-bus device driver.
 * Written for new CAN driver version by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */ 

/* This file is included in unican.c if CAN_ENABLE_VME_SUPPORT is
 * set. */


/**
 * unican_vme_init_hw_data - Initialize hardware cards
 * @candev: Pointer to candevice/board structure
 *
 * Return Value: The function always returns zero
 * File: src/unican.c
 */
int unican_vme_reset(struct candevice_t *candev)
{
	int ret;
	struct canchip_t *chip = candev->chip[0];

	ret = unican_reset(candev);

	/* Setup VME interrupt vector */
	if (ret == 0) 
		unican_writew(chip->chip_irq, chip->chip_base_addr+CL2_VME_INT_VECTOR);

	return ret;
}

/**
 * unican_vme_init_chip_data - Initialize chips
 * @candev: Pointer to candevice/board structure
 * @chipnr: Number of the CAN chip on the hardware card
 *
 * Return Value: The function always returns zero
 * File: src/unican.c
 */
int unican_vme_init_chip_data(struct candevice_t *candev, int chipnr)
{
	struct canchip_t *chip = candev->chip[chipnr];

	unican_init_chip_data(candev, chipnr);

	chip->flags |= CHIP_IRQ_VME;
	/*chip->chipspecops->irq_handler=unican_irq_handler;*/
	return 0;
}


int unican_vme_init_hw_data(struct candevice_t *candev) 
{
	unican_init_hw_data(candev);
	candev->flags |= CANDEV_PROGRAMMABLE_IRQ;

	return 0;
}

int unican_vme_register(struct hwspecops_t *hwspecops)
{
	unican_register(hwspecops);

	hwspecops->init_hw_data = unican_vme_init_hw_data;
	hwspecops->init_chip_data = unican_vme_init_chip_data;
	hwspecops->request_io = unican_request_io;
	hwspecops->reset = unican_vme_reset;
	hwspecops->release_io = unican_release_io;
	return 0;
}
