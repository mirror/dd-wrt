/**
 **************************************************************************
 * @file uof.h
 *
 * @description
 *      This is the header file for uCode Object File
 *
 * @par 
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
 **************************************************************************/ 

/*
 ****************************************************************************
 * Doxygen group definitions
 ****************************************************************************/

/**
 *****************************************************************************
 * @file uof.h
 * 
 * @defgroup UOF Microcode File Format Definition
 *
 * @description
 *      This header file that contains the definitions of micorcode file 
 *      format used by linker and loader.
 *
 *****************************************************************************/

#ifndef __UOF_H__
#define __UOF_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "ae_constants.h"

#define UOF_MAX_NUM_OF_AE   16     /**< maximum number of AE */
#define UOF_MAX_THREADS     8      /**< maximum number of context */

#define UOF_MAX_UWORDS      MAX_USTORE  /**< maximum number of micro store */

#define UOF_SIGREG_MIN      1      /**< minimum number of signore register */
#define UOF_SIGREG_MAX      15     /**< maximum number of signature register */

#define MAX_AE2_CTX         UOF_MAX_THREADS /**< maximum number of context */


#define UOF_OBJID_LEN       8     /**< length of object ID */
#define UOF_FIELD_POS_SIZE  12    /**< field postion size */


#define UOF_FID         0xc6c2      /**< UOF magic number */
#define UOF_MAJVER      0x4         /**< UOF major version */
#define UOF_MINVER      0xe         /**< UOF minor version */

#define UOF_LOC_MEM_REL 0           /**< local memory relative access mode */
#define UOF_LOC_MEM_ABS 1           /**< local memory absolute access mode */

#define UOF_NN_MODE_NEIGH       0   /**< next neighbour access mode for NN reg */
#define UOF_NN_MODE_SELF        1   /**< self access mode  for NN reg*/
#define UOF_NN_MODE_DONTCARE   0xff /**< do not care access mode  for NN reg*/ 

#define UOF_OBJS        "UOF_OBJS"  /**< object chunk ID */

#define UOF_BLNK        "        "  /**< blank section ID */
#define UOF_STRT        "UOF_STRT"  /**< string table section ID */
#define UOF_GTID        "UOF_GTID"  /**< GTID section ID */
#define UOF_IMAG        "UOF_IMAG"  /**< image section ID */
#define UOF_IMEM        "UOF_IMEM"  /**< import section ID */
#define UOF_MSEG        "UOF_MSEG"  /**< memory section ID */

#define UOF_SUCCESS     0           /**< success ID */
#define UOF_FAILURE     1           /**< failure ID */

#define UOF_MODE_ON     1           /**< UOF on mode */
#define UOF_MODE_OFF    0           /**< UOF off mode */

#define CTX_MODE_MSK                0xf  /**< context mode mask */
#define NN_MODE_MSK                 0xf  /**< next neighbour mode mask */
#define CTX_MODE_LSB                0x0  /**< context mode LSB */
#define NN_MODE_LSB                 0x4  /**< next neighbour mode LSB */
#define LOC_MEM0_LSB                0x8  /**< local memory 0 LSB */
#define LOC_MEM1_LSB                0x9  /**< local memory 1 LSB */
#define ECC_LSB                     0xa  /**< ECC LSB */
#define SHARED_USTORE_LSB           0xb  /**< shared microstore LSB */
#define RELOADABLE_CTX_SHARED_LSB   0xc  /**< relodable microstore context shared LSB */
#define LEGACY_MODE_MSK    0x1           /**< legacy mode mask */ 
#define LEGACY_MODE_LSB     0xd          /**< local mode LSB */ 

#define LOC_MEM_MODE_MSK            0x1  /**< local memory mode mask */ 
#define UOF_ONOFF_MSK               0x1  /**< UOF on mode mask */ 

#define THREAD_TYPE_MASK    0x00ff   /**< thread type mask */   
#define FILL_BIT_MASK       0x8000   /**< fill bit mask */
#define IMAGE_THREAD_TYPE(imageAttr) ((imageAttr & THREAD_TYPE_MASK))
#define IMAGE_FILL_PATTERN(imageAttr) ((imageAttr & FILL_BIT_MASK))

#define FIXUP_ASSIGN_MSK    0x02     /**< fixup assign mask */
#define FIXUP_SCOPE_MSK     0x01     /**< fixup scope mask */
#define FIXUP_SCOPE_LSB     0        /**< fixup scope LSB */
#define FIXUP_ASSIGN_LSB    1        /**< fixup assign LSB */

#define GLOBAL_SCOPE    0           /**< global scope */
#define LOCAL_SCOPE     1           /**< local scope */

#define AEV2_PACKED_UWORD_BYTES 6  /**< version 2 packed uword size */

#define INIT_EXPR               0  /**< init expresion */
#define INIT_REG                1  /**< init register */
#define INIT_REG_CTX            2  /**< init register context */
#define INIT_EXPR_ENDIAN_SWAP   3  /**< init expresion endian swap */

/**< get 4 or 8 context mode */
#define CTX_MODE(aeMode) ((aeMode) & CTX_MODE_MSK)
/**< get next neighbour mode */
#define NN_MODE(aeMode) (((aeMode) >> NN_MODE_LSB) & NN_MODE_MSK)
/**< get error code check mode */
#define ECC_MODE(aeMode) (((aeMode) >> ECC_LSB) & UOF_ONOFF_MSK)
/**< get shared microstore mode */
#define SHARED_USTORE_MODE(aeMode) (((aeMode) >> SHARED_USTORE_LSB) & UOF_ONOFF_MSK)
/**< get reloadable mode */
#define RELOADABLE_CTX_SHARED_MODE(aeMode) (((aeMode) >> RELOADABLE_CTX_SHARED_LSB) & UOF_ONOFF_MSK)
/**< get local memory 0 mode */
#define LOC_MEM0_MODE(aeMode) (((aeMode) >> LOC_MEM0_LSB) & LOC_MEM_MODE_MSK)
/**< get local memory 1 mode */
#define LOC_MEM1_MODE(aeMode) (((aeMode) >> LOC_MEM1_LSB) & LOC_MEM_MODE_MSK)
/**< get legacy mode */
#define LEGACY_MODE(aeMode) (((aeMode) >> LEGACY_MODE_LSB) & LEGACY_MODE_MSK)

/**< set instruction fixup scope */
#define SET_FIXUP_SCOPE(attrs, val) {attrs &= ~FIXUP_SCOPE_MSK; attrs |= (val << FIXUP_SCOPE_LSB);}
/**< set instruction fixup assign */
#define SET_FIXUP_ASSIGN(attrs, val) {attrs &= ~FIXUP_ASSIGN_MSK; attrs |= (val << FIXUP_ASSIGN_LSB);}
/**< get instruction fixup scope */
#define GET_FIXUP_SCOPE(attrs) (((attrs) & FIXUP_SCOPE_MSK) >> FIXUP_SCOPE_LSB)
/**< get instruction fixup assign */
#define GET_FIXUP_ASSIGN(attrs) (((attrs) & FIXUP_ASSIGN_MSK) >> FIXUP_ASSIGN_LSB)

/**< get symbol' local name */
#define LOCAL_NAME(localName, symName, uengNum) snprintf(localName, sizeof(localName), "%d!%s", uengNum, symName)

typedef icp_RegType_T uof_RegType;
typedef icp_RegType_T Ucld_RegType;

typedef enum{
    SRAM0_REGION,          /**< SRAM0 region */  
    DRAM_REGION,           /**< non-coherent DRAM region */  
    DRAM1_REGION,          /**< coherent DRAM region */  
    LMEM_REGION,           /**< local memory region */  
    SCRATCH_REGION,        /**< SCRATCH region */  
    UMEM_REGION,           /**< micro-store region */  
    RAM_REGION,            /**< RAM region */  
    SHRAM_REGION           /**< shared memory region */  
}uof_MemRegion;

typedef enum{
    UNDEF_VAL,             /**< undefined value */  
    CHAR_VAL,              /**< character value */  
    SHORT_VAL,             /**< short value */  
    INT_VAL,               /**< integer value */  
    STR_VAL,               /**< string value */  
    STRTAB_VAL,            /**< string table value */  
    NUM_VAL,               /**< number value */  
    EXPR_VAL               /**< expression value */  
}uof_ValueKind_T;


typedef enum{
    REG_ACCESS_UNDEF,      /**< register access undefined */  
    REG_ACCESS_READ,       /**< register read access */  
    REG_ACCESS_WRITE,      /**< register write access */  
    REG_ACCESS_BOTH        /**< register read/write access */  
}uof_RegAccessMode_T;

typedef struct uof_fileHdr_S{
   unsigned short   fileId;             /**< file id and endian indicator */
   unsigned short   reserved1;          /**< reserved for future use */
   char             minVer;             /**< file format minor version */
   char             majVer;             /**< file format major version */
   unsigned short   reserved2;          /**< reserved for future use */
   unsigned short   maxChunks;          /**< max chunks in file */
   unsigned short   numChunks;          /**< num of actual chunks */
   /* maxChunks of uof_fileChunkHdr_T immediately follows this header */
}uof_fileHdr_T;

typedef struct uof_fileChunkHdr_S{
   char         chunkId[UOF_OBJID_LEN]; /**< chunk identifier */
   unsigned int checksum;               /**< chunk checksum */
   unsigned int offset;                 /**< offset of the chunk in the file */
   unsigned int size;                   /**< size of the chunk */
}uof_fileChunkHdr_T;                    /**< 20 bytes total */

typedef struct uof_objHdr_S{
   unsigned int     cpuType;            /**< CPU type */
   unsigned short   minCpuVer;          /**< starting CPU version */
   unsigned short   maxCpuVer;          /**< ending CPU version */
   short            maxChunks;          /**< max chunks in chunk obj */
   short            numChunks;          /**< num of actual chunks */
   unsigned int     reserved1;          /**< reserved for future use */
   unsigned int     reserved2;          /**< reserved for future use */
   /* maxChunks of uof_chunkHdr_T immediately follows this header */
}uof_objHdr_T;

typedef struct uof_chunkHdr_S{
   char         chunkId[UOF_OBJID_LEN]; /**< chunk identifier */
   unsigned int offset;                 /**< offset of the chunk in the file */
   unsigned int size;                   /**< size of the chunk */
}uof_chunkHdr_T;                        /**< 16 bytes total */


typedef struct uof_strTab_S{
   unsigned int tableLen;               /**< length of table */
   char         *strings;               /**< NULL terminated strings */
}uof_strTab_T;

typedef struct uof_memValAttr_S{
    unsigned int    byteOffset;         /**< byte-offset from the allocated memory */
    unsigned int    value;              /**< memory value */
}uof_memValAttr_T;

typedef struct uof_initMem_S{
   unsigned int     symName;            /**< symbol name */
   char             region;             /**< memory region -- uof_MemRegion */
   char             scope;              /**< visibility scope */
   unsigned short   reserved1;          /**< reserved for future use */
   unsigned int     addr;               /**< memory address */
   unsigned int     numBytes;           /**< number of bytes */
   unsigned int     numValAttr;         /**< number of values attributes */
   /* numValAttr of uof_memValAttr_T follows this header */
}uof_initMem_T;


typedef struct uof_initRegSym_S{
   unsigned int     symName;            /**< symbol name */
   char             initType;           /**< 0=expr, 1=register, 2=ctxReg, 3=expr_endian_swap */
   char             valueType;          /**< EXPR_VAL, STRTAB_VAL */
   char             regType;            /**< register type: icp_RegType_T */
   unsigned char    ctx;                /**< AE context when initType=2 */
   unsigned int     regAddrOrOffset;    /**< reg address, or sym-value offset */
   unsigned int     value;              /**< integer value, or expression */
}uof_initRegSym_T;

typedef struct uof_VarMemSeg_S{
   unsigned int sram0Base;              /**< uC variables SRAM0 memory segment base address */
   unsigned int sram0Size;              /**< uC variables SRAM0 segment size bytes */
   unsigned int sram0Alignment;         /**< uC variables SRAM0 segment alignment bytes */
   unsigned int sdramBase;              /**< uC variables DRAM0 memory segment base address */
   unsigned int sdramSize;              /**< uC variables DRAM0 segment size bytes */
   unsigned int sdramAlignment;         /**< uC variables DRAM0 segment alignment bytes */
   unsigned int sdram1Base;             /**< uC variables DRAM1 memory segment base address */
   unsigned int sdram1Size;             /**< uC variables DRAM1 segment size bytes */
   unsigned int sdram1Alignment;        /**< uC variables DRAM1 segment alignment bytes */
   unsigned int scratchBase;            /**< uC variables SCRATCH memory segment base address */
   unsigned int scratchSize;            /**< uC variables SCRATCH segment size bytes */
   unsigned int scratchAlignment;       /**< uC variables SCRATCH segment alignment bytes */
}uof_varMemSeg_T;


typedef struct uof_GTID_S{
   char         toolId[UOF_OBJID_LEN];  /**< creator tool ID */
   int          toolVersion;            /**< creator tool version */
   unsigned int reserved1;              /**< reserved for future use */
   unsigned int reserved2;              /**< reserved for future use */
}uof_GTID_T;

typedef struct uof_sbreak_S{
    unsigned int    pageNum;            /**< page number */
    unsigned int    virtUaddr;          /**< virt uaddress */
	unsigned char   sbreakType;         /**< sbreak type */
    unsigned char   regType;            /**< register type: icp_RegType_T */
    unsigned short  reserved1;          /**< reserved for future use */
    unsigned int    addrOffset;         /**< branch target address or offset to be used
                                           with the reg value to calculate the target address */
    unsigned int    regAddr;            /**< register address */
}uof_sbreak_T;


typedef struct uof_codePage_S{
   unsigned int pageRegion;             /**< page associated region */
   unsigned int pageNum;                /**< code-page number */
   unsigned char defPage;               /**< default page indicator */
   unsigned char reserved2;             /**< reserved for future use */
   unsigned short reserved1;            /**< reserved for future use */
   unsigned int begVirtAddr;            /**< starting virtual uaddr */
   unsigned int begPhyAddr;             /**< starting physical uaddr */
   unsigned int neighRegTabOffset;      /**< offset to neighbour-reg table */
   unsigned int ucVarTabOffset;         /**< offset to uC var table */
   unsigned int impVarTabOffset;        /**< offset to import var table */
   unsigned int impExprTabOffset;       /**< offset to import expression table */
   unsigned int codeAreaOffset;         /**< offset to code area */
}uof_codePage_T;

typedef struct uof_Image_S{
   unsigned int     imageName;          /**< image name */
   unsigned int     aeAssigned;         /**< AccelEngines assigned */
   unsigned int     ctxAssigned;        /**< AccelEngine contexts assigned */
   unsigned int     cpuType;            /**< cpu type */
   unsigned int     entryAddress;       /**< entry uaddress */
   unsigned int     fillPattern[2];     /**< uword fill value */
   unsigned int     reloadableSize;     /**< size of reloadable ustore section */

   unsigned char    sensitivity;        /**< case sensitivity: 0 = insensitive, 1 = sensitive  */
   unsigned char    reserved;           /**< reserved for future use */
   unsigned short   aeMode;             /**< unused<15:14>,legacyMode<13>,reloadCtxShared<12>, sharedUstore<11>,ecc<10>,
                                           locMem1<9>,locMem0<8>,nnMode<7:4>,ctx<3:0> */

   unsigned short   maxVer;             /**< max cpu ver on which the image can run */
   unsigned short   minVer;             /**< min cpu ver on which the image can run */

   unsigned short   imageAttrib;        /**< image attributes */
   unsigned short   reserved2;          /**< reserved for future use */

   unsigned short   numOfPageRegions;   /**< number of page regions */
   unsigned short   numOfPages;         /**< number of pages */

   unsigned int     regTabOffset;       /**< offset to register table */
   unsigned int     initRegSymTab;      /**< reg/sym init table */
   unsigned int     sbreakTab;          /**< offset to sbreak table */

   unsigned int     appMetadata;        /**< application meta-data */
   /* numOfPage of uof_codePage follows this header */
}uof_Image_T;


typedef struct uof_objTable_S{
   unsigned int     numEntries;         /**< number of table entries */
   /* numEntries of object follows */
}uof_objTable_T;

typedef uof_objTable_T uof_aeRegTab_T;
typedef uof_objTable_T uof_impVarTab_T;
typedef uof_objTable_T uof_impExprTab_T;
typedef uof_objTable_T uof_neighRegTab_T;
typedef uof_objTable_T uof_ucVarTab_T;
typedef uof_objTable_T uof_initRegSymTab_T;
typedef uof_objTable_T uof_sbreakTab_T;
typedef uof_objTable_T uof_uwordBlockTab_T;

typedef struct uof_uwordFixup_S{
   unsigned int     name;               /**< offset to string table */
   unsigned int     uwordAddress;       /**< micro word address */
   unsigned int     exprValue;          /**< string table offset of expr string, or value */
   unsigned char    valType;            /**< VALUE_UNDEF, VALUE_NUM, VALUE_EXPR */
   unsigned char    valueAttrs;         /**< bit<0> (Scope: 0=global, 1=local), bit<1> (init: 0=no, 1=yes) */
   unsigned short   reserved1;          /**< reserved for future use */
   char         fieldAttrs[UOF_FIELD_POS_SIZE]; /**< field pos, size, and right shift value */
}uof_uwordFixup_T;
typedef uof_uwordFixup_T uof_ucVar_T;
typedef uof_uwordFixup_T uof_neighReg_T;
typedef uof_uwordFixup_T uof_impExpr_T;

typedef struct{
   unsigned int     name;               /**< import var name string-table offset */
   unsigned int     value;
   unsigned char    valueAttrs;         /**< bit<0> (Scope: 0=global), bit<1> (init: 0=no, 1=yes) */
   unsigned char    reserved1;          /**< reserved for future use */
   unsigned short   reserved2;          /**< reserved for future use */
}uof_importVar_T;

typedef struct uof_aeReg_S{
   unsigned int     name;               /**< reg name string-table offset */
   unsigned int     visName;            /**< reg visible name string-table offset */
   unsigned short   type;               /**< reg type */
   unsigned short   addr;               /**< reg address */
   unsigned short   accessMode;         /**< uof_RegAccessMode_T: read/write/both/undef */
   unsigned char    visible;            /**< register visibility */
   unsigned char    reserved1;          /**< reserved for future use */
   unsigned short   refCount;           /**< number of contiguous registers allocated */
   unsigned short   reserved2;          /**< reserved for future use */
   unsigned int     xoId;               /**< xfer order ID */
}uof_aeReg_T;


typedef struct uof_codeArea_S{
   unsigned int numMicroWords;          /**< number of micro words */
   unsigned int uwordBlockTab;          /**< offset to ublock table */
}uof_codeArea_T;

typedef struct uof_uWordBlock_S{
    unsigned int startAddr;             /**< start address */
    unsigned int numWords;              /**< number of microwords */
    unsigned int uwordOffset;           /**< offset to the uwords */
}uof_uWordBlock_T;

enum ENDIAN_TYPE
{
    ENDIAN_SWAP = 0,                   /**< endian swap */  
    NO_ENDIAN_SWAP,                    /**< no endian swap */  
    NOT_SET                            /**< endian swap not set */  
};

#ifdef __cplusplus
}
#endif

#endif  /* __UOF_H__ */
