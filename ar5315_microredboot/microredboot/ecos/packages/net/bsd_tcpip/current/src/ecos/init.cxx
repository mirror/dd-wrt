//==========================================================================
//
//      src/ecos/init.cxx
//
//==========================================================================
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD, 
// FreeBSD or other sources, and are covered by the appropriate
// copyright disclaimers included herein.
//
// Portions created by Red Hat are
// Copyright (C) 2002 Red Hat, Inc. All Rights Reserved.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================

//==========================================================================
//
//      ecos/init.cxx
//
//      Networking package initializer class
//
//==========================================================================
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


// Network initialization

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

#define NET_INIT CYGBLD_ATTRIB_INIT_AFTER(CYG_INIT_LIBC)

// This is a dummy class just so we can execute the network package 
// initialization at it's proper priority

externC void cyg_net_init(void);

class net_init_class {
public:
    net_init_class(void) { 
        cyg_net_init();
    }
};

// And here's an instance of the class just to make the code run
static net_init_class _net_init NET_INIT;

externC void
cyg_do_net_init(void)
{
}
