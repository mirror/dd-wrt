#ifndef CYGONCE_POWER_POWER_H
# define CYGONCE_POWER_POWER_H
//==========================================================================
//
//      include/power.h
//
//      Definitions of power management support
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
// Date:         2001-06-12
//
//
//####DESCRIPTIONEND####
//==========================================================================

#include <pkgconf/power.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_tables.h>

#ifdef CYGPKG_POWER_THREAD
#include <cyg/kernel/kapi.h> // CYGPKG_POWER_THREAD is active_if CYGPKG_KERNEL so
                             // the kernel's headers are guaranteed to be available.
#endif

#ifdef __cplusplus
extern "C" {
#endif

// The four defined modes of power operation.
#define PowerMode_Min       0    
#define PowerMode_Active    0
#define PowerMode_Idle      1
#define PowerMode_Sleep     2
#define PowerMode_Off       3
#define PowerMode_Max       3
typedef cyg_uint8   PowerMode;

// Whether a mode change is initiated globally or per-controller
#define PowerModeChange_Controller      0
#define PowerModeChange_ControllerNow   1
#define PowerModeChange_Global          2
typedef cyg_uint32    PowerModeChange;

// Some priorities.
#define PowerPri_Early       1000
#define PowerPri_Typical     5000
#define PowerPri_Late        9000
    
typedef struct PowerController {
    void                (*change_mode)(struct PowerController*, PowerMode, PowerModeChange);
#ifdef CYGIMP_POWER_PROVIDE_STRINGS
    char*               id;
#endif
#ifdef CYGIMP_POWER_PROVIDE_POLICY_DATA
    CYG_ADDRWORD        policy_data;
#endif    
    PowerMode           mode;
    PowerMode           desired_mode;
    cyg_uint8           change_this;
    cyg_uint8           attached;
} CYG_HAL_TABLE_TYPE PowerController;

// The table of power controllers, allowing application code
// to access all of them.
extern PowerController __POWER__[], __POWER_END__;

// There should always be a power controller for the cpu which can
// be accessed by any client.
extern PowerController power_controller_cpu;

// If a separate power management thread is being used, its handle
// is exported so that other code can e.g. manipulate its priority
#ifdef CYGPKG_POWER_THREAD
extern cyg_handle_t power_thread_handle;
#endif
    
// Inline functions and supporting data.
extern PowerMode    __power_mode;
extern PowerMode    __power_desired_mode;
typedef void (*__power_policy_callback_t)(PowerController*, PowerMode, PowerMode, PowerMode, PowerMode);
extern __power_policy_callback_t __power_policy_callback;

// This macro is overridden elsewhere in this package when defining linkable
// extern (i.e. non-inline) versions of these functions.
#ifndef POWER_INLINE
# define POWER_INLINE extern inline
#endif

POWER_INLINE PowerMode
power_get_mode(void)
{
    return __power_mode;
}

POWER_INLINE PowerMode
power_get_desired_mode(void)
{
    return __power_desired_mode;
}

POWER_INLINE PowerMode
power_get_controller_mode(PowerController* controller)
{
    return controller->mode;
}

POWER_INLINE PowerMode
power_get_controller_desired_mode(PowerController* controller)
{
    return controller->desired_mode;
}

POWER_INLINE void
power_set_policy_callback(__power_policy_callback_t new_callback)
{
    __power_policy_callback = new_callback;
}

POWER_INLINE __power_policy_callback_t
power_get_policy_callback(void)
{
    return __power_policy_callback;
}

#ifdef CYGIMP_POWER_PROVIDE_STRINGS
POWER_INLINE const char*
power_get_controller_id(PowerController* controller)
{
    return controller->id;
}
#endif

#ifdef CYGIMP_POWER_PROVIDE_POLICY_DATA
POWER_INLINE void
power_set_controller_policy_data(PowerController* controller, CYG_ADDRWORD data)
{
    controller->policy_data = data;
}

POWER_INLINE CYG_ADDRWORD
power_get_controller_policy_data(PowerController* controller)
{
    return controller->policy_data;
}
#endif

POWER_INLINE cyg_bool
power_get_controller_attached(PowerController* controller)
{
    return controller->attached;
}

POWER_INLINE void
power_set_controller_attached(PowerController* controller, cyg_bool new_value)
{
    controller->attached = new_value;
}

POWER_INLINE void
power_set_controller_mode_now(PowerController* controller, PowerMode new_mode)
{
    controller->desired_mode = new_mode;
    if (controller->mode != controller->desired_mode) {
        (*controller->change_mode)(controller, new_mode, PowerModeChange_ControllerNow);
    }
}

// Now for the non-inline functions.
extern void         power_set_controller_mode(PowerController*, PowerMode);
extern void         power_set_mode(PowerMode);

// Implementation support.
#ifdef CYGIMP_POWER_PROVIDE_STRINGS
# define POWER_CONTROLLER_ID(_id_) id: _id_,
#else
# define POWER_CONTROLLER_ID(_id_)
#endif
#ifdef CYGIMP_POWER_PROVIDE_POLICY_DATA
# define POWER_CONTROLLER_POLICY_DATA policy_data: 0,
#else
# define POWER_CONTROLLER_POLICY_DATA
#endif

#define POWER_CONTROLLER(_name_, _pri_, _id_, _fn_)     \
PowerController _name_                                  \
    CYG_HAL_TABLE_QUALIFIED_ENTRY(power, _pri_) = {     \
    change_mode:    _fn_,                               \
    POWER_CONTROLLER_ID(_id_)                           \
    POWER_CONTROLLER_POLICY_DATA                        \
    mode:           PowerMode_Active,                   \
    desired_mode:   PowerMode_Active,                   \
    change_this:    0,                                  \
    attached:       1                                   \
}

#define POWER_CONTROLLER_CPU(_id_, _fn_)        \
PowerController power_controller_cpu            \
    CYG_HAL_TABLE_EXTRA(power) = {              \
    change_mode:        _fn_,                   \
    POWER_CONTROLLER_ID(_id_)                   \
    POWER_CONTROLLER_POLICY_DATA                \
    mode:               PowerMode_Active,       \
    desired_mode:       PowerMode_Active,       \
    change_this:        0,                      \
    attached:           1,                      \
}

#ifdef __cplusplus
} // extern "C" {
#endif

#endif // CYGONCE_POWER_POWER_H
