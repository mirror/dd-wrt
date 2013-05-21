#ifndef _DRV_DSL_CPE_FIFO_H
#define _DRV_DSL_CPE_FIFO_H
/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/******************************************************************************
   Module      : sys_drv_fifo.h
   Description : Fifo definitions and declarations.
 *****************************************************************************/

#ifndef SWIG
/* ============================= */
/* Includes                      */
/* ============================= */

/* ============================= */
/* Local Macros  Definitions    */
/* ============================= */

/**
   FIFO data structure
*/
typedef struct
{
   /** start pointer of FIFO buffer */
   DSL_uint8_t* pStart;
   /** end pointer of FIFO buffer */
   DSL_uint8_t* pEnd;
   /** read pointer of FIFO buffer */
   DSL_uint8_t* pRead;
   /** write pointer of FIFO buffer */
   DSL_uint8_t* pWrite;
   /** element size */
   DSL_uint32_t size;
   /** element count, changed on read and write: */
   DSL_vuint32_t count;
   /** maximum of FIFO elements (or maximum element size of VFIFO)*/
   DSL_uint32_t nMaxSize;
} DSL_FIFO;

typedef DSL_FIFO DSL_VFIFO;

/* ============================= */
/* Global function declaration   */
/* ============================= */

extern DSL_int8_t  DSL_Fifo_Init (DSL_FIFO* pFifo, DSL_void_t* pStart,
                              DSL_void_t* pEnd, DSL_uint32_t size);
extern DSL_void_t  DSL_Fifo_Clear (DSL_FIFO *pFifo);
extern DSL_void_t* DSL_Fifo_readElement (DSL_FIFO *pFifo);
extern DSL_void_t* DSL_Fifo_writeElement (DSL_FIFO *pFifo);
extern DSL_void_t  DSL_Fifo_returnElement (DSL_FIFO *pFifo);
extern DSL_int8_t  DSL_Fifo_isEmpty (DSL_FIFO *pFifo);
extern DSL_int8_t  DSL_Fifo_isFull (DSL_FIFO *pFifo);
extern DSL_uint32_t DSL_Fifo_getCount(DSL_FIFO *pFifo);


extern DSL_int8_t  DSL_Var_Fifo_Init (DSL_VFIFO* pFifo, DSL_void_t* pStart,
                              DSL_void_t* pEnd, DSL_uint32_t size);
extern DSL_void_t  DSL_Var_Fifo_Clear (DSL_VFIFO *pFifo);
extern DSL_void_t* DSL_Var_Fifo_readElement (DSL_VFIFO *pFifo, DSL_uint32_t *elSize);
extern DSL_void_t* DSL_Var_Fifo_writeElement (DSL_VFIFO *pFifo, DSL_uint32_t elSize);
extern DSL_int8_t  DSL_Var_Fifo_isEmpty (DSL_VFIFO *pFifo);
extern DSL_int8_t  DSL_Var_Fifo_isFull (DSL_VFIFO *pFifo);
extern DSL_uint32_t DSL_Var_Fifo_getCount(DSL_VFIFO *pFifo);

#endif /* #ifndef SWIG*/

#endif
