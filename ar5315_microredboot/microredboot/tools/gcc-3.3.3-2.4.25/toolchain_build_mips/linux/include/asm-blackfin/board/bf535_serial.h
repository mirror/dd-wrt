/* 
 *  ADI uart macro definition :
 *     UART 0 CONTROLLER REGISTERS  (0XFFC01800 - 0XFFC01BFF)
 *     UART 1 CONTROLLER REGISTERS  (0XFFC01C00 - 0XFFC01FFF)
 */
#ifndef _BF535_SERIAL_H_
#define _BF535_SERIAL_H_

#define UART0_THR_ADDR		0xffc01800  /* UART 0 Transmit holding register
					       16 bit */
#define UART_THR(idx)		HALFWORD_REF((UART0_THR_ADDR | (idx << 10)))


#define UART0_RBR_ADDR		0xffc01800  /* UART 0 Receive buffer register  
					     16 bit */
#define UART_RBR(idx)		HALFWORD_REF((UART0_RBR_ADDR | (idx << 10)))

#define UART0_DLL_ADDR		0xffc01800  /* UART 0 Divisor latch (low byte) 
					       register  16 bit */
#define UART_DLL(idx)		HALFWORD_REF((UART0_DLL_ADDR | (idx << 10)))

#define UART0_IER_ADDR		0xffc01802  /* UART 0 Interrupt enable register  16 bit */
#define UART_IER(idx)		HALFWORD_REF((UART0_IER_ADDR | (idx << 10)))
#define UART_IER_ERBFI		0x01    /* Enable Receive Buffer Full Interrupt
					   (DR bit) */
#define UART_IER_ETBEI		0x02    /* Enable Transmit Buffer Empty 
					   Interrupt(THRE bit) */
#define UART_IER_ELSI		0x04    /* Enable RX Status Interrupt
					   (gen if any of LSR[4:1] set) */
#define UART_IER_EDDSI		0x08    /* Enable Modem Status Interrupt(gen if any UARTx_MSR[3:0] set) */

#define UART0_DLH_ADDR		0xffc01802  /* UART 0 Divisor latch (high byte) register  16 bit */
#define UART_DLH(idx)		HALFWORD_REF((UART0_DLH_ADDR | (idx << 10)))
#define UART0_IIR_ADDR		0xffc01804  /* UART 0 Interrupt identification register  16 bit */
#define UART_IIR(idx)		HALFWORD_REF((UART0_IIR_ADDR | (idx << 10)))
#define UART_IIR_NOINT		0x01    /* Bit0: cleared when no interrupt */
#define UART_IIR_STATUS		0x06    /* mask bit for the status: bit2-1 */
#define UART_IIR_LSR		0x06    /* Receive line status */
#define UART_IIR_RBR		0x04    /* Receive data ready */
#define UART_IIR_THR		0x02    /* Ready to transmit  */
#define UART_IIR_MSR		0x00    /* Modem status       */

#define UART0_LCR_ADDR          0xffc01806  /* UART 0 Line control register  16 bit */
#define UART_LCR(idx)           HALFWORD_REF((UART0_LCR_ADDR | (idx << 10)))
#define UART_LCR_WLS5           0       /* word length 5 bits */
#define UART_LCR_WLS6           0x01    /* word length 6 bits */
#define UART_LCR_WLS7           0x02    /* word length 7 bits */
#define UART_LCR_WLS8           0x03    /* word length 8 bits */
#define UART_LCR_STB            0x04    /* StopBit: 1: 2 stop bits for 
					   non-5-bit word length 1/2 stop bits 
					   for 5-bit word length 0: 
					   1 stop bit */
#define UART_LCR_PEN            0x08    /* Parity Enable 1: for enable */
#define UART_LCR_EPS            0x10    /* Parity Selection: 
					   1: for even pariety
                                           0: odd parity when PEN =1 & SP =0 */
#define UART_LCR_SP             0x20    /* Sticky Parity: */
#define UART_LCR_SB             0x40    /* Set Break: force TX pin to 0 */
#define UART_LCR_DLAB           0x80    /* Divisor Latch Access */


#define UART0_MCR_ADDR          0xffc01808  /* UART 0 Module Control register  
					       16 bit */
#define UART_MCR(idx)           HALFWORD_REF((UART0_MCR_ADDR | (idx << 10)))

#define UART0_LSR_ADDR          0xffc0180a  /* UART 0 Line status register  
					       16 bit */
#define UART_LSR(idx)           HALFWORD_REF((UART0_LSR_ADDR | (idx << 10)))
#define UART_LSR_DR             0x01    /* Data Ready */
#define UART_LSR_OE             0x02    /* Overrun Error */
#define UART_LSR_PE             0x04    /* Parity Error  */
#define UART_LSR_FE             0x08    /* Frame Error   */
#define UART_LSR_BI             0x10    /* Break Interrupt */
#define UART_LSR_THRE           0x20    /* THR empty, REady to accept */
#define UART_LSR_TEMT           0x40    /* TSR and UARTx_thr both empty */

#define UART0_MSR_ADDR          0xffc0180c  /* UART 0 Modem status register  16 bit */
#define UART_MSR(idx)           HALFWORD_REF((UART0_MSR_ADDR | (idx << 10)))
#define UART0_SCR_ADDR          0xffc0180e  /* UART 0 Scratch register  16 bit */
#define UART_SCR(idx)           HALFWORD_REF((UART0_SCR_ADDR | (idx << 10)))
#define UART0_IRCR_ADDR         0xffc01810  /* UART 0 IrDA Control register  16 bit */
#define UART_IRCR(idx)          HALFWORD_REF((UART0_IRCR_ADDR | (idx << 10)))

#endif /* _BF535_SERIAL_H_ */
