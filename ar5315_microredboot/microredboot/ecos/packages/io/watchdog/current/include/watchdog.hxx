#ifndef CYGONCE_DEVS_WATCHDOG_HXX
#define CYGONCE_DEVS_WATCHDOG_HXX

//==========================================================================
//
//      watchdog.hxx
//
//      Watchdog interface declaration
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   nickg
// Contributors:        nickg
// Date:        1998-07-14
// Purpose:     Watchdog declarations
// Description: This file defines the interface to the watchdog device
//              that provides timer based recovery from software and
//              hardware faults.
// Usage:       #include <cyg/devs/watchdog.hxx>
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>            // assertion macros

class Cyg_Watchdog_Action;

// -------------------------------------------------------------------------
// Watchdog class

class Cyg_Watchdog
{

    Cyg_Watchdog_Action         *action_list;

    cyg_uint64                  resolution;
    
public:

    Cyg_Watchdog();

    // Return time interval allowed between resets before watchdog
    // triggers, in nanoseconds.
    cyg_uint64 get_resolution( void );
    
    // Start the watchdog running.
    void start( void );

    // Reset watchdog timer. This needs to be called regularly to prevent
    // the watchdog firing.
    void reset( void );

#ifndef CYGSEM_WATCHDOG_RESETS_ON_TIMEOUT    
    // Trigger the watchdog as if the timer had expired.
    void trigger( void );

    // Register an action routine that will be called when the timer
    // triggers.
    void install_action( Cyg_Watchdog_Action *wdaction );

    // Deregister a previously registered action routine.
    void uninstall_action( Cyg_Watchdog_Action *wdaction );
#endif
    
    // A static instance of the single system defined watchdog device.
    static Cyg_Watchdog watchdog;

private:
    void init_hw( void );
};

#ifndef CYGSEM_WATCHDOG_RESETS_ON_TIMEOUT    
// -------------------------------------------------------------------------
// Watchdog action class

class Cyg_Watchdog_Action
{
    friend class Cyg_Watchdog;

    Cyg_Watchdog_Action         *next;          // link in chain
    
    void (*action)( CYG_ADDRWORD data );        // action function

    CYG_ADDRWORD data;                          // data argument
    
public:

    Cyg_Watchdog_Action(
        void (*action)( CYG_ADDRWORD data ),
        CYG_ADDRWORD data
        );

    ~Cyg_Watchdog_Action();

    void install();
    
    void uninstall();
};

// -------------------------------------------------------------------------
// Cyg_Watchdog_Action inlines

inline Cyg_Watchdog_Action::Cyg_Watchdog_Action(
    void (*action_arg)( CYG_ADDRWORD data ),
    CYG_ADDRWORD data_arg
    )
{
    next        = NULL;
    action      = action_arg;
    data        = data_arg;
}

inline Cyg_Watchdog_Action::~Cyg_Watchdog_Action()
{
    Cyg_Watchdog::watchdog.uninstall_action( this );
}

inline void Cyg_Watchdog_Action::install()
{
    Cyg_Watchdog::watchdog.install_action( this );    
}

inline void Cyg_Watchdog_Action::uninstall()
{
    Cyg_Watchdog::watchdog.uninstall_action( this );    
}

#endif // CYGSEM_WATCHDOG_RESETS_ON_TIMEOUT

// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_DEVS_WATCHDOG_HXX
// EOF watchdog.hxx
