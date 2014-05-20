#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <asm/delay.h>
#include "ar7100.h"

/*
 * PCI cfg an I/O routines are done by programming a 
 * command/byte enable register, and then read/writing
 * the data from a data regsiter. We need to ensure
 * these transactions are atomic or we will end up
 * with corrupt data on the bus or in a driver.
 */
static DEFINE_SPINLOCK(ar7100_pci_lock);

/*
 * Read from PCI config space (host ctrl)
 */
static void crp_read(uint32_t ad_cbe, uint32_t *data)
{
	unsigned long flags;

	spin_lock_irqsave(&ar7100_pci_lock, flags);

    ar7100_reg_wr(AR7100_PCI_CRP_AD_CBE, ad_cbe);
	*data     =  ar7100_reg_rd(AR7100_PCI_CRP_RDDATA);

	spin_unlock_irqrestore(&ar7100_pci_lock, flags);
}

/*
 * Write to PCI config space (host ctrl)
 */
static void crp_write(uint32_t ad_cbe, uint32_t data)
{ 
	unsigned long flags;

	spin_lock_irqsave(&ar7100_pci_lock, flags);
    /*printk("crp write adcbe: %#x to %#x val %#x to %#x\n", 
            (AR7100_CRP_CMD_WRITE | ad_cbe),
            AR7100_PCI_CRP_AD_CBE,
            data, AR7100_PCI_CRP_WRDATA);*/

    ar7100_reg_wr(AR7100_PCI_CRP_AD_CBE, (AR7100_CRP_CMD_WRITE | ad_cbe));
	ar7100_reg_wr(AR7100_PCI_CRP_WRDATA, data);

	spin_unlock_irqrestore(&ar7100_pci_lock, flags);
}


/*
 * Check for PCI errors (aborts, parity etc.), for configuration cycles
 * PCI error reg: 1:0
 * AHB error reg: 0
 * Both write-back-1 to clear.
 */
int 
ar7100_check_error(int verbose)
{
    uint32_t error = 0, trouble = 0, status;

    error = ar7100_reg_rd(AR7100_PCI_ERROR) & 3;

    if (error) {
        ar7100_local_read_config(PCI_STATUS, 2, &status);
        if (verbose) {
            printk("PCI error %d at PCI addr 0x%x status %#x\n", 
                    error, ar7100_reg_rd(AR7100_PCI_ERROR_ADDRESS),
                    status);
        }

        ar7100_reg_wr(AR7100_PCI_ERROR, error);
        ar7100_local_write_config(PCI_STATUS, 2, status);
        /*
         * flush
         */
        ar7100_local_read_config(PCI_STATUS, 2, &status);
        trouble = 1;
    }

    error = 0;
    error = ar7100_reg_rd(AR7100_PCI_AHB_ERROR) & 1;

    if (error) {
        ar7100_local_read_config(PCI_STATUS, 2, &status);

        if (verbose) {
            printk("AHB error %d at AHB status %#x\n", ar7100_reg_rd(AR7100_PCI_AHB_ERROR_ADDRESS), status);
        }

        ar7100_reg_wr(AR7100_PCI_AHB_ERROR, error);
        trouble = 1;
    }

    return trouble;
}

static int 
ar7100_pci_read(uint32_t addr, uint32_t cmd, uint32_t* data)
{
	unsigned long flags;
	int retval = 0;

	spin_lock_irqsave(&ar7100_pci_lock, flags);

    /*
     * printk("cfg rd: adr %#x cber %#x addr %#x cmd %#x\n", AR7100_PCI_CFG_AD, AR7100_PCI_CFG_CBE, addr, cmd);
     */

    ar7100_reg_wr(AR7100_PCI_CFG_AD, addr);
    ar7100_reg_wr(AR7100_PCI_CFG_CBE, cmd);

	/* 
     * the result of the read is now in CFG_RDATA 
     */
    if (!ar7100_check_error(0))
        *data = ar7100_reg_rd(AR7100_PCI_CFG_RDDATA);
    else {
        *data = 0xffffffff;
        retval = 1;
    }

	spin_unlock_irqrestore(&ar7100_pci_lock, flags);
	return retval;
}

static int 
ar7100_pci_write(uint32_t addr, uint32_t cmd, uint32_t data)
{    
	unsigned long flags;
	int retval = 0;

	spin_lock_irqsave(&ar7100_pci_lock, flags);
    /*
     * printk("cfg wr: adr %#x cber %#x addr %#x cmd %#x data %#x\n", AR7100_PCI_CFG_AD, AR7100_PCI_CFG_CBE, addr, cmd, data);
     */

    ar7100_reg_wr(AR7100_PCI_CFG_AD, addr);
    ar7100_reg_wr(AR7100_PCI_CFG_CBE, cmd);

	/* 
     * execute the write by writing to CFG_WDATA 
     */ 
    ar7100_reg_wr(AR7100_PCI_CFG_WRDATA, data);

    if (ar7100_check_error(0))
        retval = 1;

	spin_unlock_irqrestore(&ar7100_pci_lock, flags);
	return retval;
}

/*
 * This is assuming idsel of device 0 is connected to Address line 17
 * Address for type 0 config is as follows:
 * AD: 
 *  1:0 00 indicates type zero transaction
 *  7:2    indicates the target config dword
 *  10:8   indicates the target function within the physical device
 *  31:11  are reserved (and most probably used to connect idsels)
 */
static uint32_t 
ar7100_config_addr(uint8_t bus_num, uint16_t devfn, int where)
{
	uint32_t addr;

	if (!bus_num) {
		/* type 0 */
		addr = (1 << (AR7100_PCI_IDSEL_ADLINE_START + PCI_SLOT(devfn))) |
               ((PCI_FUNC(devfn)) << 8)                                 |
		       (where & ~3);	
	} else {
		/* type 1 */
		addr = (bus_num << 16) | ((PCI_SLOT(devfn)) << 11) | 
			((PCI_FUNC(devfn)) << 8) | (where & ~3) | 1;
	}

	return addr;
}

/*
 * Mask table, bits to mask for quantity of size 1, 2 or 4 bytes.
 * 0 and 3 are not valid indexes...
 */
static uint32_t bytemask[] = {
	/*0*/	0,
	/*1*/	0xff,
	/*2*/	0xffff,
	/*3*/	0,
	/*4*/	0xffffffff,
};

static uint32_t 
local_byte_lane_enable_bits(uint32_t n, int size)
{
	if (size == 1)
		return (0xf & ~BIT(n)) << 20;
	if (size == 2)
		return (0xf & ~(BIT(n) | BIT(n+1))) << 20;
	if (size == 4)
		return 0;
	return 0xffffffff;
}

int 
ar7100_local_read_config(int where, int size, uint32_t *value)
{ 
	uint32_t n, data;

	/*printk("ar7100_local_read_config from %d size %d\n", where, size);*/
	n = where % 4;
	crp_read(where & ~3, &data);
	*value = (data >> (8*n)) & bytemask[size];
	/*printk("ar7100_local_read_config read %#x\n", *value);*/

	return PCIBIOS_SUCCESSFUL;
}

int 
ar7100_local_write_config(int where, int size, uint32_t value)
{
	uint32_t n, byte_enables, data;

	n = where % 4;
	byte_enables = local_byte_lane_enable_bits(n, size);
	if (byte_enables == 0xffffffff)
		return PCIBIOS_BAD_REGISTER_NUMBER;
	data = value << (8*n);
	crp_write((where & ~3) | byte_enables, data);

	return PCIBIOS_SUCCESSFUL;
}

static uint32_t 
byte_lane_enable_bits(uint32_t n, int size)
{
	if (size == 1)
		return (0xf & ~BIT(n)) << 4;
	if (size == 2)
		return (0xf & ~(BIT(n) | BIT(n+1))) << 4;
	if (size == 4)
		return 0;

	return 0xffffffff;
}

static int 
ar7100_pci_read_config(struct pci_bus *bus, unsigned int devfn, int where, 
                       int size, uint32_t *value)
{
	uint32_t n, byte_enables, addr, data, slot = PCI_SLOT(devfn);
	uint8_t bus_num = bus->number;
	int retry = 0;

	*value = 0xffffffff;
	n = where % 4;
	byte_enables = byte_lane_enable_bits(n, size);
	if (byte_enables == 0xffffffff)
		return PCIBIOS_BAD_REGISTER_NUMBER;

	addr = ar7100_config_addr(bus_num, devfn, where);
	retry:;
	if (ar7100_pci_read(addr, byte_enables | AR7100_CFG_CMD_READ, &data)) 
		return PCIBIOS_DEVICE_NOT_FOUND;

	*value = (data >> (8*n)) & bytemask[size];

	if (where == PCI_COMMAND && (*value & 0xffff) == 0xffff && retry++ < 2)
		goto retry;

	return PCIBIOS_SUCCESSFUL;
}

static int 
ar7100_pci_write_config(struct pci_bus *bus,  unsigned int devfn, int where, 
                        int size, uint32_t value)
{
	uint32_t n, byte_enables, addr, data, slot = PCI_SLOT(devfn);
	uint8_t bus_num = bus->number;

	n = where % 4;
	byte_enables = byte_lane_enable_bits(n, size);
	if (byte_enables == 0xffffffff) {
		printk("bad register number!\n");
		return PCIBIOS_BAD_REGISTER_NUMBER;
	}

	addr = ar7100_config_addr(bus_num, devfn, where);
	data = value << (8*n);
	if (ar7100_pci_write(addr, byte_enables | AR7100_CFG_CMD_WRITE, data))
		return PCIBIOS_DEVICE_NOT_FOUND;

	return PCIBIOS_SUCCESSFUL;
}

struct pci_ops ar7100_pci_ops = {
	.read =  ar7100_pci_read_config,
	.write = ar7100_pci_write_config,
};
