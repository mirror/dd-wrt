/*
 * arch/mips/vr41xx/common/vrc4173.c
 *
 * NEC VRC4173 driver for NEC VR4122/VR4131.
 *
 * Author: Yoichi Yuasa <yyuasa@mvista.com, or source@mvista.com>
 *
 * 2001-2003 (c) MontaVista, Software, Inc. This file is licensed under
 * the terms of the GNU General Public License version 2. This program
 * is licensed "as is" without any warranty of any kind, whether express
 * or implied.
 */
#include <linux/config.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/pci.h>
#include <linux/spinlock.h>
#include <linux/types.h>

#include <asm/vr41xx/vr41xx.h>
#include <asm/vr41xx/vrc4173.h>

MODULE_DESCRIPTION("NEC VRC4173 driver for NEC VR4122/4131");
MODULE_AUTHOR("Yoichi Yuasa <yyuasa@mvista.com>");
MODULE_LICENSE("GPL");

EXPORT_SYMBOL(vrc4173_io_offset);
EXPORT_SYMBOL(vrc4173_supply_clock);
EXPORT_SYMBOL(vrc4173_mask_clock);
EXPORT_SYMBOL(vrc4173_select_function);

#define VRC4173_CMUCLKMSK	0x040
 #define MSKPIU			0x0001
 #define MSKKIU			0x0002
 #define MSKAIU			0x0004
 #define MSKPS2CH1		0x0008
 #define MSKPS2CH2		0x0010
 #define MSKUSB			0x0020
 #define MSKCARD1		0x0040
 #define MSKCARD2		0x0080
 #define MSKAC97		0x0100
 #define MSK48MUSB		0x0400
 #define MSK48MPIN		0x0800
 #define MSK48MOSC		0x1000
#define VRC4173_CMUSRST		0x042
 #define USBRST			0x0001
 #define CARD1RST		0x0002
 #define CARD2RST		0x0004
 #define AC97RST		0x0008

#define VRC4173_SYSINT1REG	0x060
#define VRC4173_MSYSINT1REG	0x06c

#define VRC4173_SELECTREG	0x09e

static struct pci_device_id vrc4173_table[] __devinitdata = {
	{	.vendor		= PCI_VENDOR_ID_NEC,
		.device		= PCI_DEVICE_ID_NEC_VRC4173,
		.subvendor	= PCI_ANY_ID,
		.subdevice	= PCI_ANY_ID,			},
	{	.vendor		= 0,				},
};

unsigned long vrc4173_io_offset = 0;

static int vrc4173_initialized;
static uint16_t vrc4173_cmuclkmsk;
static uint16_t vrc4173_selectreg;
static spinlock_t vrc4173_cmu_lock;
static spinlock_t vrc4173_giu_lock;

static inline void set_cmusrst(uint16_t val)
{
	uint16_t cmusrst;

	cmusrst = vrc4173_inw(VRC4173_CMUSRST);
	cmusrst |= val;
	vrc4173_outw(cmusrst, VRC4173_CMUSRST);
}

static inline void clear_cmusrst(uint16_t val)
{
	uint16_t cmusrst;

	cmusrst = vrc4173_inw(VRC4173_CMUSRST);
	cmusrst &= ~val;
	vrc4173_outw(cmusrst, VRC4173_CMUSRST);
}

void vrc4173_supply_clock(unsigned int clock)
{
	if (vrc4173_initialized) {
		spin_lock_irq(&vrc4173_cmu_lock);

		switch (clock) {
		case VRC4173_PIU_CLOCK:
			vrc4173_cmuclkmsk |= MSKPIU;
			break;
		case VRC4173_KIU_CLOCK:
			vrc4173_cmuclkmsk |= MSKKIU;
			break;
		case VRC4173_AIU_CLOCK:
			vrc4173_cmuclkmsk |= MSKAIU;
			break;
		case VRC4173_PS2_CH1_CLOCK:
			vrc4173_cmuclkmsk |= MSKPS2CH1;
			break;
		case VRC4173_PS2_CH2_CLOCK:
			vrc4173_cmuclkmsk |= MSKPS2CH2;
			break;
		case VRC4173_USBU_PCI_CLOCK:
			set_cmusrst(USBRST);
			vrc4173_cmuclkmsk |= MSKUSB;
			break;
		case VRC4173_CARDU1_PCI_CLOCK:
			set_cmusrst(CARD1RST);
			vrc4173_cmuclkmsk |= MSKCARD1;
			break;
		case VRC4173_CARDU2_PCI_CLOCK:
			set_cmusrst(CARD2RST);
			vrc4173_cmuclkmsk |= MSKCARD2;
			break;
		case VRC4173_AC97U_PCI_CLOCK:
			set_cmusrst(AC97RST);
			vrc4173_cmuclkmsk |= MSKAC97;
			break;
		case VRC4173_USBU_48MHz_CLOCK:
			set_cmusrst(USBRST);
			vrc4173_cmuclkmsk |= MSK48MUSB;
			break;
		case VRC4173_EXT_48MHz_CLOCK:
			if (vrc4173_cmuclkmsk & MSK48MOSC)
				vrc4173_cmuclkmsk |= MSK48MPIN;
			else
				printk(KERN_WARNING
				       "vrc4173_supply_clock: "
				       "Please supply VRC4173_48MHz_CLOCK first "
				       "rather than VRC4173_EXT_48MHz_CLOCK.\n");
			break;
		case VRC4173_48MHz_CLOCK:
			vrc4173_cmuclkmsk |= MSK48MOSC;
			break;
		default:
			printk(KERN_WARNING
			       "vrc4173_supply_clock: Invalid CLOCK value %u\n", clock);
			break;
		}

		vrc4173_outw(vrc4173_cmuclkmsk, VRC4173_CMUCLKMSK);

		switch (clock) {
		case VRC4173_USBU_PCI_CLOCK:
		case VRC4173_USBU_48MHz_CLOCK:
			clear_cmusrst(USBRST);
			break;
		case VRC4173_CARDU1_PCI_CLOCK:
			clear_cmusrst(CARD1RST);
			break;
		case VRC4173_CARDU2_PCI_CLOCK:
			clear_cmusrst(CARD2RST);
			break;
		case VRC4173_AC97U_PCI_CLOCK:
			clear_cmusrst(AC97RST);
			break;
		default:
			break;
		}

		spin_unlock_irq(&vrc4173_cmu_lock);
	}
}

void vrc4173_mask_clock(unsigned int clock)
{
	if (vrc4173_initialized) {
		spin_lock_irq(&vrc4173_cmu_lock);

		switch (clock) {
		case VRC4173_PIU_CLOCK:
			vrc4173_cmuclkmsk &= ~MSKPIU;
			break;
		case VRC4173_KIU_CLOCK:
			vrc4173_cmuclkmsk &= ~MSKKIU;
			break;
		case VRC4173_AIU_CLOCK:
			vrc4173_cmuclkmsk &= ~MSKAIU;
			break;
		case VRC4173_PS2_CH1_CLOCK:
			vrc4173_cmuclkmsk &= ~MSKPS2CH1;
			break;
		case VRC4173_PS2_CH2_CLOCK:
			vrc4173_cmuclkmsk &= ~MSKPS2CH2;
			break;
		case VRC4173_USBU_PCI_CLOCK:
			set_cmusrst(USBRST);
			vrc4173_cmuclkmsk &= ~MSKUSB;
			break;
		case VRC4173_CARDU1_PCI_CLOCK:
			set_cmusrst(CARD1RST);
			vrc4173_cmuclkmsk &= ~MSKCARD1;
			break;
		case VRC4173_CARDU2_PCI_CLOCK:
			set_cmusrst(CARD2RST);
			vrc4173_cmuclkmsk &= ~MSKCARD2;
			break;
		case VRC4173_AC97U_PCI_CLOCK:
			set_cmusrst(AC97RST);
			vrc4173_cmuclkmsk &= ~MSKAC97;
			break;
		case VRC4173_USBU_48MHz_CLOCK:
			set_cmusrst(USBRST);
			vrc4173_cmuclkmsk &= ~MSK48MUSB;
			break;
		case VRC4173_EXT_48MHz_CLOCK:
			vrc4173_cmuclkmsk &= ~MSK48MPIN;
			break;
		case VRC4173_48MHz_CLOCK:
			vrc4173_cmuclkmsk &= ~MSK48MOSC;
			break;
		default:
			printk(KERN_WARNING "vrc4173_mask_clock: Invalid CLOCK value %u\n", clock);
			break;
		}

		vrc4173_outw(vrc4173_cmuclkmsk, VRC4173_CMUCLKMSK);

		switch (clock) {
		case VRC4173_USBU_PCI_CLOCK:
		case VRC4173_USBU_48MHz_CLOCK:
			clear_cmusrst(USBRST);
			break;
		case VRC4173_CARDU1_PCI_CLOCK:
			clear_cmusrst(CARD1RST);
			break;
		case VRC4173_CARDU2_PCI_CLOCK:
			clear_cmusrst(CARD2RST);
			break;
		case VRC4173_AC97U_PCI_CLOCK:
			clear_cmusrst(AC97RST);
			break;
		default:
			break;
		}

		spin_unlock_irq(&vrc4173_cmu_lock);
	}
}

static inline void vrc4173_cmu_init(void)
{
	vrc4173_cmuclkmsk = vrc4173_inw(VRC4173_CMUCLKMSK);

	spin_lock_init(&vrc4173_cmu_lock);
}

void vrc4173_select_function(int func)
{
	if (vrc4173_initialized) {
		spin_lock_irq(&vrc4173_giu_lock);

		switch(func) {
		case PS2CH1_SELECT:
			vrc4173_selectreg |= 0x0004;
			break;
		case PS2CH2_SELECT:
			vrc4173_selectreg |= 0x0002;
			break;
		case TOUCHPANEL_SELECT:
			vrc4173_selectreg &= 0x0007;
			break;
		case KIU8_SELECT:
			vrc4173_selectreg &= 0x000e;
			break;
		case KIU10_SELECT:
			vrc4173_selectreg &= 0x000c;
			break;
		case KIU12_SELECT:
			vrc4173_selectreg &= 0x0008;
			break;
		case GPIO_SELECT:
			vrc4173_selectreg |= 0x0008;
			break;
		}

		vrc4173_outw(vrc4173_selectreg, VRC4173_SELECTREG);

		spin_unlock_irq(&vrc4173_giu_lock);
	}
}

static inline void vrc4173_giu_init(void)
{
	vrc4173_selectreg = vrc4173_inw(VRC4173_SELECTREG);

	spin_lock_init(&vrc4173_giu_lock);
}

static void enable_vrc4173_irq(unsigned int irq)
{
	uint16_t val;

	val = vrc4173_inw(VRC4173_MSYSINT1REG);
	val |= (uint16_t)1 << (irq - VRC4173_IRQ_BASE);
	vrc4173_outw(val, VRC4173_MSYSINT1REG);
}

static void disable_vrc4173_irq(unsigned int irq)
{
	uint16_t val;

	val = vrc4173_inw(VRC4173_MSYSINT1REG);
	val &= ~((uint16_t)1 << (irq - VRC4173_IRQ_BASE));
	vrc4173_outw(val, VRC4173_MSYSINT1REG);
}

static unsigned int startup_vrc4173_irq(unsigned int irq)
{
	enable_vrc4173_irq(irq);
	return 0; /* never anything pending */
}

#define shutdown_vrc4173_irq	disable_vrc4173_irq
#define ack_vrc4173_irq		disable_vrc4173_irq

static void end_vrc4173_irq(unsigned int irq)
{
	if (!(irq_desc[irq].status & (IRQ_DISABLED | IRQ_INPROGRESS)))
		enable_vrc4173_irq(irq);
}

static struct hw_interrupt_type vrc4173_irq_type = {
	.typename	= "VRC4173",
	.startup	= startup_vrc4173_irq,
	.shutdown	= shutdown_vrc4173_irq,
	.enable		= enable_vrc4173_irq,
	.disable	= disable_vrc4173_irq,
	.ack		= ack_vrc4173_irq,
	.end		= end_vrc4173_irq,
};

static int vrc4173_get_irq_number(int irq)
{
	uint16_t status, mask;
	int i;

        status = vrc4173_inw(VRC4173_SYSINT1REG);
        mask = vrc4173_inw(VRC4173_MSYSINT1REG);

	status &= mask;
	if (status) {
		for (i = 0; i < 16; i++)
			if (status & (0x0001 << i))
				return VRC4173_IRQ(i);
	}

	return -EINVAL;
}

static inline int vrc4173_icu_init(int cascade_irq)
{
	int i;

	if (cascade_irq < GIU_IRQ(0) || cascade_irq > GIU_IRQ(15))
		return -EINVAL;
	
	vrc4173_outw(0, VRC4173_MSYSINT1REG);

	vr41xx_set_irq_trigger(GIU_IRQ_TO_PIN(cascade_irq), TRIGGER_LEVEL, SIGNAL_THROUGH);
	vr41xx_set_irq_level(GIU_IRQ_TO_PIN(cascade_irq), LEVEL_LOW);

	for (i = VRC4173_IRQ_BASE; i <= VRC4173_IRQ_LAST; i++)
                irq_desc[i].handler = &vrc4173_irq_type;

	return 0;
}

static int __devinit vrc4173_probe(struct pci_dev *dev,
                                   const struct pci_device_id *id)
{
	unsigned long start, flags;
	int err;

	err = pci_enable_device(dev);
	if (err < 0) {
		printk(KERN_ERR "vrc4173: Failed to enable PCI device, aborting\n");
		return err;
	}

	pci_set_master(dev);

	start = pci_resource_start(dev, 0);
	if (start == 0) {
		printk(KERN_ERR "vrc4173:No such PCI I/O resource, aborting\n");
		return -ENXIO;
	}

	flags = pci_resource_flags(dev, 0);
	if ((flags & IORESOURCE_IO) == 0) {
		printk(KERN_ERR "vrc4173: No such PCI I/O resource, aborting\n");
		return -ENXIO;
	}

	err = pci_request_regions(dev, "NEC VRC4173");
	if (err < 0) {
		printk(KERN_ERR "vrc4173: PCI resources are busy, aborting\n");
		return err;
	}

	set_vrc4173_io_offset(start);

	vrc4173_cmu_init();
	vrc4173_giu_init();

	err = vrc4173_icu_init(dev->irq);
	if (err < 0) {
		printk(KERN_ERR "vrc4173: Invalid IRQ %d, aborting\n", dev->irq);
		return err;
	}

	err = vr41xx_cascade_irq(dev->irq, vrc4173_get_irq_number);
	if (err < 0) {
		printk(KERN_ERR "vrc4173: IRQ resource %d is busy, aborting\n", dev->irq);
		return err;
	}

	printk(KERN_INFO
	       "NEC VRC4173 at 0x%#08lx, IRQ is cascaded to %d\n", start, dev->irq);

	return 0;
}

static void vrc4173_remove(struct pci_dev *dev)
{
	free_irq(dev->irq, NULL);

	pci_release_regions(dev);
}

static struct pci_driver vrc4173_driver = {
	.name		= "NEC VRC4173",
	.probe		= vrc4173_probe,
	.remove		= vrc4173_remove,
	.id_table	= vrc4173_table,
};

static int __devinit vrc4173_init(void)
{
	int err;

	err = pci_module_init(&vrc4173_driver);
	if (err < 0)
		return err;

	vrc4173_initialized = 1;

	return 0;
}

static void __devexit vrc4173_exit(void)
{
	vrc4173_initialized = 0;

	pci_unregister_driver(&vrc4173_driver);
}

module_init(vrc4173_init);
module_exit(vrc4173_exit);
