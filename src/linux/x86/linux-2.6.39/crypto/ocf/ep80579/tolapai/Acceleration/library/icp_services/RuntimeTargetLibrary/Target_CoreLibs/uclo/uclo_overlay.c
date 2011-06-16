/**
 **************************************************************************
 * @file uclo_overlay.c
 *
 * @description
 *      This file provides Ucode Object File Loader facilities: overlay support
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

#include "uclo_platform.h"
#include "halAeApi.h"
#include "uclo.h"
#include "uclo_dev.h"

#define MAX_SW_AE_NUM  15       /* maximum software AE number */

void UcLo_ProcessHalCallbacks(Hal_UcloCallReason_T reason,
                         unsigned int         arg0,
                         unsigned int*        arg1,
                         void*                user_data);                       
static void UcLo_AssociatePage(UcLo_AeSlice_t *aeSlice, int pageNum, int regionNum);
static void addPage(UcLo_PageList_t* pPageList, UcLo_Page_t* pPage);
static UcLo_Page_t* removePage(UcLo_PageList_t* pageList);
static void processCtxArbKill(uclo_objHandle_T *objHandle,
                  unsigned int hwAeNum, unsigned int swAe,
                  unsigned int ctx, int *startingPageChange);
static void processOverlayBr(uclo_objHandle_T *objHandle,
                 unsigned int hwAeNum, unsigned int swAe,
                 unsigned int ctx, unsigned int target_va,
                 int *startingPageChange);
static void updateCtxEnables(uclo_objHandle_T *objHandle,
                 unsigned int aeMask,    
                 unsigned int *meEnMask);     
static void pausingMEs(uclo_objHandle_T *objHandle,unsigned int aeMask);                              

void UcLo_preIntrCallback(void);
void UcLo_postIntrCallback(void);
void UcLo_kickStartArbiter(unsigned int hwAeNum);

/*-----------------------------------------------------------------------------
   Function:    removePage
   Description: Remove a page from the head of the list
   Returns:     page removed or NULL
-----------------------------------------------------------------------------*/
static UcLo_Page_t*
removePage(UcLo_PageList_t* pPageList)
{
    UcLo_Page_t* pResult=NULL;

    pResult = pPageList->head;
    if (pResult) 
    {
        pPageList->head = pResult->next;
        if (pPageList->head == NULL) 
        {
            pPageList->tail = NULL;
        }    
        pResult->next = NULL;
    }
    return (pResult);
}


/*-----------------------------------------------------------------------------
   Function:    addPage
   Description: Add a page to the tail of the list
   Returns:     void
-----------------------------------------------------------------------------*/
static void
addPage(UcLo_PageList_t* pPageList, 
        UcLo_Page_t* pPage)
{
    if (pPageList->tail == NULL) 
    {
        pPageList->tail = pPageList->head = pPage;
    } else 
    {
        pPageList->tail = pPageList->tail->next = pPage;
    }
    pPage->next = NULL;
}


/*-----------------------------------------------------------------------------
   Function:    UcLo_AssociatePage
   Description: Associate a region with a page
   Returns:     void
-----------------------------------------------------------------------------*/
static void 
UcLo_AssociatePage(UcLo_AeSlice_t *pMeSlice, 
                   int pageNum, 
                   int regionNum)
{
    pMeSlice->pages[pageNum].region = &pMeSlice->regions[regionNum];
    /*    addPage(&pAeData->regions[regionNum].notWaiting, &pAeData->pages[pageNum]); */
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_ClearAeData
   Description: Initialize UcLo_MeData structure as unused
   Returns:     UCLO_SUCCESS, UCLO_BADARG
-----------------------------------------------------------------------------*/
int 
UcLo_ClearAeData(UcLo_AeData_t *pAeData)
{
    unsigned int ss = 0;

    if(pAeData == NULL)
    {
    	return (UCLO_BADARG);
    }    
    
    for(ss=0; ss < pAeData->numSlices; ss++)
    {
        pAeData->aeSlice[ss].regions = NULL;
        pAeData->aeSlice[ss].pages   = NULL;
        pAeData->aeSlice[ss].encapImage = NULL;
    }
    pAeData->numSlices = 0;
    pAeData->numPageXref = 0;
    pAeData->pageXref = NULL;
    pAeData->relocUstoreDram = (unsigned int)-1;
    return (UCLO_SUCCESS);
}


/*-----------------------------------------------------------------------------
   Function:    UcLo_AssignHalPages
   Description: 
   Returns:     UCLO_SUCCESS, UCLO_MEMFAIL, UCLO_BADARG
-----------------------------------------------------------------------------*/
int 
UcLo_AssignHalPages(UcLo_AeData_t *pAeData, 
                    unsigned int hwAeNum)
{
    int ii=0, halPageNum=0, numPages=0, cc=0;
    unsigned int ss=0;

    if(pAeData == NULL)
    {
    	return (UCLO_BADARG);
    }    
    
    for(ss = 0; ss < pAeData->numSlices; ss++)
    {
        numPages += pAeData->aeSlice[ss].encapImage->imagePtr->numOfPages;
    }
    pAeData->numPageXref = numPages;
    if(numPages == 0) 
    {
        return (UCLO_SUCCESS);
    }
    if(!(pAeData->pageXref = (UcLo_PageXref_T *)ixOsalMemAlloc(sizeof(UcLo_PageXref_T) * pAeData->numPageXref))) 
    {
        return (UCLO_MEMFAIL);
    }    
    ixOsalMemSet(pAeData->pageXref, (UINT8)cc, sizeof(UcLo_PageXref_T) * pAeData->numPageXref);

    halAe_SetNumPages((unsigned char)hwAeNum, pAeData->numPageXref);
    for(ss = 0; ss < pAeData->numSlices; ss++)
    {
        for(ii = 0; ii < pAeData->aeSlice[ss].encapImage->imagePtr->numOfPages; ii++)
        {
            halAe_SetPageData((unsigned char)hwAeNum, halPageNum,
                            pAeData->aeSlice[ss].pages[ii].encapPage->begVirtAddr,
                            pAeData->aeSlice[ss].pages[ii].encapPage->begPhyAddr,
                            pAeData->aeSlice[ss].pages[ii].encapPage->numMicroWords);
            pAeData->aeSlice[ss].pages[ii].halPageNum = halPageNum;
            pAeData->pageXref[halPageNum].meSliceIndex = ss;
            pAeData->pageXref[halPageNum].pageNum = ii;
            halPageNum++;
        }
    }
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_InitAeData
   Description: Initialize UcLo_MeData structure, and associate pages
                with regions.
   Returns:     UCLO_SUCCESS, UCLO_MEMFAIL, UCLO_BADOBJ, UCLO_BADARG
-----------------------------------------------------------------------------*/
int 
UcLo_InitAeData(uclo_objHandle_T *objHandle, 
                unsigned int swAe, 
                unsigned int imageNum)
{
    UcLo_AeData_t *pAeData;
    uof_encapAe_T *pEncapImage;
    int ii, numRegions = 0, numPages = 0;
    unsigned int ss=0;
    UcLo_Region_t   *pRegion=NULL;
    UcLo_Page_t     *pPage=NULL;
    uof_encapPage_T *pEncapPage=NULL;
    uof_initMem_T    *pInitMem=NULL;
    char localName[MAX_VARNAME], *pUstoreDramBase="__USTORE_DRAM_BASE";

    if(objHandle == NULL)
    {
    	return (UCLO_BADARG);
    }    
    pAeData = &objHandle->aeData[swAe];
    pEncapImage = &objHandle->uniqueAeImage[imageNum];

    pAeData->relocUstoreDram = 0;
    pAeData->shareableUstore = 0;
    pAeData->effectUstoreSize = 0;
    ss = pAeData->numSlices;
    pAeData->aeSlice[ss].encapImage = pEncapImage;

    if(pEncapImage->imagePtr)
    {
        pAeData->aeSlice[ss].assignedCtxMask = pEncapImage->imagePtr->ctxAssigned;

        if(RELOADABLE_CTX_SHARED_MODE(pEncapImage->imagePtr->aeMode))
        {
            pAeData->effectUstoreSize = ICP_MAX_USTORE;
            LOCAL_NAME(localName, pUstoreDramBase, objHandle->hwAeNum[swAe]);
            if((pInitMem = UcLo_findMemSym(objHandle, localName))) 
            {
                pAeData->relocUstoreDram = pInitMem->addr;
            }    
            else 
            {
                 return (UCLO_BADOBJ);
            }
        }
        else
        {
            if((pAeData->shareableUstore = SHARED_USTORE_MODE(pEncapImage->imagePtr->aeMode))) 
            {
                pAeData->effectUstoreSize = objHandle->ustorePhySize << 1;   /* twice as much ustore space */
            }    
            else 
            {
                pAeData->effectUstoreSize = objHandle->ustorePhySize;
            }
        }

        numRegions = pEncapImage->imagePtr->numOfPageRegions;
        numPages   = pEncapImage->imagePtr->numOfPages;
    }
    else
    {
        pAeData->aeSlice[ss].assignedCtxMask = 0;
    }

    /* Allocate regions */
    pAeData->aeSlice[ss].regions = (UcLo_Region_t*) ixOsalMemAlloc(numRegions * sizeof(UcLo_Region_t));
    if (pAeData->aeSlice[ss].regions == NULL)
        return (UCLO_MEMFAIL);

    /* Initialize regions */
    for (ii=0; ii<numRegions; ii++) 
    {
        pRegion = &pAeData->aeSlice[ss].regions[ii];
        pRegion->loaded = NULL;
        pRegion->waitingPageIn.head = pRegion->waitingPageIn.tail = NULL;
        /* pRegion->notWaiting.head    = pRegion->notWaiting.tail    = NULL; */
    }

    /* Allocate pages */
    pAeData->aeSlice[ss].pages = (UcLo_Page_t*) ixOsalMemAlloc(numPages  * sizeof(UcLo_Page_t));
    if (pAeData->aeSlice[ss].pages == NULL) 
    {
        ixOsalMemFree(pAeData->aeSlice[ss].regions);
        pAeData->aeSlice[ss].regions = NULL;
        return (UCLO_MEMFAIL);
    }


    /* Initialize pages and associate them with regions */
    for (ii=0; ii<numPages; ii++) 
    {
        pPage = &pAeData->aeSlice[ss].pages[ii];
        pPage->next = NULL;
        pPage->encapPage = pEncapPage = &pEncapImage->pages[ii];
        pPage->flags = 0;
        pPage->halPageNum = (unsigned int)(-1);

        UcLo_AssociatePage(&pAeData->aeSlice[ss], pEncapPage->pageNum, pEncapPage->pageRegion);
    }

    /* Initialize other AE fields */
    for (ii=0; ii<MAX_CONTEXTS; ii++) 
    {
        pAeData->aeSlice[ss].currentPage[ii] = NULL;
        pAeData->aeSlice[ss].newUaddr[ii] = 0;
    }
    pAeData->numSlices++;

    return (UCLO_SUCCESS);
}


/*-----------------------------------------------------------------------------
   Function:    UcLo_FreeAeData
   Description: Free resources associated with Overlay AE data
   Returns:     UCLO_SUCCESS, UCLO_BADARG
-----------------------------------------------------------------------------*/
int 
UcLo_FreeAeData(UcLo_AeData_t *pAeData)
{
    unsigned int ss=0;

    if(pAeData == NULL)
    {
    	return (UCLO_BADARG);
    }    
    
    for(ss=0; ss < pAeData->numSlices; ss++)
    {
        if(pAeData->aeSlice[ss].regions) 
        {
            ixOsalMemFree(pAeData->aeSlice[ss].regions);
        }    
        pAeData->aeSlice[ss].regions = NULL;

        if(pAeData->aeSlice[ss].pages) 
        {
            ixOsalMemFree(pAeData->aeSlice[ss].pages);
        }    
        pAeData->aeSlice[ss].pages = NULL;
    }
    if(pAeData->pageXref) 
    {
        ixOsalMemFree(pAeData->pageXref);
    }
    pAeData->pageXref = NULL;

    pAeData->relocUstoreDram = 0;
    return (UCLO_SUCCESS);
}


/*-----------------------------------------------------------------------------
   Function:    UcLo_PageOut
   Description: Check whether the specified page is loaded,
                if it isn't, add that page to those waiting to be paged in and
                mark that ctx as waiting
   Returns:     UCLO_PO_LOADED, UCLO_PO_WAITING
-----------------------------------------------------------------------------*/
UcLo_PAGEOUT_t 
UcLo_PageOut(UcLo_AeData_t *pAeData,
             unsigned int  ctx,
             unsigned int  newPageNum, /* New page that CTX is switching to */
             unsigned int  hwAeNum)
{
    UcLo_Page_t   *pNewPage=NULL;
    UcLo_Region_t *pNewRegion=NULL;
    unsigned int ss=0;

    if(pAeData == NULL)
    {
    	return (UCLO_PO_WAITING);
    }    
    
    /* find which slice the ctx is assigned */
    for(ss=0; ss < pAeData->numSlices; ss++)
    {
        if(pAeData->aeSlice[ss].assignedCtxMask & (1<<ctx)) 
        {
            break;
        }    
    }

    pNewPage = &pAeData->aeSlice[ss].pages[newPageNum];
    pNewRegion = pNewPage->region;

    /* Change current page for this context */
    pAeData->aeSlice[ss].currentPage[ctx] = pNewPage;

    if (pNewPage == pNewRegion->loaded) 
    {
        /* page is loaded */
        halAe_PutCtxWakeupEvents((unsigned char)hwAeNum, 1<<ctx, XCWE_VOLUNTARY);
        return (UCLO_PO_LOADED);
    }
    /* page is not loaded */
    /* put page on waiting list unless it is already on it */
    if (!(pNewPage->flags & UCLO_PAGEFLAGS_WAITING)) 
    {
        pNewPage->flags |= UCLO_PAGEFLAGS_WAITING;
        addPage(&pNewRegion->waitingPageIn, pNewPage);
    }
    return (UCLO_PO_WAITING);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_doPageIn
   Description: Actually copy a page to ustore
   Returns:     UCLO_SUCCESS, UCLO_*
-----------------------------------------------------------------------------*/
int 
UcLo_doPageIn(uclo_objHandle_T *objHandle, 
              unsigned int swAe,
              UcLo_Page_t  *pPage, 
              UcLo_Page_t  *oldPage, 
              int *startingPageChange)
{
    int status=0;
    unsigned int hwAeNum=0, oldPageNum=0;

    hwAeNum = objHandle->hwAeNum[swAe];

    if (*startingPageChange == 0) 
    {
        *startingPageChange = 1;
        halAe_CallNewPageCallback(START_OF_PAGE_CHANGE, 0, 0, 0);
    }

    /* Load the new page */
    status = UcLo_writeUimagePageRaw(objHandle, pPage->encapPage, swAe);
    if (oldPage == NULL) 
    {
        oldPageNum = (unsigned int)(-1);
    }    
    else
    {
        oldPageNum = oldPage->halPageNum;
    }    

    halAe_CallNewPageCallback(NEW_PAGE_LOADED, hwAeNum, pPage->halPageNum, oldPageNum);

    pPage->region->loaded = pPage;

    return (status);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_PageInRegion
   Description: See if a new context needs to be paged in for the specified
                region. If the loaded page is not being used by any context and
                there are waiting contexts, page in the waiting context.
   Returns:     UCLO_SUCCESS, UCLO_BADARG
-----------------------------------------------------------------------------*/
int 
UcLo_PageInRegion(uclo_objHandle_T *objHandle,
                  unsigned int      swAe,
                  UcLo_Region_t    *pRegion,
                  int              *startingPageChange)
{
    UcLo_Page_t   *pOldPage=NULL, *pNewPage=NULL;
    int ctx=0;
    UcLo_AeData_t *pAeData;
    unsigned int ss=0, hwAeNum=0;

    if((objHandle == NULL) || (pRegion == NULL) || (startingPageChange == NULL) || (swAe >= UOF_MAX_NUM_OF_AE))
    {
    	return (UCLO_BADARG);
    }    
    
    pAeData = &objHandle->aeData[swAe];
    if(pAeData == NULL)
    {
        return (UCLO_SUCCESS);
    }

    pOldPage = pRegion->loaded;
    /* this page/region has been handled by neighbour AE */
    if(objHandle->aeData[swAe].shareableUstore && (pOldPage == NULL)) 
    {
        return (UCLO_SUCCESS);
    }
    ss = pAeData->pageXref[pRegion->loaded->halPageNum].meSliceIndex;


    for (ctx=0; ctx<MAX_CONTEXTS; ctx++)
    {
        if(!(pAeData->aeSlice[ss].assignedCtxMask & (1<<ctx))) 
        {
             continue;
        }     
        if (pOldPage == pAeData->aeSlice[ss].currentPage[ctx]) 
        {
            /* region's page is still being used; don't do a page swap */
            return (UCLO_SUCCESS);
        }
    } /* end for ctx */

    /* Region's loaded page is not being used */
    pNewPage = removePage(&pRegion->waitingPageIn);
    if (pNewPage == NULL) 
    {
        /* no waiting page, don't do a page swap */
        return (UCLO_SUCCESS);
    }
    pNewPage->flags &= ~UCLO_PAGEFLAGS_WAITING;
    UcLo_doPageIn(objHandle, swAe, pNewPage, pOldPage, startingPageChange);

    hwAeNum = objHandle->hwAeNum[swAe];
    /* update data structures */
    for (ctx=0; ctx<MAX_CONTEXTS; ctx++) 
    {
        if(!(pAeData->aeSlice[ss].assignedCtxMask & (1<<ctx))) 
        {
             continue;
        }     
        if (pAeData->aeSlice[ss].currentPage[ctx] == pNewPage) 
        {
            /* context's page is paged in, mark it as OK to run */
            /* first set its PC */
            halAe_PutPC((unsigned char)hwAeNum, 1<<ctx, pAeData->aeSlice[ss].newUaddr[ctx]);
            halAe_PutCtxWakeupEvents((unsigned char)hwAeNum, 1<<ctx, XCWE_VOLUNTARY);
            halAe_CallNewPageCallback(WAITING_FOR_PAGE_CHANGE, hwAeNum,
                                    ctx, (unsigned int)(-1));
        }
    }
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    processOverlayBr
   Description: Do processing for a request to change pages
   Returns:     void
-----------------------------------------------------------------------------*/
static void 
processOverlayBr(uclo_objHandle_T *objHandle,
                 unsigned int hwAeNum, 
                 unsigned int swAe,
                 unsigned int ctx,
                 unsigned int target_va,
                 int *startingPageChange)
{
    unsigned int target_pa=0, pageNum=0, loaded=0, ss=0;
    UcLo_AeData_t *pAeData = &objHandle->aeData[swAe];
    UcLo_PAGEOUT_t pageOutStatus=0;
    UcLo_Region_t *pOld_region=NULL, *pNew_region=NULL;

    halAe_MapVirtToPhysUaddrEx((unsigned char)hwAeNum, target_va, &target_pa, &loaded, &pageNum);
    
    if(pAeData == NULL)
    {
        return;
    }
    
    /* get the uclo page and slice number */
    ss = pAeData->pageXref[pageNum].meSliceIndex;
    pageNum = pAeData->pageXref[pageNum].pageNum;

    /* find which slice the ctx is assigned */
    pOld_region = pAeData->aeSlice[ss].currentPage[ctx]->region;
    pageOutStatus = UcLo_PageOut(pAeData, ctx, pageNum, hwAeNum);
    pNew_region = pAeData->aeSlice[ss].currentPage[ctx]->region;

    /* Update PC for this context */
    if (pageOutStatus == UCLO_PO_LOADED) 
    {
        halAe_PutPC((unsigned char)hwAeNum, (1<<ctx), target_pa);
    }    
    else 
    {
        /* Defer updating the PC until it gets paged in */
        pAeData->aeSlice[ss].newUaddr[ctx] = target_pa;
        halAe_CallNewPageCallback(WAITING_FOR_PAGE_CHANGE, hwAeNum, ctx, target_va);
    }

    /* See if old region or new region needs to do a page-in */
    UcLo_PageInRegion(objHandle, swAe, pOld_region, startingPageChange);
    /* if new new page was already loaded, we don't need to do a page-in
       on the new region */
    if ((pageOutStatus != UCLO_PO_LOADED) && (pNew_region != pOld_region)) 
    {
        UcLo_PageInRegion(objHandle, swAe, pNew_region, startingPageChange);
    }    
}

/*-----------------------------------------------------------------------------
   Function:    processCtxArbKill
   Description: Do processing for a ctx_arb[kill] encoded as a bkpt
   Returns:     void
-----------------------------------------------------------------------------*/
static void
processCtxArbKill(uclo_objHandle_T *objHandle,
                  unsigned int hwAeNum, 
                  unsigned int swAe,
                  unsigned int ctx, 
                  int *startingPageChange)
{
    UcLo_Region_t *pOld_region=NULL;
    UcLo_AeData_t *pAeData = &objHandle->aeData[swAe];
    unsigned int ss=0;
    
    if(pAeData == NULL)
    {
        return;
    }
    
    /* find which slice the ctx is assigned */
    for(ss=0; ss < pAeData->numSlices; ss++)
    {
        if(pAeData->aeSlice[ss].assignedCtxMask & (1<<ctx)) 
        {
            break;
        }    
    }
    pOld_region = pAeData->aeSlice[ss].currentPage[ctx]->region;
    pAeData->aeSlice[ss].currentPage[ctx] = NULL;
    /* zero will be wrong if page-0 is not loaded, but its the best we can do */
    halAe_PutPC((unsigned char)hwAeNum, (1<<ctx), 0);
    /* See if old region or new region needs to do a page-in */
    UcLo_PageInRegion(objHandle, swAe, pOld_region, startingPageChange);
    /* Don't re-enable the wakeup-events */
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_IntrCallback
   Description: Process AE Interrupt callbacks for UCLO
   Returns:     void
-----------------------------------------------------------------------------*/
void
UcLo_IntrCallback(Hal_IntrMasks_T *masks, 
                  void* data)
{
    unsigned int hwAeNum=0, swAe=0;
    unsigned int actCtx=0, ctx=0, uPc_p=0, uPc_v=0, ii=0, target_va=0;
    uclo_objHandle_T *objHandle = (uclo_objHandle_T*)data;
    uof_encapAe_T *pEncapImage=NULL;
    uof_sbreak_T  *pSbreak=NULL;
    unsigned int restartMask=0, ss=0;
    int startingPageChange=0;
    unsigned int enableCtxMask=0;

    UcLo_preIntrCallback();

    restartMask = 0;
    startingPageChange = 0;

    MUTEX_LOCK(objHandle->overlayMutex);
    
    for (hwAeNum = 0; hwAeNum < UCLO_MAX_AES; hwAeNum++) 
    {
        if (!(masks->attn_bkpt_mask & (1<<hwAeNum))) 
        {
            continue;
        }    
        
        /* This AE has a breakpoint */

        /* get the active ctx and pc */
        halAe_GetAeCsr((unsigned char)hwAeNum, ACTIVE_CTX_STATUS, &actCtx);
        if(actCtx & ACS_ABO) 
        {
           continue;
        }   
        ctx = ACS_ACNO & actCtx;
        halAe_GetPC((unsigned char)hwAeNum, (unsigned char)ctx, &uPc_p);
        /* Subtract one from physical PC to get addr of bkpt */
        /* Wrap to ustore size */
        uPc_p = (uPc_p - 1) & (objHandle->ustorePhySize - 1);
        halAe_MapPhysToVirtUaddr((unsigned char)hwAeNum, uPc_p, &uPc_v);

        /* find the slice the ctx is assigned */
        swAe = objHandle->swAe[hwAeNum];
        if(swAe >= UOF_MAX_NUM_OF_AE)
        {
            continue;
        }
        for(ss=0; ss < objHandle->aeData[swAe].numSlices; ss++) 
        {
            if(objHandle->aeData[swAe].aeSlice[ss].assignedCtxMask & (1<<ctx)) 
            {
                break;
            }
        }    
            
        if ( ss == objHandle->aeData[swAe].numSlices) 
        {
             continue;
        }     
        /* See if it matches an pSbreak */
        pEncapImage = objHandle->aeData[swAe].aeSlice[ss].encapImage;
        for (ii=0; ii<pEncapImage->numSbreak; ii++) 
        {
            pSbreak = &pEncapImage->sbreak[ii];
            if (pSbreak->virtUaddr == uPc_v) 
            {
                break;
            }    
        } /* end for ii*/

        /* If no pSbreak, this isn't ours */
        if (ii >= pEncapImage->numSbreak) 
        {
            continue;
        }    
        switch (pSbreak->sbreakType) 
        {
        case DBGAEINFO_SBRK_KILL:
            processCtxArbKill(objHandle, hwAeNum, swAe, ctx,
                              &startingPageChange);
            break;
        case DBGAEINFO_SBRK_BR_PAGE:
            /* addrOffset is the branch target virtual addr */
            processOverlayBr(objHandle, hwAeNum, swAe, ctx, pSbreak->addrOffset,
                             &startingPageChange);
            break;
        case DBGAEINFO_SBRK_RTN_PAGE:
            /* addrOffset is the virtual addr offset from regType/Addr */
            if ((pSbreak->regType == ICP_LMEM0) || (pSbreak->regType == ICP_LMEM1)) 
            {
                /* Return address is in local memory, indexed by LM ADDR reg. In
                   this case we need to read the active LM ADDR reg, not the context
                   specific copy because the copy doesn't get updated until the next 
                   context swaps in.
                */
                unsigned int activeLmAddr, regAddr;                 
                if (pSbreak->regType == ICP_LMEM0) 
                {
                    halAe_GetAeCsr((unsigned char)hwAeNum, LM_ADDR_0_ACTIVE, &activeLmAddr);
                }    
                else 
                {
                    halAe_GetAeCsr((unsigned char)hwAeNum, LM_ADDR_1_ACTIVE, &activeLmAddr);
                }    

                regAddr = pSbreak->regAddr;
                regAddr |= ((activeLmAddr & XLA_LM_ADDR) >> XLA_LM_ADDR_BITPOS);
                halAe_GetLM((unsigned char)hwAeNum, (unsigned short)regAddr, &target_va);
            }                
            else 
            {
                halAe_GetDataReg((unsigned char)hwAeNum, (unsigned char)ctx, pSbreak->regType, (unsigned short)pSbreak->regAddr,
                                 &target_va);
            }
            /* Truncate address to 17 bits as kludge due to 16-bit load addr */
            target_va &= ((1<<17)-1);
            target_va += pSbreak->addrOffset;
            processOverlayBr(objHandle, hwAeNum, swAe, ctx, target_va,
                             &startingPageChange);
            break;
        default:
            /* isn't one of ours, so don't do any more processing on this AE */
            continue;
        }
        masks->attn_bkpt_mask &= ~(1<<hwAeNum);
        restartMask |= (1<<hwAeNum);
    } /* end for hwAeNum */

    /* restart AEs */
    if (startingPageChange) 
    {
        halAe_CallNewPageCallback(END_OF_PAGE_CHANGE, 0, 0, 0);
    }
    restartMask &= ~objHandle->pausingAeMask; /* Don't restart AEs that are in
                                                 the process of being paused */
    for (hwAeNum = 0; hwAeNum < UCLO_MAX_AES; hwAeNum++) 
    {
        if (restartMask & (1<<hwAeNum)) 
        {
            /* kick-start the context arbiter */
            UcLo_kickStartArbiter(hwAeNum);

            /* enable contexts */
            enableCtxMask = 0;
            swAe = objHandle->swAe[hwAeNum];
            if(swAe >= UOF_MAX_NUM_OF_AE)
            {
                continue;
            }
            for(ss=0; ss < objHandle->aeData[swAe].numSlices; ss++) 
            {
                enableCtxMask = enableCtxMask | objHandle->aeData[swAe].aeSlice[ss].assignedCtxMask;
            }    
            halAe_Start((unsigned char)hwAeNum, enableCtxMask);
        } /* end if restartMask */
    } /* end for hwAeNum */
    
    MUTEX_UNLOCK(objHandle->overlayMutex);

    UcLo_postIntrCallback();
}

/*-----------------------------------------------------------------------------
   Function:    updateCtxEnables
   Description: Update CtxEnables Mask for those AEs where a page-change
                is in progress
   Returns:     
-----------------------------------------------------------------------------*/
static void 
updateCtxEnables(uclo_objHandle_T *objHandle,
                 unsigned int aeMask,    /* Which AEs we're interested in */
                 unsigned int *meEnMask) /* Mask is indexed by hwAeNum */
{
    unsigned int hwAeNum=0, swAe=0;
    uof_encapAe_T *pEncapImage=NULL;
    uof_sbreak_T  *pSbreak=NULL;
    unsigned int actCtx=0, ctx=0, uPc_p=0, uPc_v=0, ii=0, ss=0;

    MUTEX_LOCK(objHandle->overlayMutex);

    for (hwAeNum = 0; hwAeNum < UCLO_MAX_AES; hwAeNum++) 
    {
        if (!(aeMask & (1<<hwAeNum))) 
        {
            continue; /* We don't care about this AE */
        }
        if (*meEnMask & (1<<hwAeNum)) 
        {
            continue; /* We already know that this AE is active */
        }
        if (objHandle->pausingAeMask & (1<<hwAeNum)) 
        {
            continue; /* We're in the process of pausing, don't claim
                         we're active if we hit a br_page */
        }
        /* see if we have any data on this AE */
        swAe = objHandle->swAe[hwAeNum];
        if (swAe > MAX_SW_AE_NUM) 
        {
            continue;
        }
        if (hwAeNum != objHandle->hwAeNum[swAe]) 
        {
            continue; /* Not a valid hwAeNum */
        }
        if (halAe_IsAeEnabled((unsigned char)hwAeNum) == HALAE_ENABLED) 
        {
            /* This AE got enabled since the check outside of the mutex */
            *meEnMask |= 1<<hwAeNum;
            continue;
        }

        /* AE is not enabled, and we have sbreaks; see if we're sitting at one */
        /* get the active ctx and pc */
        halAe_GetAeCsr((unsigned char)hwAeNum, ACTIVE_CTX_STATUS, &actCtx);
        if(actCtx & ACS_ABO) 
        {
            continue;
        }    
        ctx = ACS_ACNO & actCtx;

        /* find the slice */
        for(ss=0; ss < objHandle->aeData[swAe].numSlices; ss++)
        {
            if(objHandle->aeData[swAe].aeSlice[ss].assignedCtxMask & (1<<ctx)) 
            {
                break;
            }
        }
        if(ss >= objHandle->aeData[swAe].numSlices) 
        {
           continue; /* ctx not assigned on this AE */
        }   

        pEncapImage = objHandle->aeData[swAe].aeSlice[ss].encapImage;
        if (pEncapImage == NULL) 
        {
            continue; /* No data on this AE */
        }    
        if (pEncapImage->numSbreak == 0) 
        {
            continue; /* No sbreaks for this AE */
        }
        halAe_GetPC((unsigned char)hwAeNum, (unsigned char)ctx, &uPc_p);
        /* Subtract one from physical PC to get addr of bkpt */
        /* Wrap to ustore size */
        uPc_p = (uPc_p - 1) & (objHandle->ustorePhySize - 1);
        halAe_MapPhysToVirtUaddr((unsigned char)hwAeNum, uPc_p, &uPc_v);

        /* See if it matches an pSbreak */
        for (ii=0; ii<pEncapImage->numSbreak; ii++) 
        {
            pSbreak = &pEncapImage->sbreak[ii];
            if (pSbreak->virtUaddr == uPc_v) 
            {
                break;
            }    
        } /* end for ii */

        /* If no pSbreak, this isn't ours */
        if (ii >= pEncapImage->numSbreak) 
        {
            continue;
        } 
        switch (pSbreak->sbreakType) 
        {
        case DBGAEINFO_SBRK_KILL:
        case DBGAEINFO_SBRK_BR_PAGE:
        case DBGAEINFO_SBRK_RTN_PAGE:
            /* we're doing a branch, so mark us as enabled */
            *meEnMask |= 1<<hwAeNum;
            continue;
        } /* end switch sbreakType */

    } /* end for hwAeNum */
   
    MUTEX_UNLOCK(objHandle->overlayMutex);
}

/*-----------------------------------------------------------------------------
   Function:    pausingMEs
   Description: Update the mask of AEs in the process of being paused
   Returns:     n/a
-----------------------------------------------------------------------------*/
static void
pausingMEs(uclo_objHandle_T *objHandle,
           unsigned int aeMask) /* Mask is indexed by hwAeNum */
{
    objHandle->pausingAeMask = aeMask;
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_ProcessHalCallbacks
   Description: Process callbacks from dbgMe (typically) to UCLO
   Returns:     n/a
-----------------------------------------------------------------------------*/
void
UcLo_ProcessHalCallbacks(Hal_UcloCallReason_T reason,
                         unsigned int         arg0,
                         unsigned int*        arg1,
                         void*                user_data)
{
    switch (reason) 
    {
    case UPDATE_AE_ENABLES:
        updateCtxEnables((uclo_objHandle_T*)user_data, arg0, arg1);
        break;
    case PAUSING_AES:
        pausingMEs((uclo_objHandle_T*)user_data, arg0);
        break;
    }
}

