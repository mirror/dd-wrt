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

#include <ssdef.h>
#include <rmsdef.h>
#include <dvidef.h>
#include <jpidef.h>
#include <descrip.h>
#include <iodef.h>
#include <ttdef.h>
#include <tt2def.h>
#include <rms.h>
#include <errno.h>

#ifdef __DECC
#include <starlet.h>
#include <lib$routines>
#endif

#include "slang.h"
#include "_slang.h"

/* If this function returns -1, ^Y will be added to input buffer. */
int (*SLtty_VMS_Ctrl_Y_Hook) (void);

typedef struct {                /* I/O status block     */
        short i_cond;           /* Condition value      */
        short i_xfer;           /* Transfer count     */
        long  i_info;           /* Device information     */
} Iosb_Type;

typedef struct {                /* Terminal characteristics   */
        char  t_class;          /* Terminal class     */
        char  t_type;           /* Terminal type      */
        short t_width;          /* Terminal width in characters   */
        long  t_mandl;          /* Terminal's mode and length   */
        long  t_extend;         /* Extended terminal characteristics  */
}  TermChar_Type;

static TermChar_Type Old_Term_Char, New_Term_Char;

/* This serves to identify the channel we are reading input from.  */
static short This_Term;

typedef struct
{
   short buflen;
   short item_code;
   int *buf_addr;
   int *len_addr;
} item_list_3;

static int TTY_Inited;

/*
 *      Exit Handler Control Block
 */
static struct argument_block
  {
      int forward_link;
      int (*exit_routine)();
      int arg_count;
      int *status_address;
      int exit_status;
  }
exit_block =
  {
      0,
      NULL,
      1,
      &exit_block.exit_status,
      0
  };

static void vms_cancel_exithandler()
{
   sys$canexh(exit_block);
}

static int vms_exit_handler ()
{
   if (TTY_Inited == 0) return 0;
   SLang_reset_tty ();
   return 0;
}

static int vms_input_buffer;

static struct vms_ast_iosb
{
    short status;
    short offset;
    short termlen;
    short term;
} vms_ast_iosb;

static void vms_que_key_ast();
static int Ast_Fired_Event_Flag;
static int Timer_Event_Flag;
static int Event_Flag_Mask;
static int Ast_Stop_Input;
static int Waiting_For_Ast;

static int getkey_ast(int not_used)
{
   unsigned int c = 1000;

   if (vms_ast_iosb.offset)
     {
	c = (unsigned int) vms_input_buffer;
     }

   if (c <= 255)
     {
	if (c == SLang_Abort_Char)
	  {
	     if (SLang_Ignore_User_Abort == 0)
	       SLang_set_error (SL_USER_BREAK);
	     SLKeyBoard_Quit = 1;
	  }

	if ((c != 0x19)		       /* ^Y */
	    || (SLtty_VMS_Ctrl_Y_Hook == NULL)
	    || (-1 == (*SLtty_VMS_Ctrl_Y_Hook) ()))
	  {
	     if (SLang_Input_Buffer_Len < SL_MAX_INPUT_BUFFER_LEN - 3)
	       SLang_Input_Buffer[SLang_Input_Buffer_Len++] = c;
	  }
     }

   if (Waiting_For_Ast)  sys$setef (Ast_Fired_Event_Flag);
   Waiting_For_Ast = 0;
   vms_que_key_ast();
   return (1);
}

static void vms_que_key_ast()
{
   static int trmmsk [2] = { 0, 0 };
   int status;

   if (Ast_Stop_Input) return;
   status = sys$qio (0, This_Term,
		     IO$_READVBLK | IO$M_NOECHO | IO$_TTYREADALL,
		     &vms_ast_iosb, getkey_ast, 1,
		     &vms_input_buffer, 1, 0, trmmsk, 0, 0);
}

static char TTY_Name[8];
static int This_Process_Pid;

/* FIXME: priority=medium
 * The keypad state may have been tampered with by the application.  So, I
 * need to get the keypad status at initialization time and then reset it
 * in the call to SLang_reset_tty.  Unfortunately, this will most likely
 * involve interaction with the sldisply interface.
 */
int SLang_init_tty (int a, int flow, int out)
{
   Iosb_Type iostatus;
   int tmp, name_len, status, lastppid, ppid;
   item_list_3 itmlst[3];
   $DESCRIPTOR ( term, TTY_Name);

   itmlst[0].buflen = sizeof(int);
   itmlst[0].item_code = JPI$_PID;
   itmlst[0].buf_addr = &This_Process_Pid;
   itmlst[0].len_addr = &tmp;

   itmlst[1].buflen = 7;
   itmlst[1].item_code = JPI$_TERMINAL;
   itmlst[1].buf_addr = (int *) TTY_Name;
   itmlst[1].len_addr = &name_len;

   itmlst[2].buflen = 0;
   itmlst[2].item_code = 0;
   itmlst[2].buf_addr = 0;
   itmlst[2].len_addr = 0;

   if (a == -1) a = 3;		       /* ^C */
   SLang_Abort_Char = a;
   SLKeyBoard_Quit = 0;
   TTY_Inited = 1;
   ppid = 0, lastppid = -1;

   /* Here I get this process pid then I get the master process pid
      and use the controlling terminal of that process. */
   while (1)
     {
	status = sys$getjpiw(0,       /* event flag */
			     &ppid,   /* pid address */
			     0,       /* proc name address */
			     itmlst,
			     0, 0, 0);

	if (status != SS$_NORMAL)
	  {
	     fprintf(stderr, "PID: %X, status: %X\n", This_Process_Pid, status);
	     exit(1);
	  }

	if (lastppid == ppid) break;
	lastppid = ppid;

	itmlst[0].item_code =  JPI$_MASTER_PID;
	itmlst[0].buf_addr =  &ppid;
     }

   term.dsc$w_length = name_len;
   status = sys$assign ( &term, &This_Term, 0, 0 );
   if (status != SS$_NORMAL)
     {
	fprintf(stderr,"Unable to assign input channel\n");
	fprintf(stderr,"PID: %X, DEV %s, status: %d\n", This_Process_Pid, TTY_Name, status);
	exit(0);
     }

   if (NULL == exit_block.exit_routine)
     {
	exit_block.exit_routine = (int (*)()) vms_exit_handler;
	sys$dclexh(&exit_block);
     }

   /* allocate an event flag and clear it--- used by ast routines.  Since
    * I am only using a few local event flags, there is really no need to
    * worry about freeing these.
    *
    * The event flags are used to avoid timing problems with the getkey AST
    * as well as for a form of time out.
    */
   if (!Ast_Fired_Event_Flag) lib$get_ef (&Ast_Fired_Event_Flag);
   sys$clref (Ast_Fired_Event_Flag);

   if (!Timer_Event_Flag) lib$get_ef (&Timer_Event_Flag);
   sys$clref (Timer_Event_Flag);

   /* The working assumption here is that the event flags are in the same
    * cluster.  They need not be but it is very likely that they are.
    */
   Event_Flag_Mask = ((unsigned) 1 << (Ast_Fired_Event_Flag % 32));
   Event_Flag_Mask |= ((unsigned) 1 << (Timer_Event_Flag % 32));

   Waiting_For_Ast = 0;
   Ast_Stop_Input = 0;

   /* Get the startup terminal characteristics */
   status = sys$qiow(0,		       /* Wait on event flag zero      */
		     This_Term,	       /* Channel to input terminal    */
		     IO$_SENSEMODE,    /* Get current characteristic   */
		     &iostatus,	       /* Status after operation       */
		     0, 0,	       /* No AST service               */
                     &Old_Term_Char,   /* Terminal characteristics buf */
		     sizeof(Old_Term_Char),/* Size of the buffer           */
		     0, 0, 0, 0);

   New_Term_Char = Old_Term_Char;
   New_Term_Char.t_mandl |= TT$M_EIGHTBIT | TT$M_NOECHO;
   New_Term_Char.t_extend |= TT2$M_PASTHRU | TT2$M_XON;

   status = sys$qiow(0,		       /* Wait on event flag zero      */
		     This_Term,	       /* Channel to input terminal    */
		     IO$_SETMODE,      /* Set current characteristic   */
		     &iostatus,	       /* Status after operation       */
		     0, 0,	       /* No AST service               */
                     &New_Term_Char,   /* Terminal characteristics buf */
		     sizeof(New_Term_Char),/* Size of the buffer           */
		     0, 0, 0, 0);

   vms_que_key_ast();   /* set up the key ast */
   return 0;
}

static void cancel_ast (void)
{
   if (TTY_Inited == 0) return;

   /* stop the keyboard ast */
   sys$setast (0);		       /* disable AST delivery */
   sys$clref (Ast_Fired_Event_Flag);
   Waiting_For_Ast = 1;
   Ast_Stop_Input = 1;

   /* cancel all i/o on this channel.  This canels pending, as well as those
    * already in progress and queued.  In particular, according to the
    * manuals, cancelling I/O on the channel will cause the getkey AST
    * to fire even though the sys$qio call was aborted.  This is crucial
    * because below we wait for the AST to set the event flag.
    */
   sys$cancel (This_Term);
   sys$setast (1);		       /* enable ASTs again */
   sys$waitfr (Ast_Fired_Event_Flag);  /* sleep until it fires */
   Waiting_For_Ast = 0;
}

void SLang_reset_tty (void)
{
   Iosb_Type iostatus;

   if (!TTY_Inited) return;

   cancel_ast ();
   TTY_Inited = 0;

   /* reset the terminal characteristics */

   sys$qiow(0,			       /* event flag 0 */
	    This_Term,		       /* Channel to input terminal    */
	    IO$_SETMODE,	       /* Set current characteristic   */
	    &iostatus,		       /* Status after operation       */
	    0, 0,		       /* No AST service               */
	    &Old_Term_Char,	       /* Terminal characteristics buf */
	    sizeof(Old_Term_Char),     /* Size of the buffer           */
	    0, 0, 0, 0); 	       /* unused */

}

unsigned int _pSLsys_getkey()
{
   unsigned int c;

   if (SLKeyBoard_Quit) return((unsigned int) SLang_Abort_Char);

  /* On VMS, the keyboard ast routine should be stuffing the buffer, so
   do nothing except sleep */

   /* clear the flag which ast will set */
   Waiting_For_Ast = 0;

   if (SLang_Input_Buffer_Len) return(SLang_getkey());
   while (!_pSLsys_input_pending(450));
   c = SLang_getkey();
   return(c);
}

/* waits *secs tenth of seconds for input */
int _pSLsys_input_pending(int tsecs)
{
   unsigned long daytim[2];

   if (SLang_Input_Buffer_Len) return(SLang_Input_Buffer_Len);

   if (tsecs < 0)
     tsecs = -tsecs/100;	       /* tsecs is ms, convert to 1/10 sec */

   if (tsecs)
     {
	/* takes a quad word time.  If negative, use a relative time. */
	daytim[1] = 0xFFFFFFFF;
	daytim[0] = -(tsecs * 1000 * 1000);
	/* 1000 * 1000 is a tenth of a sec */

	sys$clref (Ast_Fired_Event_Flag);
	/* sys$clref (Timer_Event_Flag);  sys$setimr call clears this */

	/* set up a flag for the ast so it knows to set the event flag */
	Waiting_For_Ast = 1;

	sys$setimr(Timer_Event_Flag, daytim, 0, 1);

	/* this will return when ast does its job or timer expires.
	 * The first argument simply serves to identify the cluster for
	 * the event flag and that is all.  The second argument serves
	 * to identify the event flags to wait for.
	 */
	sys$wflor (Ast_Fired_Event_Flag, Event_Flag_Mask);

	Waiting_For_Ast = 0;

	/* cancel the timer */
	sys$cantim(1, 3);   /* 3 is user mode */
     }
   return (SLang_Input_Buffer_Len);
}

int SLang_set_abort_signal (void (*f)(int))
{
   return 0;
}

