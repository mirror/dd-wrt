/*
Copyright (C) 2004-2011 John E. Davis

This file is part of the S-Lang Library.

The S-Lang Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The S-Lang Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
USA.
*/

#include "slinclud.h"

#ifdef __DJGPP__
# define _NAIVE_DOS_REGS
#endif

#include <dos.h>

#if defined (__EMX__)
#  define int86		_int86
#  define delay		_sleep2
#endif	/* __EMX__ */

#if defined (__WATCOMC__)
#  include <conio.h>
#  include <bios.h>
#  define int86	int386
#endif

#if defined (__DJGPP__)
# include <sys/farptr.h>
# include <go32.h>
# include <bios.h>
#endif

#ifndef _NKEYBRD_READ
# define _NKEYBRD_READ		0x0
#endif
#ifndef _NKEYBRD_READY
# define _NKEYBRD_READY		0x1
#endif
#ifndef _NKEYBRD_SHIFTSTATUS
# define _NKEYBRD_SHIFTSTATUS	0x2
#endif

#define BIOSKEY		slbioskey
#if defined(__WATCOMC__)
# define keyWaiting() _bios_keybrd(_NKEYBRD_READY)
#else
# define keyWaiting()	BIOSKEY(_NKEYBRD_READY)
#endif

#ifndef __EMX__
# define USE_MOUSE_CODE 1
#else
# define USE_MOUSE_CODE 0
#endif

#include "slang.h"
#include "_slang.h"

#ifdef __cplusplus
# define _DOTS_ ...
#else
# define _DOTS_ void
#endif

#if !defined (__EMX__) && !defined (__GO32__) && !defined (__WATCOMC__)
#define HAS_INT9
#endif

#ifdef __GO32__
# include <signal.h>
#endif

#if defined (HAS_INT9)
static void interrupt (*int9_old) (_DOTS_);
static unsigned char far *shift = (unsigned char far *) 0x417;
static unsigned int Abort_Scan_Code = 34;  /* 34 = scan code for ^G */

/*----------------------------------------------------------------------*\
 * an interrupt 9 handler, not for use with most 32 bit compilers
\*----------------------------------------------------------------------*/
static void interrupt int9_handler (_DOTS_)
{
   unsigned char s, s1;

   s1 = *shift & 0xF;		/* ignore caps, ins, num lock, scroll lock */
   s = inp (0x60);
   if (s1 & 0x04)		/* control key */
     {
	if (s == Abort_Scan_Code)
	  {
	     if (SLang_Ignore_User_Abort == 0) SLang_Error = SL_USER_BREAK;
	     SLKeyBoard_Quit = 1;
	  }
     }
   (*int9_old) ();
}
#endif	/* HAS_INT9 */

static void int9_change (int set)
{
#if defined (HAS_INT9)
   if (set)			/* install a new handler */
     {
	if (int9_old != NULL) return;
	int9_old = getvect (9);
	setvect (9, int9_handler);
     }
   else	if (int9_old != NULL)	/* restore the old handler */
     {
	setvect (9, int9_old);
	int9_old = NULL;
     }
#else
   (void) set;
#endif	/* HAS_INT9 */
}

/*----------------------------------------------------------------------*\
 *  Function:	static void set_ctrl_break (int state);
 *
 * set the control-break setting
\*----------------------------------------------------------------------*/
static void set_ctrl_break (int state)
{
#if defined (__EMX__)
   (void) state;		/* not really required */
#else	/* __EMX__ */

   static int prev = 0;

# if defined (__GO32__)
   if (state == 0)
     {
#  if __DJGPP__ >= 2
	signal (SIGINT, SIG_IGN);
#  endif
	prev = getcbrk ();
	setcbrk (0);
     }
   else
     {
#  if __DJGPP__ >= 2
	signal (SIGINT, SIG_DFL);
#  endif
	setcbrk (prev);
     }
# else	/* __GO32__ */
#  if defined(__WATCOMC__)
   fprintf (stderr, "Have not yet defined set_ctrl_break for __WATCOMC__\n");
   prev = state;
#  else
   asm  mov dl, byte ptr prev
     asm  mov ax, state
     asm  cmp ax, 0
     asm  jne L1
     asm  mov ah, 33h
     asm  mov al, 0
     asm  mov dl, byte ptr prev
     asm  int 21h
     asm  xor ax, ax
     asm  mov al, dl
     asm  mov prev, ax
     asm  mov dl, 0
     L1:
   asm  mov al, 1
     asm  mov ah, 33h
     asm  int 21h
#  endif	/* __WATCOMC__ */
# endif	/* __GO32__ */
#endif	/* __EMX__ */
}

/*----------------------------------------------------------------------*\
 * static unsigned int slbioskey (int op);
 *
 * op 0-2 (standard) and 0x10-0x12 (extended) are valid
 *
 * 0, 0x10	_NKEYBRD_READ	- read the key
 * 1, 0x11	_NKEYBRD_READY	- check if a key is waiting
 * 		if so give a peek of its value, otherwise return 0
 * 2, 0x12	_NKEYBRD_SHIFTSTATUS	- get shift flags
 *		(Ins, Cap, Num, Scroll, Alt, ^Ctrl L_shift, R_shift)
 *		flags = ICNSA^LR	only the lower byte is valid!
\*----------------------------------------------------------------------*/
static int bios_key_f = 0;
static unsigned int slbioskey (int op)
{
   union REGS r;
   r.h.ah = (op & 0x03) | bios_key_f;
   int86 (0x16, &r, &r);
#if defined(__WATCOMC__)
   /* return (_bios_keybrd ((op & 0x03) | bios_key_f)); */
# if 1			/* the correct zero flag for watcom? */
   /* is zero flag set? (no key waiting) */
   if ((op & _NKEYBRD_READY) && (r.x.cflag & 0x40) == 0x40) return 0;
# else			/* the correct zero flag for watcom? */
   /* is zero flag set? (no key waiting) */
   if ((op & _NKEYBRD_READY) && (r.x.cflag & 0x4)) return 0;
# endif
   return (r.x.eax & 0xffff);
#else
   /* is zero flag set? (no key waiting) */
   if (op & _NKEYBRD_READY)
     {
	if ((r.x.flags & 0x40) == 0x40)
	  return 0;
	if (r.x.ax == 0)		       /* CTRL-BREAK */
	  return -1;
     }
   return (r.x.ax & 0xffff);
#endif
}

#if USE_MOUSE_CODE
/*----------------------------------------------------------------------*\
 * Simple mouse routines for 16/32-bit DOS-targets.
 * Gisle Vanem <giva@bgnett.no>
\*----------------------------------------------------------------------*/

#define HARD_MOUSE_RESET 0

static int Have_Mouse = 0;
static int Process_Mouse_Events = 0;

/*----------------------------------------------------------------------*\
 * peem_far_mem()
 *
\*----------------------------------------------------------------------*/
static unsigned long peek_dos_mem (unsigned long dos_addr,
                                   unsigned char *pentry)
{
  unsigned long vector;
  unsigned char entry;

#if defined(__DJGPP__)
# define MAKE_LINEAR(seg,ofs) ((unsigned long)(((seg) << 4) + (ofs)))
  vector = _farpeekl (_dos_ds, dos_addr);
  entry  = _farpeekb (_dos_ds, MAKE_LINEAR(vector >> 16, vector & 0xffff));

#elif defined(__EMX__)
  vector = 0;
  entry  = 0;  /* to-do!! */

#elif defined(__WATCOMC__) && defined(__FLAT__)  /* wcc386 */
  vector = *(unsigned long*) dos_addr;
  entry  = *(unsigned char*) vector;
#else
  vector = *(unsigned long far*) dos_addr;
  entry  = *(unsigned char far*) vector;
#endif

   if (pentry)
     *pentry = entry;
   return (vector);
}

/*----------------------------------------------------------------------*\
 * mouse_pressed (int button, int *x_pos, int *y_pos)
 *
 * Return 1 if left (0) or right (1) mouse-button was pressed.
\*----------------------------------------------------------------------*/
static int mouse_pressed (int button, int *x_pos, int *y_pos)
{
   union REGS r;

   r.x.ax = 5;
   r.x.bx = button;
   int86 (0x33, &r, &r);
   *x_pos = r.x.cx;
   *y_pos = r.x.dx;
   return (r.x.bx);
}

/*----------------------------------------------------------------------*\
 * mouse_show (int show)
 *
 * Show (show=1) or hide (show=0) the mouse cursor
\*----------------------------------------------------------------------*/
static int mouse_show (int show)
{
   union REGS r;

   if (Have_Mouse == 0)
     return -1;

   r.x.ax = show ? 1 : 2;
   int86 (0x33, &r, &r);
   return 0;
}

/*----------------------------------------------------------------------*\
 * mouse_exit (void)
 *
 * Do a soft-reset of the mouse-driver (hides cursor)
\*----------------------------------------------------------------------*/
static void mouse_exit (void)
{
   union REGS r;
   r.x.ax = 0x21;
   int86 (0x33, &r, &r);
}

/*----------------------------------------------------------------------*\
 * mouse_init (void)
 *
 * Peek at mouse interrupt vector for a driver.
 * Do a soft/hard-reset of the mouse-driver.
 * Add a SLang atexit function
\*----------------------------------------------------------------------*/
static int mouse_init (void)
{
   union REGS r;
   unsigned char entry  = 0;
   unsigned long vector = peek_dos_mem (4*0x33, &entry);

   if (!vector || entry == 0xCF) /* NULL or points to IRET */
     return -1;

#if HARD_MOUSE_RESET
   r.x.ax = 0;              /* mouse hard-reset and reinit */
#else
   r.x.ax = 0x21;           /* mouse soft-reset and reinit */
#endif

   int86 (0x33, &r, &r);
   if (r.x.ax != 0xFFFF)
     return -1;

   (void) SLang_add_cleanup_function (mouse_exit);
   Have_Mouse = 1;
   return 0;
}

/*----------------------------------------------------------------------*\
 * static int mouse_get_event (void);
 *
 * Poll mouse for changed button-state and encode x/y position and
 * button state into an escape sequence "\e[M.."
\*----------------------------------------------------------------------*/
static int mouse_get_event (void)
{
   char buf [6];
   int  x, y;

   if (!Have_Mouse || Process_Mouse_Events == 0)
      return (0);

   if (mouse_pressed(0, &x, &y))       /* left button pressed? */
     buf[3] = 040;
   else if (mouse_pressed(1, &x, &y))  /* right button pressed */
     buf[3] = 041;
   else return (0);

#if 0  /* test */
   fprintf (stderr, "mouse_get_event: x=%d, y=%d\n", x, y);
#endif

   /*
    * Taken from slw32tty.c / process_mouse_event():
    *
    * We have a simple press or release.  Encode it as an escape sequence
    * and buffer the result.  The encoding is:
    *   'ESC [ M b x y'
    *  where b represents the button state, and x,y represent the coordinates.
    * The ESC is handled by the calling routine.
    */
   buf[0] = 27;
   buf[1] = '[';
   buf[2] = 'M';
   buf[4] = 1 + ' ' + (x >> 3); /* textmode co-ordinates are 1/8th of */
   buf[5] = 1 + ' ' + (y >> 3); /* graphics-mode co-ordinates */

   if (SLang_buffer_keystring (buf, sizeof(buf)) < 0)
     return (0);
   return (1);
}

#endif				       /* USE_MOUSE_CODE */

/*----------------------------------------------------------------------*\
 *  Function:	int SLang_init_tty (int abort_char, int no_flow_control,
 *				    int opost);
 *
 * initialize the keyboard interface and attempt to set-up the interrupt 9
 * handler if ABORT_CHAR is non-zero.
 * NO_FLOW_CONTROL and OPOST are only for compatiblity and are ignored.
\*----------------------------------------------------------------------*/
int SLang_init_tty (int abort_char, int no_flow_control, int opost)
{
   (void) no_flow_control;
   (void) opost;

   SLKeyBoard_Quit = 0;

   bios_key_f = 0x10;		/* assume it's an enhanced keyboard */
#if defined (HAS_INT9)
   bios_key_f &= peekb (0x40,0x96);	/* verify it's true */
   if (abort_char > 0) Abort_Scan_Code = (unsigned int) abort_char;
#else
   (void) abort_char;
#endif

   set_ctrl_break (0);

   return 0;
}

/*----------------------------------------------------------------------*\
 *  Function:	void SLang_reset_tty (void);
 *
 * reset the tty before exiting
\*----------------------------------------------------------------------*/
void SLang_reset_tty (void)
{
   int9_change (0);
   set_ctrl_break (1);
}

/*----------------------------------------------------------------------*\
 *  Function:	int _pSLsys_input_pending (int tsecs);
 *
 *  sleep for *tsecs tenths of a sec waiting for input
\*----------------------------------------------------------------------*/
int _pSLsys_input_pending (int tsecs)
{
   if (keyWaiting()) return 1;

   /* Convert tsecs to units of 20 ms */
   tsecs = tsecs * 5;

   /* If tsecs is less than 0, it represents millisecs */
   if (tsecs < 0)
     tsecs = -tsecs / 100;

   while ((tsecs > 0) && (SLang_Input_Buffer_Len == 0))
     {
	delay (20);	/* 20 ms or 1/50 sec */
#if USE_MOUSE_CODE
	if (1 == mouse_get_event ())
	  return SLang_Input_Buffer_Len;
#endif
	if (keyWaiting()) break;
	tsecs--;
     }
   return (tsecs);
}

/*----------------------------------------------------------------------*\
 *  Function:	unsigned int _pSLsys_getkey (void);
 *
 * Wait for and get the next available mouse-event / keystroke.
 * Also re-maps some useful keystrokes.
 *
 *	Backspace (^H)	=>	Del (127)
 *	Ctrl-Space	=>	^@	(^@^3 - a pc NUL char)
 *	extended keys are prefixed by a null character
\*----------------------------------------------------------------------*/
unsigned int _pSLsys_getkey (void)
{
   unsigned int key, scan, ch, shift;

   while ((SLang_Input_Buffer_Len == 0)
	  && (0 == _pSLsys_input_pending (300)))
     ;

   if (SLang_Input_Buffer_Len)
     return SLang_getkey ();

   key  = BIOSKEY(_NKEYBRD_READ);
   ch   = key & 0xff;
   scan = key >> 8;
   shift = BIOSKEY(_NKEYBRD_SHIFTSTATUS) & 0xf;

   if (key == 0x0e08)
     return 127;			/* Backspace key */

   switch (ch)
     {
      case 32:
	if (0 == (shift & 0x04))
	  break;
	/* ^space = ^@ */
	scan = 3;		/* send back Ctrl-@ => ^@^C */
	/* drop */
      case 0xe0:
      case 0:			/* extended key code */
	ch = _pSLpc_convert_scancode (scan, 0, 1);
     }
   return (ch);
}

/*----------------------------------------------------------------------*\
 *  Function:	void SLang_set_abort_signal (void (*handler)(int));
\*----------------------------------------------------------------------*/
int SLang_set_abort_signal (void (*handler)(int))
{
   if (handler == NULL) int9_change (1);
   return 0;
}

int SLtt_set_mouse_mode (int mode, int force)
{
#if USE_MOUSE_CODE
   (void) force;

   if ((Have_Mouse == 0)
       && (-1 == mouse_init ()))
   {
      Process_Mouse_Events = 0;
      return -1;
   }

   Process_Mouse_Events = mode;
   return mouse_show (mode);
#else
   (void) mode;
   (void) force;

   return -1;
#endif
}
