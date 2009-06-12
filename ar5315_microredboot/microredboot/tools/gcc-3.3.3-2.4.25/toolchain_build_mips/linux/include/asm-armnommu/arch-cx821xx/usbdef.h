/****************************************************************************
*
*	Name:			usbdef.h
*
*	Description:	
*
*	Copyright:		(c) 2002 Conexant Systems Inc.
*
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

****************************************************************************
*  $Author: gerg $
*  $Revision: 1.1 $
*  $Modtime: 2/28/02 9:56a $
****************************************************************************/

#ifndef USBDEF_H
#define USBDEF_H

typedef enum
{
   USB_DISABLE,            /* 0: UDC Core not configured */
   USB_ENABLE,             /* 1: UDC Core configured */
   USB_ATTACHED,           /* 2: USB cable attached */
   USB_POWERED,            /* 3: USB powered */
   USB_DEFAULT,            /* 4: USB has been reset */
   USB_ADDRESSED,          /* 5: USB assigned unique device address */
   USB_CONFIGURED,         /* 6: USB assigned valid configuration */
   USB_SUSPENDED,          /* 7: USB attached, powered but bus idle */
   USB_RESUME_INITIATE,    /* 8: USB device initiated resume */
   USB_DETACHED            /* 9: USB cable detached */
} eUSBState;


typedef enum
{
   EP_CONTROL,
   EP_ISO,
   EP_BULK,
   EP_INTERRUPT
} eEPType;


typedef enum
{
   EP_SETTING0,
   EP_SETTING1,
   EP_SETTING2,
   EP_SETTING3,
   EP_SETTING4,
   EP_SETTING5,
   EP_SETTING6,
   EP_SETTING7
} eEPSetting;


typedef enum
{
   EP_INTERFACE0,
   EP_INTERFACE1,
   EP_INTERFACE2,
   EP_INTERFACE3
} eEPInterface;


typedef enum
{
   EP_CONF0,
   EP_CONF1,
   EP_CONF2,
   EP_CONF3
} eEPConfigure;


typedef enum
{
   ENDPOINT0,
   ENDPOINT1,
   ENDPOINT2,
   ENDPOINT3,
   ENDPOINT4,
	NOT_AVAILABLE
} eEPNumber;


typedef enum
{
   ENDPOINT0_IN,
   ENDPOINT0_OUT,
   ENDPOINT1_IN,
   ENDPOINT1_OUT,
   ENDPOINT2_IN,
   ENDPOINT2_OUT,
   ENDPOINT3_IN,
   ENDPOINT3_OUT,
   ENDPOINT4_IN,
   ENDPOINTALL_IN,
   ENDPOINTALL_OUT,
   ENDPOINTALL,
	ENDPOINT_UNKNOWN
} eEPDirection;


typedef enum
{
   EP_DISABLE,
   EP_ENABLE,
   EP_RESET,
   EP_CONFIGURE,
   EP_FLUSH
} eEPCtl;


typedef enum
{
   DESC_TYPE_NONE,
   DESC_TYPE_DEV,
   DESC_TYPE_CONFIG,
   DESC_TYPE_STRING,
   DESC_TYPE_INTER,
   DESC_TYPE_EP
} eDESCType;


/*	String Descriptor Index Enumeration */
typedef enum
{
   DESC_STRING_LANGUAGEID,												
   DESC_STRING_MANUFACTURER,
   DESC_STRING_PRODUCT,
   DESC_STRING_SERIAL 
} eDESCSTRINGType;



#define BUS_POWERED_ATTRIB    0x80
#define SELF_POWERED_ATTRIB   0x40
#define REMOTE_WAKEUP_ATTRIB  0x20

/* Defines for Device Descriptor */
#define NUM_DEV_DESC_FLD   14
#define DEV_DESC_LENGTH	   18
#define USB_SPEC_100       0x0100
#define USB_SPEC_110       0x0110
#define P50_DEV_CLASS	   0x00
#define P50_DEV_SUBCLASS   0x00
#define P50_DEV_PROTOCOL   0x00
#define CONEXANT_VENDOR_ID	0x0572
#define P50_PRODUCT_ID	   0xcafe
#define P50_DEV_ID	     	0001

/* Defines for Configuration Descriptor */
#define CONFIG_DESC_LENGTH 9
#define NUM_OF_INTERFACE   1
#define CONFIG_VALUE       1
#define CONFIG_STR         0
/* Self-powered, support Remote Wakeup */
/*#define CONFIG_ATTRIBUTES  (SELF_POWERED_ATTRIB | REMOTE_WAKEUP_ATTRIB)*/
/* Bus-powered, support Remote Wakeup */
#ifdef USB_REMOTE_WAKEUP
#define CONFIG_ATTRIBUTES  (BUS_POWERED_ATTRIB | REMOTE_WAKEUP_ATTRIB)
#else
#define CONFIG_ATTRIBUTES  BUS_POWERED_ATTRIB
#endif
#define MAX_POWER_CONSUME  250   /* 2 * 250 = 500mA */

/* Defines for Interface descriptor */
#define INTER_DESC_LENGTH        9
#define INTER_NUM                0
#define ALT_SETTING              0
#define INTER_CLASS              0
#define INTER_SUBCLASS           0
#define INTER_PROTOCOL           0
#define INTER_STR                0

/* Defines for Language ID */
#define USB_MS_ENGLISH_ID        0x0409

/* Defines for Endpoint descriptor */
#define MAX_NUM_ENDPOINT         9
#define MAX_EP_CFG_LENGTH        (MAX_NUM_ENDPOINT * 5)
#define MAX_EP_CFG_DWORD         ((MAX_EP_CFG_LENGTH / 4) + 1)
#define MAX_NUM_CONFIG           1

#define EP_OUT                   0
#define EP_IN                    1

#define EP_DESC_LENGTH           7
#define MAX_EP_DESC_LENGTH       (EP_DESC_LENGTH * (MAX_NUM_ENDPOINT - 2))
#define MAX_CONFIG_DESC_LENGTH   (CONFIG_DESC_LENGTH + INTER_DESC_LENGTH + MAX_EP_DESC_LENGTH)
#define MAX_CONFIG_DESC_DWORD    ((MAX_CONFIG_DESC_LENGTH/4) + 1)

#define BULK_POLL_INTERVAL       1	/* ms */
#define INT_POLL_INTERVAL        4	/* ms */


/*************************************************************
* Defines for USB DMA buffers map.
*************************************************************/

#define USB_MAX_PACKET_SIZE      64
#define BULK_MAX_PACKET_SIZE     64
#define INT_MAX_PACKET_SIZE      8

/* If it is not USB ADSL product we just allocate enough buffers to */
/* support Enumeration and Configuration over USB.  Assume no data  */
/* transfer. */
/*#ifdef CONFIG_DATA_OVER_USB*/
#define USB_EP0_NUM_TX_BUFF      4
#define USB_EP1_NUM_TX_BUFF      4
#define USB_EP2_NUM_TX_BUFF      160
#define USB_EP3_NUM_TX_BUFF      4
/*#else
#define USB_EP0_NUM_TX_BUFF      4
#define USB_EP1_NUM_TX_BUFF      4
#define USB_EP2_NUM_TX_BUFF      4
#define USB_EP3_NUM_TX_BUFF      4
#endif*/

#define USB_TX_DESC_SIZE         8		/* 8 bytes = 1 Qword */
#define USB_TX_LINK_SIZE         8
#define USB_TX_FRAME_SIZE        (USB_TX_DESC_SIZE + USB_MAX_PACKET_SIZE + USB_TX_LINK_SIZE)
#define USB_EP0_TX_BUFF_SIZE    	(USB_TX_FRAME_SIZE * USB_EP0_NUM_TX_BUFF)
#define USB_EP1_TX_BUFF_SIZE    	(USB_TX_FRAME_SIZE * USB_EP1_NUM_TX_BUFF)
#define USB_EP2_TX_BUFF_SIZE    	(USB_TX_FRAME_SIZE * USB_EP2_NUM_TX_BUFF)
#define USB_EP3_TX_BUFF_SIZE    	(USB_TX_FRAME_SIZE * USB_EP3_NUM_TX_BUFF)
#define USB_TXBUFF_SIZE          (USB_EP0_TX_BUFF_SIZE + USB_EP1_TX_BUFF_SIZE + USB_EP2_TX_BUFF_SIZE + USB_EP3_TX_BUFF_SIZE)

#define USB_RX_DESC_SIZE         8		/* 8 bytes = 1 Qword */
#define USB_RX_FRAME_SIZE        (USB_RX_DESC_SIZE + USB_MAX_PACKET_SIZE)
#define USB_MAX_RXBUFF_SIZE      (USB_RX_FRAME_SIZE * 230)

#ifdef USB_DMA_IN_SRAM
   #define USB_DMA_RAM_START     SRAMSTART
   #ifndef DSL_DMA_IN_SRAM
   #define USB_DMA_RAM_END       SRAMEND
   #endif
#else
   #ifdef DSL_DMA_IN_SRAM
   #define USB_DMA_RAM_START     ( SDRAMEND - (USB_MAX_RXBUFF_SIZE + USB_TXBUFF_SIZE) )
   #define USB_DMA_RAM_END       SDRAMEND
   #else
   #define USB_DMA_RAM_START     ( SDRAMEND - (USB_MAX_RXBUFF_SIZE*4) )
   #define USB_DMA_RAM_END       ( USB_DMA_RAM_START + (USB_MAX_RXBUFF_SIZE + USB_TXBUFF_SIZE) )
   #endif
#endif

#define pEP0TXBUFFER (volatile UINT32 *) USB_DMA_RAM_START
#define pEP1TXBUFFER (volatile UINT32 *) ((UINT8 *) pEP0TXBUFFER + USB_EP0_TX_BUFF_SIZE)
#define pEP2TXBUFFER (volatile UINT32 *) ((UINT8 *) pEP1TXBUFFER + USB_EP1_TX_BUFF_SIZE)
#define pEP3TXBUFFER (volatile UINT32 *) ((UINT8 *) pEP2TXBUFFER + USB_EP2_TX_BUFF_SIZE)

#define pUSB_RXBUFF 	(volatile UINT32 *) ((UINT8 *) pEP3TXBUFFER + USB_EP3_TX_BUFF_SIZE)


/* If it is not USB ADSL product we just allocate enough buffers to */
/* support Enumeration and Configuration over USB.  Assume no data  */
/* transfer. */
/*#ifdef CONFIG_DATA_OVER_USB*/
   #ifdef USB_DMA_IN_SRAM
      #ifdef DSL_DMA_IN_SRAM
      #define USB_MAX_RX_BUFF          56
      #else
      #define USB_MAX_RX_BUFF          ( ( ((UINT32)USB_DMA_RAM_END - (UINT32)pUSB_RXBUFF) / USB_RX_FRAME_SIZE) - 2 )
      #endif
   #else
      #define USB_MAX_RX_BUFF          ( ( ((UINT32)USB_DMA_RAM_END - (UINT32)pUSB_RXBUFF) / USB_RX_FRAME_SIZE) - 2 )
   #endif
/*#else
   #define USB_MAX_RX_BUFF       16
#endif*/

#define USB_RXBUFF_SIZE          (USB_RX_FRAME_SIZE * USB_MAX_RX_BUFF)


#define USB_RECIPIENT            0x1F
#define USB_RECIPIENT_DEVICE     0x00
#define USB_RECIPIENT_INTERFACE  0x01
#define USB_RECIPIENT_ENDPOINT   0x02

#define USB_MAX_REQ_NUM          13
#define USB_REQUEST_TYPE_MASK    0x60
#define USB_STANDARD_REQUEST     0x00
#define USB_CLASS_REQUEST        0x20
#define USB_VENDOR_REQUEST       0x40
#define USB_REQUEST_MASK         0x0F

/* Rx Time Out Range in ms.  We do not use 1 since it could be */
/* 1ms in between USB packet. */
#define USB_RX_MIN_TIMEOUT       2000     /* 2ms */
#define USB_RX_MAX_TIMEOUT       10000    /* 10ms */

#define USB_MONITOR_PERIOD 10  /* 10ms */
#define USB_CABLE_DEBOUNCE 100 /* 100ms */


/*********************************************************/
/* Define the Base addresses of USB registers            */
/*********************************************************/
#define USB_HW_BASE  0x00330000

#define USB_EP0_TX_DMA ((volatile UINT32 *) (USB_HW_BASE + 0x0))
#define USB_EP1_TX_DMA ((volatile UINT32 *) (USB_HW_BASE + 0x8))
#define USB_EP2_TX_DMA ((volatile UINT32 *) (USB_HW_BASE + 0x10))
#define USB_EP3_TX_DMA ((volatile UINT32 *) (USB_HW_BASE + 0x18))
#define USB_EP0_RX_DMA ((volatile UINT32 *) (USB_HW_BASE + 0x20))

#define USB_CFG	     ((volatile UINT32 *) (USB_HW_BASE + 0x24))
#define USB_IDAT       ((volatile UINT32 *) (USB_HW_BASE + 0x28))
#define USB_CTR1       ((volatile UINT32 *) (USB_HW_BASE + 0x2C))
#define USB_CTR2       ((volatile UINT32 *) (USB_HW_BASE + 0x30))
#define USB_CTR3       ((volatile UINT32 *) (USB_HW_BASE + 0x34))
#define USB_STAT       ((volatile UINT32 *) (USB_HW_BASE + 0x38))
#define USB_IER        ((volatile UINT32 *) (USB_HW_BASE + 0x3C))
#define USB_STAT2      ((volatile UINT32 *) (USB_HW_BASE + 0x40))
#define USB_IER2       ((volatile UINT32 *) (USB_HW_BASE + 0x44))

#define USB_EP0_TX_INC     ((volatile UINT32 *) (USB_HW_BASE + 0x48))
#define USB_EP0_TX_PEND    ((volatile UINT32 *) (USB_HW_BASE + 0x4C))
#define USB_EP0_TX_QWCNT   ((volatile UINT32 *) (USB_HW_BASE + 0x50))
#define USB_EP1_TX_INC     ((volatile UINT32 *) (USB_HW_BASE + 0x54))
#define USB_EP1_TX_PEND    ((volatile UINT32 *) (USB_HW_BASE + 0x58))
#define USB_EP1_TX_QWCNT   ((volatile UINT32 *) (USB_HW_BASE + 0x5C))
#define USB_EP2_TX_INC     ((volatile UINT32 *) (USB_HW_BASE + 0x60))
#define USB_EP2_TX_PEND    ((volatile UINT32 *) (USB_HW_BASE + 0x64))
#define USB_EP2_TX_QWCNT   ((volatile UINT32 *) (USB_HW_BASE + 0x68))
#define USB_EP3_TX_INC     ((volatile UINT32 *) (USB_HW_BASE + 0x6C))
#define USB_EP3_TX_PEND    ((volatile UINT32 *) (USB_HW_BASE + 0x70))
#define USB_EP3_TX_QWCNT   ((volatile UINT32 *) (USB_HW_BASE + 0x74))

#define USB_RX_DEC         ((volatile UINT32 *) (USB_HW_BASE + 0x78))
#define USB_RX_PEND        ((volatile UINT32 *) (USB_HW_BASE + 0x7C))
#define USB_RX_QWCNT       ((volatile UINT32 *) (USB_HW_BASE + 0x80))
#define USB_RX_BUFFSIZE    ((volatile UINT32 *) (USB_HW_BASE + 0x84))

#define USB_CSR       ((volatile UINT32 *) (USB_HW_BASE + 0x88))
#define UDC_TSR       ((volatile UINT32 *) (USB_HW_BASE + 0x8C))
#define UDC_STAT      ((volatile UINT32 *) (USB_HW_BASE + 0x90))

#define USB_RX_TIMER  ((volatile UINT32 *) (USB_HW_BASE + 0x94))
#define USB_RX_TMRCNT ((volatile UINT32 *) (USB_HW_BASE + 0x98))
#define USB_RX_THRESH ((volatile UINT32 *) (USB_HW_BASE + 0x9C))

/*********************************************************/
/* Define USB registers bit definitions                  */
/*********************************************************/

/* Funny game here.  We use the MSB of the EP_NUM */
/* field to indicate the direction.  So we will   */
/* mask it out when we look for physical endpoint */

/*#define RX_EP_NUM_MASK		     	0x0700*/
#define RX_EP_NUM_MASK		     	0x0F00
#define RX_EP_NUM_SHIFT		      8

#define RX_GOOD_PACKET_MASK   	0x8000
#define RX_SETUP_PACKET_MASK	   0x0080
#define RX_NUM_BYTES_MASK	      0x007F

/* USB_CTR1 bit definitions */
#define USBCTR1_ENA              BIT0
#define USBCTR1_CONFIG_ENA       BIT1
#define USBCTR1_EP0O_ENA         BIT2
#define USBCTR1_EP1O_ENA         BIT3
#define USBCTR1_EP2O_ENA         BIT4
#define USBCTR1_EP3O_ENA         BIT5
#define USBCTR1_EP0I_ENA         BIT6
#define USBCTR1_EP1I_ENA         BIT7
#define USBCTR1_EP2I_ENA         BIT8
#define USBCTR1_EP3I_ENA         BIT9
#define USBCTR1_EPINT_ENA        BIT10
#define USBCTR1_XVER_SLEEP       BIT11

#define USBCTR1_EP0I_DMA_RESET   BIT12
#define USBCTR1_EP1I_DMA_RESET   BIT13
#define USBCTR1_EP2I_DMA_RESET   BIT14
#define USBCTR1_EP3I_DMA_RESET   BIT15

#define USBCTR1_EPIALL_DMA_RESET (USBCTR1_EP0I_DMA_RESET | USBCTR1_EP1I_DMA_RESET | USBCTR1_EP2I_DMA_RESET | USBCTR1_EP3I_DMA_RESET)

#define USBCTR1_RVINIT_ENA       BIT28
#define USBCTR1_RESUME_INIT      BIT29
#define USBCTR1_RESET            BIT30
#define USBCTR1_GBL_IRQ_ENA      BIT31

#define USB_ALL_EPOUT_ENA        (USBCTR1_EP0O_ENA | USBCTR1_EP1O_ENA | USBCTR1_EP2O_ENA | USBCTR1_EP3O_ENA)
#define USB_ALL_EPIN_ENA         (USBCTR1_EP0I_ENA | USBCTR1_EP1I_ENA | USBCTR1_EP2I_ENA | USBCTR1_EP3I_ENA)

#define USB_TX_BUFF_RDY	         BIT15

#define USB_STAT_CLEAR           0xFFFFFFFF

/* EP IN ready status for Interupt EP indication */
#define USB_EP0_IN_RDY           0x01010101
#define USB_EP1_IN_RDY           0x02020202
/* Repeat bytes across to work around Interrupt endpoint HW problem */
#define USB_EP2_IN_RDY           0x04040404
#define USB_EP3_IN_RDY           0x08080808

#define USB_TX_PENDING_MSK       0xFF
#define USB_RX_PENDING_MSK       0xFF

/* USB Status bit definitions */
#define USBINT_RX_PEND        BIT28   /*  USB Rx Pending register*/
#define USBINT_UDC_VALIDINTF  BIT27   /*  UDC SetInterf detected */
#define USBINT_UDC_VALIDCONF  BIT26   /*  UDC SetConfig detected */
#define USBINT_UDC_RESET      BIT25   /*  UDC USB Reset detected */
#define USBINT_UDC_SOF        BIT24   /*  UDC SOF detected       */

#define USBINT_SUSPEND     BIT23   /*  USB Suspend detected   */
#define USBINT_RESUME      BIT22   /*  USB Resume detected    */
#define USBINT_EP3_INVLD   BIT21   /*  EP3 IN invalid header  */
#define USBINT_EP2_INVLD   BIT20   /*  EP2 IN invalid header  */
#define USBINT_EP1_INVLD   BIT19   /*  EP1 IN invalid header  */
#define USBINT_EP0_INVLD   BIT18   /*  EP0 IN invalid header  */
#define USBINT_EPINT_NAK   BIT17   /*  EP INT NAK             */
#define USBINT_EPINT_ERR   BIT16   /*  EP INT Error count reached */
#define USBINT_EP3_ERR     BIT15   /*  EP3 Error count reached */
#define USBINT_EP2_ERR     BIT14   /*  EP2 Error count reached */
#define USBINT_EP1_ERR     BIT13   /*  EP1 Error count reached */
#define USBINT_EP0_ERR     BIT12   /*  EP0 Error count reached */

#define USBINT_EPINT_DN    BIT10   /*  EP INT Tx done          */
#define USBINT_EPINT_NXT   BIT11   /*  EP INT Tx next DWORD    */

#define USBINT_EP3_IN      BIT9    /*  EP3 IN completed  */
#define USBINT_EP2_IN      BIT8    /*  EP2 IN completed  */
#define USBINT_EP1_IN      BIT7    /*  EP1 IN completed  */
#define USBINT_EP0_IN      BIT6    /*  EP0 IN completed  */
#define USBINT_EP3_OUT     BIT5    /*  EP3 OUT completed */
#define USBINT_EP2_OUT     BIT4    /*  EP2 OUT completed */
#define USBINT_EP1_OUT     BIT3    /*  EP1 OUT completed */
#define USBINT_EP0_OUT     BIT2    /*  EP0 OUT completed */
#define USBINT_CFG_DN      BIT1    /*  UDC config completed */
#define USBINT_CFG_NXT     BIT0    /*  UDC next config DWORD */

#define USBINT_EPALL_OUT   (USBINT_EP0_OUT | USBINT_EP1_OUT | USBINT_EP2_OUT | USBINT_EP3_OUT)
#define USBINT_EPALL_IN    (USBINT_EP0_IN | USBINT_EP1_IN | USBINT_EP2_IN | USBINT_EP3_IN)

/* USB Status2 bit definitions */
#define USBINT2_EP3_TX_EMPTY 	BIT31   /*  USB EP3 Tx empty          */
#define USBINT2_EP2_TX_EMPTY 	BIT30   /*  USB EP2 Tx empty          */
#define USBINT2_EP1_TX_EMPTY 	BIT29   /*  USB EP1 Tx empty          */
#define USBINT2_EP0_TX_EMPTY 	BIT28   /*  USB EP0 Tx empty          */
#define USBINT2_RX_TIMEOUT	 	BIT26   /*  USB Rx time out           */
#define USBINT2_RX_THRESHOLD  BIT25   /*  USB Rx Threshold reached  */
#define USBINT2_RX_OVERRUN    BIT24   /*  USB Rx Overrun detected   */

#define USB_INT2_EPALL_TX_EMPTY  (USBINT2_EP3_TX_EMPTY | USBINT2_EP2_TX_EMPTY | USBINT2_EP1_TX_EMPTY | USBINT2_EP0_TX_EMPTY)

#define USBINT2_SETCONF       BIT23   /*  USB SetConfig detected    */
#define USBINT2_SETINTER      BIT22   /*  USB SetInterface detected */
#define USBINT2_GETCONF       BIT21   /*  USB GetConfig detected    */
#define USBINT2_GETINTER      BIT20   /*  USB GetInterface detected */
#define USBINT2_ALTSET        BIT19   /*  USB AltSetting detected   */
#define USBINT2_SETUP         BIT18   /*  USB Setup command detected*/
#define USBINT2_EP3OUT_CLR    BIT17   /*  EP3 OUT Stall cleared     */
#define USBINT2_EP2OUT_CLR    BIT16   /*  EP2 OUT Stall cleared     */
#define USBINT2_EP1OUT_CLR    BIT15   /*  EP1 OUT Stall cleared     */
#define USBINT2_EP0OUT_CLR    BIT14   /*  EP0 OUT Stall cleared     */
#define USBINT2_EP3IN_CLR     BIT13   /*  EP3 IN Stall cleared      */
#define USBINT2_EP2IN_CLR     BIT12   /*  EP2 IN Stall cleared      */
#define USBINT2_EP1IN_CLR     BIT11   /*  EP1 IN Stall cleared      */
#define USBINT2_EP0IN_CLR     BIT10   /*  EP0 IN Stall cleared      */
#define USBINT2_EPINT_CLR     BIT9    /*  EPINT IN Stall cleared    */
#define USBINT2_EP3OUT_STALL  BIT8    /*  EP3 OUT Stall */
#define USBINT2_EP2OUT_STALL  BIT7    /*  EP2 OUT Stall */
#define USBINT2_EP1OUT_STALL  BIT6    /*  EP1 OUT Stall */
#define USBINT2_EP0OUT_STALL  BIT5    /*  EP0 OUT Stall */
#define USBINT2_EP3IN_STALL   BIT4    /*  EP3 IN Stall  */
#define USBINT2_EP2IN_STALL   BIT3    /*  EP2 IN Stall  */
#define USBINT2_EP1IN_STALL   BIT2    /*  EP1 IN Stall  */
#define USBINT2_EP0IN_STALL   BIT1    /*  EP0 IN Stall  */
#define USBINT2_EPINT_STALL   BIT0    /*  EPINT IN Stall*/

/* USB Control-Status bit definitions */
#define USBCSR_EP_RX_FULL     BIT14   /*  USB Rx pending reg is full      */
#define USBCSR_EP3_TX_EMPTY   BIT13   /*  USB EP3 Tx pending reg is empty */
#define USBCSR_EP2_TX_EMPTY   BIT12   /*  USB EP2 Tx pending reg is empty */
#define USBCSR_EP1_TX_EMPTY   BIT11   /*  USB EP1 Tx pending reg is empty */
#define USBCSR_EP0_TX_EMPTY   BIT10   /*  USB EP0 Tx pending reg is empty */
#define USBCSR_EP_RX_CLRCNT   BIT9    /*  USB clear Rx count reg          */
#define USBCSR_EP_RX_CLRPEND  BIT8    /*  USB clear Rx pending reg        */
#define USBCSR_EP3_TX_CLRCNT  BIT7    /*  USB EP3 clear Tx count reg      */
#define USBCSR_EP3_TX_CLRPEND BIT6    /*  USB EP3 clear Tx pending reg    */
#define USBCSR_EP2_TX_CLRCNT  BIT5    /*  USB EP2 clear Tx count reg      */
#define USBCSR_EP2_TX_CLRPEND BIT4    /*  USB EP2 clear Tx pending reg    */
#define USBCSR_EP1_TX_CLRCNT  BIT3    /*  USB EP1 clear Tx count reg      */
#define USBCSR_EP1_TX_CLRPEND BIT2    /*  USB EP1 clear Tx pending reg    */
#define USBCSR_EP0_TX_CLRCNT  BIT1    /*  USB EP0 clear Tx count reg      */
#define USBCSR_EP0_TX_CLRPEND BIT0    /*  USB EP0 clear Tx pending reg    */

#define USBCSR_EPIALL_TX_CLRPEND  (BIT0 | BIT2 | BIT4 | BIT6 )
#define USBCSR_EPIALL_TX_CLRCNT   (BIT1 | BIT3 | BIT5 | BIT7 )

#ifdef USB_VCOM_ENDPOINT3
#define ENDPOINTVCOM_IN       ENDPOINT3_IN
#define ENDPOINTVCOM_OUT      ENDPOINT3_OUT
#else
#define ENDPOINTVCOM_IN       ENDPOINT1_IN
#define ENDPOINTVCOM_OUT      ENDPOINT1_OUT
#endif

/*********************************************************/
/* Define USB data structures                            */
/*********************************************************/

typedef struct tagUSBCONFIGINFO {
 UINT32 Config_Total;
 UINT32 Interface_Total;
 UINT32 Alt_Interface_Total;
 UINT32 EndPoint_Total; 					/* Physical endpoints including Enpoint 0 */
} USBCONFIGINFO, *PUSBCONFIGINFO;


typedef struct tagUSBCONFIGSTRUCT {
 UINT32 dwEPAddr;
 UINT32 dwEPMaxPacketSize;
 UINT32 dwEPDirection;
 UINT32 dwEPType;
 UINT32 dwEPPoll;
 UINT32 dwEPAltSetting;
 UINT32 dwEPInterfaceNum;
 UINT32 dwEPConfiguration;
 UINT32 dwEPLogicalNum;
} USBCONFIGSTRUCT, *PUSBCONFIGSTRUCT;

typedef struct tagDEVICEDESCSTRUCT {
	UINT8 cDescLength;
	UINT8 cDescType;
	UINT16 wUSBSpec;
	UINT8 cDevClass;
	UINT8 cDevSubClass;
	UINT8 cDevProtocol;
	UINT8 cMaxPacketSize;
	UINT16 wVendorID;
	UINT16 wProductID;
	UINT16 wDevId;
	UINT8 cManuStr;
	UINT8 cProductStr;
	UINT8 cSerialNumStr;
	UINT8 cNumOfConfig;
} DEVICEDESCSTRUCT, *PDEVICEDESCSTRUCT;


typedef struct tagCONFIGDESCSTRUCT {
	UINT8 cDescLength;
	UINT8 cDescType;
	UINT16  wTotalLength;
	UINT8 cNumOfInterface;
	UINT8 cConfigValue;
	UINT8 cConfigStr;
	UINT8 cAttribute;
	UINT8 cMaxPower;
} CONFIGDESCSTRUCT, *PCONFIGDESCSTRUCT;


typedef struct tagINTERFACEDESCSTRUCT {
	UINT8 cDescLength;
	UINT8 cDescType;
	UINT8 cInterfaceNum;
	UINT8 cAltSetting;
	UINT8 cNumOfEP;
	UINT8 cInterClass;
	UINT8 cInterSubClass;
	UINT8 InterProtocol;
	UINT8 InterfaceStr;
} INTERFACEDESCSTRUCT, *PINTERFACEDESCSTRUCT;


typedef struct tagEPDESCSTRUCT {
	UINT8 cDescLength;
	UINT8 cDescType;
	UINT8 cEPAddress;
	UINT8 cAttribute;
	UINT16  wEPMaxPacketSize;
	UINT8 cPollInterval;
} EPDESCSTRUCT, *PEPDESCSTRUCT;


typedef struct tagSETUPDATASTRUCT {
	UINT8 cRequestType;
	UINT8 cRequest;
	UINT16  wValue;
	UINT16  wIndex;
	UINT16  wLength;
} SETUPDATASTRUCT, *PSETUPDATASTRUCT;


typedef struct tagCONTROLTRANSFER
{
	SETUPDATASTRUCT SetUpData;
	UINT16 wLength;
	UINT16 wCount;
	UINT8  *pData;
	UINT8  dataBuffer[USB_MAX_PACKET_SIZE];
} CONTROLTRANSFER, *PCONTROLTRANSFER;

typedef struct tagTXBUFFERSTRUCT
{
   UINT32 dwTxDesc;
   UINT32 dwReserved;
	UINT32 dwData[USB_MAX_PACKET_SIZE/4];
	volatile UINT32 *dwpTxLink;
	UINT32 dwTxLinkCount;
} TXBUFFERSTRUCT, *PTXBUFFERSTRUCT;


/*********************************************************/
/* Define USB variables.                                 */
/*********************************************************/

extern void (*VendorDeviceRequest[USB_MAX_REQ_NUM])(void);
extern void (*StandardDeviceRequest[USB_MAX_REQ_NUM])(SETUPDATASTRUCT * SetUpData);
extern UINT32 (*USBEPRxHandler_Tbl[MAX_NUM_ENDPOINT])(volatile UINT8 *cData, UINT32 dwNumRxBytes);

extern RXBUFFERSTRUCT RxBuffer;

extern EPINFOSTRUCT EPCommonInfo;
extern EPINFOSTRUCT EP0Info;
extern EPINFOSTRUCT EP1Info;
extern EPINFOSTRUCT EP2Info;
extern EPINFOSTRUCT EP3Info;

extern BOOL bUSBDebounce;
extern BOOL bUSBResume;

extern UINT32 PCLKNormalRate_Table[3];
extern UINT32 PCLKSlowRate_Table[3];

/*********************************************************/
/* Define USB functions.                                 */
/*********************************************************/

BOOL sysUSBHwInit(void);
BOOL UDCConfigure(void);
BOOL UDCWriteConf( PUSBCONFIGSTRUCT PUsbEPInfo, PUSBCONFIGINFO  PUsbConfInfo );

void USBDMAInit(void);
INT32 USBWriteBuffer(eEPDirection eEPDir, volatile UINT32 *dwTxBuff, UINT32 dwByteCount);
INT32 USBWriteDMA(eEPDirection eEPDir, UINT8 *pSrcBuff, UINT32 dwByteCount);
INT32 USBAddPacketToLink(eEPDirection eEPDir, UINT8* pSBuff, UINT32 dwByteCount);

void USBSetupHandler(SETUPDATASTRUCT * SetUpData);

void GetStatus(SETUPDATASTRUCT * SetUpData);
void ClearFeature(SETUPDATASTRUCT * SetUpData);     
void SetFeature(SETUPDATASTRUCT * SetUpData);
void SetAddress(SETUPDATASTRUCT * SetUpData);      
void GetDescriptor(SETUPDATASTRUCT * SetUpData);
void SetDescriptor(SETUPDATASTRUCT * SetUpData);
void GetConfiguration(SETUPDATASTRUCT * SetUpData);
void SetConfiguration(SETUPDATASTRUCT * SetUpData);
void GetInterface(SETUPDATASTRUCT * SetUpData);
void SetInterface(SETUPDATASTRUCT * SetUpData);
void SynchFrame(SETUPDATASTRUCT * SetUpData);

void SendDevDescriptor(UINT16 wLength);
void SendConfigDescriptor(UINT16 wLength);
BOOL SendStringDescriptor( UINT16 wValue, UINT16 wLength );

void IntEPInit(void);

BOOL EPConfParser(volatile UINT8* P_Dev_Desc_Tbl,volatile UINT8* P_Dev_Cfg_Tbl,PUSBCONFIGSTRUCT PUsbEPInfo, PUSBCONFIGINFO  PUsbConfInfo);

#endif /* USBDEF_H */

