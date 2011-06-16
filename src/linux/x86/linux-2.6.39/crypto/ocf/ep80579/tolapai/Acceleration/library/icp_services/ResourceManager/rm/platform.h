/******************************************************************************
 * 
 * This file is provided under a dual BSD/GPLv2 license.  When using or 
 *   redistributing this file, you may do so under either license.
 * 
 *   GPL LICENSE SUMMARY
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify 
 *   it under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 * 
 *   This program is distributed in the hope that it will be useful, but 
 *   WITHOUT ANY WARRANTY; without even the implied warranty of 
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 *   General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License 
 *   along with this program; if not, write to the Free Software 
 *   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *   The full GNU General Public License is included in this distribution 
 *   in the file called LICENSE.GPL.
 * 
 *   Contact Information:
 *   Intel Corporation
 * 
 *   BSD LICENSE 
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without 
 *   modification, are permitted provided that the following conditions 
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the 
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its 
 *       contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * 
 *  version: Security.L.1.0.3-98
 *             
 *****************************************************************************/

/**
 *****************************************************************************
 * 
 * @defgroup rm_platform Platform specific 
 *
 * @ingroup rm
 *
 * @description
 *         This file contains thelatform specific defines, data structures and 
 *         function declarations.
 * 
 *****************************************************************************/

#if !defined(__PLATFORM_H__)
#define __PLATFORM_H__


#if defined(__cplusplus)
extern "C"
{
#endif /* end defined(__cplusplus) */
    
    
/**************************
 * Generic Tolapai specific
 *************************/

/**
 *****************************************************************************
 * @ingroup rm_platform  
 *         Number Accelengines
 *
 * @description
 *       This define type describes the maximum number Accelengines available
 *
 * @purpose
 *
 *
 *****************************************************************************/
#define IX_MAX_ACCELENGINE_NUMBER      4U



    
    
/**************************
 * Memory management module
 *************************/
    

/**
 *****************************************************************************
 * @ingroup rm_platform
 *         RM supported memory types
 * 
 * @description
 *      This enumerated type defines all types of memory that can existent in 
 *      the system that is managed by the Resource Manager.
 *
 * @purpose
 *      
 *****************************************************************************/
typedef enum ix_e_memory_type
{
    IX_MEMORY_TYPE_FIRST = 0,
    IX_MEMORY_TYPE_DRAM = IX_MEMORY_TYPE_FIRST, /**< DRAM memory */
    IX_MEMORY_TYPE_LAST,        
} ix_memory_type;




/**
 *****************************************************************************
 * @ingroup rm_platform
 *         RM mamanged memory channels
 * 
 * @description
 *      This enumerated type defines all types of memory channels that can 
 *      existent in the system that is managed by the Resource Manager.
 *
 * @purpose
 *      
 *****************************************************************************/
typedef enum ix_e_memory_channel
{
    IX_MEMORY_CHANNEL_TYPE_FIRST = 0,
    IX_MEMORY_CHANNEL_NONCOHERENT_DRAM = IX_MEMORY_TYPE_FIRST, 
                                     /**< NCDRAM memory channel */
    IX_MEMORY_CHANNEL_COHERENT_DRAM, /**< CDRAM memory channel */
    IX_MEMORY_CHANNEL_TYPE_LAST,        
} ix_memory_channel;

/**
 *****************************************************************************
 * @ingroup rm_platform  
 *         Number of DRAM Channels
 *
 * @description
 *       This define type describes the maximum number of DRAM channels
 *       supported
 *
 * @purpose
 *
 *
 *****************************************************************************/
#define IX_DRAM_CHANNELS_NUMBER         2U 





/*************
 * Hash module
 ************/
/**
 *****************************************************************************
 * @ingroup rm_platform  
 *         Define for software hash implementation
 *
 * @description
 *       This define type describes the type of hashing that needs to be
 *         supported.
 *
 * @purpose
 *
 *
 *****************************************************************************/
#define _IX_RM_IMPL_SOFTWARE_HASH_

#if defined(_IX_RM_IMPL_HARDWARE_HASH_) 
#error "Hardware hash unit support is not available in Tolapai, \
         don't use this flag!"
#endif 


/*****************
 * Hardware Module
 ****************/


/**
 *****************************************************************************
 * @ingroup rm_platform
 *         Ring types
 * 
 * @description
 *      This enumerated type defines all types of rings that can existent in 
 *      the system that is managed by the Resource Manager.
 *
 * @purpose
 *      
 *****************************************************************************/
typedef enum ix_e_ring_type
{
    IX_TYPE_FIRST = 0,
    IX_ICP_RING = IX_TYPE_FIRST,         /**< ICP Ring */
    IX_SOFTWARE_RING,                   /**< Software Ring */
    IX_RING_LAST
} ix_ring_type;



/**
 *****************************************************************************
 * @ingroup rm_platform
 *         ICP rings buffer sizes.
 * 
 * @description
 *      This enumerated type describes the allowed sizes for an eagle tail ring
 *
 * @purpose
 *      
 *****************************************************************************/
typedef enum ix_e_icp_ring_size
{
       IX_ICP_RING_SIZE_FIRST = 0,
    IX_ICP_RING_SIZE_64 = IX_ICP_RING_SIZE_FIRST, 
    IX_ICP_RING_SIZE_128,
    IX_ICP_RING_SIZE_256,
    IX_ICP_RING_SIZE_512,
    IX_ICP_RING_SIZE_1K,
    IX_ICP_RING_SIZE_2K,
    IX_ICP_RING_SIZE_4K,
    IX_ICP_RING_SIZE_8K,
    IX_ICP_RING_SIZE_16K,
    IX_ICP_RING_SIZE_32K,
    IX_ICP_RING_SIZE_64K,
    IX_ICP_RING_SIZE_128K,
    IX_ICP_RING_SIZE_256K,
    IX_ICP_RING_SIZE_512K,
    IX_ICP_RING_SIZE_1M,
    IX_ICP_RING_SIZE_2M,
    IX_ICP_RING_SIZE_4M,
    IX_ICP_RING_SIZE_LAST
} ix_icp_ring_size;


/**
 *****************************************************************************
 * @ingroup rm_platform  
 *         ICP rings near watermark value.
 *
 * @description
 *       This enumerated type describes the allowed near watermarks for an
 *       eagle tail ring. 
 *
 * @purpose
 *
 *
 *****************************************************************************/
typedef enum ix_e_icp_ring_near_watermark
{
    IX_ICP_RING_NEAR_WATERMARK_FIRST = 0,
    IX_ICP_RING_NEAR_WATERMARK_0 = IX_ICP_RING_NEAR_WATERMARK_FIRST, 
    IX_ICP_RING_NEAR_WATERMARK_4,
    IX_ICP_RING_NEAR_WATERMARK_8,
    IX_ICP_RING_NEAR_WATERMARK_16,
    IX_ICP_RING_NEAR_WATERMARK_32,
    IX_ICP_RING_NEAR_WATERMARK_64,
    IX_ICP_RING_NEAR_WATERMARK_128,
    IX_ICP_RING_NEAR_WATERMARK_256,
    IX_ICP_RING_NEAR_WATERMARK_512,
    IX_ICP_RING_NEAR_WATERMARK_1K,
    IX_ICP_RING_NEAR_WATERMARK_2K,
    IX_ICP_RING_NEAR_WATERMARK_4K,
    IX_ICP_RING_NEAR_WATERMARK_8K,
    IX_ICP_RING_NEAR_WATERMARK_16K,
    IX_ICP_RING_NEAR_WATERMARK_32K,
    IX_ICP_RING_NEAR_WATERMARK_64K,
    IX_ICP_RING_NEAR_WATERMARK_128K,
    IX_ICP_RING_NEAR_WATERMARK_256K,
    IX_ICP_RING_NEAR_WATERMARK_LAST
} ix_icp_ring_near_watermark;



/**
 *****************************************************************************
 * @ingroup rm_platform
 *         Software rings buffer sizes.
 * 
 * @description
 *      This enumerated type describes the allowed sizes for an software ring
 *
 * @purpose
 *      
 *****************************************************************************/
typedef enum ix_e_software_ring_size
{
       IX_SOFTWARE_RING_SIZE_FIRST = 0,
    IX_SOFTWARE_RING_SIZE_64 = IX_SOFTWARE_RING_SIZE_FIRST, 
    IX_SOFTWARE_RING_SIZE_128,
    IX_SOFTWARE_RING_SIZE_256,
    IX_SOFTWARE_RING_SIZE_512,
    IX_SOFTWARE_RING_SIZE_1K,
    IX_SOFTWARE_RING_SIZE_2K,
    IX_SOFTWARE_RING_SIZE_4K,
    IX_SOFTWARE_RING_SIZE_8K,
    IX_SOFTWARE_RING_SIZE_16K,
    IX_SOFTWARE_RING_SIZE_32K,
    IX_SOFTWARE_RING_SIZE_64K,
    IX_SOFTWARE_RING_SIZE_128K,
    IX_SOFTWARE_RING_SIZE_256K,
    IX_SOFTWARE_RING_SIZE_512K,
    IX_SOFTWARE_RING_SIZE_1M,
    IX_SOFTWARE_RING_SIZE_2M,
    IX_SOFTWARE_RING_SIZE_4M,
    IX_SOFTWARE_RING_SIZE_LAST
} ix_software_ring_size;


/**
 *****************************************************************************
 * @ingroup rm_platform  
 *         Software rings near watermark value.
 *
 * @description
 *       This enumerated type describes the allowed near watermarks for an
 *       software ring. 
 *
 * @purpose
 *
 *
 *****************************************************************************/
typedef enum ix_e_software_ring_near_watermark
{
    IX_SOFTWARE_RING_NEAR_WATERMARK_FIRST = 0,
    IX_SOFTWARE_RING_NEAR_WATERMARK_0 = IX_SOFTWARE_RING_NEAR_WATERMARK_FIRST, 
    IX_SOFTWARE_RING_NEAR_WATERMARK_4,
    IX_SOFTWARE_RING_NEAR_WATERMARK_8,
    IX_SOFTWARE_RING_NEAR_WATERMARK_16,
    IX_SOFTWARE_RING_NEAR_WATERMARK_32,
    IX_SOFTWARE_RING_NEAR_WATERMARK_64,
    IX_SOFTWARE_RING_NEAR_WATERMARK_128,
    IX_SOFTWARE_RING_NEAR_WATERMARK_256,
    IX_SOFTWARE_RING_NEAR_WATERMARK_512,
    IX_SOFTWARE_RING_NEAR_WATERMARK_1K,
    IX_SOFTWARE_RING_NEAR_WATERMARK_2K,
    IX_SOFTWARE_RING_NEAR_WATERMARK_4K,
    IX_SOFTWARE_RING_NEAR_WATERMARK_8K,
    IX_SOFTWARE_RING_NEAR_WATERMARK_16K,
    IX_SOFTWARE_RING_NEAR_WATERMARK_32K,
    IX_SOFTWARE_RING_NEAR_WATERMARK_64K,
    IX_SOFTWARE_RING_NEAR_WATERMARK_128K,
    IX_SOFTWARE_RING_NEAR_WATERMARK_256K,
    IX_SOFTWARE_RING_NEAR_WATERMARK_LAST
} ix_software_ring_near_watermark;



#if defined(_IX_RM_IMPL_HARDWARE_)

/**
 *****************************************************************************
 * @ingroup rm_platform  
 *         ICP rings CSRs.
 *
 * @description
 *       This enumerated type describes the CSRs of a Eagle Tail ring. 
 *
 * @purpose
 *
 *
 *****************************************************************************/
typedef enum ix_e_icp_ring_csr
{
    IX_ICP_RING_CSR_FIRST = 0,
    IX_ICP_RING_CSR_RING_CONFIG = /*ET_RING_BASE = */0x0,
    IX_ICP_RING_CSR_RING_BASE = /*ET_RING_BASE = */0x100,
    IX_ICP_RING_CSR_RING_HEAD_OFFSET = /*ET_RING_HEAD_OFFSET = */0x200,
    IX_ICP_RING_CSR_RING_TAIL_OFFSET = /*ET_RING_TAIL_OFFSET = */0x300,
    IX_ICP_RING_CSR_INT_EN_0 = /*ET_RING_IA_INT_EN_0 = */0x900,
    IX_ICP_RING_CSR_INT_EN_1 = /*ET_RING_IA_INT_EN_1 = */0x904,
    IX_ICP_RING_CSR_INT_REG_0 = /*ET_RING_IA_INT_REG_0 = */0x908,    
    IX_ICP_RING_CSR_INT_REG_1 = /*ET_RING_IA_INT_REG_1 = */0x90C,    
    IX_ICP_RING_CSR_INT_SRCSEL_0 = /*ET_RING_IA_INT_SRCSEL_0 = */0x910,    
    IX_ICP_RING_CSR_INT_SRCSEL_1 = /*ET_RING_IA_INT_SRCSEL_1 = */0x914,    
    IX_ICP_RING_CSR_INT_SRCSEL_2 = /*ET_RING_IA_INT_SRCSEL_2 = */0x918,    
    IX_ICP_RING_CSR_INT_SRCSEL_3 = /*ET_RING_IA_INT_SRCSEL_3 = */0x91C,    
    IX_ICP_RING_CSR_INT_SRCSEL_4 = /*ET_RING_IA_INT_SRCSEL_4 = */0x920,    
    IX_ICP_RING_CSR_INT_SRCSEL_5 = /*ET_RING_IA_INT_SRCSEL_5 = */0x924,    
    IX_ICP_RING_CSR_INT_SRCSEL_6 = /*ET_RING_IA_INT_SRCSEL_6 = */0x928,    
    IX_ICP_RING_CSR_INT_SRCSEL_7 = /*ET_RING_IA_INT_SRCSEL_7 = */0x92C,
    IX_ICP_RING_CSR_LAST
}ix_icp_ring_csr;
   


#else

typedef enum ix_e_icp_ring_csr
{
    IX_ICP_RING_CSR_FIRST = 0,
    IX_ICP_RING_CSR_RING_BASE,
    IX_ICP_RING_CSR_RING_HEAD_OFFSET,
    IX_ICP_RING_CSR_RING_TAIL_OFFSET,
    IX_ICP_RING_CSR_RING_STAT,
    IX_ICP_RING_CSR_UO_STAT,
    IX_ICP_RING_CSR_E_STAT,
    IX_ICP_RING_CSR_NE_STAT,
    IX_ICP_RING_CSR_NF_STAT,
    IX_ICP_RING_CSR_F_STAT,
    IX_ICP_RING_CSR_C_STAT,
    IX_ICP_RING_CSR_INT_EN_0,
    IX_ICP_RING_CSR_INT_EN_1,
    IX_ICP_RING_CSR_INT_REG_0,    
    IX_ICP_RING_CSR_INT_REG_1,    
    IX_ICP_RING_CSR_INT_SRCSEL_0,    
    IX_ICP_RING_CSR_INT_SRCSEL_1,    
    IX_ICP_RING_CSR_INT_SRCSEL_2,    
    IX_ICP_RING_CSR_INT_SRCSEL_3,    
    IX_ICP_RING_CSR_INT_SRCSEL_4,    
    IX_ICP_RING_CSR_INT_SRCSEL_5,    
    IX_ICP_RING_CSR_INT_SRCSEL_6,    
    IX_ICP_RING_CSR_INT_SRCSEL_7,
    IX_ICP_RING_CSR_LAST
}ix_icp_ring_csr;
    

#endif /* defined(_IX_RM_IMPL_HARDWARE_) */


/**
 *****************************************************************************
 * @ingroup rm_platform  
 *         ICP rings interrupt source register.
 *
 * @description
 *       This enumerated type describes the valid interrupt source registers 
 *       can be set for an Eagle Tail ring.
 *
 * @purpose
 *
 *
 *****************************************************************************/
typedef enum ix_e_icp_ring_intrpt_src_reg
{
    IX_ICP_RING_INTRPT_SRC_REG_FIRST = 0,   /* IAIntSrcSel0 */
    IX_ICP_RING_INTRPT_SRC_REG0 = IX_ICP_RING_INTRPT_SRC_REG_FIRST,
    IX_ICP_RING_INTRPT_SRC_REG1,            /* IAIntSrcSel1 */
    IX_ICP_RING_INTRPT_SRC_REG2,            /* IAIntSrcSel2 */
    IX_ICP_RING_INTRPT_SRC_REG3,            /* IAIntSrcSel3 */
    IX_ICP_RING_INTRPT_SRC_REG4,            /* IAIntSrcSel4 */
    IX_ICP_RING_INTRPT_SRC_REG5,            /* IAIntSrcSel5 */
    IX_ICP_RING_INTRPT_SRC_REG6,            /* IAIntSrcSel6 */
    IX_ICP_RING_INTRPT_SRC_REG7,            /* IAIntSrcSel7 */
    IX_ICP_RING_INTRPT_SRC_REG_LAST
}ix_icp_ring_intrpt_src_reg;


/**
 *****************************************************************************
 * @ingroup rm_platform  
 *         ICP rings interrupt source.
 *
 * @description
 *       This enumerated type describes the valid interrupt source value that
 *       can be set for an Eagle Tail ring.
 *
 * @purpose
 *
 *
 *****************************************************************************/
typedef enum ix_e_icp_ring_intrpt_src
{
    IX_ICP_RING_INTRPT_SRC_FIRST = 0,
    IX_ICP_RING_INTRPT_SRC_EMPTY_GOING_TRUE = IX_ICP_RING_INTRPT_SRC_FIRST,
    IX_ICP_RING_INTRPT_SRC_NEAR_EMPTY_GOING_TRUE,
    IX_ICP_RING_INTRPT_SRC_NEAR_FULL_GOING_TRUE,
    IX_ICP_RING_INTRPT_SRC_FULL_GOING_TRUE,
    IX_ICP_RING_INTRPT_SRC_EMPTY_GOING_FALSE,
    IX_ICP_RING_INTRPT_SRC_NEAR_EMPTY_GOING_FALSE,
    IX_ICP_RING_INTRPT_SRC_NEAR_FULL_GOING_FALSE,
    IX_ICP_RING_INTRPT_SRC_FULL_GOING_FALSE,
    IX_ICP_RING_INTRPT_SRC_LAST
}ix_icp_ring_intrpt_src;

    
    


/**
 *****************************************************************************
 * @ingroup rm_platform  
 *         Max ICP ring number
 *
 * @description
 *       This define type describes the maximum ring number value allowed for
 *       eagle tail rings.
 *
 * @purpose
 *
 *
 *****************************************************************************/
#define IX_MAX_ICP_RINGS            64U



/**
 *****************************************************************************
 * @ingroup rm_platform  
 *         Max Software ring number
 *
 * @description
 *       This define type describes the maximum ring number value allowed for
 *       software rings.
 *
 * @purpose
 *
 *
 *****************************************************************************/
#define IX_MAX_SOFTWARE_RINGS       64U 




/***************
 * Buffer Module
 * ************/

/**
 *****************************************************************************
 * @ingroup rm_platform
 *         Buffer Pool types.
 * 
 * @description
 *      This enumerated type defines all types of buffer pools that can 
 *      existent in the system that is managed by the Resource Manager.
 *
 * @purpose
 *      
 *****************************************************************************/
typedef enum ix_e_buffer_pool_type
{
    IX_BUFFER_POOL_TYPE_FIRST = 0,
    IX_BUF_POOL_DIRECT = IX_BUFFER_POOL_TYPE_FIRST,/**< Direct buffer pool */
    IX_BUF_POOL_INDIRECT,                          /**< Indirect buffer pool */
    IX_BUFFER_POOL_TYPE_LAST,        
} ix_buffer_pool_type;


/**
 *****************************************************************************
 * @ingroup rm_platform
 *         Buffer Pool ID bits.
 * 
 * @description
 *      This symbol defines the maximum number of bits required to represent 
 *      a hardware buffer pool id. 
 *
 * @purpose
 *      
 *****************************************************************************/
#define IX_BUF_HW_BP_ID_BITS         2U


/**
 *****************************************************************************
 * @ingroup rm_platform
 *         Buffer element byte aligment in bits.
 * 
 * @description
 *      This symbol defines buffer elements byte alignmentment in power of 2. 
 *
 * @purpose
 *      
 *****************************************************************************/
#define IX_BUF_ELEMENT_BYTE_ALIGNMENT    6U

/* Buffer Elements byte alignment */
#define IX_BUF_ELEMENT_BYTE_ALIGNED    (1 << IX_BUF_ELEMENT_BYTE_ALIGNMENT)



/**************************
 * Generic error check code
 *************************/
 
/**
 * NAME: _ix_rm_platform_check_for_rmsupp_memtype
 *
 * DESCRIPTION: This function will verify if the memory that has been passed as
 *  argument can be accessed by RM. Will return err in the case of failure.
 * 
 * @Param:  - IN arg_MemType - Type of Memory being accessed
 *
 * @Return: IX_SUCCESS if successful.
 *          IX_RM_ERROR_BAD_MEMORY_TYPE if memory type is not valid.
 */
ix_error _ix_rm_platform_check_for_rmsupp_memtype(ix_uint32 arg_MemType);
    

/**
 * NAME: _ix_rm_platform_check_for_rmsupp_memory
 *
 * DESCRIPTION: This function will verify if the memory is
 *  accessible by RM. Will return err in the case of failure 
 *  (i.e. this is not a RM DRAM managed address)
 * 
 * @Param:  - IN arg_MemType - Type of Memory being accessed
 * @Param:  - IN arg_MemChannel - Channel Number for the specified memory typed
 *
 * @Return: IX_SUCCESS if successful.
 *          IX_RM_ERROR_BAD_MEMORY_TYPE if memory type is not valid.
 *          IX_RM_ERROR_BAD_MEMORY_CHANNEL if memory channel is not valid.
 */
ix_error _ix_rm_platform_check_for_rmsupp_memory (
                                            ix_uint32 arg_MemType,
                                            ix_uint32 arg_MemChannel
                                        );


/**
 * NAME: _ix_rm_platform_check_for_rmsupp_ring_memory
 *
 * DESCRIPTION: This function will verify if the memory can support a 
 * ring that can been accessed by RM. 
 * Will return err in the case of failure
 * 
 * @Param:  - IN arg_MemType - Type of Memory being accessed
 * #Param:  - IN arg_memChannel - Memory channel number where the ring resides
 *
 * @Return: IX_SUCCESS if successful.
 *          IX_RM_ERROR_HW_RING_MEM_NOT_SUPPORTED if hardware ring is not
 *              supported in that particular memory.
 */
ix_error _ix_rm_platform_check_for_rmsupp_ring_memory (
                                            ix_uint32 arg_MemType,
                                            ix_uint32 arg_memChannel    
                                        );

/**
 * NAME: _ix_rm_platform_check_for_rmsupp_icp_ring_memtype
 *
 * DESCRIPTION: This function will verify if the memory can support ICP 
 * ring that can been accessed by RM. 
 * Will return err in the case of failure
 * 
 * @Param:  - IN arg_MemType - Type of Memory being accessed
 * #Param:  - IN arg_MemChannel - Memory channel number where the ring resides
 *
 * @Return: IX_SUCCESS if successful.
 *          IX_RM_ERROR_HW_RING_MEM_NOT_SUPPORTED if ICP ring is not supported
 *              in that particular memory.
 */
ix_error _ix_rm_platform_check_for_rmsupp_icp_ring_memory(
                                                      ix_uint32 arg_MemType,
                                                      ix_uint32 arg_MemChannel
                                                      );


/**
 * NAME: _ix_rm_platform_check_for_rmsupp_software_ring_memory
 *
 * DESCRIPTION: This function will verify if the memory can support ICP 
 * ring that can been accessed by RM. 
 * Will return err in the case of failure
 * 
 * @Param:  - IN arg_MemType - Type of Memory being accessed
 *
 * @Return: IX_SUCCESS if successful.
 *          IX_RM_ERROR_HW_RING_MEM_NOT_SUPPORTED if software ring is not
 *              supported in that particular memory.
 */
ix_error _ix_rm_platform_check_for_rmsupp_software_ring_memory (
                                                    ix_uint32 arg_MemType,
                                                    ix_uint32 arg_MemChannel
                                                  );



/**
 * NAME: _ix_rm_platform_check_for_rmsupp_ring_type
 *
 * DESCRIPTION: This function will verify if the ring type can be supported
 *  by RM. 
 * Will return err in the case of failure
 * 
 * @Param:  - IN arg_RingType - Type of Ring
 *
 * @Return: IX_SUCCESS if successful.
 *          IX_RM_ERROR_HW_RING_NOT_SUPPORTED if ring type is is not valid.
 */
ix_error _ix_rm_platform_check_for_rmsupp_ring_type(ix_uint32 arg_RingType);


#if defined(__cplusplus)
}
#endif /* end defined(__cplusplus) */

#endif /* end !defined(__PLATFORM_H__) */
