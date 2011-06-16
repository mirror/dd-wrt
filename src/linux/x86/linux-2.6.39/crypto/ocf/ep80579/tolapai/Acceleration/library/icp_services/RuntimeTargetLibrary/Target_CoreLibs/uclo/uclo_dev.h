/**
 **************************************************************************
 * @file uclo_dev.h
 *
 * @description
 *      This is the header file for Ucode Object File Loader Library
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

#ifndef __UCLO_DEV_H
#define __UCLO_DEV_H

#include "uof.h"
#include "uclo_overlay.h"
#include "IxOsal.h"

/* micro-instruction length */
#define UCLO_BITS_IN_UWORDS      44
/* minimum .uof file size */
#define UCLO_MIN_UOF_SIZE        24
/* invalid micro-instruction */
#define INVLD_UWORD              0xffffffffffull
/* bad AccelEngine */
#define UCLO_BADAE               0xffffffff
/* maximux character size of import variable  */
#define MAX_VARNAME              1024
/* maximum number of physical AE addresses */
#define MAX_AE                   0x18   
/* maximum micro-store size for EP80579 */
#define ICP_MAX_USTORE           (16*1024)

/* get neighbor AE */
#define UCLO_AE_NEIGHBOR(ae, numEngs) \
 ((unsigned int)((ae)+1) < (numEngs) ? ((ae)+1) : UCLO_BADAE)

/* The following structures are used to encapsulate the uof image--they will
   be mapped over the uof image in memory */
typedef struct uof_encapUofObj_S{
   char               *begUof;              /* begin of .uof */
   uof_objHdr_T       *objHdr;              /* object header pointer */
   uof_chunkHdr_T     *chunkHdr;            /* object chunk pointer */
   uof_varMemSeg_T    *varMemSeg;           /* variable segment pointer */
}uof_encapUofObj_T;

typedef struct uof_encapUwBlock_S{            
    unsigned int        startAddr;          /* start address */ 
    unsigned int        numWords;           /* number of micro-words */
    char                *microWords;        /* packed microWords */
}uof_encapUwBlock_T;

typedef struct uof_encapPage_S{
   unsigned int        pageNum;             /* page number */
   unsigned int        defPage;             /* default page */
   unsigned int        pageRegion;          /* region of page */
   unsigned int        begVirtAddr;         /* begin virtual address */ 
   unsigned int        begPhyAddr;          /* begin physical address */
   unsigned int        numUcVar;            /* num of uC var in array */
   uof_ucVar_T         *ucVar;              /* array of import variables */
   unsigned int        numImpVar;           /* num of import var in array */
   uof_importVar_T     *impVar;             /* array of import variables */
   unsigned int        numImpExpr;          /* num of import expr in array */
   uof_uwordFixup_T    *impExpr;            /* array of import expressions */
   unsigned int        numNeighReg;         /* num of neigh-reg in array */
   uof_neighReg_T      *neighReg;           /* array of neigh-reg assignments */
   unsigned int        numMicroWords;       /* number of microwords in the seg */

   unsigned int        numUwBlocks;         /* number of uword blocks */
   uof_encapUwBlock_T  *uwBlocks;           /* array of uword blocks */
}uof_encapPage_T;

typedef struct uof_encapAe_S{
   uof_Image_T          *imagePtr;          /* image pointer */
   uof_encapPage_T      *pages;             /* array of pages */
   unsigned int         numAeReg;           /* num of registers */
   uof_aeReg_T          *aeReg;             /* array of registers */
   unsigned int         numInitRegSym;      /* num of reg/sym init values */
   uof_initRegSym_T     *initRegSym;        /* array of reg/sym init values */
   unsigned int         numSbreak;          /* num of sbreak values */
   uof_sbreak_T         *sbreak;            /* array of sbreak values */
   unsigned int         numUwordsUsed;      /* highest uword address referenced + 1 */
}uof_encapAe_T;

typedef struct uof_initMemTab_S{
    unsigned int    numEntries;             /* number of memory init entries */
    uof_initMem_T   *initMem;               /* array of init memory values */
}uof_initMemTab_T;

typedef struct uclo_objHdr_S{
    char            *fBuf;                  /* object header buffer pointer */
    unsigned int    checksum;               /* object checksum */
    unsigned int    size;                   /* object size */
}uclo_objHdr_T;

/* uclo_objHandle represents one uof file */
typedef struct uclo_objHandle_S{
    unsigned int        prodType;                          /* ptoduct type */
    unsigned int        prodRev;                           /* product revision */
    short               uofMinVer;                         /* .uof minor version */
    short               uofMajVer;                         /* .uof major version */ 
    uclo_objHdr_T       *objHdr;                           /* object header pointer */
    uof_encapUofObj_T   encapUofObj;                       /* .uof object */
    uof_strTab_T        strTable;                          /* string table */
                /* uniqueAeImage:Dense array of size numUniqueImage, each image appears 
                   once in the list, in UOF file order. To account for mixing modules,
                   we must allow for one list image for each thread.
                   imagePtr->aeAssigned is a mask indicating which AEs are associated 
                   with this image. Set by UcLo_mapImage() */
    uof_encapAe_T       uniqueAeImage[UOF_MAX_NUM_OF_AE * MAX_CONTEXTS];   /* .uof image */
    UcLo_AeData_t       aeData[UOF_MAX_NUM_OF_AE];        /* indexed by swAe */

    unsigned char       hwAeNum[UOF_MAX_NUM_OF_AE];       /* indexed by hwAe */
    unsigned char       swAe[MAX_AE];                     /* software AE */
    uof_initMemTab_T    initMemTab;                       /* memory initialize table */ 
    int                 numUniqueImage;                   /* unique image number */
    int                 objCopied;                        /* copied object size */ 
    int                 readOnly;                         /* readonly flag */
    int                 uWordBytes;                       /* micro-word bytes size */
    int                 globalInited;                     /* global initialization flag */
    unsigned int        numAEs;                           /* AE number */
    unsigned int        numCtxPerAe;                      /* context number per AE */
    unsigned int        ustorePhySize;                    /* micro-store physical size */
 
    uclo_objHdr_T       *dbgObjHdr;                       /* debug object header pointer */  
    uof_strTab_T        dbgObjStrTable;                   /* string table for debug object */

    uclo_objHdr_T       *prfObjHdr;                       /* profile object header pointer */  
    uof_strTab_T        prfObjStrTable;                   /* string table for profile object */

    void                *bkptCallbackHandle;              /* callback handle for breakpoint */  
    IxOsalMutex         overlayMutex;                     /* mutex for code overlay */  
    unsigned int        pausingAeMask;                    /* AE pause mask */  
    void                *objBuf;                          /* obj buffer pointer for readonly image */  
}uclo_objHandle_T;

typedef struct uof_dbgInfo_S{
    unsigned int     validBkPt;                           /* valid breakpoint indicator */
    unsigned char    deferCount;                          /* this instruction's defer count */
    short            brAddr;                              /* branch label address */
    short            regAddr;                             /* register address */
    unsigned short   regType;                             /* register type */
}uof_dbgInfo_T;

typedef struct uof_dbgInfoImage_S{
    unsigned int    aeAssigned;                           /* assigned AE */
    unsigned short  numDbgInfo;                           /* number of debug inforamtion */
    unsigned short  reserved1;                            /* reserved */
    uof_dbgInfo_T   *dbgInfo;                             /* debug information pointer */
}uof_dbgInfoImage_T;

typedef struct{
    unsigned int        numImage;                         /* number of image */
    uof_dbgInfoImage_T *dbgInfoImage[UOF_MAX_NUM_OF_AE];  /* debug information image */
}uof_aeDbgImage_T;

#if defined(__cplusplus)
extern "C" {
#endif

void *UcLo_findChunk(uof_objHdr_T *objHdr, char *chunkId, void *cur);

uof_aeReg_T *UcLo_getAeRegName(uclo_objHandle_T *objHandle, uof_encapAe_T *encapAeImage,
                                char *regName, unsigned int type);
uof_importVar_T *UcLo_getImportVar(uclo_objHandle_T *objHandle, unsigned char ae, char *varName);

uof_initMem_T *UcLo_findMemSym(uclo_objHandle_T *objHandle, char *symName);

int UcLo_writeUimagePageRaw(uclo_objHandle_T *objHandle,
                            uof_encapPage_T  *encapPage,
                            unsigned int      swAe);
unsigned int UcLo_getAeMode(uclo_objHandle_T *objHandle, int ae);

#if defined(__cplusplus)
}
#endif

#endif        /* __UCLO_DEV_H */
