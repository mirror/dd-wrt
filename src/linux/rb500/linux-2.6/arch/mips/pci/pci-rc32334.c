/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     PCI initialization for IDT S334 board
 *
 *  Copyright 2004 IDT Inc. (rischelp@idt.com)
 *         
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
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
 *
 *
 **************************************************************************
 * May 2004 P. Sadik
 *
 * Initial Release
 *
 * 
 *
 **************************************************************************
 */

#include <linux/config.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/types.h>
#include <linux/delay.h>

#include <asm/cpu.h>
#include <asm/io.h>

#include <asm/idt-boards/rc32300/rc32300.h>
#include <asm/idt-boards/rc32300/rc32334.h>

#define PCI_DEVICE_ID_IDT_RC32334       0x0204
#define PCI_DEVICE_ID_IDT_79S334        0x0134
#undef DEBUG
#ifdef DEBUG
#define DBG(x...) printk(x)
#else
#define DBG(x...)
#endif

#ifdef __MIPSEB__
#define SWAP_BIT 1
#else
#define SWAP_BIT 0
#endif

struct resource rc32334_res_pci_mem1 = {
	.name = "PCI Mem1",
	.start = 0x50000000,
	.end = 0x5FFFFFFF,
	.flags = IORESOURCE_MEM,
};

struct resource rc32334_res_pci_io1 = {
	.name = "PCI I/O1",
	.start = 0x18800000,
	.end = 0x188FFFFF,
	.flags = IORESOURCE_IO,
};

extern struct pci_ops rc32334_pci_ops;
struct pci_controller rc32334_controller = {
	.pci_ops = &rc32334_pci_ops,
	.mem_resource = &rc32334_res_pci_mem1,
	.io_resource = &rc32334_res_pci_io1,
	.mem_offset     = 0x00000000UL,
	.io_offset      = 0x00000000UL,
};

static void local_config_write(u8 where, u32 data)
{

	rc32300_writel((0x80000000 | (where)), PCI_CFG_CNTL);
	rc32300_sync();
	rc32300_writel(data, PCI_CFG_DATA);
}

static int __init rc32334_pcibridge_init(void)
{
  
	printk("Initializing PCI\n");
	
	ioport_resource.start = rc32334_res_pci_io1.start;
	ioport_resource.end = rc32334_res_pci_io1.end;
/*
	iomem_resource.start = rc32334_res_pci_mem1.start;
	iomem_resource.end = rc32334_res_pci_mem1.end;
*/
	
	/* allow writes to bridge config space */
	rc32300_writel(4, PCI_ARBITRATION);
	
	local_config_write(PCI_VENDOR_ID, 
			   PCI_VENDOR_ID_IDT | (PCI_DEVICE_ID_IDT_RC32334 << 16));
	local_config_write(PCI_COMMAND,  
			   PCI_COMMAND_IO | PCI_COMMAND_MEMORY |
			   PCI_COMMAND_MASTER | PCI_COMMAND_INVALIDATE |
			   PCI_COMMAND_PARITY | PCI_COMMAND_SERR |
			   ((PCI_STATUS_66MHZ | PCI_STATUS_FAST_BACK |
			     PCI_STATUS_DEVSEL_MEDIUM) << 16));
	local_config_write(PCI_CLASS_REVISION, 0x06800001);
	local_config_write(PCI_CACHE_LINE_SIZE, 0x0000ff04);
	local_config_write(PCI_BASE_ADDRESS_0, 0);    // mem bar
	local_config_write(PCI_BASE_ADDRESS_1, 0);    // reserved bar
	local_config_write(PCI_BASE_ADDRESS_2,
			   0 | PCI_BASE_ADDRESS_SPACE_IO);  // I/O bar
	local_config_write(PCI_BASE_ADDRESS_3, 0);    // reserved bar
	local_config_write(PCI_BASE_ADDRESS_4, 0);    // reserved bar
	local_config_write(PCI_BASE_ADDRESS_5, 0);    // reserved bar
	local_config_write(PCI_CARDBUS_CIS, 0);       // reserved
	local_config_write(PCI_SUBSYSTEM_VENDOR_ID,
			   PCI_VENDOR_ID_IDT | (PCI_DEVICE_ID_IDT_79S334 << 16));
	local_config_write(PCI_ROM_ADDRESS, 0);       // reserved
	local_config_write(PCI_CAPABILITY_LIST, 0);   // reserved
	local_config_write(PCI_CAPABILITY_LIST+4, 0); // reserved
	
	local_config_write(PCI_INTERRUPT_LINE, 0x38080101);
	/* retry timeout, trdy timeout */
	local_config_write(PCI_INTERRUPT_LINE+4, 0x00008080);
	
	rc32300_writel(0x00000000, PCI_CFG_CNTL);
	
	/*
	 * CPU -> PCI address translation. Make CPU physical and
	 * PCI bus addresses the same.
	 */
	
	/*
	 * Note!
	 *
	 * Contrary to the RC32334 documentation, the behavior of
	 * the PCI byte-swapping bits appears to be the following:
	 *
	 *   when cpu is in LE: 0 = don't swap, 1 = swap
	 *   when cpu is in BE: 1 = don't swap, 0 = swap
	 *
	 * This is true both when the cpu/DMA accesses PCI device
	 * memory/io, and when a PCI bus master accesses system memory.
	 *
	 * Furthermore, byte-swapping doesn't even seem to work
	 * correctly when it is enabled.
	 *
	 * The solution to all this is to disable h/w byte-swapping,
	 * use s/w swapping (CONFIG_SWAP_IO_SPACE) for the in/out/read/
	 * write macros (which takes care of device accesses by cpu/dma)
	 * and hope that drivers swap device data in memory (which takes
	 * care of memory accesses by bus-masters).
	 *
	 * Finally, despite the above workaround, there are still
	 * PCI h/w problems on the 79S334A. PCI bus timeouts and
	 * system/parity errors have been encountered.
	 */
	
	/* mem space 1 */
	rc32300_writel(rc32334_res_pci_mem1.start | SWAP_BIT, PCI_MEM1_BASE);
	
	/* i/o space */
	rc32300_writel(rc32334_res_pci_io1.start | SWAP_BIT, PCI_IO1_BASE);
	
	/* use internal arbiter, 0=round robin, 1=fixed */
	rc32300_writel(0, PCI_ARBITRATION);
	
	/*
	 * PCI -> CPU accesses
	 *
	 * Let PCI see system memory at 0x00000000 physical
	 */
	
	rc32300_writel(0x0 | SWAP_BIT, PCI_CPU_MEM1_BASE); /* mem space */
	rc32300_writel(0x0 | SWAP_BIT, PCI_CPU_IO_BASE);   /* i/o space */
	
	register_pci_controller(&rc32334_controller);
	
	rc32300_sync();   
	return 0;
}
arch_initcall(rc32334_pcibridge_init);

/* Do platform specific device initialization at pci_enable_device() time */
int pcibios_plat_dev_init(struct pci_dev *dev)
{
        return 0;
}
