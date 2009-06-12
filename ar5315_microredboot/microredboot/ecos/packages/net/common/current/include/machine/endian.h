//==========================================================================
//
//      include/machine/endian.h
//
//      Architecture/platform specific byte ordering support
//
//==========================================================================
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD or other sources,
// and are covered by the appropriate copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-01-10
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================


#ifndef _MACHINE_ENDIAN_H_
#define _MACHINE_ENDIAN_H_

#include <cyg/hal/basetype.h>

#if CYG_BYTEORDER == CYG_MSBFIRST
#define BYTE_ORDER BIG_ENDIAN
#else
#define BYTE_ORDER LITTLE_ENDIAN
#endif
#include <sys/endian.h>

#endif // _MACHINE_ENDIAN_H_


