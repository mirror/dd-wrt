/*
 * Copyright (c) 2000, 2001 by Lineo, Inc./Lineo Canada Corp. (www.lineo.com),
 * Copyright (c) 2001,2002 by Arcturus Networks Inc. (www.arcturusnetworks.com),
 * Ported for Blackfin/Frio Architecture by 
 *					Akbar Hussain  <akbar.hussain@arcturusnetworks.com>,
 *             Tony Kou <tony.ko@arcturusnetworks.com>
 *					MaTed <mated@sympatico.ca> <mated@arcturusnetworks.com>
 * Tab Size == 4 ....MaTed
 */


/* FOR BlackFin DSP:     BYTE = 08 bits,  HALFWORD = 16 bit & WORD = 32 bits    */


#ifndef _BLKFin_H_
#define _BLKFin_H_

#define BYTE_REF(addr)		 (*((volatile unsigned char*)addr))
#define HALFWORD_REF(addr) 	 (*((volatile unsigned short*)addr))
#define WORD_REF(addr)		 (*((volatile unsigned long*)addr))

/*
 * Typedefs
 */
typedef unsigned long	UINT32;
typedef unsigned short 	UINT16;
typedef	unsigned char	UCHAR;

#define PUT_FIELD(field, val) (((val) << field##_SHIFT) & field##_MASK) /* ?  */
#define GET_FIELD(reg, field) (((reg) & field##_MASK) >> field##_SHIFT) /*  ?   */

/*      FOLLOWING ARE THE BlackFin SYSTEM MMR REGISTERS (LOWER 2 MB)     */

/**************************
 *
 * L2 MULTIPLE INPUT SIGNATURE REGISTERS (MISR) (0XFFC0000 - 0XFFC003FF)
 *
 **************************/
 
#define MISR_CTL_ADDR		0xffc00000  /* Control register 32 bit*/
#define MISR_CTL			WORD_REF(MISR_CTL_ADDR)
#define MISR_RMISR0_ADDR	0xffc00004  /* CoreL2[31:0] read bus register 32 bit*/
#define MISR_RMISR0			WORD_REF(MISR_RMISR0_ADDR)
#define MISR_RMISR1_ADDR	0xffc00008  /* CoreL2[63:32] read bus register 32 bit*/
#define MISR_RMISR1			WORD_REF(MISR_RMISR1_ADDR)
#define MISR_RMISR2_ADDR	0xffc0000C  /* SysL2[31:0] read bus register 32 bit*/
#define MISR_RMISR2			WORD_REF(MISR_RMISR2_ADDR)
#define MISR_WMISR0_ADDR	0xffc00010  /* CoreL2[31:0] write bus register 32 bit*/
#define MISR_WMISR0			WORD_REF(MISR_WMISR0_ADDR)
#define MISR_WMISR1_ADDR	0xffc00014  /* CoreL2[63:32] write bus register 32 bit*/
#define MISR_WMISR1			WORD_REF(MISR_WMISR1_ADDR)
#define MISR_WMISR2_ADDR	0xffc00018  /* SysL2[31:0] write bus register 32 bit*/
#define MISR_WMISR2			WORD_REF(MISR_WMISR2_ADDR)

/**************************
 *
 * CLOCK & SYSTEM CONTROL (0XFFC0400 - 0XFFC007FF)
 *
 **************************/
 
#define PLLCTL_ADDR			0xffc00400  /* PLL control register 32 bit*/
#define PLLCTL				WORD_REF(PLLCTL_ADDR)
#define PLLSTAT_ADDR		0xffc00404  /* PLL status register 16 bit */
#define PLLSTAT				HALFWORD_REF(PLLSTAT_ADDR)
#define LOCKCNT_ADDR		0xffc00406  /* PLL lock counter register 16 bit */
#define LOCKCNT				HALFWORD_REF(LOCKCNT_ADDR)
#define IOCKR_ADDR			0xffc00408  /* Peripheral clock enable register 16 bit */
#define IOCKR				HALFWORD_REF(IOCKR_ADDR)
#define SWRST_ADDR			0xffc00410  /* Software reset register 16 bit */
#define SWRST				HALFWORD_REF(SWRST_ADDR)
#define NXTSCR_ADDR			0xffc00412  /* Next system configuration register 16 bit */
#define NXTSCR				HALFWORD_REF(NXTSCR_ADDR)
#define SYSCR_ADDR			0xffc00414  /* Sytem Configuration register */
#define SYSCR 				HALFWORD_REF(SYSCR_ADDR)

/**************************
 *
 * JTAG/DEBUG COMMUNICATION CHANNEL (0XFFC00800 - 0XFFC00BFF)
 *
 **************************/
 
#define INDATA_ADDR			0xffc00800   /* Indata register 16 bit */
#define INDATA				HALFWORD_REF(INDATA_ADDR)
#define OUTDATA_ADDR		0xffc00802   /*  Outdata register  16 bit */
#define OUTDATA				HALFWORD_REF(OUTDATA_ADDR)
#define JDCSR_ADDR			0xffc00804   /*  JDCC Control/Status register 32 bit */
#define JDCSR				WORD_REF(JDCSR_ADDR)
#define IDDEV_ADDR			0xffc00808   /*  Device ID register  32 bit */
#define IDDEV				WORD_REF(IDDEV_ADDR)
#define IDCORE_ADDR			0xffc0080c   /*  Core ID register 32 bit */
#define IDCORE				WORD_REF(IDCORE_ADDR)

/****************************
 *
 *  EXTENDED CORE INTERRUPT CONTROLLER (ECIC) 0XFFC00C00 - 0XFFC0OFFF
 *
 ****************************/
 
#define RVECT_ADDR			0xffc00c00   /* Reset vector register 32 bit */
#define RVECT				WORD_REF(RVECT_ADDR)
#define IAR0_ADDR			0xffc00c04   /* Interrupt assignment register 0 32 bit*/
#define IAR0				WORD_REF(IAR0_ADDR)
#define IAR1_ADDR			0xffc00c08   /* Interrupt assignment register 1 32 bit*/
#define IAR1				WORD_REF(IAR1_ADDR)
#define IAR2_ADDR			0xffc00c0c   /* Interrupt assignment register 2 32 bit*/
#define IAR2				WORD_REF(IAR2_ADDR)
#define IMR_ADDR			0xffc00c10   /* Interrupt mask register  32 bit*/
#define IMR				WORD_REF(IMR_ADDR)
#define ISR_ADDR			0xffc00c14   /* Interrupt status register  32 bit*/
#define ISR				WORD_REF(ISR_ADDR)
#define IWR_ADDR			0xffc00c18   /* Interrupt wakeup register  32 bit*/
#define IWR				WORD_REF(IWR_ADDR)

/****************************
 *
 *  WATCHDOG TIMER  (0XFFC01000 - 0XFFC013F)
 *
 ****************************/
 
#define WDOGCTL_ADDR		0xffc01000  /* Watchdog control register  32 bit */
#define WDOGCTL				WORD_REF(WDOGCTL_ADDR)
#define WDOGCNT_ADDR		0xffc01004  /* Watchdog count register 32 bit */
#define WDOGCNT				WORD_REF(WDOGCNT_ADDR)
#define WDOGSTAT_ADDR		0xffc01008  /* Watchdog status register 32 bit */
#define WDOGSTAT			WORD_REF(WDOGSTAT_ADDR)

/****************************
 *
 *  REAL TIME CLOCK (RTC) REGISTERS  (0XFFC01400 - 0XFFC017FF)
 *
 ****************************/
 
#define RTCSTAT_ADDR		0xffc01400  /* RTC status register  32 bit */
#define RTCSTAT				WORD_REF(RTCSTAT_ADDR)
#define RTCICTL_ADDR		0xffc01404  /* RTC Interrupt control register  32 bit */
#define RTCICTL				WORD_REF(RTCICTL_ADDR)
#define RTCISTAT_ADDR		0xffc01408  /* RTC Interrupt status register  32 bit */
#define RTCISTAT			WORD_REF(RTCISTAT_ADDR)
#define RTCSWCNT_ADDR		0xffc0140c  /* RTC Stop watch count register  32 bit */
#define RTCSWCNT			WORD_REF(RTCSWCNT_ADDR)
#define RTCALARM_ADDR		0xffc01410  /* RTC Alarm time register  32 bit */
#define RTCALARM			WORD_REF(RTCALARM_ADDR)
#define RTCFAST_ADDR		0xffc01414  /* RTC Prescalar control register  32 bit */
#define RTCFAST				WORD_REF(RTCFAST_ADDR)

/****************************
 *
 *  UART 0 CONTROLLER REGISTERS  (0XFFC01800 - 0XFFC01BFF)
 *
 ****************************/

#define UART0_THR_ADDR		0xffc01800  /* UART 0 Transmit holding register  16 bit */
#define UART0_THR			HALFWORD_REF(UART0_THR_ADDR)
#define UART0_THR_MASK		0x00ff /* Data to be transmitted */ 


#define UART0_RBR_ADDR		0xffc01800  /* UART 0 Receive buffer register  16 bit */
#define UART0_RBR			HALFWORD_REF(UART0_RBR_ADDR)
#define UART0_RBR_MASK		0x00ff /* Data to be received */ 

#define UART0_DLL_ADDR		0xffc01800  /* UART 0 Divisor latch (low byte) register  16 bit */
#define UART0_DLL			HALFWORD_REF(UART0_DLL_ADDR)

#define UART0_IER_ADDR		0xffc01802  /* UART 0 Interrupt enable register  16 bit */
#define UART0_IER			HALFWORD_REF(UART0_IER_ADDR)
#define UART0_IER_ERBFI			0x01	/* Enable Receive Buffer Full Interrupt(DR bit) */
#define UART0_IER_ETBEI			0x02	/* Enable Transmit Buffer Empty Interrupt(THRE bit) */
#define UART0_IER_ELSI			0x04	/* Enable RX Status Interrupt(gen if any of LSR[4:1] set) */
#define UART0_IER_EDDSI			0x08	/* Enable Modem Status Interrupt(gen if any UARTx_MSR[3:0] set) */


#define UART0_DLH_ADDR		0xffc01802  /* UART 0 Divisor latch (high byte) register  16 bit */
#define UART0_DLH			HALFWORD_REF(UART0_DLH_ADDR)
#define UART0_IIR_ADDR		0xffc01804  /* UART 0 Interrupt identification register  16 bit */
#define UART0_IIR			HALFWORD_REF(UART0_IIR_ADDR)
#define UART0_IIR_NOINT			0x01	/* Bit0: cleared when no interrupt */
#define UART0_IIR_STATUS		0x06	/* mask bit for the status: bit2-1 */
#define UART0_IIR_LSR			0x06	/* Receive line status */
#define UART0_IIR_RBR			0x04	/* Receive data ready */
#define UART0_IIR_THR			0x02	/* Ready to transmit  */
#define UART0_IIR_MSR			0x00	/* Modem status       */

#define UART0_LCR_ADDR		0xffc01806  /* UART 0 Line control register  16 bit */
#define UART0_LCR			HALFWORD_REF(UART0_LCR_ADDR)
#define UART0_LCR_WLS5			0	/* word length 5 bits */
#define UART0_LCR_WLS6			0x01	/* word length 6 bits */
#define UART0_LCR_WLS7			0x02	/* word length 7 bits */
#define UART0_LCR_WLS8			0x03	/* word length 8 bits */
#define UART0_LCR_STB			0x04	/* StopBit: 1: 2 stop bits for non-5-bit word length
                                                       1/2 stop bits for 5-bit word length
                                                    0: 1 stop bit */
#define UART0_LCR_PEN			0x08	/* Parity Enable 1: for enable */
#define UART0_LCR_EPS			0x10	/* Parity Selection: 1: for even pariety
                                                             0: odd parity when PEN =1 & SP =0 */
#define UART0_LCR_SP			0x20	/* Sticky Parity: */
#define UART0_LCR_SB			0x40	/* Set Break: force TX pin to 0 */
#define UART0_LCR_DLAB			0x80	/* Divisor Latch Access */


#define UART0_MCR_ADDR		0xffc01808  /* UART 0 Module Control register  16 bit */
#define UART0_MCR			HALFWORD_REF(UART0_MCR_ADDR)

#define UART0_LSR_ADDR		0xffc0180a  /* UART 0 Line status register  16 bit */
#define UART0_LSR			HALFWORD_REF(UART0_LSR_ADDR)
#define UART0_LSR_DR			0x01	/* Data Ready */
#define UART0_LSR_OE			0x02	/* Overrun Error */
#define UART0_LSR_PE			0x04	/* Parity Error  */
#define UART0_LSR_FE			0x08	/* Frame Error   */
#define UART0_LSR_BI			0x10	/* Break Interrupt */
#define UART0_LSR_THRE			0x20	/* THR empty, REady to accept */
#define UART0_LSR_TEMT			0x40	/* TSR and UARTx_thr both empty */

#define UART0_MSR_ADDR		0xffc0180c  /* UART 0 Modem status register  16 bit */
#define UART0_MSR			HALFWORD_REF(UART0_MSR_ADDR
#define UART0_SCR_ADDR		0xffc0180e  /* UART 0 Scratch register  16 bit */
#define UART0_SCR			HALFWORD_REF(UART0_SCR_ADDR)
#define UART0_IRCR_ADDR		0xffc01810  /* UART 0 IrDA Control register  16 bit */
#define UART0_IRCR			HALFWORD_REF(UART0_IRCR_ADDR)

#define UART0_CURR_PTR_RX_ADDR		0xffc01a00 /* UART 0 RCV DMA Current pointer register 16 bit*/
#define UART0_CURR_PTR_RX			HALFWORD_REF(UART0_CURR_PTR_RX_ADDR)
#define UART0_CONFIG_RX_ADDR		0xffc01a02 /* UART 0 RCV DMA Configuration register 16 bit */
#define UART0_CONFIG_RX				HALFWORD_REF(UART0_CONFIG_RX_ADDR)
#define UART0_START_ADDR_HI_RX_ADDR	0xffc01a04 /* UART 0 RCV DMA start add. hi reg 16 bit */
#define UART0_START_ADDR_HI_RX		HALFWORD_REF(UART0_START_ADDR_HI_RX_ADDR)
#define UART0_START_ADDR_LO_RX_ADDR	0xffc01a06 /* UART 0 RCV DMA start add. lo reg 16 bit */
#define UART0_START_ADDR_LO_RX		HALFWORD_REF(UART0_START_ADDR_LO_RX_ADDR)
#define UART0_COUNT_RX_ADDR			0xffc01a08 /* UART 0 RCV DMA count register  16 bit */
#define UART0_COUNT_RX				HALFWORD_REF(UART0_COUNT_RX_ADDR)
#define UART0_NEXT_DESCR_RX_ADDR    0xffc01a0a /*UART 0 RCV DMA next descripter poin reg 16 bit */
#define UART0_NEXT_DESCR_RX			HALFWORD_REF(UART0_NEXT_DESCR_RX_ADDR)
#define UART0_DESCR_RDY_RX_ADDR		0xffc01a0c /* UART 0 RCV DMA descripter ready reg 16 bit */
#define UART0_DESCR_RDY_RX			HALFWORD_REF(UART0_DESCR_RDY_RX_ADDR)
#define UART0_IRQSTAT_RX_ADDR		0xffc01a0e /* UART 0 RCV DMA Interrupt register  16 bit */
#define UART0_IRQSTAT_RX			HALFWORD_REF(UART0_IRQSTAT_RX_ADDR)

#define UART0_CURR_PTR_TX_ADDR		0xffc01b00 /* UART 0 XMT DMA Current pointer register 16 bit*/
#define UART0_CURR_PTR_TX			HALFWORD_REF(UART0_CURR_PTR_TX_ADDR)
#define UART0_CONFIG_TX_ADDR		0xffc01b02 /* UART 0 XMT DMA Configuration register 16 bit */
#define UART0_CONFIG_TX				HALFWORD_REF(UART0_CONFIG_TX_ADDR)
#define UART0_START_ADDR_HI_TX_ADDR	0xffc01b04 /* UART 0 XMT DMA start add. hi reg 16 bit */
#define UART0_START_ADDR_HI_TX		HALFWORD_REF(UART0_START_ADDR_HI_TX_ADDR)
#define UART0_START_ADDR_LO_TX_ADDR	0xffc01b06 /* UART 0 XMT DMA start add. lo reg 16 bit */
#define UART0_START_ADDR_LO_TX		HALFWORD_REF(UART0_START_ADDR_LO_TX_ADDR)
#define UART0_COUNT_TX_ADDR			0xffc01b08 /* UART 0 XMT DMA count register  16 bit */
#define UART0_COUNT_TX				HALFWORD_REF(UART0_COUNT_TX_ADDR)
#define UART0_NEXT_DESCR_TX_ADDR	0xffc01b0a /*UART 0 XMT DMA next descripter poin reg 16 bit */
#define UART0_NEXT_DESCR_TX			HALFWORD_REF(UART0_NEXT_DESCR_TX_ADDR)
#define UART0_DESCR_RDY_TX_ADDR		0xffc01b0c /* UART 0 XMT DMA descripter ready reg 16 bit */
#define UART0_DESCR_RDY_TX			HALFWORD_REF(UART0_DESCR_RDY_TX_ADDR)
#define UART0_IRQSTAT_TX_ADDR		0xffc01b0e /* UART 0 XMT DMA Interrupt register  16 bit */
#define UART0_IRQSTAT_TX			HALFWORD_REF(UART0_IRQSTAT_TX_ADDR)

/****************************
 *
 *  UART 1 CONTROLLER REGISTERS  (0XFFC01C00 - 0XFFC01FFF)
 *
 ****************************/

#define UART1_THR_ADDR		0xffc01c00  /* UART 1 Transmit holding register  16 bit */
#define UART1_THR			HALFWORD_REF(UART1_THR_ADDR)
#define UART1_THR_MASK		0x00ff /* Data to be transmitted */

#define UART1_RBR_ADDR		0xffc01c00  /* UART 1 Receive buffer register  16 bit */
#define UART1_RBR			HALFWORD_REF(UART1_RBR_ADDR)
#define UART1_RBR_MASK		0x00ff /* Data to be transmitted */

#define UART1_DLL_ADDR		0xffc01c00  /* UART 1 Divisor latch (low byte) register  16 bit */
#define UART1_DLL			HALFWORD_REF(UART1_DLL_ADDR)

#define UART1_IER_ADDR		0xffc01c02  /* UART 1 Interrupt enable register  16 bit */
#define UART1_IER			HALFWORD_REF(UART1_IER_ADDR)
#define UART1_IER_ERBFI			0x01    /* Enable Receive Buffer Full Interrupt(DR bit) */
#define UART1_IER_ETBEI			0x02    /* Enable Transmit Buffer Empty Interrupt(THRE bit) */
#define UART1_IER_ELSI			0x04    /* Enable RX Status Interrupt(gen if any of LSR[4:1] set) */
#define UART1_IER_EDDSI			0x08    /* Enable Modem Status Interrupt(gen if any UARTx_MSR[3:0] set) */

#define UART1_DLH_ADDR		0xffc01c02  /* UART 1 Divisor latch (high byte) register  16 bit */
#define UART1_DLH			HALFWORD_REF(UART1_DLH_ADDR)

#define UART1_IIR_ADDR		0xffc01c04  /* UART 1 Interrupt identification register  16 bit */
#define UART1_IIR			HALFWORD_REF(UART1_IIR_ADDR)
#define UART1_IIR_NOINT     	0x01    /* Bit0: cleared when no interrupt */
#define UART1_IIR_STATUS        0x06    /* mask bit for the status: bit2-1 */
#define UART1_IIR_LSR           0x06    /* Receive line status */
#define UART1_IIR_RBR           0x04    /* Receive data ready */
#define UART1_IIR_THR           0x02    /* Ready to transmit  */
#define UART1_IIR_MSR           0x00    /* Modem status       */

#define UART1_LCR_ADDR		0xffc01c06  /* UART 1 Line control register  16 bit */
#define UART1_LCR			HALFWORD_REF(UART1_LCR_ADDR)
#define UART1_LCR_WLS5      	0       /* word length 5 bits */
#define UART1_LCR_WLS6      	0x01    /* word length 6 bits */
#define UART1_LCR_WLS7          0x02    /* word length 7 bits */
#define UART1_LCR_WLS8          0x03    /* word length 8 bits */
#define UART1_LCR_STB           0x04    /* StopBit: 1: 2 stop bits for non-5-bit word length
                                                       1/2 stop bits for 5-bit word length
                                                    0: 1 stop bit */
#define UART1_LCR_PEN           0x08    /* Parity Enable 1: for enable */
#define UART1_LCR_EPS           0x10    /* Parity Selection: 1: for even pariety
                                                             0: odd parity when PEN =1 &
															 SP =0 */
#define UART1_LCR_SP            0x20    /* Sticky Parity: */
#define UART1_LCR_SB            0x40    /* Set Break: force TX pin to 0 */
#define UART1_LCR_DLAB          0x80    /* Divisor Latch Access */

#define UART1_MCR_ADDR		0xffc01c08  /* UART 1 Module Control register  16 bit */
#define UART1_MCR			HALFWORD_REF(UART1_MCR_ADDR)

#define UART1_LSR_ADDR		0xffc01c0a  /* UART 1 Line status register  16 bit */
#define UART1_LSR			HALFWORD_REF(UART1_LSR_ADDR)
#define UART1_LSR_DR        	0x01    /* Data Ready */
#define UART1_LSR_OE            0x02    /* Overrun Error */
#define UART1_LSR_PE            0x04    /* Parity Error  */
#define UART1_LSR_FE            0x08    /* Frame Error   */
#define UART1_LSR_BI            0x10    /* Break Interrupt */
#define UART1_LSR_THRE          0x20    /* THR empty, REady to accept */
#define UART1_LSR_TEMT          0x40    /* TSR and UARTx_thr both empty */

#define UART1_MSR_ADDR		0xffc01c0c  /* UART 1 Modem status register  16 bit */
#define UART1_MSR			HALFWORD_REF(UART1_MSR_ADDR
#define UART1_SCR_ADDR		0xffc01c0e  /* UART 1 Scratch register  16 bit */
#define UART1_SCR			HALFWORD_REF(UART1_SCR_ADDR)

#define UART1_CURR_PTR_RX_ADDR		0xffc01e00 /* UART 1 RCV DMA Current pointer register 16 bit*/
#define UART1_CURR_PTR_RX			HALFWORD_REF(UART1_CURR_PTR_RX_ADDR)
#define UART1_CONFIG_RX_ADDR		0xffc01e02 /* UART 1 RCV DMA Configuration register 16 bit */
#define UART1_CONFIG_RX				HALFWORD_REF(UART1_CONFIG_RX_ADDR)
#define UART1_START_ADDR_HI_RX_ADDR	0xffc01e04 /* UART 1 RCV DMA start add. hi reg 16 bit */
#define UART1_START_ADDR_HI_RX		HALFWORD_REF(UART1_START_ADDR_HI_RX_ADDR)
#define UART1_START_ADDR_LO_RX_ADDR	0xffc01e06 /* UART 1 RCV DMA start add. lo reg 16 bit */
#define UART1_START_ADDR_LO_RX		HALFWORD_REF(UART1_START_ADDR_LO_RX_ADDR)
#define UART1_COUNT_RX_ADDR			0xffc01e08 /* UART 1 RCV DMA count register  16 bit */
#define UART1_COUNT_RX				HALFWORD_REF(UART1_COUNT_RX_ADDR)
#define UART1_NEXT_DESCR_RX_ADDR   	0xffc01e0a /*UART 1 RCV DMA next descripter poin reg 16 bit */
#define UART1_NEXT_DESCR_RX			HALFWORD_REF(UART1_NEXT_DESCR_RX_ADDR)
#define UART1_DESCR_RDY_RX_ADDR		0xffc01e0c /* UART 1 RCV DMA descripter ready reg 16 bit */
#define UART1_DESCR_RDY_RX			HALFWORD_REF(UART1_DESCR_RDY_RX_ADDR)
#define UART1_IRQSTAT_RX_ADDR		0xffc01e0e /* UART 1 RCV DMA Interrupt register  16 bit */
#define UART1_IRQSTAT_RX			HALFWORD_REF(UART1_IRQSTAT_RX_ADDR)

#define UART1_CURR_PTR_TX_ADDR		0xffc01f00 /* UART 1 XMT DMA Current pointer register 16 bit*/
#define UART1_CURR_PTR_TX			HALFWORD_REF(UART1_CURR_PTR_TX_ADDR)
#define UART1_CONFIG_TX_ADDR		0xffc01f02 /* UART 1 XMT DMA Configuration register 16 bit */
#define UART1_CONFIG_TX				HALFWORD_REF(UART1_CONFIG_TX_ADDR)
#define UART1_START_ADDR_HI_TX_ADDR	0xffc01f04 /* UART 1 XMT DMA start add. hi reg 16 bit */
#define UART1_START_ADDR_HI_TX		HALFWORD_REF(UART1_START_ADDR_HI_TX_ADDR)
#define UART1_START_ADDR_LO_TX_ADDR	0xffc01f06 /* UART 1 XMT DMA start add. lo reg 16 bit */
#define UART1_START_ADDR_LO_TX		HALFWORD_REF(UART1_START_ADDR_LO_TX_ADDR)
#define UART1_COUNT_TX_ADDR			0xffc01f08 /* UART 1 XMT DMA count register  16 bit */
#define UART1_COUNT_TX				HALFWORD_REF(UART1_COUNT_TX_ADDR)
#define UART1_NEXT_DESCR_TX_ADDR	0xffc01f0a /*UART 1 XMT DMA next descripter poin reg 16 bit */
#define UART1_NEXT_DESCR_TX			HALFWORD_REF(UART1_NEXT_DESCR_TX_ADDR)
#define UART1_DESCR_RDY_TX_ADDR		0xffc01f0c /* UART 1 XMT DMA descripter ready reg 16 bit */
#define UART1_DESCR_RDY_TX			HALFWORD_REF(UART1_DESCR_RDY_TX_ADDR)
#define UART1_IRQSTAT_TX_ADDR		0xffc01f0e /* UART 1 XMT DMA Interrupt register  16 bit */
#define UART1_IRQSTAT_TX			HALFWORD_REF(UART1_IRQSTAT_TX_ADDR)

/****************************
 *
 *  TIMER REGISTERS  (0XFFC02000 - 0XFFC023FF)
 *       THERE ARE 3 TIMERS
 ****************************/
 
		/*             TIMER 0              */
		
#define TIMER0_STATUS_ADDR		0xffc02000 /* TIMER 0 Global status & sticky register  16 bit */
#define TIMER0_STATUS			HALFWORD_REF(TIMER0_STATUS_ADDR)
#define TIMER0_CONFIG_ADDR		0xffc02002 /* TIMER 0 configuration register  16 bit */
#define TIMER0_CONFIG			HALFWORD_REF(TIMER0_CONFIG_ADDR)
#define TIMER0_COUNTER_LO_ADDR	0xffc02004 /* TIMER 0 counter (low word) register   16 bit */
#define TIMER0_COUNTER_LO		HALFWORD_REF(TIMER0_COUNTER_LO_ADDR)
#define TIMER0_COUNTER_HI_ADDR	0xffc02006 /* TIMER 0 counter (high word) register  16 bit */
#define TIMER0_COUNTER_HI		HALFWORD_REF(TIMER0_COUNTER_HI_ADDR)
#define TIMER0_PERIOD_LO_ADDR	0xffc02008 /* TIMER 0 period (low word) register  16 bit */
#define TIMER0_PERIOD_LO		HALFWORD_REF(TIMER0_PERIOD_LO_ADDR)
#define TIMER0_PERIOD_HI_ADDR	0xffc0200a /* TIMER 0 period (high word) register  16 bit */
#define TIMER0_PERIOD_HI		HALFWORD_REF(TIMER0_PERIOD_HI_ADDR)
#define TIMER0_WIDTH_LO_ADDR	0xffc0200c /* TIMER 0 width (low word) register  16 bit */
#define TIMER0_WIDTH_LO			HALFWORD_REF(TIMER0_WIDTH_LO_ADDR)
#define TIMER0_WIDTH_HI_ADDR	0xffc0200e /* TIMER 0 width (high word) register  16 bit */
#define TIMER0_WIDTH_HI			HALFWORD_REF(TIMER0_WIDTH_HI_ADDR)

		/*             TIMER 1              */

#define TIMER1_STATUS_ADDR		0xffc02010 /* TIMER 1 Global status & sticky register  16 bit */
#define TIMER1_STATUS			HALFWORD_REF(TIMER1_STATUS_ADDR)
#define TIMER1_CONFIG_ADDR		0xffc02012 /* TIMER 1 configuration register  16 bit */
#define TIMER1_CONFIG			HALFWORD_REF(TIMER1_CONFIG_ADDR)
#define TIMER1_COUNTER_LO_ADDR	0xffc02014 /* TIMER 1 counter (low word) register   16 bit */
#define TIMER1_COUNTER_LO		HALFWORD_REF(TIMER1_COUNTER_LO_ADDR)
#define TIMER1_COUNTER_HI_ADDR	0xffc02016 /* TIMER 1 counter (high word) register  16 bit */
#define TIMER1_COUNTER_HI		HALFWORD_REF(TIMER1_COUNTER_HI_ADDR)
#define TIMER1_PERIOD_LO_ADDR	0xffc02018 /* TIMER 1 period (low word) register  16 bit */
#define TIMER1_PERIOD_LO		HALFWORD_REF(TIMER1_PERIOD_LO_ADDR)
#define TIMER1_PERIOD_HI_ADDR	0xffc0201a /* TIMER 1 period (high word) register  16 bit */
#define TIMER1_PERIOD_HI		HALFWORD_REF(TIMER1_PERIOD_HI_ADDR)
#define TIMER1_WIDTH_LO_ADDR	0xffc0201c /* TIMER 1 width (low word) register  16 bit */
#define TIMER1_WIDTH_LO			HALFWORD_REF(TIMER1_WIDTH_LO_ADDR)
#define TIMER1_WIDTH_HI_ADDR	0xffc0201e /* TIMER 1 width (high word) register  16 bit */
#define TIMER1_WIDTH_HI			HALFWORD_REF(TIMER1_WIDTH_HI_ADDR)

		/*             TIMER 2              */

#define TIMER2_STATUS_ADDR		0xffc02020 /* TIMER 2 Global status & sticky register  16 bit */
#define TIMER2_STATUS			HALFWORD_REF(TIMER2_STATUS_ADDR)
#define TIMER2_CONFIG_ADDR		0xffc02022 /* TIMER 2 configuration register  16 bit */
#define TIMER2_CONFIG			HALFWORD_REF(TIMER2_CONFIG_ADDR)
#define TIMER2_COUNTER_LO_ADDR	0xffc02024 /* TIMER 2 counter (low word) register   16 bit */
#define TIMER2_COUNTER_LO		HALFWORD_REF(TIMER2_COUNTER_LO_ADDR)
#define TIMER2_COUNTER_HI_ADDR	0xffc02026 /* TIMER 2 counter (high word) register  16 bit */
#define TIMER2_COUNTER_HI		HALFWORD_REF(TIMER2_COUNTER_HI_ADDR)
#define TIMER2_PERIOD_LO_ADDR	0xffc02028 /* TIMER 2 period (low word) register  16 bit */
#define TIMER2_PERIOD_LO		HALFWORD_REF(TIMER2_PERIOD_LO_ADDR)
#define TIMER2_PERIOD_HI_ADDR	0xffc0202a /* TIMER 2 period (high word) register  16 bit */
#define TIMER2_PERIOD_HI		HALFWORD_REF(TIMER2_PERIOD_HI_ADDR)
#define TIMER2_WIDTH_LO_ADDR	0xffc0202c /* TIMER 2 width (low word) register  16 bit */
#define TIMER2_WIDTH_LO			HALFWORD_REF(TIMER2_WIDTH_LO_ADDR)
#define TIMER2_WIDTH_HI_ADDR	0xffc0202e /* TIMER 2 width (high word) register  16 bit */
#define TIMER2_WIDTH_HI			HALFWORD_REF(TIMER2_WIDTH_HI_ADDR)

/****************************
 *
 *  GENERAL PURPOSE IO REGISTERS  (0XFFC02400 - 0XFFC027FF)
 *       
 ****************************/

#define FIO_DIR_C_ADDR		0xffc02400 /* Peripheral flag direction (clear) register 16 bit */
#define FIO_DIR_C			HALFWORD_REF(FIO_DIR_C_ADDR)
#define FIO_DIR_S_ADDR		0xffc02402 /* Peripheral flag direction (set) register 16 bit */
#define FIO_DIR_S			HALFWORD_REF(FIO_DIR_S_ADDR)
#define FIO_FLAG_C_ADDR		0xffc02404 /* Peripheral Interrupt flag (clear) register 16 bit */
#define FIO_FLAG_C			HALFWORD_REF(FIO_FLAG_C_ADDR)
#define FIO_FLAG_S_ADDR		0xffc02406 /* Peripheral Interrupt flag (set) register 16 bit */
#define FIO_FLAG_S			HALFWORD_REF(FIO_FLAG_S_ADDR)
#define FIO_MASKA_C_ADDR	0xffc02408 /* Flag Mask Interrupt A (clear) register 16 bit */
#define FIO_MASKA_C			HALFWORD_REF(FIO_MASKA_C_ADDR)
#define FIO_MASKA_S_ADDR	0xffc0240a /* Flag Mask Interrupt A (set) register 16 bit */
#define FIO_MASKA_S			HALFWORD_REF(FIO_MASKA_S_ADDR)
#define FIO_MASKB_C_ADDR	0xffc0240c /* Flag Mask Interrupt B (clear) register 16 bit */
#define FIO_MASKB_C			HALFWORD_REF(FIO_MASKB_C_ADDR)
#define FIO_MASKB_S_ADDR	0xffc0240e /* Flag Mask Interrupt B (set) register 16 bit */
#define FIO_MASKB_S			HALFWORD_REF(FIO_MASKB_S_ADDR)
#define FIO_POLAR_C_ADDR	0xffc02410 /* Flag source polarity (clear) register 16 bit */
#define FIO_POLAR_C			HALFWORD_REF(FIO_POLAR_C_ADDR)
#define FIO_POLAR_S_ADDR	0xffc02412 /* Flag source polarity (set) register 16 bit */
#define FIO_POLAR_S			HALFWORD_REF(FIO_POLAR_S_ADDR)
#define FIO_EDGE_C_ADDR		0xffc02414 /* Flag source sensitivity (clear) register 16 bit */
#define FIO_EDGE_C			HALFWORD_REF(FIO_EDGE_C_ADDR)
#define FIO_EDGE_S_ADDR		0xffc02416 /* Flag source sensitivity (set) register 16 bit */
#define FIO_EDGE_S			HALFWORD_REF(FIO_EDGE_S_ADDR)
#define FIO_BOTH_C_ADDR		0xffc02418 /* Flag set on both edges (clear) register 16 bit */
#define FIO_BOTH_C			HALFWORD_REF(FIO_BOTH_C_ADDR)
#define FIO_BOTH_S_ADDR		0xffc0241a /* Flag set on both edges (set) register 16 bit */
#define FIO_BOTH_S			HALFWORD_REF(FIO_BOTH_S_ADDR)

/****************************
 *
 *  SPORT 0 CONTROLLER REGISTERS  (0XFFC02800 - 0XFFC02BFF)
 *       
 ****************************/
 
#define SPORT0_TX_CONFIG_ADDR	0xffc02800 /* SPORT 0 Transmit configuration register 16 bit */
#define SPORT0_TX_CONFIG		HALFWORD_REF(SPORT0_TX_CONFIG_ADDR)
#define SPORT0_RX_CONFIG_ADDR	0xffc02802 /* SPORT 0 Receive configuration register 16 bit */
#define SPORT0_RX_CONFIG		HALFWORD_REF(SPORT0_RX_CONFIG_ADDR)
#define SPORT0_TX_ADDR			0xffc02804 /* SPORT 0 Transmit register 16 bit */
#define SPORT0_TX				HALFWORD_REF(SPORT0_TX_ADDR)
#define SPORT0_RX_ADDR			0xffc02806 /* SPORT 0 Receive register 16 bit */
#define SPORT0_RX				HALFWORD_REF(SPORT0_RX_ADDR)
#define SPORT0_TSCLKDIV_ADDR	0xffc02808 /* SPORT 0 Transmit serial clock divider 16 bit */
#define SPORT0_TSCLKDIV			HALFWORD_REF(SPORT0_TSCLKDIV_ADDR)
#define SPORT0_RSCLKDIV_ADDR	0xffc0280a /* SPORT 0 Receive serial clock divider 16 bit */
#define SPORT0_RSCLKDIV			HALFWORD_REF(SPORT0_RSCLKDIV_ADDR)
#define SPORT0_TFSDIV_ADDR		0xffc0280c /* SPORT 0 Transmit frame sync divider 16 bit */
#define SPORT0_TFSDIV			HALFWORD_REF(SPORT0_TFSDIV_ADDR)
#define SPORT0_RFSDIV_ADDR		0xffc0280e /* SPORT 0 Receive frame sync divider 16 bit */
#define SPORT0_RFSDIV			HALFWORD_REF(SPORT0_RFSDIV_ADDR)
#define SPORT0_STAT_ADDR		0xffc02810 /* SPORT 0 status register 16 bit */
#define SPORT0_STAT				HALFWORD_REF(SPORT0_STAT_ADDR)
#define SPORT0_MTCS0_ADDR		0xffc02812 /* SPORT 0 Multi-channel Transmit select reg 16 bit */
#define SPORT0_MTCS0			HALFWORD_REF(SPORT0_MTCS0_ADDR)
#define SPORT0_MTCS1_ADDR		0xffc02814 /* SPORT 0 Multi-channel Transmit select reg 16 bit */
#define SPORT0_MTCS1			HALFWORD_REF(SPORT0_MTCS1_ADDR)
#define SPORT0_MTCS2_ADDR		0xffc02816 /* SPORT 0 Multi-channel Transmit select reg 16 bit */
#define SPORT0_MTCS2			HALFWORD_REF(SPORT0_MTCS2_ADDR)
#define SPORT0_MTCS3_ADDR		0xffc02818 /* SPORT 0 Multi-channel Transmit select reg 16 bit */
#define SPORT0_MTCS3			HALFWORD_REF(SPORT0_MTCS3_ADDR)
#define SPORT0_MTCS4_ADDR		0xffc0281a /* SPORT 0 Multi-channel Transmit select reg 16 bit */
#define SPORT0_MTCS4			HALFWORD_REF(SPORT0_MTCS4_ADDR)
#define SPORT0_MTCS5_ADDR		0xffc0281c /* SPORT 0 Multi-channel Transmit select reg 16 bit */
#define SPORT0_MTCS5			HALFWORD_REF(SPORT0_MTCS5_ADDR)
#define SPORT0_MTCS6_ADDR		0xffc0281e /* SPORT 0 Multi-channel Transmit select reg 16 bit */
#define SPORT0_MTCS6			HALFWORD_REF(SPORT0_MTCS6_ADDR)
#define SPORT0_MTCS7_ADDR		0xffc02820 /* SPORT 0 Multi-channel Transmit select reg 16 bit */
#define SPORT0_MTCS7			HALFWORD_REF(SPORT0_MTCS7_ADDR)
#define SPORT0_MRCS0_ADDR		0xffc02822 /* SPORT 0 Multi-channel Receive select reg 16 bit */
#define SPORT0_MRCS0			HALFWORD_REF(SPORT0_MRCS0_ADDR)
#define SPORT0_MRCS1_ADDR		0xffc02824 /* SPORT 0 Multi-channel Receive select reg 16 bit */
#define SPORT0_MRCS1			HALFWORD_REF(SPORT0_MRCS1_ADDR)
#define SPORT0_MRCS2_ADDR		0xffc02826 /* SPORT 0 Multi-channel Receive select reg 16 bit */
#define SPORT0_MRCS2			HALFWORD_REF(SPORT0_MRCS2_ADDR)
#define SPORT0_MRCS3_ADDR		0xffc02828 /* SPORT 0 Multi-channel Receive select reg 16 bit */
#define SPORT0_MRCS3			HALFWORD_REF(SPORT0_MRCS3_ADDR)
#define SPORT0_MRCS4_ADDR		0xffc0282a /* SPORT 0 Multi-channel Receive select reg 16 bit */
#define SPORT0_MRCS4			HALFWORD_REF(SPORT0_MRCS4_ADDR)
#define SPORT0_MRCS5_ADDR		0xffc0282c /* SPORT 0 Multi-channel Receive select reg 16 bit */
#define SPORT0_MRCS5			HALFWORD_REF(SPORT0_MRCS5_ADDR)
#define SPORT0_MRCS6_ADDR		0xffc0282e /* SPORT 0 Multi-channel Receive select reg 16 bit */
#define SPORT0_MRCS6			HALFWORD_REF(SPORT0_MRCS6_ADDR)
#define SPORT0_MRCS7_ADDR		0xffc02830 /* SPORT 0 Multi-channel Receive select reg 16 bit */
#define SPORT0_MRCS7			HALFWORD_REF(SPORT0_MRCS7_ADDR)
#define SPORT0_MCMC1_ADDR		0xffc02832 /* SPORT 0 Multi-channel configuration reg 1 16 bit */
#define SPORT0_MCMC1			HALFWORD_REF(SPORT0_MCMC1_ADDR)
#define SPORT0_MCMC2_ADDR		0xffc02834 /* SPORT 0 Multi-channel configuration reg 2 16 bit */
#define SPORT0_MCMC2			HALFWORD_REF(SPORT0_MCMC2_ADDR)

#define SPORT0_CURR_PTR_RX_ADDR		0xffc02a00 /* SPORT 0 RCV DMA Current pointer reg 16 bit*/
#define SPORT0_CURR_PTR_RX			HALFWORD_REF(SPORT0_CURR_PTR_RX_ADDR)
#define SPORT0_CONFIG_DMA_RX_ADDR	0xffc02a02 /* SPORT 0 RCV DMA Configuration reg 16 bit */

#if 0
#define SPORT0_CONFIG_DMA_RX		HALFWORD_REF(SPORT0_CONFIG_RX_ADDR)
#else
#define SPORT0_CONFIG_DMA_RX		HALFWORD_REF(SPORT0_CONFIG_DMA_RX_ADDR)
#endif

#define SPORT0_START_ADDR_HI_RX_ADDR  	0xffc02a04 /* SPORT 0 RCV DMA start add. hi reg 16 bit */
#define SPORT0_START_ADDR_HI_RX		HALFWORD_REF(SPORT0_START_ADDR_HI_RX_ADDR)
#define SPORT0_START_ADDR_LO_RX_ADDR  	0xffc02a06 /* SPORT 0 RCV DMA start add. lo reg 16 bit */
#define SPORT0_START_ADDR_LO_RX		HALFWORD_REF(SPORT0_START_ADDR_LO_RX_ADDR)
#define SPORT0_COUNT_RX_ADDR  		0xffc02a08 /* SPORT 0 RCV DMA count reg 16 bit */
#define SPORT0_COUNT_RX				HALFWORD_REF(SPORT0_COUNT_RX_ADDR)
#define SPORT0_NEXT_DESCR_RX_ADDR  	0xffc02a0a /* SPORT 0 RCV DMA next descriptor poin reg 16 bit */
#define SPORT0_NEXT_DESCR_RX		HALFWORD_REF(SPORT0_NEXT_DESCR_RX_ADDR)
#define SPORT0_DESCR_RDY_RX_ADDR  	0xffc02a0c /* SPORT 0 RCV DMA descriptor ready reg 16 bit */
#define SPORT0_DESCR_RDY_RX			HALFWORD_REF(SPORT0_DESCR_RDY_RX_ADDR)
#define SPORT0_IRQSTAT_RX_ADDR  	0xffc02a0e /* SPORT 0 RCV DMA interrupt reg 16 bit */
#define SPORT0_IRQSTAT_RX			HALFWORD_REF(SPORT0_IRQSTAT_RX_ADDR)

#define SPORT0_CURR_PTR_TX_ADDR		0xffc02b00 /* SPORT 0 XMT DMA Current pointer reg 16 bit*/
#define SPORT0_CURR_PTR_TX			HALFWORD_REF(SPORT0_CURR_PTR_TX_ADDR)
#define SPORT0_CONFIG_DMA_TX_ADDR	0xffc02b02 /* SPORT 0 XMT DMA Configuration reg 16 bit */

/* Fixed in Jul 2 2003 for SPORT driver*/
#if 0
#define SPORT0_CONFIG_DMA_TX		HALFWORD_REF(SPORT0_CONFIG_TX_ADDR)
#else
#define SPORT0_CONFIG_DMA_TX		HALFWORD_REF(SPORT0_CONFIG_DMA_TX_ADDR)
#endif

#define SPORT0_START_ADDR_HI_TX_ADDR  	0xffc02b04 /* SPORT 0 XMT DMA start add. hi reg 16 bit */
#define SPORT0_START_ADDR_HI_TX		HALFWORD_REF(SPORT0_START_ADDR_HI_TX_ADDR)
#define SPORT0_START_ADDR_LO_TX_ADDR  	0xffc02b06 /* SPORT 0 XMT DMA start add. lo reg 16 bit */
#define SPORT0_START_ADDR_LO_TX		HALFWORD_REF(SPORT0_START_ADDR_LO_TX_ADDR)
#define SPORT0_COUNT_TX_ADDR  		0xffc02b08 /* SPORT 0 XMT DMA count reg 16 bit */
#define SPORT0_COUNT_TX				HALFWORD_REF(SPORT0_COUNT_TX_ADDR)
#define SPORT0_NEXT_DESCR_TX_ADDR  	0xffc02b0a /* SPORT 0 XMT DMA next descriptor poin reg 16 bit */
#define SPORT0_NEXT_DESCR_TX		HALFWORD_REF(SPORT0_NEXT_DESCR_TX_ADDR)
#define SPORT0_DESCR_RDY_TX_ADDR  	0xffc02b0c /* SPORT 0 XMT DMA descriptor ready reg 16 bit */
#define SPORT0_DESCR_RDY_TX			HALFWORD_REF(SPORT0_DESCR_RDY_TX_ADDR)
#define SPORT0_IRQSTAT_TX_ADDR  	0xffc02b0e /* SPORT 0 XMT DMA interrupt reg 16 bit */
#define SPORT0_IRQSTAT_TX			HALFWORD_REF(SPORT0_IRQSTAT_TX_ADDR)

/****************************
 *
 *  SPORT 1 CONTROLLER REGISTERS  (0XFFC02C00 - 0XFFC02FFF)
 *
 ****************************/

#define SPORT1_TX_CONFIG_ADDR	0xffc02c00 /* SPORT 1 Transmit configuration register 16 bit */
#define SPORT1_TX_CONFIG		HALFWORD_REF(SPORT1_TX_CONFIG_ADDR)
#define SPORT1_RX_CONFIG_ADDR	0xffc02c02 /* SPORT 1 Receive configuration register 16 bit */
#define SPORT1_RX_CONFIG		HALFWORD_REF(SPORT1_RX_CONFIG_ADDR)
#define SPORT1_TX_ADDR			0xffc02c04 /* SPORT 1 Transmit register 16 bit */
#define SPORT1_TX				HALFWORD_REF(SPORT1_TX_ADDR)
#define SPORT1_RX_ADDR			0xffc02c06 /* SPORT 1 Receive register 16 bit */
#define SPORT1_RX				HALFWORD_REF(SPORT1_RX_ADDR)
#define SPORT1_TSCLKDIV_ADDR	0xffc02c08 /* SPORT 1 Transmit serial clock divider 16 bit */
#define SPORT1_TSCLKDIV			HALFWORD_REF(SPORT1_TSCLKDIV_ADDR)
#define SPORT1_RSCLKDIV_ADDR	0xffc02c0a /* SPORT 1 Receive serial clock divider 16 bit */
#define SPORT1_RSCLKDIV			HALFWORD_REF(SPORT1_RSCLKDIV_ADDR)
#define SPORT1_TFSDIV_ADDR		0xffc02c0c /* SPORT 1 Transmit frame sync divider 16 bit */
#define SPORT1_TFSDIV			HALFWORD_REF(SPORT1_TFSDIV_ADDR)
#define SPORT1_RFSDIV_ADDR		0xffc02c0e /* SPORT 1 Receive frame sync divider 16 bit */
#define SPORT1_RFSDIV			HALFWORD_REF(SPORT1_RFSDIV_ADDR)
#define SPORT1_STAT_ADDR		0xffc02c10 /* SPORT 1 status register 16 bit */
#define SPORT1_STAT				HALFWORD_REF(SPORT1_STAT_ADDR)
#define SPORT1_MTCS0_ADDR		0xffc02c12 /* SPORT 1 Multi-channel Transmit select reg 16 bit */
#define SPORT1_MTCS0			HALFWORD_REF(SPORT1_MTCS0_ADDR)
#define SPORT1_MTCS1_ADDR		0xffc02c14 /* SPORT 1 Multi-channel Transmit select reg 16 bit */
#define SPORT1_MTCS1			HALFWORD_REF(SPORT1_MTCS1_ADDR)
#define SPORT1_MTCS2_ADDR		0xffc02c16 /* SPORT 1 Multi-channel Transmit select reg 16 bit */
#define SPORT1_MTCS2			HALFWORD_REF(SPORT1_MTCS2_ADDR)
#define SPORT1_MTCS3_ADDR		0xffc02c18 /* SPORT 1 Multi-channel Transmit select reg 16 bit */
#define SPORT1_MTCS3			HALFWORD_REF(SPORT1_MTCS3_ADDR)
#define SPORT1_MTCS4_ADDR		0xffc02c1a /* SPORT 1 Multi-channel Transmit select reg 16 bit */
#define SPORT1_MTCS4			HALFWORD_REF(SPORT1_MTCS4_ADDR)
#define SPORT1_MTCS5_ADDR		0xffc02c1c /* SPORT 1 Multi-channel Transmit select reg 16 bit */
#define SPORT1_MTCS5			HALFWORD_REF(SPORT1_MTCS5_ADDR)
#define SPORT1_MTCS6_ADDR		0xffc02c1e /* SPORT 1 Multi-channel Transmit select reg 16 bit */
#define SPORT1_MTCS6			HALFWORD_REF(SPORT1_MTCS6_ADDR)
#define SPORT1_MTCS7_ADDR		0xffc02c20 /* SPORT 1 Multi-channel Transmit select reg 16 bit */
#define SPORT1_MTCS7			HALFWORD_REF(SPORT1_MTCS7_ADDR)
#define SPORT1_MRCS0_ADDR		0xffc02c22 /* SPORT 1 Multi-channel Receive select reg 16 bit */
#define SPORT1_MRCS0			HALFWORD_REF(SPORT1_MRCS0_ADDR)
#define SPORT1_MRCS1_ADDR		0xffc02c24 /* SPORT 1 Multi-channel Receive select reg 16 bit */
#define SPORT1_MRCS1			HALFWORD_REF(SPORT1_MRCS1_ADDR)
#define SPORT1_MRCS2_ADDR		0xffc02c26 /* SPORT 1 Multi-channel Receive select reg 16 bit */
#define SPORT1_MRCS2			HALFWORD_REF(SPORT1_MRCS2_ADDR)
#define SPORT1_MRCS3_ADDR		0xffc02c28 /* SPORT 1 Multi-channel Receive select reg 16 bit */
#define SPORT1_MRCS3			HALFWORD_REF(SPORT1_MRCS3_ADDR)
#define SPORT1_MRCS4_ADDR		0xffc02c2a /* SPORT 1 Multi-channel Receive select reg 16 bit */
#define SPORT1_MRCS4			HALFWORD_REF(SPORT1_MRCS4_ADDR)
#define SPORT1_MRCS5_ADDR		0xffc02c2c /* SPORT 1 Multi-channel Receive select reg 16 bit */
#define SPORT1_MRCS5			HALFWORD_REF(SPORT1_MRCS5_ADDR)
#define SPORT1_MRCS6_ADDR		0xffc02c2e /* SPORT 1 Multi-channel Receive select reg 16 bit */
#define SPORT1_MRCS6			HALFWORD_REF(SPORT1_MRCS6_ADDR)
#define SPORT1_MRCS7_ADDR		0xffc02c30 /* SPORT 1 Multi-channel Receive select reg 16 bit */
#define SPORT1_MRCS7			HALFWORD_REF(SPORT1_MRCS7_ADDR)
#define SPORT1_MCMC1_ADDR		0xffc02c32 /* SPORT 1 Multi-channel configuration reg 1 16 bit */
#define SPORT1_MCMC1			HALFWORD_REF(SPORT1_MCMC1_ADDR)
#define SPORT1_MCMC2_ADDR		0xffc02c34 /* SPORT 1 Multi-channel configuration reg 2 16 bit */
#define SPORT1_MCMC2			HALFWORD_REF(SPORT1_MCMC2_ADDR)

#define SPORT1_CURR_PTR_RX_ADDR		0xffc02e00 /* SPORT 1 RCV DMA Current pointer reg 16 bit*/
#define SPORT1_CURR_PTR_RX			HALFWORD_REF(SPORT1_CURR_PTR_RX_ADDR)
#define SPORT1_CONFIG_DMA_RX_ADDR	0xffc02e02 /* SPORT 1 RCV DMA Configuration reg 16 bit */

/* Fixed for SPORT driver, Jul 2 2003 */
#if 0
#define SPORT1_CONFIG_DMA_RX		HALFWORD_REF(SPORT1_CONFIG_RX_ADDR)
#else
#define SPORT1_CONFIG_DMA_RX		HALFWORD_REF(SPORT1_CONFIG_DMA_RX_ADDR)
#endif

#define SPORT1_START_ADDR_HI_RX_ADDR  	0xffc02e04 /* SPORT 1 RCV DMA start add. hi reg 16 bit */
#define SPORT1_START_ADDR_HI_RX		HALFWORD_REF(SPORT1_START_ADDR_HI_RX_ADDR)
#define SPORT1_START_ADDR_LO_RX_ADDR  	0xffc02e06 /* SPORT 1 RCV DMA start add. lo reg 16 bit */
#define SPORT1_START_ADDR_LO_RX		HALFWORD_REF(SPORT1_START_ADDR_LO_RX_ADDR)
#define SPORT1_COUNT_RX_ADDR  		0xffc02e08 /* SPORT 1 RCV DMA count reg 16 bit */
#define SPORT1_COUNT_RX				HALFWORD_REF(SPORT1_COUNT_RX_ADDR)
#define SPORT1_NEXT_DESCR_RX_ADDR  	0xffc02e0a /* SPORT 1 RCV DMA next descriptor poin reg 16 bit */
#define SPORT1_NEXT_DESCR_RX		HALFWORD_REF(SPORT1_NEXT_DESCR_RX_ADDR)
#define SPORT1_DESCR_RDY_RX_ADDR  	0xffc02e0c /* SPORT 1 RCV DMA descriptor ready reg 16 bit */
#define SPORT1_DESCR_RDY_RX			HALFWORD_REF(SPORT1_DESCR_RDY_RX_ADDR)
#define SPORT1_IRQSTAT_RX_ADDR  	0xffc02e0e /* SPORT 1 RCV DMA interrupt reg 16 bit */
#define SPORT1_IRQSTAT_RX			HALFWORD_REF(SPORT1_IRQSTAT_RX_ADDR)

#define SPORT1_CURR_PTR_TX_ADDR		0xffc02f00 /* SPORT 1 XMT DMA Current pointer reg 16 bit*/
#define SPORT1_CURR_PTR_TX			HALFWORD_REF(SPORT1_CURR_PTR_TX_ADDR)
#define SPORT1_CONFIG_DMA_TX_ADDR	0xffc02f02 /* SPORT 1 XMT DMA Configuration reg 16 bit */

#if 0
#define SPORT1_CONFIG_DMA_TX		HALFWORD_REF(SPORT1_CONFIG_TX_ADDR)
#else
#define SPORT1_CONFIG_DMA_TX		HALFWORD_REF(SPORT1_CONFIG_DMA_TX_ADDR)
#endif

#define SPORT1_START_ADDR_HI_TX_ADDR  	0xffc02f04 /* SPORT 1 XMT DMA start add. hi reg 16 bit */
#define SPORT1_START_ADDR_HI_TX		HALFWORD_REF(SPORT1_START_ADDR_HI_TX_ADDR)
#define SPORT1_START_ADDR_LO_TX_ADDR  	0xffc02f06 /* SPORT 1 XMT DMA start add. lo reg 16 bit */
#define SPORT1_START_ADDR_LO_TX		HALFWORD_REF(SPORT1_START_ADDR_LO_TX_ADDR)
#define SPORT1_COUNT_TX_ADDR  		0xffc02f08 /* SPORT 1 XMT DMA count reg 16 bit */
#define SPORT1_COUNT_TX				HALFWORD_REF(SPORT1_COUNT_TX_ADDR)
#define SPORT1_NEXT_DESCR_TX_ADDR  	0xffc02f0a /* SPORT 1 XMT DMA next descriptor poin reg 16 bit */
#define SPORT1_NEXT_DESCR_TX		HALFWORD_REF(SPORT1_NEXT_DESCR_TX_ADDR)
#define SPORT1_DESCR_RDY_TX_ADDR  	0xffc02f0c /* SPORT 1 XMT DMA descriptor ready reg 16 bit */
#define SPORT1_DESCR_RDY_TX			HALFWORD_REF(SPORT1_DESCR_RDY_TX_ADDR)
#define SPORT1_IRQSTAT_TX_ADDR  	0xffc02f0e /* SPORT 1 XMT DMA interrupt reg 16 bit */
#define SPORT1_IRQSTAT_TX			HALFWORD_REF(SPORT1_IRQSTAT_TX_ADDR)

/****************************
 *
 *  SPI 0 CONTROLLER REGISTERS  (0XFFC03000 - 0XFFC033FF)
 *
 ****************************/

#define SPI0_CTL_ADDR		0xffc03000 /* SPI 0 control register 16 bit */
#define SPI0_CTL			HALFWORD_REF(SPI0_CTL_ADDR)
#define SPI0_FLG_ADDR		0xffc03002 /* SPI 0 flag register 16 bit */
#define SPI0_FLG			HALFWORD_REF(SPI0_FLG_ADDR)
#define SPI0_ST_ADDR		0xffc03004 /* SPI 0 status register 16 bit */
#define SPI0_ST				HALFWORD_REF(SPI0_ST_ADDR)
#define SPI0_TDBR_ADDR		0xffc03006 /* SPI 0 transmit data buffer register 16 bit */
#define SPI0_TDBR			HALFWORD_REF(SPI0_TDBR_ADDR)
#define SPI0_RDBR_ADDR		0xffc03008 /* SPI 0 receive data buffer register 16 bit */
#define SPI0_RDBR			HALFWORD_REF(SPI0_RDBR_ADDR)
#define SPI0_BAUD_ADDR		0xffc0300a /* SPI 0 baud rate register 16 bit */
#define SPI0_BAUD			HALFWORD_REF(SPI0_BAUD_ADDR)
#define SPI0_SHADOW_ADDR	0xffc0300c /* SPI 0 RDBR register 16 bit */
#define SPI0_SHADOW			HALFWORD_REF(SPI0_SHADOW_ADDR)

#define SPI0_CURR_PTR_ADDR		0xffc03200 /* SPI 0 DMA current pointer register 16 bit */
#define SPI0_CURR_PTR			HALFWORD_REF(SPI0_CURR_PTR_ADDR)
#define SPI0_CONFIG_ADDR		0xffc03202 /* SPI 0 DMA configuration register 16 bit */
#define SPI0_CONFIG				HALFWORD_REF(SPI0_CONFIG_ADDR)
#define SPI0_START_ADDR_HI_ADDR	0xffc03204 /* SPI 0 DMA start address hi register 16 bit */
#define SPI0_START_ADDR_HI		HALFWORD_REF(SPI0_START_ADDR_HI_ADDR)
#define SPI0_START_ADDR_LO_ADDR	0xffc03206 /* SPI 0 DMA start address lo register 16 bit */
#define SPI0_START_ADDR_LO		HALFWORD_REF(SPI0_START_ADDR_LO_ADDR)
#define SPI0_COUNT_ADDR			0xffc03208 /* SPI 0 DMA count register 16 bit */
#define SPI0_COUNT				HALFWORD_REF(SPI0_COUNT_ADDR)
#define SPI0_NEXT_DESCR_ADDR	0xffc0320a /* SPI 0 DMA Next descriptor pointer register 16 bit */
#define SPI0_NEXT_DESCR			HALFWORD_REF(SPI0_NEXT_DESCR_ADDR)
#define SPI0_DESCR_RDY_ADDR		0xffc0320c /* SPI 0 DMA descriptor ready register 16 bit */
#define SPI0_DESCR_RDY			HALFWORD_REF(SPI0_DESCR_RDY_ADDR)
#define SPI0_DMA_INT_ADDR		0xffc0320e /* SPI 0 DMA interrupt register 16 bit */
#define SPI0_DMA_INT			HALFWORD_REF(SPI0_DMA_INT_ADDR)

/****************************
 *
 *  SPI 1 CONTROLLER REGISTERS  (0XFFC03400 - 0XFFC037FF)
 *
 ****************************/

#define SPI1_CTL_ADDR		0xffc03400 /* SPI 1 control register 16 bit */
#define SPI1_CTL			HALFWORD_REF(SPI1_CTL_ADDR)
#define SPI1_FLG_ADDR		0xffc03402 /* SPI 1 flag register 16 bit */
#define SPI1_FLG			HALFWORD_REF(SPI1_FLG_ADDR)
#define SPI1_ST_ADDR		0xffc03404 /* SPI 1 status register 16 bit */
#define SPI1_ST				HALFWORD_REF(SPI1_ST_ADDR)
#define SPI1_TDBR_ADDR		0xffc03406 /* SPI 1 transmit data buffer register 16 bit */
#define SPI1_TDBR			HALFWORD_REF(SPI1_TDBR_ADDR)
#define SPI1_RDBR_ADDR		0xffc03408 /* SPI 1 receive data buffer register 16 bit */
#define SPI1_RDBR			HALFWORD_REF(SPI1_RDBR_ADDR)
#define SPI1_BAUD_ADDR		0xffc0340a /* SPI 1 baud rate register 16 bit */
#define SPI1_BAUD			HALFWORD_REF(SPI1_BAUD_ADDR)
#define SPI1_SHADOW_ADDR	0xffc0340c /* SPI 1 RDBR register 16 bit */
#define SPI1_SHADOW			HALFWORD_REF(SPI1_SHADOW_ADDR)

#define SPI1_CURR_PTR_ADDR		0xffc03600 /* SPI 1 DMA current pointer register 16 bit */
#define SPI1_CURR_PTR			HALFWORD_REF(SPI1_CURR_PTR_ADDR)
#define SPI1_CONFIG_ADDR		0xffc03602 /* SPI 1 DMA configuration register 16 bit */
#define SPI1_CONFIG				HALFWORD_REF(SPI1_CONFIG_ADDR)
#define SPI1_START_ADDR_HI_ADDR	0xffc03604 /* SPI 1 DMA start address hi register 16 bit */
#define SPI1_START_ADDR_HI		HALFWORD_REF(SPI1_START_ADDR_HI_ADDR)
#define SPI1_START_ADDR_LO_ADDR	0xffc03606 /* SPI 1 DMA start address lo register 16 bit */
#define SPI1_START_ADDR_LO		HALFWORD_REF(SPI1_START_ADDR_LO_ADDR)
#define SPI1_COUNT_ADDR			0xffc03608 /* SPI 1 DMA count register 16 bit */
#define SPI1_COUNT				HALFWORD_REF(SPI1_COUNT_ADDR)
#define SPI1_NEXT_DESCR_ADDR	0xffc0360a /* SPI 1 DMA Next descriptor pointer register 16 bit */
#define SPI1_NEXT_DESCR			HALFWORD_REF(SPI1_NEXT_DESCR_ADDR)
#define SPI1_DESCR_RDY_ADDR		0xffc0360c /* SPI 1 DMA descriptor ready register 16 bit */
#define SPI1_DESCR_RDY			HALFWORD_REF(SPI1_DESCR_RDY_ADDR)
#define SPI1_DMA_INT_ADDR		0xffc0360e /* SPI 1 DMA interrupt register 16 bit */
#define SPI1_DMA_INT			HALFWORD_REF(SPI1_DMA_INT_ADDR)

/****************************
 *
 *  MEMORY DMA CONTROLLER REGISTERS  (0XFFC03800 - 0XFFC03BFF)
 *
 ****************************/

#define MDW_DCP_ADDR		0xffc03800 /* Current pointer write channel register 16 bit */
#define MDW_DCP				HALFWORD_REF(MDW_DCP_ADDR)
#define MDW_DCFG_ADDR		0xffc03802 /* DMA configuration write channel register 16 bit */
#define MDW_DCFG			HALFWORD_REF(MDW_DCFG_ADDR)
#define MDW_DSAH_ADDR		0xffc03804 /* Start address hi write channel register 16 bit */
#define MDW_DSAH			HALFWORD_REF(MDW_DSAH_ADDR)
#define MDW_DSAL_ADDR		0xffc03806 /* Start address lo write channel register 16 bit */
#define MDW_DSAL			HALFWORD_REF(MDW_DSAL_ADDR)
#define MDW_DCT_ADDR		0xffc03808 /* DMA count write channel register 16 bit */
#define MDW_DCT				HALFWORD_REF(MDW_DCT_ADDR)
#define MDW_DND_ADDR		0xffc0380a /* Next descriptor pointer write channel register 16 bit */
#define MDW_DND				HALFWORD_REF(MDW_DND_ADDR)
#define MDW_DDR_ADDR		0xffc0380c /* Descriptor ready write channel register 16 bit */
#define MDW_DDR				HALFWORD_REF(MDW_DDR_ADDR)
#define MDW_DI_ADDR			0xffc0380e /* DMA interrupt write channel register 16 bit */
#define MDW_DI				HALFWORD_REF(MDW_DI_ADDR)

#define MDR_DCP_ADDR		0xffc03900 /* Current pointer read channel register 16 bit */
#define MDR_DCP				HALFWORD_REF(MDR_DCP_ADDR)
#define MDR_DCFG_ADDR		0xffc03902 /* DMA configuration read channel register 16 bit */
#define MDR_DCFG			HALFWORD_REF(MDR_DCFG_ADDR)
#define MDR_DSAH_ADDR		0xffc03904 /* Start address hi read channel register 16 bit */
#define MDR_DSAH			HALFWORD_REF(MDR_DSAH_ADDR)
#define MDR_DSAL_ADDR		0xffc03906 /* Start address lo read channel register 16 bit */
#define MDR_DSAL			HALFWORD_REF(MDR_DSAL_ADDR)
#define MDR_DCT_ADDR		0xffc03908 /* DMA count read channel register 16 bit */
#define MDR_DCT				HALFWORD_REF(MDR_DCT_ADDR)
#define MDR_DND_ADDR		0xffc0390a /* Next descriptor pointer read channel register 16 bit */
#define MDR_DND				HALFWORD_REF(MDR_DND_ADDR)
#define MDR_DDR_ADDR		0xffc0390c /* Descriptor ready read channel register 16 bit */
#define MDR_DDR				HALFWORD_REF(MDR_DDR_ADDR)
#define MDR_DI_ADDR			0xffc0390e /* DMA interrupt read channel register 16 bit */
#define MDR_DI				HALFWORD_REF(MDR_DI_ADDR)

/****************************
 *
 *  ASYNCHRONOUS MEMORY CONTROLLER REGISTERS EDIU (0XFFC03C00 - 0XFFC03FFF)
 *
 ****************************/

#define EBIU_AMGCTL_ADDR	0xffc03c00 /* Asynchronous memory global control register ? bit */
#define EBIU_AMGCTL			WORD_REF(MDW_AMGCTL_ADDR)
#define EBIU_AMBCTL0_ADDR	0xffc03c04 /* Asynchronous memory bank control register 0 32 bit */
#define EBIU_AMBCTL0		WORD_REF(MDW_AMBCTL0_ADDR)
#define EBIU_AMBCTL1_ADDR	0xffc03c08 /* Asynchronous memory bank control register 1 32 bit */
#define EBIU_AMBCTL1		WORD_REF(MDW_AMBCTL1_ADDR)

/****************************
 *
 *  PCI BRIDGE PAB REGISTERS  (0XFFC04000 - 0XFFC043FF)
 *
 ****************************/

#define PCI_CTL_ADDR		0xffc04000 /* PCI bridge control register 32 bit */
#define PCI_CTL				WORD_REF(PCI_CTL_ADDR)
#define PCI_STAT_ADDR		0xffc04004 /* PCI bridge status register 32 bit */
#define PCI_STAT			WORD_REF(PCI_STAT_ADDR)
#define PCI_ICTL_ADDR		0xffc04008 /* PCI bridge interrupt control register 32 bit */
#define PCI_ICTL			WORD_REF(PCI_ICTL_ADDR)
#define PCI_MBAP_ADDR		0xffc0400c /* PCI memory space base address pointer[31:27] register 32 bit */
#define PCI_MBAP			WORD_REF(PCI_MBAP_ADDR)
#define PCI_IBAP_ADDR		0xffc04010 /* PCI IO space base address pointer register 32 bit */
#define PCI_IBAP			WORD_REF(PCI_IBAP_ADDR)
#define PCI_CPAB_ADDR		0xffc04014 /* PCI config space base address port register 32 bit */
#define PCI_CPAB			WORD_REF(PCI_CPAB_ADDR)
#define PCI_TMBAP_ADDR		0xffc04018 /* PCI to Tahoe memory base address pointer register 32 bit */
#define PCI_TMBAP			WORD_REF(PCI_TMBAP_ADDR)
#define PCI_TIBAP_ADDR		0xffc0401c /* PCI to Tahoe IO base address pointer register 32 bit */
#define PCI_TIBAP			WORD_REF(PCI_TIBAP_ADDR)

/****************************
 *
 *  PCI BRIDGE EXTERNAL ACCESS BUS REGISTERS  (0XEEFFFF00 - 0XEEFFFFFF)
 *
 ****************************/

#define PCI_DMBARM_ADDR		0xeeffff00 /* PCI Device memory bar mask register 32 bit */
#define PCI_DMBARM			WORD_REF(PCI_DMBARM_ADDR)
#define PCI_DIBARM_ADDR		0xeeffff04 /* PCI Device IO bar mask register 32 bit */
#define PCI_DIBARM			WORD_REF(PCI_DIBARM_ADDR)
#define PCI_CFG_DIC_ADDR	0xeeffff08 /* PCI config Device ID register 32 bit */
#define PCI_CFG_DIC			WORD_REF(PCI_CFG_DIC_ADDR)
#define PCI_CFG_VIC_ADDR	0xeeffff0c /* PCI config vendor ID register 32 bit */
#define PCI_CFG_VIC			WORD_REF(PCI_CFG_VIC_ADDR)
#define PCI_CFG_STAT_ADDR	0xeeffff10 /* PCI config status (read only) register 32 bit */
#define PCI_CFG_STAT		WORD_REF(PCI_CFG_STAT_ADDR)
#define PCI_CFG_CMD_ADDR	0xeeffff14 /* PCI config command register 32 bit */
#define PCI_CFG_CMD			WORD_REF(PCI_CFG_CMD_ADDR)
#define PCI_CFG_CC_ADDR		0xeeffff18 /* PCI config class code register 32 bit */
#define PCI_CFG_CC			WORD_REF(PCI_CFG_CC_ADDR)
#define PCI_CFG_RID_ADDR	0xeeffff1C /* PCI config revision ID register 32 bit */
#define PCI_CFG_RID			WORD_REF(PCI_CFG_RID_ADDR)
#define PCI_CFG_BIST_ADDR	0xeeffff20 /* PCI config BIST register 32 bit */
#define PCI_CFG_BIST		WORD_REF(PCI_CFG_BIST_ADDR)
#define PCI_CFG_HT_ADDR		0xeeffff24 /* PCI config header type register 32 bit */
#define PCI_CFG_HT			WORD_REF(PCI_CFG_HT_ADDR)
#define PCI_CFG_MLT_ADDR	0xeeffff28 /* PCI config memory latency timer register 32 bit */
#define PCI_CFG_MLT			WORD_REF(PCI_CFG_MLT_ADDR)
#define PCI_CFG_CLS_ADDR	0xeeffff2C /* PCI config cache line (block) size register 32 bit */
#define PCI_CFG_CLS			WORD_REF(PCI_CFG_CLS_ADDR)
#define PCI_CFG_MBAR_ADDR	0xeeffff30 /* PCI config memory base address register 32 bit */
#define PCI_CFG_MBAR		WORD_REF(PCI_CFG_MBAR_ADDR)
#define PCI_CFG_IBAR_ADDR	0xeeffff34 /* PCI config IO base address register 32 bit */
#define PCI_CFG_IBAR		WORD_REF(PCI_CFG_IBAR_ADDR)
#define PCI_CFG_SID_ADDR	0xeeffff38 /* PCI config subsystem ID register 32 bit */
#define PCI_CFG_SID			WORD_REF(PCI_CFG_SID_ADDR)
#define PCI_CFG_SVID_ADDR	0xeeffff3C /* PCI config subsystem vendor ID register 32 bit */
#define PCI_CFG_SVID		WORD_REF(PCI_CFG_SVID_ADDR)
#define PCI_CFG_MAXL_ADDR	0xeeffff40 /* PCI config maximum latency cycles register 32 bit */
#define PCI_CFG_MAXL		WORD_REF(PCI_CFG_MAXL_ADDR)
#define PCI_CFG_MING_ADDR	0xeeffff44 /* PCI config minimum grant cycles register 32 bit */
#define PCI_CFG_MING		WORD_REF(PCI_CFG_MING_ADDR)
#define PCI_CFG_IP_ADDR		0xeeffff48 /* PCI config interrupt pin register 32 bit */
#define PCI_CFG_IP			WORD_REF(PCI_CFG_IP_ADDR)
#define PCI_CFG_IL_ADDR		0xeeffff4C /* PCI config interrupt line register 32 bit */
#define PCI_CFG_IL			WORD_REF(PCI_CFG_IL_ADDR)
#define PCI_HMCTL_ADDR		0xeeffff50 /* PCI config blocking BAR mask 1 (host only) register 32 bit */
#define PCI_HMCTL			WORD_REF(PCI_HMCTL_ADDR)

/****************************
 *
 *  UNIVERSAL SERIAL BUS INTERFACE (USB) REGISTERS  (0XFFC04400 - 0XFFC047FF)
 *
 ****************************/

#define USBD_ID_ADDR		0xffc04400 /* USB Device ID register 16 bit */
#define USBD_ID				HALFWORD_REF(USBD_ID_ADDR)
#define USBD_FRM_ADDR		0xffc04402 /* Current USB frame number register 16 bit */
#define USBD_FRM			HALFWORD_REF(USBD_FRM_ADDR)
#define USBD_FRMAT_ADDR		0xffc04404 /* Match value for USB frame number register 16 bit */
#define USBD_FRMAT			HALFWORD_REF(USBD_FRMAT_ADDR)
#define USBD_EPBUF_ADDR		0xffc04406 /* Enables download of configuration into UDC core register 16 bit */
#define USBD_EPBUF			HALFWORD_REF(USBD_EPBUF_ADDR)
#define USBD_STAT_ADDR		0xffc04408 /* Return USBD Module status register 16 bit */
#define USBD_STAT			HALFWORD_REF(USBD_STAT_ADDR)
#define USBD_CTRL_ADDR		0xffc0440a /* Allows configuration & control of USBD module register 16 bit */
#define USBD_CTRL			HALFWORD_REF(USBD_CTRL_ADDR)
#define USBD_GINTR_ADDR		0xffc0440c /* Global interrupt register 16 bit */
#define USBD_GINTR			HALFWORD_REF(USBD_GINTR_ADDR)
#define USBD_GMASK_ADDR		0xffc0440e /* Global interrupt register mask register 16 bit */
#define USBD_GMASK			HALFWORD_REF(USBD_GMASK_ADDR)

#define USBD_DMACFG_ADDR	0xffc04440 /* DMA Master channel configuration register 16 bit */
#define USBD_DMACFG			HALFWORD_REF(USBD_DMACFG_ADDR)
#define USBD_DMABL_ADDR		0xffc04442 /* DMA Master channel base address low register 16 bit */
#define USBD_DMABL			HALFWORD_REF(USBD_DMABL_ADDR)
#define USBD_DMABH_ADDR		0xffc04444 /* DMA Master channel base address high register 16 bit */
#define USBD_DMABH			HALFWORD_REF(USBD_DMABH_ADDR)
#define USBD_DMACT_ADDR		0xffc04446 /* DMA Master channel count register 16 bit */
#define USBD_DMACT			HALFWORD_REF(USBD_DMACT_ADDR)
#define USBD_DMAIRQ_ADDR	0xffc04448 /* DMA Master channel IRQ register 16 bit */
#define USBD_DMAIRQ			HALFWORD_REF(USBD_DMAIRQ_ADDR)

#define USBD_INTR0_ADDR		0xffc04480 /* USB Endpoint 0 interrupt register 16 bit */
#define USBD_INTR0			HALFWORD_REF(USBD_INTR0_ADDR)
#define USBD_MASK0_ADDR		0xffc04482 /* USB Endpoint 0 mask register 16 bit */
#define USBD_MASK0			HALFWORD_REF(USBD_MASK0_ADDR)
#define USBD_EPCFG0_ADDR	0xffc04484 /* USB Endpoint 0 control register 16 bit */
#define USBD_EPCFG0			HALFWORD_REF(USBD_EPCFG0_ADDR)
#define USBD_EPADR0_ADDR	0xffc04486 /* USB Endpoint 0 address offset register 16 bit */
#define USBD_EPADR0			HALFWORD_REF(USBD_EPADR0_ADDR)
#define USBD_EPLEN0_ADDR	0xffc04488 /* USB Endpoint 0 buffer length register 16 bit */
#define USBD_EPLEN0			HALFWORD_REF(USBD_EPLEN0_ADDR)

#define USBD_INTR1_ADDR		0xffc0448a /* USB Endpoint 1 interrupt register 16 bit */
#define USBD_INTR1			HALFWORD_REF(USBD_INTR1_ADDR)
#define USBD_MASK1_ADDR		0xffc0448c /* USB Endpoint 1 mask register 16 bit */
#define USBD_MASK1			HALFWORD_REF(USBD_MASK1_ADDR)
#define USBD_EPCFG1_ADDR	0xffc0448e /* USB Endpoint 1 control register 16 bit */
#define USBD_EPCFG1			HALFWORD_REF(USBD_EPCFG1_ADDR)
#define USBD_EPADR1_ADDR	0xffc04490 /* USB Endpoint 1 address offset register 16 bit */
#define USBD_EPADR1			HALFWORD_REF(USBD_EPADR1_ADDR)
#define USBD_EPLEN1_ADDR	0xffc04492 /* USB Endpoint 1 buffer length register 16 bit */
#define USBD_EPLEN1			HALFWORD_REF(USBD_EPLEN1_ADDR)

#define USBD_INTR2_ADDR		0xffc04494 /* USB Endpoint 2 interrupt register 16 bit */
#define USBD_INTR2			HALFWORD_REF(USBD_INTR2_ADDR)
#define USBD_MASK2_ADDR		0xffc04496 /* USB Endpoint 2 mask register 16 bit */
#define USBD_MASK2			HALFWORD_REF(USBD_MASK2_ADDR)
#define USBD_EPCFG2_ADDR	0xffc04498 /* USB Endpoint 2 control register 16 bit */
#define USBD_EPCFG2			HALFWORD_REF(USBD_EPCFG2_ADDR)
#define USBD_EPADR2_ADDR	0xffc0449a /* USB Endpoint 2 address offset register 16 bit */
#define USBD_EPADR2			HALFWORD_REF(USBD_EPADR2_ADDR)
#define USBD_EPLEN2_ADDR	0xffc0449c /* USB Endpoint 2 buffer length register 16 bit */
#define USBD_EPLEN2			HALFWORD_REF(USBD_EPLEN2_ADDR)

/* Fixed by HuTao, Jun18 2003 4:35PM */
#if 0
#define USBD_INTR3_ADDR		0xffc0448e /* USB Endpoint 3 interrupt register 16 bit */
#else
#define USBD_INTR3_ADDR		0xffc0449e /* USB Endpoint 3 interrupt register 16 bit */
#endif

#define USBD_INTR3			HALFWORD_REF(USBD_INTR3_ADDR)
#define USBD_MASK3_ADDR		0xffc044a0 /* USB Endpoint 3 mask register 16 bit */
#define USBD_MASK3			HALFWORD_REF(USBD_MASK3_ADDR)
#define USBD_EPCFG3_ADDR	0xffc044a2 /* USB Endpoint 3 control register 16 bit */
#define USBD_EPCFG3			HALFWORD_REF(USBD_EPCFG3_ADDR)
#define USBD_EPADR3_ADDR	0xffc044a4 /* USB Endpoint 3 address offset register 16 bit */
#define USBD_EPADR3			HALFWORD_REF(USBD_EPADR3_ADDR)
#define USBD_EPLEN3_ADDR	0xffc044a6 /* USB Endpoint 3 buffer length register 16 bit */
#define USBD_EPLEN3			HALFWORD_REF(USBD_EPLEN3_ADDR)

#define USBD_INTR4_ADDR		0xffc044a8 /* USB Endpoint 4 interrupt register 16 bit */
#define USBD_INTR4			HALFWORD_REF(USBD_INTR4_ADDR)
#define USBD_MASK4_ADDR		0xffc044aa /* USB Endpoint 4 mask register 16 bit */
#define USBD_MASK4			HALFWORD_REF(USBD_MASK4_ADDR)
#define USBD_EPCFG4_ADDR	0xffc044ac /* USB Endpoint 4 control register 16 bit */
#define USBD_EPCFG4			HALFWORD_REF(USBD_EPCFG4_ADDR)
#define USBD_EPADR4_ADDR	0xffc044ae /* USB Endpoint 4 address offset register 16 bit */
#define USBD_EPADR4			HALFWORD_REF(USBD_EPADR4_ADDR)
#define USBD_EPLEN4_ADDR	0xffc044b0 /* USB Endpoint 4 buffer length register 16 bit */
#define USBD_EPLEN4			HALFWORD_REF(USBD_EPLEN4_ADDR)

#define USBD_INTR5_ADDR		0xffc044b2 /* USB Endpoint 5 interrupt register 16 bit */
#define USBD_INTR5			HALFWORD_REF(USBD_INTR5_ADDR)
#define USBD_MASK5_ADDR		0xffc044b4 /* USB Endpoint 5 mask register 16 bit */
#define USBD_MASK5			HALFWORD_REF(USBD_MASK5_ADDR)
#define USBD_EPCFG5_ADDR	0xffc044b6 /* USB Endpoint 5 control register 16 bit */
#define USBD_EPCFG5			HALFWORD_REF(USBD_EPCFG5_ADDR)
#define USBD_EPADR5_ADDR	0xffc044b8 /* USB Endpoint 5 address offset register 16 bit */
#define USBD_EPADR5			HALFWORD_REF(USBD_EPADR5_ADDR)
#define USBD_EPLEN5_ADDR	0xffc044ba /* USB Endpoint 5 buffer length register 16 bit */
#define USBD_EPLEN5			HALFWORD_REF(USBD_EPLEN5_ADDR)

#define USBD_INTR6_ADDR		0xffc044bc /* USB Endpoint 6 interrupt register 16 bit */
#define USBD_INTR6			HALFWORD_REF(USBD_INTR6_ADDR)
#define USBD_MASK6_ADDR		0xffc044be /* USB Endpoint 6 mask register 16 bit */
#define USBD_MASK6			HALFWORD_REF(USBD_MASK6_ADDR)
#define USBD_EPCFG6_ADDR	0xffc044c0 /* USB Endpoint 6 control register 16 bit */
#define USBD_EPCFG6			HALFWORD_REF(USBD_EPCFG6_ADDR)
#define USBD_EPADR6_ADDR	0xffc044c2 /* USB Endpoint 6 address offset register 16 bit */
#define USBD_EPADR6			HALFWORD_REF(USBD_EPADR6_ADDR)
#define USBD_EPLEN6_ADDR	0xffc044c4 /* USB Endpoint 6 buffer length register 16 bit */
#define USBD_EPLEN6			HALFWORD_REF(USBD_EPLEN6_ADDR)

#define USBD_INTR7_ADDR		0xffc044c6 /* USB Endpoint 7 interrupt register 16 bit */
#define USBD_INTR7			HALFWORD_REF(USBD_INTR7_ADDR)
#define USBD_MASK7_ADDR		0xffc044c8 /* USB Endpoint 7 mask register 16 bit */
#define USBD_MASK7			HALFWORD_REF(USBD_MASK7_ADDR)
#define USBD_EPCFG7_ADDR	0xffc044ca /* USB Endpoint 7 control register 16 bit */
#define USBD_EPCFG7			HALFWORD_REF(USBD_EPCFG7_ADDR)
#define USBD_EPADR7_ADDR	0xffc044cc /* USB Endpoint 7 address offset register 16 bit */
#define USBD_EPADR7			HALFWORD_REF(USBD_EPADR7_ADDR)
#define USBD_EPLEN7_ADDR	0xffc044ce /* USB Endpoint 7 buffer length register 16 bit */
#define USBD_EPLEN7			HALFWORD_REF(USBD_EPLEN7_ADDR)

/****************************
 *
 * SYSTEM BUS INTERFACE UNIT REGISTERS  (0XFFC04800 - 0XFFC04FFF)
 *
 ****************************/

#define L1SBAR_ADDR		0xffc04840 /* L1 SRAM base address register 16 bit */
#define L1SBAR			HALFWORD_REF(L1SBAR_ADDR)
#define L1CSR_ADDR		0xffc04844 /* L1 SRAM control initialization register 32 bit */
#define L1CSR			WORD_REF(L1CSR_ADDR)
#define DB_NDBP_ADDR		0xffc04880 /* Next descriptor base pointer register 32 bit */
#define DB_NDBP			WORD_REF(DB_NDBP_ADDR)
#define DB_ACOMP_ADDR		0xffc04884 /* DMA bus address comparator register 32 bit */
#define DB_ACOMP		WORD_REF(DB_ACOMP_ADDR)
#define DB_CCOMP_ADDR		0xffc04888 /* DMA bus control comparator register 32 bit */
#define DB_CCOMP		WORD_REF(DB_CCOMP_ADDR)

/****************************
 *
 * SDRAM CONTROLLER EXTERNAL BUS INTERFACE REGISTERS  (0XFFC04C00 - 0XFFC04FFF)
 *
 ****************************/

#define EBIU_SDGCTL_ADDR	0xffc04c00 /* SDRAM Memory global control register 32 bit */
#define EBIU_SDGCTL		WORD_REF(EBIU_SDGCTL_ADDR)
#define EBIU_SDBCTL_ADDR	0xffc04c00 /* SDRAM Memory bank control register 32 bit */
#define EBIU_SDBCTL		WORD_REF(EBIU_SDBCTL_ADDR)
#define EBIU_SDRRC_ADDR		0xffc04c00 /* SDRAM Memory refresh rate count register 16 bit */
#define EBIU_SDRRC		HALFWORD_REF(EBIU_SDRRC_ADDR)
#define EBIU_SDSTAT_ADDR	0xffc04c00 /* SDRAM control status register 16 bit */
#define EBIU_SDSTAT		HALFWORD_REF(EBIU_SDSTAT_ADDR)


 /*      FOLLOWING ARE THE FRIO CORE MMR REGISTERS (TOP 2 MB)     */

/****************************
 *
 *  L1 DATA MEMORY CONTROLLER REGISTERS  (0XFFE00000 - 0XFFE00404)
 *
 ****************************/

#define SRAM_BASE_ADDR_ADDR	0xffe00000 /* read only register 32 bit */
#define SRAM_BASE_ADDR		WORD_REF(SRAM_BASE_ADDR_ADDR)
#define DATA_FAULT_STATUS_ADDR	0xffe00008 /* read only register 32 bit */
#define DATA_FAULT_STATUS	WORD_REF(DATA_FAULT_STATUS_ADDR)
#define DATA_FAULT_ADDR_ADDR	0xffe0000c /* read only register 32 bit */
#define DATA_FAULT_ADDR		WORD_REF(DATA_FAULT_ADDR_ADDR)

		/* These are total sixtenn */
		
#define DCPLB_ADDR_ADDR		0xffe00100 /* read/write register 32 bit */
#define DCPLB_ADDR		WORD_REF(DCPLB_ADDR_ADDR)

		/* These are total sixtenn */
		
#define DCPLB_DATA_ADDR		0xffe00200 /* read/write register 32 bit */
#define DCPLB_DATA		WORD_REF(DCPLB_DATA_ADDR)


		/* These are total 4 */

#define DTEST_DATA_ADDR		0xffe00400 /* read/write register 32 bit */
#define DTEST_DATA		WORD_REF(DTEST_DATA_ADDR)

/****************************
 *
 *  L1 CODE MEMORY CONTROLLER REGISTERS  (0XFFE01004 - 0XFFE1404)
 *
 ****************************/

#define CODE_FAULT_STATUS_ADDR	0xffe01008 /* read only register 32 bit */
#define CODE_FAULT_STATUS	WORD_REF(CODE_FAULT_STATUS_ADDR)
#define CODE_FAULT_ADDR_ADDR	0xffe0100c /* read only register 32 bit */
#define CODE_FAULT_ADDR		WORD_REF(CODE_FAULT_ADDR_ADDR)

		/* These are total sixtenn */
		
#define ICPLB_ADDR_ADDR		0xffe01100 /* read/write register 32 bit */
#define ICPLB_ADDR		WORD_REF(ICPLB_ADDR_ADDR)

		/* These are total sixtenn */
		
#define ICPLB_DATA_ADDR		0xffe01200 /* read/write register 32 bit */
#define ICPLB_DATA		WORD_REF(ICPLB_DATA_ADDR)


		/* These are total 4 */

#define ITEST_DATA_ADDR		0xffe01400 /* read/write register 32 bit */
#define ITEST_DATA		WORD_REF(ITEST_DATA_ADDR)

/****************************
 *
 *  INTERRUPT CONTROLLER REGISTERS  (0XFFE02000 - 0XFFE0210C)
 *
 ****************************/

 /*   EVENT VECTOR TABLE IS USED FOR 16 INTERRUPTS AS SHOWN BELOW
      ALSO THEIR PRIORITY SEQUENCE IS ALSO GIVEN HERE
      1		EMULATION		HIGHEST
      2		RESET
      3		NMI
      4		EXCEPTION
      5		INTERRUPTS		LOWEST */

/*  Used with JTAG register 32 bit */
#define EVT_EMULATION_ADDR	0xffe02000 
#define EVT_EMULATION		WORD_REF(EVT_EMULATION_ADDR)
/*  register 32 bit */
#define EVT_RESET_ADDR		0xffe02004 
#define EVT_RESET		WORD_REF(EVT_RESET_ADDR)
/*  NMI register 32 bit */
#define EVT_NMI_ADDR		0xffe02008 
#define EVT_NMI			WORD_REF(EVT_NMI_ADDR)
/*  Identification with code in EXCAUSE register 32 bit */
#define EVT_EXCEPTION_ADDR	0xffe0200c 
#define EVT_EXCEPTION		WORD_REF(EVT_EXCEPTION_ADDR)
/*  Global interrupt enable register 32 bit */
#define EVT_GLOBAL_INT_ENB_ADDR	0xffe02010 
#define EVT_GLOBAL_INT_ENB	WORD_REF(EVT_GLOBAL_INT_ENB_ADDR)
/*  Active while error condition exsists register 32 bit */
#define EVT_HARDWARE_ERROR_ADDR	0xffe02014 
#define EVT_HARDWARE_ERROR	WORD_REF(EVT_HARDWARE_ERROR_ADDR)
/*  High priority timer register 32 bit */
#define EVT_TIMER_ADDR		0xffe02018 
#define EVT_TIMER		WORD_REF(EVT_TIMER_ADDR)
/*  General purpose interrupt register 32 bit */
#define EVT_IVG7_ADDR		0xffe0201c 
#define EVT_IVG7		WORD_REF(EVT_IVG7_ADDR)
/*  General purpose interrupt register 32 bit */
#define EVT_IVG8_ADDR		0xffe02020 
#define EVT_IVG8		WORD_REF(EVT_IVG8_ADDR)
/*  General purpose interrupt register 32 bit */
#define EVT_IVG9_ADDR		0xffe02024 
#define EVT_IVG9		WORD_REF(EVT_IVG9_ADDR)
/*  General purpose interrupt register 32 bit */
#define EVT_IVG10_ADDR		0xffe02028 
#define EVT_IVG10		WORD_REF(EVT_IVG10_ADDR)
/*  General purpose interrupt register 32 bit */
#define EVT_IVG11_ADDR		0xffe0202c 
#define EVT_IVG11		WORD_REF(EVT_IVG11_ADDR)
/*  General purpose interrupt register 32 bit */
#define EVT_IVG12_ADDR		0xffe02030 
#define EVT_IVG12		WORD_REF(EVT_IVG12_ADDR)
/*  General purpose interrupt register 32 bit */
#define EVT_IVG13_ADDR		0xffe02034 
#define EVT_IVG13		WORD_REF(EVT_IVG13_ADDR)

/* IVG14 & IVG15 should be used as software interrupts  */

/*  General purpose interrupt register 32 bit */
#define EVT_IVG14_ADDR		0xffe02038 
#define EVT_IVG14		WORD_REF(EVT_IVG14_ADDR)
/*  General purpose interrupt register 32 bit */
#define EVT_IVG15_ADDR		0xffe0203c 
#define EVT_IVG15		WORD_REF(EVT_IVG15_ADDR)

#define EVT_OVERRIDE_ADDR	0xffe02100 /*  register 32 bit */
#define EVT_OVERRIDE		WORD_REF(EVT_OVERRIDE_ADDR)
#define IMASK_ADDR		0xffe02104 /*  register 32 bit */
#define IMASK			WORD_REF(IMASK_ADDR)
#define IPEND_ADDR		0xffe02108 /*  register 32 bit */
#define IPEND			WORD_REF(IPEND_ADDR)
#define ILAT_ADDR		0xffe0210c /*  register 32 bit */
#define ILAT			WORD_REF(ILAT_ADDR)

/****************************
 *
 *  TIMER REGISTERS  (0XFFC03000 - 0XFFC0300C)
 *
 ****************************/

#define TCNTL_ADDR		0xffe03000 /*  register 32 bit */
#define TCNTL			WORD_REF(TCNTL_ADDR)
#define TPERIOD_ADDR		0xffe03004 /*  register 32 bit */
#define TPERIOD			WORD_REF(TPERIOD_ADDR)
#define TSCALE_ADDR		0xffe03008 /*  register 32 bit */
#define TSCALE			WORD_REF(TSCALE_ADDR)
#define TCOUNT_ADDR		0xffe0300c /*  register 32 bit */
#define TCOUNT			WORD_REF(TCOUNT_ADDR)

/****************************
 *
 *  DEBUG & EMULATION UNIT REGISTERS  (0XFFC05000 - 0XFFC0500C)
 *
 ****************************/

#define DSPID_ADDR		0xffe05000 /*  register 32 bit */
#define DSPID			WORD_REF(DSPID_ADDR)
#define DBGSTAT_ADDR		0xffe05008 /*  register 32 bit */
#define DBGSTAT			WORD_REF(DBGSTAT_ADDR)
#define EMUDAT_ADDR		0xffe0500c /*  register 32 bit */
#define EMUDAT			WORD_REF(EMUDAT_ADDR)

/****************************
 *
 *  TRACE UNIT REGISTERS  (0XFFC06000 - 0XFFC06100)
 *
 ****************************/

#define TBUFCTL_ADDR		0xffe06000 /*  register 32 bit */
#define TBUFCTL			WORD_REF(TBUFCTL_ADDR)
#define TBUFSTAT_ADDR		0xffe06004 /*  register 32 bit */
#define TBUFSTAT		WORD_REF(TBUFSTAT_ADDR)
#define TBUF_ADDR		0xffe06100 /*  register 32 bit */
#define TBUF			WORD_REF(TBUF_ADDR)

/****************************
 *
 *  WATCHPOINT & PATCH UNIT REGISTERS  (0XFFC07000 - 0XFFC07200)
 *
 ****************************/

#define WPIACTL_ADDR		0xffe07000 /*  register 32 bit */
#define WPIACTL			WORD_REF(WPIACTL_ADDR)
		/* TOTAL OF 6  */
#define WPIA_ADDR		0xffe07040 /*  register 32 bit */
#define WPIA			WORD_REF(WPIA_ADDR)
		/* TOTAL OF 6  */
#define WPIACNT_ADDR		0xffe07080 /*  register 32 bit */
#define WPIACNT			WORD_REF(WPIACNT_ADDR)

#define WPDACTL_ADDR		0xffe07100 /*  register 32 bit */
#define WPDACTL			WORD_REF(WPDACTL_ADDR)
		/* TOTAL OF 2 */
#define WPDA_ADDR		0xffe07140 /*  register 32 bit */
#define WPDA			WORD_REF(WPDA_ADDR)
		/* TOTAL OF 2 */
#define WPDACNT_ADDR		0xffe07180 /*  register 32 bit */
#define WPDACNT			WORD_REF(WPDACNT_ADDR)

#define WPSTAT_ADDR		0xffe07200 /*  register 32 bit */
#define WPSTAT			WORD_REF(WPSTAT_ADDR)

/****************************
 *
 *  PERFORMANCE MONITOR REGISTERS  (0XFFC08000 - 0XFFC08104)
 *
 ****************************/

#define PFCTL_ADDR		0xffe08000 /*  register 32 bit */
#define PFCTL			WORD_REF(PFCTL_ADDR)
		/* TOTAL OF 2 */
#define PFCNTR_ADDR		0xffe08100 /*  register 32 bit */
#define PFCNTR			WORD_REF(PFCNTR_ADDR)

/***************************
 *
 * SYSTEM INTERRUPT CONTROLLER REGISTERS
 *
 ***************************/
#define SIC_ISR_ADDR		0xffc00c14 /*  register 32 bit */
#define SIC_ISR			WORD_REF(SIC_ISR_ADDR)

#define SIC_IWR_ADDR		0xffc00c18 /*  register 32 bit */
#define SIC_IWR			WORD_REF(SIC_IWR_ADDR)

#define SIC_MASK_ADDR		0xffc00c10 /*  register 32 bit */
#define SIC_MASK		WORD_REF(SIC_MASK_ADDR)

#define SIC_RVECT_ADDR		0xffc00c00 /*  register 16 bit */
#define SIC_RVECT		HALFWORD_REF(SIC_RVECT_ADDR)

#define SIC_MASK_ALL		0x80000000

#endif  /* _BLKFin_H_  */
