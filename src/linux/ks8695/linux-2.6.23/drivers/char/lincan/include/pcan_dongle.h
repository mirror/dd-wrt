#ifndef __PCAN_DONGLE_H__
#define __PCAN_DONGLE_H__

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
/****************************************************************************/

/****************************************************************************/
//
// all parts to handle device interface specific parts of pcan-dongle
//
// Revision 1.13  2004/04/11 22:03:29  klaus
// cosmetic changes
//
// Revision 1.12  2003/03/02 10:58:07  klaus
// merged USB thread into main path
//
// Revision 1.11  2003/03/02 10:58:07  klaus
// merged USB thread into main path
//
// Revision 1.10.2.3  2003/01/29 20:34:20  klaus
// release_20030129_a and release_20030129_u released
//
// Revision 1.10.2.2  2003/01/29 20:34:19  klaus
// release_20030129_a and release_20030129_u released
//
// Revision 1.10.2.1  2003/01/28 23:28:26  klaus
// reorderd pcan_usb.c and pcan_usb_kernel.c, tidied up
//
// Revision 1.10  2002/01/30 20:54:27  klaus
// simple source file header change
//
/****************************************************************************/

/****************************************************************************/
// parameter wHardwareType, used by open 
#define HW_ISA             1 // not supported with LINUX, 82C200 chip 
#define HW_DONGLE_SJA      5
#define HW_DONGLE_SJA_EPP  6 
#define HW_DONGLE_PRO      7 // not yet supported with LINUX
#define HW_DONGLE_PRO_EPP  8 // not yet supported with LINUX
#define HW_ISA_SJA         9 // use this also for PC/104
#define HW_PCI		        10 // PCI carries always SJA1000 chips
#define HW_USB            11 // don't know if this is common over peak products


struct DONGLE_PORT
{
  u32  dwPort;                                             // the port of the transport layer
  u16  wIrq;                                               // the associated irq 
  struct pardevice *pardev;                                // points to the associated parallel port (PARPORT subsytem)
  u16  wEcr;                                               // ECR register in case of EPP
  u8   ucOldDataContent;                                   // the overwritten contents of the port registers
  u8   ucOldControlContent;
  u8   ucOldECRContent;


  u16  wInitStep;                                          // device specific init state
  u16  wType;                                              // (number type) to distinguish sp and epp
  int  nMinor;                                             // the associated minor
  char *type;                                              // the literal type of the device, info only

  struct canchip_t *chip;
};


/****************************************************************************/
// DEFINES

int pcan_dongle_request_io(struct candevice_t *candev);
int pcan_dongle_release_io(struct candevice_t *candev);
int pcan_dongle_reset(struct candevice_t *candev); 
int pcan_dongle_init_hw_data(struct candevice_t *candev);
int pcan_dongle_init_chip_data(struct candevice_t *candev, int chipnr);
int pcan_dongle_init_obj_data(struct canchip_t *chip, int objnr);
void pcan_dongle_write_register(unsigned data, can_ioptr_t address);
unsigned pcan_dongle_read_register(can_ioptr_t address);
int pcan_dongle_program_irq(struct candevice_t *candev);

#endif // __PCAN_DONGLE_H__
