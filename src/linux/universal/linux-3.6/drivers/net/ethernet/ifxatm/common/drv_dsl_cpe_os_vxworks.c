/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/
#ifdef VXWORKS

#define DSL_INTERN

#include "drv_dsl_cpe_api.h"
#include "drv_dsl_cpe_api_ioctl.h"

#include "drv_dsl_cpe_intern.h"
#include "drv_dsl_cpe_intern_mib.h"

#include "drv_dsl_cpe_debug.h"

#undef DSL_DBG_BLOCK
#define DSL_DBG_BLOCK DSL_DBG_OS

#define DSL_DRV_STATIC

#ifdef __cplusplus
   extern "C" {
#endif

typedef struct DSL_DRV_DevHeader_s
{
   /** driver specific parameter */
   DEV_HDR           DevHdr;
   /** driver device number */
   IFX_int_t         deviceNum;
   /** context */
   DSL_OpenContext_t *pOpenCtx;
} DSL_DRV_DevHeader_t;

DSL_DRV_STATIC DSL_DRV_DevHeader_t DSL_DevHeader[DSL_DRV_MAX_DEVICE_NUMBER];

DSL_DRV_STATIC IFX_int_t DSL_DRV_DrvNum = -1;

DSL_DRV_STATIC const char* dsl_cpe_api_version = "@(#)DSL CPE API V" DSL_CPE_API_PACKAGE_VERSION;

DSL_DRV_STATIC DSL_int_t DSL_DRV_Write(DSL_DRV_DevHeader_t *pDrvHdr, IFX_uint8_t *pSrc, IFX_int_t nLength);

DSL_DRV_STATIC DSL_int_t DSL_DRV_Ioctls(DSL_DRV_DevHeader_t *pDrvHdr, DSL_uint_t nCommand, DSL_uint32_t nArg);

DSL_DRV_STATIC int DSL_DRV_Open(DSL_DRV_DevHeader_t *pDrvHdr, IFX_uint8_t *pAnnex, IFX_int_t flags);

DSL_DRV_STATIC int DSL_DRV_Close(DSL_DRV_DevHeader_t *pDrvHdr);

DSL_DRV_STATIC DSL_uint_t DSL_DRV_Poll(DSL_DRV_DevHeader_t *pDrvHdr);

/* global parameter debug_level: LOW (1), NORMAL (2), HIGH (3), OFF (4) */
#ifndef DSL_DEBUG_DISABLE
/* Activate high level by default */
DSL_DRV_STATIC DSL_uint8_t debug_level = 3;
#endif

/* The name of this device */
#define DRV_DSL_CPE_API_DEV_NAME "dsl_cpe_api"

DSL_DRV_STATIC int DSL_DRV_Open(DSL_DRV_DevHeader_t *pDrvHdr, IFX_uint8_t *pAnnex, IFX_int_t flags)
{
   DSL_OpenContext_t *pOpenCtx=DSL_NULL;

   DSL_DEBUG(DSL_DBG_MSG, (DSL_NULL, "Device will be opened..."DSL_DRV_CRLF));

   if ( DSL_DRV_HandleInit(pDrvHdr->deviceNum, &pOpenCtx) != DSL_SUCCESS )
   {
      DSL_DEBUG(DSL_DBG_ERR, (DSL_NULL, "Cannot initialize device..."DSL_DRV_CRLF));
      return ERROR;
   }
   
   pDrvHdr->pOpenCtx = pOpenCtx;

   DSL_DEBUG(DSL_DBG_MSG, (DSL_NULL, "Open successfull..."DSL_DRV_CRLF));

   return (int)pDrvHdr;
}

DSL_DRV_STATIC int DSL_DRV_Close(DSL_DRV_DevHeader_t *pDrvHdr)
{
   DSL_DEBUG(DSL_DBG_MSG, (DSL_NULL, "Device will be closed..."DSL_DRV_CRLF));

   if (pDrvHdr->deviceNum >= DSL_DRV_MAX_DEVICE_NUMBER)
   {
      return -1;
   }

   DSL_DRV_HandleDelete(pDrvHdr->pOpenCtx);

   DSL_DEBUG(DSL_DBG_MSG, (DSL_NULL, "Close successfull..."DSL_DRV_CRLF));
   return 0;
}

DSL_DRV_STATIC DSL_int_t DSL_DRV_Write(DSL_DRV_DevHeader_t *pDrvHdr, IFX_uint8_t *pBuf, IFX_int_t nSize)
{
   DSL_Error_t nErrCode = DSL_ERROR;
   DSL_uint32_t nOffset=0;
   DSL_OpenContext_t *pOpenContext = pDrvHdr->pOpenCtx;
   
   DSL_DEBUG(DSL_DBG_MSG, (DSL_NULL, "Device will be written..."DSL_DRV_CRLF));

   if (pOpenContext == DSL_NULL)
   {
      DSL_DEBUG(DSL_DBG_ERR, (DSL_NULL, "pOpenContext is NULL"DSL_DRV_CRLF));
      return -1;
   }

   if (pOpenContext->pDevCtx == DSL_NULL)
   {
      DSL_DEBUG(DSL_DBG_ERR, (DSL_NULL, "Device context pointer is NULL"DSL_DRV_CRLF));
      return -1;
   }

   pOpenContext->pDevCtx->bFirmwareReady = DSL_FALSE;

   nErrCode = DSL_DRV_DEV_FwDownload(pOpenContext->pDevCtx->pContext,
      pBuf, nSize, DSL_NULL, 0, 0, 0, DSL_TRUE);

   if (nErrCode != DSL_SUCCESS)
      return -1;

   pOpenContext->pDevCtx->bFirmwareReady = DSL_TRUE;
   
   DSL_DEBUG(DSL_DBG_MSG, (DSL_NULL, "Firmware was downloaded successfully "
                                    "%d bytes has written..."DSL_DRV_CRLF, nOffset));
   return (DSL_int_t)nOffset;
}

/*
   IO controls for user space accessing

   \param   ino      Pointer to the stucture of inode.
   \param   fil      Pointer to the stucture of file.
   \param   command     The ioctl command.
   \param   lon      The address of data.
   \return  Success or failure.
   \ingroup Internal
*/
DSL_int_t DSL_DRV_Ioctls(DSL_DRV_DevHeader_t *pDrvHdr, DSL_uint_t nCommand, DSL_uint32_t nArg)
{
   DSL_int_t nErr=0;
   DSL_boolean_t bIsInKernel;
   DSL_Error_t nRetCode = DSL_SUCCESS;
   DSL_Context_t *pContext;
   DSL_devCtx_t *pDevCtx;
   DSL_OpenContext_t *pOpenCtx = pDrvHdr->pOpenCtx;

   DSL_DEBUG(DSL_DBG_MSG, (DSL_NULL, "Device will be controled..."DSL_DRV_CRLF));

   DSL_DEBUG(DSL_DBG_MSG, (DSL_NULL, "IN - DSL_DRV_Ioctls: The ioctl "
      "command(0x%X) is called" DSL_DRV_CRLF, nCommand));

   if (pOpenCtx == DSL_NULL)
   {
      /* This should never happen */
      DSL_DEBUG(DSL_DBG_ERR, (DSL_NULL, "!!! Ioctl call for file which "
         "was not opened" DSL_DRV_CRLF));

      return -1;
   }
   else
   {
      if ((pDevCtx = pOpenCtx->pDevCtx) == DSL_NULL)
      {
         /* This should never happen */
         DSL_DEBUG(DSL_DBG_ERR, (DSL_NULL, "DSL_DRV_Poll: !!! Ioctl call "
            "for file which was not opened correctly" DSL_DRV_CRLF));

         return -1;
      }
      else
      {
         if ((pContext = pDevCtx->pContext) == DSL_NULL)
         {
            /* This should never happen */
            DSL_DEBUG(DSL_DBG_ERR, (DSL_NULL, "!!! Ioctl call to device "
               "which was not ready" DSL_DRV_CRLF));

            return -1;
         }
      }
   }

   bIsInKernel = DSL_TRUE;
   
   switch(nCommand)
   {
      case FIOSELECT:
      /* add to wakeup list */      
      DSL_DRV_ADD_TASK_WAKELIST(
         (DSL_DRV_SelectOsArg_t*) nArg,
         (DSL_DRV_WaitQueue_t*) &(pOpenCtx->eventWaitQueue),
         (DSL_DRV_SelectTable_t*)DSL_NULL);

      if ( DSL_Fifo_isEmpty( pOpenCtx->eventFifo ) == DSL_FALSE )
      {
         selWakeup((SEL_WAKEUP_NODE*) nArg);
      }
      return 0;

      case FIOUNSELECT:
      selNodeDelete(&pOpenCtx->eventWaitQueue, (SEL_WAKEUP_NODE*) nArg);
      return 0;
   }

   if ( (_IOC_TYPE(nCommand) == DSL_IOC_MAGIC_CPE_API) ||
        (_IOC_TYPE(nCommand) == DSL_IOC_MAGIC_CPE_API_G997) ||
        (_IOC_TYPE(nCommand) == DSL_IOC_MAGIC_CPE_API_PM) ||
        (_IOC_TYPE(nCommand) == DSL_IOC_MAGIC_CPE_API_SAR) ||
        (_IOC_TYPE(nCommand) == DSL_IOC_MAGIC_CPE_API_DEP) )
   {
      nRetCode = DSL_DRV_IoctlHandle(pOpenCtx, pContext, bIsInKernel, nCommand, nArg);

      if (nRetCode < DSL_SUCCESS)
      {
         nErr = DSL_DRV_ErrorToOS(nRetCode);
      }
   }
#if defined(INCLUDE_DSL_ADSL_MIB)
   else if (_IOC_TYPE(nCommand) == DSL_IOC_MAGIC_MIB)
   {
      nRetCode = DSL_DRV_MIB_IoctlHandle(pContext, bIsInKernel, nCommand, nArg);
      nErr = (DSL_int_t)nRetCode;
   }
#endif
   else
   {
      DSL_DEBUG(DSL_DBG_ERR, (DSL_NULL, "The ioctl command(0x%X) is not "
         "supported!" DSL_DRV_CRLF, nCommand));
      nErr = -1;
      return nErr;
   }

   DSL_DEBUG(DSL_DBG_MSG, (DSL_NULL, "OUT - DSL_DRV_Ioctls(), retCode=%d"
      DSL_DRV_CRLF, nErr));

   return nErr;
}

DSL_DRV_STATIC DSL_uint_t DSL_DRV_Poll(DSL_DRV_DevHeader_t *pDrvHdr)
{
   DSL_int_t nRet = 0;
   DSL_OpenContext_t *pOpenCtx = pDrvHdr->pOpenCtx;

   DSL_DEBUG(DSL_DBG_MSG, (DSL_NULL, "IN - DSL_DRV_Poll" DSL_DRV_CRLF));

   if (pOpenCtx == DSL_NULL)
   {
      /* This should never happen */
      DSL_DEBUG(DSL_DBG_ERR, (DSL_NULL, "!!! Ioctl call for file which "
         "was not opened" DSL_DRV_CRLF));

      return -EFAULT;
   }
#if 0   
   poll_wait(pFile, &pOpenCtx->eventWaitQueue, wait);

   if(DSL_DRV_MUTEX_LOCK(pOpenCtx->eventMutex))
   {
      DSL_DEBUG( DSL_DBG_ERR, (DSL_NULL, "Couldn't lock event mutex"DSL_DRV_CRLF));
      return DSL_ERROR;
   }

   if (pOpenCtx->eventFifo == DSL_NULL
      || pOpenCtx->eventFifoBuf == DSL_NULL
      || pOpenCtx->bEventActivation == DSL_FALSE)
   {
      DSL_DEBUG(DSL_DBG_WRN,
         (DSL_NULL, "!!! Ioctl call for file which was not configured"
                    " for event handling!!!" DSL_DRV_CRLF));
   }
   else
   {
      if ( DSL_Fifo_isEmpty( pOpenCtx->eventFifo ) == DSL_FALSE )
      {
         nRet |= POLLIN | POLLRDNORM; /* an event available */
      }
   }

   DSL_DRV_MUTEX_UNLOCK(pOpenCtx->eventMutex);

   DSL_DEBUG(DSL_DBG_MSG, (DSL_NULL, "OUT - DSL_DRV_Poll" DSL_DRV_CRLF));
#endif

   return nRet;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_os.h'
*/
int DSL_DRV_debug_printf(DSL_Context_t const *pContext, DSL_char_t const *fmt, ...)
{
   va_list ap;   /* points to each unnamed arg in turn */
   DSL_int_t nLength = 0, nRet = 0;
   DSL_boolean_t bPrint = DSL_FALSE;
   DSL_char_t msg[DSL_DBG_MAX_DEBUG_PRINT_CHAR + 1] = "\0";

   va_start(ap, fmt);   /* set ap pointer to 1st unnamed arg */

   if (bPrint == DSL_FALSE)
   {
      nRet = vsprintf(msg, fmt, ap);

      if (nRet < 0)
      {
         nLength = DSL_DBG_MAX_DEBUG_PRINT_CHAR;
         printf("DSL CPE API: WARNING - printout truncated in 'DSL_DRV_debug_printf'!" DSL_DRV_CRLF );
      }
      else
      {
         nLength = nRet;
      }

      nRet = printf(msg);
   }

   va_end(ap);

   return nRet;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_os.h'
*/
int DSL_DRV_ErrorToOS(DSL_Error_t nError)
{
   switch (nError)
   {
      case DSL_ERR_POINTER:
      case DSL_ERR_INVALID_PARAMETER: return -1;

      case DSL_ERR_NOT_IMPLEMENTED:
      case DSL_ERR_NOT_SUPPORTED:
      case DSL_ERR_NOT_SUPPORTED_BY_DEVICE: return -1;
      case DSL_ERR_NOT_SUPPORTED_BY_FIRMWARE: return -1;

      case DSL_ERR_MEMORY: return -1;

      case DSL_ERR_FILE_CLOSE:
      case DSL_ERR_FILE_OPEN:
      case DSL_ERR_FILE_READ:
      case DSL_ERR_FILE_WRITE: return -1;

      case DSL_WRN_LAST:
      case DSL_SUCCESS: return 0;

      case DSL_ERR_INTERNAL:
      case DSL_ERR_TIMEOUT:
      case DSL_ERROR:
      default:
         return -1;
   }

   return -1;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_os.h'
*/
DSL_void_t* DSL_DRV_VMalloc(
   DSL_DRV_size_t    nSize)
{
   return IFXOS_MemAlloc(nSize);
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_os.h'
*/
DSL_void_t DSL_DRV_VFree(
   DSL_void_t*    pPtr)
{
   IFXOS_MemFree(pPtr);
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_os.h'
*/
DSL_int_t DSL_DRV_snprintf(
   DSL_char_t        *pStr,
   DSL_DRV_size_t        nStrSz,
   const DSL_char_t  *pFormat,  ...)
{
   va_list arg;
   int rv;

   va_start(arg, pFormat);
   rv = vsprintf(pStr, pFormat, arg);
   va_end(arg);
   return rv;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_os.h'
*/
DSL_void_t* DSL_IoctlMemCpyFrom(
   DSL_boolean_t bIsInKernel,
   DSL_void_t    *pDest,
   DSL_void_t    *pSrc,
   DSL_DRV_size_t    nSize)
{
   DSL_void_t *pRet;

   pRet = memcpy(pDest, pSrc, nSize);

   return pRet;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_os.h'
*/
DSL_void_t* DSL_IoctlMemCpyTo(
   DSL_boolean_t bIsInKernel,
   DSL_void_t    *pDest,
   DSL_void_t    *pSrc,
   DSL_DRV_size_t    nSize)
{
   DSL_void_t *pRet;

   pRet = memcpy(pDest, pSrc, nSize);

   return pRet;
}

DSL_uint32_t DSL_DRV_SysTimeGet(DSL_uint32_t nOffset)
{
   DSL_uint32_t nTime = 0;

   nTime = IFXOS_SysTimeGet();

   if ( (nOffset == 0) || (nOffset > nTime) )
   {
      return nTime;
   }

   return (nTime - nOffset);
}


#ifndef DSL_DEBUG_DISABLE
DSL_DRV_STATIC void DSL_DRV_DebugInit(void)
{
   switch (debug_level)
   {
      case 1:
         DSL_g_dbgLvl[DSL_DBG_CPE_API].nDbgLvl = DSL_DBG_MSG;
         DSL_g_dbgLvl[DSL_DBG_G997].nDbgLvl = DSL_DBG_MSG;
         DSL_g_dbgLvl[DSL_DBG_PM].nDbgLvl = DSL_DBG_MSG;
         DSL_g_dbgLvl[DSL_DBG_MIB].nDbgLvl = DSL_DBG_MSG;
         DSL_g_dbgLvl[DSL_DBG_CEOC].nDbgLvl = DSL_DBG_MSG;
         DSL_g_dbgLvl[DSL_DBG_LED].nDbgLvl = DSL_DBG_MSG;
         DSL_g_dbgLvl[DSL_DBG_SAR].nDbgLvl = DSL_DBG_MSG;
         DSL_g_dbgLvl[DSL_DBG_DEVICE].nDbgLvl = DSL_DBG_MSG;
         DSL_g_dbgLvl[DSL_DBG_AUTOBOOT_THREAD].nDbgLvl = DSL_DBG_MSG;
         DSL_g_dbgLvl[DSL_DBG_OS].nDbgLvl = DSL_DBG_MSG;
         DSL_g_dbgLvl[DSL_DBG_CALLBACK].nDbgLvl = DSL_DBG_MSG;
         DSL_g_dbgLvl[DSL_DBG_MESSAGE_DUMP].nDbgLvl = DSL_DBG_ERR;
         DSL_g_dbgLvl[DSL_DBG_LOW_LEVEL_DRIVER].nDbgLvl = DSL_DBG_MSG;
         break;
      case 2:
         DSL_g_dbgLvl[DSL_DBG_CPE_API].nDbgLvl = DSL_DBG_WRN;
         DSL_g_dbgLvl[DSL_DBG_G997].nDbgLvl = DSL_DBG_WRN;
         DSL_g_dbgLvl[DSL_DBG_PM].nDbgLvl = DSL_DBG_WRN;
         DSL_g_dbgLvl[DSL_DBG_MIB].nDbgLvl = DSL_DBG_WRN;
         DSL_g_dbgLvl[DSL_DBG_CEOC].nDbgLvl = DSL_DBG_WRN;
         DSL_g_dbgLvl[DSL_DBG_LED].nDbgLvl = DSL_DBG_WRN;
         DSL_g_dbgLvl[DSL_DBG_SAR].nDbgLvl = DSL_DBG_WRN;
         DSL_g_dbgLvl[DSL_DBG_DEVICE].nDbgLvl = DSL_DBG_WRN;
         DSL_g_dbgLvl[DSL_DBG_AUTOBOOT_THREAD].nDbgLvl = DSL_DBG_WRN;
         DSL_g_dbgLvl[DSL_DBG_OS].nDbgLvl = DSL_DBG_WRN;
         DSL_g_dbgLvl[DSL_DBG_CALLBACK].nDbgLvl = DSL_DBG_WRN;
         DSL_g_dbgLvl[DSL_DBG_MESSAGE_DUMP].nDbgLvl = DSL_DBG_NONE;
         DSL_g_dbgLvl[DSL_DBG_LOW_LEVEL_DRIVER].nDbgLvl = DSL_DBG_WRN;
         break;
      case 3:
         DSL_g_dbgLvl[DSL_DBG_CPE_API].nDbgLvl = DSL_DBG_ERR;
         DSL_g_dbgLvl[DSL_DBG_G997].nDbgLvl = DSL_DBG_ERR;
         DSL_g_dbgLvl[DSL_DBG_PM].nDbgLvl = DSL_DBG_ERR;
         DSL_g_dbgLvl[DSL_DBG_MIB].nDbgLvl = DSL_DBG_ERR;
         DSL_g_dbgLvl[DSL_DBG_CEOC].nDbgLvl = DSL_DBG_ERR;
         DSL_g_dbgLvl[DSL_DBG_LED].nDbgLvl = DSL_DBG_ERR;
         DSL_g_dbgLvl[DSL_DBG_SAR].nDbgLvl = DSL_DBG_ERR;
         DSL_g_dbgLvl[DSL_DBG_DEVICE].nDbgLvl = DSL_DBG_ERR;
         DSL_g_dbgLvl[DSL_DBG_AUTOBOOT_THREAD].nDbgLvl = DSL_DBG_ERR;
         DSL_g_dbgLvl[DSL_DBG_OS].nDbgLvl = DSL_DBG_ERR;
         DSL_g_dbgLvl[DSL_DBG_CALLBACK].nDbgLvl = DSL_DBG_ERR;
         DSL_g_dbgLvl[DSL_DBG_MESSAGE_DUMP].nDbgLvl = DSL_DBG_NONE;
         DSL_g_dbgLvl[DSL_DBG_LOW_LEVEL_DRIVER].nDbgLvl = DSL_DBG_ERR;
         break;
      case 4:
         DSL_g_dbgLvl[DSL_DBG_CPE_API].nDbgLvl = DSL_DBG_NONE;
         DSL_g_dbgLvl[DSL_DBG_G997].nDbgLvl = DSL_DBG_NONE;
         DSL_g_dbgLvl[DSL_DBG_PM].nDbgLvl = DSL_DBG_NONE;
         DSL_g_dbgLvl[DSL_DBG_MIB].nDbgLvl = DSL_DBG_NONE;
         DSL_g_dbgLvl[DSL_DBG_CEOC].nDbgLvl = DSL_DBG_NONE;
         DSL_g_dbgLvl[DSL_DBG_LED].nDbgLvl = DSL_DBG_NONE;
         DSL_g_dbgLvl[DSL_DBG_SAR].nDbgLvl = DSL_DBG_NONE;
         DSL_g_dbgLvl[DSL_DBG_DEVICE].nDbgLvl = DSL_DBG_NONE;
         DSL_g_dbgLvl[DSL_DBG_AUTOBOOT_THREAD].nDbgLvl = DSL_DBG_NONE;
         DSL_g_dbgLvl[DSL_DBG_OS].nDbgLvl = DSL_DBG_NONE;
         DSL_g_dbgLvl[DSL_DBG_CALLBACK].nDbgLvl = DSL_DBG_NONE;
         DSL_g_dbgLvl[DSL_DBG_MESSAGE_DUMP].nDbgLvl = DSL_DBG_NONE;
         DSL_g_dbgLvl[DSL_DBG_LOW_LEVEL_DRIVER].nDbgLvl = DSL_DBG_NONE;
         break;
      default:
         /* Nothing to do */
         break;
   }

   return;
}
#endif

DSL_DRV_STATIC int DSL_DRV_DevNodeInit(DSL_DRV_DevHeader_t *pDrvHdr, char *name, DSL_uint32_t drvNum)
{
   /* Add device to the device list */
   if (iosDevAdd(&pDrvHdr->DevHdr, name, drvNum) == ERROR)
   {
      DSL_DEBUG(DSL_DBG_ERR, (DSL_NULL, DSL_DRV_CRLF DSL_DRV_CRLF"unable to register "
                        "major for %s!!!", DRV_DSL_CPE_API_DEV_NAME));
      return -1;
   }

   return 0;
}

/* Entry point of driver */
int DSL_DRV_DeviceCreate(void)
{
   DSL_int_t i;
   DSL_char_t name[32];

   printf(DSL_DRV_CRLF DSL_DRV_CRLF "Infineon CPE API Driver version: %s" DSL_DRV_CRLF,
      &(dsl_cpe_api_version[4]));

   DSL_DRV_MemSet( &ifxDevices, 0, sizeof(DSL_devCtx_t) * DSL_DRV_MAX_DEVICE_NUMBER );

   /* Apply initial debug levels. The lines below should be updated in case of
      new modules insert */
#ifndef DSL_DEBUG_DISABLE
   DSL_DRV_DebugInit();
#endif

   DSL_DRV_DrvNum = iosDrvInstall(IFX_NULL, IFX_NULL,
                                (FUNCPTR)DSL_DRV_Open,
                                (FUNCPTR)DSL_DRV_Close,
                                IFX_NULL,
                                (FUNCPTR)DSL_DRV_Write,
                                (FUNCPTR)DSL_DRV_Ioctls);   

#if defined(INCLUDE_DSL_CPE_API_VINAX) && defined(INCLUDE_DSL_BONDING)
   if ( DSL_DRV_Phy2VirtMap(
           DSL_FPGA_BND_BASE_ADDR, DSL_FPGA_BND_REGS_SZ_BYTE,
           "BND_FPGA", (DSL_uint8_t**)&g_BndFpgaBase) < 0)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (DSL_NULL, "Bonding FPGA memory mapping failed!"DSL_DRV_CRLF));
   }
#endif /* defined(INCLUDE_DSL_CPE_API_VINAX) && defined(INCLUDE_DSL_BONDING)*/

   /* Get handles for lower level driver */
   for (i = 0; i < DSL_DRV_MAX_DEVICE_NUMBER; i++)
   {
      ifxDevices[i].lowHandle = DSL_DRV_DEV_DriverHandleGet(0,i);
      if (ifxDevices[i].lowHandle == DSL_NULL)
      {
         printf("Get BSP Driver Handle Fail!"DSL_DRV_CRLF);
      }
#ifdef INCLUDE_DSL_CPE_API_VINAX
      ifxDevices[i].nfc_lowHandle = DSL_DRV_DEV_DriverHandleGet(0,i);
      if (ifxDevices[i].lowHandle == DSL_NULL)
      {
         printf("Get BSP Driver NFC Handle Fail!"DSL_DRV_CRLF);
      }
#endif /* #ifdef INCLUDE_DSL_CPE_API_VINAX*/

      ifxDevices[i].nUsageCount = 0;
      ifxDevices[i].bFirstPowerOn = DSL_TRUE;
      DSL_DEBUG(DSL_DBG_MSG, (DSL_NULL, "ifxDevices[%d].lowHandle=0x%0X"DSL_DRV_CRLF,
         i, ifxDevices[i].lowHandle));

      /* Set device number*/
      DSL_DevHeader[i].deviceNum = i;
   
      sprintf(name, "/dev/%s/%d", DRV_DSL_CPE_API_DEV_NAME, i);
      DSL_DRV_DevNodeInit(&DSL_DevHeader[i], name, DSL_DRV_DrvNum);
   }
   
   return 0;
}

void DSL_DRV_DeviceDelete(void)
{
   DSL_int_t i;

   printf("Module will be unloaded"DSL_DRV_CRLF);

   DSL_DRV_Cleanup();

   for (i = 0; i < DSL_DRV_MAX_DEVICE_NUMBER; i++)
   {
      iosDevDelete(&DSL_DevHeader[i].DevHdr);
   }

   /* remove the driver also */
   iosDrvRemove(DSL_DRV_DrvNum, IFX_TRUE);

#if defined(INCLUDE_DSL_CPE_API_VINAX) && defined(INCLUDE_DSL_BONDING)
   DSL_DRV_Phy2VirtUnmap(
               (DSL_ulong_t*)DSL_FPGA_BND_BASE_ADDR,
               DSL_FPGA_BND_REGS_SZ_BYTE,
               (DSL_uint8_t**)&g_BndFpgaBase);
#endif /* defined(INCLUDE_DSL_CPE_API_VINAX) && defined(INCLUDE_DSL_BONDING)*/

   /* and allow new DevCreate */
   DSL_DRV_DrvNum = -1;
}

#ifdef __cplusplus
}
#endif

#endif /* VXWORKS*/
