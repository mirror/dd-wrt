/****************************************************************************
*  $Author: gerg $
*  $Revision: 1.1 $
*  $Modtime: 9/06/02 11:53a $
****************************************************************************/
/******************************************************************************
****	Copyright (c) 1999, 2001, 2002 Conexant Systems, Inc.
****
*****************************************************************************

  This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option)
any later version.

  This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details.

  You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc., 59
Temple Place, Suite 330, Boston, MA 02111-1307 USA

********************************************************************************
*******************************************************************************/

#ifndef GPIO_H
#define GPIO_H

#include "bspcfg.h"
#include <linux/ptrace.h>

#define LED_ON		0
#define LED_OFF	1

#define GPIO0	0    /* GPIO0-15 in Register 1 */
#define GPIO1	1
#define GPIO2	2
#define GPIO3	3
#define GPIO4	4
#define GPIO5	5
#define GPIO6	6
#define GPIO7	7
#define GPIO8	8
#define GPIO9	9
#define GPIO10	10
#define GPIO11	11
#define GPIO12	12
#define GPIO13	13
#define GPIO14	14
#define GPIO15	15
#define GPIO16	16    /* GPIO16-31 in Register 2 */
#define GPIO17	17
#define GPIO18	18
#define GPIO19	19
#define GPIO20	20
#define GPIO21	21
#define GPIO22	22
#define GPIO23	23
#define GPIO24	24
#define GPIO25	25
#define GPIO26	26
#define GPIO27	27
#define GPIO28	28
#define GPIO29	29
#define GPIO30	30
#define GPIO31	31
#define GPIO32	32    /* GPIO32-39 in Register 3 */
#define GPIO33	33
#define GPIO34	34
#define GPIO35	35
#define GPIO36	36
#define GPIO37	37
#define GPIO38	38
#define GPIO39	39

/* For Rushmore run-time support, increase default number of GPIO registers in
   uGPIODefaultValue[]. to 22. If it is not Rushmore device, then the 3 extra registers
   GPIO_IE(x) will not be initialized */
#define RUSHMORE_NUM_GPIODEFAULT	22
#define NUM_OF_GPIODEFAULT       19

#define NUM_GPIOS_PER_REGISTER   16

//************************************************
// 		GPIO Controller I/O register map
//************************************************

#ifdef DEVICE_YELLOWSTONE
#define NUM_OF_GPIOS	   64

#define GPIO_BASE			0x00600C00
#define GRP0_OFFSET	   0
#define GRP1_OFFSET	   0x100
#define GRP2_OFFSET	   0x200
#define GRP3_OFFSET	   0x300

#define GRP_REG_OFFSET  0x100

#define p_GRP0_OUTPUT_ENABLE		( volatile UINT32* )( GPIO_BASE + GRP0_OFFSET + 0x000 )
#define p_GRP0_INPUT_ENABLE		( volatile UINT32* )( GPIO_BASE + GRP0_OFFSET + 0x004 )
#define p_GRP0_DATA_OUT				( volatile UINT32* )( GPIO_BASE + GRP0_OFFSET + 0x008 )
#define p_GRP0_DATA_IN				( volatile UINT32* )( GPIO_BASE + GRP0_OFFSET + 0x00C )
#define p_GRP0_INT_EVENT_TYPE		( volatile UINT32* )( GPIO_BASE + GRP0_OFFSET + 0x020 )
#define p_GRP0_INT_ENABLE_POS		( volatile UINT32* )( GPIO_BASE + GRP0_OFFSET + 0x024 )
#define p_GRP0_INT_ENABLE_NEG		( volatile UINT32* )( GPIO_BASE + GRP0_OFFSET + 0x028 )
#define p_GRP0_INT_STAT_POS		( volatile UINT32* )( GPIO_BASE + GRP0_OFFSET + 0x02C )
#define p_GRP0_INT_STAT_NEG		( volatile UINT32* )( GPIO_BASE + GRP0_OFFSET + 0x030 )
#define p_GRP0_INT_STAT				( volatile UINT32* )( GPIO_BASE + GRP0_OFFSET + 0x034 )
#define p_GRP0_INT_MASK				( volatile UINT32* )( GPIO_BASE + GRP0_OFFSET + 0x038 )
#define p_GRP0_INT_STAT_MASKED	( volatile UINT32* )( GPIO_BASE + GRP0_OFFSET + 0x03C )
#define p_GRP0_INT_ASSIGN_A		( volatile UINT32* )( GPIO_BASE + GRP0_OFFSET + 0x040 )
#define p_GRP0_INT_ASSIGN_B		( volatile UINT32* )( GPIO_BASE + GRP0_OFFSET + 0x044 )
#define p_GRP0_INT_ASSIGN_C		( volatile UINT32* )( GPIO_BASE + GRP0_OFFSET + 0x048 )
#define p_GRP0_INT_ASSIGN_D		( volatile UINT32* )( GPIO_BASE + GRP0_OFFSET + 0x04C )
#define p_GRP0_INT_STAT_MASKED_A	( volatile UINT32* )( GPIO_BASE + GRP0_OFFSET + 0x050 )
#define p_GRP0_INT_STAT_MASKED_B	( volatile UINT32* )( GPIO_BASE + GRP0_OFFSET + 0x054 )
#define p_GRP0_INT_STAT_MASKED_C	( volatile UINT32* )( GPIO_BASE + GRP0_OFFSET + 0x058 )
#define p_GRP0_INT_STAT_MASKED_D	( volatile UINT32* )( GPIO_BASE + GRP0_OFFSET + 0x05C )

#define p_GRP1_OUTPUT_ENABLE		( volatile UINT32* )( GPIO_BASE + GRP1_OFFSET + 0x000 )
#define p_GRP1_INPUT_ENABLE		( volatile UINT32* )( GPIO_BASE + GRP1_OFFSET + 0x004 )
#define p_GRP1_DATA_OUT				( volatile UINT32* )( GPIO_BASE + GRP1_OFFSET + 0x008 )
#define p_GRP1_DATA_IN				( volatile UINT32* )( GPIO_BASE + GRP1_OFFSET + 0x00C )
#define p_GRP1_INT_EVENT_TYPE		( volatile UINT32* )( GPIO_BASE + GRP1_OFFSET + 0x020 )
#define p_GRP1_INT_ENABLE_POS		( volatile UINT32* )( GPIO_BASE + GRP1_OFFSET + 0x024 )
#define p_GRP1_INT_ENABLE_NEG		( volatile UINT32* )( GPIO_BASE + GRP1_OFFSET + 0x028 )
#define p_GRP1_INT_STAT_POS		( volatile UINT32* )( GPIO_BASE + GRP1_OFFSET + 0x02C )
#define p_GRP1_INT_STAT_NEG		( volatile UINT32* )( GPIO_BASE + GRP1_OFFSET + 0x030 )
#define p_GRP1_INT_STAT				( volatile UINT32* )( GPIO_BASE + GRP1_OFFSET + 0x034 )
#define p_GRP1_INT_MASK				( volatile UINT32* )( GPIO_BASE + GRP1_OFFSET + 0x038 )
#define p_GRP1_INT_STAT_MASKED	( volatile UINT32* )( GPIO_BASE + GRP1_OFFSET + 0x03C )
#define p_GRP1_INT_ASSIGN_A		( volatile UINT32* )( GPIO_BASE + GRP1_OFFSET + 0x040 )
#define p_GRP1_INT_ASSIGN_B		( volatile UINT32* )( GPIO_BASE + GRP1_OFFSET + 0x044 )
#define p_GRP1_INT_ASSIGN_C		( volatile UINT32* )( GPIO_BASE + GRP1_OFFSET + 0x048 )
#define p_GRP1_INT_ASSIGN_D		( volatile UINT32* )( GPIO_BASE + GRP1_OFFSET + 0x04C )
#define p_GRP1_INT_STAT_MASKED_A	( volatile UINT32* )( GPIO_BASE + GRP1_OFFSET + 0x050 )
#define p_GRP1_INT_STAT_MASKED_B	( volatile UINT32* )( GPIO_BASE + GRP1_OFFSET + 0x054 )
#define p_GRP1_INT_STAT_MASKED_C	( volatile UINT32* )( GPIO_BASE + GRP1_OFFSET + 0x058 )
#define p_GRP1_INT_STAT_MASKED_D	( volatile UINT32* )( GPIO_BASE + GRP1_OFFSET + 0x05C )

#define p_GRP2_OUTPUT_ENABLE		( volatile UINT32* )( GPIO_BASE + GRP2_OFFSET + 0x000 )
#define p_GRP2_INPUT_ENABLE		( volatile UINT32* )( GPIO_BASE + GRP2_OFFSET + 0x004 )
#define p_GRP2_DATA_OUT				( volatile UINT32* )( GPIO_BASE + GRP2_OFFSET + 0x008 )
#define p_GRP2_DATA_IN				( volatile UINT32* )( GPIO_BASE + GRP2_OFFSET + 0x00C )
#define p_GRP2_INT_EVENT_TYPE		( volatile UINT32* )( GPIO_BASE + GRP2_OFFSET + 0x020 )
#define p_GRP2_INT_ENABLE_POS		( volatile UINT32* )( GPIO_BASE + GRP2_OFFSET + 0x024 )
#define p_GRP2_INT_ENABLE_NEG		( volatile UINT32* )( GPIO_BASE + GRP2_OFFSET + 0x028 )
#define p_GRP2_INT_STAT_POS		( volatile UINT32* )( GPIO_BASE + GRP2_OFFSET + 0x02C )
#define p_GRP2_INT_STAT_NEG		( volatile UINT32* )( GPIO_BASE + GRP2_OFFSET + 0x030 )
#define p_GRP2_INT_STAT				( volatile UINT32* )( GPIO_BASE + GRP2_OFFSET + 0x034 )
#define p_GRP2_INT_MASK				( volatile UINT32* )( GPIO_BASE + GRP2_OFFSET + 0x038 )
#define p_GRP2_INT_STAT_MASKED	( volatile UINT32* )( GPIO_BASE + GRP2_OFFSET + 0x03C )
#define p_GRP2_INT_ASSIGN_A		( volatile UINT32* )( GPIO_BASE + GRP2_OFFSET + 0x040 )
#define p_GRP2_INT_ASSIGN_B		( volatile UINT32* )( GPIO_BASE + GRP2_OFFSET + 0x044 )
#define p_GRP2_INT_ASSIGN_C		( volatile UINT32* )( GPIO_BASE + GRP2_OFFSET + 0x048 )
#define p_GRP2_INT_ASSIGN_D		( volatile UINT32* )( GPIO_BASE + GRP2_OFFSET + 0x04C )
#define p_GRP2_INT_STAT_MASKED_A	( volatile UINT32* )( GPIO_BASE + GRP2_OFFSET + 0x050 )
#define p_GRP2_INT_STAT_MASKED_B	( volatile UINT32* )( GPIO_BASE + GRP2_OFFSET + 0x054 )
#define p_GRP2_INT_STAT_MASKED_C	( volatile UINT32* )( GPIO_BASE + GRP2_OFFSET + 0x058 )
#define p_GRP2_INT_STAT_MASKED_D	( volatile UINT32* )( GPIO_BASE + GRP2_OFFSET + 0x05C )

#define p_GRP3_OUTPUT_ENABLE		( volatile UINT32* )( GPIO_BASE + GRP3_OFFSET + 0x000 )
#define p_GRP3_INPUT_ENABLE		( volatile UINT32* )( GPIO_BASE + GRP3_OFFSET + 0x004 )
#define p_GRP3_DATA_OUT				( volatile UINT32* )( GPIO_BASE + GRP3_OFFSET + 0x008 )
#define p_GRP3_DATA_IN				( volatile UINT32* )( GPIO_BASE + GRP3_OFFSET + 0x00C )
#define p_GRP3_INT_EVENT_TYPE		( volatile UINT32* )( GPIO_BASE + GRP3_OFFSET + 0x020 )
#define p_GRP3_INT_ENABLE_POS		( volatile UINT32* )( GPIO_BASE + GRP3_OFFSET + 0x024 )
#define p_GRP3_INT_ENABLE_NEG		( volatile UINT32* )( GPIO_BASE + GRP3_OFFSET + 0x028 )
#define p_GRP3_INT_STAT_POS		( volatile UINT32* )( GPIO_BASE + GRP3_OFFSET + 0x02C )
#define p_GRP3_INT_STAT_NEG		( volatile UINT32* )( GPIO_BASE + GRP3_OFFSET + 0x030 )
#define p_GRP3_INT_STAT				( volatile UINT32* )( GPIO_BASE + GRP3_OFFSET + 0x034 )
#define p_GRP3_INT_MASK				( volatile UINT32* )( GPIO_BASE + GRP3_OFFSET + 0x038 )
#define p_GRP3_INT_STAT_MASKED	( volatile UINT32* )( GPIO_BASE + GRP3_OFFSET + 0x03C )
#define p_GRP3_INT_ASSIGN_A		( volatile UINT32* )( GPIO_BASE + GRP3_OFFSET + 0x040 )
#define p_GRP3_INT_ASSIGN_B		( volatile UINT32* )( GPIO_BASE + GRP3_OFFSET + 0x044 )
#define p_GRP3_INT_ASSIGN_C		( volatile UINT32* )( GPIO_BASE + GRP3_OFFSET + 0x048 )
#define p_GRP3_INT_ASSIGN_D		( volatile UINT32* )( GPIO_BASE + GRP3_OFFSET + 0x04C )
#define p_GRP3_INT_STAT_MASKED_A	( volatile UINT32* )( GPIO_BASE + GRP3_OFFSET + 0x050 )
#define p_GRP3_INT_STAT_MASKED_B	( volatile UINT32* )( GPIO_BASE + GRP3_OFFSET + 0x054 )
#define p_GRP3_INT_STAT_MASKED_C	( volatile UINT32* )( GPIO_BASE + GRP3_OFFSET + 0x058 )
#define p_GRP3_INT_STAT_MASKED_D	( volatile UINT32* )( GPIO_BASE + GRP3_OFFSET + 0x05C )

#else
#define NUM_OF_GPIOS	   40

#define MISC_BASE       ((UINT32)0x00350000)

#define p_GRP0_ISM0            	((volatile UINT32 *)(MISC_BASE + 0xA0))
#define p_GRP0_ISM1    	         ((volatile UINT32 *)(MISC_BASE + 0xA4))
#define p_GRP0_ISM2     	         ((volatile UINT32 *)(MISC_BASE + 0xA8))

#define GPIO_OPT     	((volatile UINT32 *)(MISC_BASE + 0xB0))
#define p_GRP0_OUTPUT_ENABLE    	((volatile UINT32 *)(MISC_BASE + 0xB4))
#define p_GRP1_OUTPUT_ENABLE    	((volatile UINT32 *)(MISC_BASE + 0xB8))
#define p_GRP2_OUTPUT_ENABLE     ((volatile UINT32 *)(MISC_BASE + 0xBC))
#define p_GRP0_DATA_IN     	   ((volatile UINT32 *)(MISC_BASE + 0xC0))
#define p_GRP1_DATA_IN  	      ((volatile UINT32 *)(MISC_BASE + 0xC4))
#define p_GRP2_DATA_IN  	((volatile UINT32 *)(MISC_BASE + 0xC8))
#define p_GRP0_DATA_OUT	((volatile UINT32 *)(MISC_BASE + 0xCC))
#define p_GRP1_DATA_OUT	((volatile UINT32 *)(MISC_BASE + 0xD0))
#define p_GRP2_DATA_OUT	((volatile UINT32 *)(MISC_BASE + 0xD4))
#define p_GRP0_INT_STAT     		((volatile UINT32 *)(MISC_BASE + 0xD8))
#define p_GRP1_INT_STAT        	((volatile UINT32 *)(MISC_BASE + 0xDC))
#define p_GRP2_INT_STAT 		   ((volatile UINT32 *)(MISC_BASE + 0xE0))
#define p_GRP0_INT_MASK 		   ((volatile UINT32 *)(MISC_BASE + 0xE4))
#define p_GRP1_INT_MASK 	   	((volatile UINT32 *)(MISC_BASE + 0xE8))
#define p_GRP2_INT_MASK  		((volatile UINT32 *)(MISC_BASE + 0xEC))
#define GPIO_IPC1    	((volatile UINT32 *)(MISC_BASE + 0xF0))
#define GPIO_IPC2    	((volatile UINT32 *)(MISC_BASE + 0xF4))
#define GPIO_IPC3	      ((volatile UINT32 *)(MISC_BASE + 0xF8))

#define p_GRP0_INPUT_ENABLE     	((volatile UINT32 *)(MISC_BASE + 0xFC))
#define p_GRP1_INPUT_ENABLE     	((volatile UINT32 *)(MISC_BASE + 0x100))
#define p_GRP2_INPUT_ENABLE      ((volatile UINT32 *)(MISC_BASE + 0x104))

#define GRP_REG_OFFSET  0x4
#endif

#define GP_INPUTOFF	         0
#define GP_INPUTON	         1

#define GP_INPUT	            0
#define GP_OUTPUT	            1

#define GP_IRQ_MODE_LEVEL	   0
#define GP_IRQ_MODE_EDGE	   1

#define GP_IRQ_POL_NEGATIVE   0
#define GP_IRQ_POL_POSITIVE   1


#define GPIOINT_V90_DP	GPIO3
#define GPIOINT_F2_IRQ1	GPIO10
#define GPIOINT_F2_IRQ2	GPIO11
#define GPIOINT_USB_PWR GPIO22

#if DEBUG_CONSOLE_ON_UART_2
	#ifdef CONFIG_BD_RUSHMORE
		#define GPIOINT_UART1   GPIO14
	#endif
	#ifdef CONFIG_BD_HASBANI
		#define GPIOINT_UART1   GPIO24
	#endif
	#ifdef CONFIG_BD_MACKINAC
		#define GPIOINT_UART1   GPIO30
	#endif
#else
	#define GPIOINT_UART1   GPIO25
#endif

#ifdef CONFIG_BD_RUSHMORE
	#define GPIOINT_UART2   GPIO14
#endif
#ifdef CONFIG_BD_HASBANI
	#define GPIOINT_UART2   GPIO24
#endif
#ifdef CONFIG_BD_MACKINAC
	#define GPIOINT_UART2   GPIO30
#endif

#if !defined(BUILD_BOOTROM) && defined(INCLUDE_SOFTWARE_WLAN)

	#ifdef BOARD_DAFFY_II
	#define GPIOINT_WLAN    GPIO26
	#endif  /* End of BOARD_DAFFY_II */

	#ifdef BOARD_BRONX_MACKINAC
	#define GPIOINT_WLAN    GPIO27
	#endif  /* End of BOARD_BRONX_MACKINAC  */

#endif  /* End of INCLUDE_SOFTWARE_WLAN */

#if !defined(BUILD_BOOTROM) && defined(INCLUDE_SOFTWARE_PRINTER_SERVER)

	#ifdef BOARD_DAFFY_II
	#define GPIOINT_PRINTSERVER    GPIO27
	#endif  /* End of BOARD_DAFFY_II */

	#ifdef BOARD_BRONX_MACKINAC
	#define GPIOINT_PRINTSERVER    GPIO26
	#endif  /* End of BOARD_BRONX_MACKINAC  */

#endif  /* End of INCLUDE_SOFTWARE_PRINTER_SERVER */

#if CONFIG_CNXT_ADSL || CONFIG_CNXT_ADSL_MODULE
	#define GPOUT_TXD_LED				GPIO19
	#define GPOUT_RXD_LED				GPIO20
	#define GPOUT_F2_SCANEN				GPIO26
	#define GPOUT_SHOWTIME_LED			GPIO18
	#define GPOUT_OUTER_PAIR			GPIO31
	#define GPOUT_INNER_PAIR			GPIO21
	#define GPOUT_SHORTLOOPSW			GPIO38
	#define GPOUT_HS_LED				GPIO19
	#define GPIN_OH_DET					GPIO5		// remote offhook detect
	
	/* Falcon control pins */
	#define GPIOINT_F2_IRQ1				GPIO10
	#define GPIOINT_F2_IRQ2				GPIO11
	#define GPIOINT_F2_DYINGGASP_IRQ	GPIO6
	#define GPOUT_LDRVPWR				GPIO13
	#define GPOUT_F2PWRDWN				GPIO12
	#define GPOUT_F2RESET				GPIO9
	
	/* AFE POR */
	#define GPOUT_AFERESET 	   GPIO8
#else
	#define GPOUT_TXD_LED				GPIO19
	#define GPOUT_RXD_LED				GPIO20
	#define GPOUT_SHOWTIME_LED			GPIO18
#endif

#define	GPOUT_TASKRDY_LED	GPIO7

#if 0
	#ifdef ZIGGURAT

		#ifdef DEVICE_YELLOWSTONE
		#define GPOUT_TASKRDY_LED  GPIO1
		#else
		#define GPOUT_TASKRDY_LED  GPIO7
		#endif

		#define GPOUT_SHOWTIME_LED GPIO18
		#define GPOUT_TXD_LED      GPIO19
		#define GPOUT_RXD_LED      GPIO20
		#define GPOUT_F2_SCANEN    GPIO26
		#define GPOUT_F2_HRSTB    GPIO26        // Used for Trapper and beyond

		#ifndef DL10_D325_003
		#define GPOUT_F2_PWR       GPIO6
		#define GPOUT_OUTER_PAIR   GPIO33
		#else
		#define GPIOINT_F2_DYINGGASP_IRQ     	GPIO6
		#define GPOUT_OUTER_PAIR   GPIO31
		#endif

		#define GPOUT_INNER_PAIR   GPIO21
		#define GPOUT_SHORTLOOPSW  GPIO38
		#define GPOUT_HS_LED	   GPIO19
		#define GPIN_OH_DET	       GPIO5		// remote offhook detect
	#else

		#ifdef DEVICE_YELLOWSTONE
		#define GPOUT_TASKRDY_LED  GPIO1
		#else
		#define GPOUT_TASKRDY_LED  GPIO6
		#endif

		#define GPOUT_SHOWTIME_LED GPIO39
		#define GPOUT_TXD_LED      GPIO26
		#define GPOUT_RXD_LED      GPIO27
		#define GPOUT_F2_SCANEN    GPIO30
	#endif

	/* Falcon control pins */
	#define GPOUT_LDRVPWR	GPIO13
	#define GPOUT_F2PWRDWN	GPIO12
	#define GPOUT_F2RESET	GPIO9


	/* AFE POR */
	#define GPOUT_AFERESET 	   GPIO8
#endif
/* This is current Hasbani and Trident shared HW */
#ifdef DL10_D325_003
#define GPOUT_EMACRESET	   GPIO23
#else
#define GPOUT_FLASHRESET   GPIO23
#endif

typedef enum {
    GPIO_STATUS_ERROR    = -1,
    GPIO_STATUS_OK
} GPIO_STATUS;

extern void (*pF2_IRQ1_ISR)(void *);
extern void (*pF2_IRQ2_ISR)(void *);

void GPIO_SetGPIOIRQCallback(int gpio,void (*(pISR))(int));
void *GPIO_GetGPIOIRQCallback(int gpio);
void GPIO_SetGPIOIRQRoutine(int gpio,void (*(pISR))(int));
void *GPIO_GetGPIOIRQRoutine(int gpio);

/* Exported Function Prototypes Declaration */
extern void SetGPIOUsage( UINT8 uGPIOPin, BOOL bOpt );
extern void SetGPIODir( UINT8 uGPIOPin, BOOL bDir );
extern void SetGPIOInputEnable( UINT8 uGPIOPin, BOOL bState );
extern BOOL ReadGPIOData( UINT8 uGPIOPin );
extern void WriteGPIOData( UINT8 uGPIOPin, BOOL bState );
extern BOOL ReadGPIOIntStatus( UINT8 uGPIOPin );
extern void ClearGPIOIntStatus( UINT8 uGPIOPin );
extern void SetGPIOIntEnable( UINT8 uGPIOPin, BOOL bState );
extern BOOL ReadGPIOIntEnable( UINT8 uGPIOPin );
extern void SetGPIOIntPolarity( UINT8 uGPIOPin, BOOL bPolarity );
extern BOOL ReadGPIOIntStatus( UINT8 uGPIOPin );
extern BOOL ReadGPIOIntPolarity( UINT8 uGPIOPin );
extern void SetGPIOIntMode( UINT8 uGPIOPin, BOOL bPolarity );

extern GPIO_STATUS GPIO_Init(void);

extern void GPIOHandler(int irq, void *dev_id, struct pt_regs * regs);

// Temporary until the newer gpioisr.c file is ported over

extern void GPIOB10_Handler(int gpio_num);
extern void GPIOB11_Handler(int gpio_num);
extern void GPIOB14_Handler(int gpio_num);
extern void GPIOB24_Handler(int gpio_num);
extern void GPIOB25_Handler(int gpio_num);
extern void GPIOB30_Handler(int gpio_num);

#define MISC_BASE       ((UINT32)0x00350000)

#define GPIO_ISM1     	((volatile UINT32 *)(MISC_BASE + 0xA0))
#define GPIO_ISM2     	((volatile UINT32 *)(MISC_BASE + 0xA4))
#define GPIO_ISM3     	((volatile UINT32 *)(MISC_BASE + 0xA8))

#define GPIO_OPT     	((volatile UINT32 *)(MISC_BASE + 0xB0))
#define GPIO_OE1     	((volatile UINT32 *)(MISC_BASE + 0xB4))
#define GPIO_OE2     	((volatile UINT32 *)(MISC_BASE + 0xB8))
#define GPIO_OE3	      ((volatile UINT32 *)(MISC_BASE + 0xBC))
#define GPIO_DATA_IN1	((volatile UINT32 *)(MISC_BASE + 0xC0))
#define GPIO_DATA_IN2	((volatile UINT32 *)(MISC_BASE + 0xC4))
#define GPIO_DATA_IN3	((volatile UINT32 *)(MISC_BASE + 0xC8))
#define GPIO_DATA_OUT1	((volatile UINT32 *)(MISC_BASE + 0xCC))
#define GPIO_DATA_OUT2	((volatile UINT32 *)(MISC_BASE + 0xD0))
#define GPIO_DATA_OUT3	((volatile UINT32 *)(MISC_BASE + 0xD4))
#define GPIO_ISR1 		((volatile UINT32 *)(MISC_BASE + 0xD8))
#define GPIO_ISR2    	((volatile UINT32 *)(MISC_BASE + 0xDC))
#define GPIO_ISR3		   ((volatile UINT32 *)(MISC_BASE + 0xE0))
#define GPIO_IER1		   ((volatile UINT32 *)(MISC_BASE + 0xE4))
#define GPIO_IER2	   	((volatile UINT32 *)(MISC_BASE + 0xE8))
#define GPIO_IER3 		((volatile UINT32 *)(MISC_BASE + 0xEC))
#define GPIO_IPC1    	((volatile UINT32 *)(MISC_BASE + 0xF0))
#define GPIO_IPC2    	((volatile UINT32 *)(MISC_BASE + 0xF4))
#define GPIO_IPC3	      ((volatile UINT32 *)(MISC_BASE + 0xF8))

#endif /* GPIOVAL_H */
