//==========================================================================
//
//        kbd_drvr.c
//
//        Agilent AAED2000 - keyboard driver
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
// Date:          2001-11-03
// Description:   Keyboard driver
//####DESCRIPTIONEND####

#include <pkgconf/hal.h>

#include <cyg/infra/diag.h>
#include <cyg/hal/hal_io.h>       // IO macros
#include <cyg/hal/hal_if.h>       // Virtual vector support
#include <cyg/hal/hal_arch.h>     // Register state info
#include <cyg/hal/hal_intr.h>     // HAL interrupt macros

#include <cyg/hal/aaed2000.h>  // Board definitions
#include <cyg/hal/hal_cache.h>

//-----------------------------------------------------------------------------
// Keyboard definitions

// Standard [PC keyboard] scan codes

#define	LSHIFT		0x2a
#define	RSHIFT		0x36
#define	CTRL		0x1d
#define	ALT		0x38
#define	CAPS		0x3a
#define	NUMS		0x45

#define	BREAK		0x80

// Bits for KBFlags

#define	KBNormal	0x0000
#define	KBShift		0x0001
#define	KBCtrl		0x0002
#define KBAlt		0x0004
#define	KBIndex		0x0007	// mask for the above

#define	KBExtend	0x0010
#define	KBAck		0x0020
#define	KBResend	0x0040
#define	KBShiftL	(0x0080 | KBShift)
#define	KBShiftR	(0x0100 | KBShift)
#define	KBCtrlL		(0x0200 | KBCtrl)
#define	KBCtrlR		(0x0400 | KBCtrl)
#define	KBAltL		(0x0800 | KBAlt)
#define	KBAltR		(0x1000 | KBAlt)
#define	KBCapsLock	0x2000
#define	KBNumLock	0x4000

#define KBArrowUp       0x48
#define KBArrowRight    0x4D
#define KBArrowLeft     0x4B
#define KBArrowDown     0x50

//-----------------------------------------------------------------------------
// Keyboard Variables

static	int	KBFlags = 0;

static  CYG_BYTE	KBPending = 0xFF;

static	CYG_BYTE	KBScanTable[128][4] = {
//	Normal		Shift		Control		Alt
// 0x00
    {	0xFF,		0xFF,		0xFF,		0xFF,   },
    {	0x1b,		0x1b,		0x1b,		0xFF,	},
    {	'1',		'!',		0xFF,		0xFF,	},
    {	'2',		'@',		0xFF,		0xFF,	},
    {	'3',		'#',		0xFF,		0xFF,	},
    {	'4',		'$',		0xFF,		0xFF,	},
    {	'5',		'%',		0xFF,		0xFF,	},
    {	'6',		'^',		0xFF,		0xFF,	},
    {	'7',		'&',		0xFF,		0xFF,	},
    {	'8',		'*',		0xFF,		0xFF,	},
    {	'9',		'(',		0xFF,		0xFF,	},
    {	'0',		')',		0xFF,		0xFF,	},
    {	'-',		'_',		0xFF,		0xFF,	},
    {	'=',		'+',		0xFF,		0xFF,	},
    {	'\b',		'\b',		0xFF,		0xFF,	},
    {	'\t',		'\t',		0xFF,		0xFF,	},
// 0x10
    {	'q',		'Q',		0x11,		0xFF,	},
    {	'w',		'W',		0x17,		0xFF,	},
    {	'e',		'E',		0x05,		0xFF,	},
    {	'r',		'R',		0x12,		0xFF,	},
    {	't',		'T',		0x14,		0xFF,	},
    {	'y',		'Y',		0x19,		0xFF,	},
    {	'u',		'U',		0x15,		0xFF,	},
    {	'i',		'I',		0x09,		0xFF,	},
    {	'o',		'O',		0x0F,		0xFF,	},
    {	'p',		'P',		0x10,		0xFF,	},
    {	'[',		'{',		0x1b,		0xFF,	},
    {	']',		'}',		0x1d,		0xFF,	},
    {	'\r',		'\r',		'\n',		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	'a',		'A',		0x01,		0xFF,	},
    {	's',		'S',		0x13,		0xFF,	},
// 0x20
    {	'd',		'D',		0x04,		0xFF,	},
    {	'f',		'F',		0x06,		0xFF,	},
    {	'g',		'G',		0x07,		0xFF,	},
    {	'h',		'H',		0x08,		0xFF,	},
    {	'j',		'J',		0x0a,		0xFF,	},
    {	'k',		'K',		0x0b,		0xFF,	},
    {	'l',		'L',		0x0c,		0xFF,	},
    {	';',		':',		0xFF,		0xFF,	},
    {	0x27,		'"',		0xFF,		0xFF,	},
    {	'`',		'~',		0xFF,		0xFF,	},
    {	'`',		'~',		0xFF,		0xFF,	},
    {	'\\',		'|',		0x1C,		0xFF,	},
    {	'z',		'Z',		0x1A,		0xFF,	},
    {	'x',		'X',		0x18,		0xFF,	},
    {	'c',		'C',		0x03,		0xFF,	},
    {	'v',		'V',		0x16,		0xFF,	},
// 0x30
    {	'b',		'B',		0x02,		0xFF,	},
    {	'n',		'N',		0x0E,		0xFF,	},
    {	'm',		'M',		0x0D,		0xFF,	},
    {	',',		'<',		0xFF,		0xFF,	},
    {	'.',		'>',		0xFF,		0xFF,	},
    {	'/',		'?',		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	' ',		' ',		' ',		' ',	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xF1,		0xE1,		0xFF,		0xFF,	},
    {	0xF2,		0xE2,		0xFF,		0xFF,	},
    {	0xF3,		0xE3,		0xFF,		0xFF,	},
    {	0xF4,		0xE4,		0xFF,		0xFF,	},
    {	0xF5,		0xE5,		0xFF,		0xFF,	},
// 0x40
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},

    {	0x15,		0x15,		0x15,		0x15,	},
    {	0x10,		0x10,		0x10,		0x10,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
// 0x50
    {	0x04,		0x04,		0x04,		0x04,	},
    {	0x0e,		0x0e,		0x0e,		0x0e,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
// 0x60
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
// 0x70
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
    {	0xFF,		0xFF,		0xFF,		0xFF,	},
};

static int KBIndexTab[8] = { 0, 1, 2, 2, 3, 3, 3, 3 };

//-----------------------------------------------------------------------------

#define AAED2000_KBD_COLS 8
#define AAED2000_KBD_ROWS 12

static unsigned int KB_rowState[AAED2000_KBD_COLS];  // Last known state Row x Col

// Map AAED2000 row/col to [PC standard] scancodes
static const int scancode[AAED2000_KBD_ROWS][AAED2000_KBD_COLS] =
{
    {0x0A/*9*/, 0x25/*k*/, 0x17/*i*/, 0x09/*8*/, 0x24/*j*/, 0x15/*y*/, 0x08/*7*/, 0x00/*?*/},
    {0x0B/*0*/, 0x32/*m*/, 0x19/*p*/, 0x26/*l*/, 0x23/*h*/, 0x16/*u*/, 0x07/*8*/, 0x00/*?*/},
    {0x0C/*-*/, 0x18/*o*/, 0x1A/*{*/, 0x27/*:*/, 0x30/*b*/, 0x31/*n*/, 0x22/*g*/, 0x33/*<*/},
    {0x0D/*+*/, 0x34/*>*/, 0x1B/*}*/, 0x28/*'*/, 0x48/*?*/, 0x14/*t*/, 0x00/*?*/, 0x00/*?*/},
    {0x0E/*?*/, 0x35/*?*/, 0x2B/*|*/, 0x1C/*?*/, 0x4B/*?*/, 0x00/*?*/, 0x06/*5*/, 0x50/*?*/},
    {0x00/*?*/, 0x00/*?*/, 0x00/*?*/, 0x00/*?*/, 0x00/*?*/, 0x00/*?*/, 0x00/*?*/, 0x00/*?*/},
    {0x39/* */, 0x21/*f*/, 0x00/*?*/, 0x00/*?*/, 0x2E/*c*/, 0x13/*r*/, 0x05/*4*/, 0x2F/*v*/},
    {0x4D/*?*/, 0x00/*?*/, 0x00/*?*/, 0x00/*?*/, 0x20/*d*/, 0x12/*e*/, 0x04/*3*/, 0x38/*?*/},
    {0x00/*?*/, 0x00/*?*/, 0x00/*?*/, 0x00/*?*/, 0x1F/*s*/, 0x11/*w*/, 0x03/*2*/, 0x2D/*x*/},
    {0x53/*?*/, 0x00/*?*/, 0x00/*?*/, 0x00/*?*/, 0x1E/*a*/, 0x10/*q*/, 0x02/*1*/, 0x2C/*z*/},
    {0x00/*?*/, 0x2A/*?*/, 0x36/*?*/, 0x00/*?*/, 0x00/*?*/, 0x00/*?*/, 0x00/*?*/, 0x00/*?*/},
    {0x00/*?*/, 0x00/*?*/, 0x00/*?*/, 0x00/*?*/, 0x3A/*?*/, 0x0F/*?*/, 0x29/*~*/, 0x1D/*?*/}
};

static bool
KeyboardScan(CYG_BYTE *code)
{
    int row, col, i, down, up;
    unsigned int state, prev_state;
    int stable;

    for (col = 0;  col < AAED2000_KBD_COLS;  col++) {
        HAL_WRITE_UINT8(AAEC_KSCAN, col+8);  // Assert column #n
        
        for (i = 0, stable = 0, prev_state = 0;
             (i < 1000) && (stable < 120);  i++) {
            HAL_READ_UINT32(AAED_EXT_GPIO, state);
            if (state != prev_state) {
                prev_state = state;
                stable = 0;
            } else {
                stable++;
            }
        }
        state &= (1<<AAED2000_KBD_ROWS)-1;   // Mask off unused bits
        if (state != KB_rowState[col]) {
            // State changed
            down = state & ~KB_rowState[col];  // Bits that went 0->1
            up = ~state & KB_rowState[col];    // Bits that went 1->0
            for (row = 0;  row < AAED2000_KBD_ROWS;  row++) {
                if ((up|down) & (1<<row)) {
                    *code = scancode[row][col] | (up ? BREAK : 0);
                    // Only remember the bit we reported
                    KB_rowState[col] &= ~(1<<row);
                    state &= (1<<row);
                    KB_rowState[col] |= state;
                    return true;
                }
            }
        }
    }
    HAL_WRITE_UINT8(AAEC_KSCAN, 0x07);  // Turn off drivers
    return false;
}

//-----------------------------------------------------------------------------

static CYG_BYTE 
KeyboardAscii(CYG_BYTE scancode)
{
    CYG_BYTE ascii = 0xFF;

    // Start by handling all shift/ctl keys:

    switch( scancode ) {
    case 0xe0:
        KBFlags |= KBExtend;
        return 0xFF;

    case 0xfa:
        KBFlags |= KBAck;
        return 0xFF;

    case 0xfe:
        KBFlags |= KBResend;
        return 0xFF;

    case LSHIFT:
        KBFlags |= KBShiftL;
        return 0xFF;

    case LSHIFT | BREAK:
        KBFlags &= ~KBShiftL;
        return 0xFF;

    case RSHIFT:
        KBFlags |= KBShiftR;
        return 0xFF;

    case RSHIFT | BREAK:
        KBFlags &= ~KBShiftR;
        return 0xFF;

    case CTRL:
        if( KBFlags & KBExtend )
        {
            KBFlags |= KBCtrlR;
            KBFlags &= ~KBExtend;
        }
        else	KBFlags |= KBCtrlL;
        return 0xFF;

    case CTRL | BREAK:
        if( KBFlags & KBExtend )
        {
            KBFlags &= ~KBCtrlR;
            KBFlags &= ~KBExtend;
        }
        else	KBFlags &= ~KBCtrlL;
        return 0xFF;


    case ALT:
        if( KBFlags & KBExtend )
        {
            KBFlags |= KBAltR;
            KBFlags &= ~KBExtend;
        }
        else	KBFlags |= KBAltL;
        return 0xFF;

    case ALT | BREAK:
        if( KBFlags & KBExtend )
        {
            KBFlags &= ~KBAltR;
            KBFlags &= ~KBExtend;
        }
        else	KBFlags &= ~KBAltL;
        return 0xFF;

    case CAPS:
        KBFlags ^= KBCapsLock;
    case CAPS | BREAK:
        return 0xFF;

    case NUMS:
        KBFlags ^= KBNumLock;
    case NUMS | BREAK:
        return 0xFF;

    case KBArrowUp:
    case KBArrowDown:
#if 0 // Not really needed on this display
        screen_pan = 0;
        lcd_refresh();
#endif
        break;
    case KBArrowLeft:
#if 0 // Not really needed on this display
        screen_pan -= SCREEN_PAN;
        if (screen_pan < 0) screen_pan = 0;
        lcd_refresh();
#endif
        break;
    case KBArrowRight:
#if 0 // Not really needed on this display
        screen_pan += SCREEN_PAN;
        if (screen_pan > (SCREEN_WIDTH-SCREEN_PAN)) screen_pan = SCREEN_WIDTH-SCREEN_PAN;
        lcd_refresh();
#endif
        break;

    }

    // Clear Extend flag if set
    KBFlags &= ~KBExtend;

    // Ignore all other BREAK codes
    if( scancode & 0x80 ) return 0xFF;

    // Here the scancode is for something we can turn
    // into an ASCII value

    ascii = KBScanTable[scancode & 0x7F][KBIndexTab[KBFlags & KBIndex]];

    return ascii;
} /* KeyboardAscii */

//-----------------------------------------------------------------------------

int 
aaed2000_KeyboardTest(void)
{
    // If there is a pending character, return True
    if( KBPending != 0xFF ) return true;

    // If there is something waiting at the port, get it
    for(;;) {
        CYG_BYTE code, c;

        if (!KeyboardScan(&code)) {
            // No new activity
            break;
        }

        // Translate to ASCII
        c = KeyboardAscii(code);
		
        // if it is a real ASCII char, save it and
        // return True.
        if( c != 0xFF ) {
            KBPending = c;
            return true;
        }
    }

    // Otherwise return False
    return false;
	
} /* KeyboardTest */

int 
aaed2000_KeyboardGetc(void)
{
    unsigned char ch = KBPending;
    KBPending = 0xFF;
    return ch;
}

cyg_bool
aaed2000_KeyboardInit(void)
{
    int i;

    for (i = 0; i < AAED2000_KBD_COLS;  i++) {
        KB_rowState[i] = 0;
    }
    KBFlags = 0;

    return true;
} /* KeyboardInit */
