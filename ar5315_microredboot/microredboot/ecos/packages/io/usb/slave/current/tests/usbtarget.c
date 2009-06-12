/*{{{  Banner                                                   */

/*=================================================================
//
//        target.c
//
//        USB testing - target-side
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
// This program performs appropriate USB initialization and initializes
// itself as a specific type of USB peripheral, Red Hat eCos testing.
// There is no actual host-side device driver for this, instead there is
// a test application which performs ioctl's on /proc/bus/usb/... and
// makes appropriate functionality available to a Tcl script.
//
// Author(s):     bartv
// Date:          2001-07-04
//####DESCRIPTIONEND####
//==========================================================================
*/

/*}}}*/
/*{{{  #include's                                               */

#include <stdio.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <cyg/kernel/kapi.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/io/io.h>
#include <cyg/io/usb/usbs.h>
#include <cyg/infra/testcase.h>
#include "protocol.h"

/*}}}*/

/*{{{  Statics                                                  */

// ----------------------------------------------------------------------------
// Statics.

// The number of endpoints supported by the device driver.
static int number_endpoints     = 0;

// The control endpoint
static usbs_control_endpoint* control_endpoint = (usbs_control_endpoint*) 0;

// Buffers for incoming and outgoing data, and a length field.
static unsigned char class_request[USBTEST_MAX_CONTROL_DATA + 1];
static unsigned char class_reply[USBTEST_MAX_CONTROL_DATA + 1];
static int           class_request_size  = 0;

// This semaphore is used by DSRs to wake up the main thread when work has to
// be done at thread level.
static cyg_sem_t    main_wakeup;

// And this function pointer identifies the work that has to be done.
static void         (*main_thread_action)(void)  = (void (*)(void)) 0;

// Is the system still busy processing a previous request? This variable is
// checked in response to the synch request. It may get updated in
// DSRs as well as at thread level, hence volatile.
static volatile int idle    = 1;

// Are any tests currently running?
static int          running = 0;

// Has the current batch of tests been terminated by the host? This
// flag is checked by the various test handlers at appropriate
// intervals, and helps to handle the case where one of the side has
// terminated early because an error has been detected.
static int          current_tests_terminated = 0;

// A counter for the number of threads involved in the current batch of tests.
static int          thread_counter    = 0;

// An extra buffer for recovery operations, for example to accept and discard
// data which the host is still trying to send.
static unsigned char recovery_buffer[USBTEST_MAX_BULK_DATA + USBTEST_MAX_BULK_DATA_EXTRA];

/*}}}*/
/*{{{  Logging                                                  */

// ----------------------------------------------------------------------------
// The target-side code can provide various levels of run-time logging.
// Obviously the verbose flag cannot be controlled by a command-line
// argument, but it can be set from inside gdb or alternatively by
// a request from the host.
//
// NOTE: is printf() the best I/O routine to use here?

static int verbose = 0;

#define VERBOSE(_level_, _format_, _args_...)   \
    do {                                        \
        if (verbose >= _level_) {               \
            diag_printf(_format_, ## _args_);        \
        }                                       \
    } while (0);

/*}}}*/
/*{{{  Utilities                                                */

// ----------------------------------------------------------------------------
// A reimplementation of nanosleep, to avoid having to pull in the
// POSIX compatibility testing for USB testing.
static void
usbs_nanosleep(int delay)
{
    cyg_tick_count_t ticks;
    cyg_resolution_t resolution = cyg_clock_get_resolution(cyg_real_time_clock());

    // (resolution.dividend/resolution.divisor) == nanoseconds/tick
    //   e.g. 1000000000/100, i.e. 10000000 ns or 10 ms per tick
    // So ticks = (delay * divisor) / dividend
    //   e.g. (10000000 * 100) / 1000000000
    // with a likely value of 0 for delays of less than the clock resolution,
    // so round those up to one tick.

    cyg_uint64 tmp = (cyg_uint64) delay;
    tmp *= (cyg_uint64) resolution.divisor;
    tmp /= (cyg_uint64) resolution.dividend;

    ticks = (int) tmp;
    if (0 != ticks) {
        cyg_thread_delay(ticks);
    }
}

// ----------------------------------------------------------------------------
// Fix any problems in the driver-supplied endpoint data
//
// Maximum transfer sizes are limited not just by the capabilities
// of the driver but also by the testing code itself, since e.g.
// buffers for transfers are statically allocated.
static void
fix_driver_endpoint_data(void)
{
    int i;
    
    for (i = 0; !USBS_TESTING_ENDPOINTS_IS_TERMINATOR(usbs_testing_endpoints[i]); i++) {
        if (USB_ENDPOINT_DESCRIPTOR_ATTR_BULK == usbs_testing_endpoints[i].endpoint_type) {
            if ((-1 == usbs_testing_endpoints[i].max_size) ||
                (usbs_testing_endpoints[i].max_size > USBTEST_MAX_BULK_DATA)) {
                usbs_testing_endpoints[i].max_size = USBTEST_MAX_BULK_DATA;
            }
        }
    }
}

// ----------------------------------------------------------------------------
// A heartbeat thread.
//
// USB tests can run for a long time with no traffic on the debug channel,
// which can cause problems. To avoid problems it is possible to have a
// heartbeat thread running in the background, sending output at one
// second intervals.
//
// Depending on the configuration the output may still be line-buffered,
// but that is still sufficient to keep things happy.

static cyg_bool     heartbeat = false;
static cyg_thread   heartbeat_data;
static cyg_handle_t heartbeat_handle;
static char         heartbeat_stack[CYGNUM_HAL_STACK_SIZE_TYPICAL];

static void
heartbeat_function(cyg_addrword_t arg __attribute((unused)))
{
    char* message = "alive\n";
    int i;
    
    for ( i = 0; ; i = (i + 1) % 6) {
        usbs_nanosleep(1000000000);
        if (heartbeat) {
            diag_write_char(message[i]);
        }
    }
}

static void
start_heartbeat(void)
{
    cyg_thread_create( 0, &heartbeat_function, 0,
                       "heartbeat", heartbeat_stack, CYGNUM_HAL_STACK_SIZE_TYPICAL,
                       &heartbeat_handle, &heartbeat_data);
    cyg_thread_resume(heartbeat_handle);
}


/*}}}*/
/*{{{  Endpoint usage                                           */

// ----------------------------------------------------------------------------
// It is important to keep track of which endpoints are currently in use,
// because the behaviour of the USB I/O routines is undefined if there are
// concurrent attempts to communicate on the same endpoint. Normally this is
// not a problem because the host will ensure that a given endpoint is used
// for only one endpoint at a time, but when performing recovery action it
// is important that the system is sure that a given endpoint can be accessed
// safely.

static cyg_bool in_endpoint_in_use[16];
static cyg_bool out_endpoint_in_use[16];

// Lock the given endpoint. In theory this is only ever accessed from a single
// test thread at a time, but just in case...
static void
lock_endpoint(int endpoint, int direction)
{
    CYG_ASSERTC((endpoint >=0) && (endpoint < 16));
    CYG_ASSERTC((USB_ENDPOINT_DESCRIPTOR_ENDPOINT_IN == direction) || (USB_ENDPOINT_DESCRIPTOR_ENDPOINT_OUT == direction));
    
    cyg_scheduler_lock();
    if (0 == endpoint) {
        // Comms traffic on endpoint 0 is implemented using reserved control messages.
        // It is not really possible to have concurrent IN and OUT operations because
        // the two would interfere with each other.
        CYG_ASSERTC(!in_endpoint_in_use[0] && !out_endpoint_in_use[0]);
        in_endpoint_in_use[0]  = true;
        out_endpoint_in_use[0] = true;
    } else if (USB_ENDPOINT_DESCRIPTOR_ENDPOINT_IN == direction) {
        CYG_ASSERTC(!in_endpoint_in_use[endpoint]);
        in_endpoint_in_use[endpoint] = true;
    } else {
        CYG_ASSERTC(!out_endpoint_in_use[endpoint]);
        out_endpoint_in_use[endpoint] = true;
    }
    cyg_scheduler_unlock();
}

static void
unlock_endpoint(int endpoint, int direction)
{
    CYG_ASSERTC((endpoint >= 0) && (endpoint < 16));
    CYG_ASSERTC((USB_ENDPOINT_DESCRIPTOR_ENDPOINT_IN == direction) || (USB_ENDPOINT_DESCRIPTOR_ENDPOINT_OUT == direction));
    
    if (0 == endpoint) {
        CYG_ASSERTC(in_endpoint_in_use[0] && out_endpoint_in_use[0]);
        in_endpoint_in_use[0]   = false;
        out_endpoint_in_use[0]  = false;
    } else if (USB_ENDPOINT_DESCRIPTOR_ENDPOINT_IN == direction) {
        CYG_ASSERTC(in_endpoint_in_use[endpoint]);
        in_endpoint_in_use[endpoint] = false;
    } else {
        CYG_ASSERTC(out_endpoint_in_use[endpoint]);
        out_endpoint_in_use[endpoint] = false;
    }
}

static cyg_bool
is_endpoint_locked(int endpoint, int direction)
{
    cyg_bool    result = false;
    
    if (0 == endpoint) {
        result = in_endpoint_in_use[0];
    } else if (USB_ENDPOINT_DESCRIPTOR_ENDPOINT_IN == direction) {
        result = in_endpoint_in_use[endpoint];
    } else {
        result = out_endpoint_in_use[endpoint];
    }
    return result;
}

// For a given endpoint number, direction and protocol, search through the table 
// supplied by the device driver of all available endpoints. This can be used
// to e.g. get hold of the name of the devtab entry or a pointer to the endpoint
// data structure itself.
static int
lookup_endpoint(int endpoint_number, int direction, int protocol)
{
    int result = -1;
    int i;

    for (i = 0; !USBS_TESTING_ENDPOINTS_IS_TERMINATOR(usbs_testing_endpoints[i]); i++) {
        if ((usbs_testing_endpoints[i].endpoint_type        == protocol)        &&
            (usbs_testing_endpoints[i].endpoint_number      == endpoint_number) &&
            (usbs_testing_endpoints[i].endpoint_direction   == direction)) {
            result = i;
            break;
        }
    }
    return result;
}

/*}}}*/
/*{{{  Enumeration data                                         */

// ----------------------------------------------------------------------------
// The enumeration data.
//
// For simplicity this configuration involves just a single interface.
// The target has to list all the endpoints, or the Linux kernel will
// not allow application code to access them. Hence the information
// provided by the device drivers has to be turned into endpoint descriptors.

usb_configuration_descriptor usb_configuration = {
    length:             USB_CONFIGURATION_DESCRIPTOR_LENGTH,
    type:               USB_CONFIGURATION_DESCRIPTOR_TYPE,
    total_length_lo:    USB_CONFIGURATION_DESCRIPTOR_TOTAL_LENGTH_LO(1, 0),
    total_length_hi:    USB_CONFIGURATION_DESCRIPTOR_TOTAL_LENGTH_HI(1, 0),
    number_interfaces:  1,
    configuration_id:   1,      // id 0 is special according to the spec
    configuration_str:  0,
    attributes:         USB_CONFIGURATION_DESCRIPTOR_ATTR_REQUIRED |
                        USB_CONFIGURATION_DESCRIPTOR_ATTR_SELF_POWERED,
    max_power:          50
};

usb_interface_descriptor usb_interface = {
    length:             USB_INTERFACE_DESCRIPTOR_LENGTH,
    type:               USB_INTERFACE_DESCRIPTOR_TYPE,
    interface_id:       0,
    alternate_setting:  0,
    number_endpoints:   0,
    interface_class:    USB_INTERFACE_DESCRIPTOR_CLASS_VENDOR,
    interface_subclass: USB_INTERFACE_DESCRIPTOR_SUBCLASS_VENDOR,
    interface_protocol: USB_INTERFACE_DESCRIPTOR_PROTOCOL_VENDOR,
    interface_str:      0
};

usb_endpoint_descriptor usb_endpoints[USBTEST_MAX_ENDPOINTS];
                               
const unsigned char* usb_strings[] = {
    "\004\003\011\004",
    "\020\003R\000e\000d\000 \000H\000a\000t\000",
    "\054\003R\000e\000d\000 \000H\000a\000t\000 \000e\000C\000o\000s\000 \000"
    "U\000S\000B\000 \000t\000e\000s\000t\000"
};

usbs_enumeration_data usb_enum_data = {
    {
        length:                 USB_DEVICE_DESCRIPTOR_LENGTH,
        type:                   USB_DEVICE_DESCRIPTOR_TYPE,
        usb_spec_lo:            USB_DEVICE_DESCRIPTOR_USB11_LO,
        usb_spec_hi:            USB_DEVICE_DESCRIPTOR_USB11_HI,
        device_class:           USB_DEVICE_DESCRIPTOR_CLASS_VENDOR,
        device_subclass:        USB_DEVICE_DESCRIPTOR_SUBCLASS_VENDOR,
        device_protocol:        USB_DEVICE_DESCRIPTOR_PROTOCOL_VENDOR,
        max_packet_size:        8,
        vendor_lo:              0x42,   // Note: this is not an allocated vendor id
        vendor_hi:              0x42,
        product_lo:             0x00,
        product_hi:             0x01,
        device_lo:              0x00,
        device_hi:              0x01,
        manufacturer_str:       1,
        product_str:            2,
        serial_number_str:      0,
        number_configurations:  1
    },
    total_number_interfaces:    1,
    total_number_endpoints:     0,
    total_number_strings:       3,
    configurations:             &usb_configuration,
    interfaces:                 &usb_interface,
    endpoints:                  usb_endpoints,
    strings:                    usb_strings
};

static void
provide_endpoint_enumeration_data(void)
{
    int enum_endpoint_count = 0;
    int i;

    for (i = 0; !USBS_TESTING_ENDPOINTS_IS_TERMINATOR(usbs_testing_endpoints[i]); i++) {

        // The control endpoint need not appear in the enumeration data.
        if (USB_ENDPOINT_DESCRIPTOR_ATTR_CONTROL == usbs_testing_endpoints[i].endpoint_type) {
            continue;
        }

        usb_endpoints[enum_endpoint_count].length          = USB_ENDPOINT_DESCRIPTOR_LENGTH;
        usb_endpoints[enum_endpoint_count].type            = USB_ENDPOINT_DESCRIPTOR_TYPE;
        usb_endpoints[enum_endpoint_count].endpoint        = usbs_testing_endpoints[i].endpoint_number |
                                                             usbs_testing_endpoints[i].endpoint_direction;

        switch (usbs_testing_endpoints[i].endpoint_type) {
          case USB_ENDPOINT_DESCRIPTOR_ATTR_BULK:
            usb_endpoints[enum_endpoint_count].attributes      = USB_ENDPOINT_DESCRIPTOR_ATTR_BULK;
            usb_endpoints[enum_endpoint_count].max_packet_lo   = 64;
            usb_endpoints[enum_endpoint_count].max_packet_hi   = 0;
            usb_endpoints[enum_endpoint_count].interval        = 0;
            break;
            
          case USB_ENDPOINT_DESCRIPTOR_ATTR_ISOCHRONOUS:
            usb_endpoints[enum_endpoint_count].attributes      = USB_ENDPOINT_DESCRIPTOR_ATTR_ISOCHRONOUS;
            usb_endpoints[enum_endpoint_count].max_packet_lo   = usbs_testing_endpoints[i].max_size & 0x0FF;
            usb_endpoints[enum_endpoint_count].max_packet_hi   = (usbs_testing_endpoints[i].max_size >> 8) & 0x0FF;
            usb_endpoints[enum_endpoint_count].interval        = 1;
            break;
            
          case USB_ENDPOINT_DESCRIPTOR_ATTR_INTERRUPT:
            usb_endpoints[enum_endpoint_count].attributes      = USB_ENDPOINT_DESCRIPTOR_ATTR_INTERRUPT;
            usb_endpoints[enum_endpoint_count].max_packet_lo   = (unsigned char) usbs_testing_endpoints[i].max_size;
            usb_endpoints[enum_endpoint_count].max_packet_hi   = 0;
            usb_endpoints[enum_endpoint_count].interval        = 1;    // NOTE: possibly incorrect
            break;
        }

        enum_endpoint_count++;
    }

    usb_interface.number_endpoints          = enum_endpoint_count;
    usb_enum_data.total_number_endpoints    = enum_endpoint_count;
    usb_configuration.total_length_lo       = USB_CONFIGURATION_DESCRIPTOR_TOTAL_LENGTH_LO(1, enum_endpoint_count);
    usb_configuration.total_length_hi       = USB_CONFIGURATION_DESCRIPTOR_TOTAL_LENGTH_HI(1, enum_endpoint_count);
}

/*}}}*/
/*{{{  Host/target common code                                  */

#define TARGET
#include "common.c"

/*}}}*/
/*{{{  The tests                                                */

/*{{{  UsbTest structure                                        */

// ----------------------------------------------------------------------------
// All the information associated with a particular testcase. Much of this
// is identical to the equivalent host-side structure, but some additional
// information is needed so the structure and associated routines are not
// shared.
typedef struct UsbTest {

    // A unique identifier to make verbose output easier to understand
    int                 id;
    
    // Which test should be run
    usbtest             which_test;

    // Test-specific details.
    union {
        UsbTest_Bulk        bulk;
        UsbTest_ControlIn   control_in;
    } test_params;

    // How to recover from any problems. Specifically, what kind of message
    // could the target send or receive that would unlock the thread on this
    // side.
    UsbTest_Recovery    recovery;

    // The test result, to be collected and passed back to the host.
    int                 result_pass;
    char                result_message[USBTEST_MAX_MESSAGE];

    // Support for synchronization. This allows the UsbTest structure to be
    // used as the callback data for low-level USB calls.
    cyg_sem_t           sem;
    int                 transferred;

    // Some tests may need extra cancellation support
    void                (*cancel_fn)(struct UsbTest*);
    unsigned char       buffer[USBTEST_MAX_BULK_DATA + USBTEST_MAX_BULK_DATA_EXTRA];
} UsbTest;

// Reset the information in a given test. This is used by the pool allocation
// code. The data union is left alone, filling in the appropriate union
// member is left to other code.
static void
reset_usbtest(UsbTest* test)
{
    static int next_id = 1;
    test->id                    = next_id++;
    test->which_test            = usbtest_invalid;
    usbtest_recovery_reset(&(test->recovery));
    test->result_pass           = 0;
    test->result_message[0]     = '\0';
    cyg_semaphore_init(&(test->sem), 0);
    test->transferred           = 0;
    test->cancel_fn             = (void (*)(UsbTest*)) 0;
}

// Forward declaration. The pool code depends on run_test(), setting up a test requires the pool.
static UsbTest* pool_allocate(void);

/*}}}*/
/*{{{  Bulk transfers                                           */

/*{{{  handle_test_bulk()                                       */

// Prepare for a bulk transfer test. This means allocating a thread to do
// the work, and extracting the test parameters from the current buffer.
// The thread allocation code does not require any locking since all worker
// threads should be idle when starting a new thread, so the work can be
// done entirely at DSR level and no synch is required.
static usbs_control_return
handle_test_bulk(usb_devreq* req)
{
    UsbTest*    test;
    int         index   = 0;

    test = pool_allocate();
    unpack_usbtest_bulk(&(test->test_params.bulk), class_request, &index);
    test->which_test = (USB_DEVREQ_DIRECTION_IN == (test->test_params.bulk.endpoint & USB_DEVREQ_DIRECTION_MASK)) ?
        usbtest_bulk_in : usbtest_bulk_out;

    VERBOSE(3, "Preparing USB bulk test on endpoint %d, direction %s, for %d packets\n", \
            test->test_params.bulk.endpoint & ~USB_DEVREQ_DIRECTION_MASK,                \
            (usbtest_bulk_in == test->which_test) ? "IN" : "OUT",                           \
            test->test_params.bulk.number_packets);
    VERBOSE(3, "  I/O mechanism is %s\n", \
            (usb_io_mechanism_usb == test->test_params.bulk.io_mechanism) ? "low-level USB" : \
            (usb_io_mechanism_dev == test->test_params.bulk.io_mechanism) ? "devtab" : "<invalid>");
    VERBOSE(3, "  Data format %s, data1 %d, data* %d, data+ %d, data1* %d, data1+ %d, data** %d, data*+ %d, data+* %d, data++ %d\n",\
            (usbtestdata_none     == test->test_params.bulk.data.format) ? "none" :     \
            (usbtestdata_bytefill == test->test_params.bulk.data.format) ? "bytefill" : \
            (usbtestdata_wordfill == test->test_params.bulk.data.format) ? "wordfill" : \
            (usbtestdata_byteseq  == test->test_params.bulk.data.format) ? "byteseq"  : \
            (usbtestdata_wordseq  == test->test_params.bulk.data.format) ? "wordseq"  : "<invalid>", \
            test->test_params.bulk.data.seed,                            \
            test->test_params.bulk.data.multiplier,                      \
            test->test_params.bulk.data.increment,                       \
            test->test_params.bulk.data.transfer_seed_multiplier,        \
            test->test_params.bulk.data.transfer_seed_increment,         \
            test->test_params.bulk.data.transfer_multiplier_multiplier,  \
            test->test_params.bulk.data.transfer_multiplier_increment,   \
            test->test_params.bulk.data.transfer_increment_multiplier,   \
            test->test_params.bulk.data.transfer_increment_increment);
    VERBOSE(3, "  txsize1 %d, txsize>= %d, txsize<= %d, txsize* %d, txsize/ %d, txsize+ %d\n", \
            test->test_params.bulk.tx_size,         test->test_params.bulk.tx_size_min,        \
            test->test_params.bulk.tx_size_max,     test->test_params.bulk.tx_size_multiplier, \
            test->test_params.bulk.tx_size_divisor, test->test_params.bulk.tx_size_increment);
    VERBOSE(3, "  rxsize1 %d, rxsize>= %d, rxsize<= %d, rxsize* %d, rxsize/ %d, rxsize+ %d\n", \
            test->test_params.bulk.rx_size,         test->test_params.bulk.rx_size_min,        \
            test->test_params.bulk.rx_size_max,     test->test_params.bulk.rx_size_multiplier, \
            test->test_params.bulk.rx_size_divisor, test->test_params.bulk.rx_size_increment);
    VERBOSE(3, "  txdelay1 %d, txdelay>= %d, txdelay<= %d, txdelay* %d, txdelay/ %d, txdelay+ %d\n", \
            test->test_params.bulk.tx_delay,         test->test_params.bulk.tx_delay_min,            \
            test->test_params.bulk.tx_delay_max,     test->test_params.bulk.tx_delay_multiplier,     \
            test->test_params.bulk.tx_delay_divisor, test->test_params.bulk.tx_delay_increment);
    VERBOSE(3, "  rxdelay1 %d, rxdelay>= %d, rxdelay<= %d, rxdelay* %d, rxdelay/ %d, rxdelay+ %d\n", \
            test->test_params.bulk.rx_delay,         test->test_params.bulk.rx_delay_min,            \
            test->test_params.bulk.rx_delay_max,     test->test_params.bulk.rx_delay_multiplier,     \
            test->test_params.bulk.rx_delay_divisor, test->test_params.bulk.rx_delay_increment);
    
    return USBS_CONTROL_RETURN_HANDLED;
}

/*}}}*/
/*{{{  run_test_bulk_out()                                      */

// The same callback can be used for IN and OUT transfers. Note that
// starting the next transfer is left to the thread, it is not done
// at DSR level.
static void
run_test_bulk_in_out_callback(void* callback_arg, int transferred)
{
    UsbTest*    test    = (UsbTest*) callback_arg;
    test->transferred   = transferred;
    cyg_semaphore_post(&(test->sem));
}

// OUT transfers, i.e. the host will be sending some number of
// packets. The I/O can happen in a number of different ways, e.g. via
// the low-level USB API or via devtab routines.
static void
run_test_bulk_out(UsbTest* test)
{
    unsigned char*      buf;
    int                 endpoint_number = test->test_params.bulk.endpoint & ~USB_DEVREQ_DIRECTION_MASK;
    int                 ep_index;
    usbs_rx_endpoint*   endpoint        = 0;
    cyg_io_handle_t     io_handle       = (cyg_io_handle_t)0;
    int                 alignment;
    int                 transferred;
    int                 i;

    VERBOSE(1, "Starting test %d, bulk out on endpoint %d\n", test->id, endpoint_number);

    ep_index = lookup_endpoint(endpoint_number, USB_ENDPOINT_DESCRIPTOR_ENDPOINT_OUT, USB_ENDPOINT_DESCRIPTOR_ATTR_BULK);
    if (ep_index == -1) {
            test->result_pass   = 0;
            snprintf(test->result_message, USBTEST_MAX_MESSAGE,
                     "Target, bulk OUT transfer on endpoint %d: no such bulk endpoint", endpoint_number);
            return;
    }
    endpoint    = (usbs_rx_endpoint*) usbs_testing_endpoints[ep_index].endpoint;
    alignment   = usbs_testing_endpoints[ep_index].alignment;
    if (0 != alignment) {
        buf         = (unsigned char*) ((((cyg_uint32)test->buffer) + alignment - 1) & ~(alignment - 1));
    } else {
        buf = test->buffer;
    }
    
    CYG_ASSERTC((usb_io_mechanism_usb == test->test_params.bulk.io_mechanism) || \
                (usb_io_mechanism_dev == test->test_params.bulk.io_mechanism));
    if (usb_io_mechanism_dev == test->test_params.bulk.io_mechanism) {
        if (((const char*)0 == usbs_testing_endpoints[ep_index].devtab_entry) ||
            (0 != cyg_io_lookup(usbs_testing_endpoints[ep_index].devtab_entry, &io_handle))) {
            
            test->result_pass   = 0;
            snprintf(test->result_message, USBTEST_MAX_MESSAGE,
                     "Target, bulk OUT transfer on endpoint %d: no devtab entry", endpoint_number);
            return;
        }
    }

    // Make sure nobody else is using this endpoint
    lock_endpoint(endpoint_number, USB_ENDPOINT_DESCRIPTOR_ENDPOINT_OUT);

    for (i = 0; i < test->test_params.bulk.number_packets; i++) {
        int rx_size = test->test_params.bulk.rx_size;
        int tx_size = test->test_params.bulk.tx_size;

        VERBOSE(2, "Bulk OUT test %d: iteration %d, rx size %d, tx size %d\n", test->id, i, rx_size, tx_size);
        
        if (rx_size < tx_size) {
            rx_size = tx_size;
            VERBOSE(2, "Bulk OUT test %d: iteration %d, packet size reset to %d to match tx size\n",
                    test->id, i, rx_size);
        }
                                                              
        test->recovery.endpoint     = endpoint_number | USB_ENDPOINT_DESCRIPTOR_ENDPOINT_OUT;
        test->recovery.protocol     = USB_ENDPOINT_DESCRIPTOR_ATTR_BULK;
        test->recovery.size         = rx_size;

        // Make sure there is no old data lying around
        if (usbtestdata_none != test->test_params.bulk.data.format) {
            memset(buf, 0, rx_size);
        }

        // Do the actual transfer, using the I/O mechanism specified for this test.
        switch (test->test_params.bulk.io_mechanism)
        {
          case usb_io_mechanism_usb :
          {
              test->transferred = 0;
              usbs_start_rx_buffer(endpoint, buf, rx_size, &run_test_bulk_in_out_callback, (void*) test);
              cyg_semaphore_wait(&(test->sem));
              transferred = test->transferred;
              break;
          }

          case usb_io_mechanism_dev :
          {
              int result;
              transferred   = rx_size;
              result = cyg_io_read(io_handle, (void*) buf, &transferred);
              if (result < 0) {
                  transferred = result;
              }
              break;
          }

          default:
            CYG_FAIL("Invalid test mechanism specified");
            break;
        }

        // Has this test been aborted for some reason?
        if (current_tests_terminated) {
            VERBOSE(2, "Bulk OUT test %d: iteration %d, termination detected\n", test->id, i);
            test->result_pass = 0;
            snprintf(test->result_message, USBTEST_MAX_MESSAGE,
                     "Target, bulk OUT transfer on endpoint %d: transfer aborted after iteration %d", endpoint_number, i);
            break;
        }

        // If an error occurred, abort this run
        if (transferred < 0) {
            test->result_pass   = 0;
            snprintf(test->result_message, USBTEST_MAX_MESSAGE,
                     "Target, bulk OUT transfer on endpoint %d: transfer failed with %d", endpoint_number, transferred);
            VERBOSE(2, "Bulk OUT test %d: iteration %d, error:\n    %s\n", test->id, i, test->result_message);
            break;
        }

        // Did the host send the expected amount of data?
        if (transferred < test->test_params.bulk.tx_size) {
            test->result_pass   = 0;
            snprintf(test->result_message, USBTEST_MAX_MESSAGE,
                     "Target, bulk OUT transfer on endpoint %d : the host only sent %d bytes when %d were expected",
                     endpoint_number, transferred, tx_size);
            VERBOSE(2, "Bulk OUT test %d: iteration %d, error:\n    %s\n", test->id, i, test->result_message);
            break;
        }

        if (verbose >= 3) {
            // Output the first 32 bytes of data
            char msg[256];
            int  index;
            int  j;
            index = snprintf(msg, 255, "Bulk OUT test %d: iteration %d, transferred %d\n    Data %s:",
                             test->id, i, transferred,
                             (usbtestdata_none == test->test_params.bulk.data.format) ? "(uninitialized)" : "");

            for (j = 0; ((j + 3) < transferred) && (j < 32); j+= 4) {
                index += snprintf(msg+index, 255-index, " %02x%02x%02x%02x",
                                  buf[j], buf[j+1], buf[j+2], buf[j+3]);
            }
            if (j < 32) {
                index += snprintf(msg+index, 255-index, " ");
                for ( ; j < transferred; j++) {
                    index += snprintf(msg+index, 255-index, "%02x", buf[j]);
                }
                
            }
            VERBOSE(3, "%s\n", msg);
        }
        
        // Is the data correct?
        if (!usbtest_check_buffer(&(test->test_params.bulk.data), buf, transferred)) {
            test->result_pass   = 0;
            snprintf(test->result_message, USBTEST_MAX_MESSAGE,
                     "Target, bulk OUT transfer on endpoint %d : mismatch between received and expected data", endpoint_number);
            VERBOSE(2, "Bulk OUt test %d: iteration %d, error:\n    %s\n", test->id, i, test->result_message);
            break;
        }

        if (0 != test->test_params.bulk.rx_delay) {
            VERBOSE(2, "Bulk OUT test %d: iteration %d, sleeping for %d nanoseconds\n", test->id, \
                    i, test->test_params.bulk.rx_delay);
            usbs_nanosleep(test->test_params.bulk.rx_delay);
        }
        
        // Move on to the next transfer
        USBTEST_BULK_NEXT(test->test_params.bulk);
    }

    // Always unlock the endpoint on completion
    unlock_endpoint(endpoint_number, USB_ENDPOINT_DESCRIPTOR_ENDPOINT_OUT);

    // If all the packets have been transferred this test has passed.
    if (i >= test->test_params.bulk.number_packets) {
        test->result_pass   = 1;
    }
    
    VERBOSE(1, "Test %d bulk OUT on endpoint %d, result %d\n", test->id, endpoint_number, test->result_pass);
}

/*}}}*/
/*{{{  run_test_bulk_in()                                       */

// IN transfers, i.e. the host is expected to receive some data. These are slightly
// easier than OUT transfers because it is the host that will do the checking.
static void
run_test_bulk_in(UsbTest* test)
{
    unsigned char*      buf;
    int                 endpoint_number = test->test_params.bulk.endpoint & ~USB_DEVREQ_DIRECTION_MASK;
    int                 ep_index;
    usbs_tx_endpoint*   endpoint        = 0;
    cyg_io_handle_t     io_handle       = (cyg_io_handle_t)0;
    int                 alignment;
    int                 transferred;
    int                 i;

    VERBOSE(1, "Starting test %d, bulk IN on endpoint %d\n", test->id, endpoint_number);
    
    ep_index = lookup_endpoint(endpoint_number, USB_ENDPOINT_DESCRIPTOR_ENDPOINT_IN, USB_ENDPOINT_DESCRIPTOR_ATTR_BULK);
    if (ep_index == -1) {
            test->result_pass   = 0;
            snprintf(test->result_message, USBTEST_MAX_MESSAGE,
                     "Target, bulk IN transfer on endpoint %d: no such bulk endpoint", endpoint_number);
            return;
    }
    endpoint    = (usbs_tx_endpoint*) usbs_testing_endpoints[ep_index].endpoint;
    alignment   = usbs_testing_endpoints[ep_index].alignment;
    if (0 != alignment) {
        buf         = (unsigned char*) ((((cyg_uint32)test->buffer) + alignment - 1) & ~(alignment - 1));
    } else {
        buf = test->buffer;
    }
    
    CYG_ASSERTC((usb_io_mechanism_usb == test->test_params.bulk.io_mechanism) || \
                (usb_io_mechanism_dev == test->test_params.bulk.io_mechanism));
    if (usb_io_mechanism_dev == test->test_params.bulk.io_mechanism) {
        if (((const char*)0 == usbs_testing_endpoints[ep_index].devtab_entry) ||
            (0 != cyg_io_lookup(usbs_testing_endpoints[ep_index].devtab_entry, &io_handle))) {
            
            test->result_pass   = 0;
            snprintf(test->result_message, USBTEST_MAX_MESSAGE,
                     "Target, bulk IN transfer on endpoint %d: no devtab entry", endpoint_number);
            return;
        }
    }

    // Make sure nobody else is using this endpoint
    lock_endpoint(endpoint_number, USB_ENDPOINT_DESCRIPTOR_ENDPOINT_IN);
    
    for (i = 0; i < test->test_params.bulk.number_packets; i++) {
        int packet_size = test->test_params.bulk.tx_size;
        
        test->recovery.endpoint     = endpoint_number | USB_ENDPOINT_DESCRIPTOR_ENDPOINT_IN;
        test->recovery.protocol     = USB_ENDPOINT_DESCRIPTOR_ATTR_BULK;
        test->recovery.size         = packet_size + usbs_testing_endpoints[ep_index].max_in_padding;

        // Make sure the buffer contains the data expected by the host
        usbtest_fill_buffer(&(test->test_params.bulk.data), buf, packet_size);
                            
        if (verbose < 3) {
            VERBOSE(2, "Bulk OUT test %d: iteration %d, packet size %d\n", test->id, i, packet_size);
        } else {
            // Output the first 32 bytes of data as well.
            char msg[256];
            int  index;
            int  j;
            index = snprintf(msg, 255, "Bulk IN test %d: iteration %d, packet size %d\n    Data %s:",
                             test->id, i, packet_size,
                             (usbtestdata_none == test->test_params.bulk.data.format) ? "(uninitialized)" : "");

            for (j = 0; ((j + 3) < packet_size) && (j < 32); j+= 4) {
                index += snprintf(msg+index, 255-index, " %02x%02x%02x%02x",
                                  buf[j], buf[j+1], buf[j+2], buf[j+3]);
            }
            if (j < 32) {
                index += snprintf(msg+index, 255-index, " ");
                for ( ; j < packet_size; j++) {
                    index += snprintf(msg+index, 255-index, "%02x", buf[j]);
                }
                
            }
            VERBOSE(3, "%s\n", msg);
        }
        
        // Do the actual transfer, using the I/O mechanism specified for this test.
        switch (test->test_params.bulk.io_mechanism)
        {
          case usb_io_mechanism_usb :
          {
              test->transferred = 0;
              usbs_start_tx_buffer(endpoint, buf, packet_size, &run_test_bulk_in_out_callback, (void*) test);
              cyg_semaphore_wait(&(test->sem));
              transferred = test->transferred;
              break;
          }

          case usb_io_mechanism_dev :
          {
              int result;
              transferred   = packet_size;
              result = cyg_io_write(io_handle, (void*) buf, &transferred);
              if (result < 0) {
                  transferred = result;
              }
              break;
          }

          default:
            CYG_FAIL("Invalid test mechanism specified");
            break;
        }

        // Has this test been aborted for some reason?
        if (current_tests_terminated) {
            VERBOSE(2, "Bulk IN test %d: iteration %d, termination detected\n", test->id, i);
            test->result_pass   = 0;
            snprintf(test->result_message, USBTEST_MAX_MESSAGE,
                     "Target, bulk IN transfer on endpoint %d : terminated on iteration %d, packet_size %d\n",
                     endpoint_number, i, packet_size);
            break;
        }

        // If an error occurred, abort this run
        if (transferred < 0) {
            test->result_pass   = 0;
            snprintf(test->result_message, USBTEST_MAX_MESSAGE,
                     "Target, bulk IN transfer on endpoint %d: transfer failed with %d", endpoint_number, transferred);
            VERBOSE(2, "Bulk IN test %d: iteration %d, error:\n    %s\n", test->id, i, test->result_message);
            break;
        }

        // No need to check the transfer size, the USB code is only
        // allowed to send the exact amount of data requested.

        if (0 != test->test_params.bulk.tx_delay) {
            VERBOSE(2, "Bulk IN test %d: iteration %d, sleeping for %d nanoseconds\n", test->id, i, \
                    test->test_params.bulk.tx_delay);
            usbs_nanosleep(test->test_params.bulk.tx_delay);
        }
        
        // Move on to the next transfer
        USBTEST_BULK_NEXT(test->test_params.bulk);
    }

    // Always unlock the endpoint on completion
    unlock_endpoint(endpoint_number, USB_ENDPOINT_DESCRIPTOR_ENDPOINT_IN);

    // If all the packets have been transferred this test has passed.
    if (i >= test->test_params.bulk.number_packets) {
        test->result_pass   = 1;
    }
    
    VERBOSE(1, "Test %d bulk IN on endpoint %d, result %d\n", test->id, endpoint_number, test->result_pass);
}

/*}}}*/

/*}}}*/
/*{{{  Control IN transfers                                     */

// Control-IN transfers. These have to be handled a little bit differently
// from bulk transfers. The target never actually initiates anything. Instead
// the host will send reserved control messages which are handled at DSR
// level and passed to handle_reserved_control_messages() below. Assuming
// a control-IN test is in progress, that will take appropriate action. The
// thread will be woken up only once all packets have been transferred, or
// on abnormal termination.

// Is a control-IN test currently in progress?
static UsbTest* control_in_test    = 0;

// What is the expected packet size?
static int      control_in_test_packet_size = 0;

// How many packets have been transferred so far?
static int      control_in_packets_transferred  = 0;

// Cancel a control-in test. handle_test_control_in() will have updated the static
// control_in_test so that handle_reserved_control_messages() knows what to do.
// If the test is not actually going to be run then system consistency demands
// that this update be undone. Also, the endpoint will have been locked to
// detect concurrent tests on the control endpoint.
static void
cancel_test_control_in(UsbTest* test)
{
    CYG_ASSERTC(test == control_in_test);
    control_in_test = (UsbTest*) 0;
    control_in_test_packet_size = 0;
    control_in_packets_transferred = 0;
    unlock_endpoint(0, USB_ENDPOINT_DESCRIPTOR_ENDPOINT_IN);
    test->cancel_fn = (void (*)(UsbTest*)) 0;
}

// Prepare for a control-IN transfer test.
static usbs_control_return
handle_test_control_in(usb_devreq* req)
{
    UsbTest*    test;
    int         index   = 0;

    CYG_ASSERTC((UsbTest*)0 == control_in_test);
                
    test = pool_allocate();
    unpack_usbtest_control_in(&(test->test_params.control_in), class_request, &index);

    lock_endpoint(0, USB_ENDPOINT_DESCRIPTOR_ENDPOINT_IN);
    test->which_test            = usbtest_control_in;
    test->recovery.endpoint     = 0;
    test->recovery.protocol     = USB_ENDPOINT_DESCRIPTOR_ATTR_CONTROL;
    test->recovery.size         = 0;    // Does not actually matter
    test->cancel_fn             = &cancel_test_control_in;

    // Assume a pass. Failures are easy to detect.
    test->result_pass   = 1;
    
    control_in_test = test;
    control_in_test_packet_size = test->test_params.control_in.packet_size_initial;
    control_in_packets_transferred  = 0;

    return USBS_CONTROL_RETURN_HANDLED;
}
    
// The thread for a control-in test. Actually all the hard work is done at DSR
// level, so this thread serves simply to detect when the test has completed
// and to perform some clean-ups.
static void
run_test_control_in(UsbTest* test)
{
    CYG_ASSERTC(test == control_in_test);
    
    cyg_semaphore_wait(&(test->sem));

    // The DSR has detected that the test is complete.
    control_in_test = (UsbTest*) 0;
    control_in_test_packet_size = 0;
    control_in_packets_transferred = 0;
    test->cancel_fn = (void (*)(UsbTest*)) 0;
    unlock_endpoint(0, USB_ENDPOINT_DESCRIPTOR_ENDPOINT_IN);
}

// ----------------------------------------------------------------------------
// This is installed from inside main() as the handler for reserved
// control messages.
static usbs_control_return
handle_reserved_control_messages(usbs_control_endpoint* endpoint, void* data)
{
    usb_devreq*         req = (usb_devreq*) endpoint->control_buffer;
    usbs_control_return result;

    CYG_ASSERT(endpoint == control_endpoint, "control endpoint mismatch");
    switch(req->request) {
      case USBTEST_RESERVED_CONTROL_IN:
        {
            unsigned char*  buf;
            int             len;
            
            if ((UsbTest*)0 == control_in_test) {
                result = USBS_CONTROL_RETURN_STALL;
                break;
            }

            // Is this test over? If so indicate a failure because we
            // cannot have received all the control packets.
            if (current_tests_terminated) {
                control_in_test->result_pass   = 0;
                snprintf(control_in_test->result_message, USBTEST_MAX_MESSAGE,
                         "Target, control IN transfer: not all packets received.");
                cyg_semaphore_post(&(control_in_test->sem));
                control_in_test = (UsbTest*) 0;
                result = USBS_CONTROL_RETURN_STALL;
                break;
            }
            
            // A control-IN test is indeed in progress, and the current state is
            // held in control_in_test and control_in_test_packet_size. Check that
            // the packet size matches up, i.e. that host and target are in sync.
            len = (req->length_hi << 8) || req->length_lo;
            if (control_in_test_packet_size != len) {
                control_in_test->result_pass   = 0;
                snprintf(control_in_test->result_message, USBTEST_MAX_MESSAGE,
                         "Target, control IN transfer on endpoint %d : the host only requested %d bytes instead of %d",
                         len, control_in_test_packet_size);
                cyg_semaphore_post(&(control_in_test->sem));
                control_in_test = (UsbTest*) 0;
                result = USBS_CONTROL_RETURN_STALL;
                break;
            }

            // Prepare a suitable reply buffer. This is happening at
            // DSR level so runtime is important, but with an upper
            // bound of 255 bytes the buffer should be small enough.
            buf = control_in_test->buffer;
            usbtest_fill_buffer(&(control_in_test->test_params.control_in.data), buf, control_in_test_packet_size);
            control_endpoint->buffer_size   = control_in_test_packet_size;
            control_endpoint->buffer        = buf;
            USBTEST_CONTROL_NEXT_PACKET_SIZE(control_in_test_packet_size, control_in_test->test_params.control_in);

            // Have all the packets been transferred?
            control_in_packets_transferred++;
            if (control_in_packets_transferred == control_in_test->test_params.control_in.number_packets) {
                cyg_semaphore_post(&(control_in_test->sem));
                control_in_test = (UsbTest*) 0;
            }
            result = USBS_CONTROL_RETURN_HANDLED;
            break;
      }
      default:
        CYG_FAIL("Unexpected reserved control message");
        break;
    }
    
    return result;
}

/*}}}*/

// FIXME: add more tests.

// This utility is invoked from a thread in the thread pool whenever there is
// work to be done. It simply dispatches to the appropriate handler.
static void
run_test(UsbTest* test)
{
    switch(test->which_test)
    {
      case usbtest_bulk_out :       run_test_bulk_out(test); break;
      case usbtest_bulk_in :        run_test_bulk_in(test); break;
      case usbtest_control_in:      run_test_control_in(test); break;
      default:
        CYG_TEST_FAIL_EXIT("Internal error, attempt to run unknown test.\n");
        break;
    }
}

/*}}}*/
/*{{{  The thread pool                                          */

// ----------------------------------------------------------------------------
// Just like on the host side, it is desirable to have a pool of
// threads available to perform test operations. Strictly speaking
// some tests will run without needing a separate thread, since many
// operations can be performed at DSR level. However typical
// application code will involve threads and it is desirable for test
// code to behave the same way. Also, some operations like validating
// the transferred data are expensive, and best done in thread context.

typedef struct PoolEntry {
    cyg_sem_t           wakeup;
    cyg_thread          thread_data;
    cyg_handle_t        thread_handle;
    char                thread_name[16];
    char                thread_stack[2 * CYGNUM_HAL_STACK_SIZE_TYPICAL];
    cyg_bool            in_use;
    cyg_bool            running;
    UsbTest             test;
} PoolEntry;

// This array must be uninitialized, or the executable size would
// be ludicrous.
PoolEntry  pool[USBTEST_MAX_CONCURRENT_TESTS];

// The entry point for every thread in the pool. It just loops forever,
// waiting until it is supposed to run a test.
static void
pool_thread_function(cyg_addrword_t arg)
{
    PoolEntry*  pool_entry  = (PoolEntry*) arg;

    for ( ; ; ) {
        cyg_semaphore_wait(&(pool_entry->wakeup));
        run_test(&(pool_entry->test));
        pool_entry->running = 0;
    }
}

// Initialize all threads in the pool.
static void
pool_initialize(void)
{
    int i;
    for (i = 0; i < USBTEST_MAX_CONCURRENT_TESTS; i++) {
        cyg_semaphore_init(&(pool[i].wakeup), 0);
        pool[i].in_use  = 0;
        pool[i].running = 0;
        sprintf(pool[i].thread_name, "worker%d", i);
        cyg_thread_create( 0, &pool_thread_function, (cyg_addrword_t) &(pool[i]),
                           pool[i].thread_name, pool[i].thread_stack, 2 * CYGNUM_HAL_STACK_SIZE_TYPICAL,
                           &(pool[i].thread_handle), &(pool[i].thread_data));
        cyg_thread_resume(pool[i].thread_handle);
    }
}

// Allocate a single entry in the thread pool
static UsbTest*
pool_allocate(void)
{
    UsbTest*    result  = (UsbTest*) 0;

    if (thread_counter == USBTEST_MAX_CONCURRENT_TESTS) {
        CYG_TEST_FAIL_EXIT("Internal error, thread resources exhaused.\n");
    }
    
    result = &(pool[thread_counter].test);
    thread_counter++;
    reset_usbtest(result);
    return result;
}

// Start all the threads that are supposed to be running tests.
static void
pool_start(void)
{
    int i;
    for (i = 0; i < thread_counter; i++) {
        pool[i].running = 1;
        cyg_semaphore_post(&(pool[i].wakeup));
    }
}

/*}}}*/
/*{{{  Class control messages                                   */

// ----------------------------------------------------------------------------
// Handle class control messages. These provide the primary form of
// communication between host and target. There are requests to find out
// the number of endpoints, details of each endpoint, prepare a test run,
// abort a test run, get status, terminate the target-side, and so on.
// The handlers for starting specific test cases are kept alongside
// the test cases themselves.
//
// Note that these handlers will typically be invoked from DSR context
// and hence they are subject to the usual DSR restrictions.
//
// Problems have been experienced in some hosts sending control messages
// that involve additional host->target data. An ugly workaround is
// in place whereby any such data is sent in advance using separate
// control messages.

/*{{{  endpoint count                                           */

// How many endpoints are supported by this device? That information is
// determined during initialization.
static usbs_control_return
handle_endpoint_count(usb_devreq* req)
{
    CYG_ASSERTC((1 == req->length_lo) && (0 == req->length_hi) && \
                ((req->type & USB_DEVREQ_DIRECTION_MASK) == USB_DEVREQ_DIRECTION_IN));
    CYG_ASSERTC((0 == req->index_lo) && (0 == req->index_hi) && (0 == req->value_lo) && (0 == req->value_hi));
    
    class_reply[0]                  = (unsigned char) number_endpoints;
    control_endpoint->buffer        = class_reply;
    control_endpoint->buffer_size   = 1;
    return USBS_CONTROL_RETURN_HANDLED;
}

/*}}}*/
/*{{{  endpoint details                                         */

// The host wants to know the details of a specific USB endpoint.
// The format is specified in protocol.h
static usbs_control_return
handle_endpoint_details(usb_devreq* req)
{
    int buf_index;

    CYG_ASSERTC((req->type & USB_DEVREQ_DIRECTION_MASK) == USB_DEVREQ_DIRECTION_IN);
    CYG_ASSERTC((USBTEST_MAX_CONTROL_DATA == req->length_lo) && (0 == req->length_hi));
    CYG_ASSERTC(req->index_lo < number_endpoints);
    CYG_ASSERTC((0 == req->index_hi) && (0 == req->value_lo) && (0 == req->value_hi));
    
    class_reply[0]  = (unsigned char) usbs_testing_endpoints[req->index_lo].endpoint_type;
    class_reply[1]  = (unsigned char) usbs_testing_endpoints[req->index_lo].endpoint_number;
    class_reply[2]  = (unsigned char) usbs_testing_endpoints[req->index_lo].endpoint_direction;
    class_reply[3]  = (unsigned char) usbs_testing_endpoints[req->index_lo].max_in_padding;
    buf_index = 4;
    pack_int(usbs_testing_endpoints[req->index_lo].min_size, class_reply, &buf_index);
    pack_int(usbs_testing_endpoints[req->index_lo].max_size, class_reply, &buf_index);
    if (NULL == usbs_testing_endpoints[req->index_lo].devtab_entry) {
        class_reply[buf_index]    = '\0';
        control_endpoint->buffer_size   = buf_index + 1;
    } else {
        int len = strlen(usbs_testing_endpoints[req->index_lo].devtab_entry) + buf_index + 1;
        if (len > USBTEST_MAX_CONTROL_DATA) {
            return USBS_CONTROL_RETURN_STALL;
        } else {
            strcpy(&(class_reply[buf_index]), usbs_testing_endpoints[req->index_lo].devtab_entry);
            control_endpoint->buffer_size   = len;
        }
    }
    control_endpoint->buffer        = class_reply;
    return USBS_CONTROL_RETURN_HANDLED;
}

/*}}}*/
/*{{{  sync                                                     */

// The host wants to know whether or not the target is currently busy doing
// stuff. This information is held in a static.
static usbs_control_return
handle_sync(usb_devreq* req)
{
    CYG_ASSERTC((1 == req->length_lo) && (0 == req->length_hi) && \
                ((req->type & USB_DEVREQ_DIRECTION_MASK) == USB_DEVREQ_DIRECTION_IN));
    CYG_ASSERTC((0 == req->index_lo) && (0 == req->index_hi) && (0 == req->value_lo) && (0 == req->value_hi));
    CYG_ASSERT(0 == class_request_size, "A sync operation should not involve any data");
    
    class_reply[0]                  = (unsigned char) idle;
    control_endpoint->buffer        = class_reply;
    control_endpoint->buffer_size   = 1;
    return USBS_CONTROL_RETURN_HANDLED;
}

/*}}}*/
/*{{{  pass/fail                                                */

// Allow the host to generate some pass or fail messages, and
// optionally terminate the test. These are synchronous requests
// so the data can be left in class_request.

static int passfail_request   = 0;

// Invoked from thread context
static void
handle_passfail_action(void)
{
    switch (passfail_request) {
      case USBTEST_PASS:
        CYG_TEST_PASS(class_request);
        break;
      case USBTEST_PASS_EXIT:
        CYG_TEST_PASS(class_request);
        CYG_TEST_EXIT("Exiting normally as requested by the host");
        break;
      case USBTEST_FAIL:
        CYG_TEST_FAIL(class_request);
        break;
      case USBTEST_FAIL_EXIT:
        CYG_TEST_FAIL(class_request);
        CYG_TEST_EXIT("Exiting normally as requested by the host");
        break;
      default:
        CYG_FAIL("Bogus invocation of usbtest_main_passfail");
        break;
    }
}

// Invoked from DSR context
static usbs_control_return
handle_passfail(usb_devreq* req)
{
    CYG_ASSERTC((0 == req->length_lo) && (0 == req->length_hi));
    CYG_ASSERTC((0 == req->index_lo) && (0 == req->index_hi) && (0 == req->value_lo) && (0 == req->value_hi));
    CYG_ASSERT(class_request_size > 0, "A pass/fail message should be supplied");
    CYG_ASSERT(idle, "Pass/fail messages are only allowed when idle");
    CYG_ASSERT((void (*)(void))0 == main_thread_action, "No thread operation should be pending.");
    
    passfail_request    = req->request;
    idle                = false;
    main_thread_action  = &handle_passfail_action;
    cyg_semaphore_post(&main_wakeup);

    return USBS_CONTROL_RETURN_HANDLED;
}

/*}}}*/
/*{{{  abort                                                    */

// The host has concluded that there is no easy way to get both target and
// host back to a sensible state. For example there may be a thread that
// is blocked waiting for some I/O that is not going to complete. The abort
// should be handled at thread level, not DSR level, so that the host
// still sees the low-level USB handshake.

static void
handle_abort_action(void)
{
    CYG_TEST_FAIL_EXIT("Test abort requested by host application");
}

static usbs_control_return
handle_abort(usb_devreq* req)
{
    CYG_ASSERTC((0 == req->length_lo) && (0 == req->length_hi));
    CYG_ASSERTC((0 == req->index_lo) && (0 == req->index_hi) && (0 == req->value_lo) && (0 == req->value_hi));
    CYG_ASSERT(idle, "Abort messages are only allowed when idle");
    CYG_ASSERT((void (*)(void))0 == main_thread_action, "No thread operation should be pending.");
    
    idle                = false;
    main_thread_action  = &handle_abort_action;
    cyg_semaphore_post(&main_wakeup);
    
    return USBS_CONTROL_RETURN_HANDLED;
}

/*}}}*/
/*{{{  cancel                                                   */

// Invoked from thread context
// Cancelling pending test cases simply involves iterating over the allocated
// entries in the pool, invoking any cancellation functions that have been
// defined, and then resetting the tread count. The actual tests have not
// yet started so none of the threads will be active.
static void
handle_cancel_action(void)
{
    int i;
    for (i = 0; i < thread_counter; i++) {
        if ((void (*)(UsbTest*))0 != pool[i].test.cancel_fn) {
            (*(pool[i].test.cancel_fn))(&(pool[i].test));
            pool[i].test.cancel_fn  = (void (*)(UsbTest*)) 0;
        }
    }
    thread_counter    = 0;
}

// Invoked from DSR context
static usbs_control_return
handle_cancel(usb_devreq* req)
{
    CYG_ASSERTC((0 == req->length_lo) && (0 == req->length_hi));
    CYG_ASSERTC((0 == req->index_lo) && (0 == req->index_hi) && (0 == req->value_lo) && (0 == req->value_hi));
    CYG_ASSERT(0 == class_request_size, "A cancel operation should not involve any data");
    CYG_ASSERT(idle, "Cancel requests are only allowed when idle");
    CYG_ASSERT(!running, "Cancel requests cannot be sent once the system is running");
    CYG_ASSERT((void (*)(void))0 == main_thread_action, "No thread operation should be pending.");
    
    idle                = false;
    main_thread_action = &handle_cancel_action;
    cyg_semaphore_post(&main_wakeup);

    return USBS_CONTROL_RETURN_HANDLED;
}

/*}}}*/
/*{{{  start                                                    */

// Start the tests running. This just involves waking up the pool threads
// and setting the running flag, with the latter serving primarily for
// assertions. 

static usbs_control_return
handle_start(usb_devreq* req)
{
    CYG_ASSERTC((0 == req->length_lo) && (0 == req->length_hi));
    CYG_ASSERTC((0 == req->index_lo) && (0 == req->index_hi) && (0 == req->value_lo) && (0 == req->value_hi));
    CYG_ASSERT(0 == class_request_size, "A start operation should not involve any data");
    CYG_ASSERT(!running, "Start requests cannot be sent if the system is already running");

    current_tests_terminated = false;
    running = true;
    pool_start();
    
    return USBS_CONTROL_RETURN_HANDLED;
}

/*}}}*/
/*{{{  finished                                                 */

// Have all the tests finished? This involves checking all the threads
// involved in the current batch of tests and seeing whether or not
// their running flag is still set.

static usbs_control_return
handle_finished(usb_devreq* req)
{
    int i;
    int result = 1;
    
    CYG_ASSERTC((1 == req->length_lo) && (0 == req->length_hi) && \
                ((req->type & USB_DEVREQ_DIRECTION_MASK) == USB_DEVREQ_DIRECTION_IN));
    CYG_ASSERTC((0 == req->index_lo) && (0 == req->index_hi) && (0 == req->value_lo) && (0 == req->value_hi));
    CYG_ASSERT(0 == class_request_size, "A finished operation should not involve any data");
    CYG_ASSERT(running, "Finished requests can only be sent if the system is already running");
    
    for (i = 0; i < thread_counter; i++) {
        if (pool[i].running) {
            result = 0;
            break;
        }
    }
    class_reply[0]                  = (unsigned char) result;
    control_endpoint->buffer        = class_reply;
    control_endpoint->buffer_size   = 1;
    return USBS_CONTROL_RETURN_HANDLED;
}

/*}}}*/
/*{{{  set terminated                                           */

// A timeout has occurred, or there is some other failure. The first step
// in recovery is to set the terminated flag so that as recovery action
// takes place and the threads wake up they make no attempt to continue
// doing more transfers.

static usbs_control_return
handle_set_terminated(usb_devreq* req)
{
    CYG_ASSERTC((0 == req->length_lo) && (0 == req->length_hi));
    CYG_ASSERTC((0 == req->index_lo) && (0 == req->index_hi) && (0 == req->value_lo) && (0 == req->value_hi));
    CYG_ASSERT(0 == class_request_size, "A set-terminated operation should not involve any data");
    CYG_ASSERT(running, "The terminated flag can only be set when there are running tests");

    current_tests_terminated = 1;
    
    return USBS_CONTROL_RETURN_HANDLED;
}

/*}}}*/
/*{{{  get recovery                                             */

// Return the recovery information for one of the threads involved in the
// current batch of tests, so that the host can perform a USB operation
// that will sort out that thread.
static usbs_control_return
handle_get_recovery(usb_devreq* req)
{
    int buffer_index;
    
    CYG_ASSERT(current_tests_terminated, "Recovery should only be attempted when the terminated flag is set");
    CYG_ASSERT(running, "If there are no tests running then recovery is impossible");
    CYG_ASSERTC((12 == req->length_lo) && (0 == req->length_hi) && \
                ((req->type & USB_DEVREQ_DIRECTION_MASK) == USB_DEVREQ_DIRECTION_IN));
    CYG_ASSERTC(req->index_lo <= thread_counter);
    CYG_ASSERTC((0 == req->index_hi) && (0 == req->value_lo) && (0 == req->value_hi));
    CYG_ASSERT(0 == class_request_size, "A get-recovery operation should not involve any data");

    control_endpoint->buffer        = class_reply;
    if (!pool[req->index_lo].running) {
        // Actually, this particular thread has terminated so no recovery is needed.
        control_endpoint->buffer_size   = 0;
    } else {
        buffer_index    = 0;
        pack_usbtest_recovery(&(pool[req->index_lo].test.recovery), class_reply, &buffer_index);
        control_endpoint->buffer_size   = buffer_index;
    }
    
    return USBS_CONTROL_RETURN_HANDLED;
}

/*}}}*/
/*{{{  perform recovery                                         */

// The host has identified a course of action that could unlock a thread
// on the host-side that is currently blocked performing a USB operation.
// Typically this involves either sending or accepting some data. If the
// endpoint is still locked, in other words if there is a still a local
// thread attempting to communicate on the specified endpoint, then
// things are messed up: both sides are trying to communicate, but nothing
// is happening. The eCos USB API is such that attempting multiple
// concurrent operations on a single endpoint is disallowed, so
// the recovery request has to be ignored. If things do not sort themselves
// out then the whole test run will have to be aborted.

// A dummy completion function for when a recovery operation has completed.
static void
recovery_callback(void* callback_arg, int transferred)
{
    CYG_UNUSED_PARAM(void*, callback_arg);
    CYG_UNUSED_PARAM(int, transferred);
}
    
static usbs_control_return
handle_perform_recovery(usb_devreq* req)
{
    int                 buffer_index;
    int                 endpoint_number;
    int                 endpoint_direction;
    UsbTest_Recovery    recovery;
    
    CYG_ASSERT(current_tests_terminated, "Recovery should only be attempted when the terminated flag is set");
    CYG_ASSERT(running, "If there are no tests running then recovery is impossible");
    CYG_ASSERTC((0 == req->length_lo) && (0 == req->length_hi));
    CYG_ASSERTC((0 == req->index_lo) && (0 == req->index_hi) && (0 == req->value_lo) && (0 == req->value_hi));
    CYG_ASSERT(12 == class_request_size, "A perform-recovery operation requires recovery data");

    buffer_index = 0;
    unpack_usbtest_recovery(&recovery, class_request, &buffer_index);
    endpoint_number     = recovery.endpoint & ~USB_DEVREQ_DIRECTION_MASK;
    endpoint_direction  = recovery.endpoint & USB_DEVREQ_DIRECTION_MASK;

    if (!is_endpoint_locked(endpoint_number, endpoint_direction)) {
        // Locking the endpoint here would be good, but the endpoint would then
        // have to be unlocked again - probably in the recovery callback.
        // This complication is ignored for now.

        if (USB_ENDPOINT_DESCRIPTOR_ATTR_BULK == recovery.protocol) {
            int ep_index = lookup_endpoint(endpoint_number, endpoint_direction, USB_ENDPOINT_DESCRIPTOR_ATTR_BULK);
            CYG_ASSERTC(-1 != ep_index);

            if (USB_DEVREQ_DIRECTION_IN == endpoint_direction) {
                // The host wants some data. Supply it. A single byte will do fine to
                // complete the transfer.
                usbs_start_tx_buffer((usbs_tx_endpoint*) usbs_testing_endpoints[ep_index].endpoint,
                                     recovery_buffer, 1, &recovery_callback, (void*) 0);
            } else {
                // The host is trying to send some data. Accept all of it.
                usbs_start_rx_buffer((usbs_rx_endpoint*) usbs_testing_endpoints[ep_index].endpoint,
                                     recovery_buffer, recovery.size, &recovery_callback, (void*) 0);
            }
        }

        // No support for isochronous or interrupt transfers yet.
        // handle_reserved_control_messages() should generate stalls which
        // have the desired effect.
    }
    
    return USBS_CONTROL_RETURN_HANDLED;
}

/*}}}*/
/*{{{  get result                                               */

// Return the result of one the tests. This can be a single byte for
// a pass, or a single byte plus a message for a failure.

static usbs_control_return
handle_get_result(usb_devreq* req)
{
    CYG_ASSERTC((USBTEST_MAX_CONTROL_DATA == req->length_lo) && (0 == req->length_hi) && \
                ((req->type & USB_DEVREQ_DIRECTION_MASK) == USB_DEVREQ_DIRECTION_IN));
    CYG_ASSERTC(req->index_lo <= thread_counter);
    CYG_ASSERTC((0 == req->index_hi) && (0 == req->value_lo) && (0 == req->value_hi));
    CYG_ASSERT(0 == class_request_size, "A get-result operation should not involve any data");
    CYG_ASSERT(running, "Results can only be sent if a run is in progress");
    CYG_ASSERT(!pool[req->index_lo].running, "Cannot request results for a test that has not completed");

    class_reply[0]  = pool[req->index_lo].test.result_pass;
    if (class_reply[0]) {
        control_endpoint->buffer_size = 1;
    } else {
        strncpy(&(class_reply[1]), pool[req->index_lo].test.result_message, USBTEST_MAX_CONTROL_DATA - 2);
        class_reply[USBTEST_MAX_CONTROL_DATA - 1] = '\0';
        control_endpoint->buffer_size = 1 + strlen(&(class_reply[1])) + 1;
    }
    control_endpoint->buffer = class_reply;
    return USBS_CONTROL_RETURN_HANDLED;
}

/*}}}*/
/*{{{  batch done                                               */

// A batch of test has been completed - at least, the host thinks so.
// If the host is correct then all that is required here is to reset
// the thread pool and clear the global running flag - that is sufficient
// to allow a new batch of tests to be started.

static usbs_control_return
handle_batch_done(usb_devreq* req)
{
    int i;
    
    CYG_ASSERTC((0 == req->length_lo) && (0 == req->length_hi));
    CYG_ASSERTC((0 == req->index_lo) && (0 == req->index_hi) && (0 == req->value_lo) && (0 == req->value_hi));
    CYG_ASSERT(0 == class_request_size, "A batch-done operation should not involve any data");
    CYG_ASSERT(running, "There must be a current batch of tests");

    for (i = 0; i < thread_counter; i++) {
        CYG_ASSERTC(!pool[i].running);
    }
    thread_counter  = 0;
    running         = false;
    
    return USBS_CONTROL_RETURN_HANDLED;

}

/*}}}*/
/*{{{  verbosity                                                */

static usbs_control_return
handle_verbose(usb_devreq* req)
{
    CYG_ASSERTC((0 == req->length_lo) && (0 == req->length_hi));
    CYG_ASSERTC((0 == req->index_lo) && (0 == req->index_hi));
    CYG_ASSERT(0 == class_request_size, "A set-verbosity operation should not involve any data");

    verbose = (req->value_hi << 8) + req->value_lo;
    
    return USBS_CONTROL_RETURN_HANDLED;
}

/*}}}*/
/*{{{  initialise bulk out endpoint                             */

// ----------------------------------------------------------------------------
// Accept an initial endpoint on a bulk endpoint. This avoids problems
// on some hardware such as the SA11x0 which can start to accept data
// before the software is ready for it.

static void handle_init_callback(void* arg, int result)
{
    idle = true;
}

static usbs_control_return
handle_init_bulk_out(usb_devreq* req)
{
    static char         buf[64];
    int                 ep_index;
    usbs_rx_endpoint*   endpoint;
    
    CYG_ASSERTC((0 == req->length_lo) && (0 == req->length_hi));
    CYG_ASSERTC((0 == req->index_lo) && (0 == req->index_hi));
    CYG_ASSERTC((0 == req->value_hi) && (0 < req->value_lo) && (req->value_lo < 16));
    CYG_ASSERT(0 == class_request_size, "An init_bulk_out operation should not involve any data");

    ep_index = lookup_endpoint(req->value_lo, USB_ENDPOINT_DESCRIPTOR_ENDPOINT_OUT, USB_ENDPOINT_DESCRIPTOR_ATTR_BULK);
    CYG_ASSERTC(-1 != ep_index);
    endpoint = (usbs_rx_endpoint*) usbs_testing_endpoints[ep_index].endpoint;
    
    idle = false;
    usbs_start_rx_buffer(endpoint, buf, 64, &handle_init_callback, (void*) 0);
    
    return USBS_CONTROL_RETURN_HANDLED;
}

/*}}}*/
/*{{{  additional control data                                  */

// Accumulate some more data in the control buffer, ahead of an upcoming
// request.
static usbs_control_return
handle_control_data(usb_devreq* req)
{
    class_request[class_request_size + 0] = req->value_hi;
    class_request[class_request_size + 1] = req->value_lo;
    class_request[class_request_size + 2] = req->index_hi;
    class_request[class_request_size + 3] = req->index_lo;

    switch(req->request) {
      case USBTEST_CONTROL_DATA1 : class_request_size += 1; break;
      case USBTEST_CONTROL_DATA2 : class_request_size += 2; break;
      case USBTEST_CONTROL_DATA3 : class_request_size += 3; break;
      case USBTEST_CONTROL_DATA4 : class_request_size += 4; break;
    }

    return USBS_CONTROL_RETURN_HANDLED;
}

/*}}}*/

typedef struct class_handler {
    int     request;
    usbs_control_return (*handler)(usb_devreq*);
} class_handler;
static class_handler class_handlers[] = {
    { USBTEST_ENDPOINT_COUNT,   &handle_endpoint_count },
    { USBTEST_ENDPOINT_DETAILS, &handle_endpoint_details },
    { USBTEST_PASS,             &handle_passfail },
    { USBTEST_PASS_EXIT,        &handle_passfail },
    { USBTEST_FAIL,             &handle_passfail },
    { USBTEST_FAIL_EXIT,        &handle_passfail },
    { USBTEST_SYNCH,            &handle_sync },
    { USBTEST_ABORT,            &handle_abort },
    { USBTEST_CANCEL,           &handle_cancel },
    { USBTEST_START,            &handle_start },
    { USBTEST_FINISHED,         &handle_finished },
    { USBTEST_SET_TERMINATED,   &handle_set_terminated },
    { USBTEST_GET_RECOVERY,     &handle_get_recovery },
    { USBTEST_PERFORM_RECOVERY, &handle_perform_recovery },
    { USBTEST_GET_RESULT,       &handle_get_result },
    { USBTEST_BATCH_DONE,       &handle_batch_done },
    { USBTEST_VERBOSE,          &handle_verbose },
    { USBTEST_INIT_BULK_OUT,    &handle_init_bulk_out },
    { USBTEST_TEST_BULK,        &handle_test_bulk },
    { USBTEST_TEST_CONTROL_IN,  &handle_test_control_in },
    { USBTEST_CONTROL_DATA1,    &handle_control_data },
    { USBTEST_CONTROL_DATA2,    &handle_control_data },
    { USBTEST_CONTROL_DATA3,    &handle_control_data },
    { USBTEST_CONTROL_DATA4,    &handle_control_data },
    { -1,                       (usbs_control_return (*)(usb_devreq*)) 0 }
};

static usbs_control_return
handle_class_control_messages(usbs_control_endpoint* endpoint, void* data)
{
    usb_devreq*         req = (usb_devreq*) endpoint->control_buffer;
    int                 request = req->request;
    usbs_control_return result;
    int                 i;

    VERBOSE(3, "Received control message %02x\n", request);
    
    CYG_ASSERT(endpoint == control_endpoint, "control endpoint mismatch");
    result  = USBS_CONTROL_RETURN_UNKNOWN;
    for (i = 0; (usbs_control_return (*)(usb_devreq*))0 != class_handlers[i].handler; i++) {
        if (request == class_handlers[i].request) {
            result = (*(class_handlers[i].handler))(req);
            if ((USBTEST_CONTROL_DATA1 != request) &&
                (USBTEST_CONTROL_DATA2 != request) &&
                (USBTEST_CONTROL_DATA3 != request) &&
                (USBTEST_CONTROL_DATA4 != request)) {
                // Reset the request data buffer after all normal requests.
                class_request_size = 0;
            }
            break;
        }
    }
    CYG_UNUSED_PARAM(void*, data);
    if (USBS_CONTROL_RETURN_HANDLED != result) {
        VERBOSE(1, "Control message %02x not handled\n", request);
    }
    
    return result;
}

/*}}}*/
/*{{{  main()                                                   */

// ----------------------------------------------------------------------------
// Initialization.
int
main(int argc, char** argv)
{
    int i;

    CYG_TEST_INIT();

    // The USB device driver should have provided an array of endpoint
    // descriptors, usbs_testing_endpoints(). One entry in this array
    // should be a control endpoint, which is needed for initialization.
    // It is also useful to know how many endpoints there are.
    for (i = 0; !USBS_TESTING_ENDPOINTS_IS_TERMINATOR(usbs_testing_endpoints[i]); i++) {
        if ((0 == usbs_testing_endpoints[i].endpoint_number) &&
            (USB_ENDPOINT_DESCRIPTOR_ATTR_CONTROL== usbs_testing_endpoints[i].endpoint_type)) {
            CYG_ASSERT((usbs_control_endpoint*)0 == control_endpoint, "There should be only one control endpoint");
            control_endpoint = (usbs_control_endpoint*) usbs_testing_endpoints[i].endpoint;
        }
    }
    if ((usbs_control_endpoint*)0 == control_endpoint) {
        CYG_TEST_FAIL_EXIT("Unable to find a USB control endpoint");
    }
    number_endpoints = i;
    CYG_ASSERT(number_endpoints <= USBTEST_MAX_ENDPOINTS, "impossible number of endpoints");

    // Some of the information provided may not match the actual capabilities
    // of the testing code, e.g. max_size limits.
    fix_driver_endpoint_data();
    
    // This semaphore is used for communication between the DSRs that process control
    // messages and the main thread
    cyg_semaphore_init(&main_wakeup, 0);

    // Take care of the pool of threads and related data.
    pool_initialize();

    // Start the heartbeat thread, to make sure that the gdb session stays
    // alive.
    start_heartbeat();
    
    // Now it is possible to start up the USB device driver. The host can detect
    // this, connect, get the enumeration data, and then testing will proceed
    // in response to class control messages.
    provide_endpoint_enumeration_data();
    control_endpoint->enumeration_data      = &usb_enum_data;
    control_endpoint->class_control_fn      = &handle_class_control_messages;
    control_endpoint->reserved_control_fn   = &handle_reserved_control_messages;
    usbs_start(control_endpoint);

    // Now it is over to the host to detect this target and start performing tests.
    // Much of this is handled at DSR level, in response to USB control messages.
    // Some of those control messages require action at thread level, and that is
    // achieved by signalling a semaphore and waking up this thread. A static
    // function pointer is used to keep track of what operation is actually required.
    for (;;) {
        void (*handler)(void);
        
        cyg_semaphore_wait(&main_wakeup);
        handler = main_thread_action;
        main_thread_action   = 0;
        CYG_CHECK_FUNC_PTR(handler, "Main thread woken up when there is nothing to be done");
        (*handler)();
        idle = true;
    }
}

/*}}}*/
