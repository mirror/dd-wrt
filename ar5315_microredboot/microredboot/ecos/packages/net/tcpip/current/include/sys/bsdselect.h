//==========================================================================
//
//      include/sys/select.h
//
//      Support for 'select()' system call
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

#ifndef _SYS_SELECT_H_
#define	_SYS_SELECT_H_

#include <pkgconf/system.h>

#ifdef CYGPKG_IO_FILEIO

#include <cyg/io/file.h>

void	selwakeup __P((struct selinfo *));

#else

/*
 * Used to maintain information about processes that wish to be
 * notified when I/O becomes possible.
 */
struct selinfo {
    void *unused;
};

void	selrecord __P((void *selector, struct selinfo *));
void	selwakeup __P((struct selinfo *));

#endif

#endif /* !_SYS_SELECT_H_ */
