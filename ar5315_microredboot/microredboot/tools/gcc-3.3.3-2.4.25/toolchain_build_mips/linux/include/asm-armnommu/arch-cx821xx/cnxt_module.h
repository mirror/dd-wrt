/****************************************************************************
*
*	Name:			cnxt_module.h
*
*	Description:	Header file for adsl intermodule communication
*
*	Copyright:		(c) 2002 Conexant Systems Inc.
****************************************************************************

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

*****************************************************************************
*  $Author: gerg $
*  $Revision: 1.1 $
*  $Modtime: 11/04/02 5:12p $
****************************************************************************/

#ifndef __CNXT_MODULE_H
#define __CNXT_MODULE_H

typedef struct _CNXT_ADSL_EXPORTS_T
{
	PCARDAL_CONTEXT_T (*pcardALCfgInit)		( void *, PTIG_USER_PARAMS );
	BOOL				(*pCdalADSLModulation)	( PCARDAL_CONTEXT_T , HW_IO_MODULATION_T * );
	BOOL				(*pcardALHwIoDpGetAdslTransceiverStatus)( PCARDAL_CONTEXT_T , PHW_IO_TRANSCEIVER_STATUS_T ); 

} CNXT_ADSL_EXPORTS_T, *pCNXT_ADSL_EXPORTS_T;

#define	CNXT_ADSL	"cnxt_adsl"

#endif
