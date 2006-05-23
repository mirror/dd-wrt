/*
 * BRIEF MODULE DESCRIPTION
 *	Au1xxx irq map table
 *
 * Copyright 2003 Embedded Edge, LLC
 *		dan@embeddededge.com
 *
 *  This program is free software; you can redistribute	 it and/or modify it
 *  under  the terms of	 the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the	License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED	  ``AS	IS'' AND   ANY	EXPRESS OR IMPLIED
 *  WARRANTIES,	  INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO	EVENT  SHALL   THE AUTHOR  BE	 LIABLE FOR ANY	  DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED	  TO, PROCUREMENT OF  SUBSTITUTE GOODS	OR SERVICES; LOSS OF
 *  USE, DATA,	OR PROFITS; OR	BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN	 CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/kernel_stat.h>
#include <linux/module.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/timex.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/delay.h>

#include <asm/bitops.h>
#include <asm/bootinfo.h>
#include <asm/io.h>
#include <asm/mipsregs.h>
#include <asm/system.h>
#include <asm/au1000.h>

au1xxx_irq_map_t au1xxx_irq_map[] = {

#ifndef CONFIG_MIPS_MIRAGE
#ifdef CONFIG_MIPS_DB1550
	{ AU1000_GPIO_3, INTC_INT_LOW_LEVEL, 0 }, // PCMCIA Card 0 IRQ#
	{ AU1000_GPIO_5, INTC_INT_LOW_LEVEL, 0 }, // PCMCIA Card 1 IRQ#
	{ AU1000_GPIO_8, INTC_INT_LOW_LEVEL, 0 }, // Daughtercard IRQ#
#else
	{ AU1000_GPIO_0, INTC_INT_LOW_LEVEL, 0 }, // PCMCIA Card 0 Fully_Interted#
	{ AU1000_GPIO_1, INTC_INT_LOW_LEVEL, 0 }, // PCMCIA Card 0 STSCHG#
	{ AU1000_GPIO_2, INTC_INT_LOW_LEVEL, 0 }, // PCMCIA Card 0 IRQ#

	{ AU1000_GPIO_3, INTC_INT_LOW_LEVEL, 0 }, // PCMCIA Card 1 Fully_Interted#
	{ AU1000_GPIO_4, INTC_INT_LOW_LEVEL, 0 }, // PCMCIA Card 1 STSCHG#
	{ AU1000_GPIO_5, INTC_INT_LOW_LEVEL, 0 }, // PCMCIA Card 1 IRQ#
#endif
#else
	{ AU1000_GPIO_7, INTC_INT_RISE_EDGE, 0 }, /* touchscreen pendown */
#endif
};

int au1xxx_nr_irqs = sizeof(au1xxx_irq_map)/sizeof(au1xxx_irq_map_t);

#ifdef CONFIG_PCI

#ifdef CONFIG_SOC_AU1500
#define INTA AU1000_PCI_INTA
#define INTB AU1000_PCI_INTB
#define INTC AU1000_PCI_INTC
#define INTD AU1000_PCI_INTD
#endif

#ifdef CONFIG_SOC_AU1550
#define INTA AU1550_PCI_INTA
#define INTB AU1550_PCI_INTB
#define INTC AU1550_PCI_INTC
#define INTD AU1550_PCI_INTD
#endif

#define INTX 0xFF /* not valid */

int __init
au1xxx_pci_irqmap(struct pci_dev *dev, unsigned char idsel, unsigned char pin)
{
	/*
	 *	PCI IDSEL/INTPIN->INTLINE
	 *	A       B       C       D
	 */
#ifdef CONFIG_MIPS_DB1500
	static char pci_irq_table[][4] =
	{
		{INTA, INTX, INTX, INTX},   /* IDSEL 12 - HPT371   */
		{INTA, INTB, INTC, INTD},   /* IDSEL 13 - PCI slot */
	};
	const long min_idsel = 12, max_idsel = 13, irqs_per_slot = 4;
#endif

#ifdef CONFIG_MIPS_BOSPORUS
	static char pci_irq_table[][4] =
	{
		{INTA, INTB, INTX, INTX},   /* IDSEL 11 - miniPCI  */
		{INTA, INTX, INTX, INTX},   /* IDSEL 12 - SN1741   */
		{INTA, INTB, INTC, INTD},   /* IDSEL 13 - PCI slot */
	};
	const long min_idsel = 11, max_idsel = 13, irqs_per_slot = 4;
#endif

#ifdef CONFIG_MIPS_MIRAGE
	static char pci_irq_table[][4] =
	{
		{INTD, INTX, INTX, INTX},   /* IDSEL 11 - SMI VGX */
		{INTX, INTX, INTC, INTX},   /* IDSEL 12 - PNX1300 */
		{INTA, INTB, INTX, INTX},   /* IDSEL 13 - miniPCI */
	};
	const long min_idsel = 11, max_idsel = 13, irqs_per_slot = 4;
#endif

#ifdef CONFIG_MIPS_DB1550
	static char pci_irq_table[][4] =
	{
		{INTC, INTX, INTX, INTX},   /* IDSEL 11 - on-board HPT371    */
		{INTB, INTC, INTD, INTA},   /* IDSEL 12 - PCI slot 2 (left)  */
		{INTA, INTB, INTC, INTD},   /* IDSEL 13 - PCI slot 1 (right) */
	};
	const long min_idsel = 11, max_idsel = 13, irqs_per_slot = 4;
#endif
#if defined(CONFIG_SOC_AU1550) || defined(CONFIG_SOC_AU1500)
	return PCI_IRQ_TABLE_LOOKUP;
#else
	return 0;
#endif
};
#endif

