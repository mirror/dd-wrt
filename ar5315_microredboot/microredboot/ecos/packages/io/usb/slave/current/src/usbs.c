//==========================================================================
//
//      usbs.c
//
//      Generic USB slave-side support
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
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/cyg_trac.h>
#include <cyg/infra/diag.h>
#include <cyg/io/usb/usbs.h>
#include <cyg/hal/drv_api.h>

// ----------------------------------------------------------------------------
// Devtab entry support. This code can be compiled with no overheads as
// long as the necessary support package is in place.
#ifdef CYGPKG_IO
# include <cyg/io/io.h>
# include <cyg/io/devtab.h>
// ----------------------------------------------------------------------------
// read()/write() functions applied to USB endpoints. These just
// indirect via the usbs_endpoint structures and wait for the
// callback to happen.

typedef struct usbs_callback_data {
    bool                completed;
    int                 result;
    cyg_drv_mutex_t     lock;
    cyg_drv_cond_t      signal;
} usbs_callback_data;

static void
usbs_devtab_callback(void* arg, int result)
{
    usbs_callback_data* callback_data = (usbs_callback_data*) arg;
    callback_data->result       = result;
    callback_data->completed    = true;
    cyg_drv_cond_signal(&(callback_data->signal));
}
    
Cyg_ErrNo
usbs_devtab_cwrite(cyg_io_handle_t handle, const void* buf, cyg_uint32* size)
{
    usbs_callback_data  wait;
    cyg_devtab_entry_t* devtab_entry;
    usbs_tx_endpoint*   endpoint;
    int                 result = ENOERR;
    
    CYG_REPORT_FUNCTION();
    
    wait.completed      = 0;
    cyg_drv_mutex_init(&wait.lock);
    cyg_drv_cond_init(&wait.signal, &wait.lock);

    devtab_entry      = (cyg_devtab_entry_t*) handle;
    CYG_CHECK_DATA_PTR( devtab_entry, "A valid endpoint must be supplied");
    endpoint          = (usbs_tx_endpoint*) devtab_entry->priv;
    
    CYG_CHECK_DATA_PTR( endpoint, "The handle must correspond to a USB endpoint");
    CYG_CHECK_FUNC_PTR( endpoint->start_tx_fn, "The endpoint must have a start_tx function");

    endpoint->buffer            = (unsigned char*) buf;
    endpoint->buffer_size       = (int) *size;
    endpoint->complete_fn       = &usbs_devtab_callback;
    endpoint->complete_data     = (void*) &wait;
    (*endpoint->start_tx_fn)(endpoint);
    
    cyg_drv_mutex_lock(&wait.lock);
    while (!wait.completed) {
        cyg_drv_cond_wait(&wait.signal);
    }
    cyg_drv_mutex_unlock(&wait.lock);
    if (wait.result < 0) {
        result = wait.result;
    } else {
        *size = wait.result;
    }
    
    cyg_drv_cond_destroy(&wait.signal);
    cyg_drv_mutex_destroy(&wait.lock);

    CYG_REPORT_RETURN();
    return result;
}

Cyg_ErrNo
usbs_devtab_cread(cyg_io_handle_t handle, void* buf, cyg_uint32* size)
{
    usbs_callback_data  wait;
    cyg_devtab_entry_t* devtab_entry;
    usbs_rx_endpoint*   endpoint;
    int                 result = ENOERR;
    
    CYG_REPORT_FUNCTION();
    
    wait.completed      = 0;
    cyg_drv_mutex_init(&wait.lock);
    cyg_drv_cond_init(&wait.signal, &wait.lock);

    devtab_entry      = (cyg_devtab_entry_t*) handle;
    CYG_CHECK_DATA_PTR( devtab_entry, "A valid endpoint must be supplied");
    endpoint          = (usbs_rx_endpoint*) devtab_entry->priv;
    
    CYG_CHECK_DATA_PTR( endpoint, "The handle must correspond to a USB endpoint");
    CYG_CHECK_FUNC_PTR( endpoint->start_rx_fn, "The endpoint must have a start_rx function");

    endpoint->buffer            = (unsigned char*) buf;
    endpoint->buffer_size       = (int) *size;
    endpoint->complete_fn       = &usbs_devtab_callback;
    endpoint->complete_data     = (void*) &wait;
    (*endpoint->start_rx_fn)(endpoint);
    cyg_drv_mutex_lock(&wait.lock);
    while (!wait.completed) {
        cyg_drv_cond_wait(&wait.signal);
    }
    cyg_drv_mutex_unlock(&wait.lock);
    if (wait.result < 0) {
        result = wait.result;
    } else {
        *size = wait.result;
    }
    
    cyg_drv_cond_destroy(&wait.signal);
    cyg_drv_mutex_destroy(&wait.lock);

    CYG_REPORT_RETURN();
    return result;
}

// ----------------------------------------------------------------------------
// More devtab functions, this time related to ioctl() style operations.
Cyg_ErrNo
usbs_devtab_get_config(cyg_io_handle_t handle, cyg_uint32 code, void* buf, cyg_uint32* size)
{
    return -EINVAL;
}

Cyg_ErrNo
usbs_devtab_set_config(cyg_io_handle_t handle, cyg_uint32 code, const void* buf, cyg_uint32* size)
{
    return -EINVAL;
}

#endif  //  CYGPKG_IO

// ----------------------------------------------------------------------------
// USB-specific functions that are useful for applications/packages which
// do not want to use the devtab interface. These may get called in DSR
// context.

void
usbs_start_rx(usbs_rx_endpoint* endpoint)
{
    CYG_CHECK_DATA_PTR( endpoint, "A valid USB endpoint must be supplied");
    CYG_CHECK_FUNC_PTR( endpoint->start_rx_fn, "The USB endpoint must support receive operations");
    (*endpoint->start_rx_fn)(endpoint);
}

void
usbs_start_rx_buffer(usbs_rx_endpoint* endpoint,
                     unsigned char* buf, int size,
                     void (*callback_fn)(void *, int), void* callback_arg)
{
    CYG_CHECK_DATA_PTR( endpoint, "A valid USB endpoint must be supplied");
    CYG_CHECK_FUNC_PTR( endpoint->start_rx_fn, "The USB endpoint must support receive operations");

    endpoint->buffer            = buf;
    endpoint->buffer_size       = size;
    endpoint->complete_fn       = callback_fn;
    endpoint->complete_data     = callback_arg;
    (*endpoint->start_rx_fn)(endpoint);
}

void
usbs_start_tx(usbs_tx_endpoint* endpoint)
{
    CYG_CHECK_DATA_PTR( endpoint, "A valid USB endpoint must be supplied");
    CYG_CHECK_FUNC_PTR( endpoint->start_tx_fn, "The USB endpoint must support receive operations");
    (*endpoint->start_tx_fn)(endpoint);
}

void
usbs_start_tx_buffer(usbs_tx_endpoint* endpoint,
                     const unsigned char* buf, int size,
                     void (*callback_fn)(void*, int), void *callback_arg)
{
    CYG_CHECK_DATA_PTR( endpoint, "A valid USB endpoint must be supplied");
    CYG_CHECK_FUNC_PTR( endpoint->start_tx_fn, "The USB endpoint must support receive operations");

    endpoint->buffer            = buf;
    endpoint->buffer_size       = size;
    endpoint->complete_fn       = callback_fn;
    endpoint->complete_data     = callback_arg;
    (*endpoint->start_tx_fn)(endpoint);
}

void
usbs_start(usbs_control_endpoint* endpoint)
{
    CYG_CHECK_DATA_PTR( endpoint, "A valid USB endpoint must be supplied");
    CYG_CHECK_FUNC_PTR( endpoint->start_fn, "The USB endpoint should have a start function");

    (*endpoint->start_fn)(endpoint);
}

cyg_bool
usbs_rx_endpoint_halted(usbs_rx_endpoint* endpoint)
{
   CYG_CHECK_DATA_PTR( endpoint, "A valid USB endpoint must be supplied");
   return endpoint->halted;
}

cyg_bool
usbs_tx_endpoint_halted(usbs_tx_endpoint* endpoint)
{
   CYG_CHECK_DATA_PTR( endpoint, "A valid USB endpoint must be supplied");
   return endpoint->halted;
}

void
usbs_set_rx_endpoint_halted(usbs_rx_endpoint* endpoint, cyg_bool halted)
{
    CYG_CHECK_DATA_PTR( endpoint, "A valid USB endpoint must be supplied");
    CYG_CHECK_FUNC_PTR( endpoint->set_halted_fn, "The USB endpoint should have a set-halted function");
    (*endpoint->set_halted_fn)(endpoint, halted);
}

void
usbs_set_tx_endpoint_halted(usbs_tx_endpoint* endpoint, cyg_bool halted)
{
    CYG_CHECK_DATA_PTR( endpoint, "A valid USB endpoint must be supplied");
    CYG_CHECK_FUNC_PTR( endpoint->set_halted_fn, "The USB endpoint should have a set-halted function");
    (*endpoint->set_halted_fn)(endpoint, halted);
}

void
usbs_start_rx_endpoint_wait(usbs_rx_endpoint* endpoint, void (*callback_fn)(void*, int), void* callback_data)
{
    endpoint->buffer            = (unsigned char*) 0;
    endpoint->buffer_size       = 0;
    endpoint->complete_fn       = callback_fn;
    endpoint->complete_data     = callback_data;
    (*endpoint->start_rx_fn)(endpoint);
}

void
usbs_start_tx_endpoint_wait(usbs_tx_endpoint* endpoint, void (*callback_fn)(void*, int), void* callback_data)
{
    endpoint->buffer            = (unsigned char*) 0;
    endpoint->buffer_size       = 0;
    endpoint->complete_fn       = callback_fn;
    endpoint->complete_data     = callback_data;
    (*endpoint->start_tx_fn)(endpoint);
}


// ----------------------------------------------------------------------------
// Handling of standard control messages. This will be invoked by
// a USB device driver, usually at DSR level, to process any control
// messages that cannot be handled by the hardware itself and that
// have also not been handled by the application-specific handler
// (if any).
//
// Because this function can run at DSR level performance is important.
//
// The various standard control messages are affected as follows:
//
// clear-feature: nothing can be done here about device requests to
// disable remote-wakeup or about endpoint halt requests. It appears
// to be legal to clear no features on an interface.
//
// get-configuration: if the device is not configured a single byte 0
// should be returned. Otherwise this code assumes only one configuration
// is supported and its id can be extracted from the enumeration data.
// For more complicated devices get-configuration has to be handled
// at a higher-level.
//
// get-descriptor: this is the big one. It is used to obtain
// the enumeration data. An auxiliary refill function is needed.
//
// get-interface: this can be used to identify the current variant
// for a given interface within a configuration. For simple devices
// there will be only interface, 0. For anything more complicated
// higher level code will have to take care of the request.
//
// get-status. Much the same as clear-feature.
//
// set-address. Must be handled at the device driver level.
//
// set-configuration: a value of 0 is used to deconfigure the device,
// which can be handled here. Otherwise this code assumes that only
// a single configuration is supported and enables that. For anything
// more complicated higher-level code has to handle this request.
//
// set-descriptor: used to update the enumeration data. This is not
// supported here, although higher-level code can choose to do so.
//
// set-feature. See clear-feature and get-status.
//
// set-interface. Variant interfaces are not supported by the
// base code so this request has to be handled at a higher level.
//
// synch-frame. This is only relevant for isochronous transfers
// which are not yet supported, and anyway it is not clear what
// could be done about these requests here.

// This refill function handles GET_DESCRIPTOR requests for a
// configuration. For details of the control_buffer usage see
// the relevant code in the main callback.
static void
usbs_configuration_descriptor_refill(usbs_control_endpoint* endpoint)
{
    usb_devreq* req                 = (usb_devreq*) endpoint->control_buffer;
    int         length              = (req->length_hi << 8) | req->length_lo;
    int         sent                = (req->index_hi << 8)  | req->index_lo;
    int         current_interface   = req->type;
    int         last_interface      = req->request;
    int         current_endpoint    = req->value_lo;
    int         last_endpoint       = req->value_hi;
    cyg_bool    done                = false;

    if (current_endpoint == last_endpoint) {
        // The next transfer should be a single interface - unless we have already finished.
        if (current_interface == last_interface) {
            done = true;
        } else {
            endpoint->buffer            = (unsigned char*) &(endpoint->enumeration_data->interfaces[current_interface]);
            if (USB_INTERFACE_DESCRIPTOR_LENGTH >= (length - sent)) {
                endpoint->buffer_size = length - sent;
                done = true;
            } else {
                endpoint->buffer_size   = USB_INTERFACE_DESCRIPTOR_LENGTH;
                sent                   += USB_INTERFACE_DESCRIPTOR_LENGTH;
                // Note that an interface with zero endpoints is ok. We'll just move
                // to the next interface in the next call.
                last_endpoint           = current_endpoint +
                    endpoint->enumeration_data->interfaces[current_interface].number_endpoints;
                current_interface++;
            }
        }
    } else {
        // The next transfer should be a single endpoint. The
        // endpoints are actually contiguous array elements
        // but may not be packed, so they have to be transferred
        // one at a time.
        endpoint->buffer     = (unsigned char*) &(endpoint->enumeration_data->endpoints[current_endpoint]);
        if ((sent + USB_ENDPOINT_DESCRIPTOR_LENGTH) >= length) {
            endpoint->buffer_size = length - sent;
            done = true;
        } else {
            endpoint->buffer_size = USB_ENDPOINT_DESCRIPTOR_LENGTH;
            current_endpoint ++;
        }
    }

    if (done) {
        endpoint->fill_buffer_fn = (void (*)(usbs_control_endpoint*)) 0;
    } else {
        req->type       = (unsigned char) current_interface;
        req->value_lo   = (unsigned char) current_endpoint;
        req->value_hi   = (unsigned char) last_endpoint;
        req->index_hi   = (unsigned char) (sent >> 8);
        req->index_lo   = (unsigned char) (sent & 0x00FF);
    }
}

usbs_control_return
usbs_handle_standard_control(usbs_control_endpoint* endpoint)
{
    usbs_control_return result = USBS_CONTROL_RETURN_UNKNOWN;
    usb_devreq*         req    = (usb_devreq*) endpoint->control_buffer;
    int                 length;
    int                 direction;
    int                 recipient;

    length      = (req->length_hi << 8) | req->length_lo;
    direction   = req->type & USB_DEVREQ_DIRECTION_MASK;
    recipient   = req->type & USB_DEVREQ_RECIPIENT_MASK;

    if (USB_DEVREQ_CLEAR_FEATURE == req->request) {
        
        if (USB_DEVREQ_RECIPIENT_INTERFACE == recipient) {
            // The host should expect no data back, the device must
            // be configured, and there are no defined features to clear.
            if ((0 == length) &&
                (USBS_STATE_CONFIGURED == (endpoint->state & USBS_STATE_MASK)) &&
                (0 == req->value_lo)) {

                int interface_id = req->index_lo;
                CYG_ASSERT( 1 == endpoint->enumeration_data->total_number_interfaces, \
                            "Higher level code should have handled this request");

                if (interface_id == endpoint->enumeration_data->interfaces[0].interface_id) {                
                    result = USBS_CONTROL_RETURN_HANDLED;
                } else {
                    result = USBS_CONTROL_RETURN_STALL;
                }
                
            } else {
                result = USBS_CONTROL_RETURN_STALL;
            }
        }
        
    } else if (USB_DEVREQ_GET_CONFIGURATION == req->request) {

        // Return a single byte 0 if the device is not currently
        // configured. Otherwise assume a single configuration
        // in the enumeration data and return its id.
        if ((1 == length) && (USB_DEVREQ_DIRECTION_IN == direction)) {
            
            if (USBS_STATE_CONFIGURED == (endpoint->state & USBS_STATE_MASK)) {
                CYG_ASSERT( 1 == endpoint->enumeration_data->device.number_configurations, \
                            "Higher level code should have handled this request");
                endpoint->control_buffer[0] = endpoint->enumeration_data->configurations[0].configuration_id;
            } else {
                endpoint->control_buffer[0] = 0;
            }
            endpoint->buffer            = endpoint->control_buffer;
            endpoint->buffer_size       = 1;
            endpoint->fill_buffer_fn    = (void (*)(usbs_control_endpoint*)) 0;
            endpoint->complete_fn       = (usbs_control_return (*)(usbs_control_endpoint*, cyg_bool)) 0;
            result = USBS_CONTROL_RETURN_HANDLED;
            
        } else {
            result = USBS_CONTROL_RETURN_STALL;
        }
            
    } else if (USB_DEVREQ_GET_DESCRIPTOR == req->request) {

        // The descriptor type is in value_hi. The descriptor index
        // is in value_lo.
        // The hsot must expect at least one byte of data.
        if ((0 == length) || (USB_DEVREQ_DIRECTION_IN != direction)) {
            
            result = USBS_CONTROL_RETURN_STALL;
            
        } else if (USB_DEVREQ_DESCRIPTOR_TYPE_DEVICE == req->value_hi) {

            // The device descriptor is easy, it is a single field in the
            // enumeration data.
            endpoint->buffer            = (unsigned char*) &(endpoint->enumeration_data->device);
            endpoint->fill_buffer_fn    = (void (*)(usbs_control_endpoint*)) 0;
            endpoint->complete_fn       = (usbs_control_return (*)(usbs_control_endpoint*, cyg_bool)) 0;
            if (length < USB_DEVICE_DESCRIPTOR_LENGTH) {
                endpoint->buffer_size = length;
            } else {
                endpoint->buffer_size = USB_DEVICE_DESCRIPTOR_LENGTH;
            }
            result = USBS_CONTROL_RETURN_HANDLED;
            
        } else if (USB_DEVREQ_DESCRIPTOR_TYPE_CONFIGURATION == req->value_hi) {

            // This is where things get messy. We need to supply the
            // specified configuration data, followed by some number of
            // interfaces and endpoints. Plus there are length limits
            // to consider. First check that the specified index is valid.
            if (req->value_lo >= endpoint->enumeration_data->device.number_configurations) {
                result = USBS_CONTROL_RETURN_STALL;
            } else {
                // No such luck. OK, supplying the initial block is easy.
                endpoint->buffer        = (unsigned char*) &(endpoint->enumeration_data->configurations[req->value_lo]);
                endpoint->complete_fn   = (usbs_control_return (*)(usbs_control_endpoint*, cyg_bool)) 0;

                // How much data was actually requested. If only the
                // configuration itself is of interest then there is
                // no need to worry about the rest.
                if (length <= USB_CONFIGURATION_DESCRIPTOR_LENGTH) {
                    endpoint->buffer_size       = length;
                    endpoint->fill_buffer_fn    = (void (*)(usbs_control_endpoint*)) 0;
                } else {
                    int i, j;
                    int start_interface;
                    int start_endpoint;
                    endpoint->buffer_size       = USB_CONFIGURATION_DESCRIPTOR_LENGTH;
                    endpoint->fill_buffer_fn    = &usbs_configuration_descriptor_refill;

                    // The descriptor refill_fn needs to know what next to transfer.
                    // The desired interfaces and endpoints will be contiguous so
                    // we need to keep track of the following:
                    // 1) the current interface index being transferred.
                    // 2) the last interface that should be transferred.
                    // 3) the current endpoint index that should be transferred.
                    // 4) the last endpoint index. This marks interface/endpoint transitions.
                    // 5) how much has been transferred to date.
                    // This information can be held in the control_buffer,
                    // with the length field being preserved.
                    start_interface = 0;
                    start_endpoint  = 0;
                    // For all configurations up to the desired one.
                    for (i = 0; i < req->value_lo; i++) {
                        int config_interfaces = endpoint->enumeration_data->configurations[i].number_interfaces;

                        // For all interfaces in this configuration.
                        for (j = 0; j < config_interfaces; j++) {
                            // Add the number of endpoints in this interface to the current count.
                            CYG_ASSERT( (j + start_interface) < endpoint->enumeration_data->total_number_interfaces, \
                                        "Valid interface count in enumeration data");
                            start_endpoint += endpoint->enumeration_data->interfaces[j + start_interface].number_endpoints;
                        }
                        // And update the index for the starting interface.
                        start_interface += config_interfaces;
                    }
                    CYG_ASSERT( start_interface < endpoint->enumeration_data->total_number_interfaces, \
                                "Valid interface count in enumeration data");
                    CYG_ASSERT( ((0 == endpoint->enumeration_data->total_number_endpoints) && (0 == start_endpoint)) || \
                                (start_endpoint < endpoint->enumeration_data->total_number_endpoints), \
                                "Valid endpoint count in enumeration data");

                    req->type           = (unsigned char) start_interface;
                    req->request        = (unsigned char) (start_interface +
                                                           endpoint->enumeration_data->configurations[req->value_lo].number_interfaces
                                                           );
                    req->value_lo       = (unsigned char) start_endpoint;
                    req->value_hi       = (unsigned char) start_endpoint;
                    req->index_lo       = USB_CONFIGURATION_DESCRIPTOR_LENGTH;
                    req->index_hi       = 0;
                }
                result = USBS_CONTROL_RETURN_HANDLED;
            }
            
            
        } else if (USB_DEVREQ_DESCRIPTOR_TYPE_STRING == req->value_hi) {

            // As long as the index is valid, the rest is easy since
            // the strings are just held in a simple array.
            // NOTE: if multiple languages have to be supported
            // then things get more difficult.
            if (req->value_lo >= endpoint->enumeration_data->total_number_strings) {
                result = USBS_CONTROL_RETURN_STALL;
            } else {
                endpoint->buffer                = (unsigned char*) endpoint->enumeration_data->strings[req->value_lo];
                endpoint->fill_buffer_fn        = (void (*)(usbs_control_endpoint*)) 0;
                endpoint->complete_fn           = (usbs_control_return (*)(usbs_control_endpoint*, cyg_bool)) 0;

                if (length < endpoint->buffer[0]) {
                    endpoint->buffer_size = length;
                } else {
                    endpoint->buffer_size = endpoint->buffer[0];
                }
                result = USBS_CONTROL_RETURN_HANDLED;
            }
            
        } else {
            result = USBS_CONTROL_RETURN_STALL;
        }
        
    } else if (USB_DEVREQ_GET_INTERFACE == req->request) {

        if ((1 != length) ||
            (USB_DEVREQ_DIRECTION_IN != direction) ||
            (USBS_STATE_CONFIGURED != (endpoint->state & USBS_STATE_MASK))) {
            
            result = USBS_CONTROL_RETURN_STALL;
            
        } else {
            int interface_id;
            
            CYG_ASSERT( (1 == endpoint->enumeration_data->device.number_configurations) && \
                        (1 == endpoint->enumeration_data->total_number_interfaces),       \
                        "Higher level code should have handled this request");

            interface_id = (req->index_hi << 8) | req->index_lo;
            if (interface_id != endpoint->enumeration_data->interfaces[0].interface_id) {
                result = USBS_CONTROL_RETURN_STALL;
            } else {
                endpoint->control_buffer[0] = endpoint->enumeration_data->interfaces[0].alternate_setting;
                endpoint->buffer            = endpoint->control_buffer;
                endpoint->buffer_size       = 1;
                endpoint->fill_buffer_fn    = (void (*)(usbs_control_endpoint*)) 0;
                endpoint->complete_fn       = (usbs_control_return (*)(usbs_control_endpoint*, cyg_bool)) 0;
                result = USBS_CONTROL_RETURN_HANDLED;
            }
        }
        
    } else if (USB_DEVREQ_GET_STATUS == req->request) {

        if (USB_DEVREQ_RECIPIENT_INTERFACE == recipient) {
            // The host should expect two bytes back, the device must
            // be configured, the interface number must be valid.
            // The host should expect no data back, the device must
            // be configured, and there are no defined features to clear.
            if ((2 == length) &&
                (USB_DEVREQ_DIRECTION_IN == direction) &&
                (USBS_STATE_CONFIGURED == (endpoint->state & USBS_STATE_MASK))) {

                int interface_id = req->index_lo;
                CYG_ASSERT( 1 == endpoint->enumeration_data->total_number_interfaces, \
                            "Higher level code should have handled this request");

                if (interface_id == endpoint->enumeration_data->interfaces[0].interface_id) {
                    
                    // The request is legit, but there are no defined features for an interface...
                    endpoint->control_buffer[0] = 0;
                    endpoint->control_buffer[1] = 0;
                    endpoint->buffer            = endpoint->control_buffer;
                    endpoint->buffer_size       = 2;
                    endpoint->fill_buffer_fn    = (void (*)(usbs_control_endpoint*)) 0;
                    endpoint->complete_fn       = (usbs_control_return (*)(usbs_control_endpoint*, cyg_bool)) 0;
                    result = USBS_CONTROL_RETURN_HANDLED;
                    
                } else {
                    result = USBS_CONTROL_RETURN_STALL;
                }
            } else {
                result = USBS_CONTROL_RETURN_STALL;
            }
        }
        
    } else if (USB_DEVREQ_SET_CONFIGURATION == req->request) {

        // Changing to configuration 0 means a state change from
        // configured to addressed. Changing to anything else means a
        // state change to configured. Both involve invoking the
        // state change callback. If there are multiple configurations
        // to choose from then this request has to be handled at
        // a higher level. 
        int old_state = endpoint->state;
        if (0 == req->value_lo) {
            endpoint->state = USBS_STATE_ADDRESSED;
            if ((void (*)(usbs_control_endpoint*, void*, usbs_state_change, int))0 != endpoint->state_change_fn) {
                (*endpoint->state_change_fn)(endpoint, endpoint->state_change_data,
                                             USBS_STATE_CHANGE_DECONFIGURED, old_state);
            }
            result = USBS_CONTROL_RETURN_HANDLED;
                
        } else {
            CYG_ASSERT(1 == endpoint->enumeration_data->device.number_configurations, \
                       "Higher level code should have handled this request");
            if (req->value_lo == endpoint->enumeration_data->configurations[0].configuration_id) {
                endpoint->state = USBS_STATE_CONFIGURED;
                if ((void (*)(usbs_control_endpoint*, void*, usbs_state_change, int))0 != endpoint->state_change_fn) {
                    (*endpoint->state_change_fn)(endpoint, endpoint->state_change_data,
                                                 USBS_STATE_CHANGE_CONFIGURED, old_state);
                }
                result = USBS_CONTROL_RETURN_HANDLED;
            } else {
                result = USBS_CONTROL_RETURN_STALL;
            }
        }
        
    } else if (USB_DEVREQ_SET_FEATURE == req->request) {
        
        if (USB_DEVREQ_RECIPIENT_INTERFACE == recipient) {
            // The host should expect no data back, the device must
            // be configured, and there are no defined features to clear.
            if ((0 == length) &&
                (USBS_STATE_CONFIGURED == (endpoint->state & USBS_STATE_MASK)) &&
                (0 == req->value_lo)) {

                int interface_id = req->index_lo;
                CYG_ASSERT( 1 == endpoint->enumeration_data->total_number_interfaces, \
                            "Higher level code should have handled this request");

                if (interface_id == endpoint->enumeration_data->interfaces[0].interface_id) {                
                    result = USBS_CONTROL_RETURN_HANDLED;
                } else {
                    result = USBS_CONTROL_RETURN_STALL;
                }
                
            } else {
                result = USBS_CONTROL_RETURN_STALL;
            }
        }
        
    }
    
    return result;
}
