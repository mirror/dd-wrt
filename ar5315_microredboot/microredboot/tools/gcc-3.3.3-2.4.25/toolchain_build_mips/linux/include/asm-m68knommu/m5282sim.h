/****************************************************************************/

/*
 *	m5282sim.h -- ColdFire 5282 System Integration Module support.
 *
 *	(C) Copyright 2003, Greg Ungerer (gerg@snapgear.com)
 */

/****************************************************************************/
#ifndef	m5282sim_h
#define	m5282sim_h
/****************************************************************************/

#include <linux/config.h>

/*
 *	Define the 5282 SIM register set addresses.
 */
#define	MCFICM_INTC0		0x0c00		/* Base for Interrupt Ctrl 0 */
#define	MCFICM_INTC1		0x0d00		/* Base for Interrupt Ctrl 0 */
#define	MCFINTC_IPRH		0x00		/* Interrupt pending 32-63 */
#define	MCFINTC_IPRL		0x04		/* Interrupt pending 1-31 */
#define	MCFINTC_IMRH		0x08		/* Interrupt mask 32-63 */
#define	MCFINTC_IMRL		0x0c		/* Interrupt mask 1-31 */
#define	MCFINTC_INTFRCH		0x10		/* Interrupt force 32-63 */
#define	MCFINTC_INTFRCL		0x14		/* Interrupt force 1-31 */
#define	MCFINTC_IRLR		0x18		/* */
#define	MCFINTC_IACKL		0x19		/* */
#define	MCFINTC_ICR0		0x40		/* Base ICR register */

#define	MCFINT_UART0		13		/* Interrupt number for UART0 */
#define	MCFINT_PIT1		55		/* Interrupt number for PIT1 */

#define MCFINT_CAN_BUF00	8		/* Interrupt for CAN Buffer*/
#define MCFINT_CAN_BUF01	9		/* Interrupt for CAN Buffer*/
#define MCFINT_CAN_WARN		24		/* Interrupt for CAN Error int*/
#define MCFINT_CAN_BUSOFF	25		/* Interrupt for CAN On/Off */

#define	MCF5282_GPIO_PUAPAR	0x10005C

/****************************************************************************/
#endif	/* m5282sim_h */
