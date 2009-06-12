//==========================================================================
//
//      power.cxx
//
//      Main implementation of power management support.
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
// Author(s):    bartv
// Contributors: bartv
// Date:         2001-06-18
//
//####DESCRIPTIONEND####
//
//==========================================================================

// Provide the external (non-inline) definitions of the inline functions
// in power.h so there's something available in C code when the compiler
// chooses not to inline
#define POWER_INLINE extern "C"

#include <pkgconf/power.h>
#include <cyg/power/power.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/hal/hal_tables.h>

// ----------------------------------------------------------------------------
// Statics. Most of these are only relevant when a separate power
// management thread is being used. Some of these are exported, e.g.
// to allow the use of inline functions.

// The current power mode for the system as a whole.
PowerMode       __power_mode            = PowerMode_Active;

// The mode that the system should be running at.
PowerMode       __power_desired_mode    = PowerMode_Active;

// The policy callback function, if any.
__power_policy_callback_t __power_policy_callback = 0;

// This flag is used to abort a mode change. It allows a controller to
// call power_set_mode() while the mode is already being changed.
static volatile cyg_bool abort_mode_change = false;

#ifdef CYGPKG_POWER_THREAD
static unsigned char    power_thread_stack[CYGNUM_POWER_THREAD_STACKSIZE];
static cyg_thread       power_thread;
// The power management thread's handle is exported to support
// operations like changing the thread's priority.
cyg_handle_t     power_thread_handle;

// This semaphore is used to wake up the power management thread when there
// is work to be done.
static cyg_sem_t        power_thread_action;

#else
static cyg_bool         power_doing_it      = false;
static cyg_uint32       power_todo_count    = 0;
#endif

// ----------------------------------------------------------------------------
// Synchronisation.
//
// There are two exported functions to worry about: power_set_mode()
// and power_set_controller_mode(). There are also two main scenarios:
// CYGPKG_POWER_THREAD enabled and CYGPKG_POWER_THREAD_DISABLED.
//
// If CYGPKG_POWER_THREAD is enabled then any external code may at any
// time invoke the exported functions. These are asynchronous calls.
// In addition when the power management thread invokes a power
// controller that controller may also call the exported functions,
// synchronously. In either scenario the calls can return before the
// operation has completed, hence the policy callback functionality.
//
// If CYGPKG_POWER_THREAD is disabled then there may be only one
// external call to the exported functions, and the operation must
// complete before that call returns. If there are multiple concurrent
// external calls then the behaviour of the system is undefined.
// Really. It is still possible for power controllers to call the
// exported functions synchronously, which complicates things
// somewhat.
//
// The CYGPKG_POWER_THREAD case is the easier to handle. The power
// management thread simply loops forever, waiting on a semaphore
// until there is some work to be done and then checking internal
// state to figure out what that work should be. Some care has to be
// taken that the internal state gets updated and read atomically,
// which can be achieved by cyg_scheduler_lock() and unlock() calls in
// strategic places. Obviously it is undesirable to keep these locks
// longer than is absolutely necessary since that would impact
// dispatch latency, and in particular power controllers must not be
// invoked with the scheduler locked because there are no specific
// restrictions on what a controller may or may not do.
//
// The call graph is something like:
//    power_thread_fn()     - the thread entry point, loops waiting on the semaphore
//    power_doit()          - do the real work. This can be either a global mode
//                            change or one or more individual controller mode changes.
//                            Either operation involves iterating through the controllers.
//    power_change_controller_mode() - manipulate an individual controller.
//
// There is one little complication. If during a power_doit()
// set_mode() loop there is a call to power_set_mode() then the
// current loop should be aborted. This is especially important when
// switching to off mode and a controller has decided to cancel this
// via another call to set_mode().
//
// If no separate thread is used then there will only ever be one
// external call. That will result in an invocation of
// power_nothread_doit(), which in turn calls power_doit() and
// power_change_controller_mode() as in the threaded case. A flag is
// used so that it is possible to distinguish between external and
// synchronous calls, and a counter ensures that synchronous calls are
// processed correctly. Recursion is avoided so that stack usage
// remains deterministic.
//    power_set_mode()/power_set_controller_mode()
//    power_nothread_doit()
//    power_doit()
//    power_change_controller_mode();
//
// The main fields in the power controller data structures to worry
// about are "mode", "desired_mode", and "change_this". "mode" is only
// manipulated by the power controller itself, and since all power
// controller accesses are serialized no problems arise.
// "desired_mode" and "change_this" are updated by power_set_mode()
// and power_set_controller_mode(), and read by power_doit(). If a separate
// thread is in use then the scheduler lock protects access to thse fields.
// Without a separate thread concurrency is not an issue. Obviously there
// are other fields and variables, but most of these will only be set during
// system start-up and the rest do not require any special attention.

// ----------------------------------------------------------------------------
// Do the real work.
//
// power_change_controller_mode() acts on a single controller. It is invoked only
// from power_doit(), either for a global mode change or for an individual mode change.
// It should be invoked with the scheduler unlocked - power_doit() is responsible for
// synchronizing with the external calls.
static inline void
power_change_controller_mode(PowerController* controller, PowerMode desired_mode, cyg_bool change_this)
{
    // The policy callback will want to know the previous power mode.
    PowerMode old_mode = controller->mode;

    // Invoke the mode change operation. Note that
    // controller->change_this and controller->desired_mode may have
    // been updated by now, but at some point they did have values
    // which required a mode change.
    (*controller->change_mode)(controller, desired_mode, change_this ? PowerModeChange_Controller : PowerModeChange_Global);

    // Report the results to higher-level code. It is unlikely that
    // the policy callback will be changed while the system is running,
    // but just in case somebody installs a null pointer between the
    // check and the call...
    void        (*callback)(PowerController*, PowerMode, PowerMode, PowerMode, PowerMode) = __power_policy_callback;
    if (0 != callback) {
        (*callback)(controller, old_mode, controller->mode, desired_mode, controller->desired_mode);
    }
}

// power_doit() is responsible for a single iteration over the various controllers,
// aborting if there is a global mode change during the current iteration. The
// calling code, either power_thread_fn() or power_nothread_doit(), will take
// care of the higher-level iterating while there is work to be done.
//
// If a global mode change has been requested then the order in which the controllers
// are invoked is significant: front->back for lowering power modes, back->front for
// a higher power mode. If there are individual changes to be processed then
// arbitrarily front->back is used as well.
static inline void
power_doit()
{
    PowerController*    controller;

    abort_mode_change   = false;

    if (__power_desired_mode < __power_mode) {
        // The new mode is more active than the old one, so start with
        // the power controllers at the back of the table.
        for (controller = &(__POWER_END__) - 1; !abort_mode_change && (controller >= &(__POWER__[0])); controller--) {
            PowerMode   desired_mode;
            cyg_bool    change_this;
            
#ifdef CYGPKG_POWER_THREAD
            // Read the desired_mode and change_this flags atomically.
            cyg_scheduler_lock();
            desired_mode = controller->desired_mode;
            change_this  = controller->change_this;
            cyg_scheduler_unlock();
#else
            desired_mode = controller->desired_mode;
            change_this  = controller->change_this;
#endif
            // If this controller is not running at the desired mode, change it.
            if (desired_mode != controller->mode) {
                power_change_controller_mode(controller, desired_mode, change_this);
            }
        }
    } else {    // __power_desired_mode >= __power_mode.
        // Either a global mode change to a less active mode, or
        // one or more individual controller changes. Other than
        // iterating in a different direction, the code is the same
        // as above.
        for (controller = &(__POWER__[0]); !abort_mode_change && (controller != &(__POWER_END__)); controller++) {
            PowerMode   desired_mode;
            cyg_bool    change_this;
            
#ifdef CYGPKG_POWER_THREAD
            cyg_scheduler_lock();
            desired_mode = controller->desired_mode;
            change_this  = controller->change_this;
            cyg_scheduler_unlock();
#else
            desired_mode = controller->desired_mode;
            change_this  = controller->change_this;
#endif
            if (desired_mode != controller->mode) {
                power_change_controller_mode(controller, desired_mode, change_this);
            }
        }
    }

    // All of the controllers have been invoked. If there have been no
    // intervening calls to power_set_mode() (which would have updated
    // abort_mode_change) then we must now be running at the desired
    // global mode.
    if (!abort_mode_change) {
        __power_mode = __power_desired_mode;
    }
}

#ifdef CYGPKG_POWER_THREAD
static void
power_thread_fn(cyg_addrword_t param)
{
    for (;;) {
        // Currently idle. Wait for a request to change power modes.
        cyg_semaphore_wait(&power_thread_action);
        power_doit();
    }
}
#else
static inline void
power_nothread_doit()
{
    power_todo_count++;
    if (!power_doing_it) {
        power_doing_it = true;
        do {
            power_doit();
        } while (--power_todo_count > 0);
        power_doing_it = false;
    }
}
#endif

// ----------------------------------------------------------------------------
// The exported calls.

extern "C" void
power_set_controller_mode(PowerController* controller, PowerMode new_mode)
{
#ifdef CYGPKG_POWER_THREAD
    cyg_scheduler_lock();   // Protect against concurrent calls
#endif
    
    controller->desired_mode    = new_mode;
    controller->change_this     = true;

#ifdef CYGPKG_POWER_THREAD    
    cyg_scheduler_unlock();
    cyg_semaphore_post(&power_thread_action);
#else
    power_nothread_doit();
#endif    
}

extern "C" void
power_set_mode(PowerMode new_mode)
{
    PowerController*        controller;

#ifdef CYGPKG_POWER_THREAD
    cyg_scheduler_lock();
#endif
    
    __power_desired_mode    = new_mode;
    abort_mode_change       = true;
    // Update each controller. Most importantly, clear the
    // "change_this" flag in every power controller. The net result is
    // that power_set_mode() overrides any power_set_controller_mode()
    // operations that have not yet been processed, but future
    // power_set_controller_mode() calls will have the desired effect.
    for (controller = &(__POWER__[0]); controller != &(__POWER_END__); controller++) {
        if (controller->attached) {
            controller->change_this     = 0;
            controller->desired_mode    = new_mode;
        }
    }

#ifdef CYGPKG_POWER_THREAD    
    cyg_scheduler_unlock();
    cyg_semaphore_post(&power_thread_action);
#else
    power_nothread_doit();
#endif    
}

// ----------------------------------------------------------------------------
// Power management initialization. This gets called from
// power_data.cxx using a prioritized constructors. Doing this way
// minimizes the amount of data that is going to end up in libextras.a
// and hence in the final executable, allowing linker garbage collection
// to clean up as much as possible. The main operation here is to start
// up a separate power management thread when configured to do so.
//
// If no separate thread is being used then no run-time initialization
// is needed.
#ifdef CYGPKG_POWER_THREAD
extern "C" void
power_init(void)
{
    cyg_semaphore_init(&power_thread_action, 0);
    cyg_thread_create(CYGNUM_POWER_THREAD_PRIORITY,
                      &power_thread_fn,
                      (cyg_addrword_t) 0,
                      "Power management thread",
                      power_thread_stack,
                      CYGNUM_POWER_THREAD_STACKSIZE,
                      &power_thread_handle,
                      &power_thread
        );
    cyg_thread_resume(power_thread_handle);
}
#endif    

