//==========================================================================
//
//        kbd_support.c
//
//        Cirrus Logic EDB7xxx eval board ASCII keyboard support functions
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
// Author(s):     gthomas
// Contributors:  gthomas
// Date:          1999-05-15
// Description:   Functions used to support ASCII keyboard
//####DESCRIPTIONEND####

static char kbd_server_stack[STACK_SIZE];
static cyg_thread kbd_server_thread_data;
static cyg_handle_t kbd_server_thread_handle;

static cyg_interrupt kbd_interrupt;
static cyg_handle_t  kbd_interrupt_handle;

static cyg_mbox      kbd_events_mbox;
static cyg_handle_t  kbd_events_mbox_handle;
static cyg_sem_t     kbd_sem;

static cyg_uint8 col_state[8];
static cyg_uint8 ext_state[8];

static void
kbd_delay(void)
{
    volatile int i;
    for (i = 0;  i < 250;  i++) ;
}

static void
kbd_scan(void)
{
    int i;
    cyg_uint8 port_data, ext_data;
    // Turn off drive (de-select) all columns
    *(volatile cyg_uint32 *)SYSCON1 = (*(volatile cyg_uint32 *)SYSCON1 & ~SYSCON1_KBD_CTL) |
        SYSCON1_KBD_LOW;
    for (i = 0;  i < 8;  i++) {
        // Select column 'i'
        *(volatile cyg_uint32 *)SYSCON1 = (*(volatile cyg_uint32 *)SYSCON1 & ~SYSCON1_KBD_CTL) |
            SYSCON1_KBD_COL(i);
        // Small pause to let the wires charge up :-)
        kbd_delay();
        // Grab the data
        col_state[i] = port_data = *(volatile cyg_uint8 *)PADR;
        ext_state[i] = ext_data = *(volatile cyg_uint8 *)KBD_PORT;
        // De-Select column 'i'
        *(volatile cyg_uint32 *)SYSCON1 = (*(volatile cyg_uint32 *)SYSCON1 & ~SYSCON1_KBD_CTL) |
            SYSCON1_KBD_TRISTATE;
        // Allow line to go slack
        kbd_delay();
    }
    // Turn on drive (select) all columns - necessary to see interrupts
    *(volatile cyg_uint32 *)SYSCON1 = (*(volatile cyg_uint32 *)SYSCON1 & ~SYSCON1_KBD_CTL) |
        SYSCON1_KBD_HIGH;
}

static cyg_uint8     kbd_state[128];      // Known state of each key
static cyg_uint8     kbd_new_state[128];  // Current state of each key

// Row #1
#define KBD_Escape     0x00
#define KBD_F1         0x01
#define KBD_F2         0x02
#define KBD_F3         0x03
#define KBD_F4         0x04
#define KBD_F5         0x05
#define KBD_F6         0x06
#define KBD_F7         0x07
#define KBD_F8         0x08
#define KBD_F9         0x09
#define KBD_F10        0x0A
#define KBD_NumLock    0x0B
#define KBD_SysReq     0x0C
#define KBD_ScrlLock   0x0D
#define KBD_Break      0x0E
// Row #2
#define KBD_1          0x10
#define KBD_2          0x11
#define KBD_3          0x12
#define KBD_4          0x13
#define KBD_5          0x14
#define KBD_6          0x15
#define KBD_7          0x16
#define KBD_8          0x17
#define KBD_9          0x18
#define KBD_0          0x19
#define KBD_Hyphen     0x1A
#define KBD_Equals     0x1B
#define KBD_BackSpace  0x1C
#define KBD_Home       0x1D
// Row #3
#define KBD_Tab        0x20
#define KBD_Q          0x21
#define KBD_W          0x22
#define KBD_E          0x23
#define KBD_R          0x24
#define KBD_T          0x25
#define KBD_Y          0x26
#define KBD_U          0x27
#define KBD_I          0x28
#define KBD_O          0x29
#define KBD_P          0x2A
#define KBD_LeftBrkt   0x2B
#define KBD_RightBrkt  0x2C
#define KBD_BackSlash  0x2D
#define KBD_PageUp     0x2E
// Row #4
#define KBD_CapsLock   0x30
#define KBD_A          0x31
#define KBD_S          0x32
#define KBD_D          0x33
#define KBD_F          0x34
#define KBD_G          0x35
#define KBD_H          0x36
#define KBD_J          0x37
#define KBD_K          0x38
#define KBD_L          0x39
#define KBD_SemiColon  0x3A
#define KBD_Quote      0x3B
#define KBD_Enter      0x3C
#define KBD_PageDown   0x3D
// Row #5
#define KBD_LeftShift  0x40
#define KBD_Z          0x41
#define KBD_X          0x42
#define KBD_C          0x43
#define KBD_V          0x44
#define KBD_B          0x45
#define KBD_N          0x46
#define KBD_M          0x47
#define KBD_Comma      0x48
#define KBD_Period     0x49
#define KBD_Slash      0x4A
#define KBD_RightShift 0x4B
#define KBD_UpArrow    0x4C
#define KBD_End        0x4D
// Row #6
#define KBD_Ctrl       0x50
#define KBD_Function   0x51
#define KBD_LeftAlt    0x52
#define KBD_Accent     0x53
#define KBD_Space      0x54
#define KBD_RightAlt   0x55
#define KBD_Ins        0x56
#define KBD_Del        0x57
#define KBD_LeftArrow  0x58
#define KBD_DownArrow  0x59
#define KBD_RightArrow 0x5A

#define KBD_Press      0x80  // Event has this bit when the key is pressed
#define KBD_Empty      0xFF

// Map column, bit -> keycode
static cyg_uint32    kbd_map[8][16] = {
    // Column #0
    {KBD_Escape,    KBD_1,          KBD_Tab,        KBD_CapsLock,
     KBD_BackSlash, KBD_Space,      KBD_LeftArrow,  KBD_UpArrow,
     KBD_DownArrow, KBD_RightArrow, KBD_LeftShift,  KBD_Ctrl, 
     KBD_Function,  KBD_LeftAlt,    KBD_RightAlt,   KBD_RightShift },
    // Column #1
    {KBD_F5,        KBD_6,          KBD_T,          KBD_G,  
     KBD_B,         KBD_Slash,      KBD_SemiColon,  KBD_P,  
     KBD_Hyphen,    KBD_F10,        KBD_Empty,      KBD_Empty,
     KBD_Empty,     KBD_Empty,      KBD_Empty,      KBD_Empty     },
    // Column #2
    {KBD_F4,        KBD_5,          KBD_R,          KBD_F,  
     KBD_V,         KBD_Del,        KBD_Quote,      KBD_LeftBrkt,  
     KBD_Equals,    KBD_NumLock,    KBD_Empty,      KBD_Empty,
     KBD_Empty,     KBD_Empty,      KBD_Empty,      KBD_Empty },
    // Column #3
    {KBD_F3,        KBD_4,          KBD_E,          KBD_D,  
     KBD_C,         KBD_Ins,        KBD_Empty,      KBD_RightBrkt,  
     KBD_BackSpace, KBD_SysReq,     KBD_Empty,      KBD_Empty,
     KBD_Empty,     KBD_Empty,      KBD_Empty,      KBD_Empty },
    // Column #4
    {KBD_F2,        KBD_3,          KBD_W,          KBD_S,  
     KBD_X,         KBD_Empty,      KBD_Enter,      KBD_Empty,
     KBD_Empty,     KBD_ScrlLock,   KBD_Empty,      KBD_Empty,
     KBD_Empty,     KBD_Empty,      KBD_Empty,      KBD_Empty },
    // Column #5
    {KBD_F1,        KBD_2,          KBD_Q,          KBD_A,  
     KBD_Z,         KBD_End,        KBD_PageDown,   KBD_PageUp,
     KBD_Home,      KBD_Break,      KBD_Empty,      KBD_Empty,
     KBD_Empty,     KBD_Empty,      KBD_Empty,      KBD_Empty },
    // Column #6
    {KBD_F6,        KBD_7,          KBD_Y,          KBD_H,  
     KBD_N,         KBD_Period,     KBD_L,          KBD_O,  
     KBD_0,         KBD_F9,         KBD_Empty,      KBD_Empty,
     KBD_Empty,     KBD_Empty,      KBD_Empty,      KBD_Empty },
    // Column #7
    {KBD_F7,        KBD_8,          KBD_U,          KBD_J,  
     KBD_M,         KBD_Comma,      KBD_K,          KBD_I,  
     KBD_9,         KBD_F8,         KBD_Empty,      KBD_Empty,
     KBD_Empty,     KBD_Empty,      KBD_Empty,      KBD_Empty },
};

static cyg_uint8 kbd_chars[96] = {
    /* 0x00 - 0x0F */
    0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x10 - 0x1F */
     '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '=', 0x08, 0x00, 0x00, 0x00,
    /* 0x20 - 0x2F */
    '\t',  'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  '[',  ']', '\\', 0x00, 0x00,
    /* 0x30 - 0x3F */
    0x00,  'a',  's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';', '\'', 0x0D, 0x00, 0x00, 0x00,
    /* 0x40 - 0x4F */
    0x00,  'z',  'x',  'c',  'v',  'b',  'n',  'm',  ',',  '.',  '/', 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x50 - 0x5F */
    0x00, 0x00, 0x00, 0x00,  ' ', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static cyg_uint8 kbd_shifted_chars[96] = {
    /* 0x00 - 0x0F */
    '\b', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x10 - 0x1F */
     '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',  '_',  '+', 0x08, 0x00, 0x00, 0x00,
    /* 0x20 - 0x2F */
    '\t',  'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  'O',  'P',  '{',  '}',  '|', 0x00, 0x00,
    /* 0x30 - 0x3F */
    0x00,  'A',  'S',  'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',  '"', 0x0D, 0x00, 0x00, 0x00,
    /* 0x40 - 0x4F */
    0x00,  'Z',  'X',  'C',  'V',  'B',  'N',  'M',  '<',  '>',  '?', 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x50 - 0x5F */
    0x00, 0x00, 0x00, 0x00,  ' ', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// This ISR is called when the keyboard interrupt occurs
static int
keyboard_isr(cyg_vector_t vector, cyg_addrword_t data, HAL_SavedRegisters *regs)
{
    cyg_drv_interrupt_mask(CYGNUM_HAL_INTERRUPT_KBDINT);
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR);  // Run the DSR
}

// This DSR handles the keyboard [logical] processing
static void
keyboard_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    // Tell the keyboard processing thread to give it a shot
    cyg_semaphore_post(&kbd_sem);
}

static void
kbd_server(cyg_addrword_t p)
{
    int col, bit, key, event, timeout;
    diag_printf("KBD server here\n");
    while (TRUE) {
        cyg_semaphore_wait(&kbd_sem);
        // As long as keys are pressed, scan and update
        timeout = 0;
        while (TRUE) {
            // Wait for 20ms - time to debounce keyboard
            cyg_thread_delay(2);
            // Scan keyboard
            kbd_scan();
            // Reset state
            for (key = 0;  key < sizeof(kbd_new_state);  key++) {
                kbd_new_state[key] = 0;
            }
            // Check state of all keys and send events for those that change
            for (col = 0;  col < 8;  col++) {
                for (bit = 0;  bit < 8;  bit++) {
                    if (col_state[col] & (1<<bit)) {
                        key = kbd_map[col][bit];
                        if (key != KBD_Empty) {
                            kbd_new_state[key] = 1;
                        }
                    }
                    if (ext_state[col] & (1<<bit)) {
                        key = kbd_map[col][bit+8];
                        if (key != KBD_Empty) {
                            kbd_new_state[key] = 1;
                        }
                    }
                }
            }
            // Compare new and old (known) states, generate events for changes
            // Send events for modifier keys first.
            for (key = 0;  key < sizeof(kbd_new_state);  key++) {
                if (kbd_state[key] != kbd_new_state[key]) {
                    event = 0xFF;
                    switch (key) {
                    case KBD_LeftShift:
                    case KBD_RightShift:
                    case KBD_Ctrl:
                    case KBD_LeftAlt:
                    case KBD_RightAlt:
                    case KBD_Function:
                    case KBD_CapsLock:
                        if (kbd_state[key]) {
                            // Key going up
                            event = key;
                        } else {
                            // Key going down
                            event = key + KBD_Press;
                        }
                        kbd_state[key] = kbd_new_state[key];
                    }
                    if (event != 0xFF) {
                        if (!cyg_mbox_tryput(kbd_events_mbox_handle, (void *)event)) {
                            diag_printf("KBD event lost: %x\n", event);
                        }
                    }
                }
            }
            // First key up events
            for (key = 0;  key < sizeof(kbd_new_state);  key++) {
                if (kbd_state[key] != kbd_new_state[key]) {
                    if (kbd_state[key]) {
                        // Key going up
                        event = key;
                    } else {
                        // Key going down
                        event = key + KBD_Press;
                    }
                    if (!cyg_mbox_tryput(kbd_events_mbox_handle, (void *)event)) {
                        diag_printf("KBD event lost: %x\n", event);
                    }
                }
                kbd_state[key] = kbd_new_state[key];
            }
            // Clear interrupt (true when keys are pressed)
            cyg_drv_interrupt_acknowledge(CYGNUM_HAL_INTERRUPT_KBDINT);
#if 0
            if (*(volatile cyg_uint32 *)INTSR2 & INTSR2_KBDINT) {
                timeout = 0;
            } else if (++timeout == 5) {
                // No keys for 100ms
                break;
            }
#endif
        }
        // Allow interrupts to happen again
        cyg_drv_interrupt_acknowledge(CYGNUM_HAL_INTERRUPT_KBDINT);
        cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_KBDINT);
    }
}

void
kbd_init(void)
{
    // Initialize environment, setup interrupt handler
    cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_KBDINT,
                             99,                     // Priority - what goes here?
                             0,                      //  Data item passed to interrupt handler
                             (cyg_ISR_t *)keyboard_isr,
                             (cyg_DSR_t *)keyboard_dsr,
                             &kbd_interrupt_handle,
                             &kbd_interrupt);
    cyg_drv_interrupt_attach(kbd_interrupt_handle);
    cyg_drv_interrupt_acknowledge(CYGNUM_HAL_INTERRUPT_KBDINT);
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_KBDINT);
    // Set up the mbox for keyboard data
    cyg_mbox_create(&kbd_events_mbox_handle, &kbd_events_mbox);
    // This semaphore is set when there is a keypress
    cyg_semaphore_init(&kbd_sem, 0);  
    // Create a thread whose job it is to de-bounce the keyboard and
    // actually process the input, turning it into a series of events
    cyg_thread_create(10,                           // Priority - just a number
                      kbd_server,                   // entry
                      0,                            // initial parameter
                      "KBD_server",                 // Name
                      &kbd_server_stack[0],         // Stack
                      STACK_SIZE,                   // Size
                      &kbd_server_thread_handle,    // Handle
                      &kbd_server_thread_data       // Thread data structure
            );
    cyg_thread_resume(kbd_server_thread_handle);  // Start it
}

#define MOD_Shift    0x40
#define MOD_Ctrl     0x20
#define MOD_CapsLock 0x10
static cyg_uint32    kbd_modifiers;

cyg_uint8 
kbd_getc(void)
{
    cyg_uint8 ch;
    cyg_uint32 kbd_event;
    while (TRUE) {
        kbd_event = (cyg_uint32)cyg_mbox_get(kbd_events_mbox_handle);
        switch (kbd_event & 0x7F) {
        case KBD_LeftShift:
        case KBD_RightShift:
            if (kbd_event & KBD_Press) {
                kbd_modifiers |= MOD_Shift;
            } else {
                kbd_modifiers &= ~MOD_Shift;
            }
            break;
        case KBD_Ctrl:
            if (kbd_event & KBD_Press) {
                kbd_modifiers |= MOD_Ctrl;
            } else {
                kbd_modifiers &= ~MOD_Ctrl;
            }
            break;
        case KBD_CapsLock:
            if (kbd_event & KBD_Press) {                
                kbd_modifiers ^= MOD_CapsLock;
            }
            break;
        case KBD_LeftAlt:
        case KBD_RightAlt:
        case KBD_Function:
        default:
        }
        // Return character [if one has arrived]
        if (kbd_event & KBD_Press) {
            if (kbd_modifiers & (MOD_Shift|MOD_CapsLock)) {
                ch = kbd_shifted_chars[kbd_event & 0x7F];
            } else {
                ch = kbd_chars[kbd_event & 0x7F];
            }
            if (kbd_modifiers & MOD_Ctrl) {
                ch &= 0x1F;
            }
            if (ch) {
                return (ch);
            }
        }
    }
}
