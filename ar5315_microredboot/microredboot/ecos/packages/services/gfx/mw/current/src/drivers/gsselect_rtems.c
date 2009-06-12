/*
/////////////////////////////////////////////////////////////////////////////
// $Header: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/packages/services/gfx/mw/current/src/drivers/gsselect_rtems.c#3 $
//
// Copyright (c) 2000 - Rosimildo da Silva
//  
// MODULE DESCRIPTION: 
// This module implements the "GsSelect()" function for Nano-X.
//
// MODIFICATION/HISTORY:
//
// $Log: gsselect_rtems.c,v $
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

/* defined in input_rtems.c */
extern struct MW_UID_MESSAGE m_kbd;
extern struct MW_UID_MESSAGE m_mou;


void
GsSelect(void)
{
  struct MW_UID_MESSAGE m;
  int rc;
  unsigned int timeout = 10;

  /* perform pre-select duties, if any*/
  if(scrdev.PreSelect)
  {
     scrdev.PreSelect(&scrdev);
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
        /* timeout handling */

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
        while(GsCheckMouseEvent())
             continue;
        break;

    /* KBD event */
    case MV_UID_KBD:
        m_kbd = m;
        GsCheckKeyboardEvent();
        break;

    /* micro-windows does nothing with those.. */
    case MV_UID_TIMER:
    case MV_UID_INVALID:
    default:
       ;
  }
}
