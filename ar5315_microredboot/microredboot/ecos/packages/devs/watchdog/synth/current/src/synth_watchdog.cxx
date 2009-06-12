//==========================================================================
//
//      synth_watchdog.cxx
//
//      Watchdog driver for the synthetic target
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2002 Bart Veer
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
// Alternative licenses for eCos may be arranged by contacting the
// copyright holder(s).
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    bartv
// Contributors: bartv
// Date:         2002-09-04
//
//####DESCRIPTIONEND####
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/devs_watchdog_synth.h>

#ifdef CYGIMP_WATCHDOG_HARDWARE

#include <cyg/hal/hal_arch.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_io.h>

// FIXME: right now the generic watchdog header depends on the
// kernel. That should be fixed in the watchdog code, but will
// affect some device drivers as well
#include <pkgconf/kernel.h>
#include <cyg/io/watchdog.hxx>

// Protocol between host and target
#define SYNTH_WATCHDOG_START    0x01
#define SYNTH_WATCHDOG_RESET    0x02

// The synthetic target's watchdog implementation involves interaction
// with a watchdog.tcl script running in the I/O auxiliary. The device
// must be instantiated during system initialization, preferably via
// a prioritized C++ static constructor. The generic watchdog package
// does have a static object, but it is not prioritized. If
// CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG is enabled then that object's
// constructor would get called too late.
//
// Instead a private class is defined here, and once instance is created.
// That instance gets referenced by the Cyg_Watchdog members, so
// selective linking does not get in the way. Instantiation happens inside
// the constructor, and the main Cyg_Watchdog::start() and reset() functions
// involve passing a message to the host-side.
//
// There is an open issue re. resolution. Usually the hardware imposes
// limits on what resolutions are valid, in fact there may be only one.
// With the synthetic target it would be possible to configure the
// desired resolution either on the target-side using a CDL option, or
// on the host-side using the target definition file. The resolution
// would have to be fairly coarse, probably at least 0.1 seconds,
// to allow for communication overheads. It is not clear whether
// target-side or host-side configuration is more appropriate, so for
// now a fixed resolution of one second is used.

class _Synth_Watchdog {
  public:
    _Synth_Watchdog();
    ~_Synth_Watchdog() { }

    cyg_uint64  resolution;
};

// A static instance of the _Synth_Watchdog class, whose constructor will
// be called at the right time to instantiate host-side support.
static _Synth_Watchdog _synth_watchdog_object CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_DRIVERS);

// Id for communicating with the watchdog instance in the auxiliary
static int aux_id   = -1;

_Synth_Watchdog::_Synth_Watchdog()
{
    // SIGPWR is disabled by default. It has to be reenabled.
    struct cyg_hal_sys_sigset_t blocked;
    CYG_HAL_SYS_SIGEMPTYSET(&blocked);
    CYG_HAL_SYS_SIGADDSET(&blocked, CYG_HAL_SYS_SIGPWR);
    cyg_hal_sys_sigprocmask(CYG_HAL_SYS_SIG_UNBLOCK, &blocked, (cyg_hal_sys_sigset_t*) 0);
    
    resolution = 1000000000LL;
    if (synth_auxiliary_running) {
        aux_id  = synth_auxiliary_instantiate("devs/watchdog/synth",
                                              SYNTH_MAKESTRING(CYGPKG_DEVS_WATCHDOG_SYNTH),
                                              "watchdog",
                                              (const char*) 0,
                                              (const char*) 0);
    }
}

// Hardware initialization. This has already happened in the
// _Synth_Watchdog constructor, all that is needed here is to
// propagate the resolution.
void
Cyg_Watchdog::init_hw(void)
{
    resolution  = _synth_watchdog_object.resolution;
}

void
Cyg_Watchdog::start(void)
{
    if (synth_auxiliary_running && (-1 != aux_id)) {
        synth_auxiliary_xchgmsg(aux_id, SYNTH_WATCHDOG_START, 0, 0,
                               (const unsigned char*)0, 0,
                               (int *) 0,
                               (unsigned char*) 0, (int*) 0, 0);
    }
}

void
Cyg_Watchdog::reset()
{
    if (synth_auxiliary_running && (-1 != aux_id)) {
        synth_auxiliary_xchgmsg(aux_id, SYNTH_WATCHDOG_RESET, 0, 0,
                               (const unsigned char*)0, 0,
                               (int *) 0,
                               (unsigned char*) 0, (int*) 0, 0);
    }
}

#endif // CYGIMP_WATCHDOG_HARDWARE
