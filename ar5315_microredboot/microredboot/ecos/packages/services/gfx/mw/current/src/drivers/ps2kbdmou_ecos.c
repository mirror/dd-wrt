//=============================================================================
//
//      ps2kbdmou_ecos.c
//
//      eCos support for a PS/2 keyboard and mouse.
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    bartv
// Date:         2002-04-04
// Purpose:      Implement basic keyboard and mouse support for microwindows
//               only, interacting directly with the hardware.
//
//####DESCRIPTIONEND####
//=============================================================================

#include <pkgconf/system.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/drv_api.h>
#include <cyg/kernel/kapi.h>
#include <microwin/device.h>

#ifdef CYGPKG_KERNEL
# include <cyg/kernel/kapi.h>
#endif

// ----------------------------------------------------------------------------
// Configuration options. For now local to this file.
#define CYGDBG_DEVS_PS2KBDMOUSE_VERBOSE         1

#ifdef CYGDBG_DEVS_PS2KBDMOUSE_VERBOSE
# define DBG(format, args...)       diag_printf(format, ## args)
#else
# define DBG(format, args...)
#endif

// ----------------------------------------------------------------------------
// The hardware.
//
// On PC's with PS/2 hardware both the mouse and the keyboard are
// handled through a single keyboard controller chip. There are four
// registers: status, control, input and output. There are also 8-bit
// input and output ports which can be manipulated by writing certain
// control messages. The registers are accessed via the I/O bus.
//
// Output means keyboard controller -> cpu.
// Input means cpu -> keyboard controller.
//
// So you need to do a HAL_READ_UINT() to the output register,
// and a HAL_WRITE_UINT8 to the input register. They are actually
// at the same address...
//
// The following information was extracted from "The Indispensable
// PC Hardware Book" by Messmer, third edition, chapter 34. 

#define KC_OUTPUT       0x060
#define KC_INPUT        0x060
#define KC_CONTROL      0x064
#define KC_STATUS       0x064

// Bits in the status register.
#define KC_STATUS_PARE  (0x01 << 7)
#define KC_STATUS_TIM   (0x01 << 6)
#define KC_STATUS_AUXB  (0x01 << 5)
#define KC_STATUS_KEYL  (0x01 << 4)
#define KC_STATUS_CD    (0x01 << 3)
#define KC_STATUS_SYSF  (0x01 << 2)
#define KC_STATUS_INPB  (0x01 << 1)
#define KC_STATUS_OUTB  (0x01 << 0)

// Commands that can be written to the control register,
// plus for some the results that would come back.
#define KC_CONTROL_NULL                 -1
#define KC_CONTROL_DISABLE_AUX          0x00A7
#define KC_CONTROL_ENABLE_AUX           0x00A8
#define KC_CONTROL_CHECK_AUX            0x00A9
#define KC_CONTROL_CHECK_AUX_OK             0x000
#define KC_CONTROL_CHECK_AUX_CLOCK_LOW      0x001
#define KC_CONTROL_CHECK_AUX_CLOCK_HIGH     0x002
#define KC_CONTROL_CHECK_AUX_DATA_LOW       0x003
#define KC_CONTROL_CHECK_AUX_DATA_HIGH      0x004
#define KC_CONTROL_CHECK_AUX_NONE           0x0FF
#define KC_CONTROL_SELF_TEST            0x00AA
#define KC_CONTROL_SELF_TEST_OK             0x055
#define KC_CONTROL_CHECK_KBD            0x00AB
#define KC_CONTROL_CHECK_KBD_OK             0x000
#define KC_CONTROL_CHECK_KBD_CLOCK_LOW      0x001
#define KC_CONTROL_CHECK_KBD_CLOCK_HIGH     0x002
#define KC_CONTROL_CHECK_KBD_DATA_LOW       0x003
#define KC_CONTROL_CHECK_KBD_DATA_HIGH      0x004
#define KC_CONTROL_CHECK_KBD_ERROR          0x0FF
#define KC_CONTROL_DISABLE_KBD          0x00AD
#define KC_CONTROL_ENABLE_KBD           0x00AE
#define KC_CONTROL_READ_INPUT_PORT      0x00C0
#define KC_CONTROL_READ_INPUT_PORT_LOW  0x00C1
#define KC_CONTROL_READ_INPUT_PORT_HIGH 0x00C2
#define KC_CONTROL_READ_OUTPUT_PORT     0x00D0
#define KC_CONTROL_WRITE_OUTPUT_PORT    0x00D1
#define KC_CONTROL_WRITE_KBD_OUTPUT     0x00D2
#define KC_CONTROL_WRITE_AUX_OUTPUT     0x00D3
#define KC_CONTROL_WRITE_AUX            0x00D4
#define KC_CONTROL_READ_TEST_INPUT      0x00E0
#define KC_CONTROL_PULSE                0x00F0

// Additional commands, not from the book...
#define KC_CONTROL_READ_MODE            0x0020
#define KC_CONTROL_WRITE_MODE           0x0060
#define KC_MODE_KBD_INT                 (0x01 << 0)
#define KC_MODE_MOU_INT                 (0x01 << 1)
#define KC_MODE_SYS                     (0x01 << 2)
#define KC_MODE_NO_KEYLOCK              (0x01 << 3)
#define KC_MODE_DISABLE_KBD             (0x01 << 4)
#define KC_MODE_ENABLE_KBD              (0x01 << 5)
#define KC_MODE_KCC                     (0x01 << 6)
#define KC_MODE_RFU                     (0x01 << 7)

// The input port
#define KC_INPUT_LOCK       (0x01 << 7)
#define KC_INPUT_CM         (0x01 << 6)
#define KC_INPUT_CM_MONO    (0x01 << 6)
#define KC_INPUT_CM_COLOR   (0x00 << 6)
#define KC_INPUT_CM_COLOUR  (0x00 << 6)
#define KC_INPUT_AUX        (0x01 << 1)
#define KC_INPUT_KBD        (0x01 << 0)

// And the output port
#define KC_OUTPUT_KBDO      (0x01 << 7)
#define KC_OUTPUT_KCLK      (0x01 << 6)
#define KC_OUTPUT_AUXB      (0x01 << 5)
#define KC_OUTPUT_OUTB      (0x01 << 4)
#define KC_OUTPUT_ACLK      (0x01 << 3)
#define KC_OUTPUT_AXDO      (0x01 << 2)
#define KC_OUTPUT_GA20      (0x01 << 1)
#define KC_OUTPUT_SYSR      (0x01 << 0)

// Data from the keyboard
#define KC_KBD_OVERFLOW     0x000
#define KC_KBD_KEY_ERROR    0x0FF
#define KC_KBD_MFII_ID      0x0041AB
#define KC_KBD_BAT_COMPLETE 0x0AA
#define KC_KBD_ECHO         0x0EE
#define KC_KBD_ACK          0x0FA
#define KC_KBD_BAT_ERROR    0x0FC
#define KC_KBD_RESEND       0x0FE
#define KC_KBD_SCANCODE_MIN 0x001
// Likely to be incorrect for some modern keyboards
#define KC_KBD_SCANCODE_MAX 0x058

// Commands that can be sent to the keyboard. These
// are just written to the input register. Some of
// them will be followed by additional data.
#define KC_KBDC_LED_ONOFF           0x0ED
#define KC_KBDC_ECHO                0x0EE
#define KC_KBDC_SETSCAN             0x0F0
#define KC_KBDC_IDENTIFY            0x0F2
#define KC_KBDC_SETREPEAT           0x0F3
#define KC_KBDC_ENABLE              0x0F4
#define KC_KBDC_STANDARD_DISABLE    0x0F5
#define KC_KBDC_STANDARD_ENABLE     0x0F6
#define KC_KBDC_RESEND              0x0FE
#define KC_KBDC_RESET               0x0FF

// And commands that can be sent to the mouse. These
// involve a controller write followed by another
// write to the input register.
#define KC_MOUSEC_RESET_SCALING     0x0E6
#define KC_MOUSEC_SET_SCALING       0x0E7
#define KC_MOUSEC_SET_RESOLUTION    0x0E8
#define KC_MOUSEC_STATUS            0x0E9
#define KC_MOUSEC_SET_STREAM_MODE   0x0EA
#define KC_MOUSEC_READ_DATA         0x0EB
#define KC_MOUSEC_RESET_WRAP_MODE   0x0EC
#define KC_MOUSEC_SET_WRAP_MODE     0x0EE
#define KC_MOUSEC_SET_REMOTE_MODE   0x0F0
#define KC_MOUSEC_IDENTIFY          0x0F2
#define KC_MOUSEC_SET_SAMPLE_RATE   0x0F3
#define KC_MOUSEC_ENABLE            0x0F4
#define KC_MOUSEC_DISABLE           0x0F5
#define KC_MOUSEC_SET_STANDARD      0x0F6
#define KC_MOUSEC_RESEND            0x0FE
#define KC_MOUSEC_RESET             0x0FF

// Data back from the mouse. Some special characters.
#define KC_MOUSE_ACK                0x0FA
#define KC_MOUSE_RESEND             0x0FE

// ----------------------------------------------------------------------------
// The low-level stuff. Managing the PS/2 hardware is actually quite
// messy if you want a robust implementation because of the various
// ack's, resend requests, etc.

// The keyboard device. The interrupt handler is responsible for storing
// key press and release events in a circular buffer. The poll and read code
// will then try to convert these events into something closer to what
// microwindows expects. There is an assumption that the poll() and read()
// code will be called often enough that there is no risk of overflow.

// A circular buffer of scancodes.
#define PS2KBD_SCANCODE_BUFSIZE    64
static unsigned char    ps2kbd_scancode_buffer[PS2KBD_SCANCODE_BUFSIZE];
static volatile int     ps2kbd_scancode_buffer_head = 0;    // new data written here
static volatile int     ps2kbd_scancode_buffer_tail = 0;    // old data extracted from here

// The current mouse state. Just maintain the current X and Y deltas,
// button state,, and a delta flag. The hardware will generate
// eight-byte mouse data packets, and when a complete packet has been
// received the interrupt handler will update the values and set the
// delta flag.

#define PS2MOU_DATA_BUFSIZE 12
static MWCOORD          ps2mou_dx       = 0;
static MWCOORD          ps2mou_dy       = 0;
static int              ps2mou_buttons  = 0;
static volatile int     ps2mou_changed  = 0;

static unsigned char    ps2mou_buffer[PS2MOU_DATA_BUFSIZE];
static int              ps2mou_buffer_index = 0;

// Sending commands. In theory there are a number of variations of
// these.
//
// 1) commands to be sent directly to the controller. The control byte
//    goes to KC_CONTROL, and any additional bytes go to KC_INPUT.
//    The hardware will either ACK the additional bytes or request
//    a resend.  Any replies can be read from KC_OUTPUT, and errors
//    are possible.
//
//    For replies, it is not clear how to distinguish between keyboard
//    events that happen at just the wrong moment and the reply data.
//
// 2) commands for the keyboard. These just get written directly to
//    the input buffer, one character at a time with ACKs or resends
//    in between.
//
// 3) commands for the mouse. These involve a write of 0xD4 to the
//    control port followed by a write to the input buffer. The latter
//    results in ACKs or resends.

static unsigned char*   ps2_command             = NULL;
static int              ps2_command_mouse       = 0;
static int              ps2_command_index       = 0;
static int              ps2_command_length      = 0;
static volatile int     ps2_command_ack         = 0;
static int              ps2_command_mouse_waiting_for_ack   = 0;

// ----------------------------------------------------------------------------
// Decoding of mouse packets. There are lots of different rodent or
// rodent-like devices out there, all implementing subtly different
// protocols. A general-purpose solution would try to cope with all
// of them. The eCos approach would be to allow just one to be
// configured statically.

// Support for Synaptics touchpads and compatible. This assumes
// default relative format. Byte 0 contains various flags and
// the button state. Byte 1 contains X-offset, byte 2 contains
// the y-offset.

static int              ps2mou_packet_size  = 3;
static void
ps2mou_synaptics_translate(void)
{
    int new_buttons = 0;
    int dx, dy;
    
    // The packet consists of six bytes. Bit 3 of the first packet
    // should be set. If that condition is not satisfied then we
    // are in trouble and we may need to perform some sort of reset.
    if (0 == (ps2mou_buffer[0] & 0x08)) {
        // FIXME: perform some sort of reset to get the world
        // back in sync.
        return;
    }
    // Byte 0 holds the button flags.
    if (0 != (ps2mou_buffer[0] & (0x01 << 0))) {
        new_buttons = MWBUTTON_L;
    }
    if (0 != (ps2mou_buffer[0] & (0x01 << 1))) {
        new_buttons |= MWBUTTON_R;
    }
    ps2mou_buttons  = new_buttons;

    dx = ps2mou_buffer[1];
    if (0 != (ps2mou_buffer[0] & (0x001 << 4))) {
        // Negative number.
        if (0 != (ps2mou_buffer[0] & (0x01 << 6))) {
            // -ve overflow
            dx = -256;
        } else {
            dx = 0 - (256 - dx);
        }
    } else if (0 != (ps2mou_buffer[0] & (0x01 << 6))) {
        // +ve overflow
        dx = 256;
    }
    ps2mou_dx += dx;
    
    dy = ps2mou_buffer[2];
    if (0 != (ps2mou_buffer[0] & (0x01 << 5))) {
        // Negative number.
        if (0 != (ps2mou_buffer[0] & (0x01 << 7))) {
            // -ve overflow
            dy = -256;
        } else {
            // -ve signed, bottom byte only
            dy = 0 - (256 - dy);
        }
    } else if (0 != (ps2mou_buffer[0] & (0x01 << 7))) {
        // +ve overflow
        dy = 256;
    }
    ps2mou_dy += dy;
    
    ps2mou_changed      = 1;
}

// Mouse data. A PS/2 mouse sends events in the form of
// eight-byte packets. Some of the fields are officially
// reserved and ignored for now.
#define KC_MOUSE_DATA_FLAGS         0x00
#define KC_MOUSE_DATA_FLAGS_YOV     (0x01 << 7)
#define KC_MOUSE_DATA_FLAGS_XOV     (0x01 << 6)
#define KC_MOUSE_DATA_FLAGS_YNG     (0x01 << 5)
#define KC_MOUSE_DATA_FLAGS_XNG     (0x01 << 4)
#define KC_MOUSE_DATA_FLAGS_RIG     (0x01 << 1)
#define KC_MOUSE_DATA_FLAGS_LEF     (0x01 << 0)
#define KC_MOUSE_DATA_X             0x02
#define KC_MOUSE_DATA_Y             0x04
#define KC_MOUSE_DATA_SIZE          0x08

// ----------------------------------------------------------------------------
// An interrupt has occurred. Usually this means that there is data
// in the output register, although errors are possible as well. The
// data can be keyboard scancodes, parts of a mouse packet, or
// replies to control messages.
//
// For now errors are ignored, including parity and timeout errors. In
// theory these are supposed to be handled by requesting a resend. In
// practice that seems to cause as many complications as it might
// solve. For example what should happen if there is already a command
// being sent?
//
// The controller interrupts at two separate vectors, one for keyboard
// and another for mouse. If nested interrupts are enabled this could
// cause problems with nested calls to ps2_isr() updating the global
// data in the wrong order. It may be necessary to have a volatile flag
// to detect nesting, accompanied by an early acknowledge and return to
// the interrupted interrupt handler.

static cyg_uint32
ps2_isr(cyg_vector_t isr_vector, cyg_addrword_t isr_data)
{
    int             status;
    unsigned char   data;

    CYG_UNUSED_PARAM(cyg_addrword_t, isr_data);

    HAL_READ_UINT8(KC_STATUS, status);
    while (status & KC_STATUS_OUTB) {
        HAL_READ_UINT8(KC_OUTPUT, data);
        
        if (status & KC_STATUS_AUXB) {
            // Data from the mouse. This will be either an ACK for a
            // command, a resend request, or a byte for the current
            // packet. When a complete 8-byte packet has been received
            // it can be processed. When an ACK is received the next
            // byte for the current command should get sent, or on
            // completion the sending code can be woken up.
            //
            // The mouse can also send back other data, e.g. in response
            // to a determine-status request. These are disallowed
            // because there is no obvious way of separating out such
            // data from a current mouse packet being transferred.
            //
            // There may also be special bytes sent for disconnect and
            // reconnect. It is not clear how to distinguish those
            // from packet data either.
            if (ps2_command_mouse_waiting_for_ack && ((KC_MOUSE_ACK == data) || (KC_MOUSE_RESEND == data))) {
                int tmp;
                
                if (KC_MOUSE_ACK == data) {
                    // Is there another byte to be sent?
                    ps2_command_index++;
                    if (ps2_command_index < ps2_command_length) {
                        // Send the next byte for the current command
                        do {
                            HAL_READ_UINT8(KC_STATUS, tmp);
                        } while (tmp & KC_STATUS_INPB);
                        HAL_WRITE_UINT8(KC_CONTROL, KC_CONTROL_WRITE_AUX);
                        do {
                            HAL_READ_UINT8(KC_STATUS, tmp);
                        } while (tmp & KC_STATUS_INPB);
                        HAL_WRITE_UINT8(KC_INPUT, ps2_command[ps2_command_index]);
                    } else {
                        // The whole command has been sent and acknowledged.
                        // Allow the polling thread to resume.
                        ps2_command_index   = 0;
                        ps2_command_length  = 0;
                        ps2_command         = NULL;
                        ps2_command_ack     = 1;
                        ps2_command_mouse_waiting_for_ack = 0;
                    }
                } else {
                    // A resend request for the current byte.
                    do {
                        HAL_READ_UINT8(KC_STATUS, tmp);
                    } while (tmp & KC_STATUS_INPB);
                    HAL_WRITE_UINT8(KC_CONTROL, KC_CONTROL_WRITE_AUX);
                    do {
                        HAL_READ_UINT8(KC_STATUS, tmp);
                    } while (tmp & KC_STATUS_INPB);
                    HAL_WRITE_UINT8(KC_INPUT, ps2_command[ps2_command_index]);
                }
            } else {
                ps2mou_buffer[ps2mou_buffer_index++] = data;
                if (ps2mou_packet_size == ps2mou_buffer_index) {
                    // A complete packet has been received.
                    ps2mou_synaptics_translate();
                    ps2mou_buffer_index = 0;    // Ready for the next packet
                }
            }
        } else {
            // Data from the keyboard. Usually this will be a scancode.
            // There are a number of other possibilities such as
            // echo replies, resend requests, and acks.
            if ((KC_KBD_ACK == data) && (NULL != ps2_command) && !ps2_command_mouse_waiting_for_ack) {
                // Send the next byte for the current command, or
                // else we have completed.
                ps2_command_index++;
                if (ps2_command_index < ps2_command_length) {
                    int tmp;
                    do {
                        HAL_READ_UINT8(KC_STATUS, tmp);
                    } while (tmp & KC_STATUS_INPB);
                    HAL_WRITE_UINT8(KC_INPUT, ps2_command[ps2_command_index]);
                } else {
                    ps2_command_index   = 0;
                    ps2_command_length  = 0;
                    ps2_command         = NULL;
                    ps2_command_ack     = 1;
                }
            } else if ((KC_KBD_RESEND == data) && (NULL != ps2_command) && !ps2_command_mouse_waiting_for_ack) {
                int tmp;
                do {
                    HAL_READ_UINT8(KC_STATUS, tmp);
                } while (tmp & KC_STATUS_INPB);
                HAL_WRITE_UINT8(KC_INPUT, ps2_command[ps2_command_index]);
            } else {
                if (((ps2kbd_scancode_buffer_head + 1) % PS2KBD_SCANCODE_BUFSIZE) == ps2kbd_scancode_buffer_tail) {
                    // Already full. The data has to be discarded.
                } else {
                    ps2kbd_scancode_buffer[ps2kbd_scancode_buffer_head] = data;
                    ps2kbd_scancode_buffer_head = (ps2kbd_scancode_buffer_head + 1) % PS2KBD_SCANCODE_BUFSIZE;
                }
            }
        }

        // Just in case the keyboard controller is fast enough to send another byte,
        // go around again.
        HAL_READ_UINT8(KC_STATUS, status);
    }

    // The interrupt has been fully handled. For now there is no point
    // in running a DSR.
    cyg_drv_interrupt_acknowledge(isr_vector);
    return CYG_ISR_HANDLED;
}

// For now the DSR does nothing. 
static void
ps2_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    CYG_UNUSED_PARAM(cyg_vector_t, vector);
    CYG_UNUSED_PARAM(cyg_ucount32, count);
    CYG_UNUSED_PARAM(cyg_addrword_t, data);
}


// Sending out a command. The controller command, if any, gets sent here.
// This is followed by the first byte for the keyboard or mouse. The
// remaining bytes and any retransmits will be handled by the interrupt
// handler.
static void
ps2_send_command(int controller_command, unsigned char* command, int length, int mouse)
{
    int     status;
    
    CYG_PRECONDITION(NULL == ps2_command, "Only one send command is allowed at a time");
    CYG_PRECONDITION((KC_CONTROL_NULL != controller_command) || (NULL != command), "no-op");
    CYG_PRECONDITION(!mouse || (KC_CONTROL_NULL == controller_command), "cannot combine controller and mouse commands");
    
    ps2_command         = command;
    ps2_command_index   = 0;
    ps2_command_length  = length;
    ps2_command_mouse   = 0;
    ps2_command_ack     = 0;

    if (KC_CONTROL_NULL != controller_command) {
        do {
            HAL_READ_UINT8(KC_STATUS, status);
        } while (status & KC_STATUS_INPB);
        HAL_WRITE_UINT8(KC_CONTROL, controller_command);
    }

    if (length > 0) {
        if (mouse) {
            do {
                HAL_READ_UINT8(KC_STATUS, status);
            } while (status & KC_STATUS_INPB);
            HAL_WRITE_UINT8(KC_CONTROL, KC_CONTROL_WRITE_AUX);
        }
        do {
            HAL_READ_UINT8(KC_STATUS, status);
        } while (status & KC_STATUS_INPB);
        HAL_WRITE_UINT8(KC_INPUT, command[0]);
    }
}

// For now there is little difference between polled and non-polled
// mode, they both just spin until the ACK byte is received. The
// polled version just calls the interrupt handler as well. This is
// probably acceptable for now because commands only get sent during
// initialization, but eventually the non-polled version should be
// using a synch primitive signalled by the dsr.
//
// ACKs are only generated when there is data to be sent, not for
// operations on the control register.
//
// For keyboard commands there is no real problem because the ACK
// character 0xFA does not match a valid scancode. For the mouse
// things are more difficult because 0xFA could be present in the
// data, e.g. as a fairly large movement. Therefore the interrupt
// handler needs to know whether or not a mouse ACK is expected.
// It is assumed that the ACK and any actual 0xFA data get sent
// within a byte of each other so that the 0xFA still ends up in
// the right place in the buffer.
//
// A couple of commands do not result in an ACK. For the mouse this
// includes reset-wrap-mode and reset. For the keyboard this includes
// echo. These commands are not currently used, so there is no need
// to worry about the special cases.

static void
ps2_send_command_poll(int controller_command, unsigned char* command, int length, int mouse)
{
    if ((NULL != command) && mouse) {
        ps2_command_mouse_waiting_for_ack = 1;
    }
    ps2_send_command(controller_command, command, length, mouse);
    if (NULL != command) {
        for ( ; !ps2_command_ack; ) {
            ps2_isr( CYGNUM_HAL_INTERRUPT_KEYBOARD, 0);
        }
        ps2_command_ack = 0;
    }
}

static void
ps2_send_command_wait(int controller_command, unsigned char* command, int length, int mouse)
{
    if ((NULL != command) && mouse) {
        ps2_command_mouse_waiting_for_ack = 1;
    }
    ps2_send_command(controller_command, command, length, mouse);
    if (NULL != command) {
        for ( ; !ps2_command_ack; )
            ;
        ps2_command_ack = 0;
    }
}


// ----------------------------------------------------------------------------
// Hardware initialization and the interrupt handling.

static cyg_handle_t     ps2kbd_interrupt_handle;
static cyg_interrupt    ps2kbd_interrupt_data;
static cyg_handle_t     ps2mouse_interrupt_handle;
static cyg_interrupt    ps2mouse_interrupt_data;

static void
ps2_initialize(void)
{
    unsigned char   buf[2];
    int             status, data;
    
    // Only perform initialization once, not for both kbd and mouse.
    static int  initialized = 0;
    if (initialized) {
        return;
    }
    initialized++;

    // Start by masking out the interrupts. Other code such as the HAL
    // or RedBoot may think it currently owns the keyboard, and I do
    // not want any interference from them while setting up the
    // interrupt handlers.
    cyg_drv_interrupt_mask_intunsafe(CYGNUM_HAL_INTERRUPT_KEYBOARD);
    cyg_drv_interrupt_mask_intunsafe(CYGNUM_HAL_INTERRUPT_IRQ12);

    // Install my own interrupt handler, overwriting anything that might
    // be there already.
    cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_KEYBOARD, 0, 0,
                             &ps2_isr, &ps2_dsr, &ps2kbd_interrupt_handle, &ps2kbd_interrupt_data);
    cyg_drv_interrupt_attach(ps2kbd_interrupt_handle);
    cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_IRQ12, 0, 0,
                             &ps2_isr, &ps2_dsr, &ps2mouse_interrupt_handle, &ps2mouse_interrupt_data);
    cyg_drv_interrupt_attach(ps2mouse_interrupt_handle);

    // Get the device into a known state.
    ps2_send_command(KC_CONTROL_DISABLE_AUX, NULL, 0, 0);
    ps2_send_command(KC_CONTROL_DISABLE_KBD, NULL, 0, 0);
    
    // Discard any current data.
    HAL_READ_UINT8(KC_STATUS, status);
    while (status & KC_STATUS_OUTB) {
        HAL_READ_UINT8(KC_OUTPUT, data);
        HAL_READ_UINT8(KC_STATUS, status);
    }

    // Now restart and reset the keyboard.
    ps2_send_command(KC_CONTROL_ENABLE_KBD, NULL, 0, 0);
    buf[0] = KC_KBDC_STANDARD_DISABLE;
    ps2_send_command_poll(KC_CONTROL_NULL, buf, 1, 0);
    buf[0] = KC_KBDC_LED_ONOFF;
    buf[1] = 0;
    ps2_send_command_poll(KC_CONTROL_NULL, buf, 2, 0);
    // The keyboard-specific initialization can now reenable the keyboard
    // using enable

    // Similarly for a mouse
    // Standard mode means 100 samples/s, 1:1 scaling, stream mode, 4 counts/mm resolution,
    // and transferdisabled. Stream mode is important, that means mouse data will
    // be immediately available rather than requiring separate control messages to
    // read a packet.
    ps2_send_command(KC_CONTROL_ENABLE_AUX, NULL, 0, 0);
    buf[0] = KC_MOUSEC_SET_STANDARD;
    ps2_send_command_poll(KC_CONTROL_NULL, buf, 1, 1);
    buf[0] = KC_MOUSEC_SET_SAMPLE_RATE;
    buf[1] = 0x0A;
    ps2_send_command_poll(KC_CONTROL_NULL, buf, 2, 1);

    // WRITE_MODE does not appear to involve an ACK
    ps2_send_command_poll(KC_CONTROL_WRITE_MODE, NULL, 0, 0);
    do {
        HAL_READ_UINT8(KC_STATUS, status);
    } while (status & KC_STATUS_INPB);
    HAL_WRITE_UINT8(KC_INPUT, KC_MODE_KBD_INT | KC_MODE_MOU_INT | KC_MODE_SYS | KC_MODE_KCC);
    
    cyg_drv_interrupt_unmask_intunsafe(CYGNUM_HAL_INTERRUPT_KEYBOARD);
    cyg_drv_interrupt_unmask_intunsafe(CYGNUM_HAL_INTERRUPT_IRQ12);
}

// ----------------------------------------------------------------------------
static int
PS2Mouse_Open(MOUSEDEVICE* pmd)
{
    unsigned char buf[1];
    ps2_initialize();
    buf[0] = KC_MOUSEC_ENABLE;
    ps2_send_command_wait(KC_CONTROL_NULL, buf, 1, 1);
}

// Closing the mouse is equivalent to disabling. It is assumed that we
// are not in the middle of a packet transfer, which could really
// confuse things.
static void
PS2Mouse_Close(void)
{
    unsigned char buf[1];
    buf[0] = KC_MOUSEC_SET_STANDARD;
    ps2_send_command_wait(KC_CONTROL_NULL, buf, 1, 1);
    ps2mou_buffer_index = 0;
}

static int
PS2Mouse_GetButtonInfo(void)
{
    return MWBUTTON_L | MWBUTTON_R;
}

// The mouse is used in its default setup, which means 1:1 scaling.
// Setting the threshold to 5 seems to match most other drivers...
static void
PS2Mouse_GetDefaultAccel(int* pscale, int* pthresh)
{
    if (NULL != pscale) {
        *pscale = 1;
    }
    if (NULL != pthresh) {
        *pthresh = 5;
    }
}

static int
PS2Mouse_Read(MWCOORD* dx_arg, MWCOORD* dy_arg, MWCOORD* dz_arg, int* bp_arg)
{
    int         result  = 0;
    MWCOORD     dx      = 0;
    MWCOORD     dy      = 0;
    int         buttons = 0;

    cyg_drv_isr_lock();
    if (ps2mou_changed) {
        dx              = ps2mou_dx;
        dy              = 0 - ps2mou_dy;    // microwindows directions are the opposite from the hardware
        buttons         = ps2mou_buttons;
        ps2mou_dx       = 0;
        ps2mou_dy       = 0;
        ps2mou_changed  = 0;
        result = 1;
    }
    cyg_drv_isr_unlock();
    if (result) {
        if (NULL != dx_arg) {
            *dx_arg = dx;
        }
        if (NULL != dy_arg) {
            *dy_arg = dy;
        }
        if (NULL != dz_arg) {
            *dz_arg = 0;
        }
        if (NULL != bp_arg) {
            *bp_arg = buttons;
        }
    }
    return result;
}

static int
PS2Mouse_Poll(void)
{
    return 0 != ps2mou_changed;
}

MOUSEDEVICE mousedev = {
    PS2Mouse_Open,
    PS2Mouse_Close,
    PS2Mouse_GetButtonInfo,
    PS2Mouse_GetDefaultAccel,
    PS2Mouse_Read,
    PS2Mouse_Poll
};

// ----------------------------------------------------------------------------
// Extracting data from the scancode buffer and turning it into
// ASCII. This is only called from thread context by the poll() and
// read() routines, although the scancode buffer is updated in ISR
// context.

// The current keyboard event, if any. This involves processing
// the scancodes held in the circular buffer and translating them
// to ASCII.
static MWKEY        ps2kbd_current_key          = MWKEY_UNKNOWN;
static MWKEYMOD     ps2kbd_current_modifiers    = 0;
static MWSCANCODE   ps2kbd_current_scancode     = 0;
static int          ps2kbd_current_keydown      = 0;

// Translation between scancodes and characters, i.e. keymaps.
// For now a relatively simple approach is used. The keymaps
// only cope with shifted vs. unshifted. The control key
// is handled specially. Anything cleverer is left to microwindows.
//
// Microwindows excepts key events in the form of MWKEY's, which
// are unsigned shorts. Special keys such as the function keys
// have suitable #define's in mwtypes.h.
//
// There is a complication with PC keyboards in that some keys
// generate multi-byte sequences, usually starting with 0xE0

typedef struct ps2kbd_keymap_entry {
    MWKEY       normal;
    MWKEY       shifted;
} ps2kbd_keymap_entry;

// This keymap is for a Dell Inspiron laptop with a Japanese
// keyboard. It may not match exactly with other keyboards,
// but is probably good enough for now.

static const ps2kbd_keymap_entry ps2kbd_keymap[] = {
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },            // Scancode 0 unused
    { MWKEY_ESCAPE,     MWKEY_ESCAPE },
    { '1',              '!' },
    { '2',              '"' },
    { '3',              '#' },
    { '4',              '$' },
    { '5',              '%' },
    { '6',              '&' },
    { '7',              '\'' },
    { '8',              '(' },
    { '9',              ')' },                      // Scancode 10
    { '0',              '~' },
    { '-',              '=' },
    { '^',              '_' },
    { MWKEY_BACKSPACE,  MWKEY_BACKSPACE },
    { MWKEY_TAB,        MWKEY_TAB },
    { 'q',              'Q' },                      // 0x10
    { 'w',              'W' },
    { 'e',              'E' },
    { 'r',              'R' },
    { 't',              'T' },                      // 20
    { 'y',              'Y' },
    { 'u',              'U' },
    { 'i',              'I' },
    { 'o',              'O' },
    { 'p',              'P' },
    { '@',              '`' },
    { '[',              '{' },
    { MWKEY_ENTER,      MWKEY_ENTER },
    { MWKEY_LCTRL,      MWKEY_LCTRL },
    { 'a',              'A' },                      // 30
    { 's',              'S' },
    { 'd',              'D' },                      // 0x20
    { 'f',              'F' },
    { 'g',              'G' },
    { 'h',              'H' },
    { 'j',              'J' },
    { 'k',              'K' },
    { 'l',              'L' },
    { ';',              '+' },
    { ':',              '*' },                      // 40
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },            // Japanese-only, top-left below escape
    { MWKEY_LSHIFT,     MWKEY_LSHIFT },
    { ']',              '}' },
    { 'z',              'Z' },
    { 'x',              'X' },
    { 'c',              'C' },
    { 'v',              'V' },
    { 'b',              'B' },                      // 0x30
    { 'n',              'N' },
    { 'm',              'M' },                      // 50
    { ',',              '<' },
    { '.',              '>' },
    { '/',              '?' },
    { MWKEY_RSHIFT,     MWKEY_RSHIFT },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },            // unused ?
    { MWKEY_LALT,       MWKEY_LALT },
    { ' ',              ' ' },                      // space bar
    { MWKEY_CAPSLOCK,   MWKEY_CAPSLOCK },
    { MWKEY_F1,         MWKEY_F1 },
    { MWKEY_F2,         MWKEY_F2 },                 // 60
    { MWKEY_F3,         MWKEY_F3 },
    { MWKEY_F4,         MWKEY_F4 },
    { MWKEY_F5,         MWKEY_F5 },
    { MWKEY_F6,         MWKEY_F6 },                 // 0x40
    { MWKEY_F7,         MWKEY_F7 },
    { MWKEY_F8,         MWKEY_F8 },
    { MWKEY_F9,         MWKEY_F9 },
    { MWKEY_F10,        MWKEY_F10 },
    { MWKEY_NUMLOCK,    MWKEY_NUMLOCK },
    { MWKEY_SCROLLOCK,  MWKEY_SCROLLOCK },          // 70
    { MWKEY_KP7,        MWKEY_KP7 },                // Keypad, not actually present on laptop
    { MWKEY_KP8,        MWKEY_KP8 },                // But accessible via Fn and some centre keys
    { MWKEY_KP9,        MWKEY_KP9 },    
    { MWKEY_KP_MINUS,   MWKEY_KP_MINUS },
    { MWKEY_KP4,        MWKEY_KP4 },
    { MWKEY_KP5,        MWKEY_KP5 },
    { MWKEY_KP6,        MWKEY_KP6 },
    { MWKEY_KP_PLUS,    MWKEY_KP_PLUS },
    { MWKEY_KP1,        MWKEY_KP1 },
    { MWKEY_KP2,        MWKEY_KP2 },                // 80, 0x50
    { MWKEY_KP3,        MWKEY_KP3 },
    { MWKEY_KP0,        MWKEY_KP0 },
    { MWKEY_KP_PERIOD,  MWKEY_KP_PERIOD },
    // There are now big gaps 
    // F11 and F12 are 0x57 and 0x58
    // 0x70, 0x79 and 0x7b are Japanese compose keys.
    // 0x73 is backslash and another _
    // 0x7d is yen and pipe.
    // These could be handled by special-case code in the scancode
    // translation routine, but at 4 bytes per table entry
    // it is probably just as efficient to extend the table.
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },            // 84, 0x53
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_F11,        MWKEY_F11 },
    { MWKEY_F12,        MWKEY_F12 },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },            // 90
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },            // 0x60
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },            // 100
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },            // 110
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },            // 0x70
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { '\\',             '_' },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },            // 120
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    '|' },                      // 125
    // In theory there could also be 126 and 127, but those are unused.
    // But putting them into the table avoids a special case test in the code,
    // for a cost of only eight bytes.
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
    { MWKEY_UNKNOWN,    MWKEY_UNKNOWN },
};

// Now for the E0 sequences. The standard ones are in the range 0x47 to 0x53
// 0xE0, 0x5B is the Left Windows key. 0x5C would normally be the right one.
// 0xE0, 0x5D is the Menu key. The 
// The Dell has some additional ones for the dvd player, ignored.
// PrintScreen generates a simple sequence 0xE0, 0x2A, 0xE0, 0x37, 0xE0, 0xB7, 0xE0, 0xAA
// Break generates 0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5

#define PS2KBD_E0_MIN   0x47
static const MWKEY ps2kbd_e0_map[] = {
    MWKEY_HOME,         // 0x47
    MWKEY_UP,           // 0x48
    MWKEY_PAGEUP,       // 0x49
    MWKEY_UNKNOWN,
    MWKEY_LEFT,         // 0x4B
    MWKEY_UNKNOWN,
    MWKEY_RIGHT,        // 0x4D
    MWKEY_UNKNOWN,
    MWKEY_END,          // 0x4F
    MWKEY_DOWN,         // 0x50
    MWKEY_PAGEDOWN,     // 0x51
    MWKEY_INSERT,       // 0x52
    MWKEY_DELETE,       // 0x53
    MWKEY_UNKNOWN,
    MWKEY_UNKNOWN,
    MWKEY_UNKNOWN,
    MWKEY_UNKNOWN,
    MWKEY_UNKNOWN,
    MWKEY_UNKNOWN,
    MWKEY_UNKNOWN,
    MWKEY_LMETA,        // 0x5B
    MWKEY_RMETA,        // 0x5C
    MWKEY_MENU          // 0x5D
};
#define PS2KBD_E0_MAX   0x5D

// Modifier translations. All modifiers are supported by
// this table, even though the keyboard may not actually
// have all of them.
typedef struct ps2kbd_modifier_entry {
    MWKEY       key;
    MWKEYMOD    modifier;
    int         toggle;
} ps2kbd_modifier_entry;

static const ps2kbd_modifier_entry ps2kbd_modifier_map[] = {
    { MWKEY_LSHIFT,     MWKMOD_LSHIFT,  0 },
    { MWKEY_RSHIFT,     MWKMOD_RSHIFT,  0 },
    { MWKEY_LCTRL,      MWKMOD_LCTRL,   0 },
    { MWKEY_RCTRL,      MWKMOD_RCTRL,   0 },
    { MWKEY_LALT,       MWKMOD_LALT,    0 },
    { MWKEY_RALT,       MWKMOD_RALT,    0 },
    { MWKEY_LMETA,      MWKMOD_LMETA,   0 },
    { MWKEY_RMETA,      MWKMOD_RMETA,   0 },
    { MWKEY_NUMLOCK,    MWKMOD_NUM,     1 },
    { MWKEY_CAPSLOCK,   MWKMOD_CAPS,    1 },
    { MWKEY_ALTGR,      MWKMOD_ALTGR,   0 },
    { MWKEY_SCROLLOCK,  MWKMOD_SCR,     1 },
    { MWKEY_UNKNOWN,    MWKMOD_NONE }
};

static void
ps2kbd_process_scancodes(void)
{
    static int      e0_seen             = 0;
    static MWKEY    pending_up          = MWKEY_UNKNOWN;
    static int      pending_scancode    = 0;
    static int      discard             = 0;
    int scancode;
    int i;
    
    CYG_PRECONDITION(MWKEY_UNKNOWN == ps2kbd_current_key, "There should be no pending key");

    if (MWKEY_UNKNOWN != pending_up) {
        ps2kbd_current_key          = pending_up;
        ps2kbd_current_scancode     = pending_scancode;
        ps2kbd_current_keydown      = 0;
        pending_up                  = MWKEY_UNKNOWN;
        return;
    }
    
    while (MWKEY_UNKNOWN == ps2kbd_current_key) {
        // The ISR will manipulate the scancode buffer directly, so
        // interrupts have to be disabled temporarily.
        scancode = -1;
        cyg_drv_isr_lock();
        if (ps2kbd_scancode_buffer_head != ps2kbd_scancode_buffer_tail) {
            scancode = ps2kbd_scancode_buffer[ps2kbd_scancode_buffer_tail];
            ps2kbd_scancode_buffer_tail = (ps2kbd_scancode_buffer_tail + 1) % PS2KBD_SCANCODE_BUFSIZE;
        }
        cyg_drv_isr_unlock();

        if (scancode == -1) {
            // No more data to be processed.
            break;
        }

        // Are we in one of the special sequences, where bytes should be
        // discarded?
        if (discard > 0) {
            discard -= 1;
            continue;
        }

        // A real scancode has been extracted. Are we in an E0 sequence?
        if (e0_seen) {
            e0_seen = 0;
            ps2kbd_current_keydown = (0 == (scancode & 0x0080));
            scancode &= 0x007F;
            if ((scancode >= PS2KBD_E0_MIN) && (scancode <= PS2KBD_E0_MAX)) {
                ps2kbd_current_key = ps2kbd_e0_map[scancode - PS2KBD_E0_MIN];
                ps2kbd_current_scancode = 0x80 + scancode - PS2KBD_E0_MIN;  // Invent a key scancode
            }
            // We may or may not have a valid keycode at this time, so go
            // around the loop again to check for MWKEY_UNKNOWN
            continue;
        }

        // Is this the start of an E0 sequence?
        if (0x00E0 == scancode) {
            e0_seen = 1;
            continue;
        }

        // How about E1?
        if (0x00E1 == scancode) {
            // For now there is only one key which generates E1 sequences
            ps2kbd_current_key      = MWKEY_BREAK;
            ps2kbd_current_keydown  = 1;
            ps2kbd_current_scancode = 0x00E1;   // Invent another key scancode
            pending_up              = MWKEY_BREAK;
            pending_scancode        = 0x00E1;
            discard = 5;
            return;
        }

        // Must be an ordinary key.
        ps2kbd_current_keydown  = (0 == (scancode & 0x0080));
        scancode &= 0x007F;
        ps2kbd_current_scancode = scancode;
        ps2kbd_current_key      = ps2kbd_keymap[scancode].normal;

        // Detect the modifier keys.
        for (i = 0; MWKEY_UNKNOWN != ps2kbd_modifier_map[i].key; i++) {
            if (ps2kbd_current_key == ps2kbd_modifier_map[i].key) {
                // capslock etc. behave differently. Toggle the current
                // status on the keyup event, ignore key down (because
                // of hardware autorepeat).
                if (ps2kbd_modifier_map[i].toggle) {
                    if (!ps2kbd_current_keydown) {
                        ps2kbd_current_modifiers ^= ps2kbd_modifier_map[i].modifier;
                    }
                } else if (ps2kbd_current_keydown) {
                    ps2kbd_current_modifiers |= ps2kbd_modifier_map[i].modifier;
                } else {
                    ps2kbd_current_modifiers &= ~ps2kbd_modifier_map[i].modifier;
                }
                break;
            }
        }

        // Cope with some of the modifiers.
        if (0 != (ps2kbd_current_modifiers & (MWKMOD_LCTRL | MWKMOD_RCTRL))) {
            // Control key. a-z and A-Z go to ctrl-A - ctrl-Z, i.e. 1 to 26
            // Other characters in the range 64 to 96, e.g. [ and ], also
            // get translated. This includes the A-Z range.
            if ((64 <= ps2kbd_current_key) && (ps2kbd_current_key < 96)) {
                ps2kbd_current_key -= 64;
            } else if (('a' <= ps2kbd_current_key) && (ps2kbd_current_key <= 'z')) {
                ps2kbd_current_key -= 96;
            }
        } else if (ps2kbd_current_modifiers & (MWKMOD_LSHIFT | MWKMOD_RSHIFT)) {
            // Pick up the shift entry from the keymap
            ps2kbd_current_key = ps2kbd_keymap[scancode].shifted;
        }
        
        if (ps2kbd_current_modifiers & MWKMOD_CAPS) {
            // Capslock only affects a-z and A-z
            if ( ('a' <= ps2kbd_current_key) && (ps2kbd_current_key <= 'z')) {
                ps2kbd_current_key = (ps2kbd_current_key -'a') + 'A';
            } else if (('A' <= ps2kbd_current_key) && (ps2kbd_current_key <= 'Z')) {
                ps2kbd_current_key = (ps2kbd_current_key -'A') + 'a';
            }
        }

        // If we have found a real key, the loop will exit.
        // Otherwise try again.
    }
}

static int
PS2Kbd_Open(KBDDEVICE* pkbd)
{
    unsigned char buf[1];
    ps2_initialize();
    buf[0] = KC_KBDC_ENABLE;
    ps2_send_command_wait(KC_CONTROL_NULL, buf, 1, 0);
}

static void
PS2Kbd_Close(void)
{
    unsigned char buf[1];
    buf[0] = KC_KBDC_STANDARD_DISABLE;
    ps2_send_command_wait(KC_CONTROL_NULL, buf, 1, 0);
}

static void
PS2Kbd_GetModifierInfo(MWKEYMOD* modifiers, MWKEYMOD* curmodifiers)
{
    if (NULL != modifiers) {
        *modifiers = MWKMOD_LSHIFT | MWKMOD_RSHIFT | MWKMOD_LCTRL | MWKMOD_RCTRL |
            MWKMOD_LALT | MWKMOD_RALT | MWKMOD_LMETA | MWKMOD_RMETA |
            MWKMOD_NUM | MWKMOD_CAPS | MWKMOD_ALTGR | MWKMOD_SCR;
    }
    if (NULL != curmodifiers) {
        *modifiers = ps2kbd_current_modifiers;
    }
}

// Note: it is assumed that there are no concurrent calls to
// the poll and read functions.
static int
PS2Kbd_Poll(void)
{
    if (MWKEY_UNKNOWN == ps2kbd_current_key) {
        ps2kbd_process_scancodes();
    }
    return MWKEY_UNKNOWN != ps2kbd_current_key;
}

// Return -1 on error, 0 for no data, 1 for a keypress, 2 for a keyrelease.
static int
PS2Kbd_Read(MWKEY* buf, MWKEYMOD* modifiers, MWSCANCODE* scancode)
{
    if (MWKEY_UNKNOWN == ps2kbd_current_key) {
        ps2kbd_process_scancodes();
        if (MWKEY_UNKNOWN == ps2kbd_current_key) {
            return 0;
        }
    }

    if (NULL != buf) {
        *buf = ps2kbd_current_key;
    }
    if (NULL != modifiers) {
        *modifiers = ps2kbd_current_modifiers;
    }
    if (NULL != scancode) {
        *scancode = ps2kbd_current_scancode;
    }
    ps2kbd_current_key = MWKEY_UNKNOWN;
    return ps2kbd_current_keydown ? 1 : 2;
}

KBDDEVICE kbddev = {
    PS2Kbd_Open,
    PS2Kbd_Close,
    PS2Kbd_GetModifierInfo,
    PS2Kbd_Read,
    PS2Kbd_Poll
};

#if 0
// ----------------------------------------------------------------------------
// This code can be used for testing the mouse and keyboard without all
// of microwindows present (although the microwindows headers must still be
// visible).

#include <stdio.h>
#include <ctype.h>
int
main(int argc, char** argv)
{
    PS2Kbd_Open(&kbddev);
    PS2Mouse_Open(&mousedev);
    for ( ; ; ) {
        while (PS2Mouse_Poll()) {
            MWCOORD dx, dy, dz;
            int     buttons;
            PS2Mouse_Read(&dx, &dy, &dz, &buttons);
            printf("Mouse movement: dx %d, dy %d, dz %d, buttons 0x%02x\n", dx, dy, dz, buttons);
        }
        while (PS2Kbd_Poll()) {
            MWKEY       key;
            MWKEYMOD    modifiers;
            MWSCANCODE  scancode;
            int         which;
            which = PS2Kbd_Read(&key, &modifiers, &scancode);
            printf("Keyboard event: %s, key %c (%d) (0x%02x), modifiers 0x%02x, scancode %d (0x%02x)\n",
                   (1 == which) ? "press" : "release", isprint(key) ? key : '?', key, key, modifiers, scancode, scancode);
        }
    }
}
#endif
