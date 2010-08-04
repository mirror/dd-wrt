/******************************************************************************
 *
 *  Copyright (c) 2008 Cavium Networks 
 * 
 *  This file is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License, Version 2, as 
 *  published by the Free Software Foundation. 
 *
 *  This file is distributed in the hope that it will be useful, 
 *  but AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or 
 *  NONINFRINGEMENT.  See the GNU General Public License for more details. 
 *
 *  You should have received a copy of the GNU General Public License 
 *  along with this file; if not, write to the Free Software 
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA or 
 *  visit http://www.gnu.org/licenses/. 
 *
 *  This file may also be available under a different license from Cavium. 
 *  Contact Cavium Networks for more information
 *
 ******************************************************************************/

#ifndef _STAR_PCM_H_
#define _STAR_PCM_H_

/******************************************************************************
 * MODULE NAME:    star_pcm.h
 * PROJECT CODE:   Orion
 * DESCRIPTION:    
 * MAINTAINER:     MJLIU
 * DATE:           15 September 2005
 *
 * SOURCE CONTROL: 
 *
 * LICENSE:
 *     This source code is copyright (c) 2005 Star Semi Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *     15 September 2005  -  MJLIU	- Initial Version v1.0
 *
 *
 * SOURCE:
 * ISSUES:
 * NOTES TO USERS:
 ******************************************************************************/

//#include <asm/arch/star_sys_memory_map.h>

#define PCM_BASE_ADDR                         (CNS3XXX_SSP_BASE_VIRT)
#define PCM_MEM_MAP_ADDR(reg_offset)          (PCM_BASE_ADDR + reg_offset)
#define PCM_MEM_MAP_VALUE(reg_offset)         (*((u32 volatile *)PCM_MEM_MAP_ADDR(reg_offset)))


/*
 * define access macros
 */
#define PCM_CONFIGURATION_0_REG               PCM_MEM_MAP_VALUE(0x80)
#define PCM_CONFIGURATION_1_REG               PCM_MEM_MAP_VALUE(0x84)

#define PCM_CHANNEL_0_CONFIG_REG              PCM_MEM_MAP_VALUE(0x88)
#define PCM_CHANNEL_1_CONFIG_REG              PCM_MEM_MAP_VALUE(0x8C)
#define PCM_CHANNEL_2_CONFIG_REG              PCM_MEM_MAP_VALUE(0x90)
#define PCM_CHANNEL_3_CONFIG_REG              PCM_MEM_MAP_VALUE(0x94)

#define PCM_TX_DATA_31_0_REG                  PCM_MEM_MAP_VALUE(0x98)
#define PCM_TX_DATA_63_32_REG                 PCM_MEM_MAP_VALUE(0x9C)

#define PCM_RX_DATA_31_0_REG                  PCM_MEM_MAP_VALUE(0xA0)
#define PCM_RX_DATA_63_32_REG                 PCM_MEM_MAP_VALUE(0xA4)

#define PCM_INTERRUPT_STATUS_REG              PCM_MEM_MAP_VALUE(0xA8)
#define PCM_INTERRUPT_ENABLE_REG              PCM_MEM_MAP_VALUE(0xAC)



/*
 * define constants macros
 */
#define CH0_BIT_INDEX                         (0x1)
#define CH1_BIT_INDEX                         (0x2)
#define CH2_BIT_INDEX                         (0x4)
#define CH3_BIT_INDEX                         (0x8)

#define PCM_RXBUF_FULL_FG                     (0x1)
#define PCM_TXBUF_EMPTY_FG                    (0x2)
#define PCM_RXBUF_OVERRUN_FG                  (0x4)
#define PCM_TXBUF_UNDERRUN_FG                 (0x8)

#define PCM_ENABLE_FG                         (0x1 << 23)

#define PCM_IDL_MODE                          (0)
#define PCM_GCI_MODE                          (1)

#define PCM_DATA_BIT_8                        (0)
#define PCM_DATA_BIT_16                       (1)


/*
 * Set Commands Variables
 */
#define        Software_Reset                               (0x02)
#define        Hardware_Reset                               (0x04)
#define        Write_Transmit_Time_Slot                     (0x40)
#define        Read_Transmit_Time_Slot                      (0x41)
#define        Write_Receive_Time_Slot                      (0x42)
#define        Read_Receive_Time_Slot                       (0x43)
#define        Write_Tx_Rx_CLK_Slot_Tx_CLK_Edge             (0x44)
#define        Read_Tx_Rx_CLK_Slot_Tx_CLK_Edge              (0x45)
#define        Write_Device_Configure_Reg                   (0x46)
#define        Read_Device_Configure_Reg                    (0x47)
#define        Write_Channel_Enable_Operating_Mode_Reg      (0x4A)
#define        Read_Channel_Enable_Operating_Mode_Reg       (0x4B)
#define        Read_Signal_Reg                              (0x4D)
#define        Input_Data_Reg                               (0x52)
#define        Output_Data_Reg                              (0x53)
#define        Input_Direction_Reg                          (0x54)
#define        Output_Direction_Reg                         (0x55)
#define        Write_System_State                           (0x56)
#define        Read_System_State                            (0x57)
#define        Write_Operating_Functon                      (0x60)
#define        Read_Operating_Functon                       (0x61)
#define        Write_System_State_Config                    (0x68)
#define        Read_System_State_Config                     (0x69)
#define        Write_Interrupt_Mask_Reg                     (0x6C)
#define        Read_Interrupt_Mask_Reg                      (0x6D)
#define        Write_Operating_Condition                    (0x70)
#define        Write_Loop_Supervision_Parameter             (0xC2)
#define        Write_DC_Feed_Parameter                      (0xC6)
#define        Write_Signal_A_B_Parameter                   (0xD2)
#define        Write_Switching_Reg_Parameter                (0xE4)
#define        Write_Switching_Reg_Control                  (0xE6)


/*
 * define data structure
 */
typedef struct _PCM_CHANNEL_OBJECT_    PCM_CHANNEL_OBJECT_T;

struct _PCM_CHANNEL_OBJECT_
{
    u16          channel_0_tx_data;
    u16          channel_0_rx_data;
    u32          channel_0_data_width;     /* 0 : 8-bit, 1 : 16-bit */

    u16          channel_1_tx_data;
    u16          channel_1_rx_data;
    u32          channel_1_data_width;

    u16          channel_2_tx_data;
    u16          channel_2_rx_data;
    u32          channel_2_data_width;

    u16          channel_3_tx_data;
    u16          channel_3_rx_data;
    u32          channel_3_data_width;
    
    u32          channel_enable_config;    /* bit[0] = 0 : channel 0 disabled
                                                     [0] = 1 : channel 0 enabled
                                                  bit[1] = 0 : channel 1 disabled
                                                     [1] = 1 : channel 1 enabled
                                                  bit[2] = 0 : channel 2 disabled
                                                     [2] = 1 : channel 2 enabled
                                                  bit[3] = 0 : channel 3 disabled
                                                     [3] = 1 : channel 3 enabled */
};


typedef struct _PCM_OBJECT_    PCM_OBJECT_T;

struct _PCM_OBJECT_
{
    u32          config_0;
    u32          config_1; 
    
    u32          channel_0_config;
    u32          channel_1_config;
    u32          channel_2_config;
    u32          channel_3_config;
    
    u32          interrupt_config;
    
    /* 
     * For interrupt setting
     */
//    INTC_OBJECT_T    intc_obj;
};



/*
 * function declarations
 */
void       Hal_Pcm_Initialize(PCM_OBJECT_T *);

                                                                           
/*
 * macro declarations
 */
#define HAL_PCM_ENABLE_PCM() \
{ \
    (PCM_CONFIGURATION_0_REG) |= ((u32)0x1 << 31); \
}

#define HAL_PCM_DISABLE_PCM() \
{ \
    (PCM_CONFIGURATION_0_REG) &= ~((u32)0x1 << 31); \
}

#define HAL_PCM_ENABLE_DATA_SWAP() \
{ \
    (PCM_CONFIGURATION_0_REG) |= (0x1 << 24); \
}

#define HAL_PCM_DISABLE_DATA_SWAP() \
{ \
    (PCM_CONFIGURATION_0_REG) &= ~(0x1 << 24); \
}

#define HAL_PCM_WRITE_TX_DATA_0(tx_data_0) \
{ \
    (PCM_TX_DATA_31_0_REG) = tx_data_0; \
}

#define HAL_PCM_WRITE_TX_DATA_1(tx_data_1) \
{ \
    (PCM_TX_DATA_63_32_REG) = tx_data_1; \
}

#define HAL_PCM_READ_RX_DATA_0(rx_data_0) \
{ \
    (rx_data_0) = PCM_RX_DATA_31_0_REG; \
}

#define HAL_PCM_READ_RX_DATA_1(rx_data_1) \
{ \
    (rx_data_1) = PCM_RX_DATA_63_32_REG; \
}

#define HAL_PCM_READ_INTERRUPT_STATUS(status) \
{ \
    (status) = PCM_INTERRUPT_STATUS_REG; \
}

#define HAL_PCM_CLEAR_INTERRUPT_STATUS(status) \
{ \
    (PCM_INTERRUPT_STATUS_REG) = (status & 0xC); \
}

#define HAL_PCM_DISABLE_RECEIVE_BUFFER_FULL_INTERRUPT() \
{ \
    (PCM_INTERRUPT_ENABLE_REG) &= ~(0x1 << 0); \
}

#define HAL_PCM_DISABLE_TRANSMIT_BUFFER_EMPTY_INTERRUPT() \
{ \
    (PCM_INTERRUPT_ENABLE_REG) &= ~(0x1 << 1); \
}

#define HAL_PCM_DISABLE_RECEIVE_BUFFER_OVERRUN_INTERRUPT() \
{ \
    (PCM_INTERRUPT_ENABLE_REG) &= ~(0x1 << 2); \
}

#define HAL_PCM_DISABLE_TRANSMIT_BUFFER_UNDERRUN_INTERRUPT() \
{ \
    (PCM_INTERRUPT_ENABLE_REG) &= ~(0x1 << 3); \
}

#define HAL_PCM_DISABLE_ALL_INTERRUPT_SOURCES() \
{ \
    (PCM_INTERRUPT_ENABLE_REG) = 0; \
}

#endif  // end of #ifndef _STAR_PCM_H_

