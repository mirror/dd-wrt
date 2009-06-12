#ifndef _LINUX_EJTAG_H
#define _LINUX_EJTAG_H

/*
 *   ejtag.h
 *
 *   ejtag primitives
 *
 *   Copyright 2000 padraigo <padraigo@yahoo.com>
 */

int ejtag_switch(unsigned int cmd, unsigned long arg);

/*------------------------------------------------------------------------*/
/*  EJTAG IOCTL COMMANDS    						  */
/*------------------------------------------------------------------------*/
#define EJTAG_TAPRESET       0x0000
#define EJTAG_INIT           0x0001
#define EJTAG_TAPMOVE        0x0002
#define EJTAG_DATA           0x1000
#define EJTAG_INSTR          0x1001
#define EJTAG_IMPLEMENTATION 0x1002
#define EJTAG_VERSION        0x1003
#define EJTAG_CTRL_REG       0x1100
#define EJTAG_ADDR_REG       0x1101
#define EJTAG_DATA_REG       0x1102
#define EJTAG_WRITE_WORD     0x2000
#define EJTAG_READ_WORD      0x2001
#define EJTAG_WRITE_HWORD    0x2010
#define EJTAG_READ_HWORD     0x2011
#define EJTAG_WRITE_BYTE     0x2020
#define EJTAG_READ_BYTE      0x2021
#define EJTAG_CHECKSTATUS    0x3000
#define EJTAG_PORTWRITE      0x3001
#define EJTAG_PORTREAD       0x3002
#define EJTAG_BIGENDIAN      0x3003
#define EJTAG_WRITE_DATAP    0x4000
#define EJTAG_WRITE_CTRLP    0x4001
#define EJTAG_READ_STATP     0x4002

// AJP
#define EJTAG_DEBUG_D3_1     0x5000
#define EJTAG_DEBUG_D3_0     0x5001


#define EJTAG_F(minor)	ejtag_table[(minor)].flags		/* flags for busy, etc. */
#define EJTAG_TIME(minor)	ejtag_table[(minor)].time		/* wait time */
#define EJTAG_WAIT(minor)	ejtag_table[(minor)].wait		/* strobe wait */
#define EJTAG_IRQ(minor)	ejtag_table[(minor)].dev->port->irq /* interrupt # */

#define EJTAG_BUFFER_SIZE 256
#define EJTAG_INIT_CHAR 1000
#define EJTAG_INIT_WAIT 1
#define EJTAG_INIT_TIME 2
#define EJTAG_INIT_DEBUG 0
#define EJTAG_WAIT(minor)	ejtag_table[(minor)].wait		/* strobe wait */

/* flag definitions */
#define EJTAG_EXIST 0x0001 /* ejtag exists for this minor */
#define EJTAG_BIGEND 0x0002 /* ejtag is bigendian for this minor */

/* Magic numbers for defining port-device mappings */
#define EJTAG_PARPORT_UNSPEC -4
#define EJTAG_PARPORT_AUTO -3
#define EJTAG_PARPORT_OFF -2
#define EJTAG_PARPORT_NONE -1
#define EJTAG_BUSY	 0x0004
#define EJTAG_BUSY_BIT_POS 2
#define PROBE_MEM_SIZE 0x1000

/*------------------------------------------------------------------------*/
/*  TARGET JTAG INSTRUCTIONS  						  */
/*------------------------------------------------------------------------*/
#define EXTEST		0x00    //TAP select boundaryscan register
#define IDCODE      	0x01    //TAP select chip ID register
#define SAMPLE_PRELOAD	0x02    //TAP select boundaryscan register
#define IMPCODE		0x03    //TAP select chip implementation code
#define HI_Z		0x05    //TAP Tristate outputs disabled
#define BYPASS		0x07    //TAP Bypass mode (no registers in chain)
#define JTAG_ADDRESS_IR 0x08    //JTAG address register (DMA)
#define JTAG_DATA_IR	0x09    //JTAG data register (DMA)
#define JTAG_CONTROL_IR 0x0A    //JTAG control register (DMA/DSU)
#define JTAG_ALL_IR	0x0B    //JTAG all IR regs. linked together
#define PCTRACE		0x10    //JTAG PC-trace enabled
#define MACRO		0x1E    //JTAG macro (???)

/*------------------------------------------------------------------------*/
/*  TARGET CONTROLREGISTER BITS						  */
/*------------------------------------------------------------------------*/
/* AJP: since DMA often not supported, all DMA bits below undefined
 *      in target. May consider setting to zero so writes use 0.
 */
#define CLKEN		0x00000001  // (N) DCLK enabled
#define TOF		0x00000002  // (N) TOF semaphore
#define TIF		0x00000004  // (N) TIF semaphore
#define BRKST		0x00000008  //Break status
#define DINC		0x00000010  // (N) Auto DMA address increment
#define DLOCK		0x00000020  // (N) DMA lock
#define JTS_BYTE	0x00000000  // (N) DMA tranfser size BYTE
#define JTS_HALFWORD	0x00000080  // (N) DMA transfer size HALFWORD
#define JTS_WORD	0x00000100  // (N) DMA transfer size WORD
#define JTS_TRIPLEBYTE  0x00000180  // (N) DMA transfer size TRIPLEBYTE
#define DRWN		0x00000200  // (N) DMA Read, Write NOT
#define DERR		0x00000400  // (N) PI-Bus error
#define DSTRT		0x00000800  // (N) DMA start/busy
#define JTAGBRK		0x00001000  //Initiated a JTAG break -> Debugex.
#define DEV		0x00004000  //0 Rom 0xbfc00480, 1 EJTAG MEM 0xff200200
#define PROBEN		0x00008000  //Probe enabled/present
#define PRRST		0x00010000  //Processor reset
#define DMAACC		0x00020000  // (N) DMA access, set with every transfer!
#define PRACC		0x00040000  //Processor access, reset by software!
#define PRNW		0x00080000  //Processor access read not, write
#define PERRST		0x00100000  //Peripheral reset, JTAG excluded
#define RUN		0x00200000  //Processor is running
#define DOZE		0x00400000  //Processor is in doze state
#define SYNC		0x00800000  // (?) Trace will sync. with pctrace instr.
#define PCLEN		0x01000000  // (?) Trace addrs. long, !short

/*------------------------------------------------------------------------*/
/*  DMA TRANSFER TYPES (INTERNAL)					  */
/*------------------------------------------------------------------------*/
#define LITTLE_E	0 //Little Endian
#define BIG_E		1 //Big Endian

/*------------------------------------------------------------------------*/
/*  TARGET JTAG IMPLEMENTATION BITS					  */
/*------------------------------------------------------------------------*/
#define MIPS64		0x00000001                 //ID MiPS64, !MiPS32
#define CH(dword)	(dword>>1)&0x000000F       //# of brk. channels (old)
#define INSTBRK		0x00000020                 //Imp. Instr. Addrs. break
#define DATABRK		0x00000040                 //Imp. Data Addrs. break
#define PROCBRK		0x00000080                 //Imp. Proc. Addrs. break
#define PCSTW(dword)	1+((dword)>>8&0x0000007)   //# of PCST channels
#define TPCW(dword)	1+((dword)>>11&0x0000007)  //# of TPC channels
#define NODMA		0x00004000                 //!Imp. JTAG DMA transfer
#define NOPCTRACE       0x00008000                 //!Imp. PCTRACE
#define MIPS16		0x00010000                 //Imp. MiPS16 support
#define ICACHEC		0x00020000                 //Imp. Instr. cache coh.
#define DCACHEC		0x00040000                 //Imp. Data cache coh.
#define PHYSAW		0x00080000                 //JTAG addrs. reg. size
#define SDBBPC          0x00800000                 //SDBBPCode zero=1.3 spec, one=Special2 Opcode

/*------------------------------------------------------------------------*/
/* TARGET DSU REGISTERS              					  */
/*------------------------------------------------------------------------*/
#define BASE	0xFF300000  //DSU BASE address
#define	DCR	BASE+0x0000 //DSU Control register
#define IBS	BASE+0x0004 //Instruction Address Break Register
#define DBS	BASE+0x0008 //Data Break Status Register
#define PBS	BASE+0x000C //Processor Break Status Register
#define IBA0	BASE+0x0100 //Instruction Address Break Register 0
#define IBC0	BASE+0x0104 //Instruction Break Control Register 0
#define IBM0	BASE+0x0108 //Instruction Address Break Mask Register 0
#define DBA0	BASE+0x0200 //Data Address Break Register 0
#define DBC0	BASE+0x0204 //Data Break Control register 0
#define DBM0	BASE+0x0208 //Data Address Break Mask register 0
#define DB0	BASE+0x020C //Data Value Break Register 0
#define PBA0	BASE+0x0300 //Processor Address Bus Break Register 0
#define PBD0	BASE+0x0304 //Processor Data Bus Break Register 0
#define PBM0	BASE+0x0308 //Processor Data Bus Break Mask Register 0
#define PBC0	BASE+0x030C //Processor Bus Break Control and Address Mask Register 0

/*------------------------------------------------------------------------*/
/*  BIT MAPS DEBUG CONTROL REGISTER  					  */
/*------------------------------------------------------------------------*/
#define DCR_TM	     	0x00000001   //Trace Mode
#define DCR_MRST     	0x00000002   //Mask soft reset
#define DCR_MP	     	0x00000004   //Memory protection
#define DCR_MNMI     	0x00000008   //Mask Non-Maskable interrupt
				     //(in non debug mode)
#define DCR_MINT     	0x00000010   //Mask Interrupt
#define DCR_TOFDSU   	0x00000020   //Test Output Full
#define DCR_TIFDSU   	0x00000040   //Test Input Full
#define DCR_ENM	     	0x20000000   //Endianess '0'=littel endian '1'=big endian
#define DCR_HIS	     	0x40000000   //Halt status
#define DCR_DZS	     	0x80000000   //Doze status

/*------------------------------------------------------------------------*/
/*  BITS INSTRUCTION ADDRESS BREAK STATUS 				  */
/*------------------------------------------------------------------------*/
#define IBS_BS0		0x00000001   //Break Status
#define IBS_BS(dword)	((dword&0x00004FF2)>>1)//channel status bits
#define IBS_BCN(dword)	(dword>>24)  //Break Channel Number

/*------------------------------------------------------------------------*/
/*  BITS INSTRUCTION ADDRESS BREAK CONTROL				  */
/*------------------------------------------------------------------------*/
#define IBC_BE		0x00000001   //Break Enable
#define IBC_TE		0x00000004   //Trigger Enable

/*------------------------------------------------------------------------*/
/*  BITS DATA ADDRESS BREAK STATUS	 				  */
/*------------------------------------------------------------------------*/
#define DBS_BS0		0x00000001   //Break Status
#define DBS_BS(dword)	((dword&0x00004FF2)>>1)//channel status bits
#define DBS_BCN(dword)	(dword>>24)  //Break Channel Number

/*------------------------------------------------------------------------*/
/*  BITS DATA ADDRESS BREAK CONTROL	 				  */
/*------------------------------------------------------------------------*/
#define DBC_BE		0x00000001   //Break Enable
#define DBC_TE		0x00000004   //Trigger Enable
#define DBC_BLM 	0x00000FF0   //BLM D[63..0]

/*------------------------------------------------------------------------*/
/*  BITS PROCESSOR BREAK STATUS		 				  */
/*------------------------------------------------------------------------*/
#define PBS_BS0		0x00000001   //Break Status
#define PBS_BS(dword)	((dword&0x00004FF2)>>1)//channel status bits
#define PBS_BCN(dword)	(dword>>24)  //Break Channel Number

/*------------------------------------------------------------------------*/
/*  BITS PROCESSOR BREAK CONTROL	 				  */
/*------------------------------------------------------------------------*/
#define PBC_BE		0x00000001   //Break Enable
#define PBC_TE		0x00000004   //Trigger Enable
#define PBC_IFUC	0x00000010   //Instruction fetch from uncached area
#define PBC_DLUC        0x00000020   //Data load uncached area
#define PBC_DSUC	0x00000040   //Data store to uncached area
#define PBC_LAM		0xFFFFFF00   //Lower Address Mask

/*------------------------------------------------------------------------*/
/*  DEBUG EXCEPTION HOST-MEMORY SETUP					  */
/*------------------------------------------------------------------------*/
#define DEX_LOW		0xFF200000   //Low memory boundary
#define DEX_HIGH	0xFF200FFF   //High memory boundary
#define DEX_SIZE_W	0xFFF 	     //4kb

/*------------------------------------------------------------------------*/
/*  UPLOAD DESTINATIONS SETUP						  */
/*------------------------------------------------------------------------*/
#define DEST_NORMAL	0xA	     //Destination is normal DMA write
#define DEST_FLASH	0xB	     //Destination is FLASH memory on target
#define DEST_HOST	0xC	     //Destination is shared host memory

/*------------------------------------------------------------------------*/
/*  PARPORT BIT DEFINITIONS						  */
/*------------------------------------------------------------------------*/

//#define PP_BASE 0x3bc
#define PP_BASE 0x378
#define PP_DATA_OFF 0
#define PP_STATUS_OFF 1
#define PP_CONTROL_OFF 2

#define PARPORT_STATUS_BUSY 0x80
// ioperm(PP_BASE, 5 /* num addr */, 1 /* perm */);
// outb(BASE,char); // data, standard is write only. can be bi-dir.
// char = inb(BASE+1) // status, read only.
// outb(BASE+2,char) // ctl, write only
//
// pin 11 input is status char bit 7.


#define TRSTN_BIT_MASK  (0x1<<2)
#define TMS_BIT_MASK    (0x1<<3)
#define TDO_BIT_MASK    (0x1<<4)
#define TCK_BIT_MASK    (0x1<<5)

#define TMS_BIT(x)      ((x)?TMS_BIT_MASK:0x0)
#define TCK_BIT(x)      ((x)?TCK_BIT_MASK:0x0)
#define TRSTN_BIT(x)    ((x)?TRSTN_BIT_MASK:0x0)
#define TDO_BIT(x)      ((x)?TDO_BIT_MASK:0x0)

#endif /* _LINUX_EJTAG_H */
