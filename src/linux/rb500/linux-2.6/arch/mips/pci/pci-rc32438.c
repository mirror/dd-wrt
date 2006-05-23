/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     PCI initialization for IDT EB438 board
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
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/idt-boards/rc32438/rc32438.h>
#include <asm/idt-boards/rc32438/rc32438_pci.h>
#include <asm/idt-boards/rc32438/rc32438_pci_v.h>
#include <asm/idt-boards/rc32438/rc32438_dma_v.h>
#include <linux/interrupt.h>

#define PCI_ACCESS_READ  0
#define PCI_ACCESS_WRITE 1

/* define an unsigned array for the PCI registers */
unsigned int acaciaCnfgRegs[25] = {
  ACACIA_CNFG1,	 ACACIA_CNFG2,  ACACIA_CNFG3,  ACACIA_CNFG4,
  ACACIA_CNFG5,	 ACACIA_CNFG6,  ACACIA_CNFG7,  ACACIA_CNFG8,
  ACACIA_CNFG9,	 ACACIA_CNFG10, ACACIA_CNFG11, ACACIA_CNFG12,
  ACACIA_CNFG13, ACACIA_CNFG14, ACACIA_CNFG15, ACACIA_CNFG16,
  ACACIA_CNFG17, ACACIA_CNFG18, ACACIA_CNFG19, ACACIA_CNFG20,
  ACACIA_CNFG21, ACACIA_CNFG22, ACACIA_CNFG23, ACACIA_CNFG24
};
static struct resource rc32438_res_pci_mem1 = {
  .name = "PCI MEM1",
  .start = 0x50000000,
  .end = 0x5FFFFFFF,
  .flags = IORESOURCE_MEM,
};
static struct resource rc32438_res_pci_io1 = {
  .name = "PCI I/O1",
  .start = 0x18800000,
  .end = 0x188FFFFF,
  .flags = IORESOURCE_IO,
};

extern struct pci_ops rc32438_pci_ops;

struct pci_controller rc32438_controller = {
  .pci_ops = &rc32438_pci_ops,
  .mem_resource = &rc32438_res_pci_mem1,
  .io_resource = &rc32438_res_pci_io1,
  .mem_offset     = 0x00000000UL,
  .io_offset      = 0x00000000UL,
};

static int __init rc32438_pcibridge_init(void)
{

  unsigned int pciConfigAddr = 0;/*used for writing pci config values */
  int	     loopCount=0    ;/*used for the loop */

  unsigned int pcicValue, pcicData=0;
  unsigned int dummyRead, pciCntlVal = 0;
  printk("PCI: Initializing PCI\n");

  /* Disable the IP bus error for PCI scaning */
  pciCntlVal=rc32438_pci->pcic;
  pciCntlVal &= 0xFFFFFF7;
  rc32438_pci->pcic = pciCntlVal;

  ioport_resource.start = rc32438_res_pci_io1.start;
  ioport_resource.end = rc32438_res_pci_io1.end;
/*
  iomem_resource.start = rc32438_res_pci_mem1.start;
  iomem_resource.end = rc32438_res_pci_mem1.end;
*/

  pcicValue = rc32438_pci->pcic;
  pcicValue = (pcicValue >> PCIM_SHFT) & PCIM_BIT_LEN;
  if (!((pcicValue == PCIM_H_EA) ||
	(pcicValue == PCIM_H_IA_FIX) ||
	(pcicValue == PCIM_H_IA_RR))) {
    /* Not in Host Mode, return ERROR */
    return -1;
  }

  /* Enables the Idle Grant mode, Arbiter Parking */
  pcicData |=(PCIC_igm_m|PCIC_eap_m|PCIC_en_m);
  rc32438_pci->pcic = pcicData; /* Enable the PCI bus Interface */
  /* Zero out the PCI status & PCI Status Mask */
  for(;;)
    {
      pcicData = rc32438_pci->pcis;
      if (!(pcicData & PCIS_rip_m))
	break;
    }

  rc32438_pci->pcis = 0;
  rc32438_pci->pcism = 0xFFFFFFFF;
  /* Zero out the PCI decoupled registers */
  rc32438_pci->pcidac=0; /* disable PCI decoupled accesses at initialization */
  rc32438_pci->pcidas=0; /* clear the status */
  rc32438_pci->pcidasm=0x0000007F; /* Mask all the interrupts */
  /* Mask PCI Messaging Interrupts */
  rc32438_pci_msg->pciiic = 0;
  rc32438_pci_msg->pciiim = 0xFFFFFFFF;
  rc32438_pci_msg->pciioic = 0;
  rc32438_pci_msg->pciioim = 0;

  /* Setup PCILB0 as Memory Window */
  rc32438_pci->pcilba[0].a = (unsigned int) (PCI_ADDR_START);
  rc32438_pci->pcilba[0].m = (unsigned int) (PCI_ADDR_START);
#ifdef __MIPSEB__
  rc32438_pci->pcilba[0].c = (((SIZE_256MB & 0x1f) << PCILBAC_size_b) | PCILBAC_sb_m);
#else
  rc32438_pci->pcilba[0].c = (((SIZE_256MB & 0x1f) << PCILBAC_size_b));
#endif
  dummyRead = rc32438_pci->pcilba[0].c; /* flush the CPU write Buffers */

  rc32438_pci->pcilba[1].a = 0;
  rc32438_pci->pcilba[1].m = 0;
  rc32438_pci->pcilba[1].c = 0;
  dummyRead = rc32438_pci->pcilba[1].c; /* flush the CPU write Buffers */

  rc32438_pci->pcilba[2].a = 0;
  rc32438_pci->pcilba[2].m = 0;
  rc32438_pci->pcilba[2].c = 0;
  dummyRead = rc32438_pci->pcilba[2].c; /* flush the CPU write Buffers */

  /* Setup PCILBA3 as IO Window */
  rc32438_pci->pcilba[3].a = 0x18800000;
  rc32438_pci->pcilba[3].m = 0x18800000;
#ifdef __MIPSEB__
  rc32438_pci->pcilba[3].c = ((((SIZE_1MB & 0x1ff) << PCILBAC_size_b) | PCILBAC_msi_m) | PCILBAC_sb_m);
#else
  rc32438_pci->pcilba[3].c = (((SIZE_1MB & 0x1ff) << PCILBAC_size_b) | PCILBAC_msi_m);
#endif
  dummyRead = rc32438_pci->pcilba[3].c; /* flush the CPU write Buffers */

  pciConfigAddr=(unsigned int)(0x80000004);
  for(loopCount=0;loopCount<24;loopCount++){
    rc32438_pci->pcicfga=pciConfigAddr;
    dummyRead=rc32438_pci->pcicfga;
    rc32438_pci->pcicfgd = acaciaCnfgRegs[loopCount];
    dummyRead=rc32438_pci->pcicfgd;
    pciConfigAddr += 4;
  }
  rc32438_pci->pcitc=(unsigned int)((PCITC_RTIMER_VAL&0xff) << PCITC_rtimer_b) |
    ((PCITC_DTIMER_VAL&0xff)<<PCITC_dtimer_b);

  pciCntlVal=rc32438_pci->pcic;
  pciCntlVal &=~(PCIC_tnr_m);
  rc32438_pci->pcic = pciCntlVal;
  pciCntlVal=rc32438_pci->pcic;

  register_pci_controller(&rc32438_controller);

  rc32438_sync();  
  return 0;
}
arch_initcall(rc32438_pcibridge_init);

/* Do platform specific device initialization at pci_enable_device() time */
int pcibios_plat_dev_init(struct pci_dev *dev)
{
        return 0;
}

unsigned char rc32438_pci_inb(unsigned long port, int slow)
{
	volatile DMA_Chan_t pci_dma_regs = (DMA_Chan_t)(DMA0_VirtualAddress + DMACH_pciToMem * DMA_CHAN_OFFSET);
	volatile struct DMAD_s desc;
	DMAD_t pdesc = (DMAD_t)KSEG1ADDR(&desc);
	u32 data;
	volatile u32 *pdata = (u32 *)KSEG1ADDR(&data);
	unsigned long flags;

	*pdata = 0;
	pdesc->control = 0x01c00001;
	pdesc->ca      = CPHYSADDR(pdata);
	pdesc->devcs   = port;
	pdesc->link    = 0;

	save_and_cli(flags);
	while (pci_dma_regs->dmac & DMAC_run_m);

	pci_dma_regs->dmas = 0;
	pci_dma_regs->dmandptr = 0;
	pci_dma_regs->dmadptr = CPHYSADDR(pdesc);

	while (!pci_dma_regs->dmas);
	restore_flags(flags);

	if (slow) SLOW_DOWN_IO;

#if defined(__MIPSEB__)
	return (unsigned char)(*pdata >> 24);
#else
	return (unsigned char)(*pdata);
#endif
}

void rc32438_pci_outb(unsigned char val, unsigned long port, int slow)
{
	volatile DMA_Chan_t pci_dma_regs = (DMA_Chan_t)(DMA0_VirtualAddress + DMACH_memToPci * DMA_CHAN_OFFSET);
	volatile struct DMAD_s desc;
	DMAD_t pdesc = (DMAD_t)KSEG1ADDR(&desc);
	u32 data;
	volatile u32 *pdata = (u32 *)KSEG1ADDR(&data);
	unsigned long flags;

#if defined(__MIPSEB__)
	*pdata = val << 24;
#else
	*pdata = val;
#endif
	pdesc->control = 0x01c00001;
	pdesc->ca      = CPHYSADDR(pdata);
	pdesc->devcs   = port;
	pdesc->link    = 0;

	save_and_cli(flags);
	while (pci_dma_regs->dmac & DMAC_run_m);

	pci_dma_regs->dmas = 0;
	pci_dma_regs->dmandptr = 0;
	pci_dma_regs->dmadptr = CPHYSADDR(pdesc);

	while (!pci_dma_regs->dmas);
	restore_flags(flags);

	if (slow) SLOW_DOWN_IO;
}

unsigned short rc32438_pci_inw(unsigned long port, int slow)
{
	volatile DMA_Chan_t pci_dma_regs = (DMA_Chan_t)(DMA0_VirtualAddress + DMACH_pciToMem * DMA_CHAN_OFFSET);
	volatile struct DMAD_s desc;
	DMAD_t pdesc = (DMAD_t)KSEG1ADDR(&desc);
	u32 data;
	volatile u32 *pdata = (u32 *)KSEG1ADDR(&data);
	unsigned long flags;

	*pdata = 0;
	pdesc->control = 0x01c00002;
	pdesc->ca      = CPHYSADDR(pdata);
	pdesc->devcs   = port;
	pdesc->link    = 0;

	save_and_cli(flags);
	while (pci_dma_regs->dmac & DMAC_run_m);

	pci_dma_regs->dmas = 0;
	pci_dma_regs->dmandptr = 0;
	pci_dma_regs->dmadptr = CPHYSADDR(pdesc);

	while (!pci_dma_regs->dmas);
	restore_flags(flags);

	if (slow) SLOW_DOWN_IO;

#if defined(__MIPSEB__)
//	return (unsigned short)((*pdata >> 24) | ((*pdata >> 8) & 0x0000ff00));
	return (unsigned short)(*pdata >> 16);
#else
	return (unsigned short)(*pdata);
#endif
}

void rc32438_pci_outw(unsigned short val, unsigned long port, int slow)
{
	volatile DMA_Chan_t pci_dma_regs = (DMA_Chan_t)(DMA0_VirtualAddress + DMACH_memToPci * DMA_CHAN_OFFSET);
	volatile struct DMAD_s desc;
	DMAD_t pdesc = (DMAD_t)KSEG1ADDR(&desc);
	u32 data;
	volatile u32 *pdata = (u32 *)KSEG1ADDR(&data);
	unsigned long flags;

#if defined(__MIPSEB__)
//	*pdata = (val << 24) | ((val << 8) & 0x00ff0000);
	*pdata = (val << 16);
#else
	*pdata = val;
#endif
	pdesc->control = 0x01c00002;
	pdesc->ca      = CPHYSADDR(pdata);
	pdesc->devcs   = port;
	pdesc->link    = 0;

	save_and_cli(flags);
	while (pci_dma_regs->dmac & DMAC_run_m);

	pci_dma_regs->dmas = 0;
	pci_dma_regs->dmandptr = 0;
	pci_dma_regs->dmadptr = CPHYSADDR(pdesc);

	while (!pci_dma_regs->dmas);
	restore_flags(flags);

	if (slow) SLOW_DOWN_IO;
}

