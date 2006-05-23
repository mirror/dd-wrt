/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     PCI initialization for IDT EB365 board
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
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/types.h>

#include <asm/idt-boards/rc32300/rc32300.h>
#include <asm/idt-boards/rc32300/rc32365_pci.h> 
#include <asm/idt-boards/rc32300/rc32365_pci_v.h> 
#include <asm/idt-boards/rc32300/rc32365_dma_v.h> 
#include <linux/byteorder/swab.h>
#include <linux/interrupt.h>

/* define an unsigned array for the PCI registers */
static unsigned int cedarCnfgRegs[25] = {
  CEDAR_CNFG1,  CEDAR_CNFG2,  CEDAR_CNFG3,  CEDAR_CNFG4,	
  CEDAR_CNFG5,  CEDAR_CNFG6,  CEDAR_CNFG7,  CEDAR_CNFG8,
  CEDAR_CNFG9,  CEDAR_CNFG10, CEDAR_CNFG11, CEDAR_CNFG12,
  CEDAR_CNFG13,	CEDAR_CNFG14, CEDAR_CNFG15, CEDAR_CNFG16,
  CEDAR_CNFG17,	CEDAR_CNFG18, CEDAR_CNFG19, CEDAR_CNFG20,
  CEDAR_CNFG21,	CEDAR_CNFG22, CEDAR_CNFG23, CEDAR_CNFG24
};


static struct resource rc32365_res_pci_mem1;
static struct resource rc32365_res_pci_io1;

static struct resource rc32365_res_pci_mem1 = {
  .name = "PCI MEM1",
  .start = 0x50000000,
  .end = 0x5FFFFFFF,
  .flags = IORESOURCE_MEM,
};
static struct resource rc32365_res_pci_io1 = {
  .name = "PCI I/O1",
  .start = 0x18800000,
  .end = 0x188FFFFF,
  .flags = IORESOURCE_IO,
};

extern struct pci_ops rc32365_pci_ops;
static struct pci_controller rc32365_controller = {
  .pci_ops = &rc32365_pci_ops,
  .mem_resource = &rc32365_res_pci_mem1,
  .io_resource = &rc32365_res_pci_io1,
  .mem_offset     = 0x00000000UL,
  .io_offset      = 0x00000000UL,
};

static __init int rc32365_pcibridge_init(void)
{
  
  unsigned int pciConfigAddr;/*used for writing pci config values */
  int	     loopCount    ;/*used for the loop */
  unsigned int pcicValue, pcicData = 0;
  unsigned int dummyRead, pciCntlVal;

  printk("Initialising PCI.\n");
  ioport_resource.start = rc32365_res_pci_io1.start;
  ioport_resource.end = rc32365_res_pci_io1.end;
/*
  iomem_resource.start = rc32365_res_pci_mem1.start;
  iomem_resource.end = rc32365_res_pci_mem1.end;
*/

  /* Disable the IP bus error for PCI scaning */
  pciCntlVal=rc32365_pci->pcic;
  pciCntlVal &= 0xFFFFFF7;
  rc32365_pci->pcic = pciCntlVal;

  pcicValue = rc32365_pci->pcic;
  pcicValue = (pcicValue >> PCIM_SHFT) & PCIM_BIT_LEN;
  if (!((pcicValue == PCIM_H_EA) ||
	(pcicValue == PCIM_H_IA_FIX) ||
	(pcicValue == PCIM_H_IA_RR))) {
    /* Not in Host Mode, return ERROR */
    return -1 ;
  }
  
  /* Enables the Idle Grant mode, Arbiter Parking */
  pcicData |= (PCIC_igm_m | PCIC_eap_m | PCIC_en_m);
  rc32365_pci->pcic = pcicData; /* Enable the PCI bus Interface */
  /* Zero out the PCI status & PCI Status Mask */
  for (;;)
    {
      pcicData = rc32365_pci->pcis;
      if (!(pcicData & PCIS_rip_m))
	break;
    }
  
  rc32365_pci->pcis = 0;
  rc32365_pci->pcism = 0xFFFFFFFF;
  /* Zero out the PCI decoupled registers */
  rc32365_pci->pcidac=0; /* disable PCI decoupled accesses at initialization */
  rc32365_pci->pcidas=0; /* clear the status */
  rc32365_pci->pcidasm=0x0000007F; /* Mask all the interrupts */
  /* Mask PCI Messaging Interrupts */
  rc32365_pci_msg->pciiic = 0;
  rc32365_pci_msg->pciiim = 0xFFFFFFFF;
  rc32365_pci_msg->pciioic = 0;
  rc32365_pci_msg->pciioim = 0;
  
  /* Setup PCILB0 as Memory Window */
  rc32365_pci->pcilba[0].a = (unsigned int)(PCI_ADDR_START);
  rc32365_pci->pcilba[0].m = (unsigned int)(PCI_ADDR_START);
#ifdef __MIPSEB__
  rc32365_pci->pcilba[0].c = (((SIZE_256MB & 0x1f) << PCILBAC_size_b) | PCILBAC_sb_m);
#else
  rc32365_pci->pcilba[0].c = ((SIZE_256MB & 0x1f) << PCILBAC_size_b);
#endif
  dummyRead = rc32365_pci->pcilba[0].c; /* flush the CPU write Buffers */

  rc32365_pci->pcilba[1].a = 0;
  rc32365_pci->pcilba[1].m = 0;
  rc32365_pci->pcilba[1].c = 0;
  dummyRead = rc32365_pci->pcilba[1].c; /* flush the CPU write Buffers */

  rc32365_pci->pcilba[2].a = 0;
  rc32365_pci->pcilba[2].m = 0;
  rc32365_pci->pcilba[2].c = 0;
  dummyRead = rc32365_pci->pcilba[2].c; /* flush the CPU write Buffers */
    
  /* Setup PCILBA3 as IO Window */
  rc32365_pci->pcilba[3].a = 0x18800000;
  rc32365_pci->pcilba[3].m = 0x18800000;
#ifdef __MIPSEB__
  rc32365_pci->pcilba[3].c = ((((SIZE_1MB & 0x1ff) << PCILBAC_size_b) | PCILBAC_msi_m) | PCILBAC_sb_m);
#else
  rc32365_pci->pcilba[3].c = (((SIZE_1MB & 0x1ff) << PCILBAC_size_b) | PCILBAC_msi_m);
#endif
  dummyRead = rc32365_pci->pcilba[3].c; /* flush the CPU write Buffers */
  
  pciConfigAddr = (unsigned int)(0x80000004);
  for (loopCount = 0; loopCount < 24; loopCount++) {
    rc32365_pci->pcicfga = pciConfigAddr;
    dummyRead = rc32365_pci->pcicfga;
    if (loopCount == 16 && cedar_za == 0) /* CEDAR_CNFG17 */
      rc32365_pci->pcicfgd = cedarCnfgRegs[loopCount] | PCIPBAC_pp_m | (PCIPBAC_mr_readMult_v << PCIPBAC_mr_b) | PCIPBAC_mrl_m | PCIPBAC_mrm_m;
    else
      rc32365_pci->pcicfgd = cedarCnfgRegs[loopCount];
    dummyRead = rc32365_pci->pcicfgd;
    pciConfigAddr += 4;
  }
  rc32365_pci->pcitc = (unsigned int)((PCITC_RTIMER_VAL & 0xff) << PCITC_rtimer_b) |
    ((PCITC_DTIMER_VAL & 0xff) << PCITC_dtimer_b);
  
  pciCntlVal = rc32365_pci->pcic;
  pciCntlVal &= ~(PCIC_tnr_m);
  rc32365_pci->pcic = pciCntlVal;
  pciCntlVal = rc32365_pci->pcic;
  
  register_pci_controller(&rc32365_controller);

  rc32300_sync();   
  return 0;
}
arch_initcall(rc32365_pcibridge_init);

/* Do platform specific device initialization at pci_enable_device() time */
int pcibios_plat_dev_init(struct pci_dev *dev)
{
        return 0;
}

unsigned char rc32365_pci_inb(unsigned long port, int slow)
{
	if (cedar_za) {
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

		return (unsigned char)(*pdata >> 24);
	}
	else if (slow)
		return (rc32365_inb_p(port));
	else
		return (rc32365_inb(port));
}

void rc32365_pci_outb(unsigned char val, unsigned long port, int slow)
{
	if (cedar_za) {
		volatile DMA_Chan_t pci_dma_regs = (DMA_Chan_t)(DMA0_VirtualAddress + DMACH_memToPci * DMA_CHAN_OFFSET);
		volatile struct DMAD_s desc;
		DMAD_t pdesc = (DMAD_t)KSEG1ADDR(&desc);
		u32 data;
		volatile u32 *pdata = (u32 *)KSEG1ADDR(&data);
		unsigned long flags;

		*pdata = val << 24;
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
	else if (slow)
		rc32365_outb_p(val, port);
	else
		rc32365_outb(val, port);
}

unsigned short rc32365_pci_inw(unsigned long port, int slow)
{
	if (cedar_za) {
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

#ifdef __MIPSEB__
		return (unsigned short)(*pdata >> 16);
#else
		return (unsigned short)((*pdata >> 24) | ((*pdata >> 8) & 0x0000ff00));
#endif
	}
	else
	{
		unsigned short	val;
		if (slow)
			val = (rc32365_inw_p(port));
		else
			val = (rc32365_inw(port));
#ifdef __MIPSEB__
		val = swab16(val);
#endif
		return val;
	}
}

void rc32365_pci_outw(unsigned short val, unsigned long port, int slow)
{
	if (cedar_za) {
		volatile DMA_Chan_t pci_dma_regs = (DMA_Chan_t)(DMA0_VirtualAddress + DMACH_memToPci * DMA_CHAN_OFFSET);
		volatile struct DMAD_s desc;
		DMAD_t pdesc = (DMAD_t)KSEG1ADDR(&desc);
		u32 data;
		volatile u32 *pdata = (u32 *)KSEG1ADDR(&data);
		unsigned long flags;

#ifdef __MIPSEB__
		*pdata = (val << 16);
#else
		*pdata = (val << 24) | ((val << 8) & 0x00ff0000);
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
	else
	{
#ifdef __MIPSEB__
		val = swab16(val);
#endif
		if (slow)
			rc32365_outw_p(val, port);
		else
			rc32365_outw(val, port);
	}
}

