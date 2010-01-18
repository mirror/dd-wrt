/****************************************************************************/
// Ingenieria Almudi (www.almudi.com)
// Ported to LinCAN by Jose Pascual Ram�rez (josepascual@almudi.com)
// 
//
// Copyright (C) 2001,2002,2003,2004  PEAK System-Technik GmbH
//
// linux@peak-system.com
// www.peak-system.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// Maintainer(s): Klaus Hitschler (klaus.hitschler@gmx.de)
//
// Contributions: Marcel Offermans (marcel.offermans@luminis.nl)
//                Philipp Baer (philipp.baer@informatik.uni-ulm.de)
/****************************************************************************/

/****************************************************************************/
//
// all parts to handle the interface specific parts of pcan-dongle
//
// Revision 1.38  2004/07/28 22:03:29  jose pascual
// ported to LinCAN
//
// Revision 1.37  2004/04/11 22:03:29  klaus
// cosmetic changes
//
// Revision 1.36  2004/04/10 12:25:39  klaus
// merge polished between HEAD and kernel-2.6 branch
//
// Revision 1.35  2004/04/10 08:57:26  klaus
// merge finished between HEAD and kernel-2.6 branch
//
// Revision 1.32.2.1  2004/03/21 12:09:09  klaus
// first commit for branch to kernel 2.6 code
//
// Revision 1.34  2004/03/27 16:57:06  klaus
// modified for use with kernels <= 2.2.14
//
// Revision 1.33  2004/03/27 15:10:54  klaus
// prepared for use with gcc 3.x, modified for use with kernels < 2.2.4
//
// Revision 1.32  2004/03/04 18:50:08  klaus
// renamed PA,PB,PC to _PA_, ... to support (partially) cross-compiling for MIPS
//
// Revision 1.31  2003/06/22 15:34:50  klaus
// added parts to support devfs provided by Philipp Baer (partially untested)
//
// Revision 1.30  2003/06/04 19:26:15  klaus
// adapted to kernel 2.5.69 using GCC 3.2.3 (marcel), released release_20030604_x
//
// Revision 1.28  2003/03/02 10:58:07  klaus
// merged USB thread into main path
//
// Revision 1.27  2003/03/02 10:58:07  klaus
// merged USB thread into main path
//
// Revision 1.26.2.5  2003/01/29 20:34:20  klaus
// release_20030129_a and release_20030129_u released
//
// Revision 1.26.2.4  2003/01/29 20:34:19  klaus
// release_20030129_a and release_20030129_u released
//
// Revision 1.26.2.3  2003/01/28 23:28:26  klaus
// reorderd pcan_usb.c and pcan_usb_kernel.c, tidied up
//
// Revision 1.26.2.2  2003/01/14 20:31:53  klaus
// read/write/minor assigment is working
//
/****************************************************************************/

/****************************************************************************/
// INCLUDES


#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/sja1000p.h"

#include <linux/parport.h>

#include "../include/pcan_dongle.h"



/****************************************************************************/
// DEFINES
#define PCAN_DNG_SP_MINOR_BASE  16  // starting point of minors for SP devices
#define PCAN_DNG_EPP_MINOR_BASE 24  // starting point of minors for EPP devices
#define DNG_PORT_SIZE            4  // the address range of the dongle-port
#define ECR_PORT_SIZE            1  // size of the associated ECR register
#define DNG_DEFAULT_COUNT        4  // count of defaults for init

typedef void (*PARPORT_IRQ_HANLDER)(CAN_IRQ_HANDLER_ARGS(irq_number, dev_id));

/****************************************************************************/
// GLOBALS
CAN_DEFINE_SPINLOCK(pcan_lock);

/****************************************************************************/
// LOCALS
static u16 dng_ports[] = {0x378, 0x278, 0x3bc, 0x2bc};
static u8  dng_irqs[]  = {7, 5, 7, 5};
static u16 dng_devices = 0;        // the number of accepted dng_devices
static u16 epp_devices = 0;        // ... epp_devices
static u16 sp_devices  = 0;        // ... sp_devices

static unsigned char nibble_decode[32] =
{
  0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
  0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
  0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
  0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7
};

struct DONGLE_PORT dongle_port;

char dongle_type[] = "epp_dongle";
//char dongle_type[] = "sp_dongle";

/****************************************************************************/
// CODE

//----------------------------------------------------------------------------
// enable and disable irqs
static void _parport_disable_irq(struct DONGLE_PORT *dng)
{
  u16 _PC_ = (u16)dng->dwPort + 2;
  can_outb(can_inb(_PC_) & ~0x10, _PC_);
}

static void _parport_enable_irq(struct DONGLE_PORT *dng)
{
  u16 _PC_ = (u16)dng->dwPort + 2;
  can_outb(can_inb(_PC_) | 0x10, _PC_);
}


// functions for SP port
static u8 pcan_dongle_sp_readreg(struct DONGLE_PORT *dng, u8 port) // read a register
{
  u16 _PA_ = (u16)dng->dwPort;
  u16 _PB_ = _PA_ + 1;
  u16 _PC_ = _PB_ + 1;
  u8  b0, b1 ;
  u8  irqEnable = can_inb(_PC_) & 0x10; // don't influence irqEnable
  can_spin_irqflags_t flags;

  can_spin_lock_irqsave(&pcan_lock, flags);

  can_outb((0x0B ^ 0x0D) | irqEnable, _PC_);
  can_outb((port & 0x1F) | 0x80,      _PA_);
  can_outb((0x0B ^ 0x0C) | irqEnable, _PC_);
  b1=nibble_decode[can_inb(_PB_)>>3];
  can_outb(0x40, _PA_);
  b0=nibble_decode[can_inb(_PB_)>>3];
  can_outb((0x0B ^ 0x0D) | irqEnable, _PC_);

  can_spin_unlock_irqrestore(&pcan_lock, flags);

  return  (b1 << 4) | b0 ;
}

static void pcan_dongle_writereg(struct DONGLE_PORT *dng, u8 port, u8 data) // write a register
{
  u16 _PA_ = (u16)dng->dwPort;
  u16 _PC_ = _PA_ + 2;
  u8  irqEnable = can_inb(_PC_) & 0x10; // don't influence irqEnable
  can_spin_irqflags_t flags;

  can_spin_lock_irqsave(&pcan_lock, flags);

  can_outb((0x0B ^ 0x0D) | irqEnable, _PC_);
  can_outb(port & 0x1F,               _PA_);
  can_outb((0x0B ^ 0x0C) | irqEnable, _PC_);
  can_outb(data,                      _PA_);
  can_outb((0x0B ^ 0x0D) | irqEnable, _PC_);

  can_spin_unlock_irqrestore(&pcan_lock, flags);
}

// functions for EPP port
static u8 pcan_dongle_epp_readreg(struct DONGLE_PORT *dng, u8 port) // read a register
{
  u16 _PA_ = (u16)dng->dwPort;
  u16 _PC_ = _PA_ + 2;
  u8  wert;
  u8  irqEnable = can_inb(_PC_) & 0x10; // don't influence irqEnable
  can_spin_irqflags_t flags;

  can_spin_lock_irqsave(&pcan_lock, flags);

  can_outb((0x0B ^ 0x0F) | irqEnable, _PC_);
  can_outb((port & 0x1F) | 0x80,      _PA_);
  can_outb((0x0B ^ 0x2E) | irqEnable, _PC_);
  wert = can_inb(_PA_);
  can_outb((0x0B ^ 0x0F) | irqEnable, _PC_);

  can_spin_unlock_irqrestore(&pcan_lock, flags);

  return wert;
}

static int pcan_dongle_req_irq(struct DONGLE_PORT *dng)
{
  if (dng->wInitStep == 3)
  {
    dng->wInitStep++;
  }

  return 0;
}

static void pcan_dongle_free_irq(struct DONGLE_PORT *dng)
{
  if (dng->wInitStep == 4)
  {
    dng->wInitStep--;
  }
}

// release and probe functions
static int pcan_dongle_cleanup(struct DONGLE_PORT *dng)
{
  DEBUGMSG("%s: pcan_dongle_cleanup()\n", DEVICE_NAME);

  switch(dng->wInitStep)
  {
    case 4: pcan_dongle_free_irq(dng);
    case 3: if (dng->wType == HW_DONGLE_SJA)
              sp_devices--;
	    else
	      epp_devices--;
	    dng_devices = sp_devices + epp_devices;
    case 2:
    case 1:
            parport_unregister_device(dng->pardev);
    case 0: dng->wInitStep = 0;
  }

  return 0;
}

// to switch epp on or restore register
static void setECR(struct DONGLE_PORT *dng)
{
	u16 wEcr = dng->wEcr;

	dng->ucOldECRContent = can_inb(wEcr);
	can_outb((dng->ucOldECRContent & 0x1F) | 0x20, wEcr);

	if (dng->ucOldECRContent == 0xff)
		DEBUGMSG("%s: realy ECP mode configured?\n", DEVICE_NAME);
}

static void restoreECR(struct DONGLE_PORT *dng)
{
  u16 wEcr = dng->wEcr;

  can_outb(dng->ucOldECRContent, wEcr);

  DEBUGMSG("%s: restore ECR\n", DEVICE_NAME);
}

static int pcan_dongle_probe(struct DONGLE_PORT *dng) // probe for type
{
  struct parport *p;

  DEBUGMSG("%s: pcan_dongle_probe() - PARPORT_SUBSYSTEM\n", DEVICE_NAME);
  
  // probe does not probe for the sja1000 device here - this is done at sja1000_open()
  p = parport_find_base(dng->dwPort);
  if (!p)
  {
    DEBUGMSG("found no parport\n");
    return -ENXIO;
  }
  else
  {
       dng->pardev = parport_register_device(p, "can", NULL, NULL, 
                          (PARPORT_IRQ_HANLDER)dng->chip->chipspecops->irq_handler,
			  0, (void *)dng->chip);
	
//    DEBUGMSG("datos IRQ: irq_handler=0x%x p=0x%x dng->chip=0x%x dng->pardev->port->irq=0x%x irq_handler2=0x%x\n",
//		dng->chip->chipspecops->irq_handler,
//		p,dng->chip,dng->pardev->port->irq, &sja1000p_irq_handler);
				      
    if (!dng->pardev)
    {
      DEBUGMSG("found no parport device\n");
      return -ENODEV;
    }

  }
  
  return 0;
}

// interface depended open and close
static int pcan_dongle_open(struct DONGLE_PORT *dng)
{
  int result = 0;
  u16 wPort;
  
  DEBUGMSG("%s: pcan_dongle_open()\n", DEVICE_NAME);
  
  result = parport_claim(dng->pardev);
  
  if (!result)
  {
    if (dng->pardev->port->irq == PARPORT_IRQ_NONE)
    {
      DEBUGMSG(KERN_ERR "%s: no irq associated to parport.\n", DEVICE_NAME);
      result = -ENXIO;
    }
  }
  else
   DEBUGMSG(KERN_ERR "%s: can't claim parport.\n", DEVICE_NAME);	
  
  // save port state
  if (!result)
   {
      wPort    = (u16)dng->dwPort;
	  
     // save old port contents
     dng->ucOldDataContent     = can_inb(wPort);
     dng->ucOldControlContent  = can_inb(wPort + 2);
	  
     // switch to epp mode if possible
     if (dng->wType == HW_DONGLE_SJA_EPP)
        setECR(dng); 
  
    // enable irqs
    _parport_enable_irq(dng); // parport_enable_irq(dng->pardev->port); not working since 2.4.18
  }	
	
  return result;
}

static int pcan_dongle_release(struct DONGLE_PORT *dng)
{
  u16 wPort = (u16)dng->dwPort;

  DEBUGMSG("%s: pcan_dongle_release()\n", DEVICE_NAME);
  
  // disable irqs
  _parport_disable_irq(dng); // parport_disable_irq(dng->pardev->port); not working since 2.4.18

  if (dng->wType == HW_DONGLE_SJA_EPP)
    restoreECR(dng);
    
  // restore port state
  can_outb(dng->ucOldDataContent, wPort);
  can_outb(dng->ucOldControlContent, wPort + 2);
      
  parport_release(dng->pardev);
  
  return 0;
}

int  pcan_dongle_init(struct DONGLE_PORT *dng, u32 dwPort, u16 wIrq, char *type)
{
  int err;
  
  DEBUGMSG("%s: pcan_dongle_init(), dng_devices = %d\n", DEVICE_NAME, dng_devices);
  
  dng->type = type;

  dng->wType = (!strncmp(dongle_type, "sp", 4)) ? HW_DONGLE_SJA : HW_DONGLE_SJA_EPP;
  
  // set this before any instructions, fill struct pcandev, part 1 
  dng->wInitStep   = 0;  
        
  // fill struct pcandev, 1st check if a default is set
  if (!dwPort)
  {
    // there's no default available
    if (dng_devices >= DNG_DEFAULT_COUNT)
      return -ENODEV;
    
    dng->dwPort = dng_ports[dng_devices];
  }
  else
    dng->dwPort = dwPort;
  
  if (!wIrq)
  {
    if (dng_devices >= DNG_DEFAULT_COUNT)
      return -ENODEV;
    
    dng->wIrq   = dng_irqs[dng_devices];    
  }
  else
    dng->wIrq   = wIrq;    
  
  if (dng->wType == HW_DONGLE_SJA)
    {
	   dng->nMinor        = PCAN_DNG_SP_MINOR_BASE + sp_devices; 
    	   dng->wEcr = 0; // set to anything
    }
  else
    {
	    dng->nMinor        = PCAN_DNG_EPP_MINOR_BASE + epp_devices;  
    	    dng->wEcr = (u16)dng->dwPort + 0x402;
    }
	  
	// is the device really available?		
  if ((err = pcan_dongle_probe(dng)) < 0)
    return err;
  
  if (dng->wType == HW_DONGLE_SJA)
    sp_devices++;
  else
    epp_devices++;
	  
  dng_devices = sp_devices + epp_devices;
  
  dng->wInitStep = 3;

  DEBUGMSG(KERN_INFO "%s: %s device minor %d prepared (io=0x%04x,irq=%d)\n", DEVICE_NAME, 
                               dng->type, dng->nMinor, dng->dwPort, dng->wIrq);
	
  return 0;
}




/**
 * template_request_io: - reserve io or memory range for can board
 * @candev: pointer to candevice/board which asks for io. Field @io_addr
 *	of @candev is used in most cases to define start of the range
 *
 * The function template_request_io() is used to reserve the io-memory. If your
 * hardware uses a dedicated memory range as hardware control registers you
 * will have to add the code to reserve this memory as well. 
 * %IO_RANGE is the io-memory range that gets reserved, please adjust according
 * your hardware. Example: #define IO_RANGE 0x100 for i82527 chips or
 * #define IO_RANGE 0x20 for sja1000 chips in basic CAN mode.
 * Return Value: The function returns zero on success or %-ENODEV on failure
 * File: src/template.c
 */
int pcan_dongle_request_io(struct candevice_t *candev)
{
        int res_init;

	dongle_port.chip = candev->chip[0];

	res_init = pcan_dongle_init(&dongle_port, 0, 0, dongle_type);

	return res_init;
}

/**
 * template_elease_io - free reserved io memory range
 * @candev: pointer to candevice/board which releases io
 *
 * The function template_release_io() is used to free reserved io-memory.
 * In case you have reserved more io memory, don't forget to free it here.
 * IO_RANGE is the io-memory range that gets released, please adjust according
 * your hardware. Example: #define IO_RANGE 0x100 for i82527 chips or
 * #define IO_RANGE 0x20 for sja1000 chips in basic CAN mode.
 * Return Value: The function always returns zero
 * File: src/template.c
 */
int pcan_dongle_release_io(struct candevice_t *candev)
{
	/* release I/O port */
	pcan_dongle_release(&dongle_port);
	
	pcan_dongle_cleanup(&dongle_port);

	return 0;
}

/**
 * template_reset - hardware reset routine
 * @candev: Pointer to candevice/board structure
 *
 * The function template_reset() is used to give a hardware reset. This is 
 * rather hardware specific so I haven't included example code. Don't forget to 
 * check the reset status of the chip before returning.
 * Return Value: The function returns zero on success or %-ENODEV on failure
 * File: src/template.c
 */
int pcan_dongle_reset(struct candevice_t *candev)
{
	int i=0;
	struct canchip_t *chip;
	int chipnr;
	unsigned cdr;
	
	DEBUGMSG("Resetting pcan_dongle hardware ...\n");
	for(chipnr=0;chipnr<candev->nr_sja1000_chips;chipnr++) {
		chip=candev->chip[chipnr];

	  pcan_dongle_write_register(sjaMOD_RM, chip->chip_base_addr+SJAMOD);
	  udelay(1000);
	
	  cdr=pcan_dongle_read_register(chip->chip_base_addr+SJACDR);
	  pcan_dongle_write_register(cdr|sjaCDR_PELICAN, chip->chip_base_addr+SJACDR);

	  pcan_dongle_write_register(0, chip->chip_base_addr+SJAIER);

	  i=20;
	  pcan_dongle_write_register(0, chip->chip_base_addr+SJAMOD);
	  while (pcan_dongle_read_register(chip->chip_base_addr+SJAMOD)&sjaMOD_RM) {
		if(!i--) {
			CANMSG("Reset status timeout!\n");
			CANMSG("Please check your hardware.\n");
			return -ENODEV;
		}
		udelay(1000);
		pcan_dongle_write_register(0, chip->chip_base_addr+SJAMOD);
	  }

	  cdr = pcan_dongle_read_register(chip->chip_base_addr+SJACDR);
	  pcan_dongle_write_register(cdr|sjaCDR_PELICAN, chip->chip_base_addr+SJACDR);

	  pcan_dongle_write_register(0, chip->chip_base_addr+SJAIER);
	}
	return 0;
}

#define RESET_ADDR 0x0
#define NR_82527 0
#define NR_SJA1000 1

/**
 * template_init_hw_data - Initialize hardware cards
 * @candev: Pointer to candevice/board structure
 *
 * The function template_init_hw_data() is used to initialize the hardware
 * structure containing information about the installed CAN-board.
 * %RESET_ADDR represents the io-address of the hardware reset register.
 * %NR_82527 represents the number of intel 82527 chips on the board.
 * %NR_SJA1000 represents the number of philips sja1000 chips on the board.
 * The flags entry can currently only be %CANDEV_PROGRAMMABLE_IRQ to indicate that
 * the hardware uses programmable interrupts.
 * Return Value: The function always returns zero
 * File: src/template.c
 */
int pcan_dongle_init_hw_data(struct candevice_t *candev) 
{
	candev->res_addr=RESET_ADDR;
	candev->nr_82527_chips=NR_82527;
	candev->nr_sja1000_chips=NR_SJA1000;
	candev->nr_all_chips=NR_82527+NR_SJA1000;
//	candev->flags &= ~CANDEV_PROGRAMMABLE_IRQ;
	candev->flags |= CANDEV_PROGRAMMABLE_IRQ;

	return 0;
}

#define CHIP_TYPE "sja1000p"
/**
 * template_init_chip_data - Initialize chips
 * @candev: Pointer to candevice/board structure
 * @chipnr: Number of the CAN chip on the hardware card
 *
 * The function template_init_chip_data() is used to initialize the hardware
 * structure containing information about the CAN chips.
 * %CHIP_TYPE represents the type of CAN chip. %CHIP_TYPE can be "i82527" or
 * "sja1000".
 * The @chip_base_addr entry represents the start of the 'official' memory map
 * of the installed chip. It's likely that this is the same as the @io_addr
 * argument supplied at module loading time.
 * The @clock entry holds the chip clock value in Hz.
 * The entry @sja_cdr_reg holds hardware specific options for the Clock Divider
 * register. Options defined in the %sja1000.h file:
 * %CDR_CLKOUT_MASK, %CDR_CLK_OFF, %CDR_RXINPEN, %CDR_CBP, %CDR_PELICAN
 * The entry @sja_ocr_reg holds hardware specific options for the Output Control
 * register. Options defined in the %sja1000.h file:
 * %OCR_MODE_BIPHASE, %OCR_MODE_TEST, %OCR_MODE_NORMAL, %OCR_MODE_CLOCK,
 * %OCR_TX0_LH, %OCR_TX1_ZZ.
 * The entry @int_clk_reg holds hardware specific options for the Clock Out
 * register. Options defined in the %i82527.h file:
 * %iCLK_CD0, %iCLK_CD1, %iCLK_CD2, %iCLK_CD3, %iCLK_SL0, %iCLK_SL1.
 * The entry @int_bus_reg holds hardware specific options for the Bus 
 * Configuration register. Options defined in the %i82527.h file:
 * %iBUS_DR0, %iBUS_DR1, %iBUS_DT1, %iBUS_POL, %iBUS_CBY.
 * Return Value: The function always returns zero
 * File: src/template.c
 */
int pcan_dongle_init_chip_data(struct candevice_t *candev, int chipnr)
{
//    if (chipnr == 0)
     {
	/* initialize common routines for the SJA1000 chip */
	sja1000p_fill_chipspecops(candev->chip[chipnr]);


	candev->chip[chipnr]->chip_type=CHIP_TYPE;
	candev->chip[chipnr]->chip_base_addr=can_ioport2ioptr(candev->io_addr);
	candev->chip[chipnr]->clock = 16000000;
	candev->chip[chipnr]->int_clk_reg = 0x0;
	candev->chip[chipnr]->int_bus_reg = 0x0;
	candev->chip[chipnr]->sja_cdr_reg = sjaCDR_CBP | sjaCDR_CLK_OFF;
	candev->chip[chipnr]->sja_ocr_reg = sjaOCR_MODE_NORMAL | sjaOCR_TX0_LH;

	candev->chip[chipnr]->flags |= CHIP_IRQ_CUSTOM;  // I don't want setup call request_irq 
				      // I'm going to do it through parport_register_device 

     }

    return 0;
}

/**
 * template_init_obj_data - Initialize message buffers
 * @chip: Pointer to chip specific structure
 * @objnr: Number of the message buffer
 *
 * The function template_init_obj_data() is used to initialize the hardware
 * structure containing information about the different message objects on the
 * CAN chip. In case of the sja1000 there's only one message object but on the
 * i82527 chip there are 15.
 * The code below is for a i82527 chip and initializes the object base addresses
 * The entry @obj_base_addr represents the first memory address of the message 
 * object. In case of the sja1000 @obj_base_addr is taken the same as the chips
 * base address.
 * Unless the hardware uses a segmented memory map, flags can be set zero.
 * Return Value: The function always returns zero
 * File: src/template.c
 */
int pcan_dongle_init_obj_data(struct canchip_t *chip, int objnr)
{
	chip->msgobj[objnr]->obj_base_addr=chip->chip_base_addr;
	chip->msgobj[objnr]->obj_flags=0;
	
	return 0;
}

/**
 * template_program_irq - program interrupts
 * @candev: Pointer to candevice/board structure
 *
 * The function template_program_irq() is used for hardware that uses 
 * programmable interrupts. If your hardware doesn't use programmable interrupts
 * you should not set the @candevices_t->flags entry to %CANDEV_PROGRAMMABLE_IRQ and 
 * leave this function unedited. Again this function is hardware specific so 
 * there's no example code.
 * Return value: The function returns zero on success or %-ENODEV on failure
 * File: src/template.c
 */
int pcan_dongle_program_irq(struct candevice_t *candev)
{
	int ret_open;

	pcan_dongle_req_irq(&dongle_port);
	ret_open = pcan_dongle_open(&dongle_port);

	return ret_open;
}


/**
 * template_write_register - Low level write register routine
 * @data: data to be written
 * @address: memory address to write to
 *
 * The function template_write_register() is used to write to hardware registers
 * on the CAN chip. You should only have to edit this function if your hardware
 * uses some specific write process.
 * Return Value: The function does not return a value
 * File: src/template.c
 */
void pcan_dongle_write_register(unsigned data, can_ioptr_t address)
{
   address -= dongle_port.chip->chip_base_addr;  // it's in mutiplexed mode

   pcan_dongle_writereg(&dongle_port, (u8) address, (u8) data); // write a register

//   DEBUGMSG("Write Reg at: 0x%lx data 0x%x \n", address, (unsigned) data);
}

/**
 * template_read_register - Low level read register routine
 * @address: memory address to read from
 *
 * The function template_read_register() is used to read from hardware registers
 * on the CAN chip. You should only have to edit this function if your hardware
 * uses some specific read process.
 * Return Value: The function returns the value stored in @address
 * File: src/template.c
 */
unsigned pcan_dongle_read_register(can_ioptr_t address)
{
   u8 val;

  address -= dongle_port.chip->chip_base_addr;  // it's in mutiplexed mode
	
  if (dongle_port.wType == HW_DONGLE_SJA)
     val = pcan_dongle_sp_readreg(&dongle_port, (u8) address); // functions for SP port
  else 
     val = pcan_dongle_epp_readreg(&dongle_port, (u8) address); // functions for EPP port

//  DEBUGMSG("Read Reg at: 0x%lx data 0x%x \n", address, val);

  return ((unsigned)val);
}

/* !!! Don't change this function !!! */
int pcan_dongle_register(struct hwspecops_t *hwspecops)
{
	hwspecops->request_io = pcan_dongle_request_io;
	hwspecops->release_io = pcan_dongle_release_io;
	hwspecops->reset = pcan_dongle_reset;
	hwspecops->init_hw_data = pcan_dongle_init_hw_data;
	hwspecops->init_chip_data = pcan_dongle_init_chip_data;
	hwspecops->init_obj_data = pcan_dongle_init_obj_data;
	hwspecops->write_register = pcan_dongle_write_register;
	hwspecops->read_register = pcan_dongle_read_register;
	hwspecops->program_irq = pcan_dongle_program_irq;
	return 0;
}


