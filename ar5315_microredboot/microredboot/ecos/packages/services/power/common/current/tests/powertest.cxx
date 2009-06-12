//==========================================================================
//
//      powertest.cxx
//
//      Testcase for the power management support.
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
// Date:         2001-07-29
//
// This testcase checks most of the functionality of the power management
// package. The main exception is the POWER_CONTROLLER_CPU macro and
// the power_controller_cpu variable: that controller may or may not be
// provided by one of the HAL packages, so attempting to manipulate that
// HAL could cause link-time conflicts.
//
// The testcase is likely to fail if some package supplies a policy
// module that is making concurrent calls, for example if that module
// installs its own callback. For now there is no way of detecting
// that scenario, but also for now policy modules are assumed to be
// provided by the application rather than by any eCos package.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/power.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/testcase.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/power/power.h>

#ifdef CYGPKG_KERNEL
// Additional #include's to support the use of multiple threads.
#include <cyg/kernel/kapi.h>
#include <cyg/kernel/sched.hxx>
#endif

// Additional prototypes. These are builtins, but in some configurations
// the relevant headers may not be present.
extern "C" int strcmp(const char*, const char*);

// ----------------------------------------------------------------------------
// Define the various tests in the system. This allows the power controllers
// and the policy callback to take appropriate action as required.
enum PowerTest {
    PowerTest_Presence,
    PowerTest_Order,
    PowerTest_InitialState,
    PowerTest_Ids
} current_test;

// ----------------------------------------------------------------------------
// Statics.
#ifdef CYGPKG_KERNEL
static unsigned char    policy_thread_stack[CYGNUM_HAL_STACK_SIZE_TYPICAL];
static cyg_thread       policy_thread;
static cyg_handle_t     policy_thread_handle;
#endif

// ----------------------------------------------------------------------------
// Provide three power controllers at different priorities.
static void xyzzy_power_controller_fn(PowerController*, PowerMode, PowerModeChange);
static void wumpus_power_controller_fn(PowerController*, PowerMode, PowerModeChange);
static void erwin_power_controller_fn(PowerController*, PowerMode, PowerModeChange);

POWER_CONTROLLER(xyzzy_controller,  PowerPri_Early,   "xyzzy",  &xyzzy_power_controller_fn);
POWER_CONTROLLER(wumpus_controller, PowerPri_Typical, "wumpus", &wumpus_power_controller_fn);
POWER_CONTROLLER(erwin_controller,  PowerPri_Late,    "erwin",  &erwin_power_controller_fn);

// ----------------------------------------------------------------------------
// The test cases.

// Check that all three power controllers are actually present in the array.
static void
test_presence(void)
{
    bool                seen_xyzzy  = false;
    bool                seen_wumpus = false;
    bool                seen_erwin  = false;
    PowerController*    controller;

    for (controller = &(__POWER__[0]); controller != &__POWER_END__; controller++) {
        if (controller == &xyzzy_controller) {
            CYG_ASSERTC(!seen_xyzzy);
            seen_xyzzy = true;
        } else if (controller == &wumpus_controller) {
            CYG_ASSERTC(!seen_wumpus);
            seen_wumpus = true;
        } else if (controller == &erwin_controller) {
            CYG_ASSERTC(!seen_erwin);
            seen_erwin = true;
        }
    }

    if (seen_xyzzy && seen_wumpus && seen_erwin) {
        CYG_TEST_PASS("All expected controllers found");
    } else {
        CYG_TEST_FAIL_FINISH("Failed to find all the controllers that should be present");
    }
}

// Now check that the controllers appear in the expected order.
static void
test_order(void)
{
    int                 xyzzy_index  = -1;
    int                 wumpus_index = -1;
    int                 erwin_index  = -1;
    int                 i;

    for (i = 0; &(__POWER__[i]) != &__POWER_END__; i++) {
        if (&__POWER__[i] == &xyzzy_controller) {
            CYG_ASSERTC(-1 == xyzzy_index);
            xyzzy_index = i;
        } else if (&__POWER__[i] == &wumpus_controller) {
            CYG_ASSERTC(-1 == wumpus_index);
            wumpus_index = i;
        } else if (&__POWER__[i] == &erwin_controller) {
            CYG_ASSERTC(-1 == erwin_index);
            erwin_index = i;
        }
    }
    CYG_ASSERTC(-1 != xyzzy_index);
    CYG_ASSERTC(-1 != wumpus_index);
    CYG_ASSERTC(-1 != erwin_index);

    if ((xyzzy_index < wumpus_index) && (wumpus_index < erwin_index)) {
        CYG_TEST_PASS("The power controllers are in the correct order");
    } else {
        CYG_TEST_FAIL_FINISH("The power controllers are not in the correct order");
    }
}

// Check the initial state of the system. Power controllers other than the ones
// defined by this testcase are ignored, since conceivably they might well come
// up in a mode other than active.
static void
test_initial_state(void)
{
    bool    test_ok = true;
    // There have been no calls to change power mode, so the system should be active
    if (PowerMode_Active != power_get_mode()) {
        CYG_TEST_FAIL("Initial power mode not active");
        test_ok = false;
    }
    if (PowerMode_Active != power_get_desired_mode()) {
        CYG_TEST_FAIL("Initial desired power mode not active");
        test_ok = false;
    }
    if ((void (*)(PowerController*, PowerMode, PowerMode, PowerMode, PowerMode)) 0 != power_get_policy_callback()) {
        CYG_TEST_FAIL("A policy callback has already been installed");
        test_ok = false;
    }

    if ((PowerMode_Active != power_get_controller_mode(&xyzzy_controller))  ||
        (PowerMode_Active != power_get_controller_mode(&wumpus_controller)) ||
        (PowerMode_Active != power_get_controller_mode(&erwin_controller))) {
        CYG_TEST_FAIL("Not all power controllers initialized to active");
        test_ok = false;
    }
    if ((PowerMode_Active != power_get_controller_desired_mode(&xyzzy_controller))  ||
        (PowerMode_Active != power_get_controller_desired_mode(&wumpus_controller)) ||
        (PowerMode_Active != power_get_controller_desired_mode(&erwin_controller))) {
        CYG_TEST_FAIL("Not all power controllers initialized to desired active");
        test_ok = false;
    }
    
    if (!power_get_controller_attached(&xyzzy_controller)  ||
        !power_get_controller_attached(&wumpus_controller) ||
        !power_get_controller_attached(&erwin_controller)) {
        CYG_TEST_FAIL("Some power controllers started off detached");
        test_ok = false;
    }

    // The initial policy data is checked in a separate testcase
    if (test_ok) {
        CYG_TEST_PASS("Initial power management state is as expected");
    }
}

// If the configuration involves per-controller id's, check them.
#ifdef CYGIMP_POWER_PROVIDE_STRINGS
static int test_ids_data    = 0;    // avoid problems with over-optimising

static void
test_ids(void)
{
    // All of the power controllers should have a valid id, i.e. it
    // should be possible to dereference the pointer. A SEGV here
    // probably indicates that a HAL or device driver package has
    // failed to supply a sensible string, which is a test failure
    // albeit one in that package rather than the power management
    // package.
    PowerController*    controller;
    for (controller = &(__POWER__[0]); controller != &__POWER_END__; controller++) {
        const char* id  = power_get_controller_id(controller);
        test_ids_data   += id[0];
    }

    // In addition the id strings for the three known controllers are also known.
    if ((0 == strcmp("xyzzy",  power_get_controller_id(&xyzzy_controller)))  &&
        (0 == strcmp("wumpus", power_get_controller_id(&wumpus_controller))) &&
        (0 == strcmp("erwin",  power_get_controller_id(&erwin_controller)))) {
        CYG_TEST_PASS("Controller id strings match up");
    } else {
        CYG_TEST_FAIL("Controller id strings fail to match");
    }
}
#else
static void
test_ids(void)
{
    CYG_TEST_NA("Unable to test controller ids - this functionaly has been configured out");
}
#endif
// ----------------------------------------------------------------------------
// The individual power controller functions. By default these just obey the
// requested mode change, but there are some special cases.
static void
xyzzy_power_controller_fn(PowerController* controller, PowerMode desired_mode, PowerModeChange change)
{
    switch (current_test) {
      default :
        controller->mode  = desired_mode;
        break;
    }
}

static void
wumpus_power_controller_fn(PowerController* controller, PowerMode desired_mode, PowerModeChange change)
{
    switch (current_test) {
      default :
        controller->mode  = desired_mode;
        break;
    }
}

static void
erwin_power_controller_fn(PowerController* controller, PowerMode desired_mode, PowerModeChange change)
{
    switch (current_test) {
      default :
        controller->mode  = desired_mode;
        break;
    }
}

// ----------------------------------------------------------------------------
// The main policy callback. Like the controller functions by default this
// does nothing, but for specific tests it has to take action.
static void
policy_callback(PowerController* controller, PowerMode old_mode, PowerMode current_mode, PowerMode old_desired_mode, PowerMode current_desired_mode)
{
}

// ----------------------------------------------------------------------------
// Utilities.

// Detach all controllers except the ones supplied by this package. It would
// be unfortunate if say the cpu stopped running because a test case decided
// to set the global mode to off.
static void
detach_all_controllers(void)
{
    PowerController*    controller;
    for (controller = &(__POWER__[0]); controller != &__POWER_END__; controller++) {
        if ((controller != &xyzzy_controller) && (controller != &wumpus_controller) && (controller != &erwin_controller)) {
            power_set_controller_attached(controller, false);
        }
    }
}

// ----------------------------------------------------------------------------
// The main routine, which acts as the policy module. This either runs as
// a separate thread, if the kernel is present, or else it is called
// directly from cyg_start().
static void
powertest_main(CYG_ADDRWORD data)
{
    CYG_ASSERT(0 == data, "No data should be supplied");

    CYG_TEST_INIT();
    
    current_test    = PowerTest_Presence;
    test_presence();

    current_test    = PowerTest_Order;
    test_order();

    current_test    = PowerTest_InitialState;
    test_initial_state();

    current_test    = PowerTest_Ids;
    test_ids();
    
    detach_all_controllers();

    CYG_TEST_FINISH("All tests have been run");
}

// ----------------------------------------------------------------------------
// Test startup.
externC void
cyg_start(void)
{
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif

#ifdef CYGPKG_KERNEL
    cyg_thread_create(CYG_SCHED_DEFAULT_INFO,
                      &powertest_main,
                      (cyg_addrword_t) 0,
                      "Power policy thread",
                      policy_thread_stack,
                      CYGNUM_HAL_STACK_SIZE_TYPICAL,
                      &policy_thread_handle,
                      &policy_thread);
    cyg_thread_resume(policy_thread_handle);
    Cyg_Scheduler::start();
    CYG_TEST_FAIL_FINISH("Not reached");
#else    
    powertest_main(0);
#endif
}




