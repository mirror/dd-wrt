/*
/////////////////////////////////////////////////////////////////////////////
// $Header: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/packages/services/gfx/mw/current/src/drivers/mwselect_rtems.c#3 $
//
// Copyright (c) 2000 - Rosimildo da Silva
//  
// MODULE DESCRIPTION: 
// This module implements the "GsSelect()" function for MicroWindows.
//
// MODIFICATION/HISTORY:
//
// $Log: mwselect_rtems.c,v $
// Revision 1.1.1.1  2001/06/21 06:32:41  greg
// Microwindows pre8 with patches
//
// Revision 1.1.1.1  2001/06/05 03:44:01  root
// First import of 5/5/2001 Microwindows to CVS
//
//
/////////////////////////////////////////////////////////////////////////////
*/
#include <stdio.h>
#include <errno.h>

#include <rtems/mw_uid.h>
#include "device.h"
#include "windef.h"

extern MWBOOL MwCheckMouseEvent();
extern MWBOOL MwCheckKeyboardEvent();

#if ANIMATEPALETTE
static int fade = 0;
#endif

extern struct MW_UID_MESSAGE m_kbd;
extern struct MW_UID_MESSAGE m_mou;

extern HWND  dragwp;	 /* window user is dragging*/
/*
 * "Select() routine called by the Microwindows framework to receive events 
 * from the input devices.
 */
void MwSelect(void)
{
  struct MW_UID_MESSAGE m;
  int rc;
  unsigned int timeout = 0;

  /* perform pre-select duties, if any*/
  if(scrdev.PreSelect)
  {
     scrdev.PreSelect(&scrdev);
  }
	/* Set up the timeout for the main select().  If
	 * the mouse is captured we're probably moving a window,
	 * so poll quickly to allow other windows to repaint while
	 * checking for more event input.
	 */
	if( !dragwp )
   {
		timeout = MwGetNextTimeoutValue();	/* returns ms*/
#if ANIMATEPALETTE
		if(fade < 100)
			timeout = 40;
		else 
#endif
		if(timeout == 0)
			timeout = 10;	/* 10ms required for vt fb switch*/
	}

  /* let's make sure that the type is invalid */
  m.type = MV_UID_INVALID;

  /* wait up to 100 milisecons for events */
  rc = uid_read_message( &m, timeout );

  /* return if timed-out or something went wrong */
  if( rc < 0 )
  {
     if( errno != ETIMEDOUT )
        EPRINTF( " rc= %d, errno=%d\n", rc, errno );
     else
     {
#if ANIMATEPALETTE
       if(fade <= 100) {
		   	setfadelevel( &scrdev, fade );
			   fade += 5;
		 }
#endif
		MwHandleTimers();
     }
     return;
  }

  /* let's pass the event up to microwindows */
  switch( m.type )
  {
    /* Mouse or Touch Screen event */
    case MV_UID_REL_POS:
    case MV_UID_ABS_POS:
        m_mou = m;
        while( MwCheckMouseEvent() )
  		         continue;
        break;

    /* KBD event */
    case MV_UID_KBD:
        m_kbd = m;
        MwCheckKeyboardEvent();
        break;

    /* micro-windows does nothing with those.. */
    case MV_UID_TIMER:
    case MV_UID_INVALID:
    default:
       ;
  }
}

