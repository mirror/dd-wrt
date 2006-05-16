/*
 * Copyright 2003 PMC-Sierra
 * Author: Manish Lachwani (lachwani@pmc-sierra.com)
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Currently there is support for:
 *  Alliance Sipackets HT-PCI bridge
 *  PLX HT-PCI bridge
 *  Altera Hypertransport device
 */

#include <linux/types.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/pci_ids.h>
#include <asm/pci.h>

/* PLX Specific Defines */
#ifdef CONFIG_PLX_HT_BRIDGE

#define AMD_VENDOR_ID           0x1022
#define AMD_DEVICE_ID           0x7450
#define AMD_IOAPIC_ID           0x7451

/* RDR Registers and Index */
#define AMD_RDR_INDEX           0xb8
#define AMD_RDR_VALUE           0xbc

/* AMD RDR Index for INTX# */
#define AMD_RDR_INTA            0x80100008
#define AMD_RDR_INTB            0x80120008
#define AMD_RDR_INTC            0x80140008
#define AMD_RDR_INTD            0x80160008

#endif

/*
 * HT Bus fixup for the Titan
 */
void __init titan_ht_pcibios_fixup_bus(struct pci_bus *bus)
{
        struct pci_bus *current_bus = bus;
        struct pci_dev *devices;
        struct list_head *devices_link;
	unsigned long cmd;

	list_for_each(devices_link, &(current_bus->devices)) {
                devices = pci_dev_b(devices_link);
                if (devices == NULL)
                        continue;

		/*
		 * Set the device IRQ to 3. This coressponds to 
		 * hardware interrupt line 0 and processor 
		 * interrupt line 0
		 */
		devices->irq = 3;
	}
	/*
         * Turn on IO SPACE, MEM SPACE and BUS MASTER among
         * other things
         */

	if (devices->vendor == PCI_VENDOR_ID_SIPACKETS) {

		bus->ops->read_word(devices, PCI_COMMAND, &cmd);
	        cmd |= 0x17;
        	bus->ops->write_word(devices, PCI_COMMAND, cmd); 

#ifdef CONFIG_SPKT_HT_BRIDGE
		/*
        	 * Configure the Block #0, #1, #2 and #3 
		 * for interrupt routing
        	 */
		printk(KERN_INFO "Fixing SiPKT HT-PCI IOAPIC \n");

#if 0 
		/* Interrupt Routing Diagnostics */
		bus->ops->write_byte(devices, 0xc2, 0x48); 
#endif

		/* 
		 * The following are the config options:
		 * Polarity is active low
		 * Destination mode is logical
		 * Destination processor is core 0
		 * Message type is Fixed
		 * Interrupt Vector configured to PIRQ 0
		 */
		bus->ops->write_dword(devices, 0xa0, 0xc0c1c0c0);
		bus->ops->write_dword(devices, 0xa4, 0xc0c3c0c2);
		bus->ops->write_dword(devices, 0xa8, 0xc0c1c0c0);
		bus->ops->write_dword(devices, 0xac, 0xc0c3c0c2);
		bus->ops->write_dword(devices, 0xb0, 0xc0c1c0c0);
		bus->ops->write_dword(devices, 0xb4, 0xc0c3c0c2);
		bus->ops->write_dword(devices, 0xb8, 0xc0c1c0c0);
		bus->ops->write_dword(devices, 0xbc, 0xc0c3c0c2);

		bus->ops->write_byte(devices, 0xc3, 0x47);
		bus->ops->write_byte(devices, 0xc4, 0x47);
		bus->ops->write_byte(devices, 0xc5, 0x47);
		bus->ops->write_byte(devices, 0xc6, 0x47);

		printk(KERN_ERR "Done configuring interrupts \n");

#ifdef CONFIG_HT_LEVEL_TRIGGER
		/*
		 * Support for Level Triggered mode 
		 */
		bus->ops->write_dword(devices, 0xa0, 0xc0c5c0c4);
		bus->ops->write_dword(devices, 0xa4, 0xc0c7c0c6);
		bus->ops->write_dword(devices, 0xa8, 0xc0c5c0c4);
		bus->ops->write_dword(devices, 0xaC, 0xc0c7c0c6);
#endif
		/* Set the cacheline size to 8 */
		bus->ops->write_byte(devices, 0x0c, 0x08); 

		/* Set the Prefetch related configuration */
		bus->ops->write_dword(devices, 0x60, 0x0f3f0cff); 
		bus->ops->read_dword(devices, 0x60, &cmd);

		printk(KERN_ERR "Done configuring the Read Prefetch \n");
#endif
	
#ifdef CONFIG_PLX_HT_BRIDGE
		/*
	         * For the HT-PCIX bridge, we need to make changes to
        	 * the IO space limits. Note that this is valid only for
	         * the AMD 7450 PLX bridge. Hence we need to compare with
        	 * that
	         */
		if (devices->vendor == AMD_VENDOR_ID &&
			devices->device == AMD_DEVICE_ID) {
			printk(KERN_INFO "AMD HT-PCIX bridge found  ");

			/*
			 * These are the RDR (Redirection Registers) for the 
			 * AMD 8131 IOAPIC
			 */
	
			/* INTA# */
			bus->ops->write_dword(devices, AMD_RDR_INDEX, AMD_RDR_INTA);
			bus->ops->write_dword(devices, AMD_RDR_VALUE, 0xf8200140);

			/* INTB# */
			bus->ops->write_dword(devices, AMD_RDR_INDEX, AMD_RDR_INTB);
			bus->ops->write_dword(devices, AMD_RDR_VALUE, 0xf8200140);

			/* INTC# */
			bus->ops->write_dword(devices, AMD_RDR_INDEX, AMD_RDR_INTC);
			bus->ops->write_dword(devices, AMD_RDR_VALUE, 0xf8200140);

			/* INTD# */
			bus->ops->write_dword(devices, AMD_RDR_INDEX, AMD_RDR_INTD);
			bus->ops->write_dword(devices, AMD_RDR_VALUE, 0xf8200140);

			/* Enable the Prefetching, MRM and MRL */
			bus->ops->read_dword(devices, 0x4c, &val);
			val |= 0x00001dc0;
			bus->ops->write_dword(devices, 0x4c, val);
		}

		if (devices->vendor == AMD_VENDOR_ID &&
			devices->device == AMD_IOAPIC_ID) {
			printk("AMD 8131 IO APIC Fixup \n");

			/* Turn the IOAPIC ON */
			bus->ops->write_dword(devices, 0x44, 0x00000003);
		}
#endif /* CONFIG_PLX_HT_BRIDGE */
#ifdef CONFIG_ALTERA_HT_BRIDGE
		if ( (devices->vendor == PCI_VENDOR_ID_ALTERA) &&
			(devices->device == PCI_DEVICE_ID_ALTERA_HT_BRIDGE)) {
			printk(KERN_INFO "Found an Altera HT device, fixing up \n");
		}

		/* Check if the Link Initialization completed successfully */
		bus->ops->read_dword(devices, 0x44, &val);
		if (!(val & 0x20))
			printk(KERN_ERR "Link Initialization Error !! \n");
#endif /* CONFIG_ALTERA_HT_BRIDGE */

		/* enable master */
		if (((current_bus->number != 0) && (current_bus->number != 1))
				|| (PCI_SLOT(devices->devfn) != 0)) {
			bus->ops->read_word(devices, PCI_COMMAND, &cmd);
			cmd |= PCI_COMMAND_MASTER;
			bus->ops->write_word(devices, PCI_COMMAND, cmd);
		}

	}
}

