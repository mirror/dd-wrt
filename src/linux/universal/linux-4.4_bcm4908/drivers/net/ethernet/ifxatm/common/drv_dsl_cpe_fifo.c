/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/******************************************************************************
   Module      : sys_drv_fifo.c
   Description : Fifo implementation
   Remarks     :
      Initialize with Fifo_Init with previosly allocated memory.
      The functions Fifo_writeElement and Fifo_readElement return a pointer
      to the next element to be written or read respectively.
      If empty Fifo_readElement returns IFX_NULL. If full Fifo_writeElement
      returns IFX_NULL.
 *****************************************************************************/


/* ============================= */
/* Includes                      */
/* ============================= */


#define DSL_INTERN

#include "drv_dsl_cpe_api.h"
#include "drv_dsl_cpe_fifo.h"

/* ============================= */
/* Local Macros  Definitions    */
/* ============================= */

/* increment FIFO index */
#define DSL_FIFO_INCREMENT_INDEX(p)       \
{                                \
   if (p == pFifo->pEnd)         \
      p = pFifo->pStart;         \
   else                          \
      p = (DSL_void_t*)(((DSL_uint_t)p) + pFifo->size); \
}

/* decrement FIFO index */
#define DSL_FIFO_DECREMENT_INDEX(p)       \
{                                \
   if (p == pFifo->pStart)       \
      p = pFifo->pEnd;           \
   else                          \
      p = (DSL_void_t*)(((DSL_uint_t)p) - pFifo->size); \
}

/* ============================= */
/* Global variable definition    */
/* ============================= */

/* ============================= */
/* Local function declaration    */
/* ============================= */

/* ============================= */
/* Local variable definition     */
/* ============================= */

/* ============================= */
/* Local function definition     */
/* ============================= */

/* ============================= */
/* Global function definition    */
/* ============================= */


/*
   Initializes the fifo structure
   \param *pFifo - Pointer to the Fifo structure
   \param *pStart - Pointer to the fifo start memory
   \param *pEnd - Pointer to the fifo end memory
   \param elSize - size of each element in bytes
   \return
   Always zero, otherwise error
*/
DSL_int8_t DSL_Fifo_Init (DSL_FIFO* pFifo, DSL_void_t* pStart, DSL_void_t* pEnd, DSL_uint32_t elSize)
{
   pFifo->pEnd   = pEnd;
   pFifo->pStart = pStart;
   pFifo->pRead  = pStart;
   pFifo->pWrite = pStart;
   pFifo->size   = elSize;
   pFifo->count  = 0;

   if (((DSL_uint32_t)pFifo->pEnd - (DSL_uint32_t)pFifo->pStart) % elSize != 0)
   {
      /* element size must be a multiple of fifo memory */
      return -1;
   }
   pFifo->nMaxSize = ((DSL_uint32_t)pFifo->pEnd - (DSL_uint32_t)pFifo->pStart) / elSize + 1;

   return 0;
}

/*
   Clear the fifo
   \param *pFifo - Pointer to the Fifo structure
*/
DSL_void_t DSL_Fifo_Clear (DSL_FIFO *pFifo)
{
   pFifo->pRead  = pFifo->pStart;
   pFifo->pWrite = pFifo->pStart;
   pFifo->count = 0;
}

/*
   Get the next element to read from
   \param *pFifo - Pointer to the Fifo structure
   \return
   Returns the element to read from, or IFX_NULL if no element available or an error occured
   \remarks
   Error occurs if fifo is empty
*/
DSL_void_t* DSL_Fifo_readElement (DSL_FIFO *pFifo)
{
   DSL_void_t *ret = DSL_NULL;

#ifdef CGC_CRC_DEBUG
/* temporary code, should be removed in final code */
{
   int cnt;
   int bo;

   if ((IFX_uint32_t)pFifo->pWrite >= (IFX_uint32_t)pFifo->pRead)
   {
      cnt = (((IFX_uint32_t)pFifo->pWrite - (IFX_uint32_t)pFifo->pRead) / pFifo->size);
   }
   else
   {
      cnt = (((((IFX_uint32_t)pFifo->pEnd - (IFX_uint32_t)pFifo->pRead) + ((IFX_uint32_t)pFifo->pWrite - (IFX_uint32_t)pFifo->pStart)) / pFifo->size) + 1);
      bo=1;
   }

   if (cnt != pFifo->count)
   {
      printf ("%s: read fifo inconsistent"DSL_DRV_CRLF, taskName(taskIdSelf()));
      printf (" pStart: 0x%08x"DSL_DRV_CRLF, (IFX_uint32_t)pFifo->pStart);
      printf (" pEnd: 0x%08x"DSL_DRV_CRLF, (IFX_uint32_t)pFifo->pEnd);
      printf (" pRead: 0x%08x"DSL_DRV_CRLF, (IFX_uint32_t)pFifo->pRead);
      printf (" pWrite: 0x%08x"DSL_DRV_CRLF, (IFX_uint32_t)pFifo->pWrite);
      printf (" Count: %d"DSL_DRV_CRLF, pFifo->count);
   }
}
/* temporary code, should be removed in final code */
#endif /* CGC_CRC_DEBUG */

   if (pFifo->count == 0)
   {
      return DSL_NULL;
   }
   ret = pFifo->pRead;

   DSL_FIFO_INCREMENT_INDEX(pFifo->pRead);
   pFifo->count--;

   return ret;
}

/*
   Delivers empty status
   \param *pFifo - Pointer to the Fifo structure
   \return
   Returns TRUE if empty (no data available)
   \remarks
   No change on fifo!
*/
DSL_int8_t DSL_Fifo_isEmpty (DSL_FIFO *pFifo)
{
   return (pFifo->count == 0);
}

/*
   Get the next element to write to
   \param *pFifo - Pointer to the Fifo structure
   \return
   Returns the element to write to, or IFX_NULL in case of error
   \remarks
   Error occurs if the write pointer reaches the read pointer, meaning
   the fifo if full and would otherwise overwrite the next element.
*/
DSL_void_t* DSL_Fifo_writeElement (DSL_FIFO *pFifo)
{
   DSL_void_t* ret = DSL_NULL;

   if (DSL_Fifo_isFull(pFifo))
      return DSL_NULL;
   else
   {
      /* get the next entry of the Fifo */
      ret = pFifo->pWrite;
      DSL_FIFO_INCREMENT_INDEX(pFifo->pWrite);
      pFifo->count++;

      return ret;
   }
}

/*
   Delivers full status
   \param *pFifo - Pointer to the Fifo structure
   \return
   TRUE if full (overflow on next write)
   \remarks
   No change on fifo!
*/
DSL_int8_t DSL_Fifo_isFull (DSL_FIFO *pFifo)
{
   return (pFifo->count >= pFifo->nMaxSize);
}

/*
   Returns the last element taken back to the fifo
   \param *pFifo - Pointer to the Fifo structure
   \remarks
   Makes an undo to a previosly Fifo_writeElement
*/
DSL_void_t DSL_Fifo_returnElement (DSL_FIFO *pFifo)
{
   DSL_FIFO_DECREMENT_INDEX(pFifo->pWrite);
   pFifo->count--;
}

/*
   Get the number of stored elements
   \param *pFifo - Pointer to the Fifo structure
   \return
   Number of containing elements
*/
DSL_uint32_t DSL_Fifo_getCount(DSL_FIFO *pFifo)
{
   return pFifo->count;
}

#ifdef INCLUDE_FIFO_TEST
/*
   test routine
*/
DSL_void_t DSL_FifoTest(DSL_void_t)
{
   DSL_uint16_t buf [3];
   DSL_uint16_t tst = 1;
   DSL_uint16_t *entry = DSL_NULL;
   DSL_FIFO fifo;

   DSL_Fifo_Init (&fifo, &buf[0], &buf[2], sizeof (DSL_uint16_t));

   entry = DSL_Fifo_writeElement (&fifo);
   if (entry != DSL_NULL)
      *entry = 1;
   entry = DSL_Fifo_writeElement (&fifo);
   if (entry != DSL_NULL)
   {
      *entry = 2;
      DSL_Fifo_returnElement (&fifo);
   }
   entry = DSL_Fifo_readElement (&fifo);
   if (entry != DSL_NULL)
      tst = *entry;
   entry = DSL_Fifo_readElement (&fifo);
   if (entry != DSL_NULL)
      tst = *entry;
}
#endif

