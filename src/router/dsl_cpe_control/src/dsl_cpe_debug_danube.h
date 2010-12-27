/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DSL_CPE_DEBUG_DANUBE_H
#define _DSL_CPE_DEBUG_DANUBE_H

#define DSL_CPE_IFX_LOW_DEV "/dev/ifx_mei"

typedef struct
{
   /* device file descriptor */
   DSL_int_t arrDeviceFd;
} DSL_DANUBE_TcpDebugInfo_t;

typedef struct
{
  DSL_uint32_t nAddress;
  DSL_uint32_t nReadCount;
  DSL_uint32_t buffer[20];
} DSL_CPE_DANUBE_ReadDebug_Parameter_t;

typedef struct
{
  DSL_uint32_t nAddress;
  DSL_uint32_t nData;
} DSL_CPE_DANUBE_Parameter_t;

typedef struct {
   DSL_uint16_t nFunction;
   DSL_uint16_t nGroup;
   DSL_uint16_t nAddress;
   DSL_uint16_t nIndex;
   DSL_uint16_t payload[12];
} DSL_CPE_DANUBE_Message_t;

#define DSL_CPE_BYTE_SIZE   (1 << 14)
#define DSL_CPE_DWORD_SIZE  (2 << 14)
#define DSL_CPE_WORD_SIZE   (0 << 14)

#define DSL_CPE_MSG_RECV_LENGTH (32)

#define DSL_CPE_DCE_COMMAND (0x1000)

#define DSL_CPE_MP_FLAG_REPLY (1)

/* Host-to-DCE */
#define H2DCE_DEBUG_RESET        (0x02)
#define H2DCE_DEBUG_REBOOT       (0x04)
#define H2DCE_DEBUG_READ_MEI        (0x06)
#define H2DCE_DEBUG_WRITE_MEI       (0x08)
#define H2DCE_DEBUG_DOWNLOAD        (0x0A)
#define H2DCE_DEBUG_RUN          (0x0C)
#define H2DCE_DEBUG_HALT         (0x0E)
#define H2DCE_DEBUG_REMOTE       (0x10)
#define H2DCE_DEBUG_READBUF         (0x12)
#define H2DCE_DEBUG_READDEBUG       (0x14)
#define H2DCE_DEBUG_DESCRIPTORSET      (0x16)
#define H2DCE_DEBUG_DESCRIPTORSTAT     (0x18)
#define H2DCE_DEBUG_DESCRIPTORLOG      (0x1A)
#define H2DCE_DEBUG_DESCRIPTORCLEAR    (0x1C)
#define H2DCE_DEBUG_DESCRIPTORREAD     (0x1E)

/* DCE-to-Host */
#define DCE2H_DEBUG_RESET_ACK       (0x03)
#define DCE2H_DEBUG_REBOOT_ACK         (0x05)
#define DCE2H_DEBUG_READ_MEI_REPLY     (0x07)
#define DCE2H_DEBUG_WRITE_MEI_REPLY    (0x09)
#define DCE2H_ERROR_OPCODE_UNKNOWN     (0x0B)
#define DCE2H_ERROR_ADDR_UNKNOWN       (0x0D)
#define D2DCE_DEBUG_READBUF_ACK        (0x13)
#define H2DCE_DEBUG_READDEBUG_ACK     (0x15)

#define DCE2H_DEBUG_HALT_ACK           (0x0F)
#define DCE2H_ERROR_DMA                (0x34)

/********************************************************************************************************
 * DSL CPE API Driver Stack Interface Definitions
 * *****************************************************************************************************/
/** IOCTL codes for bsp driver */
#define DSL_IOC_MEI_BSP_MAGIC           's'

#define  NEW_MP_PAYLOAD_SIZE  12

#define DSL_FIO_BSP_DSL_START           _IO  (DSL_IOC_MEI_BSP_MAGIC, 0)
#define DSL_FIO_BSP_RUN                 _IO  (DSL_IOC_MEI_BSP_MAGIC, 1)
#define DSL_FIO_BSP_FREE_RESOURCE       _IO  (DSL_IOC_MEI_BSP_MAGIC, 2)
#define DSL_FIO_BSP_RESET               _IO  (DSL_IOC_MEI_BSP_MAGIC, 3)
#define DSL_FIO_BSP_REBOOT              _IO  (DSL_IOC_MEI_BSP_MAGIC, 4)
#define DSL_FIO_BSP_HALT                _IO  (DSL_IOC_MEI_BSP_MAGIC, 5)
#define DSL_FIO_BSP_BOOTDOWNLOAD        _IO  (DSL_IOC_MEI_BSP_MAGIC, 6)
#define DSL_FIO_BSP_JTAG_ENABLE         _IO  (DSL_IOC_MEI_BSP_MAGIC, 7)
#define DSL_FIO_FREE_RESOURCE           _IO  (DSL_IOC_MEI_BSP_MAGIC, 8)
#define DSL_FIO_ARC_MUX_TEST            _IO  (DSL_IOC_MEI_BSP_MAGIC, 9)
#define DSL_FIO_BSP_REMOTE              _IOW (DSL_IOC_MEI_BSP_MAGIC, 10, DSL_uint32_t)
#define DSL_FIO_BSP_GET_BASE_ADDRESS    _IOR (DSL_IOC_MEI_BSP_MAGIC, 11, DSL_uint32_t)
/*#define DSL_FIO_BSP_IS_MODEM_READY      _IOR (DSL_IOC_MEI_BSP_MAGIC, 12, u32)
#define DSL_FIO_BSP_GET_VERSION         _IOR (DSL_IOC_MEI_BSP_MAGIC, 13, DSL_DEV_Version_t)*/
#define DSL_FIO_BSP_CMV_WINHOST         _IOWR(DSL_IOC_MEI_BSP_MAGIC, 14, \
                                          DSL_CPE_DANUBE_Message_t)
#define DSL_FIO_BSP_CMV_READ            _IOWR(DSL_IOC_MEI_BSP_MAGIC, 15, \
                                          DSL_CPE_DANUBE_Parameter_t)
#define DSL_FIO_BSP_CMV_WRITE           _IOW (DSL_IOC_MEI_BSP_MAGIC, 16, \
                                          DSL_CPE_DANUBE_Parameter_t)
#define DSL_FIO_BSP_DEBUG_READ          _IOWR(DSL_IOC_MEI_BSP_MAGIC, 17, \
                                          DSL_CPE_DANUBE_ReadDebug_Parameter_t)
#define DSL_FIO_BSP_DEBUG_WRITE         _IOWR(DSL_IOC_MEI_BSP_MAGIC, 18, \
                                          DSL_CPE_DANUBE_ReadDebug_Parameter_t)

#if (defined(INCLUDE_DSL_CPE_CLI_SUPPORT) && !defined(DSL_CPE_DEBUG_DISABLE)) || defined(DSL_DEBUG_TOOL_INTERFACE)
DSL_int_t DSL_CPE_DEV_DeviceOpen(DSL_char_t *pDevName, DSL_uint32_t dev_num);
#endif /* #if (defined(INCLUDE_DSL_CPE_CLI_SUPPORT) && !defined(DSL_CPE_DEBUG_DISABLE)) ||
               defined(DSL_DEBUG_TOOL_INTERFACE)*/

#endif /* _DSL_CPE_DEBUG_DANUBE_H */
