//==========================================================================
//
//      kbd_vnc_ecos.c
//
//
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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
// Author(s):    Chris Garry <cgarry@sweeneydesign.co.uk>
// Contributors:
// Date:         2003-08-22
// Purpose:
// Description:  Microwindows keyboard driver for VNC server on eCos
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pkgconf/vnc_server.h>  /* CYGDAT_VNC_SERVER_KEYBOARD_NAME */
#include "device.h"

static int  vnc_Open(KBDDEVICE *pkd);
static void vnc_Close(void);
static void vnc_GetModifierInfo(MWKEYMOD *modifiers, MWKEYMOD *curmodifiers);
static int  vnc_Read(MWKEY *kbuf, MWKEYMOD *modifiers, MWSCANCODE *scancode);

KBDDEVICE kbddev = {
	vnc_Open,
	vnc_Close,
	vnc_GetModifierInfo,
	vnc_Read,
	NULL
};

static int kbd_fd;  /* File descriptor for keyboard */
static MWKEYMOD current_mods;  /* Current keyboard modifiers */


static int vnc_Open(KBDDEVICE *pkd)
{
    /* Open the keyboard and get it ready for use */
    kbd_fd = open(CYGDAT_VNC_SERVER_KEYBOARD_NAME, O_RDONLY | O_NONBLOCK);

    if (kbd_fd < 0)
    {
        EPRINTF("%s - Can't open keyboard!\n", __FUNCTION__);
        return -1;
    }

    current_mods = MWKMOD_NONE;  /* Initialise the current modifiers */

    /* Keyboard opened okay - return file descriptor */
    return(kbd_fd);
}


static void vnc_Close(void)
{
    /* Close the mouse device. */
    if (kbd_fd >= 0)
    {
        close(kbd_fd);
    }

    kbd_fd = -1;
}


static void vnc_GetModifierInfo(MWKEYMOD *modifiers, MWKEYMOD *curmodifiers)
{
    if (modifiers)
    {
        *modifiers = MWKMOD_CTRL | MWKMOD_SHIFT | MWKMOD_ALT | MWKMOD_META
                   | MWKMOD_CAPS | MWKMOD_NUM | MWKMOD_SCR;
    }
    if (curmodifiers)
    {
        *curmodifiers = current_mods;
    }
}


/*
 * This reads one keystroke from the keyboard, and the current state of
 * the mode keys (ALT, SHIFT, CTRL).  Returns -1 on error, 1 if key is
 * pressed, and 0 if key was released.  This is a non-blocking call.
 */
static int vnc_Read(MWKEY *kbuf, MWKEYMOD *modifiers, MWSCANCODE *scancode)
{
    cyg_uint8 keystroke_buff[4];  /* Smal buffer to hold data for 1 keystroke */
    int bytes_read;

    /* Try to read the data for 1 keystroke event from the keyboard device */
    bytes_read = read(kbd_fd, keystroke_buff, 4);

    /* Each keystroke event generates 4 x cyg_uint8 values in the queue
     *   0: Padding - always zero
     *   1: Key pressed (1 when key is pressed)
     *   2: Keysym value MSB
     *   3: Keysym value LSB
     */

    if (bytes_read != 4)
    {
        return 0;
    }

    /* Log modifier bits and convert Keysym values to Unicode */
    switch(keystroke_buff[2]*256 + keystroke_buff[3])
    {
    case 0x0000 ... 0x007F:  /* Standard ASCII */
        *kbuf = keystroke_buff[3];
        break;

    case 0x0080 ... 0x00FF:  /* Not so sure what to do with these - but it works for '£' */
        *kbuf = keystroke_buff[3];
        break;

    case 0xFF08:  /* BACKSPACE */
        *kbuf = MWKEY_BACKSPACE;
        break;

    case 0xFF09:  /* TAB */
        *kbuf = MWKEY_TAB;
        break;

    case 0xFF0D:  /* ENTER */
        *kbuf = MWKEY_ENTER;
        break;

    case 0xFF13:  /* BREAK */
        *kbuf = MWKEY_BREAK;
        break;

    case 0xFF14:  /* Scroll lock */
        if (!keystroke_buff[0])  /* Key released */
            current_mods ^= MWKMOD_SCR;
            /* Note we invert the Scroll lock bit on each Scroll lock release event */
        *kbuf = MWKEY_SCROLLOCK;
        break;

    case 0xFF1B: /* ESC */
        *kbuf = MWKEY_ESCAPE;
        break;

    case 0xFF50: /* HOME */
        *kbuf = MWKEY_HOME;
        break;

    case 0xFF51: /* LEFT ARROW */
        *kbuf = MWKEY_LEFT;
        break;

    case 0xFF52: /* UP ARROW */
        *kbuf = MWKEY_UP;
        break;

    case 0xFF53: /* RIGHT ARROW */
        *kbuf = MWKEY_RIGHT;
        break;

    case 0xFF54: /* DOWN ARROW */
        *kbuf = MWKEY_DOWN;
        break;

    case 0xFF55: /* PAGE UP */
        *kbuf = MWKEY_PAGEUP;
        break;

    case 0xFF56: /* PAGE DOWN */
        *kbuf = MWKEY_PAGEDOWN;
        break;

    case 0xFF57: /* END */
        *kbuf = MWKEY_END;
        break;

    case 0xFF61:  /* PRINT SCREEN */
        *kbuf = MWKEY_PRINT;
        break;

    case 0xFF63:  /* INSERT */
        *kbuf = MWKEY_INSERT;
        break;

    case 0xFF7F:  /* NUM lock */
        if (!keystroke_buff[0])  /* Key released */
            current_mods ^= MWKMOD_NUM;
            /* Note we invert the NUM lock bit on each NUN lock release event */

        *kbuf = MWKEY_NUMLOCK;
        break;

    case 0xFFAA:  /* KEYPAD * */
        *kbuf = MWKEY_KP_MULTIPLY;
        break;

    case 0xFFAB:  /* KEYPAD + */
        *kbuf = MWKEY_KP_PLUS;
        break;

    case 0xFFAD:  /* KEYPAD - */
        *kbuf = MWKEY_KP_MINUS;
        break;

    case 0xFFAE:  /* KEYPAD . */
        *kbuf = MWKEY_KP_PERIOD;
        break;

    case 0xFFAF:  /* KEYPAD / */
        *kbuf = MWKEY_KP_DIVIDE;
        break;

    case 0xFFB0 ... 0xFFB9:  /* Numeric keypad 0 to 9 */
        /* We can calculate this since both the Keysym codes and unicodes are consecutive */
        *kbuf = MWKEY_KP0 + (keystroke_buff[2]*256 + keystroke_buff[3]) - 0xFFB0;
        break;

    case 0xFFBE ... 0xFFC9:  /* F1 to F12 */
        /* We can calculate this since both the Keysym codes and unicodes are consecutive */
        *kbuf = MWKEY_F1 + (keystroke_buff[2]*256 + keystroke_buff[3]) - 0xFFBE;
        break;

    case 0xFFE1:  /* SHIFT left*/
        if (keystroke_buff[1])  /* Key pressed */
            current_mods |= MWKMOD_LSHIFT;  /* Set SHIFT modifier bit */
        else
            current_mods &= (0xFFFF ^ MWKMOD_LSHIFT);  /* Clear SHIFT modifier bit */

        *kbuf = MWKEY_LSHIFT;
        break;

    case 0xFFE2:  /* SHIFT right */
        if (keystroke_buff[1])  /* Key pressed */
            current_mods |= MWKMOD_RSHIFT;  /* Set SHIFT modifier bit */
        else
            current_mods &= (0xFFFF ^ MWKMOD_RSHIFT);  /* Clear SHIFT modifier bit */

        *kbuf = MWKEY_RSHIFT;
        break;

    case 0xFFE3:  /* CTRL left */
        if (keystroke_buff[1])  /* Key pressed */
            current_mods |= MWKMOD_LCTRL;  /* Set CTRL modifier bit */
        else
        {
            /* Clear CTRL modifier bit */
            current_mods &= (0xFFFF ^ MWKMOD_LCTRL);
        }

        *kbuf = MWKEY_LCTRL;
        break;

    case 0xFFE4:  /* CTRL right */
        if (keystroke_buff[1])  /* Key pressed */
            current_mods |= MWKMOD_RCTRL;  /* Set CTRL modifier bit */
        else
            current_mods &= (0xFFFF ^ MWKMOD_RCTRL);  /* Clear CTRL modifier bit */

        *kbuf = MWKEY_RCTRL;
        break;

    case 0xFFE5:  /* CAPS lock */
        if (!keystroke_buff[0])  /* Key released */
            current_mods ^= MWKMOD_CAPS;
            /* Note we invert the CAPS lock bit on each CAPS lock release event */

        *kbuf = MWKEY_CAPSLOCK;
        break;

    case 0xFFE7:  /* META Left */
        if (keystroke_buff[1])  /* Key pressed */
            current_mods |= MWKMOD_LMETA;  /* Set META modifier bit */
        else
            current_mods &= (0xFFFF ^ MWKMOD_LMETA);  /* Clear META modifier bit */

        *kbuf = MWKEY_LMETA;
        break;

    case 0xFFE8:  /* META Right */
        if (keystroke_buff[1])  /* Key pressed */
            current_mods |= MWKMOD_RMETA;  /* Set META modifier bit */
        else
            current_mods &= (0xFFFF ^ MWKMOD_RMETA);  /* Clear META modifier bit */

        *kbuf = MWKEY_RMETA;
        break;

    case 0xFFE9:  /* ALT left */
        if (keystroke_buff[1])  /* Key pressed */
            current_mods |= MWKMOD_LALT;  /* Set ALT modifier bit */
        else
            current_mods &= (0xFFFF ^ MWKMOD_LALT);  /* Clear ALT modifier bit */

        *kbuf = MWKEY_LALT;
        break;

    case 0xFFEA:  /* ALT right */
        if (keystroke_buff[1])  /* Key pressed */
            current_mods |= MWKMOD_RALT;  /* Set ALT modifier bit */
        else
            current_mods &= (0xFFFF ^ MWKMOD_RALT);  /* Clear ALT modifier bit */

        *kbuf = MWKEY_RALT;
        break;

    case 0xFFFF: /* DELETE */
        *kbuf = MWKEY_DELETE;
        break;

    default:
       *kbuf = MWKEY_UNKNOWN;
    }

    *modifiers = current_mods;
    *scancode = 0;  /* Scan code not supported because this is just a virtual device */

    if (keystroke_buff[1])
    {
      return 1;
    }
    else
    {
      return 0;
    }
}
