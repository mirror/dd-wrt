/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     PCI init for Ralink RT2880 solution
 *
 *  Copyright 2007 Ralink Inc. (bruce_chang@ralinktech.com.tw)
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
 * May 2007 Bruce Chang
 *
 * Initial Release
 *
 *
 *
 **************************************************************************
 */


#include <linux/types.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <asm/pci.h>
#include <asm/io.h>
#include <asm/rt2880/eureka_ep430.h>
#include <linux/init.h>
#include <linux/mod_devicetable.h>

#ifdef CONFIG_PCI



/*
 * These functions and structures provide the BIOS scan and mapping of the PCI
 * devices.
 */

#define MAX_PCI_DEVS 10
#if 1
#define RT2880_PCI_SLOT1_BASE	0x20000000 //0x00510000 
#define RT2880_PCI_SLOT1_END	RT2880_PCI_SLOT2_BASE -1
#define RT2880_PCI_SLOT2_BASE	0x28000000 //0x00518000 
#define RT2880_PCI_SLOT2_END	0x2fffffff //0x0051ffff 

#define RT2880_PCI_SLOT1_1_BASE	RT2880_PCI_SLOT1_BASE+0x04000000
#define RT2880_PCI_SLOT1_2_BASE	RT2880_PCI_SLOT1_BASE+0x05000000
#define RT2880_PCI_SLOT1_3_BASE	RT2880_PCI_SLOT1_BASE+0x06000000
#define RT2880_PCI_SLOT1_4_BASE	RT2880_PCI_SLOT1_BASE+0x07000000
#define RT2880_PCI_SLOT1_5_BASE	RT2880_PCI_SLOT1_BASE+0x07800000

#define RT2880_PCI_SLOT2_1_BASE	RT2880_PCI_SLOT2_BASE+0x04000000
#define RT2880_PCI_SLOT2_2_BASE	RT2880_PCI_SLOT2_BASE+0x05000000
#define RT2880_PCI_SLOT2_3_BASE	RT2880_PCI_SLOT2_BASE+0x06000000
#define RT2880_PCI_SLOT2_4_BASE	RT2880_PCI_SLOT2_BASE+0x07000000
#define RT2880_PCI_SLOT2_5_BASE	RT2880_PCI_SLOT2_BASE+0x07800000
#else
#define RT2880_PCI_SLOT1_BASE	0x00510000 
#define RT2880_PCI_SLOT1_END	RT2880_PCI_SLOT2_BASE -1
#define RT2880_PCI_SLOT2_BASE	0x00518000 
#define RT2880_PCI_SLOT2_END	0x0051ffff 
#endif

#define PCI_ACCESS_READ  0
#define PCI_ACCESS_WRITE 1

//extern pci_probe_only;

void __inline__ read_config(unsigned long bus, unsigned long dev, unsigned long func, unsigned long reg, unsigned long *val);
void __inline__ write_config(unsigned long bus, unsigned long dev, unsigned long func, unsigned long reg, unsigned long val);
#if 0
/*  Functions to implement "pci ops"  */
static int rt2880_pcibios_read_config_word(struct pci_dev *dev,
					    int offset, u16 * val);
static int rt2880_pcibios_read_config_byte(struct pci_dev *dev,
					    int offset, u8 * val);
static int rt2880_pcibios_read_config_dword(struct pci_dev *dev,
					     int offset, u32 * val);
static int rt2880_pcibios_write_config_byte(struct pci_dev *dev,
					     int offset, u8 val);
static int rt2880_pcibios_write_config_word(struct pci_dev *dev,
					     int offset, u16 val);
static int rt2880_pcibios_write_config_dword(struct pci_dev *dev,
					      int offset, u32 val);
#endif

static int config_access(unsigned char access_type, struct pci_bus *bus,
                         unsigned int devfn, unsigned char where,
                         u32 * data)
{
  unsigned int slot = PCI_SLOT(devfn);
  u8 func = PCI_FUNC(devfn);
  uint32_t address_reg, data_reg;
  unsigned int address;

  address_reg = EUREKA_EP430_PCI_CONFIG_ADDR;
  data_reg = EUREKA_EP430_PCI_CONFIG_DATA_VIRTUAL_REG;

  /* Setup address */
  address = (bus->number << 16) | (slot << 11) | (func << 8) | (where& 0xfc) | 0x80000000;

  /* start the configuration cycle */
  MV_WRITE(address_reg, address);

  if (access_type == PCI_ACCESS_WRITE){
    MV_WRITE(data_reg, *data);
  }else{
    MV_READ(data_reg, data);
  }
  return 0;
}



static int read_config_byte(struct pci_bus *bus, unsigned int devfn,
                            int where, u8 * val)
{
  u32 data;
  int ret;

  ret = config_access(PCI_ACCESS_READ, bus, devfn, where, &data);
  *val = (data >> ((where & 3) << 3)) & 0xff;
  return ret;
}

static int read_config_word(struct pci_bus *bus, unsigned int devfn,
                            int where, u16 * val)
{
  u32 data;
  int ret;

  ret = config_access(PCI_ACCESS_READ, bus, devfn, where, &data);
  *val = (data >> ((where & 3) << 3)) & 0xffff;
  return ret;
}

static int read_config_dword(struct pci_bus *bus, unsigned int devfn,
                             int where, u32 * val)
{
  int ret;

  ret = config_access(PCI_ACCESS_READ, bus, devfn, where, val);
  return ret;
}
static int
write_config_byte(struct pci_bus *bus, unsigned int devfn, int where,
                  u8 val)
{
  u32 data = 0;

  if (config_access(PCI_ACCESS_READ, bus, devfn, where, &data))
    return -1;

  data = (data & ~(0xff << ((where & 3) << 3))) |
    (val << ((where & 3) << 3));

  if (config_access(PCI_ACCESS_WRITE, bus, devfn, where, &data))
    return -1;

  return PCIBIOS_SUCCESSFUL;
}

static int
write_config_word(struct pci_bus *bus, unsigned int devfn, int where,
                  u16 val)
{
  u32 data = 0;

  if (config_access(PCI_ACCESS_READ, bus, devfn, where, &data))
    return -1;

  data = (data & ~(0xffff << ((where & 3) << 3))) |
    (val << ((where & 3) << 3));

  if (config_access(PCI_ACCESS_WRITE, bus, devfn, where, &data))
    return -1;


  return PCIBIOS_SUCCESSFUL;
}

static int
write_config_dword(struct pci_bus *bus, unsigned int devfn, int where,
                   u32 val)
{
  if (config_access(PCI_ACCESS_WRITE, bus, devfn, where, &val))
    return -1;

  return PCIBIOS_SUCCESSFUL;
}

static int pci_config_read(struct pci_bus *bus, unsigned int devfn,
                       int where, int size, u32 * val)
{
   switch (size) {
  case 1:
    return read_config_byte(bus, devfn, where, (u8 *) val);
  case 2:
    return read_config_word(bus, devfn, where, (u16 *) val);
  default:
    return read_config_dword(bus, devfn, where, val);
  }
}

static int pci_config_write(struct pci_bus *bus, unsigned int devfn,
                        int where, int size, u32 val)
{
  switch (size) {
  case 1:
    return write_config_byte(bus, devfn, where, (u8) val);
  case 2:
    return write_config_word(bus, devfn, where, (u16) val);
  default:
    return write_config_dword(bus, devfn, where, val);
  }
}


/*
 *  General-purpose PCI functions.
 */

struct pci_ops rt2880_pci_ops= {
  .read =  pci_config_read,
  .write = pci_config_write,
};

static struct resource rt2880_res_pci_mem1 = {
  .name = "PCI MEM1",
//  .start = 0,
//  .end = 0x0fffffff,
  .start = 0x20000000,
  .end = 0x2FFFFFFF,
  .flags = IORESOURCE_MEM,
};
static struct resource rt2880_res_pci_io1 = {
  .name = "PCI I/O1",
  .start = 0x00460000,
  .end = 0x0046FFFF,
//  .start = 0,
//  .end = 0xffff,
  .flags = IORESOURCE_IO,
};

struct pci_controller rt2880_controller = {
  .pci_ops = &rt2880_pci_ops,
  .mem_resource = &rt2880_res_pci_mem1,
  .io_resource = &rt2880_res_pci_io1,
  .mem_offset     = 0x00000000UL,
  .io_offset      = 0x00000000UL,
};


/*
 * pci_range_ck -
 *
 * Check if the pci device that are trying to access does really exists
 * on the evaluation board.  
 *
 * Inputs :
 * bus - bus number (0 for PCI 0 ; 1 for PCI 1)
 * dev - number of device on the specific pci bus
 *
 * Outpus :
 * 0 - if OK , 1 - if failure
 */
static __inline__ int pci_range_ck(unsigned char bus, unsigned char dev)
{
	/* Accessing device 31 crashes the MV-64340. */
	return 0;
	if (dev < 5)
		return 0;
	return -1;
}

void __inline__ read_config(unsigned long bus, unsigned long dev, unsigned long func, unsigned long reg, unsigned long *val)
{
	unsigned long address_reg, data_reg, address;

 	address_reg = EUREKA_EP430_PCI_CONFIG_ADDR;
        data_reg = EUREKA_EP430_PCI_CONFIG_DATA_VIRTUAL_REG;

        /* set addr */
        address = (bus << 16) | (dev << 11) | (func << 8) | (reg & 0xfc) | 0x80000000 ;

        /* start the configuration cycle */
        MV_WRITE(address_reg, address);
        /* read the data */
        MV_READ(data_reg, val);
	return;
}

void __inline__ write_config(unsigned long bus, unsigned long dev, unsigned long func, unsigned long reg, unsigned long val)
{
	unsigned long address_reg, data_reg, address;

 	address_reg = EUREKA_EP430_PCI_CONFIG_ADDR;
        data_reg = EUREKA_EP430_PCI_CONFIG_DATA_VIRTUAL_REG;

        /* set addr */
        address = (bus << 16) | (dev << 11) | (func << 8) | (reg & 0xfc) | 0x80000000;

        /* start the configuration cycle */
        MV_WRITE(address_reg, address);
        /* read the data */
        MV_WRITE(data_reg, val);
	return;
}

/*
 * marvell_pcibios_(read/write)_config_(dword/word/byte) -
 *
 * reads/write a dword/word/byte register from the configuration space
 * of a device.
 *
 * Note that bus 0 and bus 1 are local, and we assume all other busses are
 * bridged from bus 1.  This is a safe assumption, since any other
 * configuration will require major modifications to the CP7000G
 *
 * Inputs :
 * bus - bus number
 * dev - device number
 * offset - register offset in the configuration space
 * val - value to be written / read
 *
 * Outputs :
 * PCIBIOS_SUCCESSFUL when operation was succesfull
 * PCIBIOS_DEVICE_NOT_FOUND when the bus or dev is errorneous
 * PCIBIOS_BAD_REGISTER_NUMBER when accessing non aligned
 */
#if 0
static int rt2880_pcibios_read_config_dword(struct pci_dev *device,
					      int offset, u32* val)
{
	int dev, bus, func;
	uint32_t address_reg, data_reg;
	uint32_t address;

	bus = device->bus->number;
	dev = PCI_SLOT(device->devfn);
	func = PCI_FUNC(device->devfn);

	/* verify the range */
	if (pci_range_ck(bus, dev))
		return PCIBIOS_DEVICE_NOT_FOUND;

	/* select the MV-64340 registers to communicate with the PCI bus */
	
		address_reg = EUREKA_EP430_PCI_CONFIG_ADDR;
		data_reg = EUREKA_EP430_PCI_CONFIG_DATA_VIRTUAL_REG;
	

	address = (bus << 16) | (dev << 11) | (func << 8) |
		(offset & 0xfc) | 0x80000000;

	/* start the configuration cycle */
	MV_WRITE(address_reg, address);

	/* read the data */
	MV_READ(data_reg, val);

	return PCIBIOS_SUCCESSFUL;
}


static int rt2880_pcibios_read_config_word(struct pci_dev *device,
					     int offset, u16* val)
{
	int dev, bus, func;
	uint32_t address_reg, data_reg;
	uint32_t address,temp;

	bus = device->bus->number;
	dev = PCI_SLOT(device->devfn);
	func = PCI_FUNC(device->devfn);

	/* verify the range */
	if (pci_range_ck(bus, dev))
		return PCIBIOS_DEVICE_NOT_FOUND;

	/* select the MV-64340 registers to communicate with the PCI bus */
	
		address_reg = EUREKA_EP430_PCI_CONFIG_ADDR;
		data_reg = EUREKA_EP430_PCI_CONFIG_DATA_VIRTUAL_REG;
	

	address = (bus << 16) | (dev << 11) | (func << 8) |
		(offset & 0xfc) | 0x80000000;

	/* start the configuration cycle */
	MV_WRITE(address_reg, address);

	/* read the data */
	//MV_READ_16(data_reg + (offset & 0x3), val);
	/* read the data */
	MV_READ(data_reg, &temp);

	switch(offset % 4)
    {
        case 0:
	case 1:
	    temp &= 0x0000FFFF;
	    break;
	case 2:
	case 3:
	    temp &= 0xFFFF0000;
	    temp = temp >> 16;
	    break;
    }
	*val = (u16)temp;

	return PCIBIOS_SUCCESSFUL;
}

static int rt2880_pcibios_read_config_byte(struct pci_dev *device,
					     int offset, u8* val)
{
	int dev, bus, func;
	uint32_t address_reg, data_reg;
	uint32_t address;
	uint32_t temp;

	bus = device->bus->number;
	dev = PCI_SLOT(device->devfn);
	func = PCI_FUNC(device->devfn);

	/* verify the range */
	if (pci_range_ck(bus, dev))
		return PCIBIOS_DEVICE_NOT_FOUND;

	/* select the MV-64340 registers to communicate with the PCI bus */
	
		address_reg = EUREKA_EP430_PCI_CONFIG_ADDR;
		data_reg = EUREKA_EP430_PCI_CONFIG_DATA_VIRTUAL_REG;
	

	address = (bus << 16) | (dev << 11) | (func << 8) |
		(offset & 0xfc) | 0x80000000;

	/* start the configuration cycle */
	MV_WRITE(address_reg, address);

	/* read the data */
	MV_READ(data_reg, &temp);


	/* read the data */
	//MV_READ(data_reg, val);
	
	/* write the data */
	//MV_READ_8(data_reg + (offset & 0x3), val);

	switch(offset % 4)
    {
        case 0:
	    temp &= 0x000000FF;
	    break;
	case 1:
	    temp &= 0x0000FF00;
   	    temp = temp >> 8;
	    break;
	case 2:
            temp &= 0x00FF0000;
            temp = temp >> 16;
            break;
 	case 3:
            temp &= 0xFF000000;
            temp = temp >> 24;
 	break;
    }

	*val = (u8)temp;

	return PCIBIOS_SUCCESSFUL;
}

static int rt2880_pcibios_write_config_dword(struct pci_dev *device,
					      int offset, u32 val)
{
	int dev, bus, func;
	uint32_t address_reg, data_reg;
	uint32_t address;

	bus = device->bus->number;
	dev = PCI_SLOT(device->devfn);
	func = PCI_FUNC(device->devfn);

	/* verify the range */
	if (pci_range_ck(bus, dev))
		return PCIBIOS_DEVICE_NOT_FOUND;

	/* select the MV-64340 registers to communicate with the PCI bus */
	
		address_reg = EUREKA_EP430_PCI_CONFIG_ADDR;
		data_reg = EUREKA_EP430_PCI_CONFIG_DATA_VIRTUAL_REG;
	

	address = (bus << 16) | (dev << 11) | (func << 8) |
		(offset & 0xfc) | 0x80000000;

	/* start the configuration cycle */
	MV_WRITE(address_reg, address);

	/* write the data */
	MV_WRITE(data_reg, val);

	return PCIBIOS_SUCCESSFUL;
}


static int rt2880_pcibios_write_config_word(struct pci_dev *device,
					     int offset, u16 val)
{
	int dev, bus, func;
	uint32_t address_reg, data_reg;
	uint32_t address,temp;

	bus = device->bus->number;
	dev = PCI_SLOT(device->devfn);
	func = PCI_FUNC(device->devfn);

	/* verify the range */
	if (pci_range_ck(bus, dev))
		return PCIBIOS_DEVICE_NOT_FOUND;

	/* select the MV-64340 registers to communicate with the PCI bus */
	
		address_reg = EUREKA_EP430_PCI_CONFIG_ADDR;
		data_reg = EUREKA_EP430_PCI_CONFIG_DATA_VIRTUAL_REG;
	
	address = (bus << 16) | (dev << 11) | (func << 8) |
		(offset & 0xfc) | 0x80000000;

	/* start the configuration cycle */
	MV_WRITE(address_reg, address);
	

	/* read the data */
	MV_READ(data_reg, &temp);

	switch(offset % 4)
    { 
        case 0:
		case 1:
			temp &= 0xFFFF0000;
            temp += val;
	    
	    break;
		case 2:
		case 3:
            temp &= 0x0000FFFF;
            temp += (u32)(((u32)val) << 16);
	    
		break;
    }

	
	/* write the data */
	MV_WRITE(data_reg, temp);
	

	/* write the data */
	//MV_WRITE_16(data_reg + (offset & 0x3), val);

	return PCIBIOS_SUCCESSFUL;
}

static int rt2880_pcibios_write_config_byte(struct pci_dev *device,
					     int offset, u8 val)
{
	int dev, bus, func;
	uint32_t address_reg, data_reg;
	uint32_t address,temp;

	bus = device->bus->number;
	dev = PCI_SLOT(device->devfn);
	func = PCI_FUNC(device->devfn);

	/* verify the range */
	if (pci_range_ck(bus, dev))
		return PCIBIOS_DEVICE_NOT_FOUND;

	/* select the MV-64340 registers to communicate with the PCI bus */
	
		address_reg = EUREKA_EP430_PCI_CONFIG_ADDR;
		data_reg = EUREKA_EP430_PCI_CONFIG_DATA_VIRTUAL_REG;
	

	address = (bus << 16) | (dev << 11) | (func << 8) |
		(offset & 0xfc) | 0x80000000;

	/* start the configuration cycle */
	MV_WRITE(address_reg, address);

	/* read the data */
	MV_READ(data_reg, &temp);

 	switch (offset % 4)
    {
        case 0:
	    temp &= 0xFFFFFF00;
	    temp += val;
	     break;
	case 1:
		temp &= 0xFFFF00FF;
	    temp += (u32)(((u32)val) << 8);
	    break;
	case 2:
		temp &= 0xFF00FFFF;
	    temp += (u32)(((u32)val) << 16);
	     break;
	case 3:
   	    temp &= 0x00FFFFFF;
	    temp += (u32)(((u32)val) << 24);
	    break;
    }

//--------------------------	

	/* write the data */
//	MV_WRITE_8(data_reg + (offset & 0x3), val);

	/* write the data */
	MV_WRITE(data_reg, temp);


	return PCIBIOS_SUCCESSFUL;
}
#endif


#if 0
void __init pcibios_fixup_bus(struct pci_bus *bus)
{
	struct pci_bus *current_bus = bus;
	struct pci_dev *devices;
	struct list_head *devices_link;
	u32 cmd;
	/* loop over all known devices on this bus */
	//printk("%s %s():%d\n",__FILE__,__FUNCTION__,__LINE__);
	list_for_each(devices_link, &(current_bus->devices)) {

		devices = pci_dev_b(devices_link);
		if (devices == NULL)
			continue;
		#if 0
		if ((current_bus->number == 0) &&
			(PCI_SLOT(devices->devfn) == 1) &&
			(PCI_FUNC(devices->devfn) == 0)) {
			/* LSI 53C10101R SCSI (A) */
			devices->irq = 2;
		} else if ((current_bus->number == 0) &&
			(PCI_SLOT(devices->devfn) == 1) &&
			(PCI_FUNC(devices->devfn) == 1)) {
			/* LSI 53C10101R SCSI (B) */
			devices->irq = 2;
		} else if ((current_bus->number == 1) &&
			(PCI_SLOT(devices->devfn) == 1)) {
			/* Intel 21555 bridge */
			devices->irq = 12;
		} else if ((current_bus->number == 1) &&
			(PCI_SLOT(devices->devfn) == 2)) {
			/* PMC Slot */
			devices->irq = 4;
		} else {
			/* We don't have assign interrupts for other devices. */
			devices->irq = 0xff;
		}
		#endif
		/* Assign an interrupt number for the device */
		//bus->ops->write_byte(devices, PCI_INTERRUPT_LINE, devices->irq);

		/* enable master for everything but the MV-64340 */
		//if (((current_bus->number != 0) && (current_bus->number != 1))
		//		|| (PCI_SLOT(devices->devfn) != 0)) {
		//	bus->ops->read(devices, PCI_COMMAND, &cmd);
		//	cmd |= PCI_COMMAND_MASTER;
		//	bus->ops->write(devices, PCI_COMMAND, cmd);
		//}
	}
	//mv64340_board_pcibios_fixup_bus(c);
}
#endif
#if 0
int pci_scan(unsigned long slot)
{
	unsigned long val, i, BaseAddr, data = 0;
	//int io_set=0, mem_set=0;
	
	//printk("%s %s():%d\n",__FILE__,__FUNCTION__,__LINE__);
	BaseAddr = PCI_BASE_ADDRESS_0;

	read_config(0, slot, 0, 0, &val);
	if(val !=0){
	  for (i=0;i<2;i++, BaseAddr+=4) {  //detect resource usage
	
	  write_config(0, slot, 0, BaseAddr, 0xffffffff);
	  read_config(0, slot, 0, BaseAddr, &data);
	  //printk("write %d 0xffffffff and read back %x\n", i, data);
          if (data!=0) {  //resource request exist
              int j;
              if (data&1) {  //IO space
		  //if(io_set == 1)
		//	  continue;
                  //pci_config->BAR[i].Type = IO_SPACE;
                  //scan resource size
                  for (j=2;j<32;j++)
                      if (data&(1<<j)) break;
		  if(j>16){
			printk("slot 0x%x, request memory over 32k, not support\n", slot);
			return -1;
		  }
		  if(slot == 0x11)
		    write_config(0, slot, 0, BaseAddr, 0x00520000);
		  else if(slot == 0x12)
		    write_config(0, slot, 0, BaseAddr, 0x00528000);

		    read_config(0, slot, 0, BaseAddr, &data);
		    //printk(" ********* %x\n", data);
		  //io_set = 1;
                  //if (j<32) pci_config->BAR[i].Size = 1<<j;
                  //else  pci_config->BAR[i].Size = 0;
              } else {  //Memory space
		  //if(mem_set == 1)
		//	  continue;
                  //pci_config->BAR[i].Type = MEM_SPACE;
                  //bus width
                  if ((data&0x0006)==4) {
		    //pci_config->BAR[i].Width = WIDTH64; //bus width 64
		    printk("slot 0x%x, 64bit, not support\n", slot);
		    return -1;
		  }
                  //else 
		     //pci_config->BAR[i].Width = WIDTH32;  //bus width 32

                  //prefetchable
                  //if (data&0x0008) pci_config->BAR[i].Prefetch = 1; //prefetchable
                  //else pci_config->BAR[i].Prefetch = 0;  //no prefetch
                  //scan resource size
                  //if (pci_config->BAR[i].Width==WIDTH32) {
                    for (j=4;j<32;j++)
                      if (data&(1<<j)) break;
		    if(j>16){
			printk("slot 0x%x, request memory over 32k, not support\n", slot);
			return -1;
		    }
		  if(slot == 0x11)
		    write_config(0, slot, 0, BaseAddr, RT2880_PCI_SLOT1_BASE);
		  else if(slot == 0x12)
		    write_config(0, slot, 0, BaseAddr, RT2880_PCI_SLOT2_BASE);

		    //read_config(0, slot, 0, BaseAddr, &data);
		    //printk(" ********* %x\n", data);
		    //mem_set = 1;
                    //if (j<32) pci_config->BAR[i].Size = 1<<j;
                    //else  pci_config->BAR[i].Size = 0;
                  //} else //width64 is not support
                  //  pci_config->BAR[i].Size = 0;
              };
          } else {  //no resource
              //memset(&(pci_config->BAR[i]), 0, sizeof(base_address_s));
		printk("slot 0x%x, error access\n", slot);
		return -1;
          };
    	  };//for
	} else {
	  printk("slot 0x%x empty\n", slot);
	}
}
void pcibios_fixup_resources(struct pci_dev *dev)
{

	u16 cmd;
	u32 bus, devid, func;

		printk("*********************%s %s():%d\n",__FILE__,__FUNCTION__,__LINE__);
		pci_write_config_byte(dev, PCI_CACHE_LINE_SIZE, 0x14);  //configure cache line size 0x14
  		pci_write_config_byte(dev, PCI_LATENCY_TIMER, 0xFF);  //configure latency timer 0x10
		//Set device
		pci_read_config_word(dev, PCI_COMMAND, &cmd);		
#if 0
		cmd = cmd | PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY;
		cmd |= (PCI_COMMAND_PARITY | PCI_COMMAND_SERR);			 
#else
		cmd = cmd | PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY |
			PCI_COMMAND_INVALIDATE | PCI_COMMAND_FAST_BACK | PCI_COMMAND_SERR |
			PCI_COMMAND_WAIT | PCI_COMMAND_PARITY;
#endif
		pci_write_config_word(dev, PCI_COMMAND, cmd);
		
        	bus = dev->bus->number;
        	devid = PCI_SLOT(dev->devfn);
        	func = PCI_FUNC(dev->devfn);
		//printk("bus = %d, dev = %d, fun = %d\n", bus,devid,func);
		
		if (bus == 0 && (devid == 0x11 ||devid==0x12) ) {

		unsigned long i, j, BaseAddr;
		u32 val, data=0;
		int mem_cnt=0;
		struct resource *res;
		BaseAddr = PCI_BASE_ADDRESS_0;

		pci_write_config_dword(dev, BaseAddr, ~0);
		pci_read_config_dword(dev, BaseAddr, &val);
		if(val !=0){	//find the card
	  	  for (i=0;i<3;i++, BaseAddr+=4) {  //detect resource usage
			pci_write_config_dword(dev, BaseAddr, ~0);
			pci_read_config_dword(dev, BaseAddr, &data);
			res = &dev->resource[i];
          		if (data!=0) {  //resource request exist
              		  if (data&1) {  //IO space

				for (j=2;j<32;j++)
				  if (data&(1<<j)) break;	
		  		if(devid == 0x11){
	  			  pci_write_config_dword(dev, BaseAddr, 0x00460000);
				  res->start = 0x00460000;
				  res->end = 0x00467FFF;
				}
		  		else if(devid == 0x12){
	  			  pci_write_config_dword(dev, BaseAddr, 0x00528000);
				  res->start = 0x00528000;
				  res->end = 0x0052FFFF;
				}

				pci_read_config_dword(dev, BaseAddr, &data);
		    		//printk(" ********* %x\n", data);
              		  } else { //memory space 
				for (j=4;j<32;j++)
                      		  if (data&(1<<j)) break;

		  		if(devid == 0x11){
				 if(mem_cnt==0) {
	  			  pci_write_config_dword(dev, BaseAddr, RT2880_PCI_SLOT1_BASE);
				  res->start = RT2880_PCI_SLOT1_BASE;
				  //res->size = 1<<j;
				  res->end = RT2880_PCI_SLOT1_BASE + (1<<j) -1;
				 }else if(mem_cnt==1){
	  			  pci_write_config_dword(dev, BaseAddr, RT2880_PCI_SLOT1_1_BASE);
				  res->start = RT2880_PCI_SLOT1_1_BASE;
				  //res->size = 1<<j;
				  res->end = RT2880_PCI_SLOT1_1_BASE + (1<<j) -1;
				 }
				}
		  		else if(devid== 0x12){
				 if(mem_cnt==0) {
	  			  pci_write_config_dword(dev, BaseAddr, RT2880_PCI_SLOT2_BASE);
				  res->start = RT2880_PCI_SLOT2_BASE;
				  //res->size = 1<<j;
				  res->end = RT2880_PCI_SLOT2_BASE + (1<<j) -1;
				 }else if(mem_cnt==1){
	  			  pci_write_config_dword(dev, BaseAddr, RT2880_PCI_SLOT2_1_BASE);
				  res->start = RT2880_PCI_SLOT2_1_BASE;
				  //res->size = 1<<j;
				  res->end = RT2880_PCI_SLOT2_1_BASE + (1<<j) -1;
				 }
				}

				pci_read_config_dword(dev, BaseAddr, &data);
				//printk("start = 0x%08x, end = 0x%08x\n", res->start, res->end);
		    		//printk(" ********* %x\n", data);
				mem_cnt++;
              		  }//if(data&1)
          		}else{
				printk("%s %s():%d   ",__FILE__,__FUNCTION__,__LINE__);
				printk("baseaddr = %x\n", BaseAddr);
			}//if(data!=0)
		  }//for
		}else{
			printk(" slot =%d configuration space access error\n", devid);
		}
		if(devid == 0x11) {
			dev->irq = 2;
			pci_write_config_byte(dev, PCI_INTERRUPT_LINE, 2);
			//pci_write_config_byte(dev, PCI_INTERRUPT_PIN, 2);
		}else if(devid == 0x12) {
			dev->irq = 15;
			pci_write_config_byte(dev, PCI_INTERRUPT_LINE, 15);
			//pci_write_config_byte(dev, PCI_INTERRUPT_PIN, 15);
		}

		}

		return;

}
#endif

int __init pcibios_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
  u16 cmd;
  u32 val;
  struct resource *res;
  int i;
  if (dev->bus->number != 0) {
    return 0;
  }
  slot = PCI_SLOT(dev->devfn);

  if(slot == 0) {
  	write_config(0, 0, 0, PCI_BASE_ADDRESS_0, 0x08000000);
  	read_config(0, 0, 0, PCI_BASE_ADDRESS_0, &val);
  	dev->irq = 0;
    res = &dev->resource[0];
    res->start = 0x08000000;
    res->end   = 0x09ffffff;
 	printk("BAR0 at slot 0 = %x\n", val);
  }else if(slot ==0x11){
	dev->irq = 2;
  }else if(slot==0x12){
	dev->irq = 15;
  }else{
  	return 0;
  }	

  for(i=0;i<6;i++){
    res = &dev->resource[i];
    //printk("res[%d]->start = %x\n", i, res->start);
    //printk("res[%d]->end = %x\n", i, res->end);
  }

  pci_write_config_byte(dev, PCI_CACHE_LINE_SIZE, 0x14);  //configure cache line size 0x14
  pci_write_config_byte(dev, PCI_LATENCY_TIMER, 0xFF);  //configure latency timer 0x10
#if 1
  pci_read_config_word(dev, PCI_COMMAND, &cmd);
  cmd = cmd | PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY |
  	PCI_COMMAND_INVALIDATE | PCI_COMMAND_FAST_BACK | PCI_COMMAND_SERR |
  	PCI_COMMAND_WAIT | PCI_COMMAND_PARITY;
  pci_write_config_word(dev, PCI_COMMAND, cmd);
#endif
  pci_write_config_byte(dev, PCI_INTERRUPT_LINE, dev->irq);
  //pci_write_config_byte(dev, PCI_INTERRUPT_PIN, dev->irq);
  return (dev->irq);
}

int init_rt2880pci(void)
{
	unsigned long val = 0;
	int i;


	//pci_probe_only = 1;
	RALINK_PCI_PCICFG_ADDR = 0;
	for(i=0;i<0xfffff;i++);
	RT2880_PCI_ARBCTL = 0x79;
	//printk(" RT2880_PCI_ARBCTL = %x\n", RT2880_PCI_ARBCTL);

/*
	ioport_resource.start = rt2880_res_pci_io1.start;
  	ioport_resource.end = rt2880_res_pci_io1.end;
*/

	RALINK_PCI_BAR0SETUP_ADDR = 0x07FF0001;//0x07ff0001;	//open 32M
	//RALINK_PCI_BAR1SETUP_ADDR = 0;
	RT2880_PCI_MEMBASE = RT2880_PCI_SLOT1_BASE;
	RT2880_PCI_IOBASE = 0x00460000;
	RALINK_PCI_IMBASEBAR0_ADDR = 0x08000000;

	RT2880_PCI_ID = 0x08021814;
	RT2880_PCI_CLASS = 0x00800001;
	RT2880_PCI_SUBID = 0x28801814;

	RALINK_PCI_PCIMSK_ADDR = 0x000c0000; // enable pci interrupt

	write_config(0, 0, 0, PCI_BASE_ADDRESS_0, 0x08000000);
	read_config(0, 0, 0, PCI_BASE_ADDRESS_0, &val);
	printk("BAR0 at slot 0 = %x\n", val);
/*
	val = RALINK_PCI_PCIMSK_ADDR;
	val |= 0x000C0000;
	RALINK_PCI_PCIMSK_ADDR = val;
*/
	register_pci_controller(&rt2880_controller);
	return 0;

}
#if 0
void __init pcibios_fixup_irqs(void)
{
    struct pci_dev *dev;

        pci_for_each_dev(dev) {
                dev->irq = 2; // fix irq
        }
}
#endif


#if 0
void __init rt2880_pcibios_init(void)
{
	//printk("\n pcibios_init is called ioport_resource = %08X\n",&ioport_resource);
	//printk("\n pcibios_init is called iomem_resource = %08X\n",&iomem_resource);
	
	/* Reset PCI I/O and PCI MEM values */
	/* Reset PCI I/O and PCI MEM values */
	//ioport_resource.start = 0;//0xc0000000;
	//ioport_resource.end   = 0;//0xc0000000 + 0x20000000 - 1;
	//iomem_resource.start  = 0;//0xc0000000;
	//iomem_resource.end    = 0;//0xc0000000 + 0x20000000 - 1;
	
	//printk("%s %s():%d\n",__FILE__,__FUNCTION__,__LINE__);
	init_rt2880pci();
		
	pci_scan_bus(0, &rt2880_pci_ops, NULL);

	//pci_scan(0x11);
	//pci_scan(0x12);

	//pcibios_fixup_irqs();
}
#endif

arch_initcall(init_rt2880pci);


/* Do platform specific device initialization at pci_enable_device() time */
int pcibios_plat_dev_init(struct pci_dev *dev)
{
	return 0;
}

struct pci_fixup pcibios_fixups[] = {
//	{PCI_ANY_ID, PCI_ANY_ID, pcibios_fixup_resources },
	{0}
};
//DECLARE_PCI_FIXUP_FINAL(PCI_ANY_ID, PCI_ANY_ID, pcibios_fixup_resources)
#endif	/* CONFIG_PCI */
