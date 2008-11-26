/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell 
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File in accordance with the terms and conditions of the General 
Public License Version 2, June 1991 (the "GPL License"), a copy of which is 
available along with the File in the license.txt file or by writing to the Free 
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or 
on the worldwide web at http://www.gnu.org/licenses/gpl.txt. 

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File under the following licensing terms. 
Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
	    this list of conditions and the following disclaimer. 

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution. 

    *   Neither the name of Marvell nor the names of its contributors may be 
        used to endorse or promote products derived from this software without 
        specific prior written permission. 
    
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/


#ifndef MV_OS_H
#define MV_OS_H

/*************/
/* Includes  */
/*************/

#include "mvTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************/
/* Constants */
/*************/

#define MV_OS_WAIT_FOREVER				0


/*************/
/* Datatypes */
/*************/



/*********************/
/* OS Specific Stuff */
/*********************/

#if defined(MV_VXWORKS)

#include "mvOsVxw.h"

#elif defined(MV_LINUX)

#include "mvOsLinux.h"

#elif defined(MV_UBOOT)

#include "mvOsUboot.h"

#elif defined(MV_WINCE_OAL)

#include "mvOsWinceOal.h"

#elif defined(MV_WINCE_DDK)

#include "mvOsWinceDdk.h"

#elif defined(MV_WINCE_NDIS)

#include "mvOsWinceNdis.h"

#elif defined(MV_NETGX)

#include "mvOsNetgx.h"

#elif defined(MV_WIN32)

#include "mvOsWin32.h"

#else /* No OS */

#error "OS type not selected"

#endif /* OS specific */

/*	Useful functions to deal with Physical memory using OS dependent inline
 *	functions defined in the file mv_os_???.h
 */

/*  Set physical memory with specified value. Use this function instead of
 *  memset when physical memory should be set.
 */
static INLINE void mvOsSetPhysMem(MV_ULONG hpaddr, MV_U8 val, int size)
{
	int	i;

	for(i=0; i<size; i++)
	{
		MV_MEMIO8_WRITE( (hpaddr+i), val);
	}
}

/*	Copy data from Physical memory. Use this fuction instead of memcpy,
 *  when data should be copied from Physical memory to Local (virtual) memory. 
 */
static INLINE void mvOsCopyFromPhysMem(MV_U8* localAddr, MV_ULONG hpaddr, int size)
{
	int		i;
	MV_U8	byte;

	for(i=0; i<size; i++)
	{
		byte = MV_MEMIO8_READ(hpaddr+i);
		localAddr[i] = byte;
	}
}

/*	Copy data to Physical memory. Use this fuction instead of memcpy,
 *  when data should be copied from Local (virtual) memory to Physical memory. 
 */
static INLINE void mvOsCopyToPhysMem(MV_ULONG hpaddr, const MV_U8* localAddr, int size)
{
	int		i;

	for(i=0; i<size; i++)
	{
		MV_MEMIO8_WRITE( (hpaddr+i), localAddr[i]);
	}
}


/***********/
/* General */
/***********/

/* String compare function */
#ifndef mvOsStrCmp
int mvOsStrCmp(const char *str1,const char *str2);
#endif

/* mvOsGetCurrentTime -- Return current system's up-time */
#ifndef mvOsGetCurrentTime
unsigned long mvOsGetCurrentTime(void);
#endif

/* mvOsSleep
 *
 * DESCRIPTION:
 *     Sends the current task to sleep.
 *
 * INPUTS:
 *     mils -- number of miliseconds to sleep.
 *
 * RETURN VALUES:
 *     MV_OS_OK if succeeds.
 */
#ifndef mvOsSleep
void	mvOsSleep(unsigned long mils);
#endif


/*********/
/* Misc. */
/*********/

/* Returns last error number */
#ifndef mvOsGetErrNo
unsigned long	mvOsGetErrNo(void);
#endif

/* Returns random 32 bits value */
#ifndef mvOsRand
int	mvOsRand(void);
#endif


/**************/
/* Semaphores */
/**************/

/* mvOsSemCreate
 *
 * DESCRIPTION:
 *     Creates a semaphore.
 *
 * INPUTS:
 *     name  -- semaphore name.
 *     init  -- init value of semaphore counter.
 *     count -- max counter value (must be positive).
 *
 * OUTPUTS:
 *     smid -- pointer to semaphore ID.
 *
 * RETURN VALUES:
 *     MV_OS_OK if succeeds.
 */
#ifndef mvOsSemCreate
MV_STATUS   mvOsSemCreate(char *name,unsigned long init,unsigned long count,
					 unsigned long *smid);
#endif


/* mvOsSemDelete
 *
 * DESCRIPTION:
 *     Deletes a semaphore.
 *
 * INPUTS:
 *     smid -- semaphore ID.
 *
 * RETURN VALUES:
 *     MV_OS_OK if succeeds.
 */
#ifndef mvOsSemDelete
MV_STATUS 	mvOsSemDelete(unsigned long smid);
#endif

/* mvOsSemWait
 *
 * DESCRIPTION:
 *     Waits on a semaphore.
 *
 * INPUTS:
 *     smid     -- semaphore ID.
 *     time_out -- time out in miliseconds, or 0 to wait forever.
 *
 * RETURN VALUES:
 *     MV_OS_OK if succeeds.
 */
#ifndef mvOsSemWait
MV_STATUS	mvOsSemWait(unsigned long smid, unsigned long time_out);
#endif

/* mvOsSemSignal
 *
 * DESCRIPTION:
 *     Signals a semaphore.
 *
 * INPUTS:
 *     smid -- semaphore ID.
 *
 * RETURN VALUES:
 *     MV_OS_OK if succeeds.
 */
#ifndef mvOsSemSignal
MV_STATUS	mvOsSemSignal(unsigned long smid);
#endif


/**************/
/*   Memory   */
/**************/

/*******************************************************************************
* mvOsMalloc - Memory allocate
*
* DESCRIPTION:
*       Allocates memory block of specified size. The allocated memory 
*       fregment can be either blocked or paged mapped (continuous or non 
*       continuous memory in DRAM)
*
* INPUTS:
*       size - Bytes to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Virtual void pointer to the allocated space, or NULL if there is
*       insufficient memory available.
*
*******************************************************************************/
#ifndef mvOsMalloc
void*   mvOsMalloc(MV_U32 size);
#endif 

/*******************************************************************************
* mvOsRealloc - Memory reallocate
*
* DESCRIPTION:
*       Reallocate memory block of specified size.
*
* INPUTS:
*       pVirtAddr  - Virtual pointer to previously allocated buffer
*       size       - bytes to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Void pointer to the allocated space, or NULL if there is
*       insufficient memory available
*
*******************************************************************************/
#ifndef mvOsRealloc
void*   mvOsRealloc(void *pVirtAddr, MV_U32 size);
#endif

/*******************************************************************************
* mvOsFree - Free memory block
*
* DESCRIPTION:
*       Deallocates or frees a specified memory block acuired by malloc().
*
* INPUTS:
*       pVirtAddr - Virtual address pointer to previously allocated block.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
*******************************************************************************/
#ifndef mvOsFree
void mvOsFree(void *pVirtAddr);
#endif

/*******************************************************************************
* mvOsIoCachedMalloc - Allocate memory fregment for IO.
*
* DESCRIPTION:
*       Allocates memory block of specified size for the use of IO purposes.
*       The memory fregment is blocked map (continuous memory address in DRAM)
*       thus suitable for IO purposes (DMA buffers etc.). 
*       Memory cache coherency of this block is NOT guaranteed.
*
* INPUTS:
*       pDev        - Device for which memory was acuired. NULL if not relevant. 
*       size        - bytes to allocate
*
* OUTPUTS:
*       pPhyAddr    - Physical address pointer to memory block.
*
* RETURNS:
*       Virtual void pointer to the allocated space, or NULL if there is
*       insufficient memory available
*
*******************************************************************************/
#ifndef mvOsIoCachedMalloc 
void*   mvOsIoCachedMalloc(void* pDev, MV_U32 size, MV_ULONG* pPhyAddr);
#endif 

/*******************************************************************************
* mvOsIoCachedFree - Free memory fregment allocated by mvOsIoCachedMalloc.
*
* DESCRIPTION:
*       Deallocates or frees a specified memory block previously allocated
*       by mvOsIoCachedMalloc().
*
* INPUTS:
*       pDev        - Device for which memory was acuired. NULL if not relevant. 
*       size        - Size of released block.
*       phyAddr     - Physical address pointer to memory block.
*       pVirtAddr   - Virtual address pointer to memory block.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
*******************************************************************************/
#ifndef mvOsIoCachedFree
void    mvOsIoCachedFree(void* pDev, MV_U32 size, MV_ULONG phyAddr, void* pVirtAddr);
#endif

/*******************************************************************************
* mvOsIoUncachedMalloc
*
* DESCRIPTION:
*       Allocate a cache-safe buffer of specified size for DMA devices
*       and drivers. 
*       The memory fregment is blocked map (continuous memory address in DRAM)
*       thus suitable for IO purposes (DMA buffers etc.). Furthermore,
*       memory cache coherency of this block is guaranteed.
*
* INPUTS:
*       pDev    - Device for which memory was acuired. NULL if not relevant. 
*       size    - bytes to allocate
*
* OUTPUTS:
*       pPhyAddr - Physical pointer to the cache-safe buffer.
*
* RETURNS:
*       A virtual pointer to the cache-safe buffer, or NULL
*
*******************************************************************************/
#ifndef mvOsIoUncachedMalloc 
void*   mvOsIoUncachedMalloc(void* pDev, MV_U32 size, MV_ULONG* pPhyAddr); 
#endif

/*******************************************************************************
* mvOsIoUncachedFree
*
* DESCRIPTION:
*       Free the buffer acquired with mvOsIoUncachedMalloc()
*
* INPUTS:
*       pDev      - Device for which memory was acuired. NULL if not relevant. 
*       size      - Size of released block.
*       phyAddr   - Physical address pointer to memory block.
*       pVirtAddr - Virtual address pointer to memory block.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
*
*******************************************************************************/
#ifndef mvOsIoUncachedFree 
void    mvOsIoUncachedFree (void* pDev, MV_U32 size, MV_ULONG phyAddr, void*  pVirtAddr);
#endif

/*******************************************************************************
* mvOsIoVirtToPhy
*
* DESCRIPTION:
*       Converts virtual address to physical.
*
* INPUTS:
*       pVirtAddr - virtual address pointer
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       physical address value 
*
*******************************************************************************/
#ifndef mvOsIoVirtToPhy  
MV_ULONG  mvOsIoVirtToPhy (void* pDev, void* pVirtAddr);
#endif 

/*******************************************************************************
* mvOsCacheFlush
*
* DESCRIPTION:
*       This function flush cached memory to system bus.
*
* INPUTS:
*       pDev      - Device for which memory was acuired. NULL if not relevant. 
*       pVirtAddr - virtual address pointer.
*       size      - Size of memory block to flush.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       physical address of the flushed block.
*
*******************************************************************************/
#ifndef mvOsCacheFlush 
MV_ULONG  mvOsCacheFlush (void* pDev, void* pVirtAddr, int size);
#endif

/*******************************************************************************
* mvOsCacheInvalidate 
*
* DESCRIPTION:
*       This function invalidates cached memory.
*
* INPUTS:
*       pDev      - Device for which memory was acuired. NULL if not relevant. 
*       pVirtAddr - virtual address pointer.
*       size      - Size of memory block to invalidate.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       physical address of the invalidated block.
*
*******************************************************************************/
#ifndef mvOsCacheInvalidate 
MV_ULONG  mvOsCacheInvalidate (void* pDev, void* pVirtAddr, int size);
#endif

#ifdef __cplusplus
}
#endif

#endif /* MV_OS_H */
