//==========================================================================
//
//      wallclock_synth.cxx
//
//      eCos synthetic wallclock driver.
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
// Author(s):     Savin Zlobec <savin@elatec.si>
// Contributors:  
// Date:          2003-10-02
// Purpose:        
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/wallclock.h>           
#include <pkgconf/devs_wallclock_synth.h>

#include <cyg/hal/hal_io.h>               
#include <cyg/hal/hal_arch.h>

#include <cyg/infra/cyg_type.h>          
#include <cyg/infra/diag.h>            

#include <cyg/io/wallclock.hxx>          

//-----------------------------------------------------------------------------

#ifdef CYGSEM_WALLCLOCK_SET_GET_MODE
// Difference between system and eCos wallclock
static cyg_uint32 epoch_ticks;
static cyg_uint32 epoch_time_stamp;
#endif

//-----------------------------------------------------------------------------
// Functions required for the hardware-driver API.

// Initializes the clock
void
Cyg_WallClock::init_hw_seconds(void)
{
#ifdef CYGSEM_WALLCLOCK_SET_GET_MODE
    int fd;

    // Read difference between system and eCos wallclock from file
    fd = cyg_hal_sys_open(CYGDAT_DEVS_WALLCLOCK_SYNTH_FILENAME, 
        CYG_HAL_SYS_O_RDONLY, 0);
   
    if (fd > 0)
    {
        cyg_hal_sys_read(fd, &epoch_time_stamp, sizeof(epoch_time_stamp));
        cyg_hal_sys_read(fd, &epoch_ticks, sizeof(epoch_ticks));
        cyg_hal_sys_close(fd);
    }
#endif
}

// Returns the number of seconds elapsed since 1970-01-01 00:00:00
cyg_uint32 
Cyg_WallClock::get_hw_seconds(void)
{
    cyg_uint32 res;
    struct cyg_hal_sys_timeval  ctv;
    struct cyg_hal_sys_timezone ctz;
    
    cyg_hal_sys_gettimeofday(&ctv, &ctz);

#ifdef CYGSEM_WALLCLOCK_SET_GET_MODE
    res = epoch_time_stamp + ctv.hal_tv_sec - epoch_ticks;    
#else
    res = ctv.hal_tv_sec;    
#endif

    return res;    
}

#ifdef CYGSEM_WALLCLOCK_SET_GET_MODE

// Sets the clock. Argument is seconds elapsed since 1970-01-01 00:00:00
void
Cyg_WallClock::set_hw_seconds(cyg_uint32 secs)
{
    int fd;
    struct cyg_hal_sys_timeval  ctv;
    struct cyg_hal_sys_timezone ctz;

    // System wallclock time    
    cyg_hal_sys_gettimeofday(&ctv, &ctz);
    
    // Set the difference between the system and eCos wallclock
    epoch_time_stamp = secs;
    epoch_ticks      = ctv.hal_tv_sec;
   
    // Write difference to file
    fd = cyg_hal_sys_open(CYGDAT_DEVS_WALLCLOCK_SYNTH_FILENAME, 
        CYG_HAL_SYS_O_WRONLY | CYG_HAL_SYS_O_CREAT,
        CYG_HAL_SYS_S_IRWXU | CYG_HAL_SYS_S_IRWXG | CYG_HAL_SYS_S_IRWXO);

    if (fd > 0)
    {
        cyg_hal_sys_write(fd, &epoch_time_stamp, sizeof(epoch_time_stamp));
        cyg_hal_sys_write(fd, &epoch_ticks, sizeof(epoch_ticks));
        cyg_hal_sys_close(fd);
    }
}

#endif // CYGSEM_WALLCLOCK_SET_GET_MODE

//-----------------------------------------------------------------------------
// EOF wallclock_synth.cxx
