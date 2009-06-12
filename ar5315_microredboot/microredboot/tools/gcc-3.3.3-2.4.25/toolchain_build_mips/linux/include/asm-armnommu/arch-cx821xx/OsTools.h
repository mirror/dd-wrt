/****************************************************************************
*
*	Name:			OsTools.h
*
*	Description:	
*
*	Copyright:		(c) 2002 Conexant Systems Inc.
*
*****************************************************************************

  This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option)
any later version.

  This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details.

  You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc., 59
Temple Place, Suite 330, Boston, MA 02111-1307 USA

****************************************************************************
*  $Author: gerg $
*  $Revision: 1.1 $
*  $Modtime: 7/09/02 2:16p $
****************************************************************************/

#ifndef _OSTOOLS_H_
#define _OSTOOLS_H_

#include "OsDefs.h"

#include <linux/kernel.h>
#include <linux/config.h>
#include <linux/skbuff.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/wait.h>

#include <linux/version.h>
#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) (((a) << 16) | ((b) << 8) | (c))
#endif

#if ( LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) ) && ( LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0) )
	#include <linux/compatmac.h>
#endif

#if 1 // can we support this in embedded linux?
////////////////////////////////////////////////////////
// timers
////////////////////////////////////////////////////////
typedef struct timer_list LINUX_TIMER;

static __inline void  INIT_TIMER( IN LINUX_TIMER* pTimer,
                                  IN void (*pTimerFunction)(ULONG),
                                  IN ULONG pUserData )
{
	pTimer->data		= pUserData;
	pTimer->function	= pTimerFunction;

	init_timer( pTimer );
}

static __inline void CANCEL_TIMER( LINUX_TIMER* Tmr, BOOLEAN* Stat)
{
	int TimerStat;

	TimerStat = del_timer(	Tmr );

	if (TimerStat == 0)
		*Stat = FALSE;
	else
		*Stat = TRUE;
}

static __inline void START_TIMER( LINUX_TIMER* Tmr, ULONG msTime )
{
	Tmr->expires = jiffies + ((msTime*HZ) / 1000);
	add_timer(	Tmr );
}
#endif // #ifdef EMBEDDED_LINUX



///////////////////////////////////////////////////////
// spin locks
///////////////////////////////////////////////////////
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)

	typedef spinlock_t SPIN_LOCK;

	#define INIT_SPIN_LOCK( pLock ) spin_lock_init( pLock )

	#define ACQUIRE_LOCK( pLock, flag ) spin_lock_irqsave( pLock, flag )

	#define RELEASE_LOCK( pLock, flag ) spin_unlock_irqrestore( pLock, flag )

	#define ACQUIRE_LOCK_AT_ISR( pLock ) spin_lock( pLock )

	#define RELEASE_LOCK_AT_ISR( pLock ) spin_unlock( pLock )

	#define FREE_SPIN_LOCK( pLock )

#else  // #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) #else
	#ifdef LINUX_EMBEDDED
	
		#include "cnxtbsp.h"
		
		#define INIT_SPIN_LOCK( pLock ) 

		#define ACQUIRE_LOCK( pLock, flag )		disable_irq(INT_LVL_TIMER_1) 

		#define RELEASE_LOCK( pLock, flag ) 	enable_irq(INT_LVL_TIMER_1)

		#define ACQUIRE_LOCK_AT_ISR( pLock ) 

		#define RELEASE_LOCK_AT_ISR( pLock ) 

		#define FREE_SPIN_LOCK( pLock )
	#endif // #ifdef LINUX_EMBEDDED

#endif  // #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)



////////////////////////////////////////////////////////
// EVENTS
////////////////////////////////////////////////////////
//
//  This implements the manual set/reset event functions.
//
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,2,0)
	typedef struct wait_queue *wait_queue_head_t ;
#endif
typedef struct _EVENT_HNDL_
{
	volatile int		SetFlag;
	volatile int		WaitCnt;
	wait_queue_head_t	WaitQue;
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)
		spinlock_t			FlagLock;
	#endif
} EVENT_HNDL;

void INIT_EVENT( EVENT_HNDL* pEvent );
int WAIT_EVENT( EVENT_HNDL* pEvent, ULONG msecDelay );
void SET_EVENT( EVENT_HNDL* pEvent ) ;
void SET_EVENT_FROM_ISR( EVENT_HNDL* pEvent ) ;
void RESET_EVENT( EVENT_HNDL* pEvent );


///////////////////////////////////////////////////////////
// Sleep Function
///////////////////////////////////////////////////////////
static __inline void SLEEP( ULONG timems )
{
	current->state = TASK_INTERRUPTIBLE;
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)
		schedule_timeout(timems*HZ/1000);
	#else
		current->timeout = jiffies + timems;
		schedule();
		current->timeout = 0;
	#endif
}

static __inline void MICRO_DELAY( ULONG MicroSecs )
{
	udelay( MicroSecs );
}

/////////////////////////////////////////////////////////
// Que handling
/////////////////////////////////////////////////////////
typedef struct _LIST_ENTRY_
{
	struct _LIST_ENTRY_ *pNext;
	struct _LIST_ENTRY_ *pPrev;
} LIST_ENTRY, *PLIST_ENTRY;

//
//  VOID
//  InitializeListHead(
//      PLIST_ENTRY ListHead
//      );
//

#define InitializeListHead(ListHead) (\
    (ListHead)->pNext = (ListHead)->pPrev = (ListHead))

//
//  BOOLEAN
//  IsListEmpty(
//      PLIST_ENTRY ListHead
//      );
//

#define IsListEmpty(ListHead) \
    ((ListHead)->pNext == (ListHead))

//
//  PLIST_ENTRY
//  RemoveHeadList(
//      PLIST_ENTRY ListHead
//      );
//

#define RemoveHeadList(ListHead) \
    (ListHead)->pNext;\
    {RemoveEntryList((ListHead)->pNext)}

//
//  PLIST_ENTRY
//  RemoveTailList(
//      PLIST_ENTRY ListHead
//      );
//

#define RemoveTailList(ListHead) \
    (ListHead)->pPrev;\
    {RemoveEntryList((ListHead)->pPrev)}

//
//  VOID
//  RemoveEntryList(
//      PLIST_ENTRY Entry
//      );
//

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_pPrev;\
    PLIST_ENTRY _EX_pNext;\
    _EX_pNext = (Entry)->pNext;\
    _EX_pPrev = (Entry)->pPrev;\
    _EX_pPrev->pNext = _EX_pNext;\
    _EX_pNext->pPrev = _EX_pPrev;\
    }

//
//  VOID
//  InsertTailList(
//      PLIST_ENTRY ListHead,
//      PLIST_ENTRY Entry
//      );
//

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_pPrev;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_pPrev = _EX_ListHead->pPrev;\
    (Entry)->pNext = _EX_ListHead;\
    (Entry)->pPrev = _EX_pPrev;\
    _EX_pPrev->pNext = (Entry);\
    _EX_ListHead->pPrev = (Entry);\
    }

//
//  VOID
//  InsertHeadList(
//      PLIST_ENTRY ListHead,
//      PLIST_ENTRY Entry
//      );
//

#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_pNext;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_pNext = _EX_ListHead->pNext;\
    (Entry)->pNext = _EX_pNext;\
    (Entry)->pPrev = _EX_ListHead;\
    _EX_pNext->pPrev = (Entry);\
    _EX_ListHead->pNext = (Entry);\
    }


#ifdef SOFTSAR_DRIVER
////////////////////////////////////////////////////////
// Kernel memory operations
////////////////////////////////////////////////////////
// This flag enables the memory allocation debugger - set it to
// nonzero to enable it
#define MEM_ALLOC_DBG 0

extern ULONG gAllocCnt;

#if MEM_ALLOC_DBG
typedef struct ALLOC_HEAD
{
	LIST_ENTRY	List;
	ULONG	    Magic; // number used to indicate allocated memory
	ULONG		Tag;
} ALLOC_HEAD_T, *PALLOC_HEAD_T;

#define ALLOC_MAGIC 0x01216560

extern ALLOC_HEAD_T gAllocList;
#endif 

static __inline 	STATUS ALLOCATE_MEMORY(	OUT PVOID* pMem,
        IN  ULONG size,
        IN  ULONG tag )
{
	STATUS Status = STATUS_SUCCESS;

	// if in memory debug keep a list of all allocated memory
	// blocks to help chase down leaks
#if MEM_ALLOC_DBG
	UCHAR 		 *pAll;
	PALLOC_HEAD_T pAllocEntry;

	pAll= kmalloc (size+sizeof(ALLOC_HEAD_T), GFP_KERNEL);

	if (!pAll)
	{
		*pMem = NULL;
		printk("<1> memory allocation failure\n");
		Status = -ENOMEM;
	}

	else
	{
		pAllocEntry = (PALLOC_HEAD_T)pAll;
		pAllocEntry->Magic = ALLOC_MAGIC;
		InsertTailList(&gAllocList.List, (PLIST_ENTRY)&pAllocEntry->List);
		pAllocEntry->Tag = tag;

		*pMem = pAll+sizeof(ALLOC_HEAD_T);
		gAllocCnt++;
	}

#else

// if not in memory debug then just allocate
	*pMem = kmalloc (size, GFP_ATOMIC);

	if (!*pMem)
	{
		printk("<1> memory allocation failure\n");
		Status = -ENOMEM;
	}
	else
		gAllocCnt++;

#endif

	return Status;
}

static __inline VOID FREE_MEMORY( IN PVOID pMem,
                                  IN ULONG Length,
                                  IN ULONG Flags )
{
	// if in memory debug then validate the block before freeing it
#if MEM_ALLOC_DBG
	PALLOC_HEAD_T pAllocEntry=(PALLOC_HEAD_T)((ULONG)pMem-sizeof(ALLOC_HEAD_T));

	// validate that it was allocated memory
	if ( pAllocEntry->Magic != ALLOC_MAGIC )
	{
		printk("<1>CnxADSL attempting to free invalid memory block %lx\n",(DWORD)pMem);
		return;
	}

	// clear the magic number to catch extra frees on the same block
	pAllocEntry->Magic = 0;

	// now take it out of the alloc list
	RemoveListEntry( &pAllocEntry->List );

	// and free it
	kfree( pAllocEntry );

#else
kfree( pMem );
#endif

	gAllocCnt--;
}

#define CLEAR_MEMORY( pMem, size ) memset( pMem, 0, size )

#define MEMORY_SET( pMem, value, size ) memset( pMem, value, size )

#define COPY_MEMORY( pDest, pSrc, size ) memcpy( pDest, pSrc, size )

#endif	//#ifdef NDIS_MINIPORT_DRIVER

////////////////////////////////////////////////////////
// ATM support functions
////////////////////////////////////////////////////////
#define RELEASE_SKB( pVcc, skb )	\
{									\
	if (pVcc->pop)					\
		pVcc->pop(pVcc,skb);		\
	else							\
		dev_kfree_skb(skb);			\
}

#define CNX_VCC( pVcc ) ((CDSL_VC_T *)(pVcc)->dev_data)

#endif //_OSTOOLS_H_


