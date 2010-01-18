/******************************************************************************

Cl2.H - PCAN and VCAN cards hardware access library
""""""""
                   (C) Copyright 1997   Unicontrols a.s.

PROJEKT            :    CANopen
AUTOR              :    F.Spurny
FIRMA              :    CVUT FEL, Dept. of Measurement
PORTING            :    Adapted for LinCAN driver by Pavel Pisa, OCERA team member

DULEZITE UPOZORNENI:

ARCHIVACNI INFORMACE:
Log: unican_cl2.h,v 

27.1.1999 - corected value of CL2_ERROR_WARNING and CL2_ERROR_BUS_OFF constants

12.12.2000 - CL2_STATUS_RESET_CARD, CL2_ERROR_RESET_CARD added
             fields of sCAN_MESSAGE aligned

10.8.2001 - PCI PCAN ID registers content defined
	- cl2_find_card function prototype added
	- cl2_get_ID function prototype added

10.2.2004 - Start of PCAN support porting to RT-Linux and Linux environment

===============================================================================

UCEL A FUNKCE:

******************************************************************************/

#ifndef __INC_CL2_H
#define __INC_CL2_H


/****** includes ******/

#ifndef __INC_ANCTYPES_H
#include "unican_types.h"
#endif

/****** Definitions of constants  ******/

/* PCI card Configuration Space Constants */
#define PCANDeviceID			0x0101
#define PCANVendorID		    	0xFA3C
#define PCANSubsystemID			0x0001
#define PCANSubsystemVendorID		0x7A52


/* Description of card and card registers location. Registers location are
 * mapped relative to card base address (BA) */

#define CL2_RAM_SIZE			0x1000	/* size of dual-port RAM */
#define CL2_RX_BUFFER_DEFAULT		0x800	/* off(BA->rx buffer) */
#define CL2_SYNC_BUFFER_DEFAULT		0x400	/* off(BA->sync buffer) */
#define CL2_ASYNC_BUFFER_DEFAULT	0x600	/* off(BA->async buffer) */
#define CL2_COMMAND_REGISTER		0x3FE	/* off(BA->command register )*/
#define CL2_VERSION_REGISTER		0x3F6	/* off(BA->version register )*/
#define CL2_ID_REGISTER			0x3EE	/* off(BA->ID register) */
#define CL2_GEN_INT_REGISTER		0x3E4	/* off(BA->generate interrupt reg.)*/
#define CL2_CLEAR_INT_REGISTER		0x3E2	/* off(BA->clear interrupt reg)*/
#define CL2_RESET_REGISTER		0x3E0	/* off(BA->reset register)*/
#define CL2_ERROR_REGISTER		0x3F8	/* off(BA->error register)*/
#define CL2_TIME_REGISTER			0x3FC	/* currently not used */
#define CL2_START_IT_REGISTER		0x3E6	/* off(BA->start inhibit time reg.) */
#define CL2_STATUS_REGISTER		0x3FA	/* off(BA->status register)*/
#define CL2_VME_INT_VECTOR		0x3F0
#define CL2_DATA_BUFFER			0x100	/* off(BA->data buffer)*/

/* Default sizes of buffers (in messages)
 * FYI: 1 message needs 16 bytes to be stored */
#define CL2_RX_BUFFER_SIZE				128	/* size of receive (rx) buffer */
#define CL2_TX_SYNC_BUFFER_SIZE		32		/* size of synchronnous buffer */
#define CL2_TX_ASYNC_BUFFER_SIZE		32		/* size of asynchronnous buffer*/

/* Command valid flag */
#define CL2_COMMAND_VALID			 0x0080

/* Message flags */
#define CL2_MESSAGE_VALID         0x0001

/* status bits */
#define CL2_DATA_IN_RBUF          0x0001   /* message is in rx buffer*/
#define CL2_X_DATA_IN_RBUF        0x0002   /* more than LIMIT messages are in
														  * rx buffer*/
#define CL2_RBUF_OVERFLOW         0x0004   /* owerflow of rx buffer, some
                                            * messages are lost*/
#define CL2_SYNC_QUEUE_EMPTY      0x0010   /* sync queue is empty */
#define CL2_ASYNC_QUEUE_EMPTY     0x0020   /* async queue is empty */
#define CL2_CARD_ERROR            0x0040   /* card reports an error */
#define CL2_STATUS_VALID_FLAG     0x0080   /* bit indicating that status
                                            * register contains valid data*/
#define CL2_CARD_READY            0x0100   /* card is ready */
#define CL2_SYNC_PASSIVE          0x0200   /* passive SYNC mode */
#define CL2_SYNC_ACTIVE           0x0400   /* active SYNC mode */
#define CL2_RTR_LIST              0x0800   /* RTR list sending */
#define CL2_STATUS_RESET_CARD     0x8000   /* card reset occurred */

/* error bits */
#define CL2_ERROR_LL              0x0007   /*  */
#define CL2_ERROR_WARNING         0x0040   /* bus warning detected */
#define CL2_ERROR_BUS_OFF         0x0080   /* bus error detected */
#define CL2_ERROR_RESET_CARD      0x0100   /* card reset occurred */
#define CL2_ERROR_FIRMWARE        0x1000   /* firmware error detected */
#define CL2_ERROR_DPRAM           0x2000   /* dual port RAM error detected */
#define CL2_ERROR_RAM             0x4000   /* internal RAM error detected */
#define CL2_ERROR_CAN             0x8000   /* CAN controller error detected */

/* interrupt generation */
#define INT_MODE_RX               0x01     /* if data are in receive buffer*/
#define INT_MODE_ERROR            0x02     /* if any error occurs */
#define INT_MODE_SYNC_EMPTY       0x04     /* if tx sync. queue is empty*/
#define INT_MODE_ASYNC_EMPTY      0x08     /* if tx async. queue is empty*/
#define INT_MODE_ALL              0x0F     /* if any event occurs*/

/* CAN message types */
#define CL2_REMOTE_FRAME          0x08     /* frame is a remote frame*/
#define CL2_LINE_FLAG             0x80
#define CL2_EXT_FRAME             0x04     /* frame is extended format*/

/* Receive message flags */
#define CL2_FRAME_VALID           0x01     /* message in buffer is valid */
#define CL2_RX_OVERFLOW           0x02     /* stored unread message was
                                            * overwritten by another one */

/* Bitrates */
#define CL2_BITRATE_5K            0x7f7f   /* bit-rate 5 kb/s */
#define CL2_BITRATE_10K           0x5c67   /* bit-rate 10 kb/s */
#define CL2_BITRATE_20K           0x5c53   /* bit-rate 20 kb/s */
#define CL2_BITRATE_50K           0x5c47   /* bit-rate 50 kb/s */
#define CL2_BITRATE_100K          0x5c43   /* bit-rate 100 kb/s */
#define CL2_BITRATE_125K          0x6743   /* bit-rate 125 kb/s */
#define CL2_BITRATE_200K          0x5c41   /* bit-rate 200 kb/s */
#define CL2_BITRATE_250K          0x6741   /* bit-rate 250 kb/s */
#define CL2_BITRATE_500K          0x6740   /* bit-rate 500 kb/s */
#define CL2_BITRATE_800K          0x3440   /* bit-rate 800 kb/s */
#define CL2_BITRATE_1M            0x2340   /* bit-rate 1 Mb/s */


/****** Definition of structures  ******/

typedef struct  /*** card definition structure ***/
	{
	U16  intNumber;         /* Card interrupt (IRQ) number */
	U8   *baseAddressPtr;   /* Pointer to card base address (BA) */
	U8   *rxBufPtr;         /* Pointer to receive buffer */
	U8   *rxBufBase;        /* Pointer to empty receive buffer */
	U8   *asyncTxBufPtr;    /* Pointer to async transmit buffer */
	U8   *asyncTxBufBase;   /* Pointer to empty async. transmit buffer */
	U8   *syncTxBufPtr;     /* Pointer to sync. transmit buffer */
	U8   *syncTxBufBase;    /* Pointer to empty sync. transmit buffer */
	U8   *commandRegister;  /* Pointer to command register */
	U8   *dataPtr;          /* Pointer to command data buffer */
	U16  rxBufSize;         /* size of receive buffer (x16 bytes) */
	U16  syncTxBufSize;     /* size of sync. transmit buffer (x16 bytes)*/
	U16  asyncTxBufSize;    /* size of async. transmit buffer (x 16 bytes)*/
	U16  status;            /* last card status */
	U16  error;             /* last not reported card error */
	U32 rtrSub[10];
	} sCAN_CARD;

//typedef struct  /*** CAN message formet ***/
//	{
//	U8   data[8];           /* Data message buffer (8 bytes) */
//	U8   dataLength;        /* Data length (in bytes) */
//	U32  COB_ID;            /* COB_ID */
//	U16  timeStamp;         /* Message time stamp */
//	U8   dataType;          /* Message data type */
//	} sCAN_MESSAGE;

typedef struct
   {
   U32  COB_ID;             /* COB identifier */
   U8   dataType;          /* Message data type */
   U8   dataLength;        /* Data length (in bytes) */
   U8   data[8];           /* Data message buffer (8 bytes) */
   U16  timeStamp;         /* Message time stamp [us] */
   } sCAN_MESSAGE;


typedef	struct  /*** Remote Request RTR frame ***/
   {
   U32  cob_id;           /* RTR frame ID */
   U16  period;           /* RTR period */
   U16  subperiod;        /* RTR subperiod */
   } sRTR_FRAME;

typedef struct  /*** Remote Request (RTR) list ***/
	{
	sRTR_FRAME *data;       /* RTR data */
	U32 nb;                 /* Number of RTR definitions */
	} sRTR_LIST;

typedef enum  /*** CL2 functions return codes ***/
	{
	CL2_OK = 0,           /* OK */
	CL2_NO_REQUEST,       /* No request*/
	CL2_HW_FAILURE,       /* HW failure */
	CL2_HW_QUEUE_FULL,    /* Transmit queue full */
	CL2_BAD_PARAM,        /* Bad number of parameters */
   CL2_HW_QUEUE_EMPTY,   /* Receive queue empty */
	CL2_COMMAND_BUSY,     /* Command busy - previous command not completed */
	CL2_UNKNOWN_COMMAND,  /* Unknown command */
	CL2_NO_PCI_BIOS		 /* missing PCI BIOS support */
   } eCL2_RESULT;

typedef enum  /*** CL2 commands ***/
   {
   cmCL2_INT_MODE = 0x01,            /**01 - Set Interrupt mode */
   cmCL2_IIT_MODE,                   /**02 - Inhibit Interrupt Time mode */
   cmCL2_SYNC_MODE,                  /**03 - SYNC mode */
	cmCL2_RTR_MODE,                   /**04 - RTR list mode */
	cmCL2_BUF_SIZE,                   /**05 - Set buffers sizes */
   cmCL2_SET_IIT,                    /**06 - Start Inhibit Interrupt Time */
   cmCL2_START_FIRMWARE,             /**07 - Start firmware */
   cmCL2_LOAD_FIRMWARE,              /* 08 - Load firmware */
	cmCL2_SET_REC_MODE,               /**09 - Set Receive mode */

   cmCL2_CLR_RX_BUFFER = 0x10,       /**10 - Clear receive buffer */
   cmCL2_CLR_SYNC_BUFFER,            /**11 - Clear synchronous buffer */
   cmCL2_CLR_ASYNC_BUFFER,           /**12 - Clear asynchronous buffer */
	cmCL2_SEND_TIME_SYNC,             /**13 - Sends time synchronization */
	cmCL2_SET_TIME_COBID,             /**14 - Sets time frames COB-ID */
   cmCL2_SET_RECEIVE_LIMIT,          /* 15 - Sets receive limit */

	cmCL2_DOWNLOAD_RTR_LIST = 0x20,   /**20 - Download RTR list */
   cmCL2_SUBSCRIBE_RTR,              /**21 - Subscribe RTR */
   cmCL2_DESUBSCRIBE_RTR,            /* 22 - Desubscribe RTR */

   cmCL2_SET_COBID = 0x30,           /**30 - Set COB-ID */
   cmCL2_SET_SYNC_PERIOD,            /**31 - Set SYNC period */
   cmCL2_SET_SYNC_WINDOW,            /**32 - Set SYNC period window */

	cmCL2_SET_BITRATE = 0x40,         /**40 - Set CAN bit rate */
	cmCL2_BUS_RESET                   /* 41 - CAN controller reset */

	} eCLU2_COMMAND;


/****** prototypes ******/


/*******************************************************************************
* cl2_find_card - find PCIPCAN card
* """""""""""""
*
* Searches for PCIPCAN cards in the system. The mapping (memory and interrupt)
* of the card is found. An index parameter specifies how many PCIPCAN cards have
* to be skipped in search.
*
* RETURNS:
*   CL2_OK - card found and mapped under 1 MB
*   CL2_HW_FAILURE - card not found or not mapped under 1 MB
*/
eCL2_RESULT cl2_find_card
	 (
	 U16 *baseAddress,   		 /* card's physical base address */
	 U16 *intNumber,     		 /* card's interrupt level */
	 U16 index           	    /* number of PCIPCAN cards to be skipped */
	 );


/*******************************************************************************
* cl2_init_card - initialize card to default parameters
* """""""""""""
*
* Command installs card. The data in sCAN_CARD structure pointed
* by *card are initialized to their predefined default values.
* Command must be called before any operation with sCAN_CARD
* structure.
*
* RETURNS:
*   CL2_OK - command completed succesfuly
*
*/
eCL2_RESULT cl2_init_card
	(
	sCAN_CARD *card,            /* Pointer to card structure */
	void *baseAddress,          /* Card base address pointer */
	U16 intNumber               /* Card interrupt number */
	);


/*******************************************************************************
* cl2_test_card - test card
* """""""""""""
*
* Test whether the card is installed in system and working properly
* or not. If this function fails (return value is CL2_HW_FAILURE)
* check if the card is present and card base address.
*
* RETURNS:
*   CL2_OK - card is present and working properly
*   CL2_HW_FAILURE - card not found or card error
*
*/
eCL2_RESULT cl2_test_card
	(
	sCAN_CARD *card           /* Pointer to card structure */
	);


/*******************************************************************************
* cl2_reset_card - reset card
* """"""""""""""
*
* Card pointed by *card gets hardware reset. This command resets
* card processor, card settings are restored to their default
* values.
*
* RETURNS:
*   CL2_OK - command completed successfuly
*
*/
eCL2_RESULT cl2_reset_card
   (
   sCAN_CARD *card             /* Pointer to card structure */
   );


/*******************************************************************************
* cl2_get_version - read card version
* """""""""""""""
*
* Fucntion reads a value from card version register.
*
* RETURNS:
*   CL2_OK - command completed successfuly
*
*/
eCL2_RESULT cl2_get_version
   (
   sCAN_CARD *card,            /* Pointer to card structure */
   U16 *version                /* returns card version */
   );


/*******************************************************************************
* cl2_get_ID - read card ID
* """""""""""""""
*
* Fucntion reads a value from card ID register.
*
* RETURNS:
*   CL2_OK - command completed successfuly
*
*/
eCL2_RESULT cl2_get_ID
   (
   sCAN_CARD *card,            /* Pointer to card structure */
   U32 *IDlow,
   U32 *IDhigh                         /* returns card version */
   );


/*******************************************************************************
* cl2_gen_interrupt - request for interrupt
* """""""""""""""""
*
* CAN card is requested to generate interrupt if there is any reason
* to do it. The condition for interrupt generation is defined by
* cl2_int_mode command.
*
* RETURNS:
*   CL2_OK - command completed successfuly
*
*/
eCL2_RESULT cl2_gen_interrupt
   (
   sCAN_CARD *card           /* Pointer to card structure */
   );


/*******************************************************************************
* cl2_start_it - start inhibit time
* """"""""""""
*
* Command starts interrupt inhibit time. If there is any reason for
* interrupt geneation, the card generates interrupt after end of
* specified time interval. Time interval is set by cl2_set_iit command.
*
* RETURNS:
*   CL2_OK - command completed successfuly
*
*/
eCL2_RESULT cl2_start_it
   (
   sCAN_CARD *card           /* Pointer to card structure */
   );


/*******************************************************************************
* cl2_clear_interrupt - clear interrupt
* """""""""""""""""""
*
* Comand clears interrupt (IRQ) generated by a card.
*
* RETURNS:
*   CL2_OK - command completed successfuly
*
*/
eCL2_RESULT cl2_clear_interrupt
   (
   sCAN_CARD *card          /* Pointer to card structure */
   );


/*******************************************************************************
* cl2_int_mode - set interrupt mode
* """"""""""""
*
* Command controls, which event will generate interrupt. Constants
* CL2_INT_XXXX are used for setting of interrupt mode.
*
* RETURNS:
*   CL2_OK - command completed successfuly
*   CL2_BAD_PARAM - bad command parameter
*   CL2_COMMAND_BUSY - previous command not completed
*
*/
eCL2_RESULT cl2_int_mode
   (
   sCAN_CARD *card,            /* Pointer to card structure */
   U16 mode                    /* Interrupt mode */
   );


/*******************************************************************************
* cl2_iit_mode - inhibit interrupt time mode
* """"""""""""
*
* Command enables/disables inhibit interupt time mode.
*
* RETURNS:
*   CL2_OK - command completed successfuly
*   CL2_COMMAND_BUSY - previous command not completed
*
*/
eCL2_RESULT cl2_iit_mode
   (
   sCAN_CARD *card,            /* Pointer to card structure */
   BOOLEAN1 onoff              /* IIT mode - TRUE=on, FALSE=off */
   );


/*******************************************************************************
* cl2_sync_mode - sync mode
* """""""""""""
*
* Command enables/disables transmission of SYNC frames.
*
* RETURNS:
*   CL2_OK - command completed successfuly
*   CL2_COMMAND_BUSY - previous command not completed
*
*/
eCL2_RESULT cl2_sync_mode
   (
   sCAN_CARD *card,            /* Pointer to card structure */
   BOOLEAN1 onoff              /* Sync mode - TRUE=on, FALSE=off */
   );


/*******************************************************************************
* cl2_rtr_mode - rtr mode
* """"""""""""
*
* Command enables/disables automatic transmission of RTR frames
*
* RETURNS:
*   CL2_OK - command completed successfuly
*   CL2_COMMAND_BUSY - previous command not completed
*
*/
eCL2_RESULT cl2_rtr_mode
   (
   sCAN_CARD *card,            /* Pointer to card structure */
   BOOLEAN1 onoff              /* RTR mode - TRUE=on, FALSE=off */
   );


/*******************************************************************************
* cl2_buf_size - size of synchronous queue
* """"""""""""
*
* Command sets the size of synchronous send buffer. The size is
* in numbers of messages. Default buffer size is 32 messages.
* The sum of synchronous_buffer_size and asynchronous_buffer_size
* is constant and equal to 64. So, if the size od synchronous
* buffer increases, the size of asynchronous buffer decreases and
* vice versa.
* NOTE: 1 message = 16 bytes
*
* RETURNS:
*   CL2_OK - command completed successfully
*   CL2_COMMAND_BUSY - previous command not completed
*   CL2_BAD_PARAM - bad command parameter (bufSize>64)
*
*/
eCL2_RESULT cl2_buf_size
   (
   sCAN_CARD *card,            /* Pointer to card structure */
   U16 bufSize                 /* Size of synchronous buffer */
   );


/*******************************************************************************
* cl2_set_iit - set value of inhibit interrupt time
* """""""""""
*
* Command sets value of inhibit interrupt time. If inhibit
* interrupt time mode is enabled and started, generation of
* interrupt (IRQ) is disabled during this time period.
* Inhibit interrupt time can be set from 100 us to 6.5535 s
*
* RETURNS:
*   CL2_OK - command completed successfuly
*   CL2_COMMAND_BUSY - previous command not completed
*
*/
eCL2_RESULT cl2_set_iit
   (
   sCAN_CARD *card,            /* Pointer to card structure */
   U16 iit                     /* time period in x100 us */
   );


/*******************************************************************************
* cl2_start_firmware - start firmware
* """"""""""""""""""
*
* Command starts card firmware
*
* RETURNS:
*   CL2_OK - command completed successfuly
*   CL2_COMMAND_BUSY - previous command not completed
*
*/
eCL2_RESULT cl2_start_firmware
   (
   sCAN_CARD *card             /* Pointer to card structure */
   );


/*******************************************************************************
* cl2_set_rec_mode - set receive mode
* """"""""""""""""
*
* Command sets card receive mode. This enable reception of standard
* or extended frames according to CAN 2.0A and 2.0B specifications.
* If value of mode is TRUE, card receives extended frames, if mode
* is FALSE, card receives standard massage format (default).
*
* RETURNS:
*   CL2_OK - command completed successfuly
*   CL2_COMMAND_BUSY - previous command not completed
*
*/
eCL2_RESULT cl2_set_rec_mode
   (
   sCAN_CARD *card,           /* Pointer to card structure */
   BOOLEAN1 mode              /* Mode - TRUE=ext, FALSE=std */
   );


/*******************************************************************************
* cl2_clr_rx_buffer - clear RX buffer
* """""""""""""""""
*
* Command clears receive (rx) buffer. All messages stored in
* rx buffer will be lost.
*
* RETURNS:
*   CL2_OK - command completed successfuly
*   CL2_COMMAND_BUSY - previous command not completed
*
*/
eCL2_RESULT cl2_clr_rx_buffer
   (
   sCAN_CARD *card            /* Pointer to card structure */
   );


/*******************************************************************************
* cl2_clr_sync_buffer - clear synchronous buffer
* """""""""""""""""""
*
* Command clears synchronous send buffer. All messages stored
* in synchronous buffer will be lost.
*
* RETURNS:
*   CL2_OK - command completed successfuly
*   CL2_COMMAND_BUSY - previous command not completed
*
*/
eCL2_RESULT cl2_clr_sync_buffer
   (
   sCAN_CARD *card             /* Pointer to card structure */
   );


/*******************************************************************************
* cl2_clr_async_buffer - clear asynchronous buffer
* """"""""""""""""""""
*
* Command clears asynchronnous send buffer. All messages stored
* in async buffer will be lost.
*
* RETURNS:
*   CL2_OK - command completed successfuly
*   CL2_COMMAND_BUSY - previous command not completed
*
*/
eCL2_RESULT cl2_clr_async_buffer
   (
   sCAN_CARD *card            /* Pointer to card structure */
   );


/*******************************************************************************
* cl2_send_time_sync - send time synchronization
* """"""""""""""""""
*
* Command forces the card to start the High Resolution Synchronization
* Protocol according to the CANopen Communication profile. The SYNC
* mode has to be enabled (cl2_sync_mode) otherwise this command has
* no effect.
*
* RETURNS:
*   CL2_OK - command completed successfuly
*   CL2_COMMAND_BUSY - previous command not completed
*
*/
eCL2_RESULT cl2_send_time_sync
   (
   sCAN_CARD *card             /* Pointer to card structure */
   );


/*******************************************************************************
* cl2_set_time_cobid - set time COB-ID
* """"""""""""""""""
*
* Command sets the COB-ID for high resolution synchronization
* frame. The synchronization can be then made by means of
* cl2_send_time_sync command.
*
* RETURNS:
*   CL2_OK - command completed successfuly
*   CL2_COMMAND_BUSY - previous command not completed
*
*/
eCL2_RESULT cl2_set_time_cobid
   (
   sCAN_CARD *card,           /* Pointer to card structure */
   U32 COBID                  /* HRS frame COB-ID */
   );


/*******************************************************************************
* cl2_set_receive_limit - set limit for receive signaling
* """""""""""""""""""""
*
* Command is used to set the receive limit signalized by bit
* RL (in CL2.H CL2_X_DATA_IN_RBUF) of the Status Register.
* This bit is set when more then the limit number of frames
* was received since the last interrupt was generated (in interrupt
* mode) or since the Status Register was last time read.
*
* RETURNS:
*   CL2_OK
*   CL2_COMMAND_BUSY - previous command not completed
*   CL2_BAD_PARAM - bad command parameter
*/
eCL2_RESULT cl2_set_receive_limit
   (
   sCAN_CARD *card,          /* pointer to card structure */
   U16 limit                 /* limit of messages in receive buffer */
   );


/*******************************************************************************
* cl2_download_rtr_list - download rtr list
* """""""""""""""""""""
*
* Command downloads a list of up to 64 RTR frames. These frames are
* periodically transmitted by the card. The parameters, how often
* frames are send and in which SYNC period is defined by period and
* subperiod in sRTR_FRAME structure.
*
* RETURNS:
*   CL2_OK - command completed successfuly
*   CL2_COMMAND_BUSY - previous command not completed
*   CL2_BAD_PARAM - bad command parameter
*
*/
eCL2_RESULT cl2_download_rtr_list
   (
   sCAN_CARD *card,           /* Pointer to card structure */
   sRTR_LIST *rtrList         /* RTR list */
   );


/*******************************************************************************
* cl2_subscribe_rtr - subscribe RTR frame
* """""""""""""""""
*
* Command subscribes RTR frame. Incoming RTR frames which were
* subscribed are accepted, while other are ignored. Up to 10
* RTR frames can be subscribed.
*
* RETURNS:
*   CL2_OK - command completed successfuly
*   CL2_COMMAND_BUSY - previous command not completed
*   CL2_BAD_PARAM - bad command parameter
*
*/
eCL2_RESULT cl2_subscribe_rtr
   (
   sCAN_CARD *card,            /* Pointer to card structure */
   sCAN_MESSAGE *canMessage,   /* RTR frame */
   U16 RTRnumber               /* number of RTR */
   );


/*******************************************************************************
* cl2_desubscribe_rtr - desubscribe rtr frame
* """""""""""""""""""
*
* Command desubscribes RTR frame. Card will not accept RTR frames
* with this identifier.
*
* RETURNS:
*   CL2_OK - command completed successfuly
*   CL2_COMMAND_BUSY - previous command not completed
*   CL2_BAD_PARAM - bad command parameter
*
*/
eCL2_RESULT cl2_desubscribe_rtr
   (
   sCAN_CARD *card,            /* Pointer to card structure */
   sCAN_MESSAGE *canMessage    /* RTR frame */
   );


/*******************************************************************************
* cl2_set_sync_cobid - set COB-ID
* """"""""""""""""""
*
* Command sets COB-ID of SYNC frame. In active SYNC mode, the SYNC
* frame with this COB-ID is periodically sent with period defined
* by cl2_set_sync_period command.
*
* RETURNS:
*   CL2_OK - command completed successfuly
*   CL2_COMMAND_BUSY - previous command not completed
*
*/
eCL2_RESULT cl2_set_sync_cobid
   (
   sCAN_CARD *card,            /* Pointer to card structure */
   U32 COBID                   /* COB-ID */
   );


/*******************************************************************************
* cl2_set_sync_period - set SYNC period
* """""""""""""""""""
*
* Coomand sets the SYNC frame send period in active SYNC mode in
* x100 us. The period range is from 0 to 0xFFFF (SYNC period can
* be set from 100us to 6.5535s).
*
* RETURNS:
*   CL2_OK - command completed successfuly
*   CL2_COMMAND_BUSY - previous command not completed
*
*/
eCL2_RESULT cl2_set_sync_period
   (
   sCAN_CARD *card,            /* Pointer to card structure */
   U16 period                  /* period in x100 us */
   );


/*******************************************************************************
* cl2_set_sync_window - set SYNC window
* """""""""""""""""""
*
* Command sets the SYNC window length. Only during this time period
* after SYNC frame was send or receive the frames from the synchronous
* send buffer can be sent.
*
* RETURNS:
*   CL2_OK - command completed successfuly
*   CL2_COMMAND_BUSY - previous command not completed
*
*/
eCL2_RESULT cl2_set_sync_window
   (
   sCAN_CARD *card,            /* Pointer to card structure */
   U16 window                  /* period in x100 us */
   );


/*******************************************************************************
* cl2_set_bitrate - set CAN bit-rate
* """""""""""""""
*
* Command switches the bus bit-rate. There are some predefined
* constants CL2_BITRATE_XXXX.
*
* RETURNS:
*   CL2_OK - command completed successfuly
*   CL2_COMMAND_BUSY - previous command not completed
*
*/
eCL2_RESULT cl2_set_bitrate
   (
   sCAN_CARD *card,            /* Pointer to card structure */
   U16 bitrate                 /* CAN bitrate */
   );


/*******************************************************************************
* cl2_bus_reset - resets CAN controller
* """""""""""""
*
* Command resets CAN controller
*
* RETURNS:
*   CL2_OK - command completed successfuly
*   CL2_COMMAND_BUSY - previously command not completed
*
*/
eCL2_RESULT cl2_bus_reset
   (
   sCAN_CARD *card            /* Pointer to card structure */
   );


/*******************************************************************************
* cl2_send_sync - sends synchronous frame
* """""""""""""
*
* Command stores massage in synchronous send buffer.
*
* RETURNS:
*   CL2_OK - command completed successfuly
*   CL2_HW_QUEUE_FULL - synchronous send buffer is full
*   CL2_BAD_PARAM - bad command parameter
*   CL2_HW_FAILURE - error in HW configuration
*
*/
eCL2_RESULT cl2_send_sync
   (
   sCAN_CARD *card,           /* Pointer to card structure */
   sCAN_MESSAGE *message      /* message to be sent */
   );


/*******************************************************************************
* cl2_send_async - sends asynchronous frame
* """"""""""""""
*
* Command stores message in asynchronous send buffer.
*
* RETURNS:
*   CL2_OK - command completed successfuly
*   CL2_HW_QUEUE_FULL - asynchronous buffer full
*   CL2_HW_FAILURE - error in HW configuration
*   CL2_BAD_PARAM - bad command parameter
*
*/
eCL2_RESULT cl2_send_async
   (
   sCAN_CARD *card,           /* Pointer to card structure */
   sCAN_MESSAGE *message      /* message */
   );


/*******************************************************************************
* cl2_get_status - reads card status
* """"""""""""""
*
* Command reads card status register. If data in status register
* are valid (status valid flag is set), the value of status is read
* and stored in status and sCAN_CARD structure.
*
* RETURNS:
*   CL2_OK - command completed successfuly
*   CL2_NO_REQUEST - status is not valid
*
*/
eCL2_RESULT cl2_get_status
   (
   sCAN_CARD *card,           /* Pointer to card structure */
   U16 *status                /* Returned status */
   );


/*******************************************************************************
* cl2_get_error - reads card error
* """""""""""""
*
* Command reads card error register. If data in error register
* are valid (error register valid flag is set), the value of error
* register is read and stored in error and sCAN_CARD structure.
*
* RETURNS:
*   Cl2_OK - command completed successfuly
*
*/
eCL2_RESULT cl2_get_error
   (
   sCAN_CARD *card,           /* Pointer to card structure */
   U16 *error                 /* Returned card error code */
   );


/*******************************************************************************
* cl2_receive_data - reads received frame
* """"""""""""""""
*
* Command reads new messages received by a card.
*
* RETURNS:
*   CL2_OK - command commpleted successfuly
*   CL2_NO_REQUEST - there is no new message
*
*/
eCL2_RESULT cl2_receive_data
   (
   sCAN_CARD *card,             /* Pointer to card structure */
   sCAN_MESSAGE *canMessage     /* Message */
   );


/* *********************************************************************** *
 * END OF CL2.H                                                            *
 * *********************************************************************** */

#endif /* ifndef __INC_CL2_H */
