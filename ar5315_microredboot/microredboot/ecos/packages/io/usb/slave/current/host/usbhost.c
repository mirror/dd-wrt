/*{{{  Banner                                                   */

//=================================================================
//
//        host.c
//
//        USB testing - host-side
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
// Author(s):     bartv
// Date:          2001-07-04
//####DESCRIPTIONEND####
//==========================================================================

// The overall architecture is as follows.
//
// The target hardware runs a special application which provides a
// particular type of USB application, "Red Hat eCos USB testing".
// This will not be recognised by any device driver, so the Linux
// kernel will pretty much ignore the device (other host OS's are not
// considered at this time).
//
// This program is the only supported way to interact with that service.
// It acts as an extended Tcl interpreter, providing a number of new
// Tcl commands for interacting with the target. All test cases can
// then be written as Tcl scripts which invoke a series of these commands.
// These Tcl commands operate essentially though the LINUX usb devfs
// service which allows ordinary application code to perform USB operations
// via ioctl()'s.

/*}}}*/
/*{{{  #include's                                               */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
// Avoid compatibility problems with Tcl 8.4 vs. earlier
#define USE_NON_CONST
#include <tcl.h>
#include <linux/usb.h>
#include <linux/usbdevice_fs.h>
#include "../tests/protocol.h"

/*}}}*/

/*{{{  Statics                                                  */

// ----------------------------------------------------------------------------
// Statics.

// Has the current batch of tests actually terminated? This flag is
// checked by the various test handlers at appropriate intervals, and
// helps to handle the case where one of the side has terminated early
// because an error has been detected.
static int          current_tests_terminated = 0;

// The next local thread to be allocated for testing. This variable can also
// be used to find out how many threads are involved in the current test.
// This counter should always be reset to 0 at the end of every test run.
static int          local_thread_count      = 0;

// A similar counter for remote threads.
static int          remote_thread_count     = 0;

// A file handle for manipulating the USB device at a low level
static int          usb_master_fd  = -1;

/*}}}*/
/*{{{  Logging                                                  */

// ----------------------------------------------------------------------------
// The user can provide one or more -V/--verbose arguments to increase
// the amount of output generated.

static int verbose = 0;

#define VERBOSE(_level_, _format_, _args_...)   \
    do {                                        \
        if (verbose >= _level_) {               \
            printf(_format_, ## _args_);        \
        }                                       \
    } while (0);

/*}}}*/
/*{{{  Low-level USB access                                     */

// ----------------------------------------------------------------------------
// Low-level access to a USB device.
//
// The various ioctl() calls require a file handle which corresponds to one
// of the /proc/bus/usb/<abc>/<def> entries. <abc> is a bus number,
// typically 001 or 001, and <def> is a device number on that bus,
// e.g. 003. Figuring out <abc> and <def> requires scanning
// /proc/bus/usb/devices, which is a somewhat complicated text file.
//
// This is all somewhat vulnerable to incompatible changes in the
// Linux kernel, specifically the implementation of the /proc/bus/usb.
// An alternative approach would be to write a new Linux device driver
// and interact with that, but that approach is vulnerable to any
// internal kernel API changes affecting USB device drivers.

// How to access USB devices from userland
#define USB_ROOT        "/proc/bus/usb/"

// How to identify the eCos test case
#define PRODUCT_STRING    "Red Hat eCos USB test"

// Scan through /proc/bus/usb/devices looking for an entry that
// matches what we are after, specifically a line
//   S:  Product=Red Hat eCos USB testcase
// The required information can then be obtained from the previous
// line:
//   T:  Bus=<abc> ... Dev#= <def> ...
//
// Of course the T: line is going to come first, so it is necessary
// to keep track of the current bus and device numbers.
//
// Note: this code is duplicated in usbchmod.c. Any changes here
// should be propagated. For now the routine is too small to warrant
// a separate source file.

static int
usb_scan_devices(int* bus, int* dev)
{
    FILE*       devs_file;
    int         current_bus     = -1;
    int         current_dev     = -1;
    int         ch;

    *bus = -1;
    *dev = -1;

    VERBOSE(1, "Searching " USB_ROOT "devices for the eCos USB test code\n");
    
    devs_file = fopen(USB_ROOT "devices", "r");
    if (NULL == devs_file) {
        fprintf(stderr, "usbhost: error, unable to access " USB_ROOT "devices\n");
        return 0;
    }
    ch = getc(devs_file);
    while (EOF != ch) {
        if ('T' == ch) {
            if (2 !=fscanf(devs_file, ":  Bus=%d %*[^D\n]Dev#=%d", &current_bus, &current_dev)) { 
                current_bus = -1;
                current_dev = -1;
            }
        } else if ('S' == ch) {
            int start = 0, end = 0;
            if (EOF != fscanf(devs_file, ":  Product=%n" PRODUCT_STRING "%n", &start, &end)) {
                if (start < end) {
                    *bus = current_bus;
                    *dev = current_dev;
                    break;
                }
            } 
        }
        // Move to the end of the current line.
        do {
            ch = getc(devs_file);
        } while ((EOF != ch) && ('\n' != ch));
        if (EOF != ch) {
            ch = getc(devs_file);
        }
    }
    
    fclose(devs_file);
    if ((-1 != *bus) && (-1 != *dev)) {
        VERBOSE(1, "Found eCos USB test code on bus %d, device %d\n", *bus, *dev);
        return 1;
    }
    fprintf(stderr, "usbhost: error, failed to find a USB device \"" PRODUCT_STRING "\"\n");
    return 0;
}

// Actually open the USB device, allowing subsequent ioctl() operations.
//
// Typically /proc/bus/usb/... will not allow ordinary applications
// to perform ioctl()'s. Instead root privileges are required. To work
// around this there is a little utility usbchmod, installed suid,
// which can be used to get access to the raw device.
static int
usb_open_device(void)
{
    char devname[_POSIX_PATH_MAX];
    static int  bus = -1;
    static int  dev = -1;
    int         result;
    
    if ((-1 == bus) || (-1 == dev)) {
        if (!usb_scan_devices(&bus, &dev)) {
            return -1;
        }
    }
    
    if (_POSIX_PATH_MAX == snprintf(devname, _POSIX_PATH_MAX, USB_ROOT "%03d/%03d", bus, dev)) {
        fprintf(stderr, "usbhost: internal error, buffer overflow\n");
        exit(EXIT_FAILURE);
    }

    VERBOSE(1, "Attempting to access USB target via %s\n", devname);
    
    result = open(devname, O_RDWR);
    if (-1 == result) {
        // Check for access right problems. If so, try to work around them
        // by invoking usbchmod. Always look for this in the install tree,
        // since it is only that version which is likely to have been
        // chown'ed and chmod'ed to be suid root.
        if (EACCES == errno) {
            char command_name[_POSIX_PATH_MAX];

            VERBOSE(1, "Insufficient access to USB target, running usbchmod\n");
            if (_POSIX_PATH_MAX == snprintf(command_name, _POSIX_PATH_MAX, "%s/usbchmod %d %d", USBAUXDIR, bus, dev)) {
                fprintf(stderr, "usbhost: internal error, buffer overflow\n");
                exit(EXIT_FAILURE);
            }
            (void) system(command_name);
            result = open(devname, O_RDWR);
        }
    }
    if (-1 == result) {
        fprintf(stderr, "usbhost: error, failed to open \"%s\", errno %d\n", devname, errno);
    }

    VERBOSE(1, "USB device now accessible via file descriptor %d\n", result);
    
    // Also perform a set-configuration call, to avoid warnings from
    // the Linux kernel. Target-side testing is always configuration 1
    // because only a single configuration is supported.
    (void) ioctl(result, USBDEVFS_SETCONFIGURATION, 1);
    return result;
}

// Exchange a control message with the host. The return value should
// be 0, or a small positive number indicating the actual number of
// bytes received which may be less than requested.
//
// There appear to be problems with some hosts, manifesting itself as
// an inability to send control messages that involve additional data
// from host->target. These problems are not yet well-understood. For
// now the workaround is to send multiple packets, each with up to
// four bytes encoded in the index and length fields.
static int
usb_control_message(int fd, int request_type, int request, int value, int index, int length, void* data)
{
    struct usbdevfs_ctrltransfer        transfer;
    int         result = 0;

    VERBOSE(3, "usb_control_message, request %02x, len %d\n", request, length);
    
    if (length > USBTEST_MAX_CONTROL_DATA) {
        fprintf(stderr, "usbhost: internal error, control message involves too much data.\n");
        exit(EXIT_FAILURE);
    }

#if 1
    // Workaround - send additional data in the index and length fields.
    if ((length > 0) && (USB_DIR_OUT == (USB_ENDPOINT_DIR_MASK & request_type))) {
        int i;
        unsigned char*  buf = (unsigned char*) data;
        
        for (i = 0; i < length; i+= 4) {
            int this_len = length - 1;
            int ioctl_result;
            
            transfer.requesttype    = USB_TYPE_CLASS | USB_RECIP_DEVICE;
            if (this_len > 4) {
                this_len = 4;
            }
            switch (this_len) {
              case 1: transfer.request  = USBTEST_CONTROL_DATA1; break;
              case 2: transfer.request  = USBTEST_CONTROL_DATA2; break;
              case 3: transfer.request  = USBTEST_CONTROL_DATA3; break;
              case 4: transfer.request  = USBTEST_CONTROL_DATA4; break;
              default:
                fprintf(stderr, "usbhost: internal error, confusion about transfer length.\n");
                exit(EXIT_FAILURE);
            }
            transfer.value      = (buf[i]   << 8) | buf[i+1];   // Possible read beyond end of buffer,
            transfer.index      = (buf[i+2] << 8) | buf[i+3];   // but not worth worrying about.
            transfer.length     = 0;
            transfer.timeout    = 10 * 1000; // ten seconds, the target should always accept data faster than this.
            transfer.data       = NULL;

            // This is too strict, deciding what to do about errors should be
            // handled by higher-level code. However it will do for now.
            ioctl_result = ioctl(fd, USBDEVFS_CONTROL, &transfer);
            if (0 != ioctl_result) {
                fprintf(stderr, "usbhost: error, failed to send control message (data) to target.\n");
                exit(EXIT_FAILURE);
            }
        }
        // There is no more data to be transferred.
        length = 0;
    }
#endif    
    transfer.requesttype        = request_type;
    transfer.request            = request;
    transfer.value              = value;
    transfer.index              = index;
    transfer.length             = length;
    transfer.timeout            = 10000;
    transfer.data               = data;

    result = ioctl(fd, USBDEVFS_CONTROL, &transfer);
    return result;
}

// A variant of the above which can be called when the target should always respond
// correctly. This can be used for class control messages.
static int
usb_reliable_control_message(int fd, int request_type, int request, int value, int index, int length, void* data)
{
    int result = usb_control_message(fd, request_type, request, value, index, length, data);
    if (-1 == result) {
        fprintf(stderr, "usbhost: error, failed to send control message %02x to target.\n", request);
        fprintf(stderr, "       : errno %d (%s)\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return result;
}

    
// Either send or receive a single bulk message. The top bit of the endpoint
// number indicates the direction.
static int
usb_bulk_message(int fd, int endpoint, unsigned char* buffer, int length)
{
    struct  usbdevfs_bulktransfer    transfer;
    int     result;
    
    transfer.ep         = endpoint;
    transfer.len        = length;
    transfer.timeout    = 60 * 60 * 1000;
    // An hour. These operations should not time out because that
    // leaves the system in a confused state. Instead there is
    // higher-level recovery code that should ensure the operation
    // really does complete, and the return value here is used
    // by the calling code to determine whether the operation
    // was successful or whether there was an error and the recovery
    // code was invoked.
    transfer.data       = buffer;
    errno               = 0;
    result = ioctl(fd, USBDEVFS_BULK, &transfer);
    return result;
}


// Synchronise with the target. This can be used after the host has sent a request that
// may take a bit of time, e.g. it may involve waking up a thread. The host will send
// synch requests at regular intervals, until the target is ready.
//
// The limit argument can be used to avoid locking up. -1 means loop forever, otherwise
// it means that many iterations of 100ms apiece.
static int
usb_sync(int fd, int limit)
{
    unsigned char   buf[1];
    struct timespec delay;
    int             loops   = 0;
    int             result  = 0;

    VERBOSE(2, "Synchronizing with target\n");
    
    while (1) {
        buf[0]  = 0;
        usb_reliable_control_message(fd, USB_TYPE_CLASS | USB_RECIP_DEVICE | USB_DIR_IN, USBTEST_SYNCH, 0, 0, 1, buf);
        if (buf[0]) {
            result = 1;
            break;
        } else {
            if ((-1 != limit) && (++loops > limit)) {
                break;
            } else {
                VERBOSE(3, "Not yet synchronized, sleeping\n");
                delay.tv_sec    = 0;
                delay.tv_nsec   = 100000000;    // 100 ms
                nanosleep(&delay, NULL);
            }
        }
    }
    VERBOSE(2, "%s\n", result ? "Synchronized" : "Not synchronized");
    return result;
}

// Abort the target. Things seem to be completely messed up and there is no easy
// way to restore sanity to both target and host.
static void
usb_abort(int fd)
{
    VERBOSE(2, "Target-side abort operation invoked\n");
    usb_reliable_control_message(fd, USB_TYPE_CLASS | USB_RECIP_DEVICE, USBTEST_ABORT, 0, 0, 0, (void*)0);
}

/*}}}*/
/*{{{  Initialise endpoints                                     */

// ----------------------------------------------------------------------------
// On power-up some endpoints may not be in a sensible state. For example,
// with the SA11x0 the hardware may start accepting bulk OUT transfers
// before the target-side software has started a receive operation,
// so if the host sends a bulk packet before the target is ready then
// things get messy. This is especially troublesome if the target-side
// attempts any diagnostic output because of verbosity.
//
// This code loops through the various endpoints and makes sure that
// they are all in a reasonable state, before any real tests get run
// That means known hardware flaws do not show up as test failures,
// but of course they are still documented and application software
// will have to do the right thing.

static void
usb_initialise_control_endpoint(int min_size, int max_size)
{
    // At this time there are no known problems on any hardware
    // that would need to be addressed
}

static void
usb_initialise_isochronous_in_endpoint(int number, int min_size, int max_size)
{
    // At this time there are no known problems on any hardware
    // that would need to be addressed
}

static void
usb_initialise_isochronous_out_endpoint(int number, int min_size, int max_size)
{
    // At this time there are no known problems on any hardware
    // that would need to be addressed
}

static void
usb_initialise_bulk_in_endpoint(int number, int min_size, int max_size, int padding)
{
    // At this time there are no known problems on any hardware
    // that would need to be addressed
}

static void
usb_initialise_bulk_out_endpoint(int number, int min_size, int max_size)
{
    char buf[1];
    
    // On the SA1110 the hardware comes up with a bogus default value,
    // causing the hardware to accept packets before the software has
    // set up DMA or in any way prepared for incoming data. This is
    // a problem. It is worked around by making the target receive
    // a single packet, sending that packet, and then performing a
    // sync.
    VERBOSE(2, "Performing bulk OUT initialization on endpoint %d\n", number);
    
    usb_reliable_control_message(usb_master_fd, USB_TYPE_CLASS | USB_RECIP_DEVICE | USB_DIR_IN,
                                 USBTEST_INIT_BULK_OUT, number, 0, 0, (void*) 0);
    usb_bulk_message(usb_master_fd, number, buf, 1);
    usb_sync(usb_master_fd, 10);
}

static void
usb_initialise_interrupt_in_endpoint(int number, int min_size, int max_size)
{
    // At this time there are no known problems on any hardware
    // that would need to be addressed
}

static void
usb_initialise_interrupt_out_endpoint(int number, int min_size, int max_size)
{
    // At this time there are no known problems on any hardware
    // that would need to be addressed
}

/*}}}*/
/*{{{  Host/target common code                                  */

#define HOST
#include "../tests/common.c"

/*}}}*/
/*{{{  The test cases themselves                                */

/*{{{  UsbTest definition                                       */

// ----------------------------------------------------------------------------
// All the data associated with a single test.

typedef struct UsbTest {

    // A "unique" identifier to make verbose output easier to understand.
    int                 id;          
    // Which file descriptor should be used to access USB.
    int                 fd;
    
    // Which test should be run.
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

    int                 result_pass;
    char                result_message[USBTEST_MAX_MESSAGE];
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
}

/*}}}*/
/*{{{  bulk OUT                                                 */

static void
run_test_bulk_out(UsbTest* test)
{
    unsigned char*  buf     = test->buffer;
    int             i;

    VERBOSE(1, "Starting test %d, bulk OUT on endpoint %d\n", test->id, test->test_params.bulk.endpoint);
    
    for (i = 0; i < test->test_params.bulk.number_packets; i++) {
        int transferred;
        int packet_size = test->test_params.bulk.tx_size;

        test->recovery.endpoint     = test->test_params.bulk.endpoint;
        test->recovery.protocol     = USB_ENDPOINT_XFER_BULK;
        test->recovery.size         = packet_size;
        
        usbtest_fill_buffer(&(test->test_params.bulk.data), buf, packet_size);
        if (verbose < 3) {
            VERBOSE(2, "Bulk OUT test %d: iteration %d, packet size %d\n", test->id, i, packet_size);
        } else {
            // Output the first 32 bytes of data as well.
            char msg[256];
            int  index;
            int  j;
            index = snprintf(msg, 255, "Bulk OUT test %d: iteration %d, packet size %d\n    Data %s:",
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

        transferred = usb_bulk_message(test->fd, test->test_params.bulk.endpoint, buf, packet_size);
        
        // Has this test run been aborted for some reason?
        if (current_tests_terminated) {
            VERBOSE(2, "Bulk OUT test %d: iteration %d, termination detected\n", test->id, i);
            test->result_pass = 0;
            snprintf(test->result_message, USBTEST_MAX_MESSAGE,
                     "Host, bulk OUT transfer on endpoint %d: aborted after %d iterations\n",
                     test->test_params.bulk.endpoint & USB_ENDPOINT_NUMBER_MASK, i);
            return;
        }

        // If an error occurred, abort this run.
        if (-1 == transferred) {
            char    errno_buf[USBTEST_MAX_MESSAGE];
            test->result_pass  = 0;
            snprintf(test->result_message, USBTEST_MAX_MESSAGE,
                     "Host, bulk OUT transfer on endpoint %d : host ioctl() system call failed\n    errno %d (%s)",
                     test->test_params.bulk.endpoint & USB_ENDPOINT_NUMBER_MASK, errno,
                     strerror_r(errno, errno_buf, USBTEST_MAX_MESSAGE));
            VERBOSE(2, "Bulk OUT test %d: iteration %d, error:\n    %s\n", test->id, i, test->result_message);
            break;
        }

        if (0 != test->test_params.bulk.tx_delay) {
            struct timespec delay;
            
            VERBOSE(2, "Bulk OUT test %d: iteration %d, sleeping for %d nanoseconds\n", test->id, \
                    i, test->test_params.bulk.tx_delay);
            // Note that nanosleep() can return early due to incoming signals,
            // with the unelapsed time returned in a second argument. This
            // allows for a retry loop. In practice this does not seem
            // worthwhile, the delays are approximate anyway.
            delay.tv_sec  = test->test_params.bulk.tx_delay / 1000000000;
            delay.tv_nsec = test->test_params.bulk.tx_delay % 1000000000;
            nanosleep(&delay, NULL);
        }

        // Now move on to the next transfer
        USBTEST_BULK_NEXT(test->test_params.bulk);
    }

    // If all the packets have been transferred this test has passed.
    if (i >= test->test_params.bulk.number_packets) {
        test->result_pass   = 1;
    }
    
    VERBOSE(1, "Test %d bulk OUT on endpoint %d, result %d\n", test->id, test->test_params.bulk.endpoint, test->result_pass);
}

/*}}}*/
/*{{{  bulk IN                                                  */

static void
run_test_bulk_in(UsbTest* test)
{
    unsigned char*  buf     = test->buffer;
    int             i;

    VERBOSE(1, "Starting test %d bulk IN on endpoint %d\n", test->id, test->test_params.bulk.endpoint);
    
    for (i = 0; i < test->test_params.bulk.number_packets; i++) {
        int transferred;
        int tx_size             = test->test_params.bulk.tx_size;
        int rx_size             = test->test_params.bulk.rx_size;
        int size_plus_padding;
        
        VERBOSE(2, "Bulk IN test %d: iteration %d, rx size %d, tx size %d\n", test->id, i, rx_size, tx_size);
        
        if (rx_size < tx_size) {
            rx_size = tx_size;
            VERBOSE(2, "Bulk IN test %d: iteration %d, packet size reset to %d to match tx size\n",
                    test->id, i, rx_size);
        }
        test->recovery.endpoint     = test->test_params.bulk.endpoint;
        test->recovery.protocol     = USB_ENDPOINT_XFER_BULK;
        test->recovery.size         = rx_size;

        // Make sure there is no old data lying around
        if (usbtestdata_none != test->test_params.bulk.data.format) {
            memset(buf, 0, rx_size);
        }

        // And do the actual transfer.
        size_plus_padding = rx_size;
        if (size_plus_padding < (tx_size + test->test_params.bulk.rx_padding)) {
            size_plus_padding += test->test_params.bulk.rx_padding;
        }
        do {
            transferred = usb_bulk_message(test->fd, test->test_params.bulk.endpoint, buf, size_plus_padding);
        } while (0 == transferred);
        
        // Has this test run been aborted for some reason?
        if (current_tests_terminated) {
            VERBOSE(2, "Bulk IN test %d: iteration %d, termination detected\n", test->id, i);
            snprintf(test->result_message, USBTEST_MAX_MESSAGE,
                     "Host, bulk IN transfer on endpoint %d: aborted after %d iterations\n",
                     test->test_params.bulk.endpoint & USB_ENDPOINT_NUMBER_MASK, i);
            return;
        }

        // If an error occurred, abort this run.
        if (-1 == transferred) {
            char    errno_buf[USBTEST_MAX_MESSAGE];
            test->result_pass  = 0;
            snprintf(test->result_message, USBTEST_MAX_MESSAGE,
                     "Host, bulk IN transfer on endpoint %d : host ioctl() system call failed\n    errno %d (%s)",
                     test->test_params.bulk.endpoint & USB_ENDPOINT_NUMBER_MASK, errno,
                     strerror_r(errno, errno_buf, USBTEST_MAX_MESSAGE));
            VERBOSE(2, "Bulk IN test %d: iteration %d, error:\n    %s\n", test->id, i, test->result_message);
            break;
        }

        // Did the target send the expected amount of data?
        if (transferred < tx_size) {
            test->result_pass   = 0;
            snprintf(test->result_message, USBTEST_MAX_MESSAGE,
                     "Host, bulk IN transfer on endpoint %d : the target only sent %d bytes when %d were expected",
                     test->test_params.bulk.endpoint & USB_ENDPOINT_NUMBER_MASK, transferred, tx_size);
            VERBOSE(2, "Bulk IN test %d: iteration %d, error:\n    %s\n", test->id, i, test->result_message);
            break;
        }
        
        if (verbose >= 3) {
            // Output the first 32 bytes of data
            char msg[256];
            int  index;
            int  j;
            index = snprintf(msg, 255, "Bulk IN test %d: iteration %d, transferred %d\n    Data %s:",
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
        if (!usbtest_check_buffer(&(test->test_params.bulk.data), buf, tx_size)) {
            test->result_pass   = 0;
            snprintf(test->result_message, USBTEST_MAX_MESSAGE,
                     "Host, bulk IN transfer on endpoint %d : mismatch between received and expected data",
                     test->test_params.bulk.endpoint & USB_ENDPOINT_NUMBER_MASK);
            VERBOSE(2, "Bulk IN test %d: iteration %d, error:\n    %s\n", test->id, i, test->result_message);
            break;
        }
        
        if (0 != test->test_params.bulk.rx_delay) {
            struct timespec delay;
            
            VERBOSE(2, "Bulk IN test %d: iteration %d, sleeping for %d nanoseconds\n", test->id, \
                    i, test->test_params.bulk.tx_delay);
            // Note that nanosleep() can return early due to incoming signals,
            // with the unelapsed time returned in a second argument. This
            // allows for a retry loop. In practice this does not seem
            // worthwhile, the delays are approximate anyway.
            delay.tv_sec  = test->test_params.bulk.rx_delay / 1000000000;
            delay.tv_nsec = test->test_params.bulk.rx_delay % 1000000000;
            nanosleep(&delay, NULL);
        }
        
        USBTEST_BULK_NEXT(test->test_params.bulk);
    }

    
    // If all the packets have been transferred this test has passed.
    if (i >= test->test_params.bulk.number_packets) {
        test->result_pass   = 1;
    }

    VERBOSE(1, "Test %d bulk IN on endpoint %d, result %d\n", test->id, test->test_params.bulk.endpoint, test->result_pass);
}

/*}}}*/
/*{{{  control IN                                               */

// Receive appropriate packets via the control endpoint. This is somewhat
// different from bulk transfers. It is implemented using reserved control
// messages.
//
// Note: it is not entirely clear that this test is safe. There will be
// concurrent control traffic to detect test termination and the like,
// and these control messages may interfere with each other. It is not
// entirely clear how the Linux kernel handles concurrent control
// operations.

static void
run_test_control_in(UsbTest* test)
{
    unsigned char*  buf     = test->buffer;
    int             packet_size;
    int             i;

    packet_size = test->test_params.control_in.packet_size_initial;
    for (i = 0; i < test->test_params.control_in.number_packets; i++) {
        int transferred;
        
        test->recovery.endpoint     = 0;
        test->recovery.protocol     = USB_ENDPOINT_XFER_CONTROL;
        test->recovery.size         = packet_size;

        // Make sure there is no old data lying around
        if (usbtestdata_none != test->test_params.control_in.data.format) {
            memset(buf, 0, packet_size);
        }

        // And do the actual transfer.
        transferred = usb_control_message(test->fd, USB_TYPE_RESERVED | USB_RECIP_DEVICE | USB_DIR_IN, USBTEST_RESERVED_CONTROL_IN,
                                          0, 0, packet_size, buf);

        // Has this test run been aborted for some reason?
        if (current_tests_terminated) {
            return;
        }

        // If an error occurred, abort this run.
        if (-1 == transferred) {
            char    errno_buf[USBTEST_MAX_MESSAGE];
            test->result_pass  = 0;
            snprintf(test->result_message, USBTEST_MAX_MESSAGE,
                     "Host, control IN transfer: host ioctl() system call failed\n    errno %d (%s)",
                     errno, strerror_r(errno, errno_buf, USBTEST_MAX_MESSAGE));
            break;
        }

        // Did the target send the expected amount of data?
        if (transferred < packet_size) {
            test->result_pass   = 0;
            snprintf(test->result_message, USBTEST_MAX_MESSAGE,
                     "Host, control IN transfer: the target only sent %d bytes when %d were expected",
                     transferred, packet_size);
            break;
        }
        
        // Is the data correct?
        if (!usbtest_check_buffer(&(test->test_params.control_in.data), buf, packet_size)) {
            test->result_pass   = 0;
            snprintf(test->result_message, USBTEST_MAX_MESSAGE,
                     "Host, control IN transfer: mismatch between received and expected data");
            break;
        }
        
        USBTEST_CONTROL_NEXT_PACKET_SIZE(packet_size, test->test_params.control_in);
    }

    // If all the packets have been transferred this test has passed.
    if (i >= test->test_params.control_in.number_packets) {
        test->result_pass   = 1;
    }
}

/*}}}*/

// FIXME: add more tests

/*{{{  run_test()                                               */

// This utility is invoked from a thread in the thread pool whenever there is
// work to be done. It simply dispatches to the appropriate handler.
static void
run_test(UsbTest* test)
{
    switch (test->which_test) {
      case usbtest_bulk_out:    run_test_bulk_out(test); break;
      case usbtest_bulk_in:     run_test_bulk_in(test); break;
      case usbtest_control_in:  run_test_control_in(test); break;
      default:
        fprintf(stderr, "usbhost: internal error, attempt to execute an unknown test.\n");
        exit(EXIT_FAILURE);
    }
}

/*}}}*/

/*}}}*/
/*{{{  The thread pool                                          */

// ----------------------------------------------------------------------------
// A pool of threads and buffers which do the real work. The number of possible
// concurrent tests is defined in protocol.h. Each one requires a separate
// thread, transfer buffer, semaphore, and some state information.
//
// Although the application is multi-threaded, in practice there is little
// need for synchronization. Tests will only be started while the pool threads
// are idle. When the pool threads are running the main thread will be waiting
// for them all to finish, with a bit of polling to detect error conditions.
// The pool threads do not share any data, apart from the file descriptor for
// the USB device.

typedef struct PoolEntry {
    pthread_t       thread;
    sem_t           wakeup;
    int             running;
    UsbTest         test;
} PoolEntry;

static PoolEntry pool[USBTEST_MAX_CONCURRENT_TESTS];

// This is the entry point for every thread in the pool. It just loops forever,
// waiting until it is supposed to run a test. These threads never actually
// exit, instead there should be a call to exit() somewhere.
static void*
pool_function(void* arg)
{
    PoolEntry*  pool_entry  = (PoolEntry*) arg;
    for ( ; ; ) {
        sem_wait(&(pool_entry->wakeup));
        run_test(&(pool_entry->test));
        pool_entry->running = 0;
    }

    return NULL;
}

// Initialize all threads in the pool.
static void
pool_initialize(void)
{
    int i;
    for (i = 0; i < USBTEST_MAX_CONCURRENT_TESTS; i++) {
        pool[i].running = 0;
        pool[i].test.fd = dup(usb_master_fd);
        if (0 != sem_init(&(pool[i].wakeup), 0, 0)) {
            fprintf(stderr, "usbhost: internal error, failed to initialize all semaphores.\n");
            exit(EXIT_FAILURE);
        }
        if (0 != pthread_create(&(pool[i].thread), NULL, &pool_function, (void*) &(pool[i]))) {
            fprintf(stderr, "usbhost: internal error, failed to start all threads.\n");
            exit(EXIT_FAILURE);
        }
    }
}

// Allocate a single entry in the thread pool.
static UsbTest*
pool_allocate(void)
{
    UsbTest* result = (UsbTest*) 0;

    if (local_thread_count == USBTEST_MAX_CONCURRENT_TESTS) {
        fprintf(stderr, "usbhost: internal error, thread resource exhausted.\n");
        exit(EXIT_FAILURE);
    }

    result = &(pool[local_thread_count].test);
    local_thread_count++;
    reset_usbtest(result);
    return result;
}

// Start all the threads that are supposed to be running tests.
static void
pool_start(void)
{
    int i;
    for (i = 0; i < local_thread_count; i++) {
        pool[i].running = 1;
        sem_post(&(pool[i].wakeup));
    }
}

/*}}}*/
/*{{{  Tcl routines                                             */

// ----------------------------------------------------------------------------
// Tcl routines to provide access to the USB device from inside Tcl
// scripts, plus some general utilities. These routines deal mostly
// with preparing a test run. The actual work is done in C: the
// ioctl() operations are not readily accessible from Tcl, and
// operations like filling in buffers and calculating checksums are
// cpu-intensive.

/*{{{  pass/fail/abort                                          */

// ----------------------------------------------------------------------------
// Some simple routines accessible from Tcl to get the target to report pass/fail or
// to make the target abort.

static int
tcl_target_pass(ClientData     clientData  __attribute__ ((unused)),
                Tcl_Interp*    interp,
                int            argc,
                char**         argv)
{
    if (2 != argc) {
        Tcl_SetResult(interp, "wrong # args: should be \"usbtest::target_pass <message>\"", TCL_STATIC);
        return TCL_ERROR;
    }
    usb_reliable_control_message(usb_master_fd, USB_TYPE_CLASS | USB_RECIP_DEVICE, USBTEST_PASS, 0, 0, strlen(argv[1]) + 1, argv[1]);
    usb_sync(usb_master_fd, -1);
    return TCL_OK;
}

static int
tcl_target_fail(ClientData     clientData  __attribute__ ((unused)),
                Tcl_Interp*    interp,
                int            argc,
                char**         argv)
{
    if (2 != argc) {
        Tcl_SetResult(interp, "wrong # args: should be \"usbtest::target_fail <message>\"", TCL_STATIC);
        return TCL_ERROR;
    }
    usb_reliable_control_message(usb_master_fd, USB_TYPE_CLASS | USB_RECIP_DEVICE, USBTEST_FAIL, 0, 0, strlen(argv[1]) + 1, argv[1]);
    usb_sync(usb_master_fd, -1);
    return TCL_OK;
}

// The next three routines cause the target to exit, so a usb_sync() is inappropriate.
static int
tcl_target_pass_exit(ClientData     clientData  __attribute__ ((unused)),
                     Tcl_Interp*    interp,
                     int            argc,
                     char**         argv)
{
    if (2 != argc) {
        Tcl_SetResult(interp, "wrong # args: should be \"usbtest::target_pass_exit <message>\"", TCL_STATIC);
        return TCL_ERROR;
    }
    usb_reliable_control_message(usb_master_fd, USB_TYPE_CLASS | USB_RECIP_DEVICE, USBTEST_PASS_EXIT, 0, 0,
                                 strlen(argv[1]) + 1, argv[1]);
    return TCL_OK;
}


static int
tcl_target_fail_exit(ClientData     clientData  __attribute__ ((unused)),
                     Tcl_Interp*    interp,
                     int            argc,
                     char**         argv)
{
    if (2 != argc) {
        Tcl_SetResult(interp, "wrong # args: should be \"usbtest::target_fail_exit <message>\"", TCL_STATIC);
        return TCL_ERROR;
    }
    usb_reliable_control_message(usb_master_fd, USB_TYPE_CLASS | USB_RECIP_DEVICE, USBTEST_FAIL_EXIT, 0, 0,
                                 strlen(argv[1]) + 1, argv[1]);
    return TCL_OK;
}

static int
tcl_target_abort(ClientData     clientData  __attribute__ ((unused)),
                 Tcl_Interp*    interp,
                 int            argc,
                 char**         argv        __attribute__ ((unused)) )
{
    if (1 != argc) {
        Tcl_SetResult(interp, "wrong # args: should be \"usbtest::target_abort\"", TCL_STATIC);
        return TCL_ERROR;
    }
    usb_abort(usb_master_fd);
    return TCL_OK;
}

/*}}}*/
/*{{{  start bulk test                                          */

// ----------------------------------------------------------------------------
// Start a bulk test. The real Tcl interface to this functionality is
// implemented in Tcl: it takes care of figuring out sensible default
// arguments, validating the data, etc. All that this code does is
// allocate a thread and fill in the appropriate data, plus request
// the target-side to do the same thing.

static int
tcl_test_bulk(ClientData     clientData  __attribute__ ((unused)),
              Tcl_Interp*    interp,
              int            argc,
              char**         argv)
{
    int             i;
    int             tmp;
    UsbTest*        test;
    unsigned char   request[USBTEST_MAX_CONTROL_DATA];
    int             request_index;

    // The data consists of 28 numbers for UsbTest_Bulk itself, and
    // another 10 numbers for the test data definition.
    if (39 != argc) {
        Tcl_SetResult(interp, "wrong # args: should be \"usbtest::_test_bulk <message>\"", TCL_STATIC);
        return TCL_ERROR;
    }
    for (i = 1; i < 39; i++) {
        int discard;
        if (TCL_OK != Tcl_GetInt(interp, argv[i], &discard)) {
            Tcl_SetResult(interp, "invalid argument: all arguments should be numbers", TCL_STATIC);
            return TCL_ERROR;
        }
    }

    test = pool_allocate();
    Tcl_GetInt(interp, argv[1], &(test->test_params.bulk.number_packets));
    Tcl_GetInt(interp, argv[2], &(test->test_params.bulk.endpoint));
    test->which_test = (USB_DIR_IN == (test->test_params.bulk.endpoint & USB_ENDPOINT_DIR_MASK))
        ? usbtest_bulk_in : usbtest_bulk_out;
    Tcl_GetInt(interp, argv[ 3], &(test->test_params.bulk.tx_size));
    Tcl_GetInt(interp, argv[ 4], &(test->test_params.bulk.tx_size_min));
    Tcl_GetInt(interp, argv[ 5], &(test->test_params.bulk.tx_size_max));
    Tcl_GetInt(interp, argv[ 6], &(test->test_params.bulk.tx_size_multiplier));
    Tcl_GetInt(interp, argv[ 7], &(test->test_params.bulk.tx_size_divisor));
    Tcl_GetInt(interp, argv[ 8], &(test->test_params.bulk.tx_size_increment));
    Tcl_GetInt(interp, argv[ 9], &(test->test_params.bulk.rx_size));
    Tcl_GetInt(interp, argv[10], &(test->test_params.bulk.rx_size_min));
    Tcl_GetInt(interp, argv[11], &(test->test_params.bulk.rx_size_max));
    Tcl_GetInt(interp, argv[12], &(test->test_params.bulk.rx_size_multiplier));
    Tcl_GetInt(interp, argv[13], &(test->test_params.bulk.rx_size_divisor));
    Tcl_GetInt(interp, argv[14], &(test->test_params.bulk.rx_size_increment));
    Tcl_GetInt(interp, argv[15], &(test->test_params.bulk.rx_padding));
    Tcl_GetInt(interp, argv[16], &(test->test_params.bulk.tx_delay));
    Tcl_GetInt(interp, argv[17], &(test->test_params.bulk.tx_delay_min));
    Tcl_GetInt(interp, argv[18], &(test->test_params.bulk.tx_delay_max));
    Tcl_GetInt(interp, argv[19], &(test->test_params.bulk.tx_delay_multiplier));
    Tcl_GetInt(interp, argv[20], &(test->test_params.bulk.tx_delay_divisor));
    Tcl_GetInt(interp, argv[21], &(test->test_params.bulk.tx_delay_increment));
    Tcl_GetInt(interp, argv[22], &(test->test_params.bulk.rx_delay));
    Tcl_GetInt(interp, argv[23], &(test->test_params.bulk.rx_delay_min));
    Tcl_GetInt(interp, argv[24], &(test->test_params.bulk.rx_delay_max));
    Tcl_GetInt(interp, argv[25], &(test->test_params.bulk.rx_delay_multiplier));
    Tcl_GetInt(interp, argv[26], &(test->test_params.bulk.rx_delay_divisor));
    Tcl_GetInt(interp, argv[27], &(test->test_params.bulk.rx_delay_increment));
    Tcl_GetInt(interp, argv[28], &tmp);
    test->test_params.bulk.io_mechanism = (usb_io_mechanism) tmp;
    Tcl_GetInt(interp, argv[29], &tmp);
    test->test_params.bulk.data.format = (usbtestdata) tmp;
    Tcl_GetInt(interp, argv[30], &(test->test_params.bulk.data.seed));
    Tcl_GetInt(interp, argv[31], &(test->test_params.bulk.data.multiplier));
    Tcl_GetInt(interp, argv[32], &(test->test_params.bulk.data.increment));
    Tcl_GetInt(interp, argv[33], &(test->test_params.bulk.data.transfer_seed_multiplier));
    Tcl_GetInt(interp, argv[34], &(test->test_params.bulk.data.transfer_seed_increment));
    Tcl_GetInt(interp, argv[35], &(test->test_params.bulk.data.transfer_multiplier_multiplier));
    Tcl_GetInt(interp, argv[36], &(test->test_params.bulk.data.transfer_multiplier_increment));
    Tcl_GetInt(interp, argv[37], &(test->test_params.bulk.data.transfer_increment_multiplier));
    Tcl_GetInt(interp, argv[38], &(test->test_params.bulk.data.transfer_increment_increment));

    VERBOSE(3, "Preparing USB bulk test on endpoint %d, direction %s, for %d packets\n", \
            test->test_params.bulk.endpoint, \
            (usbtest_bulk_in == test->which_test) ? "IN" : "OUT", \
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
            
    
    // That is all the data converted from Tcl to C, and a local thread is set up to handle this
    // request. Also set up a thread on the target.
    request_index = 0;
    pack_usbtest_bulk(&(test->test_params.bulk), request, &request_index);
    usb_reliable_control_message(usb_master_fd, USB_TYPE_CLASS | USB_RECIP_DEVICE, USBTEST_TEST_BULK, 0, 0, request_index, request);
    remote_thread_count++;
    
    return TCL_OK;
}

/*}}}*/
/*{{{  start control-in test                                    */

// ----------------------------------------------------------------------------
// Start a control-in test. The real Tcl interface to this
// functionality is implemented in Tcl: it takes care of figuring out
// sensible default arguments, validating the data, etc. All that this
// code does is allocate a thread and fill in the appropriate data,
// plus request the target-side to do the same thing.

static int
tcl_test_control_in(ClientData     clientData  __attribute__ ((unused)),
                    Tcl_Interp*    interp,
                    int            argc,
                    char**         argv)
{
    int             i;
    int             tmp;
    UsbTest*        test;
    unsigned char   request[USBTEST_MAX_CONTROL_DATA];
    int             request_index;
        
    // The data consists of 6 numbers for UsbTest_ControlIn itself, and
    // another 10 numbers for the test data definition.
    if (17 != argc) {
        Tcl_SetResult(interp, "wrong # args: should be \"usbtest::_test_control_in <message>\"", TCL_STATIC);
        return TCL_ERROR;
    }
    for (i = 1; i < 17; i++) {
        int discard;
        if (TCL_OK != Tcl_GetInt(interp, argv[i], &discard)) {
            Tcl_SetResult(interp, "invalid argument: all arguments should be numbers", TCL_STATIC);
            return TCL_ERROR;
        }
    }

    test                = pool_allocate();
    test->which_test    = usbtest_control_in;
    Tcl_GetInt(interp, argv[1], &(test->test_params.control_in.number_packets));
    Tcl_GetInt(interp, argv[2], &(test->test_params.control_in.packet_size_initial));
    Tcl_GetInt(interp, argv[3], &(test->test_params.control_in.packet_size_min));
    Tcl_GetInt(interp, argv[4], &(test->test_params.control_in.packet_size_max));
    Tcl_GetInt(interp, argv[5], &(test->test_params.control_in.packet_size_multiplier));
    Tcl_GetInt(interp, argv[6], &(test->test_params.control_in.packet_size_increment));
    Tcl_GetInt(interp, argv[7], &tmp);
    test->test_params.bulk.data.format = (usbtestdata) tmp;
    Tcl_GetInt(interp, argv[ 8], &(test->test_params.control_in.data.seed));
    Tcl_GetInt(interp, argv[ 9], &(test->test_params.control_in.data.multiplier));
    Tcl_GetInt(interp, argv[10], &(test->test_params.control_in.data.increment));
    Tcl_GetInt(interp, argv[11], &(test->test_params.control_in.data.transfer_seed_multiplier));
    Tcl_GetInt(interp, argv[12], &(test->test_params.control_in.data.transfer_seed_increment));
    Tcl_GetInt(interp, argv[13], &(test->test_params.control_in.data.transfer_multiplier_multiplier));
    Tcl_GetInt(interp, argv[14], &(test->test_params.control_in.data.transfer_multiplier_increment));
    Tcl_GetInt(interp, argv[15], &(test->test_params.control_in.data.transfer_increment_multiplier));
    Tcl_GetInt(interp, argv[16], &(test->test_params.control_in.data.transfer_increment_increment));

    // That is all the data converted from Tcl to C, and a local thread is set up to handle this
    // request. Also set up a thread on the target.
    request_index = 0;
    pack_usbtest_control_in(&(test->test_params.control_in), request, &request_index);
    usb_reliable_control_message(usb_master_fd, USB_TYPE_CLASS | USB_RECIP_DEVICE, USBTEST_TEST_CONTROL_IN, 0, 0,
                                 request_index, request);
    remote_thread_count++;
    
    return TCL_OK;
}

/*}}}*/
/*{{{  Cancel the current batch of tests                        */

static int
tcl_cancel(ClientData     clientData    __attribute__ ((unused)),
           Tcl_Interp*    interp,
           int            argc,
           char**         argv          __attribute__ ((unused)) )
{
    if (1 != argc) {
        Tcl_SetResult(interp, "wrong # args: should be \"usbtest::cancel\"", TCL_STATIC);
        return TCL_ERROR;
    }

    // Send the request on to the target.
    usb_reliable_control_message(usb_master_fd, USB_TYPE_CLASS | USB_RECIP_DEVICE, USBTEST_CANCEL, 0, 0, 0, (void*)0);

    // Now cancel all the local tests. This can be done by resetting the counter
    // of allocated threads: no actual work will have been started yet.
    local_thread_count = 0;

    // And synchronise with the target
    if (!usb_sync(usb_master_fd, 30)) {
        fprintf(stderr, "usbhost: error, target has failed to process test cancel request.\n");
        exit(EXIT_FAILURE);
        
    }
    remote_thread_count     = 0;
    
    return TCL_OK;
}

/*}}}*/
/*{{{  Run a batch of tests                                     */

// ----------------------------------------------------------------------------
// This code does an awful lot of the hard work. Start with various utilities.

// Has the current batch finished as far as the local threads are concerned?
static int
local_batch_finished(void)
{
    int result = 1;
    int i;

    for (i = 0; i < local_thread_count; i++) {
        if (pool[i].running) {
            result = 0;
            break;
        }
    }
    return result;
}

// Has the current batch finished as far as remote threads are concerned?
static int
remote_batch_finished(void)
{
    char buf[1];
    usb_reliable_control_message(usb_master_fd, USB_TYPE_CLASS | USB_RECIP_DEVICE | USB_DIR_IN, USBTEST_FINISHED,
                                 0, 0, 1, (void*) buf);
    return buf[0];
}

// Perform recovery for a thread on the target. This involves asking the
// target for recovery information, then performing an appropriate
// action. If no data is returned then no recovery is needed for this thread.
static void
recover_remote(int index)
{
    unsigned char       buffer[USBTEST_MAX_CONTROL_DATA];
    int                 buffer_index;
    UsbTest_Recovery    recovery;
    int                 i;

    if (0 != usb_reliable_control_message(usb_master_fd, USB_TYPE_CLASS | USB_RECIP_DEVICE | USB_DIR_IN,
                                          USBTEST_GET_RECOVERY, 0, index, 12, buffer)) {
        // There is work to be done
        buffer_index = 0;
        unpack_usbtest_recovery(&recovery, buffer, &buffer_index);

        // We have an endpoint, a protocol, and a size.
        if (0 == recovery.endpoint) {
            // The target just needs a dummy reserved control message
            usb_reliable_control_message(usb_master_fd, USB_TYPE_RESERVED | USB_RECIP_DEVICE, USBTEST_RESERVED_CONTROL_IN,
                                         0, 0, 0, (void*) 0);
        } else if (USB_ENDPOINT_XFER_BULK == recovery.protocol) {
            // Either we need to send some data to the target, or we need to accept some data.
            static unsigned char recovery_buffer[USBTEST_MAX_BULK_DATA + USBTEST_MAX_BULK_DATA_EXTRA];
            
            struct  usbdevfs_bulktransfer    transfer;
            transfer.ep         = recovery.endpoint;
            transfer.timeout    = 2000; // Two seconds.  Should be plenty, even for a large bulk transfer.
            transfer.data       = recovery_buffer;
            if (USB_DIR_IN == (recovery.endpoint & USB_ENDPOINT_DIR_MASK)) {
                transfer.len = recovery.size;
            } else {
                transfer.len = 1;
            }
            errno = 0;
            i = ioctl(usb_master_fd, USBDEVFS_BULK, &transfer);
        }

        // There is no recovery support yet for other protocols.
    }
}

// Perform recovery for a local thread. This involves extracting the
// recovery information from the local thread and asking the target
// to take appropriate action.
static void
recover_local(int index)
{
    unsigned char   buffer[USBTEST_MAX_CONTROL_DATA];
    int             buffer_index;

    if (pool[index].running) {
        buffer_index = 0;
        pack_usbtest_recovery(&(pool[index].test.recovery), buffer, &buffer_index);
        usb_reliable_control_message(usb_master_fd, USB_TYPE_CLASS | USB_RECIP_DEVICE, USBTEST_PERFORM_RECOVERY,
                                     0, 0, buffer_index, (void*) buffer);
    }
}

// All done, time for a clean-up on both target and host. The latter
// is achieved simply by resetting the thread pool, which actually
// just means resetting the counter since all the threads are blocked
// waiting for the next batch.
static void
run_done(void)
{
    usb_reliable_control_message(usb_master_fd, USB_TYPE_CLASS | USB_RECIP_DEVICE, USBTEST_BATCH_DONE, 0, 0, 0, (void*) NULL);
    local_thread_count = 0;
    remote_thread_count = 0;
}

// The main routine, as invoked from Tcl. This takes a single
// argument, a timeout in seconds.
static int
tcl_run(ClientData     clientData    __attribute__ ((unused)),
        Tcl_Interp*    interp,
        int            argc,
        char**         argv          __attribute__ ((unused)) )
{
    struct timespec delay;
    int             timeout;
    time_t          start;
    time_t          now;
    int             i, j;
    unsigned char   result_buf[USBTEST_MAX_CONTROL_DATA];
    int             all_ok;
    
    if (2 != argc) {
        Tcl_SetResult(interp, "wrong # args: should be \"usbtest::_run <timeout>\"", TCL_STATIC);
        return TCL_ERROR;
    }
    if (TCL_OK != Tcl_GetInt(interp, argv[1], &timeout)) {
        Tcl_SetResult(interp, "invalid argument: timeout should be numeric", TCL_STATIC);
        return TCL_ERROR;
    }
    
    VERBOSE(2, "Starting a testrun, timeout %d seconds\n", timeout);
    
    // Start the tests running on the target. The target USB hardware
    // will not actually do anything except in response to packets
    // from the host, so it is better to start the target before the
    // local threads.
    usb_reliable_control_message(usb_master_fd, USB_TYPE_CLASS | USB_RECIP_DEVICE, USBTEST_START, 0, 0, 0, (void*) 0);

    // Now the local threads can get going.
    current_tests_terminated = 0;
    pool_start();

    // Now leave the various testing threads to do their thing until
    // either side believes that the batch has finished, or until the
    // timeout expires. Note that if one side decides that the batch
    // has finished but the other disagrees, that in itself indicates
    // a test failure of sorts.
    //
    // There is a question of polling frequency. Once a second avoids
    // excessive polling traffic on the USB bus, and should not impose
    // intolerable delays for short-duration tests.
    start = time(NULL);
    do {
        VERBOSE(3, "The tests are running, waiting for termination\n");
        delay.tv_sec    = 1;
        delay.tv_nsec   = 0;
        nanosleep(&delay, NULL);
        now = time(NULL);
    } while (((start + timeout) > now) && !local_batch_finished() && !remote_batch_finished());

    VERBOSE(2, "Termination detected, time elapsed %ld\n", (long) now - start);

    // If either side believes that testing is not complete, things
    // get messy. Start by setting the terminated flag. Any tests that
    // are actually still running happily but have not finished within
    // the timeout should detect this and stop.
    if (!local_batch_finished() || !remote_batch_finished()) {
        VERBOSE(2, "Testing is not yet complete, setting TERMINATED flag\n");
        current_tests_terminated    = 1;
        usb_reliable_control_message(usb_master_fd, USB_TYPE_CLASS | USB_RECIP_DEVICE, USBTEST_SET_TERMINATED, 0, 0, 0, (void*) 0);
        // And another delay, to give threads a chance to detect the
        // flag's update
        delay.tv_sec    = 1;
        delay.tv_nsec   = 0;
        nanosleep(&delay, NULL);
    }

    // If there is still are unfinished threads, recovery action
    // is needed. It is not clear whether it is better to unlock
    // the local threads first, or the remote threads. For now the
    // latter approach is taken.
    if (!remote_batch_finished()) {
        int i;
        VERBOSE(2, "Remote threads still running, performing remote recovery\n");
        for (i = 0; i < remote_thread_count; i++) {
            recover_remote(i);
        }
        // Allow the recovery actions to take effect
        delay.tv_sec    = 1;
        delay.tv_nsec   = 0;
        nanosleep(&delay, NULL);
    }

    if (!local_batch_finished()) {
        int i;
        VERBOSE(2, "Local threads still running, performing local recovery\n");
        for (i = 0; i < local_thread_count; i++) {
            recover_local(i);
        }
        // Allow the recovery actions to take effect
        delay.tv_sec    = 1;
        delay.tv_nsec   = 0;
        nanosleep(&delay, NULL);
    }

    // One last check to make sure that everything is finished. If not,
    // testing has broken down and it is necessary to abort.
    if (!local_batch_finished() || !remote_batch_finished()) {
        VERBOSE(2, "Giving local and remote threads another chance to finish.\n");
        // Allow the recovery actions to take effect
        delay.tv_sec    = 5;
        delay.tv_nsec   = 0;
        nanosleep(&delay, NULL);
        if (!local_batch_finished() || !remote_batch_finished()) {
            // OK, normality has not been restored.
            // It would be nice to get hold of and display any error messages.
            usb_abort(usb_master_fd);
            fprintf(stderr, "Fatal error: the host test program and the remote target are out of synch.\n");
            fprintf(stderr, "             recovery has been attempted, without success.\n");
            fprintf(stderr, "             USB testing cannot continue.\n");
            exit(EXIT_FAILURE);
        }
    }

    VERBOSE(2, "Local and remote threads are in synch, collecting results.\n");
    
    // The world is in a coherent state. Time to collect the results.
    // The return value of this function is a simple boolean. More
    // detailed results will be held in a Tcl variable as a list of
    // messages. It is desirable to keep both local and remote results
    // in order.
    for (i = 0; i < ((local_thread_count < remote_thread_count) ? local_thread_count : remote_thread_count); i++) {
        if (!pool[i].test.result_pass) {
            Tcl_SetVar(interp, "usbtest::results", pool[i].test.result_message,
                       all_ok ? (TCL_GLOBAL_ONLY | TCL_LIST_ELEMENT) : (TCL_GLOBAL_ONLY | TCL_APPEND_VALUE | TCL_LIST_ELEMENT));
            all_ok = 0;
        }
        usb_reliable_control_message(usb_master_fd, USB_TYPE_CLASS | USB_RECIP_DEVICE | USB_DIR_IN, USBTEST_GET_RESULT,
                                     0, i, USBTEST_MAX_CONTROL_DATA, (void*) result_buf);
        if (!result_buf[0]) {
            Tcl_SetVar(interp, "usbtest::results", &(result_buf[1]),
                       all_ok ? TCL_GLOBAL_ONLY : (TCL_GLOBAL_ONLY | TCL_APPEND_VALUE | TCL_LIST_ELEMENT));
            all_ok = 0;
        }
    }
    for (j = i; j < local_thread_count; j++) {
        if (!pool[j].test.result_pass) {
            Tcl_SetVar(interp, "usbtest::results", pool[j].test.result_message,
                       all_ok ? TCL_GLOBAL_ONLY : (TCL_GLOBAL_ONLY | TCL_APPEND_VALUE | TCL_LIST_ELEMENT));
            all_ok = 0;
        }
    }
    for (j = i; j < remote_thread_count; j++) {
        usb_reliable_control_message(usb_master_fd, USB_TYPE_CLASS | USB_RECIP_DEVICE | USB_DIR_IN, USBTEST_GET_RESULT,
                                     0, i, USBTEST_MAX_CONTROL_DATA, (void*) result_buf);
        if (!result_buf[0]) {
            Tcl_SetVar(interp, "usbtest::results", &(result_buf[1]),
                       all_ok ? TCL_GLOBAL_ONLY : (TCL_GLOBAL_ONLY | TCL_APPEND_VALUE | TCL_LIST_ELEMENT));
            all_ok = 0;
        }
    }
    VERBOSE(2, "Overall test result %d\n", all_ok);
    
    Tcl_SetResult(interp, all_ok ? "1" : "0", TCL_STATIC);
    
    run_done();
    
    return TCL_OK;
}

/*}}}*/
/*{{{  Set verbosity                                            */

// ----------------------------------------------------------------------------
// Allow Tcl scripts to control verbosity levels for both host and target
static int
tcl_host_verbose(ClientData     clientData    __attribute__ ((unused)),
                  Tcl_Interp*    interp,
                  int            argc,
                  char**         argv)
{
    int level;
    
    if (2 != argc) {
        Tcl_SetResult(interp, "wrong # args: should be \"usbtest::host_verbose <level>\"", TCL_STATIC);
        return TCL_ERROR;
    }
    if (TCL_OK != Tcl_GetInt(interp, argv[1], &level)) {
        Tcl_SetResult(interp, "invalid argument: verbosity level should be numeric", TCL_STATIC);
        return TCL_ERROR;
    }

    verbose = level;
    return TCL_OK;
}

static int
tcl_target_verbose(ClientData     clientData    __attribute__ ((unused)),
                   Tcl_Interp*    interp,
                   int            argc,
                   char**         argv)
{
    int level;
    
    if (2 != argc) {
        Tcl_SetResult(interp, "wrong # args: should be \"usbtest::target_verbose <level>\"", TCL_STATIC);
        return TCL_ERROR;
    }
    if (TCL_OK != Tcl_GetInt(interp, argv[1], &level)) {
        Tcl_SetResult(interp, "invalid argument: verbosity level should be numeric", TCL_STATIC);
        return TCL_ERROR;
    }
    
    usb_reliable_control_message(usb_master_fd, USB_TYPE_CLASS | USB_RECIP_DEVICE, USBTEST_VERBOSE, level, 0, 0, NULL);
    usb_sync(usb_master_fd, -1);

    return TCL_OK;
}

/*}}}*/

/*}}}*/
/*{{{  AppInit()                                                */

// ----------------------------------------------------------------------------
// Application-specific initialization. We have a bare Tcl interpreter ready
// to start executing scripts that define various test cases. However some
// additional functions will have to be added to the interpreter, plus
// information about the various endpoints.

static int
usbhost_appinit(Tcl_Interp* interp)
{
    unsigned char   buf[USBTEST_MAX_CONTROL_DATA];
    int             number_of_endpoints;
    int             i;
    char*           location;
    
    // Start by creating a usbtest namespace, for use by the various functions
    // and variables.
    if (TCL_OK != Tcl_Eval(interp,
                           "namespace eval usbtest {\n"
                           "    variable number_of_endpoints 0\n"
                           "    array set endpoint [list]\n"
                           "}\n")) {
        fprintf(stderr, "usbhost: internal error, failed to create Tcl usbtest:: namespace\n");
        fprintf(stderr, "         Please check Tcl version (8.0b1 or later required).\n");
        exit(EXIT_FAILURE);
    }

    // Add some information about the install path so that the
    // main Tcl script can find and execute test scripts.
    location = getenv("USBHOSTDIR");
    if (NULL == location) {
        location = USBAUXDIR;
    }
    Tcl_SetVar(interp, "usbtest::USBAUXDIR", location, TCL_GLOBAL_ONLY);

    // Also set the verbosity level correctly
    Tcl_SetVar2Ex(interp, "usbtest::verbose", NULL, Tcl_NewIntObj(verbose), TCL_GLOBAL_ONLY);
    
    // Next we need to know the number of endpoints, and for each
    // endpoint we want additional information such as type. The
    // results are placed in a Tcl array.
    usb_reliable_control_message(usb_master_fd, USB_TYPE_CLASS | USB_RECIP_DEVICE | USB_DIR_IN, USBTEST_ENDPOINT_COUNT,
                                 0, 0, 1, buf);
    number_of_endpoints = buf[0];
    Tcl_SetVar2Ex(interp, "usbtest::endpoint_count", NULL, Tcl_NewIntObj(number_of_endpoints), TCL_GLOBAL_ONLY);
    
    for (i = 0; i < number_of_endpoints; i++) {
        char            varname[256];
        int             result;
        int             endpoint_min_size;
        int             endpoint_max_size;
        int             index;
        
        memset(buf, 0, USBTEST_MAX_CONTROL_DATA);
        result = usb_reliable_control_message(usb_master_fd, USB_TYPE_CLASS | USB_RECIP_DEVICE | USB_DIR_IN,
                                              USBTEST_ENDPOINT_DETAILS, 0, i, USBTEST_MAX_CONTROL_DATA, buf);
        if (result < 13) {
            fprintf(stderr, "usbhost: error, received insufficient endpoint data back from the target.\n");
            exit(EXIT_FAILURE);
        }

        // See protocol.h for the encoding used.
        sprintf(varname, "usbtest::endpoint_data(%d,type)", i);
        switch(buf[0]) {
          case USB_ENDPOINT_XFER_CONTROL    : Tcl_SetVar(interp, varname, "control",     TCL_GLOBAL_ONLY); break;
          case USB_ENDPOINT_XFER_ISOC       : Tcl_SetVar(interp, varname, "isochronous", TCL_GLOBAL_ONLY); break;
          case USB_ENDPOINT_XFER_BULK       : Tcl_SetVar(interp, varname, "bulk",        TCL_GLOBAL_ONLY); break;
          case USB_ENDPOINT_XFER_INT        : Tcl_SetVar(interp, varname, "interrupt",   TCL_GLOBAL_ONLY); break;
        }

        sprintf(varname, "usbtest::endpoint_data(%d,number)", i);
        Tcl_SetVar2Ex(interp, varname, NULL, Tcl_NewIntObj((int) buf[1]), TCL_GLOBAL_ONLY);

        sprintf(varname, "usbtest::endpoint_data(%d,direction)", i);
        if (USB_DIR_OUT == buf[2]) {
            Tcl_SetVar(interp, varname, "out", TCL_GLOBAL_ONLY);
        } else {
            Tcl_SetVar(interp, varname, "in", TCL_GLOBAL_ONLY);
        }

        sprintf(varname, "usbtest::endpoint_data(%d,max_in_padding)", i);
        Tcl_SetVar2Ex(interp, varname, NULL, Tcl_NewIntObj((int) buf[3]), TCL_GLOBAL_ONLY);

        sprintf(varname, "usbtest::endpoint_data(%d,min_size)", i);
        index               = 4;
        endpoint_min_size   = unpack_int(buf, &index);
        Tcl_SetVar2Ex(interp, varname, NULL, Tcl_NewIntObj(endpoint_min_size), TCL_GLOBAL_ONLY);
            
        sprintf(varname, "usbtest::endpoint_data(%d,max_size)", i);
        endpoint_max_size   = unpack_int(buf, &index);
        if (USB_ENDPOINT_XFER_CONTROL == buf[0]) {
            if (endpoint_max_size > USBTEST_MAX_CONTROL_DATA) {
                endpoint_max_size = USBTEST_MAX_CONTROL_DATA;
            }
        } else {
            if ((-1 == endpoint_max_size) || (endpoint_max_size > USBTEST_MAX_BULK_DATA)) {
                endpoint_max_size = USBTEST_MAX_BULK_DATA;
            }
        }
        Tcl_SetVar2Ex(interp, varname, NULL, Tcl_NewIntObj(endpoint_max_size), TCL_GLOBAL_ONLY);

        sprintf(varname, "usbtest::endpoint_data(%d,devtab)", i);
        Tcl_SetVar(interp, varname, (char*) &(buf[12]), TCL_GLOBAL_ONLY);

        // Perform any additional endpoint-specific initialization to make
        // sure host and target can actually communicate via this endpoint.
        switch(buf[0]) {
          case USB_ENDPOINT_XFER_CONTROL    :
          {
              usb_initialise_control_endpoint(endpoint_min_size, endpoint_max_size);
              break;
          }
          case USB_ENDPOINT_XFER_ISOC       :
          {
              if (USB_DIR_OUT == buf[2]) {
                  usb_initialise_isochronous_out_endpoint(buf[1], endpoint_min_size, endpoint_max_size);
              } else {
                  usb_initialise_isochronous_in_endpoint(buf[1], endpoint_min_size, endpoint_max_size);
              }
              break;
          }
          case USB_ENDPOINT_XFER_BULK       :
          {
              if (USB_DIR_OUT == buf[2]) {
                  usb_initialise_bulk_out_endpoint(buf[1], endpoint_min_size, endpoint_max_size);
              } else {
                  usb_initialise_bulk_in_endpoint(buf[1], endpoint_min_size, endpoint_max_size, buf[3]);
              }
              
              break;
          }
          case USB_ENDPOINT_XFER_INT        :
          {
              if (USB_DIR_OUT == buf[2]) {
                  usb_initialise_interrupt_out_endpoint(buf[1], endpoint_min_size, endpoint_max_size);
              } else {
                  usb_initialise_interrupt_in_endpoint(buf[1], endpoint_min_size, endpoint_max_size);
              }
              break;
          }
        }
    }

    // Register appropriate commands with the Tcl interpreter
    Tcl_CreateCommand(interp, "usbtest::target_pass",       &tcl_target_pass,       (ClientData) NULL, (Tcl_CmdDeleteProc*) NULL);
    Tcl_CreateCommand(interp, "usbtest::target_pass_exit",  &tcl_target_pass_exit,  (ClientData) NULL, (Tcl_CmdDeleteProc*) NULL);
    Tcl_CreateCommand(interp, "usbtest::target_fail",       &tcl_target_fail,       (ClientData) NULL, (Tcl_CmdDeleteProc*) NULL);
    Tcl_CreateCommand(interp, "usbtest::target_fail_exit",  &tcl_target_fail_exit,  (ClientData) NULL, (Tcl_CmdDeleteProc*) NULL);
    Tcl_CreateCommand(interp, "usbtest::target_abort",      &tcl_target_abort,      (ClientData) NULL, (Tcl_CmdDeleteProc*) NULL);
    Tcl_CreateCommand(interp, "usbtest::_test_bulk",        &tcl_test_bulk,         (ClientData) NULL, (Tcl_CmdDeleteProc*) NULL);
    Tcl_CreateCommand(interp, "usbtest::_test_control_in",  &tcl_test_control_in,   (ClientData) NULL, (Tcl_CmdDeleteProc*) NULL);
    Tcl_CreateCommand(interp, "usbtest::_cancel",           &tcl_cancel,            (ClientData) NULL, (Tcl_CmdDeleteProc*) NULL);
    Tcl_CreateCommand(interp, "usbtest::_run",              &tcl_run,               (ClientData) NULL, (Tcl_CmdDeleteProc*) NULL);
    Tcl_CreateCommand(interp, "usbtest::host_verbose",      &tcl_host_verbose,      (ClientData) NULL, (Tcl_CmdDeleteProc*) NULL);
    Tcl_CreateCommand(interp, "usbtest::target_verbose",    &tcl_target_verbose,    (ClientData) NULL, (Tcl_CmdDeleteProc*) NULL);

    return TCL_OK;
}

/*}}}*/
/*{{{  main()                                                   */

// ----------------------------------------------------------------------------
// System start-up. After argument processing this code checks that
// there is a suitable USB target attached - if not then there is no
// point in proceeding. Otherwise further initialization is performed
// and then control is passed to a Tcl interpreter.

static void
usage(void)
{
    printf("usbhost: usage, usbhost [-V|--verbose] [-v|--version] [-h|--help] <test> [args]\n");
    printf("    -V, --verbose    Make the host-side output additional information\n");
    printf("                     during test runs. This argument can be repeated to\n");
    printf("                     increase verbosity.\n");
    printf("    -v, --version    Output version information for usbhost.\n");
    printf("    -h, --help       Output this help information.\n");
    printf("    <test>           The name of a USB test case, for example list.tcl\n");
    printf("    [args]           Optional additional arguments for the testcase.\n");
    exit(0);
}

static void
version(void)
{
    printf("usbhost: version %s\n", USBHOST_VERSION);
    printf("       : built from USB slave package version %s\n", PKGVERSION);
    printf("       : support files installed in %s\n", USBAUXDIR);
    exit(0);
}

int
main(int argc, char** argv)
{
    char*   interpreter = argv[0];
    char**  new_argv;
    char    path[_POSIX_PATH_MAX];
    char*   location;
    int     i;

    // Argument processing
    for (i = 1; i < argc; i++) {
        if ((0 == strcmp("-h", argv[i])) || (0 == strcmp("-H", argv[i])) || (0 == strcmp("--help", argv[i]))) {
            usage();
        }
        if ((0 == strcmp("-v", argv[i])) || (0 == strcmp("--version", argv[i]))) {
            version();
        }
        if ((0 == strcmp("-V", argv[i])) || (0 == strcmp("--verbose", argv[i]))) {
            verbose++;
            continue;
        }

        // The first unrecognised argument should correspond to the test script.
        break;
    }
    argc  = (argc - i) + 1;
    argv  = (argv + i) - 1;

    if (1 == argc) {
        fprintf(stderr, "usbhost: at least one test script must be specified on the command line.\n");
        exit(EXIT_FAILURE);
    }

    usb_master_fd = usb_open_device();
    if (-1 == usb_master_fd) {
        return EXIT_FAILURE;
    }

    // There is a valid USB target. Initialize the pool of threads etc.
    pool_initialize();

    // Now start a Tcl interpreter. Tcl_Main() will interpret the
    // first argument as the name of a Tcl script to execute,
    // i.e. usbhost.tcl. This can be found in the install tree,
    // but during development it is inconvenient to run
    // "make install" every time the Tcl script is edited so an
    // environment variable can be used to override the location.
    new_argv = malloc((argc + 2) * sizeof(char*));
    if (NULL == new_argv) {
        fprintf(stderr, "usbhost: internal error, out of memory.\n");
        exit(EXIT_FAILURE);
    }
    new_argv[0] = interpreter;

    location = getenv("USBHOSTDIR");
    if (NULL == location) {
        location = USBAUXDIR;
    }
    snprintf(path, _POSIX_PATH_MAX, "%s/usbhost.tcl", location);
    if (0 != access(path, R_OK)) {
        fprintf(stderr, "usbhost: cannot find or access required Tcl script\n");
        fprintf(stderr, "       : %s\n", path);
        exit(EXIT_FAILURE);
    }
    new_argv[1] = path;

    for (i = 1; i < argc; i++) {
        new_argv[i+1] = argv[i];
    }
    new_argv[i+1]   = NULL;

    Tcl_Main(i+1, new_argv, &usbhost_appinit);
    
    return EXIT_SUCCESS;
}

/*}}}*/
