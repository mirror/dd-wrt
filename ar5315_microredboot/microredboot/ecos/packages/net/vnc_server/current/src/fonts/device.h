/*
 * Copyright (c) 1999, 2000 Greg Haerr <greg@censoft.com>
 *
 * Engine-level Screen, Mouse and Keyboard device driver API's and types
 *
 * Contents of this file are not for general export
 */

/* This is a fragment of the device.h file as used by the eCos port */
/* of MicroWindows.  This file is required to allow the VNC server  */
/* to use the font definition files supplied with microwindows.     */
/* This file is only included by the font definition files, other   */
/* files, including application files should include the            */
/* vnc-server.h file which also contains these 2 typedefs           */

/* Chris Garry <cgarry@sweeneydesign.co.uk> 26-Aug-2003             */


typedef unsigned short  MWIMAGEBITS;    /* bitmap image unit size*/

typedef struct {
    char *      name;       /* font name*/
    int     maxwidth;   /* max width in pixels*/
    int     height;     /* height in pixels*/
    int     ascent;     /* ascent (baseline) height*/
    int     firstchar;  /* first character in bitmap*/
    int     size;       /* font size in characters*/
    MWIMAGEBITS *   bits;       /* 16-bit right-padded bitmap data*/
    unsigned short *offset;     /* 256 offsets into bitmap data*/
    unsigned char * width;      /* 256 character widths or 0 if fixed*/
} MWCFONT;

