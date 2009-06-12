/*
/////////////////////////////////////////////////////////////////////////////
// $Header: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/packages/services/gfx/mw/current/src/drivers/input_rtems.c#3 $
//
// Copyright (c) 2000 - Rosimildo da Silva
//  
// MODULE DESCRIPTION: 
// This module implements the Microwindows Drivers for systems that implements 
// the Micro Input Device interface. This driver is not specific in any way
// to RTEMS. It could be used with any sustem that implements such interface.
//
// The skeleton of the drivers were based on standard Microwindows drivers.
//
// MODIFICATION/HISTORY:
//
// $Log: input_rtems.c,v $
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
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

#include <rtems/mw_uid.h>
#include "device.h"
#include "windef.h"   /* UCHAR */


extern int close( int fd ); /* RTEMS does not include close() in stdio.h */

#define    SCALE        3    /* default scaling factor for acceleration */
#define    THRESH       5    /* default threshhold for acceleration */

/* prototypes of the mouse driver */
static int      MWMou_Open(MOUSEDEVICE *pmd);
static void     MWMou_Close(void);
static int      MWMou_GetButtonInfo(void);
static void     MWMou_GetDefaultAccel(int *pscale,int *pthresh);
static int      MWMou_Read(MWCOORD *dx, MWCOORD *dy, MWCOORD *dz, int *bp);

/* prototypes of the Kbd driver */
static int     MWKbd_Open(KBDDEVICE *pkd);
static void    MWKbd_Close(void);
static void    MWKbd_GetModifierInfo(int *modifiers);
static int     MWKbd_Read(MWUCHAR *buf, int *modifiers);


MOUSEDEVICE mousedev = 
{
    MWMou_Open,
    MWMou_Close,
    MWMou_GetButtonInfo,
    MWMou_GetDefaultAccel,
    MWMou_Read,
    NULL
};


KBDDEVICE kbddev = {
    MWKbd_Open,
    MWKbd_Close,
    MWKbd_GetModifierInfo,
    MWKbd_Read,
    NULL
};

struct MW_UID_MESSAGE m_kbd = { 0 };
struct MW_UID_MESSAGE m_mou = { 0 };


static int mou_fd = -1;
static int kbd_fd   = -1;

static const char *Q_NAME        = "MWQ";
#define            Q_MAX_MSGS      128
#define            MOUSE_DEVICE    "/dev/mouse"


/* Open and register driver */
static int open_queue_and_register_driver( int fd )
{
   int rc;
   rc = uid_open_queue( Q_NAME, O_CREAT | O_RDWR, Q_MAX_MSGS );
   if( rc ) 
   {
      return rc;
   }
   return uid_register_device( fd, Q_NAME );
}

/* close and unregister device */
static int close_queue_and_unregister_device( int fd )
{
    uid_unregister_device( fd );
    return uid_close_queue();
}  


/*
 * Open up the mouse device.
 */
static int
MWMou_Open(MOUSEDEVICE *pmd)
{
   int rc;
   /* no valid event */
   m_mou.type = MV_UID_INVALID;
   mou_fd = open( MOUSE_DEVICE, O_NONBLOCK );
   /* Open your mouse device here */
   rc = open_queue_and_register_driver( mou_fd );
   if( rc )
      return -1;
   return 2;
}

/*
 * Close the mouse device.
 */
static void
MWMou_Close(void)
{
   close_queue_and_unregister_device( mou_fd );
   close( mou_fd );
}

/*
 * Get mouse buttons supported
 */
static int
MWMou_GetButtonInfo(void)
{
   return 0;
}

/*
 * Get default mouse acceleration settings
 */
static void
MWMou_GetDefaultAccel(int *pscale,int *pthresh)
{
    *pscale = SCALE;
    *pthresh = THRESH;
}

/*
 * Attempt to read bytes from the mouse and interpret them.
 * Returns -1 on error, 0 if either no bytes were read or not enough
 * was read for a complete state, or 1 if the new state was read.
 * When a new state is read, the current buttons and x and y deltas
 * are returned.  This routine does not block.
 */
static int
MWMou_Read(MWCOORD *dx, MWCOORD *dy, MWCOORD *dz, int *bp)
{
   /* check if a new mouse event has been posted */
   if( m_mou.type != MV_UID_INVALID )
   {
      /* check which return to send up ... */
      int rc = ( m_mou.type == MV_UID_REL_POS ) ? 1 : 2;

      *bp = m_mou.m.pos.btns;
      *dx = m_mou.m.pos.x;
      *dy = m_mou.m.pos.y;
      *dz = m_mou.m.pos.z;
      /* consume event */
      m_mou.type = MV_UID_INVALID;
      return rc;
   }
   return 0;
}



/*
 * Open the keyboard.
 */
static int
MWKbd_Open(KBDDEVICE *pkd)
{
   int rc;
   /* no valid event */
   m_kbd.type = MV_UID_INVALID;
  /* kbd it is already opened */
   kbd_fd = fileno( stdin );
   /* register kbd driver */
   rc = open_queue_and_register_driver( kbd_fd );
   if( rc )
      return -1;
   return 1;
}

/*
 * Close the keyboard.
 */
static void
MWKbd_Close(void)
{
}

/*
 * Return the possible modifiers for the keyboard.
 */
static  void
MWKbd_GetModifierInfo(int *modifiers)
{
    *modifiers = 0;      /* no modifiers available */
}

/*
 * This reads one keystroke from the keyboard, and the current state of
 * the mode keys (ALT, SHIFT, CTRL).  Returns -1 on error, 0 if no data
 * is ready, and 1 if data was read.  This is a non-blocking call.
 */
static int
MWKbd_Read(MWUCHAR *buf, int *modifiers)
{
   /* check if new KBD event has been posted */
   if( m_kbd.type != MV_UID_INVALID )
   {
      *buf = (UCHAR)m_kbd.m.kbd.code;
/*    *modifiers = m_kbd.m.kbd.modifiers;  */
      *modifiers = 0;

      /* consume event */
      m_kbd.type = MV_UID_INVALID;
      return 1;
   }
    return 0;
}
