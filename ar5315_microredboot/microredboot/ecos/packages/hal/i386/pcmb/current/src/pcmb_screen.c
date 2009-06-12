//=============================================================================
//
//      pcmb_screen.c
//
//      HAL diagnostic output code
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
// Author(s):   proven
// Contributors:proven
// Date:        1998-10-05
// Purpose:     HAL diagnostic output
// Description: Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <pkgconf/hal_i386_pcmb.h>

#if CYGINT_HAL_I386_PCMB_SCREEN_SUPPORT

#include <cyg/infra/cyg_type.h>         // base types

#include <cyg/hal/hal_arch.h>           // basic machine info
#include <cyg/hal/hal_intr.h>           // interrupt macros
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_if.h>             // interface API
#include <cyg/hal/hal_misc.h>

#include <cyg/hal/pcmb_serial.h>

// Index into pc_ser_channels[] for screen entry.
#define PCMB_PORT_INDEX (CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS - 1)

//-----------------------------------------------------------------------------
// Screen output definitions...

static short 		*DisplayBuffer = (short *)0xB8000;
static short		DisplayAttr = 0x0700;

static	short		DisplayPort = 0x03d4;

static	int		XPos;
static	int		YPos;

static	int		ScreenWidth = 80;
static  int		ScreenLength = 25;

//-----------------------------------------------------------------------------

static void MoveLine
(
	short 	*dest,
	short 	*src,
	int	count		
)
{

	while( count-- ) *dest++ = *src++;
	
} /* MoveLine */

//-----------------------------------------------------------------------------

static void FillLine
(
	short  *dest,
	short  val,
	int    count
)
{
	while( count-- ) *dest++ = val;
	
} /* FillLine */

//-----------------------------------------------------------------------------

void ClearScreen(void)
{
    FillLine(DisplayBuffer, ' ' | DisplayAttr, ScreenWidth*ScreenLength);
	
} /* ClearScreen */

void MoveCursor
(
	void
)
{
	int pos = XPos + YPos * ScreenWidth;

	HAL_WRITE_UINT8(DisplayPort, 0x0e );
	HAL_WRITE_UINT8(DisplayPort+1, pos >> 8 );

	HAL_WRITE_UINT8(DisplayPort, 0x0f );
	HAL_WRITE_UINT8(DisplayPort+1, pos & 0xFF );
	
} /* MoveCursor */

//-----------------------------------------------------------------------------

void ScrollUp
(
	int	lines
)
{
//	Report_Function(ScrollUp)

	int rest = ScreenLength - lines;
	
	MoveLine
	(
		DisplayBuffer,
		DisplayBuffer+(lines*ScreenWidth),
		rest*ScreenWidth
	);

	FillLine
	(
		DisplayBuffer+(rest*ScreenWidth),
		' ' | DisplayAttr,
		lines*ScreenWidth
	);
	
} /* ScrollUp */

//-----------------------------------------------------------------------------

void ScrollDown
(
	int	lines
)
{
//	Report_Function(ScrollDown)

	int rest = ScreenLength - lines;
	short *db = DisplayBuffer+(ScreenWidth*(ScreenLength-1));
	
	while( rest )
	{
		MoveLine
		(
			db,
			db-ScreenWidth,
			ScreenWidth
		);

		rest--;
		db -= ScreenWidth;
	}

	FillLine
	(
		DisplayBuffer,
		' ' | DisplayAttr,
		lines*ScreenWidth
	);
	
} /* ScrollDown */

//-----------------------------------------------------------------------------

void NewLine
(
	void
)
{

	XPos = 0;
	YPos++;
	
	if( YPos >= ScreenLength )
	{
		YPos = ScreenLength-1;
		ScrollUp(1);
	}

	MoveCursor();
	
} /* NewLine */

//-----------------------------------------------------------------------------

void DisplayChar
(
	char	ch
)
{

	DisplayBuffer[XPos + YPos*ScreenWidth] = ch | DisplayAttr;

	XPos++;

	if( XPos >= ScreenWidth )
	{
		XPos = 0;
		YPos++;
		if( YPos >= ScreenLength )
		{
			YPos = ScreenLength-1;
			ScrollUp(1);
		}
	}

	MoveCursor();
	
} /* DisplayChar */

//-----------------------------------------------------------------------------
// Keyboard definitions

#define	KBDATAPORT	0x0060		// data I/O port
#define	KBCMDPORT	0x0064		// command port (write)
#define	KBSTATPORT	0x0064		// status port	(read)

// Scan codes

#define	LSHIFT		0x2a
#define	RSHIFT		0x36
#define	CTRL		0x1d
#define	ALT		    0x38
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

//-----------------------------------------------------------------------------
// Keyboard Variables

static	int	KBFlags = 0;

static	CYG_BYTE	KBPending = 0xFF;

static	CYG_BYTE	KBScanTable[128][4] =
{
//	Normal		Shift		Control		Alt
// 0x00
{	0xFF,		0xFF,		0xFF,		0xFF,   },
{	0x1b,		0x1b,		0x1b,		0xFF,	},
{	'1',		'!',		0xFF,		0xFF,	},
{	'2',		'"',		0xFF,		0xFF,	},
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
{	0x27,		'@',		0xFF,		0xFF,	},
{	'#',		'~',		0xFF,		0xFF,	},
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
{	'*',		0xFF,		0xFF,		0xFF,	},
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
{	'7',		0xFF,		0xFF,		0xFF,	},

{	'8',		0x15,		0x15,		0x15,	},
{	'9',		0x10,		0x10,		0x10,	},
{	'-',		0xFF,		0xFF,		0xFF,	},
{	'4',		0xFF,		0xFF,		0xFF,	},
{	'5',		0xFF,		0xFF,		0xFF,	},
{	'6',		0xFF,		0xFF,		0xFF,	},
{	'+',		0xFF,		0xFF,		0xFF,	},
{	'1',		0xFF,		0xFF,		0xFF,	},
// 0x50
{	'2',		0x04,		0x04,		0x04,	},
{	'3',		0x0e,		0x0e,		0x0e,	},
{	'0',		0xFF,		0xFF,		0xFF,	},
{	'.',		0xFF,		0xFF,		0xFF,	},
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

static void KeyboardInit
(
	void
)
{
    KBFlags = 0;
	
} /* KeyboardInit */

//-----------------------------------------------------------------------------

static CYG_BYTE KeyboardAscii
(
	CYG_BYTE	scancode
)
{
    CYG_BYTE ascii = 0xFF;

    // Start by handling all shift/ctl keys:

    switch( scancode )
    {
    case 0x53:  // delete
        if (KBFlags & KBCtrl && KBFlags & KBAlt)
        {
            CYGACC_CALL_IF_RESET();
        }
        break;

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

static int KeyboardTest
(
	void
)
{
    // If there is a pending character, return True
    if( KBPending != 0xFF ) return true;


    // If there is something waiting at the port, get it
    for(;;) 
    {
        CYG_BYTE stat, code;
        CYG_BYTE c;
        
        HAL_READ_UINT8( KBSTATPORT, stat );

        if( (stat & 0x01) == 0 )
            break;

        HAL_READ_UINT8( KBDATAPORT, code );

        // Translate to ASCII
        c = KeyboardAscii(code);
		
        // if it is a real ASCII char, save it and
        // return True.
        if( c != 0xFF )
        {
            KBPending = c;
            return true;
        }

    }

    // Otherwise return False
    return false;
	
} /* KeyboardTest */

//=============================================================================
// Basic IO functions

static void
cyg_hal_plf_screen_init_channel(void* __ch_data)
{
    KeyboardInit();
    
    XPos	= 0;
    YPos	= 0;

    ClearScreen();
    MoveCursor();
}


void
cyg_hal_plf_screen_putc(void *__ch_data, char ch)
{
    CYGARC_HAL_SAVE_GP();

	switch( ch )
	{
	case '\n':
		NewLine();
		return;

	case '\r':
		XPos = 0;
		MoveCursor();
		return;

	case '\b':
		if( XPos == 0 ) return;
		XPos--;
		MoveCursor();
		return;

	case '\t':
		do
		{
			DisplayChar(' ');
		} while( (XPos % 8) != 0 );
		return;

	case 0x0c:
		ClearScreen();
		XPos = YPos = 0;
		MoveCursor();
		return;		

	case 1:
		ScrollUp(1);
		XPos = 0;
		YPos = ScreenLength-1;
		return;

	case 2:
		ScrollDown(1);
		XPos = 0;
		YPos = 0;
		return;
	
		
	default:
		DisplayChar(ch);
		return;
	}

    CYGARC_HAL_RESTORE_GP();
}

static cyg_bool
cyg_hal_plf_screen_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    if( !KeyboardTest() )
        return false;

    *ch = KBPending;
    KBPending = 0xFF;

    return true;
}

cyg_uint8
cyg_hal_plf_screen_getc(void* __ch_data)
{
    cyg_uint8 ch;
    CYGARC_HAL_SAVE_GP();

    while(!cyg_hal_plf_screen_getc_nonblock(__ch_data, &ch));

    CYGARC_HAL_RESTORE_GP();

    return ch;
}



//=============================================================================
// Call IF support

#if defined(CYGSEM_HAL_VIRTUAL_VECTOR_DIAG) \
    || defined(CYGPRI_HAL_IMPLEMENTS_IF_SERVICES)

static void
cyg_hal_plf_screen_write(void* __ch_data, const cyg_uint8* __buf, 
                         cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        cyg_hal_plf_screen_putc(__ch_data, *__buf++);

    CYGARC_HAL_RESTORE_GP();
}

static void
cyg_hal_plf_screen_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        *__buf++ = cyg_hal_plf_screen_getc(__ch_data);

    CYGARC_HAL_RESTORE_GP();
}

cyg_bool
cyg_hal_plf_screen_getc_timeout(void* __ch_data, cyg_uint8* ch)
{
    int delay_count;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_bool res;
    CYGARC_HAL_SAVE_GP();

    delay_count = chan->msec_timeout * 10; // delay in .1 ms steps

    for(;;) {
        res = cyg_hal_plf_screen_getc_nonblock(__ch_data, ch);
        if (res || 0 == delay_count--)
            break;
        
        CYGACC_CALL_IF_DELAY_US(100);
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

static int
cyg_hal_plf_screen_control(void *__ch_data, __comm_control_cmd_t __func, ...)
{
    static int irq_state = 0;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    int ret = 0;
    CYGARC_HAL_SAVE_GP();

    switch (__func) {
    case __COMMCTL_IRQ_ENABLE:
        irq_state = 1;

        HAL_INTERRUPT_UNMASK(chan->isr_vector);
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;

        HAL_INTERRUPT_MASK(chan->isr_vector);
        break;
    case __COMMCTL_DBG_ISR_VECTOR:
        ret = chan->isr_vector;
        break;
    case __COMMCTL_SET_TIMEOUT:
    {
        va_list ap;

        va_start(ap, __func);

        ret = chan->msec_timeout;
        chan->msec_timeout = va_arg(ap, cyg_uint32);

        va_end(ap);
    }        
    default:
        break;
    }
    CYGARC_HAL_RESTORE_GP();
    return ret;
}

static int
cyg_hal_plf_screen_isr(void *__ch_data, int* __ctrlc, 
                       CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
    int res = 0;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    char c;

    CYGARC_HAL_SAVE_GP();

    cyg_drv_interrupt_acknowledge(chan->isr_vector);

    *__ctrlc = 0;

    if ( KeyboardTest() ) {

        c = KBPending;
        KBPending = 0xFF;
        if( cyg_hal_is_break( &c , 1 ) )
            *__ctrlc = 1;

        res = CYG_ISR_HANDLED;
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

void cyg_hal_plf_screen_init(void)
{
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

    // Disable interrupts.
    HAL_INTERRUPT_MASK(pc_ser_channels[PCMB_PORT_INDEX].isr_vector);

    // Init channels
    cyg_hal_plf_screen_init_channel(&pc_ser_channels[PCMB_PORT_INDEX]);

    // Setup procs in the vector table

    // Set channel 2
    CYGACC_CALL_IF_SET_CONSOLE_COMM(PCMB_PORT_INDEX);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &pc_ser_channels[PCMB_PORT_INDEX]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_screen_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_screen_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_screen_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_screen_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_screen_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_screen_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_screen_getc_timeout);

    // Restore original console
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);
}

#endif //  defined(CYGSEM_HAL_VIRTUAL_VECTOR_DIAG)
       // || defined(CYGPRI_HAL_IMPLEMENTS_IF_SERVICES)


//-----------------------------------------------------------------------------

#endif // CYGINT_HAL_I386_PCMB_SCREEN_SUPPORT

//-----------------------------------------------------------------------------
// End of pcmb_screen.c
