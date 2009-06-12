#ifndef __IDT_TIM_H__
#define __IDT_TIM_H__

/*******************************************************************************
 *
 * Copyright 2002 Integrated Device Technology, Inc.
 *		All rights reserved.
 *
 * Timer register definition.
 *
 * File   : $Id: //depot/sw/releases/linuxsrc/src/kernels/mips-linux-2.4.25/include/asm-mips/rc32438/tim.h#1 $
 *
 * Author : ryan.holmQVist@idt.com
 * Date   : 20011005
 * Update :
 *	    $Log: tim.h,v $
 *	    Revision 1.2  2002/06/06 18:34:05  astichte
 *	    Added XXX_PhysicalAddress and XXX_VirtualAddress
 *	
 *	    Revision 1.1  2002/05/29 17:33:25  sysarch
 *	    jba File moved from vcode/include/idt/acacia
 *	
 *
 ******************************************************************************/


#include  <asm/rc32438/types.h> 

enum
{
	TIM0_PhysicalAddress	= 0x18028000,
	TIM_PhysicalAddress	= TIM0_PhysicalAddress,		// Default

	TIM0_VirtualAddress	= 0xb8028000,
	TIM_VirtualAddress	= TIM0_VirtualAddress,		// Default
} ;

enum
{
	TIM_Count = 3,
} ;

struct TIM_CNTR_s
{
	U32 count ;
	U32 compare ;
	U32 ctc ;	//use CTC_
} ;

typedef struct TIM_s
{
	struct TIM_CNTR_s	tim [TIM_Count] ;
	U32			rcount ;	//use RCOUNT_
	U32			rcompare ;	//use RCOMPARE_
	U32			rtc ;		//use RTC_
} volatile * TIM_t ;

enum
{
	CTC_en_b	= 0,		
	CTC_en_m	= 0x00000001,
	CTC_to_b	= 1,		 
	CTC_to_m	= 0x00000002,

	RCOUNT_count_b		= 0,	     
	RCOUNT_count_m		= 0x0000ffff,
	RCOMPARE_compare_b	= 0,	   
	RCOMPARE_compare_m	= 0x0000ffff,
	RTC_ce_b		= 0,		
	RTC_ce_m		= 0x00000001,
	RTC_to_b		= 1,		
	RTC_to_m		= 0x00000002,
	RTC_rqe_b		= 2,		
	RTC_rqe_m		= 0x00000004,
				 
} ;
#endif	// __IDT_TIM_H__

