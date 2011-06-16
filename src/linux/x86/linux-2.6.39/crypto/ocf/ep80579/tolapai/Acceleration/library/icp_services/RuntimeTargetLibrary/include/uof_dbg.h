/**
 **************************************************************************
 * @file uof_dbg.h
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
 * @file uof_dbg.h
 * 
 * @defgroup uof_dbg UOF Microcode File Format Definition for Debug Information
 *                   Chunk
 * 
 * @ingroup UOF
 *
 * @description
 *      This header file that contains the debug information chunk definitions 
 *      of micorcode file format used by linker and loader.
 *
 *****************************************************************************/
 
#ifndef __UOF_DBG_H__
#define __UOF_DBG_H__

#include "uof.h"
#include "dbgAeInfo.h"

typedef enum{
    LitleEndian,                    /**< little-endian */
    BigEndian                       /**< big-endian */
}Ucld_EndianTyp;

typedef enum{
    GlobalScope,                    /**< global scope */
    FileScope,                      /**< file scope */
    FuncScope,                      /**< function scope */
    BlockScope                      /**< block scope */
}Ucld_ScopeType;

typedef enum{
    Int1,                           /**< interger type 1 */
    Uint1,                          /**< unsigned interger type 1 */
    Int2,                           /**< interger type 2 */
    Uint2,                          /**< unsigned interger type 2 */
    Int4,                           /**< interger type 4 */
    Uint4,                          /**< unsigned interger type 4 */
    Int8,                           /**< interger type 8 */
    Uint8,                          /**< unsigned interger type 8 */
    Enum,                           /**< enum type */
    Reg,                            /**< register type */
    Sig,                            /**< signal type */
    Struct,                         /**< structure type */
    Array,                          /**< array type */
    ScratchPtr,                     /**< scratch memory type */
    Sram0Ptr,                       /**< sram memory type */
    DramPtr,                        /**< dram memory type */
    RfifoPtr,                       /**< receive fifo type */
    TfifoPtr,                       /**< transmission fifo type */
    LmemPtr,                        /**< local memory type */
    UmemPtr,                        /**< micro ustore type */
    AbstractPtr                     /**< abstract pointer type */

}Ucld_TypeType;

typedef enum{
    AeReg=0x52,                     /**< AE register */
    AeGlobal=0x53,                  /**< shared symbol */
    AeCtxLocal=0x54,                /**< thread local variables */
    Rfifo,                          /**< receive FIFO */
    Tfifo,                          /**< transmit FIFO */
    LocalMem,                       /**< shared localmemory */
    Optimized,                      /**< optimized variable */
    MultiStorage,                   /**< stored in multiple storage types */
	ScratchRing,					/**< scratch ring */
	NNring,							/**< NN ring */
    Sram0Ring,                      /**< sram0 ring */
    RamRing,                        /**< ram ET ring */
    ConstValue,                     /**< constant value liverange */
    AddrTaken                       /**< address taken liverange */
}Ucld_VarLocType;

typedef enum{
    UdefTool,                       /**< unspecified tool type */
    Uca,                            /**< assembler */
    Mcpcom                          /**< uC compiler */
}Ucld_LstFileToolType;

#define DBG_OBJS    "DBG_OBJS"      /**< debug section ID */
#define DBG_IMAG    "DBG_IMAG"      /**< debug image section ID */
#define DBG_STRT    "DBG_STRT"      /**< debug string section ID */ 
#define DBG_SYMB    "DBG_SYMB"      /**< debug string table section ID */

typedef struct dbg_ObjTable_S{
   unsigned int     numEntries;     /**< num of entries in table */
   /* numEntries * sizeof(element) follows */
}dgb_ObjTable_T;

typedef dgb_ObjTable_T dbg_RegTab_T;
typedef dgb_ObjTable_T dbg_LblTab_T;
typedef dgb_ObjTable_T dbg_SymTab_T;
typedef dgb_ObjTable_T dbg_SrcTab_T;
typedef dgb_ObjTable_T dbg_TypTab_T;
typedef dgb_ObjTable_T dbg_ScopeTab_T;

typedef uof_strTab_T    dbg_strTab;
typedef uof_chunkHdr_T  dbg_chunkHdr_T;


typedef struct dbg_InstOprnd_S{
    unsigned int    addr;           /**< micro address of instruction */
    int             src1Name;       /**< source operand 1 reg name offset in string table */
    int             src1Addr;       /**< source operand 1 reg address offset in string table */
    int             src2Name;       /**< source operand 2 reg name offset in string table */
    int             src2Addr;       /**< source operand 2 reg address offset in string table */
    int             destName;       /**< destination reg name offset in string table */
    int             destAddr;       /**< destination reg address offset in string table */
    int             xferName;       /**< xfer reg name offset in string table */
    int             xferAddr;       /**< xfer reg address offset in string table */
    unsigned int    mask;           /**< <31:3>unused, <2>I/O indirect, <1>I/O read, <0> I/O write */
    unsigned char   refCount;       /**< I/O reference count (1 to 16) */
    unsigned char   deferCount;     /**< branch defer count (0-3) */
    unsigned short  reserved1;      /**< reserved for future use */   
    unsigned int    reserved2;      /**< reserved for future use */   
}dbg_InstOprnd_T;

typedef struct dbg_Image_S{
   int          lstFileName;        /**< list file name str-table offset */
   unsigned int aeAssigned;         /**< bit values of assigned AEs */
   unsigned int ctxAssigned;        /**< bit values of assigned AE contexts */
   unsigned char lstFileCreatedBy;  /**< Ucld_LstFileToolType: tool list file was created by */
   unsigned char sharedCsMode;      /**< shared control-store mode -- on/off */
   unsigned char ctxMode;           /**< number of contexts -- 4 or 8 */
   unsigned char endianMode;        /**< endian: little=0, big=1 */
   unsigned int scopeTabOffset;     /**< uC scope table offset */
   unsigned int regTabSize;         /**< registers table size */
   unsigned int lblTabSize;         /**< labels table size */
   unsigned int srcTabSize;         /**< source lines table size */
   unsigned int regTabOffset;       /**< registers table offset */
   unsigned int lblTabOffset;       /**< labels table offset */
   unsigned int srcTabOffset;       /**< source lines table offset */
   unsigned int typTabSize;         /**< uC type table size */
   unsigned int scopeTabSize;       /**< uC scope table size */
   unsigned int typTabOffset;       /**< uC type table offset */      
   unsigned int instOprndTabSize;   /**< instruction operands table size */
   unsigned int instOprndTabOffset; /**< instruction operands table offset */     
   unsigned int reserved2;          /**< reserved for future use */
}dbg_Image_T;


typedef uof_aeReg_T dbg_Reg_T;

typedef struct dbg_Label_S{
   int          name;               /**< label name string-table offset */
   int          addr;               /**< label address */
}dbg_Label_T;

typedef struct dbg_Source_S{
   int              fileName;       /**< source file name */
   int              lines;          /**< source lines str-table offset */
   unsigned int     lineNum;        /**< source line number */
   unsigned int     addr;           /**< associated micro address */
   unsigned int     validBkPt;      /**< valid breakpoint indicator */
   int              brAddr;         /**< branch label address */
   unsigned char    ctxArbKill;     /**< this instruction's is a ctx_arb[kill] */
   unsigned char    reserved1;      /**< reserved for future use */
   short            regAddr;        /**< register address */
   unsigned short   regType;        /**< register type */
   unsigned short   deferCount;     /**< this instruction's defer count */
   int              funcInsId;      /**< function instantiation ID */
}dbg_Source_T;


typedef struct dbg_Symb_S{
   int              name;           /**< symbol name string-table offset */
   unsigned char    scope;          /**< scope -- global=0, local=1 */
   unsigned char    region;         /**< selected icp_RegType_T values: IXP_SRx_MEM_ADDR,
                                       IXP_DR_MEM_ADDR, IXP_SCRATCH_MEM_ADDR, IXP_LMEM_ADDR,
                                       IXP_UMEM_ADDR, IXP_SRAM0_IMPORTED, IXP_SRAM1_IMPORTED,
                                       IXP_SRAM2_IMPORTED, IXP_SRAM3_IMPORTED, IXP_DRAM_IMPORTED,
                                       IXP_DRAM1_IMPORTED, IXP_SCRATCH_IMPORTED */
   unsigned short   reserved;       /**< reserved for future use */
   unsigned int     addr;           /**< symbol memory location */
   unsigned int     byteSize;       /**< size of the symbol */
}dbg_Symb_T;


typedef struct dbg_Type_S{
   int              name;           /**< type name string-table offset */
   unsigned short   typeId;         /**< id of type -- Ucld_TypeType */
   unsigned short   type;           /**< type referenced -- could be itself */
   unsigned int     size;           /**< size/bound of the type */
   unsigned int     defOffset;      /**< offset to dbg_StructDef_T or dbg_EnumDef_T */
}dbg_Type_T;


/* used for Ucld_TypeType Struct */
typedef struct dbg_StructDef_S{
   unsigned short   numFields;      /**< number of fields */
   unsigned short   reserved;       /**< reserved for future use */
   unsigned int     fieldOffset;    /**< offset to dbg_StructField_T */
}dbg_StructDef_T;

typedef struct dbg_StructField_S{
   int              name;           /**< field name string-table offset */
   unsigned int     offset;         /**< offset from beg of struct */
   unsigned short   type;           /**< field type */
   unsigned char    bitOffset;      /**< bitOffset */
   unsigned char    bitSize;        /**< bitSize */
}dbg_StructField_T;


/* used for Ucld_TypeType Enum */
typedef struct dbg_EnumDef_S{
   unsigned short   numValues;      /**< number of values */
   unsigned short   reserved;       /**< reserved for future use */
   unsigned int     valueOffset;    /**< offset to dbg_EnumValue_T */
}dbg_EnumDef_T;

typedef struct dbg_EnumValue_S{
   int              name;           /**< enum value name string-table offset */
   int              value;          /**< enum value */
   unsigned int     reserved;       /**< reserved for future use */
}dbg_EnumValue_T;


typedef struct dbg_Scope_S{
   int              name;           /**< scope name string-table offset */
   int              interFuncName;  /**< for funcScope: internal function name string-table offset */
   int              fileName;       /**< file name string-table offset */
   unsigned int     inLinedLine;    /**< for funcScope: source line the function was inlined */
   unsigned int		uwordBeg;       /**< scope in effect at uword */
   unsigned int		uwordEnd;       /**< scope stops at uword */
   unsigned short   lineBeg;        /**< scope in effect at source line */
   unsigned short   lineEnd;        /**< scope stops at source line */
   unsigned int     openCurly;      /**< the source line of "{" for the function */
   unsigned short   type;           /**< Ucld_ScopeType -- global, file, funct, ect.. */
   unsigned short   numScopes;      /**< number of dbg_Scope_T within this scope */
   unsigned short   numVars;        /**< number of variables in this scope */
   unsigned short   numFuncRet;     /**< for funcScope: number of function-return locations */
   unsigned int     scopeOffset;    /**< offset to dbg_Scope_T within this scope */
   unsigned int     varOffset;      /**< offset to dbg_Variable_T within this scope */
   unsigned int     funcRetOffset;  /**< func return value offset to dbg_ValueLoc_T */
   int              funcInsId;      /**< func instantiation ID */
   unsigned short   numEntryEdges;    /**< # of function entry edge uaddrs. 0 = no edges */
   unsigned int     entryEdgesOffset; /**< offset to array of function entry edges uaddrs (ints) */
   unsigned short   numExitEdges;     /**< # of function exit edge uaddrs. 0 = no edges */
   unsigned int     exitEdgesOffset;  /**< offset to array of function exit edge uaddrs (ints) */
}dbg_Scope_T;


typedef struct dbg_ValueLoc_S{
   unsigned int     locId;          /**< icp_RegType_T -- reg, mem, spill, uword */
   int              symbName;       /**< symbol name offset to string-table */
   unsigned int     location;       /**< memAddr, regNum, uwordAddr, or spill-offset */
   unsigned int     multiplier;     /**< spill multiplier */
}dbg_ValueLoc_T;


typedef struct dbg_Variable_S{
   int              name;           /**< variable name offset to string-table */
   unsigned short   type;           /**< type to refer */
   unsigned char    reserved;       /**< reserved for future use */
   unsigned char    locType;        /**< location type:- Ucld_VarLocType */
   unsigned int     definedOnLine;  /**< line var was defined */
   unsigned int     locOffset;      /**< offset to dbg_Sloc_T, dbg_Tloc_T, dbg_RlocTab_T, or dbg_Lmloc_T, dbg_Ring_T */
}dbg_Variable_T;


typedef struct dbg_Ring_S{
   int              symbName;       /**< symbolic name offset to string-table */
   unsigned int     memOffset;      /**< memory region offset */
   unsigned int     id;             /**< scratch/sram ring number */
   unsigned int     size;           /**< ring byte size */
}dbg_Ring_T;



typedef struct dbg_Sloc_S{
   int              symbName;       /**< symbol name offset to string-table */
   unsigned int     offset;         /**< offset from the symbol address */
}dbg_Sloc_T;


typedef struct dbg_Tloc_S{
   int              symbName;       /**< symbol name offset to string-table */
   unsigned int     offset;         /**< local mem offset */
   unsigned int     multiplier;     /**< local mem multiplier */
}dbg_Tloc_T;

typedef struct dbg_Lmloc_S{
   unsigned int     offset;         /**< localmemory offset */
}dbg_Lmloc_T;


typedef struct dbg_RlocTab_S{
   unsigned short   numEntries;     /**< number of live ranges */
   unsigned short   reserved;       /**< reserved for future use */
   /* numEntries of dbg_Liverange_T follows */
}dbg_RlocTab_T;

typedef struct dbg_Liverange_S{
   unsigned int     offset;         /**< byte offset from var */
   unsigned int     size;           /**< bit size */
   unsigned int     locId;          /**< icp_RegType_T -- reg, mem, spill, uword, const */
   unsigned int     regNumOrOffset; /**< reg-num, spill-offset, or constant value */
   unsigned int     multiplier;     /**< spill -- multiplier */
   unsigned int     symName;        /**< spill -- symbol name (sram0$tls, localmem$tls, etc..) */
   unsigned short   numRanges;      /**< number of ranges */
   unsigned char    ambiguous;      /**< the location may or may not contain valid value */
   unsigned char    allocType;      /**< shared: MeGlobal, ThreadLocal: MeCtxLocal */
   unsigned int     offsetInLocation;   /**< bit offset from the base of current location */
   unsigned int     rangeOffset;    /**< offset to dbg_Range_T */
}dbg_Liverange_T;

typedef struct dbg_Range_S{
   unsigned int start;              /**< range start position */
   unsigned int stop;               /**< range stop position */
}dbg_Range_T;

#if defined(__cplusplus)
  extern "C" {
#endif
int UcLo_getDbgInfo(uof_objHdr_T *dbgObjHdr,
                    dbgAeInfo_Image_T *meDbgImage,
                    uof_strTab_T *strTable);
void UcLo_deleDbgInfo(dbgAeInfo_Image_T *meDbgImage);
#if defined(__cplusplus)
  }
#endif

#endif          // __UOF_DBG_H__
