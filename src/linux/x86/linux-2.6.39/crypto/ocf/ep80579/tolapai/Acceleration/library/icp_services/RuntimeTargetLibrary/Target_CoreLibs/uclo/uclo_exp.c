/**
 **************************************************************************
 * @file uclo_exp.c
 *
 * @description
 *      This file provides Ucode Object File Loader facilities
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

#include "uclo.h"
#include "uclo_dev.h"
#include "hal_ae.h"
#include "hal_global.h"
#include "halAeApi.h"
#include "uclo_helper.h"

#define EXPR_SIZE         1024     /* maximum size of expression */    
#define MAX_SYMNAME       512      /* maximum size of symobl name */    
#define UCLOEXP_MAX_QSIZE 32       /* maximum size of expression queue */    
#define UCLO_BADREG       -6       /* bad register ID */    

#define SRAM_BASEADDRESS  0xFFFC0000 /* sram base address */    
  
enum {UCLO_OPERAND,   /* operand kind */    
      UCLO_ADD,       /* '+' operator */
      UCLO_MULT,      /* '*' operator */
      UCLO_SUBT,      /* '-' operator */
      UCLO_DIV,       /* '/' operator */
      UCLO_MOD,       /* '%' operator */
      UCLO_MINUS,     /* '-' operator */
      UCLO_AND,       /* '&&' operator */
      UCLO_BITAND,    /* '&' operator */
      UCLO_OR,        /* '||' operator */
      UCLO_BITOR,     /* '|' operator */
      UCLO_EQU,       /* '==' operator */
      UCLO_NEQU,      /* '!=' operator */
      UCLO_NOT,       /* '!' operator */
      UCLO_XOR,       /* '^' operator */
      UCLO_LT,        /* '<' operator */
      UCLO_LTEQU,     /* '<=' operator */
      UCLO_GT,        /* '>' operator */
      UCLO_GTEQU,     /* '>=' operator */ 
      UCLO_RSHFT,     /* '>>' operator */
      UCLO_LSHFT,     /* '<<' operator */
      UCLO_INV,       /* '~' operator */
      UCLO_TRIARY,    /* '?' operator */ 
      UCLO_NEIGHREG,  /* operator of getting address of neighbor register */
      UCLO_IMPVALUE,  /* operator of getting imported value */
      UCLO_AEREG,     /* operator of getting address of register in given AE */
      UCLO_SYMADDR,   /* operator of getting address of a symbol */
      UCLO_LEGACY_MODE, /* legacy mode */
      UCLO_CONTEXT_NUM  /* number of context 4 or 8 */
};

enum {EXPQ_OK=0,    /* expression queue is ok */    
      EXPQ_FULL,    /* expression queue is full */    
      EXPQ_EMPTY,   /* expression queue is empty */    
      EXPQ_BADARG}; /* expression queue is in bad status */    

typedef enum {
    OPRND_BAD,      /* operand is bad */    
    OPRND_SYM,      /* operand is symbol */    
    OPRND_INT       /* operand is integer */    
}OprndTyp_t;

typedef struct Oprnd_S{
    OprndTyp_t type;   /* operand type */    
    union expVar{
        int  ii;       /* operand integer value */    
        char *sym;     /* operand symbol string */    
    }value;            /* operand value */    
}Oprnd_t;

typedef struct{
    Oprnd_t      pQ[UCLOEXP_MAX_QSIZE];  /* expression queue element */    
    unsigned int pQsize;                 /* expression queue size */    
    unsigned int pQhead;                 /* expression queue head pointer */    
}ExpQue_t;

static int ExpErr;
unsigned int UcLo_getSwAe(unsigned int prodType, unsigned char aeNum);
unsigned int UcLo_getHwAe(unsigned int prodType, unsigned char aeNum);

void UcloExp_queInit(ExpQue_t *pQueue);
int UcloExp_queSize(ExpQue_t *pQueue);
int UcloExp_quePush(ExpQue_t *pQueue, Oprnd_t *pValue);
int UcloExp_quePop(ExpQue_t *pQueue, Oprnd_t *pValue);
int UcloExp_getImpValue(uclo_objHandle_T *pObjHandle, unsigned char swAe,
                        char *pVarName, Oprnd_t *pResult);
int UcloExp_getSymAddr(uclo_objHandle_T *pObjHandle, unsigned char swAe, 
                       char *pSymName, Oprnd_t *pResult);
int UcloExp_getAeRegMode(uclo_objHandle_T *pObjHandle, int hwAe, 
                         char *pRegName, unsigned short *pAeMode);
int UcloExp_getAeRegAddr(uclo_objHandle_T *pObjHandle, int hwAe, char *pRegName);
int UcloExp_getNeighRegAddr(uclo_objHandle_T *pObjHandle, int swAe,
                            char *pRegName, Oprnd_t *pResult);
Oprnd_t *UcloExp_parseOperand(char *pToken, Oprnd_t *pOprnd);
int UcloExp_oprndKind(char *pToken);
int UcloExp_doTriary(int unOp, Oprnd_t *pOp1, Oprnd_t *pOp2, 
                     Oprnd_t *pOp3, Oprnd_t *pResult);
int UcloExp_doBinary(uclo_objHandle_T *pObjHandle, int biOp, Oprnd_t *pOp1, Oprnd_t *pOp2, Oprnd_t *pResult);
int UcloExp_doUnary(uclo_objHandle_T *pObjHandle, unsigned char swAe, int unOp,
                    Oprnd_t *pOp1, Oprnd_t *pResult);
int UcloExp_evalPostfix(uclo_objHandle_T *pObjHandle, unsigned char swAe,
                        char *pExp, int *pRes);
                        
/*-----------------------------------------------------------------------------
   Function:       UcloExp_queInit
   Description:    Init an expression queue
   Returns:        n/a
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
void 
UcloExp_queInit(ExpQue_t *pQueue)
{
    pQueue->pQsize = UCLOEXP_MAX_QSIZE;
    pQueue->pQhead = 0;
    return;
}

/*-----------------------------------------------------------------------------
   Function:       UcloExp_queSize
   Description:    Determine if the que size
   Returns:        the size of the que
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcloExp_queSize(ExpQue_t *pQueue)
{
    return (pQueue->pQhead);
}

/*-----------------------------------------------------------------------------
   Function:       UcloExp_quePush
   Description:    Push a value on the queue
   Returns:        n/a
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcloExp_quePush(ExpQue_t *pQueue, 
                Oprnd_t *pValue)
{
    if(pQueue->pQhead >= pQueue->pQsize)
    {
       return (EXPQ_FULL);
    }    
    pQueue->pQ[pQueue->pQhead].type = pValue->type;
    pQueue->pQ[pQueue->pQhead].value = pValue->value;
    pQueue->pQhead++;
    return (EXPQ_OK);
}

/*-----------------------------------------------------------------------------
   Function:       UcloExp_quePop
   Description:    Pop a value from the queue
   Returns:        n/a
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int
UcloExp_quePop(ExpQue_t *pQueue, 
               Oprnd_t *pValue)
{
    if(!pValue) 
    {
        return (EXPQ_BADARG);
    }    
    if(pQueue->pQhead == 0)
    {
        return (EXPQ_EMPTY);
    }    
    pQueue->pQhead--;
    pValue->type = pQueue->pQ[pQueue->pQhead].type;
    pValue->value = pQueue->pQ[pQueue->pQhead].value;
    return (EXPQ_OK);
}

/*-----------------------------------------------------------------------------
   Function:       UcloExp_getImpValue
   Description:    get the value assigned to an import variable
   Returns:        UCLO_SUCCESS, UCLO_UNINITVAR, UCLO_FAILURE,
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcloExp_getImpValue(uclo_objHandle_T *pObjHandle,  
                    unsigned char swAe,
                    char *pVarName, 
                    Oprnd_t *pResult)
{
    uof_importVar_T        *pImpVar = NULL;

    if(!(pImpVar = UcLo_getImportVar(pObjHandle, swAe, pVarName)))
    {
        return (UCLO_FAILURE);
    }

    pResult->value.ii = pImpVar->value;
    pResult->type = OPRND_INT;
    if((!GET_FIXUP_ASSIGN(pImpVar->valueAttrs))) 
    {
        return (UCLO_UNINITVAR);
    }    

    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:       UcloExp_getSymAddr
   Description:    get the address assigned to a symbol
   Returns:        UCLO_SUCCESS, UCLO_FAILURE,
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcloExp_getSymAddr(uclo_objHandle_T *pObjHandle, 
                   unsigned char swAe, 
                   char *pSymName, 
                   Oprnd_t *pResult)
{
    uof_initMem_T    *pInitMem = NULL;
    char            localSym[MAX_SYMNAME];
    unsigned int    hwAe = 0, ss = 0;
    unsigned int    baseAddr = 0;
    unsigned int ScratchOffset = 0, SramOffset = 0, NCDramOffset = 0, CDramOffset = 0;

    halAe_GetMemoryStartOffset(&ScratchOffset, &SramOffset, 
                               &NCDramOffset, &CDramOffset); 
    
    if((swAe >= UOF_MAX_NUM_OF_AE) || (hwAe = UcLo_getHwAe(pObjHandle->prodType, swAe)) == UCLO_BADAE) 
    {
        return (UCLO_EXPRFAIL);
    }    

    /* if found, symbol is global */
    if(!(pInitMem = UcLo_findMemSym(pObjHandle, pSymName)))
    {
        /* not global so search all slice -- symbol should be unique accross slices */
        for(ss=0; ss < pObjHandle->aeData[swAe].numSlices; ss++)
        {
            /* if found, symbol is local */
            if(pObjHandle->aeData[swAe].aeSlice[ss].encapImage->imagePtr)
            {
                LOCAL_NAME(localSym, pSymName, hwAe);
                if((pInitMem = UcLo_findMemSym(pObjHandle, localSym)))
                {
                    break;       
                }    
            }
        }
        if(!pInitMem) 
        {
            return (UCLO_EXPRFAIL);
        }    
    }

    switch(pInitMem->region)
    {
    case SRAM0_REGION: 
            /* SRAM0 region starts from fixed address 0xFFFC0000 from AE view */
            pResult->value.ii = pInitMem->addr + SramOffset + SRAM_BASEADDRESS; 
            break;

    case DRAM_REGION: 
            halAe_GetNCDramBaseAddr(&baseAddr);
            pResult->value.ii = pInitMem->addr + NCDramOffset + baseAddr;
            break;

    case DRAM1_REGION: 
            halAe_GetCDramBaseAddr(&baseAddr);
            pResult->value.ii = pInitMem->addr + CDramOffset + baseAddr;
            break;

    case SCRATCH_REGION:
            baseAddr = 0;
            pResult->value.ii = pInitMem->addr + ScratchOffset + baseAddr;
            break;
	    
    default: 
            /* other regions require no special handling */
            pResult->value.ii = pInitMem->addr; 
            break;
    }
    pResult->type = OPRND_INT;

    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:       UcloExp_getAeRegMode
   Description:    get the aeMode associated with the register
   Returns:        the aeMode, or UCLO_BADREG
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcloExp_getAeRegMode(uclo_objHandle_T *pObjHandle, 
                    int hwAe, 
                    char *pRegName, 
                    unsigned short *pAeMode)
{
    uof_aeReg_T        *pAeReg = NULL;
    unsigned int    swAe = 0, ss = 0;

    if(pAeMode==NULL) 
    {
        return (UCLO_BADREG);
    }
    if((swAe = UcLo_getSwAe(pObjHandle->prodType, (unsigned char)hwAe)) == UCLO_BADAE) 
    {
        return (UCLO_EXPRFAIL);
    }    

    for(ss=0; ss<pObjHandle->aeData[swAe].numSlices; ss++){
        /* ensure the register existing in each slice of specified AE */
        if(!(pAeReg = UcLo_getAeRegName(pObjHandle, 
                          pObjHandle->aeData[swAe].aeSlice[ss].encapImage,
                          pRegName, ICP_ANY_REG))) 
        {
            break;
        }                            
    }
    if(!pAeReg) 
    {
        return (UCLO_BADREG);
    }    

    if(pAeMode) 
    {
        *pAeMode = pObjHandle->aeData[swAe].aeSlice[ss-1].encapImage->imagePtr->aeMode;
    }    
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:       UcloExp_getAeRegAddr
   Description:    get the address associated with the register
   Returns:        the register address, or UCLO_BADREG
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcloExp_getAeRegAddr(uclo_objHandle_T *pObjHandle, 
                     int hwAe, 
                     char *pRegName)
{
    uof_aeReg_T        *pAeReg = 0;
    unsigned int    swAe = 0, ss = 0;

    if((swAe = UcLo_getSwAe(pObjHandle->prodType, (unsigned char)hwAe)) == UCLO_BADAE) 
    {
        return (UCLO_EXPRFAIL);
    }    

    for(ss=0; ss<pObjHandle->aeData[swAe].numSlices; ss++)
    {
        if(!(pAeReg = UcLo_getAeRegName(pObjHandle, 
                         pObjHandle->aeData[swAe].aeSlice[ss].encapImage,
                         pRegName, ICP_ANY_REG))) 
        {
             break;
        }     
    }
    if(!pAeReg) 
    {
        return (UCLO_BADREG);
    }    
    return (pAeReg->addr);
}

/*-----------------------------------------------------------------------------
   Function:       UcloExp_getNeighRegAddr
   Description:    get the address associated with the neighbor-register
   Returns:        the register address, or UCLO_BADREG
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcloExp_getNeighRegAddr(uclo_objHandle_T *pObjHandle, 
                        int swAe,
                        char *pRegName, 
                        Oprnd_t *pResult)
{
    uof_aeReg_T  *pAeReg = NULL;
    unsigned int ss = 0;

    if((swAe >= UOF_MAX_NUM_OF_AE) || (UCLO_AE_NEIGHBOR(swAe, pObjHandle->numAEs)== UCLO_BADAE))
    {
        return (UCLO_FAILURE);
    }    

    for(ss=0; ss<pObjHandle->aeData[swAe].numSlices; ss++)
    {
        if(!(pAeReg = UcLo_getAeRegName(pObjHandle, 
                          pObjHandle->aeData[swAe].aeSlice[ss].encapImage,
                          pRegName, ICP_NEIGH_REL))) 
        {
            break;
        }    
    }
    if(!pAeReg) 
    {
        return (UCLO_BADREG);
    }    

    pResult->value.ii = pAeReg->addr;
    pResult->type = OPRND_INT;
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:       UcloExp_parseOperand
   Description:    parse an operand and set its type
   Returns:        pointer to the operand
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
Oprnd_t *
UcloExp_parseOperand(char *pToken, 
                     Oprnd_t *pOprnd)
{
    int num = 0;

    if(UcLo_parseNum(pToken, &num))
    {
        pOprnd->value.sym = pToken;
        pOprnd->type = OPRND_SYM;
    }
    else
    {
        pOprnd->value.ii = num;
        pOprnd->type = OPRND_INT;
    }
    return (pOprnd);
}

/*-----------------------------------------------------------------------------
   Function:       UcloExp_oprndKind
   Description:    identify the operand
   Returns:        identity of the operand
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcloExp_oprndKind(char *pToken)
{
    if(!strcmp("+", pToken)) 
    {        
        return (UCLO_ADD);
    }    
    if(!strcmp("-", pToken)) 
    {
        return (UCLO_SUBT);
    }    
    if(!strcmp("*", pToken))
    {
        return (UCLO_MULT);
    }    
    if(!strcmp("/", pToken)) 
    {
        return (UCLO_DIV);
    }    
    if(!strcmp("-u", pToken)) 
    {
         return (UCLO_MINUS);
    }     
    if(!strcmp("&", pToken)) 
    {
        return (UCLO_BITAND);
    }    
    if(!strcmp("|", pToken)) 
    {
        return (UCLO_BITOR);
    }    
    if(!strcmp("&&", pToken)) 
    {
        return (UCLO_AND);
    }    
    if(!strcmp("||", pToken)) 
    {
        return (UCLO_OR);
    }    
    if(!strcmp("==", pToken)) 
    {
        return (UCLO_EQU);
    }    
    if(!strcmp("!=", pToken)) 
    {
        return (UCLO_NEQU);
    }    
    if(!strcmp("<", pToken)) 
    {
        return (UCLO_LT);
    }    
    if(!strcmp("<=", pToken)) 
    { 
        return (UCLO_LTEQU);
    }    
    if(!strcmp(">", pToken)) 
    {
        return (UCLO_GT);
    }    
    if(!strcmp(">=", pToken)) 
    {
        return (UCLO_GTEQU);
    }    
    if(!strcmp(">>", pToken)) 
    {
        return (UCLO_RSHFT);
    }    
    if(!strcmp("<<", pToken)) 
    {
        return (UCLO_LSHFT);
    }    
    if(!strcmp("^", pToken)) 
    {
        return (UCLO_XOR);
    }    
    if(!strcmp("!", pToken)) 
    {
        return (UCLO_NOT);
    }    
    if(!strcmp("%", pToken)) 
    {
        return (UCLO_MOD);
    }    
    if(!strcmp("~", pToken)) 
    {
        return (UCLO_INV);
    }    
    if(!strcmp("?", pToken)) 
    {
        return (UCLO_TRIARY);
    }    
    if(!strcmp("&n", pToken)) 
    {
        return (UCLO_NEIGHREG);    /* address of neighbor register */
    }    
    if(!strcmp("&i", pToken)) 
    {
        return (UCLO_IMPVALUE);    /* imported value */
    }    
    if(!strcmp("&r", pToken)) 
    {
        return (UCLO_AEREG);       /* address of register in given AE */
    }    
    if(!strcmp("&v", pToken)) 
    {
        return (UCLO_SYMADDR);     /* address of a symbol */
    }    
    if(!strcmp("&l", pToken)) 
    {
        return (UCLO_LEGACY_MODE);  /* 1 for legacy on and 0 for legacy off */
    }    
    if(!strcmp("&c", pToken)) 
    {
        return (UCLO_CONTEXT_NUM);  /* number of context--4 or 8 */
    }    
    return (UCLO_OPERAND);
}

/*-----------------------------------------------------------------------------
   Function:     UcloExp_doTriary
   Description:  Evaluate Triary expressions
   Returns:      UCLO_SUCCESS, UCLO_FAILURE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcloExp_doTriary(int unOp, 
                 Oprnd_t *pOp1, 
                 Oprnd_t *pOp2, 
                 Oprnd_t *pOp3, 
                 Oprnd_t *pResult)
{
    if((pOp1->type == OPRND_INT) && (pOp2->type == OPRND_INT) && \
       (pOp3->type == OPRND_INT))
    {
        switch(unOp)
        {
        case UCLO_TRIARY:
                pResult->value.ii = (pOp1->value.ii) ? pOp2->value.ii : pOp3->value.ii;
                pResult->type = OPRND_INT;
                break;
        default:
                ExpErr++;
                break;
        }
        return (UCLO_SUCCESS);
    }
    return (UCLO_EXPRFAIL);
}

/*-----------------------------------------------------------------------------
   Function:     UcloExp_doBinary
   Description:  Evaluate Binary expressions
   Returns:      UCLO_FAILURE, UCLO_SUCCESS
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcloExp_doBinary(uclo_objHandle_T *pObjHandle, 
                 int biOp, 
                 Oprnd_t *pOp1, 
                 Oprnd_t *pOp2, 
                 Oprnd_t *pResult)
{
    unsigned int temp;
    
    if((pOp1->type == OPRND_INT) && ((pOp2->type == OPRND_INT)))
    {
        switch(biOp)
        {
        case UCLO_ADD:  pResult->value.ii = (pOp1->value.ii + pOp2->value.ii); break;
        case UCLO_SUBT: pResult->value.ii = (pOp1->value.ii - pOp2->value.ii); break;
        case UCLO_MULT: pResult->value.ii = (pOp1->value.ii * pOp2->value.ii); break;
        case UCLO_DIV:
                if(pOp2->value.ii == 0) 
                {
                    return (UCLO_FAILURE);
                }    
                pResult->value.ii = (pOp1->value.ii / pOp2->value.ii);
                break;
        case UCLO_BITAND: pResult->value.ii = (pOp1->value.ii & pOp2->value.ii); break;
        case UCLO_BITOR: pResult->value.ii = (pOp1->value.ii | pOp2->value.ii); break;
        case UCLO_AND:    pResult->value.ii = (pOp1->value.ii &&  pOp2->value.ii); break;
        case UCLO_OR:    pResult->value.ii = (pOp1->value.ii || pOp2->value.ii); break;
        case UCLO_EQU:    pResult->value.ii = (pOp1->value.ii == pOp2->value.ii); break;
        case UCLO_NEQU:    pResult->value.ii = (pOp1->value.ii != pOp2->value.ii); break;
        case UCLO_XOR:    pResult->value.ii = (pOp1->value.ii ^ pOp2->value.ii); break;
        case UCLO_LT:    pResult->value.ii = (pOp1->value.ii < pOp2->value.ii); break;
        case UCLO_LTEQU: pResult->value.ii = (pOp1->value.ii <= pOp2->value.ii); break;
        case UCLO_GT:    pResult->value.ii = (pOp1->value.ii > pOp2->value.ii); break;
        case UCLO_GTEQU: pResult->value.ii = (pOp1->value.ii >= pOp2->value.ii); break;
        case UCLO_RSHFT: 
                {   
                    /* rigth shift a unsigned integter*/
                    temp = (unsigned int)pOp1->value.ii;
                    temp = (temp >> pOp2->value.ii);
                    pResult->value.ii = (int)temp;
                    break;
                }    
        case UCLO_LSHFT: pResult->value.ii = (pOp1->value.ii << pOp2->value.ii); break;
        case UCLO_MOD:
                if(pOp2->value.ii == 0) 
                {
                   return (UCLO_EXPRFAIL);
                }   
                pResult->value.ii = (pOp1->value.ii % pOp2->value.ii);
                break;
        default:
                ExpErr++;
                return (UCLO_EXPRFAIL);
                break;
        }
        pResult->type = OPRND_INT;
        return (UCLO_SUCCESS);
    }

    if((biOp == UCLO_AEREG) && (pOp1->type == OPRND_INT) && ((pOp2->type == OPRND_SYM)))
    {
        if((pResult->value.ii = UcloExp_getAeRegAddr(pObjHandle, 
                                                     pOp1->value.ii, 
                                                     pOp2->value.sym)) != UCLO_BADREG)
        {
            pResult->type = OPRND_INT;
            return (UCLO_SUCCESS);
        }
    }

    return (UCLO_EXPRFAIL);
}

/*-----------------------------------------------------------------------------
   Function:     UcldInFile::doUnary
   Description:  Evaluate Unary expressions
   Returns:      UCLO_SUCCESS, UCLO_FAILURE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcloExp_doUnary(uclo_objHandle_T *pObjHandle,  
                unsigned char swAe, 
                int unOp,
                Oprnd_t *pOp1, 
                Oprnd_t *pResult)
{
    if(pOp1->type == OPRND_INT)
    {
        switch(unOp)
        {
        case UCLO_MINUS:    pResult->value.ii = -pOp1->value.ii; break;
        case UCLO_NOT:    pResult->value.ii = !pOp1->value.ii; break;
        case UCLO_INV:    pResult->value.ii = ~pOp1->value.ii; break;
        default:
                ExpErr++;
                return (UCLO_EXPRFAIL);
                break;
        }
        pResult->type = OPRND_INT;
        return (UCLO_SUCCESS);
    }
    if(pOp1->type == OPRND_SYM)
    {
        switch(unOp)
        {
        case UCLO_IMPVALUE:
                return (UcloExp_getImpValue(pObjHandle, swAe, pOp1->value.sym, pResult));
        case UCLO_SYMADDR:
                return (UcloExp_getSymAddr(pObjHandle, swAe, pOp1->value.sym, pResult));
        case UCLO_NEIGHREG:
                return (UcloExp_getNeighRegAddr(pObjHandle, swAe, pOp1->value.sym, pResult));
        default:
                ExpErr++;
                return (UCLO_EXPRFAIL);
        }
    }

    return (UCLO_EXPRFAIL);
}

/*-----------------------------------------------------------------------------
   Function:    UcloExp_evalPostfix
   Description: Evaluate Postfix expressions
   Returns:     UCLO_SUCCESS, UCLO_FAILURE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcloExp_evalPostfix(uclo_objHandle_T *pObjHandle, 
                    unsigned char swAe,
                    char *pExp, 
                    int *pRes)
{
    char *savPtr = NULL, *expPtr = NULL, *pToken = NULL, *reflectRegName = NULL;
    unsigned int op = 0;
    int stat=UCLO_FAILURE;
    Oprnd_t x, y, z, pResult;
    ExpQue_t queue;
    int reflectMeNum, reflectCtxNum, reflectLegacyNum;
    unsigned short aeMode;   
    
    char *expression = NULL, *tmp_str = NULL;
    expression = (char *)ixOsalMemAlloc(EXPR_SIZE);
    if(expression == NULL)
    {
        return (UCLO_FAILURE);
    }    
    tmp_str = (char *)ixOsalMemAlloc(EXPR_SIZE);
    if(tmp_str == NULL) 
    {
        ixOsalMemFree(expression);
        return (UCLO_FAILURE);
    }    

    if(pExp == NULL) 
    {
       ixOsalMemFree(expression);
       ixOsalMemFree(tmp_str);
       return (UCLO_FAILURE);
    }

    strncpy(expression, pExp, EXPR_SIZE-1);
    expPtr = expression;
    ExpErr = 0;
    UcloExp_queInit(&queue);

    while((ExpErr == 0) && (pToken = UcLo_strtoken(expPtr, " \t\n", &savPtr)))
    {
        switch((op = UcloExp_oprndKind(pToken)))
        {
        case UCLO_OPERAND:
                if(UcloExp_quePush(&queue, UcloExp_parseOperand(pToken, &x)))
                {
                    ExpErr++;
                }    
                break;
        case UCLO_IMPVALUE:
        case UCLO_SYMADDR:
        case UCLO_MINUS:
        case UCLO_NOT:
        case UCLO_INV:
        case UCLO_NEIGHREG:
                if(UcloExp_quePop(&queue, &x)) 
                {
                    ExpErr++;
                }    
                else
                {
                    if((stat = UcloExp_doUnary(pObjHandle, swAe, op, &x, &x)) == UCLO_SUCCESS)
                    {
                        /* pResult is returned int x */
                        if(UcloExp_quePush(&queue, &x)) 
                        {
                            ExpErr++;
                        }    
                    }
                    else 
                    {
                        ExpErr++;
                    }    
                }
                break;
        case UCLO_LEGACY_MODE:
                {
                    if(UcloExp_quePop(&queue,&x))
                    {
                        ExpErr++;
                    }    
                    reflectRegName = x.value.sym;
                    if(UcloExp_quePop(&queue,&x)) 
                    {
                        ExpErr++;
                    }    
                    reflectMeNum = x.value.ii;

                    if(UcloExp_getAeRegMode(pObjHandle,reflectMeNum,reflectRegName, &aeMode) == UCLO_SUCCESS) 
                    {
                        reflectLegacyNum = LEGACY_MODE(aeMode);
                    }    
                    else
                    {
                        reflectLegacyNum = 0;
                        ExpErr++;
                    }

                    snprintf(tmp_str,sizeof(tmp_str),"%d",reflectLegacyNum);
                    if(UcloExp_quePush(&queue,UcloExp_parseOperand(tmp_str, &x))) 
                    {
                        ExpErr++;
                    }    
                    break;
                }
        case UCLO_CONTEXT_NUM:
                {
                    if(UcloExp_quePop(&queue,&x))
                    {
                        ExpErr++;
                    }    
                    reflectMeNum = x.value.ii;

                    reflectCtxNum = CTX_MODE(UcLo_getAeMode(pObjHandle,reflectMeNum));
                    snprintf(tmp_str,sizeof(tmp_str),"%d",reflectCtxNum);
                    if(UcloExp_quePush(&queue,UcloExp_parseOperand(tmp_str, &x))) 
                    {
                        ExpErr++;
                    }    
                    break;
                }
        case UCLO_ADD:
        case UCLO_SUBT:
        case UCLO_MULT:
        case UCLO_DIV:
        case UCLO_BITAND:
        case UCLO_BITOR:
        case UCLO_AND:
        case UCLO_OR:
        case UCLO_EQU:
        case UCLO_NEQU:
        case UCLO_XOR:
        case UCLO_LT:
        case UCLO_LTEQU:
        case UCLO_GT:
        case UCLO_GTEQU:
        case UCLO_RSHFT:
        case UCLO_LSHFT:
        case UCLO_MOD:
        case UCLO_AEREG:
                if(UcloExp_quePop(&queue, &y)) 
                {
                   ExpErr++;
                }
                else if(UcloExp_quePop(&queue, &x)) 
                {
                   ExpErr++;
                }
                else if((stat = UcloExp_doBinary(pObjHandle, op, &x, &y, &x)) == UCLO_SUCCESS)
                {
                   /* pResult is returned int x */
                   if(UcloExp_quePush(&queue, &x)) 
                   {
                       ExpErr++;
                   }    
                }
                else 
                {
                    ExpErr++;
                }  
                break;
        case UCLO_TRIARY: 
                if(UcloExp_quePop(&queue, &z)) 
                {
                   ExpErr++;
                }     
                else if(UcloExp_quePop(&queue, &y)) 
                {
                   ExpErr++;
                }
                else if(UcloExp_quePop(&queue, &x)) 
                {
                   ExpErr++;
                } 
                else if((stat = UcloExp_doTriary(op, &x, &y, &z, &x)) == UCLO_SUCCESS)
                {
                   /* pResult is returned int x */
                   if(UcloExp_quePush(&queue, &x)) 
                   {
                        ExpErr++;
                   }    
                }
                else 
                {
                   ExpErr++;
                }
                break;
        default:
                ExpErr++;
                break;
        }
        expPtr = savPtr;
    }

    if(ExpErr || (UcloExp_queSize(&queue) != 1)) 
    {
        ixOsalMemFree(expression);
        ixOsalMemFree(tmp_str);
        return (stat);
    }

    UcloExp_quePop(&queue, &pResult);
    *pRes = pResult.value.ii;

    ixOsalMemFree(expression);
    ixOsalMemFree(tmp_str);

    return (UCLO_SUCCESS);
}

