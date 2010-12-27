/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/*
Includes
*/
#include "dsl_cpe_control.h"
#include "dsl_cpe_debug.h"
#include "drv_dsl_cpe_api_ioctl.h"
#if defined (INCLUDE_DSL_CPE_API_DANUBE)
   #include "drv_dsl_cpe_cmv_danube.h"
#endif

#ifndef DSL_CPE_DEBUG_DISABLE

/* Default initialization for all debug blocks */
DSL_CCA_debugLevelEntry_t DSL_CCA_g_dbgLvl[DSL_CCA_DBG_MAX_ENTRIES] =
{
   { DSL_CCA_DBG_NONE,   "DSL_CCA_DBG_NO_BLOCK" },   /* 00 */
   { DSL_CCA_DBG_WRN,    "DSL_CCA_DBG_APP"      },   /* 01 */
   { DSL_CCA_DBG_ERR,    "DSL_CCA_DBG_OS"       },   /* 02 */
   { DSL_CCA_DBG_WRN,    "DSL_CCA_DBG_CLI"      },   /* 03 */
   { DSL_CCA_DBG_ERR,    "DSL_CCA_DBG_PIPE"     },   /* 04 */
   { DSL_CCA_DBG_ERR,    "DSL_CCA_DBG_SOAP"     },   /* 05 */
   { DSL_CCA_DBG_ERR,    "DSL_CCA_DBG_CONSOLE"  },   /* 06 */
   { DSL_CCA_DBG_ERR,    "DSL_CCA_DBG_TCPMSG"   },   /* 07 */
   { DSL_CCA_DBG_NONE,   "not used"             },   /* 08 */
};

#ifndef _lint
/* Initialization for all debug levels */
DSL_CCA_debugLevelEntry_t DSL_CCA_g_dbgLvlNames[] =
{
   { DSL_CCA_DBG_NONE,   "DSL_CCA_DBG_NONE"  },
   { DSL_CCA_DBG_PRN,    "DSL_CCA_DBG_PRN"   },
   { DSL_CCA_DBG_ERR,    "DSL_CCA_DBG_ERR"   },
   { DSL_CCA_DBG_WRN,    "DSL_CCA_DBG_WRN"   },
   { DSL_CCA_DBG_MSG,    "DSL_CCA_DBG_MSG"   },
   { DSL_CCA_DBG_LOCAL,  "DSL_CCA_DBG_LOCAL" },
};

const DSL_uint8_t DSL_CCA_g_dbgLvlNumber = sizeof(DSL_CCA_g_dbgLvlNames) / sizeof(DSL_CCA_debugLevelEntry_t);
#endif /* _lint*/

DSL_CCA_debugLevels_t DSL_CCA_g_globalDbgLvl = DSL_CCA_DBG_LOCAL;

#endif /* DSL_CPE_DEBUG_DISABLE */


#if defined (INCLUDE_DSL_CPE_API_DANUBE)

#define MEI_SPACE_ACCESS   0xBE116000

static DSL_CmvGroupEntry_t DSL_CmvGroups[] =
{
   { "optn", DSL_CMV_GROUP_OPTN },
   { "cnfg", DSL_CMV_GROUP_CNFG },
   { "cntl", DSL_CMV_GROUP_CNTL },
   { "stat", DSL_CMV_GROUP_STAT },
   { "rate", DSL_CMV_GROUP_RATE },
   { "plam", DSL_CMV_GROUP_PLAM },
   { "info", DSL_CMV_GROUP_INFO },
   { "test", DSL_CMV_GROUP_TEST },
   { "dsl",  DSL_CMV_GROUP_DSL  },
   { "",     0                  }
};
#endif /* #if defined (INCLUDE_DSL_CPE_API_DANUBE)*/

DSL_Error_t DSL_strlwr(DSL_char_t *psStr)
{
   DSL_char_t *pChar = DSL_NULL;

   for(pChar = psStr; *pChar != '\0'; pChar++)
   {
      if ((*pChar >= 0x41) && (*pChar <= 0x5A))
      {
         *pChar += 0x20;
      }
   }
   return DSL_SUCCESS;
}

#if defined (INCLUDE_DSL_CPE_API_DANUBE)
void DSL_CMV_Prepare (
   DSL_uint8_t opcode,
   DSL_uint8_t group,
   DSL_uint16_t address,
   DSL_uint16_t index,
   DSL_int_t size,
   DSL_uint16_t *data,
   DSL_uint16_t *Message)
{
   memset (Message, 0, DSL_MAX_CMV_MSG_LENGTH * 2);
   Message[0] = (opcode << 4) + (size & 0xf);
   if (opcode == DSL_CMV_OPCODE_H2D_DEBUG_WRITE_DM)
   {
      Message[1] = (group & 0x7f);
   }
   else
   {
      Message[1] = (((index == 0) ? 0 : 1) << 7) + (group & 0x7f);
   }
   Message[2] = address;
   Message[3] = index;

   if ((opcode == DSL_CMV_OPCODE_H2D_CMV_WRITE) ||
       (opcode == DSL_CMV_OPCODE_H2D_DEBUG_WRITE_DM))
   {
      memcpy (&(Message[4]), data, (DSL_uint32_t)(size * 2));
   }

   return;
}
#endif /* #if defined (INCLUDE_DSL_CPE_API_DANUBE)*/

DSL_Error_t DSL_CMV_Read (
   DSL_CPE_Control_Context_t * pContext,
   DSL_char_t *str_group,
   DSL_uint16_t address,
   DSL_uint16_t index,
   DSL_int_t size,
   DSL_uint16_t *pData)
{
#if defined (INCLUDE_DSL_CPE_API_DANUBE)
   DSL_uint8_t group = 0;
   DSL_boolean_t bFound = DSL_FALSE;
   DSL_CmvGroupEntry_t *pGroups = &DSL_CmvGroups[0];
   DSL_uint16_t Message[DSL_MAX_CMV_MSG_LENGTH];
   DSL_DeviceMessage_t msg;

   memset(&msg, 0, sizeof(DSL_DeviceMessage_t));

   DSL_strlwr(str_group);

   for (;(pGroups->nGroupId != 0) && (strlen(pGroups->psGroupName) > 0); pGroups++)
   {
      if (strcmp (str_group, pGroups->psGroupName) == 0)
      {
         group = (DSL_uint8_t)(pGroups->nGroupId);
         bFound = DSL_TRUE;
         break;
      }
   }

   if (bFound == DSL_FALSE)
   {
      DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "wrong group type!" DSL_CPE_CRLF);
      return DSL_ERR_INVALID_PARAMETER;
   }

   DSL_CMV_Prepare (DSL_CMV_OPCODE_H2D_CMV_READ, group, address, index, size, DSL_NULL, Message);
   msg.data.pMsg = (DSL_uint8_t *)Message;
   msg.data.nSizeTx = 8;
   msg.data.nSizeRx = (DSL_uint16_t)((size + DSL_CMV_HEADER_LENGTH) * 2);
   if (DSL_CPE_Ioctl(pContext->fd[pContext->nDevNum], DSL_FIO_DBG_DEVICE_MESSAGE_SEND, (int)&msg) < 0)
   {
      DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "cr read %d %d %d fail", group, address, index);
      return DSL_ERROR;
   }

   memcpy(pData, &(Message[4]), (DSL_uint32_t)(size * 2));

   return msg.accessCtl.nReturn;
#else
   return DSL_ERR_NOT_IMPLEMENTED;
#endif /* #if defined (INCLUDE_DSL_CPE_API_DANUBE)*/
}


DSL_Error_t DSL_CMV_Write (
   DSL_CPE_Control_Context_t *pContext,
   DSL_char_t *str_group,
   DSL_uint16_t address,
   DSL_uint16_t index,
   DSL_int_t size,
   DSL_uint16_t *pData)
{
#if defined (INCLUDE_DSL_CPE_API_DANUBE)
   DSL_uint8_t group = 0;
   DSL_boolean_t bFound = DSL_FALSE;
   DSL_CmvGroupEntry_t *pGroups = &DSL_CmvGroups[0];
   DSL_uint16_t Message[DSL_MAX_CMV_MSG_LENGTH];
   DSL_DeviceMessage_t msg;

   memset(&msg, 0, sizeof(DSL_DeviceMessage_t));

   DSL_strlwr(str_group);

   for (;(pGroups->nGroupId != 0) && (strlen(pGroups->psGroupName) > 0); pGroups++)
   {
      if (strcmp (str_group, pGroups->psGroupName) == 0)
      {
         group = (DSL_uint8_t)pGroups->nGroupId;
         bFound = DSL_TRUE;
         break;
      }
   }

   if (bFound == DSL_FALSE)
   {
      DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "wrong group type!" DSL_CPE_CRLF);
      return DSL_ERR_INVALID_PARAMETER;
   }

   DSL_CMV_Prepare (DSL_CMV_OPCODE_H2D_CMV_WRITE, group, address, index, size,
      pData, Message);

   msg.data.pMsg = (DSL_uint8_t *)Message;
   msg.data.nSizeTx = (DSL_uint16_t)((size + DSL_CMV_HEADER_LENGTH) * 2);
   msg.data.nSizeRx = 0;
   if (DSL_CPE_Ioctl(pContext->fd[pContext->nDevNum], DSL_FIO_DBG_DEVICE_MESSAGE_SEND, (int)&msg) < 0)
   {
      DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "cw %d %d %d failed!", group,
         address, index);
      return DSL_ERR_MSG_EXCHANGE;
   }

   return msg.accessCtl.nReturn;
#else
   return DSL_ERR_NOT_IMPLEMENTED;
#endif /* #if defined (INCLUDE_DSL_CPE_API_DANUBE)*/
}

#ifdef DSL_DEBUG_TOOL_INTERFACE

#ifndef DSL_CPE_IFX_LOW_DEV
#error "Please define low level device name!"
#endif

#undef DSL_CCA_DBG_BLOCK
#define DSL_CCA_DBG_BLOCK DSL_CCA_DBG_TCPMSG

#ifdef INCLUDE_DSL_RESOURCE_STATISTICS
DSL_Error_t DSL_CPE_TcpDebugMessageResourceUsageGet (
   DSL_CPE_Control_Context_t * pContext,
   DSL_CPE_TcpDebugResourceUsageStatisticsData_t *pResUsage)
{
   DSL_Error_t nRet = DSL_SUCCESS;
   DSL_CPE_TcpDebugClientInfo_t *clientInfo = DSL_NULL;
#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   DSL_CPE_TcpDebugCliClientInfo_t *CliClientInfo = DSL_NULL;
   DSL_int_t c = 0;
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT*/
   DSL_uint32_t devStaticMemUsage = 0, devDynamicMemUsage = 0;

   if (pContext == DSL_NULL ||
       pResUsage == DSL_NULL)
   {
      return DSL_ERROR;
   }

   pResUsage->staticMemUsage  = 0;
   pResUsage->dynamicMemUsage = 0;

   if (pContext->pDebugClientInfo != DSL_NULL)
   {
      pResUsage->dynamicMemUsage += DSL_FD_SETSIZE * sizeof(DSL_CPE_TcpDebugClientInfo_t);

      clientInfo = (DSL_CPE_TcpDebugClientInfo_t*)pContext->pDebugClientInfo;

      nRet = DSL_CPE_DEV_TcpDebugMessageResourceUsageGet(
                clientInfo, &devStaticMemUsage, &devDynamicMemUsage);
      if (nRet != DSL_SUCCESS)
      {
         return nRet;
      }

      pResUsage->staticMemUsage  += devStaticMemUsage;
      pResUsage->dynamicMemUsage += devDynamicMemUsage;
   }

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   if (pContext->pDebugCliClientInfo != DSL_NULL)
   {
      CliClientInfo = (DSL_CPE_TcpDebugCliClientInfo_t*)pContext->pDebugCliClientInfo;

      pResUsage->dynamicMemUsage += DSL_FD_SETSIZE * sizeof(DSL_CPE_TcpDebugCliClientInfo_t);

      for (c = 0; c < DSL_FD_SETSIZE; c++)
      {
         if (CliClientInfo[c].buf != DSL_NULL)
         {
            pResUsage->dynamicMemUsage += DSL_CPE_TCP_CLI_COMMAND_LENGTH_MAX;
         }
      }
   }
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT*/

   return DSL_SUCCESS;
}
#endif /* INCLUDE_DSL_RESOURCE_STATISTICS*/

/* Tcp messages debug handler */
DSL_int_t DSL_CPE_TcpDebugMessageHandle (DSL_CPE_Thread_Params_t *params )
{
   DSL_int_t sockfd, nRet;
   DSL_int_t width;
   DSL_CPE_fd_set_t readFds,tempFds;
   DSL_Socket_t socketaccept;
   DSL_sockaddr_in_t my_addr;
   /* client address*/
   DSL_sockaddr_in_t remoteaddr;
#ifdef LINUX
   DSL_int_t so_reuseaddr=1;
   DSL_int_t so_keepalive =1 ;
   DSL_int_t so_debug =1;
   DSL_TimeVal_t tv_sendtimeout;
   DSL_SockOptLinger_t struct_lg;
#endif /* LINUX*/
   DSL_int_t c;
   /*number of requests*/
   DSL_int_t n=0;
   /* stores the client information */
   DSL_CPE_TcpDebugClientInfo_t *clientInfo;
   DSL_int_t nClients=0;
   DSL_CPE_Control_Context_t *pCtrlContext = (DSL_CPE_Control_Context_t*)params->nArg1;

   /**$$ND FIXME: all linux and BSD sockets calls should be renamed to DSL_xxx() */

   /* Socket Address */
   memset((char *) &my_addr, '\0', sizeof(DSL_sockaddr_in_t));
   my_addr.sin_family = AF_INET;
   my_addr.sin_port   = DSL_CPE_Htons(DSL_CPE_TCP_MESSAGES_PORT); /* Listens at port 2000 */

   if (sTcpMessagesSocketAddr)
   {
      if (DSL_CPE_StringToAddress(sTcpMessagesSocketAddr, &my_addr.sin_addr) == 0)
      {
         DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "ERROR - invalid IP address "
            "specified" DSL_CPE_CRLF);
         return -1;
      }

      DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "TCP messages debug server: "
         "Address: %s" DSL_CPE_CRLF, DSL_CPE_AddressToString(my_addr.sin_addr));
   }
   else
   {
      DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "Using multihomed host "
         "for TCP messages..."DSL_CPE_CRLF);
      my_addr.sin_addr.s_addr = DSL_CPE_Htonl(DSL_CPE_INADDR_ANY);
   }

   /* Initialise file descriptor to zero */
   DSL_CPE_FD_ZERO(&readFds);
   DSL_CPE_FD_ZERO(&tempFds);

   /* Set up socket parameters */
   if ((DSL_CPE_Socket(SOCK_STREAM, (DSL_Socket_t*)&sockfd)) == -1)
   {
      DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "ERROR - Socket was "
         "not created" DSL_CPE_CRLF);
      return 0;
   }

#ifdef LINUX
   /*$$ ND: I'm not sure whether all of options below are supported
        by another OS than Linux */

   /* set the socket to be reuseable */
   DSL_CPE_SockOptSet(sockfd, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr,
      sizeof(so_reuseaddr));

   /* set up the struct timeval for the timeout If connected socket fails
      to respond to these messages, the connection is broken and processes
      writing to that socket are notified with an ENETRSET errno. This option
      takes an int value in the optval argument. This is a BOOL option */
   DSL_CPE_SockOptSet(sockfd, SOL_SOCKET, SO_KEEPALIVE, &so_keepalive,
      sizeof(so_keepalive) );

   /* SO_DEBUG */
   /* Enables recording of debugging information */
   DSL_CPE_SockOptSet(sockfd, SOL_SOCKET, SO_DEBUG, &so_debug, sizeof(so_debug) );

   /* SO_LINGER linger on close if data present SO_LINGER controls the action
      taken when unsent messages are queued on socket and a close(2)
      is performed */
   struct_lg.l_onoff=0;
   struct_lg.l_linger=0;
   DSL_CPE_SockOptSet(sockfd, SOL_SOCKET, SO_LINGER, &struct_lg,
      sizeof(DSL_SockOptLinger_t) );

   /* SO_RCVTIME0 and SO_SNDTIME0 Specify the sending or receiving timeouts
      until reporting an error. They are fixed to a protocol specific setting
      in Linux adn cannot be read or written They can be easily emulated
      using alarm */
   tv_sendtimeout.tv_sec=60;
   tv_sendtimeout.tv_usec=0;
   DSL_CPE_SockOptSet(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv_sendtimeout, sizeof(DSL_TimeVal_t) );
#endif /* LINUX*/
#if 0
   if (jtag_debug_enable)
   {
      if(ioctl(nLowFd, IFX_ADSL_BSP_IOC_JTAG_ENABLE)!=MEI_SUCCESS)
      {
         printf("\n IFX_ADSL_BSP_IOC_JTAG_ENABLE failed");
         close(nLowFd);
         return -1;
      }
   }
#endif

   clientInfo = DSL_CPE_Malloc(DSL_FD_SETSIZE * sizeof(DSL_CPE_TcpDebugClientInfo_t));
   if (clientInfo == DSL_NULL)
   {
      return -1;
   }

   for (c = 0; c < DSL_FD_SETSIZE; c++)
   {
      clientInfo[c].fd = -1;
      clientInfo[c].pDevData = DSL_NULL;
   }

   /* Set info in the Control Context*/
   pCtrlContext->pDebugClientInfo = clientInfo;

   if (DSL_CPE_SocketBind(sockfd, &my_addr) == -1)
   {
      DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "ERROR - Could not "
         "bind to socket!" DSL_CPE_CRLF);
   }
   else
   {
      if (listen(sockfd, 1) == -1)
      {
         DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "Error: Listening Error" DSL_CPE_CRLF);
      }
      else
      {
         DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "TCP_IP Socket is Listening" DSL_CPE_CRLF);

         c = 0;

#if 0
         if (firmare_flag)
         {
            nRet = Download_Firmware(filename,nLowFd);
            nRet=ioctl(nLowFd, IFX_ADSL_BSP_IOC_DSLSTART, NULL);
         }

         if (ioctl(nLowFd, AUTOBOOT_CONTROL_SET , 0)<0)
         {
            printf("\n\n mei ioctl AUTOBOOT_CONTROL_SET fail.\n");
         }
#endif

         DSL_CPE_FD_SET(sockfd, &readFds);

         width = sockfd + 1;
         /* Listen to the selected port */
         while(1)
         {
            /* Update the local file descriptor by the copy in the task parameter  */
            memcpy(&tempFds,&readFds,sizeof(DSL_CPE_fd_set_t));

            /* Wait for incoming events */
            /*n=select(width, &tempFds,(fd_set *)NULL, (fd_set *)NULL, &tv);*/
            n = DSL_CPE_Select(width, &tempFds, &tempFds, (DSL_uint32_t)-1);

            /* Look if messages were received */
            /* New connection */
            if (DSL_CPE_FD_ISSET(sockfd,&tempFds))
            {
               /* Received socket stream Accept connection from socket*/
               socketaccept = DSL_CPE_Accept(sockfd, &remoteaddr);

               DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "Connected from %s, sock = %d."DSL_CPE_CRLF,
                  DSL_CPE_AddressToString(remoteaddr.sin_addr), socketaccept);

               /* store information to clientInfo table */
               for (c = 0; c < FD_SETSIZE; c++)
               {
                   if (nClients >= FD_SETSIZE)
                  {
                     DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "Error: "
                        "Too many clients."DSL_CPE_CRLF);
                    DSL_CPE_SocketClose(socketaccept);
                        continue;
                  }

                   if (clientInfo[c].fd < 0)
                   {
                     /* try to create FILE structure for socket */
                     clientInfo[c].fd = (DSL_int_t)socketaccept;
                     clientInfo[c].client_addr = remoteaddr;
                     nClients++;

                     /* update fd to readFds */
                     if ((DSL_int_t)socketaccept >= width)
                        width = (DSL_int_t)socketaccept + 1;
                     DSL_CPE_FD_SET((DSL_int_t)socketaccept,&readFds);
                     n--;

                     break;
                 }
               }
            }

            /* check fd for all clients  */
            for (c = 0; c < FD_SETSIZE; c++)
            {
               if (n==0)
               {
                  break;
               }

               if ((socketaccept = (DSL_Socket_t)clientInfo[c].fd) < 0)
               {
                  continue;
               }


               /* Check for receive time-out */
               if (FD_ISSET((DSL_int_t)socketaccept ,&tempFds))
               {
                 n--;
                 nRet = DSL_CPE_DEV_TcpMessageHandle(&clientInfo[c]);
                  if (nRet == -ENODATA) /* read 0 bytes, client closed the connection */
                  {
                     DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "Warning: "
                        "Connection closed from %s."DSL_CPE_CRLF,
                        DSL_CPE_AddressToString(clientInfo[c].client_addr.sin_addr));

                    DSL_CPE_SocketClose(socketaccept);
                    DSL_CPE_FD_CLR((DSL_int_t)socketaccept,&readFds);
                    clientInfo[c].fd = -1;

                     if (clientInfo[c].pDevData)
                     {
                        DSL_CPE_DEV_DeviceDataFree(clientInfo[c].pDevData);
                        clientInfo[c].pDevData = DSL_NULL;
                     }

                     nClients--;
                  }
               }
            }
         }
      }
   }

   /* Free device data*/
   for (c = 0; c < DSL_FD_SETSIZE; c++)
   {
      if (clientInfo[c].pDevData)
      {
         DSL_CPE_DEV_DeviceDataFree(clientInfo[c].pDevData);
      }
   }

   DSL_CPE_Free(clientInfo);

   return 0;
}

/*
   Start TCP message debug interface thread
*/
DSL_Error_t DSL_CPE_TcpDebugMessageIntfStart (
   DSL_CPE_Control_Context_t * pContext)
{

   if (DSL_CPE_ThreadInit (&pContext->nTcpMsgHandler, "tcpmsg", DSL_CPE_TcpDebugMessageHandle,
            DSL_CPE_TCP_MSG_STACKSIZE, DSL_CPE_TCP_MSG_PRIORITY, (DSL_uint32_t) pContext, 0) == 0)
   {
      /* return the task/process id */
      return DSL_SUCCESS;
   }

   DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "ERROR - TCP message debug interface"
      " thread start error."DSL_CPE_CRLF);

   return DSL_ERROR;
}

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
DSL_int_t DSL_CPE_TcpCliHandle(
   DSL_CPE_Control_Context_t *pCtx,
   DSL_CPE_TcpDebugCliClientInfo_t *pClient,
   DSL_int_t socketaccept)
{
   DSL_int_t nBytesReceived;
   DSL_char_t nCurrChar = 0;
   DSL_char_t *pArg;

   /* receive command line */
   nBytesReceived = DSL_CPE_SocketRecv(socketaccept, (DSL_char_t *)&nCurrChar,
      sizeof(nCurrChar));

   /* Error or termination from host */
   if(nBytesReceived == 0)
   {
      return -ENODATA;
   }

   /* TODO: Get system last error for analysis*/
   if(nBytesReceived < 0)
   {
      return 0;
   }

   if (nCurrChar == '\r')
   {
      return 0;
   }

   if (pClient->pPos == DSL_NULL)
   {
      pClient->pPos = pClient->buf;
   }

   if ((nCurrChar == '\n') || (pClient->pPos >= pClient->buf +
      DSL_CPE_TCP_CLI_COMMAND_LENGTH_MAX - 1))
   {
      if (pClient->pPos == pClient->buf)
      {
         return 0;
      }

      if (pClient->pPos > pClient->buf + DSL_CPE_TCP_CLI_COMMAND_LENGTH_MAX - 1)
      {
         pClient->pPos = 0;
         return -EFAULT;
      }
      else
      {
         *(pClient->pPos) = '\0';
         pArg = strstr(pClient->buf, " ");
         if (pArg == DSL_NULL)
         {
            pArg = pClient->pPos;
         }
         else
         {
            *pArg = '\0';
            pArg++;
         }

         DSL_CPE_CliDeviceCommandExecute(pCtx, -1, pClient->buf, pArg, pClient->out);
         pClient->pPos = 0;
      }
   }
   else
   {
      *(pClient->pPos) = nCurrChar;
      (pClient->pPos)++;
   }

   return 0;
}

/* TCP CLI debug handler */
DSL_int_t DSL_CPE_TcpDebugCliHandle ( DSL_CPE_Thread_Params_t *params )
{
   DSL_int_t nSockFd, nRet;
   DSL_int_t nWidth;
   DSL_CPE_fd_set_t readFds, tempFds;
   DSL_Socket_t nSocketAccept;
   DSL_sockaddr_in_t myAddr;
   /* client address*/
   DSL_sockaddr_in_t remoteAddr;
#ifdef LINUX
   DSL_int_t nSockOpt = 1;
   DSL_TimeVal_t sendTimeout;
   DSL_SockOptLinger_t structLg;
#endif
   DSL_int_t c;
   /*number of requests*/
   DSL_int_t n = 0;
   /* stores the client information */
   DSL_CPE_TcpDebugCliClientInfo_t *clientInfo;
   DSL_int_t nClients = 0;
   DSL_CPE_Control_Context_t *pCtrlContext = (DSL_CPE_Control_Context_t*)params->nArg1;

   clientInfo = DSL_CPE_Malloc(DSL_FD_SETSIZE * sizeof(DSL_CPE_TcpDebugCliClientInfo_t));
   if (clientInfo == DSL_NULL)
   {
      return -1;
   }

   /* Socket Address */
   memset((char *) &myAddr, '\0', sizeof(DSL_sockaddr_in_t));
   myAddr.sin_family = AF_INET;
   myAddr.sin_port = DSL_CPE_Htons(DSL_CPE_TCP_CLI_PORT); /* Listens at port 2001 */

   if (sTcpMessagesSocketAddr)
   {
      if (DSL_CPE_StringToAddress(sTcpMessagesSocketAddr, &myAddr.sin_addr) == 0)
      {
         DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "Error: invalid IP address "
            "specified" DSL_CPE_CRLF);
         return -1;
      }

      DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "TCP CLI debug server: "
         "Address: %s" DSL_CPE_CRLF, DSL_CPE_AddressToString(myAddr.sin_addr));
   }
   else
   {
      DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "Using multihomed host "
         "for TCP messages..."DSL_CPE_CRLF);
      myAddr.sin_addr.s_addr = DSL_CPE_Htonl(DSL_CPE_INADDR_ANY);
   }

   /* Initialise file descriptors to zero */
   DSL_CPE_FD_ZERO(&readFds);
   DSL_CPE_FD_ZERO(&tempFds);

   /* Set up socket parameters */
   if ((DSL_CPE_Socket(SOCK_STREAM, (DSL_Socket_t*)&nSockFd)) == -1)
   {
      DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "Error: Socket was "
         "not created" DSL_CPE_CRLF);
      return 0;
   }

#ifdef LINUX
   /*$$ ND: I'm not sure whether all of options below are supported
        by another OS than Linux */

   /* set the socket to be reuseable */
   DSL_CPE_SockOptSet(nSockFd, SOL_SOCKET, SO_REUSEADDR, &nSockOpt,
      sizeof(nSockOpt));

   /* set up the struct timeval for the timeout If connected socket fails
      to respond to these messages, the connection is broken and processes
      writing to that socket are notified with an ENETRSET errno. This option
      takes an int value in the optval argument. This is a BOOL option */
   nSockOpt = 1;
   DSL_CPE_SockOptSet(nSockFd, SOL_SOCKET, SO_KEEPALIVE, &nSockOpt,
      sizeof(nSockOpt) );

   /* SO_DEBUG */
   /* Enables recording of debugging information */
   nSockOpt = 1;
   DSL_CPE_SockOptSet(nSockFd, SOL_SOCKET, SO_DEBUG, &nSockOpt, sizeof(nSockOpt) );

   /* SO_LINGER linger on close if data present SO_LINGER controls the action
      taken when unsent messages are queued on socket and a close(2)
      is performed */
   structLg.l_onoff=0;
   structLg.l_linger=0;
   DSL_CPE_SockOptSet(nSockFd, SOL_SOCKET, SO_LINGER, &structLg,
      sizeof(DSL_SockOptLinger_t) );

   /* SO_RCVTIME0 and SO_SNDTIME0 Specify the sending or receiving timeouts
      until reporting an error. They are fixed to a protocol specific setting
      in Linux adn cannot be read or written They can be easily emulated
      using alarm */
   sendTimeout.tv_sec=60;
   sendTimeout.tv_usec=0;
   DSL_CPE_SockOptSet(nSockFd, SOL_SOCKET, SO_SNDTIMEO, &sendTimeout,
      sizeof(DSL_TimeVal_t) );
#endif

   for (c = 0; c < DSL_FD_SETSIZE; c++)
   {
      clientInfo[c].fd = -1;
      clientInfo[c].buf = 0;
   }

   /* Set info in the Control Context*/
   pCtrlContext->pDebugCliClientInfo = clientInfo;

   if (DSL_CPE_SocketBind(nSockFd, &myAddr) == -1)
   {
      DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "Error: Could not "
         "bind to socket open failed" DSL_CPE_CRLF);
   }
   else
   {
      if (listen(nSockFd, 1) == -1)
      {
         DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "Error: Listening Error" DSL_CPE_CRLF);
      }
      else
      {
         DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "TCP_IP Socket for CLI is Listening" DSL_CPE_CRLF);

         c = 0;

         DSL_CPE_FD_SET(nSockFd, &readFds);

         nWidth = nSockFd + 1;
         /* Listen to the selected port */
         while(1)
         {
            /* Update the local file descriptor by the copy in the task parameter  */
            memcpy(&tempFds, &readFds, sizeof(DSL_CPE_fd_set_t));

            /* Wait for incoming events */
            n = DSL_CPE_Select(nWidth, &tempFds, &tempFds, (DSL_uint32_t)-1);

            /* Look if messages were received */
            /* New connection */
            if (DSL_CPE_FD_ISSET(nSockFd,&tempFds))
            {
               /* Received socket stream Accept connection from socket*/
               nSocketAccept = DSL_CPE_Accept(nSockFd, &remoteAddr);

               DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "Connected from %s."DSL_CPE_CRLF,
                  DSL_CPE_AddressToString(remoteAddr.sin_addr));

               /* store information to clientInfo table */
               for (c = 0; c < DSL_FD_SETSIZE; c++)
               {
                   if (nClients >= DSL_FD_SETSIZE)
                  {
                     DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "Error: "
                        "Too many clients."DSL_CPE_CRLF);
                    DSL_CPE_SocketClose(nSocketAccept);
                        continue;
                  }

                   if (clientInfo[c].fd < 0)
                   {
                     clientInfo[c].out = DSL_CPE_FdOpen((DSL_int_t)nSocketAccept, "w");
                     if (clientInfo[c].out != DSL_NULL)
                     {
                        if (clientInfo[c].buf != DSL_NULL)
                        {
                           DSL_CPE_Free(clientInfo[c].buf);
                        }

                        clientInfo[c].buf =
                           DSL_CPE_Malloc(DSL_CPE_TCP_CLI_COMMAND_LENGTH_MAX);

                        if (clientInfo[c].buf == DSL_NULL)
                        {
                           DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "Error: "
                              "Could not allocate buffer for "
                              "CLI command" DSL_CPE_CRLF);
                        }
                        else
                        {
                          clientInfo[c].fd = (DSL_int_t)nSocketAccept;
                          clientInfo[c].client_addr = remoteAddr;
                          nClients++;
                          /* update fd to readFds */
                           if ((DSL_int_t)nSocketAccept >= nWidth)
                           {
                              nWidth = (DSL_int_t)nSocketAccept + 1;
                           }

                           DSL_CPE_FD_SET((DSL_int_t)nSocketAccept,&readFds);
                          n--;
                        }
                     }
                     else
                     {
                        DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "Error: "
                           "Could not create file structure for socket" DSL_CPE_CRLF);
                     }
                     break;
                 }
               }
            }

            /* check fd for all clients  */
            for (c = 0; c < DSL_FD_SETSIZE; c++)
            {
               if (n == 0)
               {
                  break;
               }

               if ((nSocketAccept = (DSL_Socket_t)clientInfo[c].fd) < 0)
               {
                 continue;
               }

               /* Check for receive time-out */
               if (DSL_CPE_FD_ISSET((DSL_int_t)nSocketAccept, &tempFds))
               {
                 n--;
                 nRet = DSL_CPE_TcpCliHandle((DSL_CPE_Control_Context_t*) params->nArg1,
                     &clientInfo[c], (DSL_int_t)nSocketAccept);

                  if (nRet == -ENODATA) /* read 0 bytes, client closed the connection */
                  {
                     DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX
                        "Connection closed from %s."DSL_CPE_CRLF,
                        DSL_CPE_AddressToString(clientInfo[c].client_addr.sin_addr));

                     if (clientInfo[c].out != DSL_NULL)
                     {
                        DSL_CPE_FClose(clientInfo[c].out);
                        clientInfo[c].out = DSL_NULL;
                        if (clientInfo[c].buf != DSL_NULL)
                        {
                           DSL_CPE_Free(clientInfo[c].buf);
                           clientInfo[c].buf = DSL_NULL;
                        }
                     }
                     else
                     {
                          DSL_CPE_SocketClose(nSocketAccept);
                     }

                     DSL_CPE_FD_CLR((DSL_int_t)nSocketAccept, &readFds);
                    clientInfo[c].fd = -1;
                    nClients--;
                  }
               }
            }
         }
      }
   }

   for (c = 0; c < DSL_FD_SETSIZE; c++)
   {
      if (clientInfo[c].buf != DSL_NULL)
      {
         DSL_CPE_Free(clientInfo[c].buf);
      }
   }

   return 0;
}

/*
   Start TCP CLI debug interface thread
*/
DSL_Error_t DSL_CPE_TcpDebugCliIntfStart (
   DSL_CPE_Control_Context_t * pContext)
{
   if (DSL_CPE_ThreadInit (&pContext->nTcpCliHandler, "tcpcli", DSL_CPE_TcpDebugCliHandle,
          2*DSL_CPE_TCP_MSG_STACKSIZE, DSL_CPE_TCP_MSG_PRIORITY, (DSL_uint32_t) pContext, 0) == 0)
   {
      return DSL_SUCCESS;
   }

   DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "ERROR - TCP CLI debug interface"
      " thread start error."DSL_CPE_CRLF);

   return DSL_ERROR;
}
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT*/

#endif
