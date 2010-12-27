/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   Simulator
*/

#include "dsl_cpe_control.h"

#include "dsl_cpe_os.h"
#include "dsl_cpe_simulator.h"
#include "drv_dsl_cpe_api.h"
#include "drv_dsl_cpe_api_ioctl.h"
#include "drv_dsl_cpe_debug.h"

#undef DSL_CCA_DBG_BLOCK
#define DSL_CCA_DBG_BLOCK DSL_CCA_DBG_OS

#ifdef DSL_CPE_SIMULATOR_CONTROL

#define DSL_SIM_RX_BUFFER_SIZE   1024

/*static DSL_uint32_t     device_fd=0;*/
static DSL_boolean_t    g_bFirst = DSL_TRUE;
static DSL_boolean_t    g_bVerbose = DSL_TRUE;
static DSL_CPE_Lock_t  semData;
static DSL_DBG_ModuleLevelData_t g_nDbgLevelData[DSL_DBG_MAX_ENTRIES];

typedef struct
{
   DSL_uint8_t      msg_buffer[DSL_SIM_RX_BUFFER_SIZE];
   DSL_uint8_t      msg_count;
   DSL_uint8_t      device_count;
} dsl_line_t;

dsl_line_t dsl_line;


DSL_int_t DSL_CPE_SIM_Open( void *device, const char *appendix )
{
   DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX "open simulated" DSL_CPE_CRLF));

   return (int)device;
}

DSL_int_t DSL_SIM_Close( void *pprivate )
{
   DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX "close simulated" DSL_CPE_CRLF));

   return 0;
}

DSL_int_t DSL_SIM_Write(void *pprivate, const char *buf, const int len )
{
   DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX "write simulated" DSL_CPE_CRLF ));

   return 0;
}

DSL_int_t DSL_SIM_Read( void *pprivate, char *buf, const int len )
{
   DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX "read simulated %d" DSL_CPE_CRLF));

   return 0;
}

DSL_int_t DSL_SIM_Ioctl( void *pprivate, unsigned int cmd, unsigned int arg )
{
   DSL_int_t nRet = 0;

   dsl_line_t *ctx = (dsl_line_t *)pprivate;

   DSL_CPE_LockTimedGet(&semData, 0xFFFF, DSL_NULL);

   switch(cmd)
   {
   case DSL_FIO_VERSION_INFORMATION_GET:
      if(g_bVerbose)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX "ioctl simulated - VersionInformationGet (cmd=0x%08X)" DSL_CPE_CRLF, cmd));
      }
      {
         DSL_VersionInformation_t *pData = (DSL_VersionInformation_t *)arg;
         DSL_char_t verApi[10] = "2.9.9";
         DSL_char_t ver[10] = "0.0.0";
#if defined (INCLUDE_DSL_CPE_API_DANUBE)
         DSL_char_t chip[15] = "Ifx-Danube";
#elif defined (INCLUDE_DSL_CPE_API_VINAX)
         DSL_char_t chip[15] = "Ifx-Vinax";
#else
         DSL_char_t chip[15] = "Ifx-Unknown";
#endif

         memset(&(pData->data), 0, sizeof(pData->data));
         memset(&(pData->accessCtl), 0, sizeof(pData->accessCtl));
         memcpy(pData->data.DSL_DriverVersionApi, verApi, sizeof(verApi));
         memcpy(pData->data.DSL_ChipSetFWVersion, ver, sizeof(ver));
         memcpy(pData->data.DSL_ChipSetHWVersion, ver, sizeof(ver));
         memcpy(pData->data.DSL_ChipSetType, chip, sizeof(chip));
         memcpy(pData->data.DSL_DriverVersionMeiBsp, ver, sizeof(ver));
      }
      break;

   case DSL_FIO_DBG_MODULE_LEVEL_GET:
      {
         DSL_DBG_ModuleLevel_t *pData = (DSL_DBG_ModuleLevel_t *)arg;

         memset(&(pData->accessCtl), 0, sizeof(pData->accessCtl));
         pData->data.nDbgLevel = g_nDbgLevelData[pData->data.nDbgModule].nDbgLevel;
      }
      break;

   case DSL_FIO_DBG_MODULE_LEVEL_SET:
      {
         DSL_DBG_ModuleLevel_t *pData = (DSL_DBG_ModuleLevel_t *)arg;

         memset(&(pData->accessCtl), 0, sizeof(pData->accessCtl));
         g_nDbgLevelData[pData->data.nDbgModule].nDbgLevel = pData->data.nDbgLevel;
         g_nDbgLevelData[pData->data.nDbgModule].nDbgModule = pData->data.nDbgModule;
      }
      break;

   case DSL_FIO_AUTOBOOT_CONTROL_SET:
      break;

   case DSL_FIO_AUTOBOOT_LOAD_FIRMWARE:
      break;

   case DSL_FIO_AUTOBOOT_STATUS_GET:
      {
         DSL_AutobootStatus_t *pData = (DSL_AutobootStatus_t *)arg;
         memset(&(pData->data), 0, sizeof(pData->data));
         memset(&(pData->accessCtl), 0, sizeof(pData->accessCtl));
         pData->data.nStatus = DSL_AUTOBOOT_STATUS_STOPPED;
      }
      break;

   default:
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "unhandled simulated ioctl (cmd=0x%08X)" DSL_CPE_CRLF, cmd));
      nRet = -1;
      break;
   }

   DSL_CPE_LockSet(&semData);

   return nRet;
}

DSL_int_t DSL_SIM_Poll( void *pprivate )
{
   dsl_line_t *ctx = (dsl_line_t *)pprivate;
   return ctx->msg_count;
}
#endif /* DSL_CPE_SIMULATOR_CONTROL */

#ifdef DSL_CPE_SIMULATOR_CONTROL
DSL_Error_t DSL_SimulatorInitialize(DSL_void_t)
{
#ifndef DSL_CPE_SIMULATOR_DRIVER
   int drv_num;

   if(g_bFirst)
   {
      g_bFirst = DSL_FALSE;

      DSL_CPE_LockCreate(&semData);

      drv_num = (int)DEVIO_driver_install(
                   DSL_CPE_SIM_Open,
                   DSL_SIM_Close,
                   DSL_SIM_Read,
                   DSL_SIM_Write,
                   DSL_SIM_Ioctl,
                   DSL_SIM_Poll );

      DEVIO_device_add(&dsl_line, "/dev/dsl_cpe_api", drv_num);
   }
#else
   /* Create DSL CPE API driver Simulator device*/
   if (DSL_DRV_SIM_DeviceCreate() < 0)
   {
      return DSL_ERROR;
   }
#endif /* DSL_CPE_SIMULATOR_DRIVER*/
   return DSL_SUCCESS;
}
#endif /* DSL_CPE_SIMULATOR_CONTROL*/
