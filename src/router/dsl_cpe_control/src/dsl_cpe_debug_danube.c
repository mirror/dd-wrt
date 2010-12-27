/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/*$$ ND:
   TODO:
      1. map all names to own DSL_xxx() names
      2. blocking calls should be replaced by non-blocking
*/

/*
Includes
*/
#include "dsl_cpe_control.h"

#if defined (INCLUDE_DSL_CPE_API_DANUBE)

#include "dsl_cpe_debug.h"
#include "drv_dsl_cpe_cmv_danube.h"
#include "drv_dsl_cpe_api_ioctl.h"

#ifdef DSL_DEBUG_TOOL_INTERFACE

/** TCP messages debug stuff */
#undef DSL_CCA_DBG_BLOCK
#define DSL_CCA_DBG_BLOCK DSL_CCA_DBG_TCPMSG

#define DSL_CPE_FW_SEGMENT_SIZE (64 * 1024)
#define DSL_CPE_HOST_DCE_FLAG 0x1000

extern DSL_int32_t bOptimize;

static DSL_int_t DSL_CPE_DANUBE_TcpMessageFirmwareDownload(
   DSL_char_t *fw_filename,
   DSL_int_t mei_fd)
{
   DSL_int_t ret = 0;
   DSL_int_t nFdImage = 0;
   DSL_char_t *pBuf = NULL;
   DSL_int_t nSize = 0, nReadSize = DSL_CPE_FW_SEGMENT_SIZE;
   struct stat fileStat;

   nFdImage = open(fw_filename, O_RDONLY);
   if (nFdImage <= 0)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX DSL_CPE_CRLF
         "open %s fail." DSL_CPE_CRLF, fw_filename));
      return -1;
   }

   if (lstat(fw_filename, &fileStat) < 0)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX DSL_CPE_CRLF
         "lstat error"));
      return -1;
   }
   nSize = fileStat.st_size;
   pBuf = malloc((DSL_uint32_t)nReadSize);
   if(pBuf == NULL)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX DSL_CPE_CRLF
         "malloc failed in MEI main()"));
      return -1;
   }
   lseek(nFdImage, 0, SEEK_SET);
   lseek(mei_fd, 0, SEEK_SET);

   while(nSize > 0)
   {
      if (nSize > DSL_CPE_FW_SEGMENT_SIZE)
      {
         nReadSize = DSL_CPE_FW_SEGMENT_SIZE;
      }
      else
      {
         nReadSize = nSize;
      }

      if(read(nFdImage, pBuf, (DSL_uint32_t)nReadSize) <= 0)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX DSL_CPE_CRLF
            "firmware image not present"));
         ret = -1;
         break;
      }

      if (write(mei_fd, pBuf, (DSL_uint32_t)nReadSize) != nReadSize)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX DSL_CPE_CRLF
            "write to mei driver failed"));
         ret = -1;
         break;
      }
      nSize -= nReadSize;
   }

   free(pBuf);
   close(nFdImage);
   return ret;
}

static DSL_int_t DSL_CPE_TcpMessageModemCommandHandle(
   DSL_int_t mei_fd,
   DSL_CPE_DANUBE_Message_t *RxMsg,
   DSL_CPE_DANUBE_Message_t *TxMsg,
   DSL_char_t *fw_filename,
   DSL_int_t *MpFlag)
{
   DSL_int_t nRet = 0;
   DSL_uint16_t nPayload = 0;
   DSL_uint16_t  nFunctionOpcode = 0;
   DSL_uint32_t nReadDebugWordSize = 0, nReadDebugOffset = 0;
   DSL_int16_t nTemp = 0;
   DSL_int_t nDataIdx = 0;
   DSL_char_t nTmp;

   /* This message does not have the DCE bit set, so it's a message for the mode (i.e. CMV read, CMV write)
      send the correct message to the Mailbox of ARC wait for reply back from arch and receive it here.
      send via mailbox of arc*/
   *MpFlag = DSL_CPE_MP_FLAG_REPLY;

   /* Payload is ==1 */
   nPayload = 1;

   nFunctionOpcode = (RxMsg->nFunction & 0x0FF0) >> 4;
   if( (nFunctionOpcode & 0x02) == 0 )
   {
      /* Case for CMV Message Code */
      switch (nFunctionOpcode)
      {
      case DSL_CMV_OPCODE_H2D_CMV_READ:
         DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
            "H2D_CMV_READ opcode" DSL_CPE_CRLF));

         /* Case for CMV read reply */
         nFunctionOpcode = DSL_CMV_OPCODE_D2H_CMV_READ_REPLY;
         nPayload = RxMsg->nFunction & 0xF;

         nRet = ioctl(mei_fd, DSL_FIO_BSP_CMV_WINHOST, (int)RxMsg);

         if (nRet < 0)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "Error TCP_IP ioctl: H2D_CMV_READ" DSL_CPE_CRLF));
            nFunctionOpcode = DSL_CMV_OPCODE_D2H_ERROR_CMV_READ_NOT_AVAILABLE;
         }

         *TxMsg = *RxMsg;
         TxMsg->nFunction = (DSL_uint16_t)((nFunctionOpcode << 4) | nPayload);
         break;

      case DSL_CMV_OPCODE_H2D_CMV_WRITE:
         DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
            "H2D_CMV_WRITE opcode" DSL_CPE_CRLF));

         DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
            "Function: %04x, iGroup: %04x, iAddress:%04x, "
            "nIndex: %04x, payload: %04x, payload: %04x" DSL_CPE_CRLF,
            RxMsg->nFunction, RxMsg->nGroup, RxMsg->nAddress,
            RxMsg->nIndex, RxMsg->payload[0], RxMsg->payload[1]));

         nRet = ioctl(mei_fd, DSL_FIO_BSP_CMV_WINHOST, (int)RxMsg);
         nFunctionOpcode = DSL_CMV_OPCODE_D2H_CMV_WRITE_REPLY;
         if (nRet < 0)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "Error TCP_IP ioctl: H2D_CMV_WRITE" DSL_CPE_CRLF));
            nFunctionOpcode = DSL_CMV_OPCODE_D2H_ERROR_CMV_UNKNOWN;
         }
         *TxMsg = *RxMsg;
         TxMsg->nFunction = (DSL_uint16_t)((nFunctionOpcode << 4) | nPayload);
         break;
      default:
         nFunctionOpcode = DSL_CMV_OPCODE_D2H_ERROR_CMV_UNKNOWN;
         nRet = ioctl(mei_fd, DSL_FIO_BSP_CMV_WINHOST, (int)RxMsg);
         if (nRet < 0)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "Error TCP_IP ioctl: D2H_ERROR_CMV_UNKNOWN" DSL_CPE_CRLF));
         }
         *TxMsg = *RxMsg;
         TxMsg->nFunction = (DSL_uint16_t)((nFunctionOpcode << 4)|nPayload);
         break;
      }
   }
   else
   {
      /* Case for Peek/Poke Messages */
      switch (nFunctionOpcode)
      {
      case DSL_CMV_OPCODE_H2D_DEBUG_READ_DM:
         DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
            "H2D_DEBUG_READ_DM opcode" DSL_CPE_CRLF));

         nReadDebugWordSize=RxMsg->nFunction & 0xC000;
         nPayload = RxMsg->nFunction & 0xF;
         nTemp = nPayload;
         nReadDebugOffset = 0;
         switch(nReadDebugWordSize)
         {
         case DSL_CPE_BYTE_SIZE:
            nPayload = (nPayload + 1) / 2 +1;
            if (nPayload > 12 )
            {
               nPayload = 12;
            }
            nReadDebugOffset = RxMsg->nIndex % 2;
            RxMsg->nIndex -= ((DSL_uint16_t)nReadDebugOffset);
            break;
         case DSL_CPE_WORD_SIZE:
            break;
         case DSL_CPE_DWORD_SIZE:
            nPayload = (DSL_uint16_t)(nPayload * 2);
            if (nPayload > 12)
            {
               nPayload = 12;
            }
            /* 32 bit arc buffer = payload [16 bits] 0xAAAABBBB */
            /* format: TxMsg->payload[1]= 0xAAAA TxMsg->payload[0]= 0xBBBB */
            break;
         default:
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "Not support !!" DSL_CPE_CRLF));
            break;
         }

         RxMsg->nFunction = ((RxMsg->nFunction & (~0xF)) | nPayload) & (~(0xC000));
         nRet = ioctl(mei_fd, DSL_FIO_BSP_CMV_WINHOST, (int)RxMsg);
         *TxMsg = *RxMsg;
         nFunctionOpcode = DSL_CMV_OPCODE_D2H_DEBUG_READ_DM_REPLY;
         if (nRet < 0)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "Error TCP_IP ioctl: H2D_DEBUG_READ_DM" DSL_CPE_CRLF));
            nFunctionOpcode = DSL_CMV_OPCODE_D2H_ERROR_ADDR_UNKNOWN;
         }
         nPayload = (DSL_uint16_t)nTemp;
         switch(nReadDebugWordSize)
         {
         case DSL_CPE_BYTE_SIZE:
            nPayload |= DSL_CPE_BYTE_SIZE;
            break;
         case DSL_CPE_WORD_SIZE:
            break;
         case DSL_CPE_DWORD_SIZE:
            nPayload |= DSL_CPE_DWORD_SIZE;
            /* 32 bit arc buffer = payload [16 bits] 0xAAAABBBB */
            /* format: TxMsg->payload[1]= 0xAAAA TxMsg->payload[0]= 0xBBBB */
            break;
         default:
            break;
         }

         /* offset data */
         if (nReadDebugOffset)
         {
            for (nDataIdx = 0; nDataIdx < 23; nDataIdx += 2)
            {
               nTmp = ((char *)TxMsg->payload)[nDataIdx];
               ((char *)TxMsg->payload)[nDataIdx] = ((char *)TxMsg->payload)[nDataIdx + 1];
               ((char *)TxMsg->payload)[nDataIdx + 1] = nTmp;
            }
            for (nDataIdx = 0; nDataIdx < 23; nDataIdx++)
            {
               ((char *)TxMsg->payload)[nDataIdx] =
                  ((char *)TxMsg->payload)[nDataIdx + 1];
            }
            for (nDataIdx = 0; nDataIdx < 23; nDataIdx += 2)
            {
               nTmp = ((char *)TxMsg->payload)[nDataIdx];
               ((char *)TxMsg->payload)[nDataIdx] =
                  ((char *)TxMsg->payload)[nDataIdx + 1];
               ((char *)TxMsg->payload)[nDataIdx + 1] = nTmp;
            }
         }
         TxMsg->nFunction = (DSL_uint16_t)((nFunctionOpcode << 4) | nPayload);
         break;

      case DSL_CMV_OPCODE_H2D_DEBUG_READ_PM:
         DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
            "H2D_DEBUG_READ_PM opcode" DSL_CPE_CRLF));

         nPayload = RxMsg->nFunction &0xF;
         nRet = ioctl(mei_fd, DSL_FIO_BSP_CMV_WINHOST, (int)RxMsg);
         *TxMsg = *RxMsg;
         nFunctionOpcode = DSL_CMV_OPCODE_D2H_DEBUG_READ_FM_REPLY;
         if (nRet < 0)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "Error TCP_IP ioctl: H2D_DEBUG_READ_PM" DSL_CPE_CRLF));
            nFunctionOpcode = DSL_CMV_OPCODE_D2H_ERROR_ADDR_UNKNOWN;
         }
         TxMsg->nFunction = (DSL_uint16_t)((nFunctionOpcode << 4) | nPayload);
         break;

      case DSL_CMV_OPCODE_H2D_DEBUG_WRITE_DM:
         DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
            "H2D_DEBUG_WRITE_DM" DSL_CPE_CRLF));

         DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
            "Function: %04x, iGroup: %04x, iAddress:%04x, nIndex: %04x, "
            "payload[0]:%04x, payload[1]: %04x" DSL_CPE_CRLF, RxMsg->nFunction,
            RxMsg->nGroup, RxMsg->nAddress , RxMsg->nIndex, RxMsg->payload[0],
            RxMsg->payload[1] ));

         nReadDebugWordSize=RxMsg->nFunction & 0xC000; //240561:tc.chen
         nPayload = RxMsg->nFunction &0xF;
         nRet=ioctl(mei_fd,DSL_FIO_BSP_CMV_WINHOST, (int)RxMsg);
         nFunctionOpcode = DSL_CMV_OPCODE_D2H_DEBUG_WRITE_DM_REPLY;
         if (nRet < 0)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "Error TCP_IP ioctl: H2D_DEBUG_WRITE_DM" DSL_CPE_CRLF));
            nFunctionOpcode = DSL_CMV_OPCODE_D2H_ERROR_ADDR_UNKNOWN;
         }

         *TxMsg = *RxMsg;
         TxMsg->nFunction = (DSL_uint16_t)((nFunctionOpcode << 4)|nPayload | nReadDebugWordSize); //240561:tc.chen
         break;
      case DSL_CMV_OPCODE_H2D_DEBUG_WRITE_PM:
         DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
            "H2D_DEBUG_WRITE_PM opcode" DSL_CPE_CRLF));

         nPayload = RxMsg->nFunction & 0xF;
         nRet = ioctl(mei_fd, DSL_FIO_BSP_CMV_WINHOST, (int)RxMsg);
         *TxMsg = *RxMsg;
         nFunctionOpcode = DSL_CMV_OPCODE_D2H_DEBUG_WRITE_FM_REPLY;
         if (nRet < 0)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "Error TCP_IP ioctl: H2D_DEBUG_WRITE_PM" DSL_CPE_CRLF));
            nFunctionOpcode = DSL_CMV_OPCODE_D2H_ERROR_ADDR_UNKNOWN;
         }
         TxMsg->nFunction = (DSL_uint16_t)((nFunctionOpcode << 4) | nPayload);
         break;
      default:
         nRet = ioctl(mei_fd, DSL_FIO_BSP_CMV_WINHOST, (int)RxMsg);
         if (nRet < 0)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "Error TCP_IP default" DSL_CPE_CRLF));
         }
         *TxMsg = *RxMsg;
         nFunctionOpcode = DSL_CMV_OPCODE_D2H_ERROR_ADDR_UNKNOWN;
         TxMsg->nFunction = (DSL_uint16_t)((nFunctionOpcode << 4) | nPayload);
         break;
      }
   }

   return nRet;
}

static DSL_int_t DSL_CPE_DANUBE_LoadFirmwareFromFile(
   DSL_char_t *psFirmwareName,
   DSL_uint8_t **pFirmware,
   DSL_uint32_t *pnFirmwareSize)
{
   DSL_int_t nRet = 0;
   DSL_CPE_File_t *fd_image = DSL_NULL;
   DSL_CPE_stat_t file_stat;

   *pnFirmwareSize = 0;
   if (*pFirmware != DSL_NULL)
   {
      DSL_CPE_Free(*pFirmware);
      *pFirmware = DSL_NULL;
   }

   for (;;)
   {
      fd_image = DSL_CPE_FOpen (psFirmwareName, "rb");
      if (fd_image == DSL_NULL)
      {
         DSL_CPE_FPrintf (DSL_CPE_STDOUT,
            DSL_CPE_PREFIX "open %s fail!"DSL_CPE_CRLF, psFirmwareName);

         nRet = -1;
         break;
      }

      DSL_CPE_FStat (psFirmwareName, &file_stat);
      *pnFirmwareSize = file_stat.st_size;
      *pFirmware = DSL_CPE_Malloc (*pnFirmwareSize);
      if (DSL_CPE_FRead (*pFirmware, sizeof (DSL_uint8_t), *pnFirmwareSize, fd_image) <= 0)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "\n firmware_image not present!"DSL_CPE_CRLF));

         nRet = -1;
         break;
      }

      break;
   }

   if (fd_image != DSL_NULL)
   {
      DSL_CPE_FClose (fd_image);
      fd_image = DSL_NULL;
   }

   return nRet;
}

static DSL_int_t DSL_CPE_DANUBE_Reboot(DSL_void_t)
{
   DSL_int_t nRet = 0;
   DSL_char_t *pcFw = DSL_NULL;
   DSL_AutobootLoadFirmware_t ldFw;
   DSL_AutobootControl_t pAcs;
   DSL_AutobootStatus_t pAsg;
   DSL_CPE_Control_Context_t *pContext = DSL_NULL;
   DSL_CPE_File_t *fd_image = DSL_NULL;
   DSL_CPE_stat_t file_stat;
   DSL_uint8_t *pChunkData = DSL_NULL;
   DSL_int_t nSize = 0, nReadSize = DSL_CPE_FW_CHUNK_SIZE,
             nWriteSize = 0;

   if (strlen(g_sFirmwareName1) > 0)
   {
      pcFw = g_sFirmwareName1;
   }

   memset(&ldFw, 0, sizeof(DSL_AutobootLoadFirmware_t));

   pContext = DSL_CPE_GetGlobalContext();
   if (pContext == DSL_NULL)
   {
     return -1;
   }

   while(1)
   {
      if (bOptimize == 1)
      {
         /*
            Footprint optimized download uses chunks for firmware binary download.
            This is to reduce the necessary runtime memory allocation for download
            procedure
         */
         DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX "Using optimized fw "
            "download..." DSL_CPE_CRLF));

         if (pcFw == DSL_NULL)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               " firmware file not found!" DSL_CPE_CRLF, pcFw));
            nRet = -1;
            break;
         }

         fd_image = DSL_CPE_FOpen (pcFw, "r");
         if (fd_image == DSL_NULL)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX " open %s failed."
               DSL_CPE_CRLF, pcFw));
            nRet = -1;
            break;
         }

         DSL_CPE_FStat (pcFw, &file_stat);
         nSize = file_stat.st_size;

         pChunkData = DSL_CPE_Malloc ((DSL_uint32_t)nReadSize);
         if (pChunkData == DSL_NULL)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX "Memory allocation "
               "failed (optimized FW download)" DSL_CPE_CRLF));
            nRet = -1;
            break;
         }

         ldFw.data.pFirmware = pChunkData;

         while (nSize > 0)
         {
            if (nSize > DSL_CPE_FW_CHUNK_SIZE)
            {
               nReadSize = DSL_CPE_FW_CHUNK_SIZE;
            }
            else
            {
               nReadSize = nSize;
            }

            if (DSL_CPE_FRead (pChunkData, sizeof (DSL_uint8_t), (DSL_uint32_t)nReadSize,
               fd_image) <= 0)
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
                  "firmware_image not present" DSL_CPE_CRLF));
               nRet = -1;
               break;
            }

            ldFw.data.nFirmwareSize   = (DSL_uint32_t)nReadSize;
            ldFw.data.nFirmwareOffset = (DSL_uint32_t)nWriteSize;

            if (nSize - nReadSize <= 0)
            {
               ldFw.data.bLastChunk = DSL_TRUE;
            }
            else
            {
               ldFw.data.bLastChunk = DSL_FALSE;
            }

            nRet = DSL_CPE_Ioctl(pContext->fd[pContext->nDevNum],
                      DSL_FIO_AUTOBOOT_LOAD_FIRMWARE, (DSL_int_t) &ldFw);

            if (nRet < 0)
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
                  "Autoboot Load Firmware (using chunks) failed!, nRet = %d!"
                  DSL_CPE_CRLF, ldFw.accessCtl.nReturn));

               nRet = -1;
               break;
            }

            nSize -= nReadSize;
            nWriteSize += nReadSize;
         }

         if ( nRet < 0)
         {
            break;
         }

         /* $$TD: Temporary only because better solution would be to start
               the link directly within driver after last chunk is downloaded */
         DSL_CPE_Sleep(1);
         nRet = (DSL_Error_t) DSL_CPE_Ioctl(pContext->fd[pContext->nDevNum],
               DSL_FIO_AUTOBOOT_STATUS_GET, (DSL_int_t) &pAsg);
         if (pAsg.data.nStatus == DSL_AUTOBOOT_STATUS_STOPPED)
         {
            pAcs.data.nCommand = DSL_AUTOBOOT_CTRL_START;
            nRet = (DSL_Error_t) DSL_CPE_Ioctl(pContext->fd[pContext->nDevNum],
               DSL_FIO_AUTOBOOT_CONTROL_SET, (DSL_int_t) &pAcs);
         }
         break;
      }
      else
      {
         if (pcFw != DSL_NULL)
         {
             nRet = DSL_CPE_DANUBE_LoadFirmwareFromFile(
                      pcFw, &ldFw.data.pFirmware, &ldFw.data.nFirmwareSize);
         }

         if ( ((ldFw.data.pFirmware  != DSL_NULL) && (ldFw.data.nFirmwareSize)) )
         {
            ldFw.data.bLastChunk = DSL_TRUE;

            nRet = (DSL_Error_t) DSL_CPE_Ioctl(pContext->fd[pContext->nDevNum],
               DSL_FIO_AUTOBOOT_LOAD_FIRMWARE, (DSL_int_t) &ldFw);

            if (nRet < 0)
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
                     "Reboot failed!"DSL_CPE_CRLF));
            }
            break;
         }
         else
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
                  "FW binary not specified!"DSL_CPE_CRLF));

            nRet = -1;
            break;
         }
      }
   }

   /* take care to clean up all memory that might be allocated during function
      processing */
   if (fd_image != DSL_NULL)
   {
      DSL_CPE_FClose(fd_image);
   }

   if (pChunkData != DSL_NULL)
   {
      DSL_CPE_Free(pChunkData);
      pChunkData = DSL_NULL;
      ldFw.data.pFirmware = DSL_NULL;
   }

   if (ldFw.data.pFirmware)
   {
      DSL_CPE_Free(ldFw.data.pFirmware);
   }

   return nRet;
}

static DSL_int_t DSL_CPE_DANUBE_TcpMessageDceCommandHandle(
   DSL_int_t mei_fd,
   DSL_CPE_DANUBE_Message_t *RxMsg,
   DSL_CPE_DANUBE_Message_t *TxMsg,
   DSL_char_t *fw_filename,
   DSL_int_t *MpFlag)
{
   DSL_uint16_t nFunctionOpcode;
   DSL_int_t nRet=0;
   DSL_CPE_DANUBE_ReadDebug_Parameter_t readDebugDeviceData;
   DSL_uint32_t nReadDebugCount, nReadDebugWordSize, nReadDebugOffset;
   DSL_uint32_t nBaseAddress;
   DSL_uint32_t i = 0, j = 0;
   DSL_int_t nPayload = 0;
   DSL_CPE_DANUBE_Parameter_t devicedata;
   DSL_int_t nRemote = 0;

   /* Message for DCE, not Modem... */
   DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
      "Message for DCE, not modem" DSL_CPE_CRLF));

   *MpFlag = DSL_CPE_MP_FLAG_REPLY;
   /* Obtain the function op_code */
   nFunctionOpcode = (RxMsg->nFunction & 0x0FF0) >> 4;

   switch (nFunctionOpcode)
   {
      case H2DCE_DEBUG_RESET:
         DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
            "H2DC_DEBUG_RESET (%d) opcode" DSL_CPE_CRLF, DSL_FIO_BSP_RESET));

         nRet = ioctl(mei_fd, DSL_FIO_BSP_RESET, (int)NULL);
         nFunctionOpcode = DCE2H_DEBUG_RESET_ACK;
         if (nRet<0)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "Error TCP_IP ioctl: H2DCE_DEBUG_RESET" DSL_CPE_CRLF));
            nFunctionOpcode = 0;
         }
         else
         {
            nRet = ioctl(mei_fd, DSL_FIO_BSP_DSL_START, (int)NULL);
            if (nRet < 0)
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
                  "Error TCP_IP ioctl: IFX_ADSL_BSP_IOC_DSLSTART" DSL_CPE_CRLF));
               nFunctionOpcode = 0;
            }

#if 0
            if (ioctl(mei_fd, AUTOBOOT_CONTROL_SET , 0) < 0)
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX DSL_CPE_CRLF DSL_CPE_CRLF
                  "mei ioctl AUTOBOOT_CONTROL_SET fail." DSL_CPE_CRLF));
            }
#endif
         }

         TxMsg->nFunction = (DSL_uint16_t)((nFunctionOpcode << 4) | DSL_CPE_DCE_COMMAND);
         TxMsg->nAddress = 0;
         TxMsg->nIndex = 0;
         for (i = 0; i < NEW_MP_PAYLOAD_SIZE; i++)
         {
            TxMsg->payload[i] = 0;
         }
         break;

         /*
            Reboot modem - Halt the modem, yank its reset line, reload its code...
            then respond to WinHost
         */

   case H2DCE_DEBUG_REBOOT:
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "H2DCE_DEBUG_REBOOT opcode" DSL_CPE_CRLF));

      nRet = DSL_CPE_DANUBE_Reboot();

      nFunctionOpcode = DCE2H_DEBUG_REBOOT_ACK;
      if (nRet < 0)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "Error TCP_IP ioctl: H2DCE_DEBUG_REBOOT" DSL_CPE_CRLF));
         nFunctionOpcode = 0;
      }

      TxMsg->nFunction = (DSL_uint16_t)((nFunctionOpcode << 4) | DSL_CPE_DCE_COMMAND);
      TxMsg->nGroup = 0;
      TxMsg->nAddress = 0;
      TxMsg->nIndex = 0;
      for (i = 0; i < NEW_MP_PAYLOAD_SIZE; i++)
      {
         TxMsg->payload[i] = 0;
      }
      /* Reboot and Reset has same function
      send(socketaccept,(char *)&TxMsg, sizeof(&TxMsg), 0);*/
      break;

    /* 24.01.2005 : H2DCE Debug _Download */
   case H2DCE_DEBUG_DOWNLOAD:
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "H2DCE_DEBUG_DOWNLOAD opcode" DSL_CPE_CRLF));

      if (fw_filename == DSL_NULL)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "Please specify the firmware file." DSL_CPE_CRLF));
         nRet = -ENOENT;
         break;
      }
      else
      {
         nRet = DSL_CPE_DANUBE_TcpMessageFirmwareDownload(fw_filename,mei_fd);
         if (nRet<0)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "Error TCP_IP ioctl: H2DCE_DEBUG_DOWNLOAD failed." DSL_CPE_CRLF));
         }
         nRet = ioctl(mei_fd,DSL_FIO_BSP_BOOTDOWNLOAD, (int)NULL);
         if (nRet < 0)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "Error TCP_IP ioctl: H2DCE_DEBUG_DOWNLOAD" DSL_CPE_CRLF));
         }
         TxMsg->nFunction = (DSL_uint16_t)(nFunctionOpcode << 4) | DSL_CPE_DCE_COMMAND;
         TxMsg->nGroup = 0;
         TxMsg->nAddress = 0;
         TxMsg->nIndex = 0;
         for (i = 0; i < NEW_MP_PAYLOAD_SIZE; i++)
         {
            TxMsg->payload[i] = 0;
         }
      }
      break;

      /*** 01- 09 - 2004:  H2DCE Debug_ readDebug *****/
   case H2DCE_DEBUG_READDEBUG:
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "H2DCE_DEBUG_READDEBUG opcode" DSL_CPE_CRLF));

      readDebugDeviceData.nAddress = (
         (DSL_uint32_t)((RxMsg->nAddress & 0x0000FFFF) << 16)
         | (DSL_uint32_t)(RxMsg->nIndex & 0x0000FFFF ) );
      nReadDebugWordSize = RxMsg->nFunction & 0xC000;
      nReadDebugCount = RxMsg->nFunction & 0x0F;

      nFunctionOpcode = H2DCE_DEBUG_READDEBUG_ACK;
      TxMsg->nFunction = (DSL_uint16_t)((RxMsg->nFunction & 0xC000)
         | (nFunctionOpcode << 4) | DSL_CPE_HOST_DCE_FLAG
         | 0x0010 | (RxMsg->nFunction & 0x0F));

      readDebugDeviceData.nReadCount = nReadDebugCount;
      switch(nReadDebugWordSize)
      {
         /* DSL_CPE_BYTE_SIZE       1 << 14 */
      case DSL_CPE_BYTE_SIZE:
         if (nReadDebugCount % 4) /* not divisible by 4 */
         {
            if( readDebugDeviceData.nAddress % 4) /* Not an even boundary */
            {
               readDebugDeviceData.nReadCount = (nReadDebugCount /4) + 1;
               nReadDebugOffset = readDebugDeviceData.nAddress % 4;
            }
            else
            {
               /* Even Dword boundary */
               readDebugDeviceData.nReadCount = (nReadDebugCount /4 ) + 1;
               nReadDebugOffset = 0;
            }
         }
         else
         {  /* Even DWord count */
            if( readDebugDeviceData.nAddress % 4) /* Not an even boundary */
            {
               readDebugDeviceData.nReadCount = (nReadDebugCount /4) + 2;
               nReadDebugOffset = readDebugDeviceData.nAddress % 4;
            }
            else
            {
               /* Even DWord boundary */
               readDebugDeviceData.nReadCount = (nReadDebugCount /4 );
               nReadDebugOffset = 0;
            }
         }

         readDebugDeviceData.nAddress = (readDebugDeviceData.nAddress & 0xFFFFFFFC );
         /* No data is returned to the CPE */
         /* No offsets Only double word read is supported
            in this case is returned to the CPE user */
         nRet = ioctl(mei_fd, DSL_FIO_BSP_DEBUG_READ, (int)&readDebugDeviceData);
         if (nRet < 0)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "Error TCP_IP ioctl: H2DCE_DEBUG_READDEBUG." DSL_CPE_CRLF));
            TxMsg->nFunction &= ~(0x0FF0);
         }
         /* nPayload = 12 */
         if (readDebugDeviceData.nReadCount > 12)
         {
            readDebugDeviceData.nReadCount = 12;
         }

         /* 32 bit arc buffer = payload [8 bits] */
         /* send back data to useri of the following format 0xAABBCCDD */
         /* payload[1]= 0xAA  0xBB  payload[0]= 0xCC 0xDD */
         for (i=0; i < readDebugDeviceData.nReadCount; i++)
         {
            if(nReadDebugOffset == 0)
            {
               /* TxMsg->payload[i] =(DSL_uint16_t) readDebugDeviceData.buffer[i + nReadDebugOffset]; */
               j = i*2;
               TxMsg->payload[j+1] =
                  (DSL_uint16_t)((readDebugDeviceData.buffer[i+nReadDebugOffset]
                  & 0xFFFF0000) >> 16);
               TxMsg->payload[j] =
                  (DSL_uint16_t)(readDebugDeviceData.buffer[i+nReadDebugOffset]
                  & 0x0000FFFF);
            }
            else if (nReadDebugOffset == 1)
            {
               j=i*2;
               /* printf("%d, %08x, %08x \n", nReadDebugOffset,
                  readDebugDeviceData.buffer[j],
                  readDebugDeviceData.buffer[j+1] ); */
               /* swap the bytes to edian */
               TxMsg->payload[j + 1] =
                  (DSL_uint16_t)((readDebugDeviceData.buffer[j+1]
                     & 0xFF000000) >> 24 )
                  | (DSL_uint16_t)((readDebugDeviceData.buffer[j+1]
                     & 0x000000FF ) << 8) ;
               TxMsg->payload[j] =
                  (DSL_uint16_t)((readDebugDeviceData.buffer[j]
                     & 0x0000FF00) >> 8 )
                  | (DSL_uint16_t)((readDebugDeviceData.buffer[j+1]
                     & 0x00FF0000)>> 8);
               /* swap due to endian */
            }
            else if (nReadDebugOffset == 2)
            {
               j = i * 2;
               TxMsg->payload[j+1] =
                  (DSL_uint16_t)((readDebugDeviceData.buffer[j+1]
                     & 0x0000FFFF) );
               TxMsg->payload[j]   =
                  (DSL_uint16_t)((readDebugDeviceData.buffer[j]
                     & 0xFFFF0000) >> 16 ) ;
            }
            else if (nReadDebugOffset == 3)
            {
               j = i * 2;
               /*printf("%d, %08x, %08x \n", nReadDebugOffset, readDebugDeviceData.buffer[j], readDebugDeviceData.buffer[j+1] );*/
               /* swap the bytes to edian */
               TxMsg->payload[j+1] =
                  (DSL_uint16_t)((readDebugDeviceData.buffer[j+1]
                     & 0x0000FF00) >> 8 )
                  | (DSL_uint16_t)((readDebugDeviceData.buffer[j+2]
                     & 0x00FF0000 ) >> 8) ;
               TxMsg->payload[j] =
                  (DSL_uint16_t)((readDebugDeviceData.buffer[j+1]
                     & 0x00FF0000) >> 16 )
                  | (DSL_uint16_t)((readDebugDeviceData.buffer[j+1]
                     & 0x000000FF) << 8 );
               /* swap due to endian */
            }
         }
         break;
      case DSL_CPE_WORD_SIZE:
         /*
            Read the data and send it back.
            Trick cases are non-DWORD boundaries and
            odd counts.
         */
         if (nReadDebugCount & 1 ) /* odd number to read */
         {
            if (readDebugDeviceData.nAddress & 2 ) /* Odd Address */
            {
               readDebugDeviceData.nReadCount = (nReadDebugCount / 2) + 1;
               nReadDebugOffset = 1;
            }
            else
            {
               /* Even address */
               readDebugDeviceData.nReadCount = (nReadDebugCount / 2) + 1;
               nReadDebugOffset = 0;
            }
         }
         else
         {
            /* even number to read */
            if(readDebugDeviceData.nAddress & 2 ) /* Odd Address */
            {
               readDebugDeviceData.nReadCount = (nReadDebugCount / 2) + 1;
               nReadDebugOffset = 1;
            }
            else
            {
               /* even address */
               readDebugDeviceData.nReadCount = (nReadDebugCount / 2);
               nReadDebugOffset = 0;
            }
         }

         readDebugDeviceData.nAddress =
            (readDebugDeviceData.nAddress & 0xFFFFFFFC );
         /* No offsets Only double word read is supported in
            this case is returned to the CPE user */
         if (readDebugDeviceData.nReadCount <= 20)
         {
            /* No offsets Only double word read is supported in this
               case is returned to the CPE user */
            nRet = ioctl(mei_fd, DSL_FIO_BSP_DEBUG_READ, (int)&readDebugDeviceData);
            if (nRet < 0)
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
                  "Error TCP_IP ioctl: H2DCE_DEBUG_READDEBUG" DSL_CPE_CRLF));
               TxMsg->nFunction &= ~(0x0FF0);
            }
            if (readDebugDeviceData.nReadCount > 12)
            {
               readDebugDeviceData.nReadCount = 12;
            }

            /* 32 bit arc buffer = payload [8 bits] */
            /* send back data to useri of the following format 0xAAAABBBB */
            for(i = 0; i < readDebugDeviceData.nReadCount; i++)
            {
               if (nReadDebugOffset == 0)
               {
                  j = i * 2;
                  TxMsg->payload[j+1] =
                     (DSL_uint16_t)((readDebugDeviceData.buffer[i+nReadDebugOffset]
                     & 0xFFFF0000) >> 16);
                  TxMsg->payload[j] =
                     (DSL_uint16_t)(readDebugDeviceData.buffer[i+nReadDebugOffset]
                     & 0x0000FFFF);
               }
               else
               {
                  j = i * 2;
                  TxMsg->payload[j+1] =
                     (DSL_uint16_t)((readDebugDeviceData.buffer[i+nReadDebugOffset]
                     & 0x0000FFFF));
                  TxMsg->payload[j] =
                     (DSL_uint16_t)((readDebugDeviceData.buffer[i+nReadDebugOffset+1]
                     & 0xFFFF0000) >> 16);
               }
            }
         }
         else
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "ReadCount greater than 20" DSL_CPE_CRLF));
         }
         break;
      case DSL_CPE_DWORD_SIZE:
         /* No offsets Only double word read is supported in this
            case is returned to the CPE user*/
         nRet = ioctl(mei_fd, DSL_FIO_BSP_DEBUG_READ, (int)&readDebugDeviceData);
         if (nRet<0)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "Error TCP_IP ioctl: H2DCE_DEBUG_READDEBUG doubleword" DSL_CPE_CRLF));
            TxMsg->nFunction &= ~(0x0FF0);
         }
         nReadDebugOffset = 0;
         if (readDebugDeviceData.nReadCount > 12)
         {
            readDebugDeviceData.nReadCount = 12;
         }

         /* 32 bit arc buffer = payload [16 bits] 0xAAAABBBB */
         /* format: TxMsg->payload[1]= 0xAAAA TxMsg->payload[0]= 0xBBBB */
         for (i=0; i < readDebugDeviceData.nReadCount; i++)
         {
            j=i*2;
            TxMsg->payload[j+1] =
               (DSL_uint16_t)((readDebugDeviceData.buffer[i+nReadDebugOffset]
               & 0xFFFF0000) >> 16);
            TxMsg->payload[j] =
               (DSL_uint16_t)(readDebugDeviceData.buffer[i+nReadDebugOffset]
               & 0x0000FFFF);
         }
         break;
      default:
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "Error H2DCE_DEBUG_READDEBUG" DSL_CPE_CRLF));
      }
      TxMsg->nIndex = RxMsg->nIndex ;
      break;

      /***  Read an MEI register ***/
   case H2DCE_DEBUG_READ_MEI:
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "H2DCE_DEBUG_READ_MEI opcode" DSL_CPE_CRLF));

      nRet = ioctl(mei_fd, DSL_FIO_BSP_GET_BASE_ADDRESS, (int)&nBaseAddress);
      if (nRet<0 || nBaseAddress == 0)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "Error TCP_IP ioctl: DSL_FIO_BSP_GET_BASE_ADDRESS failed" DSL_CPE_CRLF));
      }
      nPayload = 2;
      devicedata.nAddress = RxMsg->nAddress  +  nBaseAddress;
      devicedata.nData = (((RxMsg->payload[1] & 0x0000FFFF) << 16)
         + (RxMsg->payload [0] & 0xFFFF));
      switch(RxMsg->nGroup)   /* nGroup */
      {
      case 0:  /* Both words */
         nRet = ioctl(mei_fd, DSL_FIO_BSP_CMV_READ, (int)&devicedata);
         if (nRet < 0)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "Error TCP_IP ioctl: H2DCE_DEBUG_READ_MEI" DSL_CPE_CRLF));
         }
         break;
      default:
         break;
      }

      nFunctionOpcode = DCE2H_DEBUG_READ_MEI_REPLY;
      TxMsg->nFunction = (DSL_uint16_t)((nFunctionOpcode << 4) | DSL_CPE_HOST_DCE_FLAG
         | nPayload);

      TxMsg->nAddress = RxMsg->nAddress;
      TxMsg->nIndex = RxMsg->nIndex ;

      TxMsg->payload[1] = 0;
      TxMsg->payload[0] = 0;
      TxMsg->payload[1] = (DSL_uint16_t) ((devicedata.nData
         & 0xFFFF0000) >> 16 );
      TxMsg->payload[0] = (DSL_uint16_t) (devicedata.nData & 0x0000FFFF);
      break;

      /*
         Write to MEI register
      */
   case H2DCE_DEBUG_WRITE_MEI:
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "H2DCE_DEBUG_WRITE_MEI opcode" DSL_CPE_CRLF));

      nRet = ioctl(mei_fd, DSL_FIO_BSP_GET_BASE_ADDRESS, (int)&nBaseAddress);
      if (nRet < 0 || nBaseAddress == 0)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "Error TCP_IP ioctl: DSL_FIO_BSP_GET_BASE_ADDRESS failed" DSL_CPE_CRLF));
      }
      nPayload=2;
      devicedata.nAddress = RxMsg->nAddress  +  nBaseAddress;
      devicedata.nData = (((RxMsg->payload[1] & 0x0000FFFF) << 16)
         + (RxMsg->payload [0] & 0xFFFF));
      nRet = ioctl(mei_fd, DSL_FIO_BSP_CMV_WRITE, (int)&devicedata);
      if (nRet < 0)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "Error TCP_IP ioctl: H2DCE_DEBUG_WRITE_MEI" DSL_CPE_CRLF));
      }
      nFunctionOpcode = DCE2H_DEBUG_WRITE_MEI_REPLY;
      TxMsg->nFunction = (DSL_uint16_t)((nFunctionOpcode << 4) | DSL_CPE_HOST_DCE_FLAG
         | nPayload);
      TxMsg->nAddress = RxMsg->nAddress;
      TxMsg->nIndex = RxMsg->nIndex ;

      TxMsg->payload[1] = 0;
      TxMsg->payload[0] = 0;
      TxMsg->payload[1] =
         (DSL_uint16_t) ((devicedata.nData & 0xFFFF0000) >> 16 );
      TxMsg->payload[0] = (DSL_uint16_t) (devicedata.nData & 0x0000FFFF);
      break;

      /*:w:W
         H2DCE_DEBUG_HALT
      */
   case H2DCE_DEBUG_HALT:
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "H2DCE_DEBUG_HALT opcode" DSL_CPE_CRLF));

      nPayload = 1;
      nRet = ioctl(mei_fd, DSL_FIO_BSP_HALT, (int)NULL);
      if (nRet<0)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "Error TCP_IP ioctl: H2DCE_DEBUG_HALT" DSL_CPE_CRLF));
      }
      nFunctionOpcode = DCE2H_DEBUG_HALT_ACK;
      TxMsg->nFunction = (DSL_uint16_t)((nFunctionOpcode << 4) | DSL_CPE_DCE_COMMAND | nPayload);
      TxMsg->nGroup = 0;
      TxMsg->nAddress = 0;
      TxMsg->nIndex = 0;
      for (i = 0; i< NEW_MP_PAYLOAD_SIZE; i++)
      {
         TxMsg->payload[i] = 0;
      }
      break;

   /*
      Remote
   */
   case H2DCE_DEBUG_REMOTE:
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "H2DCE_DEBUG_REMOTE opcode" DSL_CPE_CRLF));

      nPayload = 1;
      nRemote = (RxMsg->payload[0] & 0xFFFF );
      nRet = ioctl(mei_fd, DSL_FIO_BSP_REMOTE, (int)&nRemote);
      if (nRet < 0)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "Error TCP_IP ioctl: H2DCE_DEBUG_REMOTE" DSL_CPE_CRLF));
      }
      nFunctionOpcode = H2DCE_DEBUG_REMOTE + 1;
      TxMsg->nFunction = (DSL_uint16_t)((nFunctionOpcode << 4) | DSL_CPE_DCE_COMMAND
         | nPayload);
      TxMsg->nGroup = 0;
      TxMsg->nAddress = 0;
      TxMsg->nIndex = 0;
      for (i = 0; i < NEW_MP_PAYLOAD_SIZE; i++)
      {
         TxMsg->payload[i] = 0;
      }
      break;

   case H2DCE_DEBUG_RUN:
      nRet = ioctl(mei_fd, DSL_FIO_BSP_RUN, (int)NULL);
      if (nRet < 0)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "Error TCP_IP ioctl: H2DCE_DEBUG_START" DSL_CPE_CRLF));
      }

      /* nFunctionOpcode = DCE2H_DEBUG_RESET_ACK; */
      TxMsg->nFunction = (DSL_uint16_t)(nFunctionOpcode << 4) | DSL_CPE_DCE_COMMAND;
      TxMsg->nGroup = 0;
      TxMsg->nAddress = 0;
      TxMsg->nIndex = 0;
      for (i = 0; i < NEW_MP_PAYLOAD_SIZE; i++)
      {
         TxMsg->payload[i] = 0;
      }

      break;

    /*
    **   Default
    */
   default:
      nPayload = 1;
      nFunctionOpcode = DCE2H_ERROR_OPCODE_UNKNOWN;
      TxMsg->nFunction = (DSL_uint16_t)((nFunctionOpcode << 4) | DSL_CPE_DCE_COMMAND | nPayload);
      TxMsg->nGroup = 0;
      TxMsg->nAddress = 0;
      TxMsg->nIndex = 0;
      for (i = 0; i < NEW_MP_PAYLOAD_SIZE; i++)
      {
         TxMsg->payload[i] = 0;
      }
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "ERROR TCP_IP: DCE2H_ERROR_OPCODE_UNKNOWN" DSL_CPE_CRLF));
      break;
   }

   return nRet;
}

#ifdef INCLUDE_DSL_RESOURCE_STATISTICS
DSL_Error_t DSL_CPE_DEV_TcpDebugMessageResourceUsageGet(
   DSL_CPE_TcpDebugClientInfo_t *clientInfo,
   DSL_uint32_t *pStaticMemUsage,
   DSL_uint32_t *pDynamicMemUsage)
{
   DSL_int_t c = 0;

   if (clientInfo == DSL_NULL ||
       pStaticMemUsage == DSL_NULL ||
       pDynamicMemUsage == DSL_NULL)
   {
      return DSL_ERROR;
   }

   *pStaticMemUsage = 0;
   *pDynamicMemUsage = 0;

   for (c = 0; c < DSL_FD_SETSIZE; c++)
   {
      if (clientInfo[c].pDevData != DSL_NULL)
      {
         *pDynamicMemUsage += sizeof(DSL_DANUBE_TcpDebugInfo_t);
      }
   }

   return DSL_SUCCESS;
}
#endif /* INCLUDE_DSL_RESOURCE_STATISTICS*/

DSL_void_t DSL_CPE_DEV_DeviceDataFree(DSL_void_t *pDevData)
{
   if (pDevData)
   {
      if ( ((DSL_DANUBE_TcpDebugInfo_t*)pDevData)->arrDeviceFd > 0)
      {
         /* Close low-level driver*/
         close(((DSL_DANUBE_TcpDebugInfo_t*)pDevData)->arrDeviceFd);
      }

      DSL_CPE_Free(pDevData);
   }

   return;
}

DSL_int_t DSL_CPE_DEV_TcpMessageHandle(
   DSL_CPE_TcpDebugClientInfo_t *pDevData)
{
   DSL_int_t nBytesReceived, nBytesSent;
   DSL_int_t nMpFlag=0;
   DSL_CPE_DANUBE_Message_t RxMsg;
   DSL_CPE_DANUBE_Message_t TxMsg;
   DSL_int_t mei_fd;
   DSL_int_t nRet = 0;
   DSL_DANUBE_TcpDebugInfo_t *pTcpDebugInfo = (DSL_DANUBE_TcpDebugInfo_t*)(pDevData->pDevData);

   if (pDevData->pDevData == DSL_NULL)
   {
      /* Create device specific data*/
      pDevData->pDevData = DSL_CPE_Malloc(sizeof(DSL_DANUBE_TcpDebugInfo_t));
      if (pDevData->pDevData == DSL_NULL)
      {
         DSL_CPE_FPrintf (DSL_CPE_STDOUT,
            DSL_CPE_PREFIX "Can't allocate memory for device specific data!"DSL_CPE_CRLF);
         return -1;
      }

      pTcpDebugInfo = (DSL_DANUBE_TcpDebugInfo_t*)(pDevData->pDevData);
      pTcpDebugInfo->arrDeviceFd = -1;
   }

   nBytesReceived = DSL_CPE_SocketRecv(pDevData->fd, (char *)&RxMsg,
      (sizeof(RxMsg) / sizeof(DSL_uint16_t)) * 2);

   /* Error or termination from host */
   if(nBytesReceived <= 0)
   {
      return -ENODATA;
   }

   DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
      "RxMsg:Len:%d nFunction:%04x, Group:%04x, nAddress:%04x, nIndex:%04x"
      DSL_CPE_CRLF, nBytesReceived, RxMsg.nFunction, RxMsg.nGroup, RxMsg.nAddress,
      RxMsg.nIndex));

   if (pTcpDebugInfo->arrDeviceFd < 0)
   {
      /* Open low-level device*/
      pTcpDebugInfo->arrDeviceFd = DSL_CPE_DEV_DeviceOpen(DSL_CPE_IFX_LOW_DEV, 0);

      if (pTcpDebugInfo->arrDeviceFd <= 0)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
                     "Error: MEI BSP device file open failed!" DSL_CPE_CRLF));
         return -1;
      }
   }

   mei_fd = pTcpDebugInfo->arrDeviceFd;

   /** If the message is the right nSize... (16 x 16 bit)  32 Bytes received*/
   if (nBytesReceived == DSL_CPE_MSG_RECV_LENGTH)
   {
      if ((RxMsg.nFunction & DSL_CPE_DCE_COMMAND) != 0)
      {
         /* Message for DCE, not Modem... */
         nRet = DSL_CPE_DANUBE_TcpMessageDceCommandHandle(mei_fd, &RxMsg, &TxMsg,
            g_sFirmwareName1, &nMpFlag);
      }
      else
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
            "Message for modem, not DCE" DSL_CPE_CRLF));

         nRet = DSL_CPE_TcpMessageModemCommandHandle(mei_fd, &RxMsg, &TxMsg,
            g_sFirmwareName1, &nMpFlag);
      }/* end of message for modem*/

      /* Setup the new return Message */
      if (DSL_CPE_MP_FLAG_REPLY == nMpFlag)
      {
         /* MSG_DONTWAIT Enables non-blocking operation if the operation
            would block EAGAIN is returned. #define MSG_DONTWAIT 0x40 */
         /* Bytes of message sent has to be 32 bytes */
         nBytesSent = DSL_CPE_SocketSend(pDevData->fd, (char *)&TxMsg,
            (sizeof(TxMsg) / sizeof(DSL_uint16_t)) * 2);
         /*nBytesSent=send(socketaccept,(char *)&TxMsg,sizeof(TxMsg),0 );*/
         if(nBytesSent == -1)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "Error: send failed" DSL_CPE_CRLF));
         }
      }
   } /* End of message length ==32 */

   return 0;
}

#endif /* #ifdef DSL_DEBUG_TOOL_INTERFACE*/

#if (defined(INCLUDE_DSL_CPE_CLI_SUPPORT) && !defined(DSL_CPE_DEBUG_DISABLE)) || defined(DSL_DEBUG_TOOL_INTERFACE)
DSL_int_t DSL_CPE_DEV_DeviceOpen(DSL_char_t *pDevName, DSL_uint32_t dev_num)
{
   DSL_char_t text[30];

   if (pDevName == DSL_NULL)
   {
      return -1;
   }

   sprintf(text, "%s", pDevName);

   return open(text, O_RDWR);
}
#endif /* (defined(INCLUDE_DSL_CPE_CLI_SUPPORT) && !defined(DSL_CPE_DEBUG_DISABLE)) ||
          defined(DSL_DEBUG_TOOL_INTERFACE)*/

#endif /* #if defined (INCLUDE_DSL_CPE_API_DANUBE)*/
