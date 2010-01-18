/******************************************************************************

Cl2.C - PCAN and VCAN cards hardware access library
"""""
                   (C) Copyright 1997   Unicontrols a.s.

PROJEKT            :    CANopen
AUTOR              :    F.Spurny
FIRMA              :    CVUT FEL, Dept. of Measurement

DULEZITE UPOZORNENI:

ARCHIVACNI INFORMACE:
Log: unican_cl2.c,v

12.12.2000, J.B., cl2_receive_data - time stamp for even number of bytes corr.
26.8.1998 - cl2_buf_size - corrected, new function return code CL2_BAD_PARAM
            cl2_clr_async_buffer - corrected

===============================================================================

UCEL A FUNKCE:

******************************************************************************/

/* Constnt used by CL2 functions */
#define CL2_TMP_RF                0x0800
#define CL2_TMP_EX                0x0400
#define CL2_TMP_EX2               0x0004
#define CL2_TMP_EXHIGH            0x80000000L


/* includes */
#include "../include/canmsg.h"
#include "../include/can_sysdep.h"
#include "../include/unican_types.h"
#include "../include/unican_cl2.h"
#include "linux/delay.h"


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
   sCAN_CARD *card,     /* Pointer to card structure */
   void *baseAddress,   /* Card base address pointer */
   U16 intNumber        /* Card interrupt number */
   )
   {
   int i;

   card->intNumber = intNumber;
   card->baseAddressPtr = (U8*)baseAddress;
   card->rxBufBase = card->baseAddressPtr + CL2_RX_BUFFER_DEFAULT;
   card->asyncTxBufBase = card->baseAddressPtr + CL2_ASYNC_BUFFER_DEFAULT;
   card->syncTxBufBase = card->baseAddressPtr + CL2_SYNC_BUFFER_DEFAULT;
   card->rxBufPtr = card->rxBufBase;
   card->asyncTxBufPtr = card->asyncTxBufBase;
   card->syncTxBufPtr = card->syncTxBufBase;
   card->commandRegister = card->baseAddressPtr+CL2_COMMAND_REGISTER;
   card->dataPtr = card->baseAddressPtr + CL2_DATA_BUFFER;
   card->rxBufSize = CL2_RX_BUFFER_SIZE;
   card->syncTxBufSize = CL2_TX_SYNC_BUFFER_SIZE;
   card->asyncTxBufSize = CL2_TX_ASYNC_BUFFER_SIZE;
   card->status = 0;
   for ( i = 0; i < 10; i++ ) card->rtrSub[i] = 0xFFFFFFFFL;
   return CL2_OK;
   } /* cl2_init_card */


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
   )
   {
   BOOLEAN1 isAA = FALSE, is55 = FALSE;
   int i;
   U16 volatile tmpWord;

   /* timeout for card testing - 1000 read cycles */
   for ( i = 0; i < 10000; i++ )
   {
     if ( isAA && is55 ) return CL2_OK;
     tmpWord = unican_readw(card->baseAddressPtr);
     /*printk("cl2_test_card: %08lx %04x\n", (long)card->baseAddressPtr, tmpWord);*/
     udelay(100);
     if ( (tmpWord & 0x00FF) == 0x00AA ) isAA = TRUE;
     if ( (tmpWord & 0x00FF) == 0x0055 ) is55 = TRUE;
   }

  return CL2_HW_FAILURE;
   } /* cl2_test_card */


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
   sCAN_CARD *card           /* Pointer to card structure */
   )
   {
   unican_writew(0x0000, card->baseAddressPtr + CL2_RESET_REGISTER);
   return CL2_OK;
   } /* cl2_reset_card */


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
   )
   {
   *version = unican_readw(card->baseAddressPtr + CL2_VERSION_REGISTER);
   return CL2_OK;
   } /* cl2_get_version */


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
   )
   {
   unican_writew(0x0000, card->baseAddressPtr + CL2_GEN_INT_REGISTER);
   return CL2_OK;
   } /* cl2_gen_interrupt */


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
   )
   {
   unican_writew(0x0000, card->baseAddressPtr + CL2_START_IT_REGISTER);
   return CL2_OK;
   } /* cl2_start_it */


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
   sCAN_CARD *card           /* Pointer to card structure */
   )
   {
   unican_writew(0x0000, card->baseAddressPtr + CL2_CLEAR_INT_REGISTER);
   return CL2_OK;
   } /* cl2_clear_interrupt */


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
   )
   {
   if ( mode > INT_MODE_ALL ) return CL2_BAD_PARAM;
   if ( unican_readw(card->commandRegister) & CL2_COMMAND_VALID )
      return CL2_COMMAND_BUSY;
   unican_writew(mode, card->dataPtr);
   unican_writew(((U16)cmCL2_INT_MODE + CL2_COMMAND_VALID), card->commandRegister);
   return CL2_OK;
   } /* cl2_int_mode */


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
   )
   {
   if ( unican_readw(card->commandRegister) & CL2_COMMAND_VALID ) return
      CL2_COMMAND_BUSY;
   unican_writew((U16)onoff, card->dataPtr);
   unican_writew(((U16)cmCL2_IIT_MODE + CL2_COMMAND_VALID), card->commandRegister);
   return CL2_OK;
   } /* cl2_iit_mode */


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
   )
   {
   if ( unican_readw(card->commandRegister) & CL2_COMMAND_VALID )
      return CL2_COMMAND_BUSY;
   unican_writew((U16)onoff, card->dataPtr);
   unican_writew(((U16)cmCL2_SYNC_MODE + CL2_COMMAND_VALID), card->commandRegister);
   return CL2_OK;
   } /* cl2_sync_mode */


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
   )
   {
   if ( unican_readw(card->commandRegister) & CL2_COMMAND_VALID )
      return CL2_COMMAND_BUSY;
   unican_writew((U16)onoff, card->dataPtr);
   unican_writew(((U16)cmCL2_RTR_MODE + CL2_COMMAND_VALID), card->commandRegister);
   return CL2_OK;
   } /* cl2_rtr_mode */


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
   )
   {
   if ( bufSize > 64 ) return CL2_BAD_PARAM;
   if ( unican_readw(card->commandRegister) & CL2_COMMAND_VALID )
      return CL2_COMMAND_BUSY;
   unican_writew(bufSize, card->dataPtr);
   unican_writew(((U16)cmCL2_BUF_SIZE + CL2_COMMAND_VALID), card->commandRegister);
   card->syncTxBufSize = bufSize;
   card->asyncTxBufSize = 64 - bufSize;
   card->syncTxBufPtr = card->syncTxBufBase;
   card->asyncTxBufPtr = card->asyncTxBufBase = card->syncTxBufBase+bufSize*16;
   return CL2_OK;
   } /* cl2_buf_size */


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
   )
   {
   if ( unican_readw(card->commandRegister) & CL2_COMMAND_VALID )
      return CL2_COMMAND_BUSY;
   unican_writew(iit, card->dataPtr);
   unican_writew(((U16)cmCL2_SET_IIT + CL2_COMMAND_VALID), card->commandRegister);
   return CL2_OK;
   } /* cl2_set_iit */


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
   sCAN_CARD *card            /* Pointer to card structure */
   )
   {
   if ( unican_readw(card->commandRegister) & CL2_COMMAND_VALID )
      return CL2_COMMAND_BUSY;
   unican_writew((U16)cmCL2_START_FIRMWARE + CL2_COMMAND_VALID, card->commandRegister);
   return CL2_OK;
   } /* cl2_start_firmware */


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
   )
   {
   if ( unican_readw(card->commandRegister) & CL2_COMMAND_VALID )
      return CL2_COMMAND_BUSY;
   unican_writew((U16)mode, card->dataPtr);
   unican_writew((U16)cmCL2_SET_REC_MODE + CL2_COMMAND_VALID, card->commandRegister);
   return CL2_OK;
   } /* cl2_set_rec_mode */


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
   sCAN_CARD *card           /* Pointer to card structure */
   )
   {
   if ( unican_readw(card->commandRegister) & CL2_COMMAND_VALID )
      return CL2_COMMAND_BUSY;
   card->rxBufPtr = card->rxBufBase;
   unican_writew((U16)cmCL2_CLR_RX_BUFFER + CL2_COMMAND_VALID, card->commandRegister);
   return CL2_OK;
   } /* cl2_clr_rx_buffer */


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
   sCAN_CARD *card            /* Pointer to card structure */
   )
   {
   if ( unican_readw(card->commandRegister) & CL2_COMMAND_VALID )
      return CL2_COMMAND_BUSY;
   card->syncTxBufPtr = card->syncTxBufBase;
   unican_writew((U16)cmCL2_CLR_SYNC_BUFFER + CL2_COMMAND_VALID, card->commandRegister);
   return CL2_OK;
   } /* cl2_clr_sync_buffer */


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
   )
   {
   if ( unican_readw(card->commandRegister) & CL2_COMMAND_VALID )
      return CL2_COMMAND_BUSY;
   card->asyncTxBufPtr = card->syncTxBufBase + card->syncTxBufSize*16;
   unican_writew((U16)cmCL2_CLR_ASYNC_BUFFER + CL2_COMMAND_VALID, card->commandRegister);
   return CL2_OK;
   } /* cl2_clr_async_buffer */


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
   )
   {
   if ( unican_readw(card->commandRegister) & CL2_COMMAND_VALID )
      return CL2_COMMAND_BUSY;
   unican_writew((U16)cmCL2_SEND_TIME_SYNC + CL2_COMMAND_VALID, card->commandRegister);
   return CL2_OK;
   } /* cl2_send_time_sync */


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
   )
   {
   U16 cobidL, cobidH;
   U16 *ptr = (U16 *)card->dataPtr;

   if ( unican_readw(card->commandRegister) & CL2_COMMAND_VALID )
      return CL2_COMMAND_BUSY;

   if ( COBID & CL2_TMP_EXHIGH ) /* standard or extended format? */
   { /* 2.0B frame */
     COBID <<= 3;
     cobidL = (U16)(COBID & 0x0000FFFFL);
     cobidH = (U16)((COBID & 0xFFFF0000L)>>16);
     cobidH = ((cobidH >> 8) & 0x00FF) | (((cobidH << 8) & 0xFF00));
     *ptr++ = cobidH;
     cobidL = ((cobidL >> 8) & 0x00FF) | (((cobidL << 8) & 0xFF00));
     *ptr++ = cobidL;
     *ptr = CL2_TMP_EX2;
   }
   else
   { /* 2.0A frame */
     COBID <<= 5;
     cobidH = (U16)(COBID & 0x0000FFFFL);
     cobidH = ((cobidH >> 8) & 0x00FF) | (((cobidH << 8) & 0xFF00));
     *ptr++ = cobidH;
     *ptr++ = 0;
     *ptr = 0;
   }

   unican_writew((U16)cmCL2_SET_TIME_COBID + CL2_COMMAND_VALID, card->commandRegister);
   return CL2_OK;
   } /* cl2_set_time_cobid */


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
   sCAN_CARD *card,  /* pointer to card structure */
   U16 limit         /* limit of messages in receive buffer */
   )
   {
   if ( limit > 127 ) return CL2_BAD_PARAM;
   if ( unican_readw(card->commandRegister) & CL2_COMMAND_VALID )
      return CL2_COMMAND_BUSY;
   unican_writew(limit, card->dataPtr);
   unican_writew((U16)cmCL2_SET_RECEIVE_LIMIT + CL2_COMMAND_VALID, card->commandRegister);
   return CL2_OK;
   } /* cl2_set_receive_limit */


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
   )
   {
   U16 *ptrTmp = (U16*)card->dataPtr;
   sRTR_FRAME *ptrRTR = rtrList->data;
   U16 tmpU16;
   U32 COBID;
   U16 cobidH, cobidL, i;

   if ( unican_readw(card->commandRegister) & CL2_COMMAND_VALID ) return
      CL2_COMMAND_BUSY;
   if ( rtrList->nb > 64 ) return CL2_BAD_PARAM;

   *ptrTmp++ = (U16)rtrList->nb;
   for ( i = 0; i < rtrList->nb; i++ )
   {
   if ( ptrRTR->period < ptrRTR->subperiod ) return CL2_BAD_PARAM;
   if ( ptrRTR->subperiod == 0 ) ptrRTR->subperiod = 1;
   tmpU16 = (ptrRTR->period & 0x00FF) + ((ptrRTR->subperiod & 0x00FF)<<8);
   *ptrTmp++ = tmpU16;
   COBID = ptrRTR->cob_id;

   if ( COBID & CL2_TMP_EXHIGH ) /* standard or extended format? */
   { /* 2.0B frame */
     COBID <<= 3;
     cobidL = (U16)(COBID & 0x0000FFFFL);
     cobidH = (U16)((COBID & 0xFFFF0000L)>>16);
     cobidH = ((cobidH >> 8) & 0x00FF) | (((cobidH << 8) & 0xFF00));
     *ptrTmp++ = cobidH;
     cobidL = ((cobidL >> 8) & 0x00FF) | (((cobidL << 8) & 0xFF00));
     *ptrTmp++ = cobidL;
   }
   else
   { /* 2.0A frame */
     COBID <<= 5;
     cobidH = (U16)(COBID & 0x0000FFFFL);
     cobidH = ((cobidH >> 8) & 0x00FF) | (((cobidH << 8) & 0xFF00));
     *ptrTmp++ = cobidH;
     *ptrTmp++ = 0;
   }

   *ptrTmp++ = 0x0000;  /* rezerva */
   ptrRTR++;
   }

   unican_writew((U16)cmCL2_DOWNLOAD_RTR_LIST + CL2_COMMAND_VALID, card->commandRegister);
   return CL2_OK;
   } /* cl2_download_rtrlist */


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
   )
   {
   U16 *ptrU16 = (U16*)card->dataPtr;
   U32 COBID;
   U16 cobidH, cobidL;

   if ( unican_readw(card->commandRegister) & CL2_COMMAND_VALID )
      return CL2_COMMAND_BUSY;
   if ( RTRnumber > 9 ) return CL2_BAD_PARAM;

   card->rtrSub[RTRnumber] = canMessage->COB_ID;
   *ptrU16 = RTRnumber;
   ptrU16++;
   COBID = canMessage->COB_ID;

   if ( COBID & CL2_TMP_EXHIGH ) /* standard or extended format? */
   { /* 2.0B frame */
     COBID <<= 3;
     cobidL = (U16)(COBID & 0x0000FFFFL);
     cobidH = (U16)((COBID & 0xFFFF0000L)>>16);
     cobidH = ((cobidH >> 8) & 0x00FF) | (((cobidH << 8) & 0xFF00));
     *ptrU16++ = cobidH;
     cobidL = ((cobidL >> 8) & 0x00FF) | (((cobidL << 8) & 0xFF00));
     *ptrU16++ = cobidL;
     *ptrU16 = (U16)CL2_EXT_FRAME;
   }
   else
   { /* 2.0A frame */
     COBID <<= 5;
     cobidH = (U16)(COBID & 0x0000FFFFL);
     cobidH = ((cobidH >> 8) & 0x00FF) | (((cobidH << 8) & 0xFF00));
     *ptrU16++ = cobidH;
     *ptrU16++ = 0;
     *ptrU16 = 0;
   }

   unican_writew((U16)cmCL2_SUBSCRIBE_RTR + CL2_COMMAND_VALID, card->commandRegister);
   return CL2_OK;
   } /* cl2_subscribe_rtr */


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
   sCAN_CARD *card,           /* Pointer to card structure */
   sCAN_MESSAGE *canMessage   /* RTR frame */
   )
   {
   U16 i;

   if ( unican_readw(card->commandRegister) & CL2_COMMAND_VALID )
      return CL2_COMMAND_BUSY;

   for ( i = 0; i < 10; i++ )
   {
     if ( card->rtrSub[i] == canMessage->COB_ID )
     {
       card->rtrSub[i] = 0xFFFFFFFFL;
       break;
     }
   }

   if ( i >= 10 ) return CL2_BAD_PARAM;

   unican_writew(i, card->dataPtr);
   unican_writew((U16)cmCL2_DESUBSCRIBE_RTR + CL2_COMMAND_VALID, card->commandRegister);
   return CL2_OK;
   } /* cl2_desubscribe_rtr */



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
   sCAN_CARD *card,           /* Pointer to card structure */
   U32 COBID                  /* COB-ID */
   )
   {
   U16 cobidL, cobidH;
   U16 *ptr = (U16 *)card->dataPtr;

   if ( unican_readw(card->commandRegister) & CL2_COMMAND_VALID )
      return CL2_COMMAND_BUSY;

   if ( COBID & CL2_TMP_EXHIGH ) /* standard or extended format? */
   { /* 2.0B frame */
     COBID <<= 3;
     cobidL = (U16)(COBID & 0x0000FFFFL);
     cobidH = (U16)((COBID & 0xFFFF0000L)>>16);
     cobidH = ((cobidH >> 8) & 0x00FF) | (((cobidH << 8) & 0xFF00));
     *ptr++ = cobidH;
     cobidL = ((cobidL >> 8) & 0x00FF) | (((cobidL << 8) & 0xFF00));
     *ptr++ = cobidL;
     *ptr = CL2_TMP_EX2;
   }
   else
   { /* 2.0A frame */
     COBID <<= 5;
     cobidH = (U16)(COBID & 0x0000FFFFL);
     cobidH = ((cobidH >> 8) & 0x00FF) | (((cobidH << 8) & 0xFF00));
     *ptr++ = cobidH;
     *ptr++ = 0;
     *ptr = 0;
   }

   unican_writew(((U16)cmCL2_SET_COBID + CL2_COMMAND_VALID), card->commandRegister);
   return CL2_OK;
   } /* cl2_set_sync_cobid */


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
   )
   {
   if ( unican_readw(card->commandRegister) & CL2_COMMAND_VALID )
      return CL2_COMMAND_BUSY;
   unican_writew((U16)period, card->dataPtr);
   unican_writew((U16)cmCL2_SET_SYNC_PERIOD + CL2_COMMAND_VALID, card->commandRegister);
   return CL2_OK;
   } /* cl2_set_sync_period */


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
   )
   {
   if ( unican_readw(card->commandRegister) & CL2_COMMAND_VALID )
      return CL2_COMMAND_BUSY;
   unican_writew((U16)window, card->dataPtr);
   unican_writew((U16)cmCL2_SET_SYNC_WINDOW + CL2_COMMAND_VALID, card->commandRegister);
   return CL2_OK;
   } /* cl2_set_sync_window */


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
   )
   {
   if ( unican_readw(card->commandRegister) & CL2_COMMAND_VALID )
      return CL2_COMMAND_BUSY;
   unican_writew(bitrate, card->dataPtr);
   unican_writew((U16)cmCL2_SET_BITRATE + CL2_COMMAND_VALID, card->commandRegister);
   return CL2_OK;
   } /* cl2_set_bitrate */


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
   sCAN_CARD *card
   )
   {
   if ( unican_readw(card->commandRegister) & CL2_COMMAND_VALID )
      return CL2_COMMAND_BUSY;
   unican_writew((U16)cmCL2_BUS_RESET + CL2_COMMAND_VALID, card->commandRegister);
   return CL2_OK;
   } /* cl2_bus_reset */


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
   sCAN_CARD *card,           /* pointer to card */
   sCAN_MESSAGE *message      /* massage to be sent */
   )
   {
   U32 cobid;
   U16 cobidL,cobidH;
   U16 *ptrU16 = (U16*)card->syncTxBufPtr;
   U16 tmpU16;
   int i;
   int timeStamp = 0;

   if ( card->syncTxBufSize==0 ) return CL2_HW_FAILURE;
   if ( message->dataLength > 8 ) return CL2_BAD_PARAM;
   if ( *ptrU16 & CL2_FRAME_VALID ) return CL2_HW_QUEUE_FULL;

   cobid = message->COB_ID;
   if ( (message->dataType & CL2_EXT_FRAME) || (cobid & CL2_TMP_EXHIGH) )
   {  /* 2.0B frame */
     cobid <<= 3;
     cobidL = (U16)(cobid & 0x0000FFFFL);
     cobidH = (U16)((cobid & 0xFFFF0000L)>>16);
   }
   else
   {  /* 2.0A frame */
     cobid <<= 5;
     cobidL = 0;
     cobidH = (U16)(cobid & 0x0000FFFFL);
   }
   ptrU16++;
   tmpU16 = (cobidH & 0x00FF) + (cobidL & 0xFF00);
   *ptrU16++ = tmpU16;

   tmpU16 = (((U16)message->dataLength) << 12) + (cobidL & 0x00FF);
   if ( !(message->dataType & CL2_REMOTE_FRAME) ) tmpU16 |= CL2_TMP_RF;
   if ( (message->dataType & CL2_EXT_FRAME) ||
        (message->COB_ID & CL2_TMP_EXHIGH) )
     tmpU16 |= CL2_TMP_EX;
   *ptrU16++ = tmpU16;

   for ( i = 0; i < message->dataLength; )
      {
      tmpU16 = (U16)message->data[i]; i++;
      if ( i == message->dataLength )
         {
         timeStamp = 1;
         tmpU16 |= ((message->timeStamp & 0x00FF)<<8);
         *ptrU16++ = tmpU16;
         }
      else
         {
         tmpU16 |= ((U16)message->data[i]<<8); i++;
         *ptrU16++ = tmpU16;
         }
      }
   if ( timeStamp )
      {
      tmpU16 = (message->timeStamp>>8) & 0x00FF;
      *ptrU16 = tmpU16;
      }
   else
      {
      *ptrU16 = message->timeStamp;
      }

   tmpU16 = (((U16)cobidH) & 0xFF00) | CL2_MESSAGE_VALID;
   unican_writew(tmpU16, card->syncTxBufPtr);

   if ( (card->syncTxBufBase + card->syncTxBufSize*16) <=
        (card->syncTxBufPtr += 16) )
     {
     card->syncTxBufPtr = card->syncTxBufBase;
     }
   return CL2_OK;
   } /* cl2_send_sync */


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
   sCAN_CARD *card,           /* pointer to card */
   sCAN_MESSAGE *message      /* message to be sent */
   )
   {
   U32 cobid;
   U16 cobidL,cobidH;
   U16 *ptrU16 = (U16*)card->asyncTxBufPtr;
   U16 tmpU16;
   int i;
   int timeStamp = 0;

   if ( card->asyncTxBufSize==0 ) return CL2_HW_FAILURE;
   if ( message->dataLength > 8 ) return CL2_BAD_PARAM;
   if ( *ptrU16 & CL2_FRAME_VALID ) return CL2_HW_QUEUE_FULL;

   cobid = message->COB_ID;
   if ( (message->dataType & CL2_EXT_FRAME) || (cobid & CL2_TMP_EXHIGH) )
   {  /* 2.0B frame */
     cobid <<= 3;
     cobidL = (U16)(cobid & 0x0000FFFFL);
     cobidH = (U16)((cobid & 0xFFFF0000L)>>16);
   }
   else
   {  /* 2.0A frame */
     cobid <<= 5;
     cobidL = 0;
     cobidH = (U16)(cobid & 0x0000FFFFL);
   }
   ptrU16++;
   tmpU16 = (cobidH & 0x00FF ) + (cobidL & 0xFF00);
   *ptrU16++ = tmpU16;

   tmpU16 = (((U16)message->dataLength) << 12) + (cobidL & 0x00FF);
   if ( !(message->dataType & CL2_REMOTE_FRAME) ) tmpU16 |= CL2_TMP_RF;
   if ( (message->dataType & CL2_EXT_FRAME) ||
        (message->COB_ID & CL2_TMP_EXHIGH ) )
      tmpU16 |= CL2_TMP_EX;
   *ptrU16++ = tmpU16;

   for ( i = 0; i < message->dataLength; )
      {
      tmpU16 = (U16)message->data[i]; i++;
      if ( i == message->dataLength )
         {
         timeStamp = 1;
         tmpU16 |= ((message->timeStamp & 0x00FF)<<8);
         *ptrU16++ = tmpU16;
         }
      else
         {
         tmpU16 |= ((U16)message->data[i]<<8); i++;
         *ptrU16++ = tmpU16;
         }
      }
   if ( timeStamp )
      {
      tmpU16 = (message->timeStamp>>8) & 0x00FF;
      *ptrU16 = tmpU16;
      }
   else
      {
      *ptrU16 = message->timeStamp;
      }

   tmpU16 = (((U16)cobidH) & 0xFF00) | CL2_MESSAGE_VALID;
   unican_writew(tmpU16, card->asyncTxBufPtr);

   if ( (card->asyncTxBufBase + card->asyncTxBufSize*16) <=
        (card->asyncTxBufPtr += 16) )
      {
      card->asyncTxBufPtr = card->asyncTxBufBase;
      }
   return CL2_OK;
   } /* cl2_send_async */


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
   sCAN_CARD *card,          /* pointer to card */
   U16 *status               /* card status word */
   )
   {
   U16 *ptr;

   ptr = (U16*)(card->baseAddressPtr + CL2_STATUS_REGISTER);
   *status = *ptr;
   if ( (*status & CL2_STATUS_VALID_FLAG) )
      {
      *ptr = *status & ~CL2_STATUS_VALID_FLAG;
      card->status = *status;
      return CL2_OK;
      }
   return CL2_NO_REQUEST;
   } /* cl2_get_status */


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
   sCAN_CARD *card,          /* pointer to card */
   U16 *error                /* card error word */
   )
   {
   U16 *ptr;

   ptr = (U16*)(card->baseAddressPtr + CL2_ERROR_REGISTER);
   *error = *ptr;
   card->error |= *error;
   *ptr = 0x0000;
   return CL2_OK;
   } /* cl2_get_error */


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
   )
   {
   U16 *ptrU16 = (U16*)card->rxBufPtr;
   U16 tmpU16;
   U16 i;

   tmpU16 = *ptrU16++;
   if ( !(tmpU16 & CL2_MESSAGE_VALID) ) return CL2_NO_REQUEST;
   canMessage->COB_ID = ((U32)(tmpU16 & 0xFF00 )) << 16;
   tmpU16 = *ptrU16++;
   canMessage->COB_ID |= ((U32)( tmpU16 & 0x00FF )) << 16;
   canMessage->COB_ID |= (U32)( tmpU16 & 0xFF00 );
   tmpU16 = *ptrU16++;
   canMessage->COB_ID |= (U32)( tmpU16 & 0x00FF );
   canMessage->dataType = (U8)(( tmpU16 & 0xFF00 ) >> 8);

   if ( canMessage->dataType & CL2_EXT_FRAME )
   {  /* 2.0B frame */
     canMessage->COB_ID >>= 3;
     /* canMessage->COB_ID |= CL2_TMP_EXHIGH; */
   }
   else
   {  /* 2.0A frame */
     canMessage->COB_ID >>= 21;
   }
   canMessage->dataLength = (U8)( (tmpU16 >> 12) & 0x000F );
   /* if ( !(tmpU16 & CL2_TMP_RF) ) canMessage->dataType |= CL2_REMOTE_FRAME; */
   for ( i = 0; i < canMessage->dataLength; )
      {
      tmpU16 = *ptrU16++;
      canMessage->data[i++] = (U8)( tmpU16 );
      canMessage->data[i++] = (U8)( tmpU16 >> 8 );
      }
   if ( canMessage->dataLength & 0x01 )
      {  /* odd */
      canMessage->timeStamp = ( (*ptrU16 & 0x00FF) | (tmpU16 & 0xFF00) );
      }
   else  /* even */
      {
      canMessage->timeStamp = *ptrU16 << 8 | *ptrU16 >> 8;
      }
   unican_writew(0x0000, card->rxBufPtr);

   /* increment rx-buffer pointer */
   if ( (card->rxBufBase + card->rxBufSize*16 ) <= (card->rxBufPtr += 16) )
      {
      card->rxBufPtr = card->rxBufBase;
      }

   return CL2_OK;
   } /* cl2_receive_data */


/* **************************************************************** *
 * END OF CL2.C                                                     *
 * **************************************************************** */


