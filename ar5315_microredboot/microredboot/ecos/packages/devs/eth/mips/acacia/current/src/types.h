#ifndef __IDT_TYPES_H__
#define __IDT_TYPES_H__

/*******************************************************************************
 *
 * Copyright 2002 Integrated Device Technology, Inc.
 *		All rights reserved.
 *
 * Common typedefs used in IDT-generated code.
 *
 * File   : $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/packages/devs/eth/mips/acacia/current/src/types.h#1 $
 *
 * Author : Allen.Stichter@idt.com
 * Date   : 20020606
 * Update :
 *	    $Log: types.h,v $
 *	    Revision 1.1  2002/06/06 16:16:56  astichte
 *	    Added
 *	
 *
 ******************************************************************************/

typedef unsigned char		U8 ;
typedef signed char		S8 ;

typedef unsigned short		U16 ;
typedef signed short		S16 ;

typedef unsigned int		U32 ;
typedef signed int		S32 ;

typedef unsigned long long	U64 ;
typedef signed long long	S64 ;

#if 0
#ifndef __cplusplus
	typedef U32		bool ;	// (false == 0), (true is != false)
#endif	// __cplusplus
#endif

#endif	// __IDT_TYPES_H__
