/****************************************************************************
*
*	Name:			CardAlpub.h
*
*	Description:	Abstraction Layer routines public declarations.
*
*	Copyright:		(c) 2000 Conexant Systems Inc.
*
*****************************************************************************
*	$Author: gerg $
*	$Revision: 1.1 $
*	$Modtime: 3/06/02 1:12p $
****************************************************************************/

#ifndef CARDALPUB_H
#define CARDALPUB_H


#ifndef PCARDAL_CONTEXT_T_DEFINED
	#define PCARDAL_CONTEXT_T_DEFINED
	typedef struct _CARDAL_CONTEXT_T *PCARDAL_CONTEXT_T ;
#endif
				
#ifndef _COMMON_DATA_H_
				typedef struct _TIG_USER_PARAMS_	TIG_USER_PARAMS,	*PTIG_USER_PARAMS;
#endif


PCARDAL_CONTEXT_T cardALCfgInit
(
	void								* adapter,
	PTIG_USER_PARAMS					  pTigParams
) ;

void cardALChipStartAdslLine
(
	PCARDAL_CONTEXT_T				  pCardAl_Context
);

void cardALChipShutdownAdslLine
(
	PCARDAL_CONTEXT_T				  pCardAl_Context
);

DWORD cardALChipGetLineStatus
(
	PCARDAL_CONTEXT_T				  pCardAl_Context
);

BOOLEAN ChipAlTigrisDpWriteNvram
(
	PCARDAL_CONTEXT_T				  pCardal,
	UINT16							  offset,
	const UINT8 					* data,
	UINT16							  size
) ;

BOOLEAN ChipAlTigrisDpReadNvram
(
	PCARDAL_CONTEXT_T				  pCardal,
	UINT16							  offset,
	UINT8 							* data,
	UINT16							  size
) ;

#endif /* CARDALPUB_H */ 
