#ifndef CYGONCE_SMDK2410_H
#define CYGONCE_SMDK2410_H

//=============================================================================
//
//      s3c2410x.h
//
//      Platform specific support (register layout, etc)
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    michael anburaj <michaelanburaj@hotmail.com>
// Contributors: michael anburaj <michaelanburaj@hotmail.com>
// Date:         2003-08-01
// Purpose:      SMDK2410 platform specific support definitions
// Description: 
// Usage:        #include <cyg/hal/s3c2410x.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal_arm_arm9_smdk2410.h>

// Memory layout details needed by conversion macro
#define SMDK2410_SDRAM_PHYS_BASE         0x30000000
#define SMDK2410_SDRAM_VIRT_BASE         0x00000000

#define SMDK2410_FLASH_PHYS_BASE         0x00000000
#define SMDK2410_FLASH_VIRT_BASE         0x80000000


// Internal clocks
#define FCLK CYGNUM_HAL_ARM_SMDK2410_CPU_CLOCK
#define HCLK CYGNUM_HAL_ARM_SMDK2410_BUS_CLOCK
#define PCLK CYGNUM_HAL_ARM_SMDK2410_PERIPHERAL_CLOCK
#define UCLK 48000000


// Memory control 
#define BWSCON    0x48000000  //Bus width & wait status
#define BANKCON0  0x48000004  //Boot ROM control
#define BANKCON1  0x48000008  //BANK1 control
#define BANKCON2  0x4800000c  //BANK2 cControl
#define BANKCON3  0x48000010  //BANK3 control
#define BANKCON4  0x48000014  //BANK4 control
#define BANKCON5  0x48000018  //BANK5 control
#define BANKCON6  0x4800001c  //BANK6 control
#define BANKCON7  0x48000020  //BANK7 control
#define REFRESH   0x48000024  //DRAM/SDRAM refresh
#define BANKSIZE  0x48000028  //Flexible Bank Size
#define MRSRB6    0x4800002c  //Mode register set for SDRAM
#define MRSRB7    0x48000030  //Mode register set for SDRAM


// USB Host


// INTERRUPT
#define SRCPND     0x4a000000  //Interrupt request status
#define INTMOD     0x4a000004  //Interrupt mode control
#define INTMSK     0x4a000008  //Interrupt mask control
#define PRIORITY   0x4a00000a  //IRQ priority control
#define INTPND     0x4a000010  //Interrupt request status

// PENDING BIT
#define BIT_EINT0      (0x1)
#define BIT_EINT1      (0x1<<1)
#define BIT_EINT2      (0x1<<2)
#define BIT_EINT3      (0x1<<3)
#define BIT_EINT4_7    (0x1<<4)
#define BIT_EINT8_23   (0x1<<5)
#define BIT_NOTUSED6   (0x1<<6)
#define BIT_BAT_FLT    (0x1<<7)
#define BIT_TICK       (0x1<<8)
#define BIT_WDT        (0x1<<9)
#define BIT_TIMER0     (0x1<<10)
#define BIT_TIMER1     (0x1<<11)
#define BIT_TIMER2     (0x1<<12)
#define BIT_TIMER3     (0x1<<13)
#define BIT_TIMER4     (0x1<<14)
#define BIT_UART2      (0x1<<15)
#define BIT_LCD        (0x1<<16)
#define BIT_DMA0       (0x1<<17)
#define BIT_DMA1       (0x1<<18)
#define BIT_DMA2       (0x1<<19)
#define BIT_DMA3       (0x1<<20)
#define BIT_SDI        (0x1<<21)
#define BIT_SPI0       (0x1<<22)
#define BIT_UART1      (0x1<<23)
#define BIT_NOTUSED24  (0x1<<24)
#define BIT_USBD       (0x1<<25)
#define BIT_USBH       (0x1<<26)
#define BIT_IIC        (0x1<<27)
#define BIT_UART0      (0x1<<28)
#define BIT_SPI1       (0x1<<29)
#define BIT_RTCC       (0x1<<30)
#define BIT_ADC        (0x1<<31)
#define BIT_ALLMSK     (0xffffffff)

#define INTOFFSET  0x4a000014  //Interruot request source offset
#define SUBSRCPND  0x4a000018  //Sub source pending
#define INTSUBMSK  0x4a00001c  //Interrupt sub mask

#define BIT_SUB_ALLMSK (0x7ff)
#define BIT_SUB_ADC    (0x1<<10)
#define BIT_SUB_TC     (0x1<<9)
#define BIT_SUB_ERR2   (0x1<<8)
#define BIT_SUB_TXD2   (0x1<<7)
#define BIT_SUB_RXD2   (0x1<<6)
#define BIT_SUB_ERR1   (0x1<<5)
#define BIT_SUB_TXD1   (0x1<<4)
#define BIT_SUB_RXD1   (0x1<<3)
#define BIT_SUB_ERR0   (0x1<<2)
#define BIT_SUB_TXD0   (0x1<<1)
#define BIT_SUB_RXD0   (0x1<<0)


// DMA
#define DISRC0     0x4b000000  //DMA 0 Initial source
#define DISRCC0    0x4b000004  //DMA 0 Initial source control
#define DIDST0     0x4b000008  //DMA 0 Initial Destination
#define DIDSTC0    0x4b00000c  //DMA 0 Initial Destination control
#define DCON0      0x4b000010  //DMA 0 Control
#define DSTAT0     0x4b000014  //DMA 0 Status
#define DCSRC0     0x4b000018  //DMA 0 Current source
#define DCDST0     0x4b00001c  //DMA 0 Current destination
#define DMASKTRIG0 0x4b000020  //DMA 0 Mask trigger

#define DISRC1     0x4b000040  //DMA 1 Initial source
#define DISRCC1    0x4b000044  //DMA 1 Initial source control
#define DIDST1     0x4b000048  //DMA 1 Initial Destination
#define DIDSTC1    0x4b00004c  //DMA 1 Initial Destination control
#define DCON1      0x4b000050  //DMA 1 Control
#define DSTAT1     0x4b000054  //DMA 1 Status
#define DCSRC1     0x4b000058  //DMA 1 Current source
#define DCDST1     0x4b00005c  //DMA 1 Current destination
#define DMASKTRIG1 0x4b000060  //DMA 1 Mask trigger

#define DISRC2     0x4b000080  //DMA 2 Initial source
#define DISRCC2    0x4b000084  //DMA 2 Initial source control
#define DIDST2     0x4b000088  //DMA 2 Initial Destination
#define DIDSTC2    0x4b00008c  //DMA 2 Initial Destination control
#define DCON2      0x4b000090  //DMA 2 Control
#define DSTAT2     0x4b000094  //DMA 2 Status
#define DCSRC2     0x4b000098  //DMA 2 Current source
#define DCDST2     0x4b00009c  //DMA 2 Current destination
#define DMASKTRIG2 0x4b0000a0  //DMA 2 Mask trigger

#define DISRC3     0x4b0000c0  //DMA 3 Initial source
#define DISRCC3    0x4b0000c4  //DMA 3 Initial source control
#define DIDST3     0x4b0000c8  //DMA 3 Initial Destination
#define DIDSTC3    0x4b0000cc  //DMA 3 Initial Destination control
#define DCON3      0x4b0000d0  //DMA 3 Control
#define DSTAT3     0x4b0000d4  //DMA 3 Status
#define DCSRC3     0x4b0000d8  //DMA 3 Current source
#define DCDST3     0x4b0000dc  //DMA 3 Current destination
#define DMASKTRIG3 0x4b0000e0  //DMA 3 Mask trigger


// CLOCK & POWER MANAGEMENT
#define LOCKTIME   0x4c000000  //PLL lock time counter
#define MPLLCON    0x4c000004  //MPLL Control
#define UPLLCON    0x4c000008  //UPLL Control
#define CLKCON     0x4c00000c  //Clock generator control
#define CLKSLOW    0x4c000010  //Slow clock control
#define CLKDIVN    0x4c000014  //Clock divider control


// LCD CONTROLLER
#define LCDCON1    0x4d000000  //LCD control 1
#define LCDCON2    0x4d000004  //LCD control 2
#define LCDCON3    0x4d000008  //LCD control 3
#define LCDCON4    0x4d00000c  //LCD control 4
#define LCDCON5    0x4d000010  //LCD control 5
#define LCDSADDR1  0x4d000014  //STN/TFT Frame buffer start address 1
#define LCDSADDR2  0x4d000018  //STN/TFT Frame buffer start address 2
#define LCDSADDR3  0x4d00001c  //STN/TFT Virtual screen address set
#define REDLUT     0x4d000020  //STN Red lookup table
#define GREENLUT   0x4d000024  //STN Green lookup table 
#define BLUELUT    0x4d000028  //STN Blue lookup table
#define DITHMODE   0x4d00004c  //STN Dithering mode
#define TPAL       0x4d000050  //TFT Temporary palette
#define LCDINTPND  0x4d000054  //LCD Interrupt pending
#define LCDSRCPND  0x4d000058  //LCD Interrupt source
#define LCDINTMSK  0x4d00005c  //LCD Interrupt mask
#define LPCSEL     0x4d000060  //LPC3600 Control
#define PALETTE    0x4d000400  //Palette start address


// NAND flash
#define NFCONF     0x4e000000  //NAND Flash configuration
#define NFCMD      0x4e000004  //NADD Flash command
#define NFADDR     0x4e000008  //NAND Flash address
#define NFDATA     0x4e00000c  //NAND Flash data
#define NFSTAT     0x4e000010  //NAND Flash operation status
#define NFECC      0x4e000014  //NAND Flash ECC
#define NFECC0     0x4e000014
#define NFECC1     0x4e000015
#define NFECC2     0x4e000016


// UART
#define ULCON0     0x50000000  //UART 0 Line control
#define UCON0      0x50000004  //UART 0 Control
#define UFCON0     0x50000008  //UART 0 FIFO control
#define UMCON0     0x5000000c  //UART 0 Modem control
#define UTRSTAT0   0x50000010  //UART 0 Tx/Rx status
#define UERSTAT0   0x50000014  //UART 0 Rx error status
#define UFSTAT0    0x50000018  //UART 0 FIFO status
#define UMSTAT0    0x5000001c  //UART 0 Modem status
#define UBRDIV0    0x50000028  //UART 0 Baud rate divisor

#define ULCON1     0x50004000  //UART 1 Line control
#define UCON1      0x50004004  //UART 1 Control
#define UFCON1     0x50004008  //UART 1 FIFO control
#define UMCON1     0x5000400c  //UART 1 Modem control
#define UTRSTAT1   0x50004010  //UART 1 Tx/Rx status
#define UERSTAT1   0x50004014  //UART 1 Rx error status
#define UFSTAT1    0x50004018  //UART 1 FIFO status
#define UMSTAT1    0x5000401c  //UART 1 Modem status
#define UBRDIV1    0x50004028  //UART 1 Baud rate divisor

#define ULCON2     0x50008000  //UART 2 Line control
#define UCON2      0x50008004  //UART 2 Control
#define UFCON2     0x50008008  //UART 2 FIFO control
#define UMCON2     0x5000800c  //UART 2 Modem control
#define UTRSTAT2   0x50008010  //UART 2 Tx/Rx status
#define UERSTAT2   0x50008014  //UART 2 Rx error status
#define UFSTAT2    0x50008018  //UART 2 FIFO status
#define UMSTAT2    0x5000801c  //UART 2 Modem status
#define UBRDIV2    0x50008028  //UART 2 Baud rate divisor

#define UTXH0 0x50000020  //UART 0 Transmission Hold
#define URXH0 0x50000024  //UART 0 Receive buffer
#define UTXH1 0x50004020  //UART 1 Transmission Hold
#define URXH1 0x50004024  //UART 1 Receive buffer
#define UTXH2 0x50008020  //UART 2 Transmission Hold
#define URXH2 0x50008024  //UART 2 Receive buffer

#define OFS_ULCON     (ULCON0-ULCON0)  //UART Line control
#define OFS_UCON      (UCON0-ULCON0)   //UART Control
#define OFS_UFCON     (UFCON0-ULCON0)  //UART FIFO control
#define OFS_UMCON     (UMCON0-ULCON0)  //UART Modem control
#define OFS_UTRSTAT   (UTRSTAT0-ULCON0)//UART Tx/Rx status
#define OFS_UERSTAT   (UERSTAT0-ULCON0)//UART Rx error status
#define OFS_UFSTAT    (UFSTAT0-ULCON0) //UART FIFO status
#define OFS_UMSTAT    (UMSTAT0-ULCON0) //UART Modem status
#define OFS_UBRDIV    (UBRDIV0-ULCON0) //UART Baud rate divisor
#define OFS_UTXH      (UTXH0-ULCON0)   //UART Transmission Hold
#define OFS_URXH      (URXH0-ULCON0)   //UART Receive buffer

// ULCON bits
#define SHF_ULCON_WL       0
#define MSK_ULCON_WL       (0x3<<SHF_ULCON_WL)
#define VAL_ULCON_WL_5     (0x0<<SHF_ULCON_WL)
#define VAL_ULCON_WL_6     (0x1<<SHF_ULCON_WL)
#define VAL_ULCON_WL_7     (0x2<<SHF_ULCON_WL)
#define VAL_ULCON_WL_8     (0x3<<SHF_ULCON_WL)

#define SHF_ULCON_SB       2
#define MSK_ULCON_SB       (0x1<<SHF_ULCON_SB)
#define VAL_ULCON_SB_1     (0x0<<SHF_ULCON_SB)
#define VAL_ULCON_SB_2     (0x1<<SHF_ULCON_SB)

#define SHF_ULCON_PM       3
#define MSK_ULCON_PM       (0x7<<SHF_ULCON_PM)
#define VAL_ULCON_PM_N     (0x0<<SHF_ULCON_PM)
#define VAL_ULCON_PM_O     (0x4<<SHF_ULCON_PM)
#define VAL_ULCON_PM_E     (0x5<<SHF_ULCON_PM)
#define VAL_ULCON_PM_FC1   (0x6<<SHF_ULCON_PM)
#define VAL_ULCON_PM_FC0   (0x7<<SHF_ULCON_PM)

#define SHF_ULCON_IRM      6
#define MSK_ULCON_IRM      (0x1<<SHF_ULCON_IRM)
#define VAL_ULCON_IRM_N    (0x0<<SHF_ULCON_IRM)
#define VAL_ULCON_IRM_IR   (0x1<<SHF_ULCON_IRM)


// PWM TIMER
#define TCFG0  0x51000000  //Timer 0 configuration
#define TCFG1  0x51000004  //Timer 1 configuration
#define TCON   0x51000008  //Timer control
#define TCNTB0 0x5100000c  //Timer count buffer 0
#define TCMPB0 0x51000010  //Timer compare buffer 0
#define TCNTO0 0x51000014  //Timer count observation 0
#define TCNTB1 0x51000018  //Timer count buffer 1
#define TCMPB1 0x5100001c  //Timer compare buffer 1
#define TCNTO1 0x51000020  //Timer count observation 1
#define TCNTB2 0x51000024  //Timer count buffer 2
#define TCMPB2 0x51000028  //Timer compare buffer 2
#define TCNTO2 0x5100002c  //Timer count observation 2
#define TCNTB3 0x51000030  //Timer count buffer 3
#define TCMPB3 0x51000034  //Timer compare buffer 3
#define TCNTO3 0x51000038  //Timer count observation 3
#define TCNTB4 0x5100003c  //Timer count buffer 4
#define TCNTO4 0x51000040  //Timer count observation 4


// USB DEVICE
#define FUNC_ADDR_REG     0x52000140  //Function address
#define PWR_REG           0x52000144  //Power management
#define EP_INT_REG        0x52000148  //EP Interrupt pending and clear
#define USB_INT_REG       0x52000158  //USB Interrupt pending and clear
#define EP_INT_EN_REG     0x5200015c  //Interrupt enable
#define USB_INT_EN_REG    0x5200016c
#define FRAME_NUM1_REG    0x52000170  //Frame number lower byte
#define FRAME_NUM2_REG    0x52000174  //Frame number higher byte
#define INDEX_REG         0x52000178  //Register index
#define MAXP_REG          0x52000180  //Endpoint max packet
#define EP0_CSR           0x52000184  //Endpoint 0 status
#define IN_CSR1_REG       0x52000184  //In endpoint control status
#define IN_CSR2_REG       0x52000188
#define OUT_CSR1_REG      0x52000190  //Out endpoint control status
#define OUT_CSR2_REG      0x52000194
#define OUT_FIFO_CNT1_REG 0x52000198  //Endpoint out write count
#define OUT_FIFO_CNT2_REG 0x5200019c
#define EP0_FIFO          0x520001c0  //Endpoint 0 FIFO
#define EP1_FIFO          0x520001c4  //Endpoint 1 FIFO
#define EP2_FIFO          0x520001c8  //Endpoint 2 FIFO
#define EP3_FIFO          0x520001cc  //Endpoint 3 FIFO
#define EP4_FIFO          0x520001d0  //Endpoint 4 FIFO
#define EP1_DMA_CON       0x52000200  //EP1 DMA interface control
#define EP1_DMA_UNIT      0x52000204  //EP1 DMA Tx unit counter
#define EP1_DMA_FIFO      0x52000208  //EP1 DMA Tx FIFO counter
#define EP1_DMA_TTC_L     0x5200020c  //EP1 DMA total Tx counter
#define EP1_DMA_TTC_M     0x52000210
#define EP1_DMA_TTC_H     0x52000214
#define EP2_DMA_CON       0x52000218  //EP2 DMA interface control
#define EP2_DMA_UNIT      0x5200021c  //EP2 DMA Tx unit counter
#define EP2_DMA_FIFO      0x52000220  //EP2 DMA Tx FIFO counter
#define EP2_DMA_TTC_L     0x52000224  //EP2 DMA total Tx counter
#define EP2_DMA_TTC_M     0x52000228
#define EP2_DMA_TTC_H     0x5200022c
#define EP3_DMA_CON       0x52000240  //EP3 DMA interface control
#define EP3_DMA_UNIT      0x52000244  //EP3 DMA Tx unit counter
#define EP3_DMA_FIFO      0x52000248  //EP3 DMA Tx FIFO counter
#define EP3_DMA_TTC_L     0x5200024c  //EP3 DMA total Tx counter
#define EP3_DMA_TTC_M     0x52000250
#define EP3_DMA_TTC_H     0x52000254
#define EP4_DMA_CON       0x52000258  //EP4 DMA interface control
#define EP4_DMA_UNIT      0x5200025c  //EP4 DMA Tx unit counter
#define EP4_DMA_FIFO      0x52000260  //EP4 DMA Tx FIFO counter
#define EP4_DMA_TTC_L     0x52000264  //EP4 DMA total Tx counter
#define EP4_DMA_TTC_M     0x52000268
#define EP4_DMA_TTC_H     0x5200026c


// WATCH DOG TIMER
#define WTCON   0x53000000  //Watch-dog timer mode
#define WTDAT   0x53000004  //Watch-dog timer data
#define WTCNT   0x53000008  //Eatch-dog timer count


// IIC
#define IICCON  0x54000000  //IIC control
#define IICSTAT 0x54000004  //IIC status
#define IICADD  0x54000008  //IIC address
#define IICDS   0x5400000c  //IIC data shift


// IIS
#define IISCON  0x55000000  //IIS Control
#define IISMOD  0x55000004  //IIS Mode
#define IISPSR  0x55000008  //IIS Prescaler
#define IISFCON 0x5500000c  //IIS FIFO control

#define IISFIFO 0x55000010  //IIS FIFO entry


// I/O PORT 
#define GPACON    0x56000000  //Port A control
#define GPADAT    0x56000004  //Port A data
                        
#define GPBCON    0x56000010  //Port B control
#define GPBDAT    0x56000014  //Port B data
#define GPBUP     0x56000018  //Pull-up control B
                        
#define GPCCON    0x56000020  //Port C control
#define GPCDAT    0x56000024  //Port C data
#define GPCUP     0x56000028  //Pull-up control C
                        
#define GPDCON    0x56000030  //Port D control
#define GPDDAT    0x56000034  //Port D data
#define GPDUP     0x56000038  //Pull-up control D
                        
#define GPECON    0x56000040  //Port E control
#define GPEDAT    0x56000044  //Port E data
#define GPEUP     0x56000048  //Pull-up control E
                        
#define GPFCON    0x56000050  //Port F control
#define GPFDAT    0x56000054  //Port F data
#define GPFUP     0x56000058  //Pull-up control F
                        
#define GPGCON    0x56000060  //Port G control
#define GPGDAT    0x56000064  //Port G data
#define GPGUP     0x56000068  //Pull-up control G
                        
#define GPHCON    0x56000070  //Port H control
#define GPHDAT    0x56000074  //Port H data
#define GPHUP     0x56000078  //Pull-up control H
                        
#define MISCCR    0x56000080  //Miscellaneous control
#define DCLKCON   0x56000084  //DCLK0/1 control
#define EXTINT0   0x56000088  //External interrupt control register 0
#define EXTINT1   0x5600008c  //External interrupt control register 1
#define EXTINT2   0x56000090  //External interrupt control register 2
#define EINTFLT0  0x56000094  //Reserved
#define EINTFLT1  0x56000098  //Reserved
#define EINTFLT2  0x5600009c  //External interrupt filter control register 2
#define EINTFLT3  0x560000a0  //External interrupt filter control register 3
#define EINTMASK  0x560000a4  //External interrupt mask
#define EINTPEND  0x560000a8  //External interrupt pending
#define GSTATUS0  0x560000ac  //External pin status
#define GSTATUS1  0x560000b0  //Chip ID(0x32410000)
#define GSTATUS2  0x560000b4  //Reset type
#define GSTATUS3  0x560000b8  //Saved data0(32-bit) before entering POWER_OFF mode 
#define GSTATUS4  0x560000bc  //Saved data0(32-bit) before entering POWER_OFF mode 


// RTC
#define RTCCON    0x57000040  //RTC control
#define TICNT     0x57000044  //Tick time count
#define RTCALM    0x57000050  //RTC alarm control
#define ALMSEC    0x57000054  //Alarm second
#define ALMMIN    0x57000058  //Alarm minute
#define ALMHOUR   0x5700005c  //Alarm Hour
#define ALMDAY    0x57000060  //Alarm day
#define ALMMON    0x57000064  //Alarm month
#define ALMYEAR   0x57000068  //Alarm year
#define RTCRST    0x5700006c  //RTC round reset
#define BCDSEC    0x57000070  //BCD second
#define BCDMIN    0x57000074  //BCD minute
#define BCDHOUR   0x57000078  //BCD hour
#define BCDDAY    0x5700007c  //BCD day
#define BCDDATE   0x57000080  //BCD date
#define BCDMON    0x57000084  //BCD month
#define BCDYEAR   0x57000088  //BCD year


// ADC
#define ADCCON    0x58000000  //ADC control
#define ADCTSC    0x58000004  //ADC touch screen control
#define ADCDLY    0x58000008  //ADC start or Interval Delay
#define ADCDAT0   0x5800000c  //ADC conversion data 0
#define ADCDAT1   0x58000010  //ADC conversion data 1

                        
// SPI          
#define SPCON0    0x59000000  //SPI0 control
#define SPSTA0    0x59000004  //SPI0 status
#define SPPIN0    0x59000008  //SPI0 pin control
#define SPPRE0    0x5900000c  //SPI0 baud rate prescaler
#define SPTDAT0   0x59000010  //SPI0 Tx data
#define SPRDAT0   0x59000014  //SPI0 Rx data

#define SPCON1    0x59000020  //SPI1 control
#define SPSTA1    0x59000024  //SPI1 status
#define SPPIN1    0x59000028  //SPI1 pin control
#define SPPRE1    0x5900002c  //SPI1 baud rate prescaler
#define SPTDAT1   0x59000030  //SPI1 Tx data
#define SPRDAT1   0x59000034  //SPI1 Rx data


// SD Interface
#define SDICON     0x5a000000  //SDI control
#define SDIPRE     0x5a000004  //SDI baud rate prescaler
#define SDICARG    0x5a000008  //SDI command argument
#define SDICCON    0x5a00000c  //SDI command control
#define SDICSTA    0x5a000010  //SDI command status
#define SDIRSP0    0x5a000014  //SDI response 0
#define SDIRSP1    0x5a000018  //SDI response 1
#define SDIRSP2    0x5a00001c  //SDI response 2
#define SDIRSP3    0x5a000020  //SDI response 3
#define SDIDTIMER  0x5a000024  //SDI data/busy timer
#define SDIBSIZE   0x5a000028  //SDI block size
#define SDIDCON    0x5a00002c  //SDI data control
#define SDIDCNT    0x5a000030  //SDI data remain counter
#define SDIDSTA    0x5a000034  //SDI data status
#define SDIFSTA    0x5a000038  //SDI FIFO status
#define SDIIMSK    0x5a000040  //SDI interrupt mask

#define SDIDAT     0x5a00003c  //SDI data


#endif // CYGONCE_SMDK2410_H
//-----------------------------------------------------------------------------
// end of s3c2410x.h
