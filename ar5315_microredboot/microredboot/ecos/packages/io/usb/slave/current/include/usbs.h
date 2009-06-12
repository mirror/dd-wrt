#ifndef CYGONCE_USBS_H
# define CYGONCE_USBS_H
//==========================================================================
//
//      include/usbs.h
//
//      The generic USB slave-side support
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
// Date:         2000-10-04
// Purpose:
// Description:  USB slave-side support
//
//
//####DESCRIPTIONEND####
//==========================================================================

# include <pkgconf/system.h>
# include <cyg/infra/cyg_type.h>
# include <cyg/io/usb/usb.h>

#ifdef __cplusplus
extern "C" {
#endif
    
// The USB slave-side eCos support involves a number of different
// components:
//
// 1) a hardware-specific package to drive a specific chip implementation.
//    This provides access to the endpoints. All the hardware-specific
//    packages implement a common interface.
//
// 2) a common package (this one). This defines the interface implemented
//    by the hardware-specific packages. It also provides support for
//    the various generic control messages, using information provided
//    by higher-level code and invoking callbacks as appropriate.
//
// 3) some number of support packages for particular types of
//    application, for example ethernet or mass-storage.
//
// Typically there will only be one USB slave device, although the design
// does allow for multiple devices. Each device should provide a
// usbs_control_endpoint structure and zero or more usbs_data_endpoint
// structures. Each usbs_data_endpoint structure supports uni-directional
// transfers on a single endpoint. If an endpoint can support multiple
// types of transfer then there will be some control operation to switch
// between bulk, interrupt and isochronous.
//
// Access to the USB endpoints can go either via usbs_ calls which
// take a usbs_endpoint structure, or via open/read/write calls. The
// latter is more likely to be used in application code since it
// involves a familiar interface. The former is more appropriate for
// eCos packages layered on top of the USB code. The difference is
// synchronous vs. asynchronous: the open/read/write model involves
// blocking operations, implying a need for extra threads; the usbs_
// calls involve start operations and a completion callback. In
// practice the read and write calls are implemented using the
// lower-level code.

// Enumeration data. This requires information about the hardware,
// specifically what endpoints are available and what they get used
// for. It also requires information about the application class
// packages that are in the configuration, and quite possibly about
// things in application space. Some of the enumeration info such as
// the vendor id is inherently application-specific. Hence there is no
// way of generating part or all of the the enumeration information
// automatically, instead it is up to application code to supply this.
//
// The intention is that application provides all the data via const
// static objects, allowing the data to live in ROM. Alternatively the
// data structures can go into the .data section as normal, allowing
// them to be edited at run-time.
//
// There can be only one device descriptor, so that is part of the
// main enumeration data structure. There can be an unknown number of
// configurations so application code has to initialize an array of
// these. Ditto for interfaces and endpoints. The first x interfaces
// in the array correspond to the first configuration, the next y
// interfaces to the second configuration, etc. The endpoints array
// works in the same way.
//
// In the initial implementation multiple languages are not supported
// so a simple array of strings suffices. The first entry of these
// is still special in that it should define a single supported
// LANGID. All strings should be encoded as per the USB standard:
// a length field, a type code of USB_STRING_DESCRIPTOR_TYPE,
// and data in unicode format. In future multiple language support
// may be supported via configury with the default case remaining
// a single language, thus avoiding incompatibility problems.
    
typedef struct usbs_enumeration_data {
    usb_device_descriptor               device;
    int                                 total_number_interfaces;
    int                                 total_number_endpoints;
    int                                 total_number_strings;
    const usb_configuration_descriptor* configurations;
    const usb_interface_descriptor*     interfaces;
    const usb_endpoint_descriptor*      endpoints;
    const unsigned char**               strings;
} usbs_enumeration_data;
    
// The current state of a USB device. This involves a bit to mark
// whether or not the device has been suspended, plus a state machine.
// On some hardware it may not be possible to distinguish between the
// detached, attached and powered states. If so then the initial state
// will be POWERED.

#define USBS_STATE_DETACHED             0x01
#define USBS_STATE_ATTACHED             0x02
#define USBS_STATE_POWERED              0x03
#define USBS_STATE_DEFAULT              0x04
#define USBS_STATE_ADDRESSED            0x05
#define USBS_STATE_CONFIGURED           0x06
#define USBS_STATE_MASK                 0x7F
#define USBS_STATE_SUSPENDED            (1 << 7)

// State changes. Application code or higher-level packages should
// install an appropriate state change function which will get
// invoked with details of the state change.
typedef enum {
    USBS_STATE_CHANGE_DETACHED          = 1,
    USBS_STATE_CHANGE_ATTACHED          = 2,
    USBS_STATE_CHANGE_POWERED           = 3,
    USBS_STATE_CHANGE_RESET             = 4,    
    USBS_STATE_CHANGE_ADDRESSED         = 5,
    USBS_STATE_CHANGE_CONFIGURED        = 6,
    USBS_STATE_CHANGE_DECONFIGURED      = 7,    
    USBS_STATE_CHANGE_SUSPENDED         = 8,
    USBS_STATE_CHANGE_RESUMED           = 9
} usbs_state_change;

typedef enum {
    USBS_CONTROL_RETURN_HANDLED = 0,
    USBS_CONTROL_RETURN_UNKNOWN = 1,
    USBS_CONTROL_RETURN_STALL   = 2
} usbs_control_return;

typedef struct usbs_control_endpoint {
    // The state is maintained by the USB code and should not be
    // modified by anything higher up.
    int                 state;

    // The enumeration data should be supplied by higher level code,
    // usually the application. Often this data will be constant.
    const usbs_enumeration_data* enumeration_data;
    
    // This function pointer is supplied by the USB device driver.
    // Application code should invoke it directly or via the
    // usbs_start() function when the system is ready. Typically it
    // will cause the USB lines to switch from tristate to active,
    // and the USB host/hub should detect this.
    void                (*start_fn)(struct usbs_control_endpoint*);

    // This function is used for polled operation when interrupts
    // are disabled. This can happen in some debugging contexts.
    // Higher-level code may also need to know about the interrupt
    // number(s) used.
    void                (*poll_fn)(struct usbs_control_endpoint*);
    int                 interrupt_vector;
    
    // When a new control message arrives it will be in this buffer
    // where the appropriate callback functions can examine it. The
    // USB code will not modify the buffer unless a new control
    // message arrives. The control_buffer can also be re-used
    // by handlers to maintain some state information, e.g.
    // for coping with complicated IN requests, but this is only
    // allowed if they actually handle the request.
    unsigned char       control_buffer[8];

    // This callback will be invoked by the USB code following a
    // change in USB state, e.g. to SUSPENDED mode. Higher-level code
    // should install a suitable function. There is some callback data
    // as well. This gets passed explicitly to the callback function,
    // in addition to the control endpoint structure. The reason is
    // that the actual state change callback may be some sort of
    // multiplexer inside a multifunction peripheral, and this
    // multiplexer wants to invoke device-specific state change
    // functions. However in simple devices those device-specific
    // state change functions could be invoked directly.
    void                (*state_change_fn)(struct usbs_control_endpoint*, void*, usbs_state_change, int /* old state */);
    void*               state_change_data;
    // When a standard control message arrives, the device driver will
    // detect some requests such as SET_ADDRESS and handle it
    // internally. Otherwise if higher-level code has installed a
    // callback then that will be invoked. If the callback returns
    // UNKNOWN then the default handler usbs_handle_standard_control()
    // is used to process the request. 
    usbs_control_return (*standard_control_fn)(struct usbs_control_endpoint*, void*);
    void*               standard_control_data;
    
    // These three callbacks are used for other types of control
    // messages. The generic USB code has no way of knowing what
    // such control messages are about.
    usbs_control_return (*class_control_fn)(struct usbs_control_endpoint*, void*);
    void*               class_control_data;
    usbs_control_return (*vendor_control_fn)(struct usbs_control_endpoint*, void*);
    void*               vendor_control_data;
    usbs_control_return (*reserved_control_fn)(struct usbs_control_endpoint*, void*);
    void*               reserved_control_data;

    // If a control operation involves transferring more data than
    // just the initial eight-byte packet, the following fields are
    // used to keep track of the current operation. The original
    // control request indicates the direction of the transfer (IN or
    // OUT) and a length field. For OUT this length is exact, for IN
    // it is an upper bound. The transfer operates mostly as per the
    // bulk protocol, but if the length requested is an exact multiple
    // of the control fifo size (typically eight bytes) then there
    // is no need for an empty packet at the end.
    //
    // For an OUT operation the control message handler should supply
    // a suitable buffer via the "buffer" field below. The only other
    // field of interest is the complete_fn which must be provided and
    // will be invoked once all the data has arrived. Alternatively
    // the OUT operation may get aborted if a new control message
    // arrives. The second argument is an error code -EPIPE or -EIO,
    // or zero to indicate success. The return code is used by the
    // device driver during the status phase.
    //
    // IN is more complicated and the defined interface makes it
    // possible to gather data from multiple locations, eliminating
    // the need for copying into large buffers in some circumstances.
    // Basically when an IN request arrives the device driver will
    // look at the buffer and buffer_size fields, extracting data from
    // there if possible. If the current buffer has been exhausted
    // then the the refill function will be invoked, and this can
    // reset the buffer and size fields to point somewhere else. 
    // This continues until such time that there is no longer
    // a refill function and the current buffer is empty. The
    // refill function can use the refill_data and refill_index
    // to keep track of the current state. The control_buffer
    // fields are available as well. At the end of the transfer,
    // if a completion function has been supplied then it will
    // be invoked. The return code will be ignored.
    unsigned char*      buffer;
    int                 buffer_size;
    void                (*fill_buffer_fn)(struct usbs_control_endpoint*);
    void*               fill_data;
    int                 fill_index;
    usbs_control_return (*complete_fn)(struct usbs_control_endpoint*, int);
} usbs_control_endpoint;

// Data endpoints are a little bit simpler, but not much. From the
// perspective of a device driver things a single buffer is most
// convenient, but that is quite likely to require a max-size buffer
// at a higher level and an additional copy operation. Supplying
// a vector of buffers is a bit more general, but in a layered
// system it may be desirable to prepend to this vector...
// A combination of a current buffer and a refill/empty function
// offers flexibility, at the cost of additional function calls
// from inside the device driver.
//
// FIXME: implement support for fill/empty functions.
//
// Some USB devices may prefer buffers of particular alignment,
// e.g. for DMA purposes. This is hard to reconcile with the
// current interface. However pushing such alignment restrictions
// etc. up into the higher levels is difficult, e.g. it does
// not map at all onto a conventional read/write interface.
// The device driver will just have to do the best it can.
//
// The completion function will be invoked at the end of the transfer.
// The second argument indicates per-transfer completion data. The
// third argument indicates the total amount received, or an error
// code: typically -EPIPE to indicate a broken conenction; -EAGAIN to
// indicate a stall condition; -EMSGSIZE if the host is sending more
// data than the target is expecting; or -EIO to indicate some other
// error. Individual device drivers should avoid generating other
// errors.
typedef struct usbs_rx_endpoint {
    void                (*start_rx_fn)(struct usbs_rx_endpoint*);
    void                (*set_halted_fn)(struct usbs_rx_endpoint*, cyg_bool);
    void                (*complete_fn)(void*, int);
    void*               complete_data;
    unsigned char*      buffer;
    int                 buffer_size;
    cyg_bool            halted;
} usbs_rx_endpoint;

typedef struct usbs_tx_endpoint {
    void                (*start_tx_fn)(struct usbs_tx_endpoint*);
    void                (*set_halted_fn)(struct usbs_tx_endpoint*, cyg_bool);
    void                (*complete_fn)(void*, int);
    void*               complete_data;
    const unsigned char*buffer;
    int                 buffer_size;
    cyg_bool            halted;
} usbs_tx_endpoint;

// Functions called by device drivers.
extern usbs_control_return usbs_handle_standard_control(struct usbs_control_endpoint*);
    
// Utility functions. These just invoke the corresponding function
// pointers in the endpoint structures. It is assumed that the
// necessary fields in the endpoint structures will have been
// filled in already.
extern void     usbs_start(usbs_control_endpoint*);
extern void     usbs_start_rx(usbs_rx_endpoint*);
extern void     usbs_start_tx(usbs_tx_endpoint*);
extern void     usbs_start_rx_buffer(usbs_rx_endpoint*, unsigned char*, int, void (*)(void*, int), void*);
extern void     usbs_start_tx_buffer(usbs_tx_endpoint*, const unsigned char*, int, void (*)(void*, int), void*);
extern cyg_bool usbs_rx_endpoint_halted(usbs_rx_endpoint*);
extern cyg_bool usbs_tx_endpoint_halted(usbs_tx_endpoint*);
extern void     usbs_set_rx_endpoint_halted(usbs_rx_endpoint*, cyg_bool);
extern void     usbs_set_tx_endpoint_halted(usbs_tx_endpoint*, cyg_bool);
extern void     usbs_start_rx_endpoint_wait(usbs_rx_endpoint*, void (*)(void*, int), void*);
extern void     usbs_start_tx_endpoint_wait(usbs_tx_endpoint*, void (*)(void*, int), void*);
    
// Functions that can go into devtab entries. These should not be
// called directly, they are intended only for use by USB device
// drivers.
#if defined(CYGPKG_IO) && defined(CYGPKG_ERROR)
#include <cyg/io/io.h>
extern Cyg_ErrNo usbs_devtab_cwrite(cyg_io_handle_t, const void*, cyg_uint32*);
extern Cyg_ErrNo usbs_devtab_cread(cyg_io_handle_t, void*, cyg_uint32*);
extern Cyg_ErrNo usbs_devtab_get_config(cyg_io_handle_t, cyg_uint32, void*, cyg_uint32*);
extern Cyg_ErrNo usbs_devtab_set_config(cyg_io_handle_t, cyg_uint32, const void*, cyg_uint32*);
#endif

// Additional support for testing.
// Test cases need to have some way of finding out about what support is
// actually provided by the USB device driver, for example what endpoints
// are available. There is no perfect way of achieving this. One approach
// would be to scan through the devtab table looking for devices of the
// form /dev/usbs1r. That is not reliable: the devtab entries may have been
// configured out if higher-level code uses the usb-specific API; or the
// devtab entries may have been renamed. Also having a devtab entry does not
// really give the kind of information a general-purpose testcase needs,
// for example upper bounds on transfer size.    
// 
// An alternative approach is to have a data structure that somehow
// defines the USB hardware, and the USB device driver then creates an
// instance of this. This is the approach actually taken. The problem
// now is how the test code can access this instance. Accessing by
// unique name is simple, as long as there is only one USB device in
// the system (which of course will usually be the case on the USB
// slave side). Alternative approaches such as creating a table at
// link time or a list during static construction time are vulnerable
// either to selective linking or to having these structures present
// in applications other than the test cases. In future it might be
// possible to address the latter issue by extending the build system
// support, e.g. a new library libtesting.a and a new object file
// testing.o.
//
// Note that a given endpoint could be used for bulk transfers some
// of the time, then for isochronous transfers, etc. It is the
// responsibility of the host to only perform one type of IN operation
// for a given endpoint number, and ditto for OUT.    

typedef struct usbs_testing_endpoint {
    int         endpoint_type;          // One of ATTR_CONTROL, ATTR_BULK, ...
    int         endpoint_number;        // Between 0 and 15
    int         endpoint_direction;     // ENDPOINT_IN or ENDPOINT_OUT
    void*       endpoint;               // pointer to the usbs_control_endpoint, usbs_rx_endpoint, ...
    const char* devtab_entry;           // e.g. "/dev/usbs1r", or 0 if inaccessible via devtab
    int         min_size;               // Minimum transfer size
    int         max_size;               // -1 indicates no specific upper bound
    int         max_in_padding;         // extra bytes that the target may send, usually 0.
                                        // Primarily for SA11x0 hardware. It is assumed
                                        // for now that no other hardware will exhibit
                                        // comparable problems.
    int         alignment;              // Buffer should be aligned to a suitable boundary
} usbs_testing_endpoint;

// A specific instance provided by the device driver. The end of
// the table is indicated by a NULL endpoint field.    
extern usbs_testing_endpoint usbs_testing_endpoints[];    

#define USBS_TESTING_ENDPOINTS_TERMINATOR                           \
    {                                                               \
        endpoint_type       : USB_ENDPOINT_DESCRIPTOR_ATTR_CONTROL, \
        endpoint_number     : 0,                                    \
        endpoint_direction  : USB_ENDPOINT_DESCRIPTOR_ENDPOINT_IN,  \
        endpoint            : (void*) 0,                            \
        devtab_entry        : (const char*) 0,                      \
        min_size            : 0,                                    \
        max_size            : 0,                                    \
        max_in_padding      : 0,                                    \
        alignment           : 0                                     \
    }

#define USBS_TESTING_ENDPOINTS_IS_TERMINATOR(_endpoint_) ((void*)0 == (_endpoint_).endpoint)
    
#ifdef __cplusplus
} // extern "C" {
#endif

#endif // CYGONCE_USBS_H

