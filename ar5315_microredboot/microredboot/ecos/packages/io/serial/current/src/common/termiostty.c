//==========================================================================
//
//      termiostty.c
//
//      POSIX Termios compatible TTY I/O driver
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Jonathan Larmour
// Copyright (C) 2003 Gary Thomas
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
// Author(s):    jlarmour
// Contributors: gthomas
// Date:         2000-07-22
// Purpose:      Device driver for termios emulation tty I/O, layered on
//               top of serial I/O
// Description:  TODO: Add OPOST support for 80x25 (configurable) windows
//               TODO: Support _POSIX_VDISABLE
//
//####DESCRIPTIONEND####
//
//==========================================================================

// CONFIGURATION

#include <pkgconf/io_serial.h>

#ifdef CYGPKG_IO_SERIAL_TERMIOS

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common types 
#include <cyg/infra/cyg_ass.h>     // Assertion support
#include <cyg/infra/cyg_trac.h>    // Tracing support
#include <cyg/io/io.h>
#include <cyg/io/devtab.h>
#include <cyg/io/serialio.h>       // public serial API
#include <termios.h>               // Termios header
#include <cyg/hal/drv_api.h>
#include <stdlib.h>                // malloc
#include <string.h>
#ifdef CYGSEM_IO_SERIAL_TERMIOS_USE_SIGNALS
# include <signal.h>
#endif

//==========================================================================
// FUNCTION PROTOTYPES

static bool
termios_init(struct cyg_devtab_entry *tab);

static Cyg_ErrNo
termios_lookup(struct cyg_devtab_entry **tab, 
               struct cyg_devtab_entry *sub_tab,
               const char *name);
static Cyg_ErrNo
termios_write(cyg_io_handle_t handle, const void *buf, cyg_uint32 *len);
static Cyg_ErrNo
termios_read(cyg_io_handle_t handle, void *buf, cyg_uint32 *len);
static cyg_bool
termios_select(cyg_io_handle_t handle, cyg_uint32 which, CYG_ADDRWORD info);
static Cyg_ErrNo 
termios_get_config(cyg_io_handle_t handle, cyg_uint32 key, void *buf,
                   cyg_uint32 *len);
static Cyg_ErrNo 
termios_set_config(cyg_io_handle_t handle, cyg_uint32 key, const void *buf,
                   cyg_uint32 *len);

//==========================================================================
// TYPE DEFINITIONS

struct termios_private_info {
    struct termios  termios;
    cyg_io_handle_t dev_handle;
    cyg_drv_mutex_t lock;
    cyg_bool        init;
    cyg_uint8      *errbuf;
    cyg_uint8      *errbufpos;
    cyg_uint32      errbufsize;
};

typedef struct {
    struct termios *termios_p;
    int optact;
} setattr_struct;


//==========================================================================
// STATIC OBJECTS

static DEVIO_TABLE(termios_devio,
                   termios_write,
                   termios_read,
                   termios_select,
                   termios_get_config,
                   termios_set_config);

#ifdef CYGPKG_IO_SERIAL_TERMIOS_TERMIOS0
static struct termios_private_info termios_private_info0;
DEVTAB_ENTRY(termios_io0, 
             "/dev/termios0",
             CYGDAT_IO_SERIAL_TERMIOS_TERMIOS0_DEV,
             &termios_devio,
             termios_init, 
             termios_lookup,
             &termios_private_info0);
#endif

#ifdef CYGPKG_IO_SERIAL_TERMIOS_TERMIOS1
static struct termios_private_info termios_private_info1;
DEVTAB_ENTRY(termios_io1, 
             "/dev/termios1", 
             CYGDAT_IO_SERIAL_TERMIOS_TERMIOS1_DEV,
             &termios_devio, 
             termios_init, 
             termios_lookup,
             &termios_private_info1);
#endif

#ifdef CYGPKG_IO_SERIAL_TERMIOS_TERMIOS2
static struct termios_private_info termios_private_info2;
DEVTAB_ENTRY(termios_io2, 
             "/dev/termios2", 
             CYGDAT_IO_SERIAL_TERMIOS_TERMIOS2_DEV,
             &termios_devio, 
             termios_init, 
             termios_lookup,
             &termios_private_info2);
#endif

static const cc_t c_cc_init[ NCCS ] = { 
    0x04,     /* EOF == ^D */
    0,        /* EOL */
    0x08,     /* ERASE = BS ; NB DEL=0x7f */
    0x03,     /* INTR = ^C */
    0x15,     /* KILL = ^U */
    0,        /* MIN = 0 */
    0x1c,     /* QUIT = ^\ */
    0x1a,     /* SUSP = ^Z ; NB ignored in this impl - no job control */
    0,        /* TIME = 0 */
#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_SOFTWARE
    CYGDAT_IO_SERIAL_FLOW_CONTROL_XON_CHAR,
    CYGDAT_IO_SERIAL_FLOW_CONTROL_XOFF_CHAR,
#else
    17,
    19,
#endif
};

// map eCos bitrates to POSIX bitrates.
static speed_t ecosbaud2posixbaud[] = {
    0, B50, B75, B110, B134, B150, B200, B300, B600, B1200, B2400, B3600,
    B4800, B7200, B9600, B14400, B19200, B38400, B57600, B115200, B230400 };

// map POSIX bitrates to eCos bitrates.
static cyg_serial_baud_rate_t posixbaud2ecosbaud[] = {
    0, CYGNUM_SERIAL_BAUD_50, CYGNUM_SERIAL_BAUD_75, CYGNUM_SERIAL_BAUD_110,
    CYGNUM_SERIAL_BAUD_134_5, CYGNUM_SERIAL_BAUD_150, CYGNUM_SERIAL_BAUD_200,
    CYGNUM_SERIAL_BAUD_300, CYGNUM_SERIAL_BAUD_600, CYGNUM_SERIAL_BAUD_1200,
    CYGNUM_SERIAL_BAUD_1800, CYGNUM_SERIAL_BAUD_2400, CYGNUM_SERIAL_BAUD_3600,
    CYGNUM_SERIAL_BAUD_4800, CYGNUM_SERIAL_BAUD_7200, CYGNUM_SERIAL_BAUD_9600,
    CYGNUM_SERIAL_BAUD_14400, CYGNUM_SERIAL_BAUD_19200,
    CYGNUM_SERIAL_BAUD_38400, CYGNUM_SERIAL_BAUD_57600,
    CYGNUM_SERIAL_BAUD_115200, CYGNUM_SERIAL_BAUD_230400 };


//==========================================================================
// FUNCTIONS

static __inline__ speed_t
map_ecosbaud_to_posixbaud( cyg_serial_baud_rate_t ebaud )
{
    if ( ebaud > (sizeof(ecosbaud2posixbaud) / sizeof(speed_t)) )
        return 0;
    return ecosbaud2posixbaud[ ebaud ];
}

static __inline__ cyg_serial_baud_rate_t
map_posixbaud_to_ecosbaud( speed_t pbaud )
{
    if ( pbaud > (sizeof(posixbaud2ecosbaud)/sizeof(cyg_serial_baud_rate_t)) )
        return 0;
    return posixbaud2ecosbaud[ pbaud ];
}

//==========================================================================
// real_termios_init is used to initialize the termios structure. This is
// called at lookup time, and not from termios_init() because it needs
// to query the serial device which may not be set up yet at that point
// in termios_init()

#ifdef CYGSEM_IO_SERIAL_TERMIOS_USE_SIGNALS
# define C_IFLAG_INIT (ICRNL|IGNBRK|BRKINT)
#else
# define C_IFLAG_INIT (ICRNL|IGNBRK)
#endif
#define C_OFLAG_INIT (ONLCR)
#define C_CFLAG_INIT (CREAD)
#define C_LFLAG_INIT (ECHO|ECHOE|ECHOK|ICANON)

static Cyg_ErrNo
real_termios_init( struct termios_private_info *priv )
{
    Cyg_ErrNo res;
    struct termios *t;
    cyg_serial_info_t dev_conf;
    cyg_serial_buf_info_t dev_buf_conf;
    cyg_uint32 len = sizeof( dev_conf );

    CYG_REPORT_FUNCTYPE("returning %d");
    CYG_REPORT_FUNCARG1XV( priv );
    CYG_CHECK_DATA_PTRC( priv );

    t = &priv->termios;

    // Get info from driver
    res = cyg_io_get_config( priv->dev_handle, CYG_IO_GET_CONFIG_SERIAL_INFO,
                             &dev_conf, &len );
    if ( ENOERR == res ) {
        len = sizeof( dev_buf_conf );
        res = cyg_io_get_config( priv->dev_handle,
                                 CYG_IO_GET_CONFIG_SERIAL_BUFFER_INFO,
                                 &dev_buf_conf, &len );
    }

    priv->errbuf = (cyg_uint8 *)malloc( dev_buf_conf.rx_bufsize );
    if ( NULL == priv->errbuf )
        res = ENOMEM;   // FIXME: Are we allowed to do this?
    priv->errbufpos = priv->errbuf;
    priv->errbufsize = dev_buf_conf.rx_bufsize;

    if ( ENOERR != res ) {
        CYG_REPORT_RETVAL( res );
        return res;
    }
    
    // we only support symmetric baud rates
    t->c_ispeed = t->c_ospeed = map_ecosbaud_to_posixbaud( dev_conf.baud );
    t->c_iflag = C_IFLAG_INIT;
    t->c_oflag = C_OFLAG_INIT;
    t->c_cflag = C_CFLAG_INIT;
    t->c_lflag = C_LFLAG_INIT;
    memcpy( t->c_cc, c_cc_init, sizeof( t->c_cc ) );
    
    switch ( dev_conf.parity ) {
    case CYGNUM_SERIAL_PARITY_NONE:
        t->c_iflag |= IGNPAR;
        break;
    case CYGNUM_SERIAL_PARITY_ODD:
        t->c_cflag |= PARODD;
        // DROPTHROUGH
    case CYGNUM_SERIAL_PARITY_EVEN:
        t->c_iflag |= PARENB;
        break;
    default:
        CYG_FAIL( "Unsupported default parity" );
        break;
    }

    switch( dev_conf.word_length ) {
    case CYGNUM_SERIAL_WORD_LENGTH_5:
        t->c_cflag |= CS5;
        break;        
    case CYGNUM_SERIAL_WORD_LENGTH_6:
        t->c_cflag |= CS6;
        break;
    case CYGNUM_SERIAL_WORD_LENGTH_7:
        t->c_cflag |= CS7;
        break;
    case CYGNUM_SERIAL_WORD_LENGTH_8:
        t->c_cflag |= CS8;
        break;
    default:
        CYG_FAIL( "Unsupported word length" );
        break;
    }

    switch ( dev_conf.stop ) {
    case CYGNUM_SERIAL_STOP_1:
        // Don't need to do anything
        break;
    case CYGNUM_SERIAL_STOP_2:
        t->c_cflag |= CSTOPB;
        break;
    default:
        CYG_FAIL( "Unsupported number of stop bits" );
        break;
    }

    switch ( dev_conf.flags ) {
    case CYGNUM_SERIAL_FLOW_RTSCTS_RX:
        t->c_cflag |= CRTSCTS;
        // drop through
    case CYGNUM_SERIAL_FLOW_XONXOFF_RX:
        t->c_iflag |= IXOFF;
        break;
    case CYGNUM_SERIAL_FLOW_RTSCTS_TX:
        t->c_cflag |= CRTSCTS;
        // drop through
    case CYGNUM_SERIAL_FLOW_XONXOFF_TX:
        t->c_iflag |= IXON;
        break;
    default:
        // Ignore flags we don't grok
        break;
    }

    return ENOERR;
} // real_termios_init()

//==========================================================================
// set_attr() actually enacts the termios config. We come in here with
// the mutex in priv locked
//
// Possible deviation from standard: POSIX says we should enact which ever
// bits we can and only return EINVAL when none of them can be performed
// Rather than tracking whether *none* of them worked, instead we just
// always claim success. At the very least, setting c_cc is never to
// fail so I'm not sure if this is really non-standard or not!

static Cyg_ErrNo
set_attr( struct termios *t, struct termios_private_info *priv )
{
    Cyg_ErrNo res = ENOERR;
    cyg_serial_info_t dev_conf, new_dev_conf;
    cyg_uint32 len = sizeof( dev_conf );
    cc_t *tempcc = &priv->termios.c_cc[0];
    struct termios *ptermios = &priv->termios;
    
    // Get info from driver
    res = cyg_io_get_config( priv->dev_handle, CYG_IO_GET_CONFIG_SERIAL_INFO,
                             &dev_conf, &len );

    if ( ENOERR != res )
        return res;
        
    // We need to set up each facet of config to change one by one because
    // POSIX says we should try and change as much of the config as possible
    // This is tedious and has to be done by steam :-(

    if ( t->c_ospeed != ptermios->c_ospeed ) {
        new_dev_conf = dev_conf;
        new_dev_conf.baud = map_posixbaud_to_ecosbaud( t->c_ospeed );
        if ( 0 != new_dev_conf.baud ) {
            len = sizeof( new_dev_conf );
            res = cyg_io_set_config( priv->dev_handle,
                                     CYG_IO_SET_CONFIG_SERIAL_INFO,
                                     &new_dev_conf, &len );
            if ( ENOERR == res ) {
                // It worked, so update dev_conf to reflect the new state
                dev_conf.baud = new_dev_conf.baud;
                // and termios
                ptermios->c_ispeed = t->c_ospeed;
                ptermios->c_ospeed = t->c_ospeed;
            }
        }
    }

    if ( (t->c_cflag & CSTOPB) != (ptermios->c_cflag & CSTOPB) ) {
        new_dev_conf = dev_conf;
        if ( t->c_cflag & CSTOPB )
            new_dev_conf.stop = CYGNUM_SERIAL_STOP_2;
        else
            new_dev_conf.stop = CYGNUM_SERIAL_STOP_1;
        
        len = sizeof( new_dev_conf );
        res = cyg_io_set_config( priv->dev_handle,
                                 CYG_IO_SET_CONFIG_SERIAL_INFO,
                                 &new_dev_conf, &len );
        if ( ENOERR == res ) {
            // It worked, so update dev_conf to reflect the new state
            dev_conf.stop = new_dev_conf.stop;
            // and termios
            ptermios->c_cflag &= ~CSTOPB;
            ptermios->c_cflag |= t->c_cflag & CSTOPB;
        }
    }

    if ( ((t->c_cflag & PARENB) != (ptermios->c_cflag & PARENB)) ||
         ((t->c_cflag & PARODD) != (ptermios->c_cflag & PARODD)) ) {
        new_dev_conf = dev_conf;
        if ( t->c_cflag & PARENB )
            if ( t->c_cflag & PARODD )
                new_dev_conf.parity = CYGNUM_SERIAL_PARITY_ODD;
            else
                new_dev_conf.parity = CYGNUM_SERIAL_PARITY_EVEN;
        else
            new_dev_conf.parity = CYGNUM_SERIAL_PARITY_NONE;
        
        len = sizeof( new_dev_conf );
        res = cyg_io_set_config( priv->dev_handle,
                                 CYG_IO_SET_CONFIG_SERIAL_INFO,
                                 &new_dev_conf, &len );
        if ( ENOERR == res ) {
            // It worked, so update dev_conf to reflect the new state
            dev_conf.parity = new_dev_conf.parity;
            // and termios
            ptermios->c_cflag &= ~(PARENB|PARODD);
            ptermios->c_cflag |= t->c_cflag & (PARENB|PARODD);
        }
    }

    if ( (t->c_cflag & CSIZE) != (ptermios->c_cflag & CSIZE) ) {
        new_dev_conf = dev_conf;
        switch ( t->c_cflag & CSIZE ) {
        case CS5:
            new_dev_conf.word_length = CYGNUM_SERIAL_WORD_LENGTH_5;
            break;
        case CS6:
            new_dev_conf.word_length = CYGNUM_SERIAL_WORD_LENGTH_6;
            break;
        case CS7:
            new_dev_conf.word_length = CYGNUM_SERIAL_WORD_LENGTH_7;
            break;
        case CS8:
            new_dev_conf.word_length = CYGNUM_SERIAL_WORD_LENGTH_8;
            break;
        }
        
        len = sizeof( new_dev_conf );
        res = cyg_io_set_config( priv->dev_handle,
                                 CYG_IO_SET_CONFIG_SERIAL_INFO,
                                 &new_dev_conf, &len );
        if ( ENOERR == res ) {
            // It worked, so update dev_conf to reflect the new state
            dev_conf.word_length = new_dev_conf.word_length;
            // and termios
            ptermios->c_cflag &= ~CSIZE;
            ptermios->c_cflag |= t->c_cflag & CSIZE;
        }
    }

    if ( (t->c_cflag & IXOFF) != (ptermios->c_cflag & IXOFF) ) {
        new_dev_conf = dev_conf;
        new_dev_conf.flags &=
            ~(CYGNUM_SERIAL_FLOW_XONXOFF_RX|CYGNUM_SERIAL_FLOW_RTSCTS_RX);
        if ( t->c_cflag & IXOFF )
            if ( t->c_cflag & CRTSCTS)
                new_dev_conf.flags |= CYGNUM_SERIAL_FLOW_RTSCTS_RX;
            else
                new_dev_conf.flags |= CYGNUM_SERIAL_FLOW_XONXOFF_RX;
        else
            new_dev_conf.flags |= CYGNUM_SERIAL_FLOW_NONE;
        
        len = sizeof( new_dev_conf );
        res = cyg_io_set_config( priv->dev_handle,
                                 CYG_IO_SET_CONFIG_SERIAL_INFO,
                                 &new_dev_conf, &len );
        if ( ENOERR == res ) {
            // It worked, so update dev_conf to reflect the new state
            dev_conf.flags = new_dev_conf.flags;
            // and termios
            ptermios->c_cflag &= ~(IXOFF|CRTSCTS);
            ptermios->c_cflag |= t->c_cflag & (IXOFF|CRTSCTS);
        }
    }

    if ( (t->c_cflag & IXON) != (ptermios->c_cflag & IXON) ) {
        new_dev_conf = dev_conf;
        new_dev_conf.flags &=
            ~(CYGNUM_SERIAL_FLOW_XONXOFF_TX|CYGNUM_SERIAL_FLOW_RTSCTS_TX);
        if ( t->c_cflag & IXON )
            if ( t->c_cflag & CRTSCTS)
                new_dev_conf.flags |= CYGNUM_SERIAL_FLOW_RTSCTS_TX;
            else
                new_dev_conf.flags |= CYGNUM_SERIAL_FLOW_XONXOFF_TX;
        else
            new_dev_conf.flags |= CYGNUM_SERIAL_FLOW_NONE;
        
        len = sizeof( new_dev_conf );
        res = cyg_io_set_config( priv->dev_handle,
                                 CYG_IO_SET_CONFIG_SERIAL_INFO,
                                 &new_dev_conf, &len );
        if ( ENOERR == res ) {
            // It worked, so update dev_conf to reflect the new state
            dev_conf.flags = new_dev_conf.flags;
            // and termios
            ptermios->c_cflag &= ~(IXON|CRTSCTS);
            ptermios->c_cflag |= t->c_cflag & (IXON|CRTSCTS);
        }
    }

    // Input/Output processing flags can just be set as we grok them all
    // with few exceptions (see lflags below)
    ptermios->c_iflag &= ~(BRKINT|ICRNL|IGNBRK|IGNCR|IGNPAR|INLCR|INPCK|
                           ISTRIP|PARMRK);
    ptermios->c_iflag |= t->c_iflag & (
#ifdef CYGSEM_IO_SERIAL_TERMIOS_USE_SIGNALS
                                       BRKINT|
#endif
                                       ICRNL|IGNBRK|IGNCR|IGNPAR|
                                       INLCR|INPCK|ISTRIP|PARMRK );
    
    ptermios->c_oflag &= ~(OPOST|ONLCR);
    ptermios->c_oflag |= t->c_oflag & (OPOST|ONLCR);

    ptermios->c_cflag &= ~(CLOCAL|CREAD|HUPCL);
    ptermios->c_cflag |= t->c_cflag & (CLOCAL|CREAD|HUPCL);

    ptermios->c_lflag &= ~(ECHO|ECHOE|ECHOK|ECHONL|ICANON|
                                 IEXTEN|ISIG|NOFLSH|TOSTOP);
    // Note we don't support IEXTEN nor TOSTOP so we don't set them
    ptermios->c_lflag |= t->c_lflag & (ECHO|ECHOE|ECHOK|ECHONL|ICANON|
#ifdef CYGSEM_IO_SERIAL_TERMIOS_USE_SIGNALS
                                       ISIG|
#endif
                                       NOFLSH);

    // control characters. We don't support changing of VSTART, VSTOP,
    // VTIME or VSUSP though
    tempcc[VEOF]   = t->c_cc[VEOF];
    tempcc[VEOL]   = t->c_cc[VEOL];
    tempcc[VERASE] = t->c_cc[VERASE];
    tempcc[VINTR]  = t->c_cc[VINTR];
    tempcc[VKILL]  = t->c_cc[VKILL];
    tempcc[VMIN]   = t->c_cc[VMIN];
    tempcc[VQUIT]  = t->c_cc[VQUIT];
        
    return res;
}

//==========================================================================

static bool 
termios_init(struct cyg_devtab_entry *tab)
{
    // can't initialize the termios structure because we can't
    // query the serial driver yet. Wait until lookup time.

    return true;
} // termios_init()

//==========================================================================

static Cyg_ErrNo 
termios_lookup(struct cyg_devtab_entry **tab, 
           struct cyg_devtab_entry *sub_tab,
           const char *name)
{
    cyg_io_handle_t chan = (cyg_io_handle_t)sub_tab;
    struct termios_private_info *priv =
        (struct termios_private_info *)(*tab)->priv;
    Cyg_ErrNo err = ENOERR;
    
    if ( !priv->init ) {
        cyg_drv_mutex_lock( &priv->lock );
        if ( !priv->init ) {  // retest as we may have been pre-empted
            priv->dev_handle = chan;
            err = real_termios_init( priv );
        }
        cyg_drv_mutex_unlock( &priv->lock );
    }
    return err;
}

//==========================================================================

#define WRITE_BUFSIZE 100 // FIXME: ->CDL
// #define MAX_CANON 64  FIXME: relevance?


static Cyg_ErrNo 
termios_write(cyg_io_handle_t handle, const void *_buf, cyg_uint32 *len)
{
    cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    struct termios_private_info *priv = (struct termios_private_info *)t->priv;
    cyg_io_handle_t chan = (cyg_io_handle_t)priv->dev_handle;
    cyg_int32 xbufsize, input_bytes_read;
    cyg_uint8 xbuf[WRITE_BUFSIZE];
    cyg_uint8 *buf = (cyg_uint8 *)_buf;
    Cyg_ErrNo res;

    xbufsize = input_bytes_read = 0;
    while (input_bytes_read++ < *len) {
        if ( (*buf == '\n') && (priv->termios.c_oflag & (OPOST|ONLCR)) ) {
            xbuf[xbufsize++] = '\r';
        }
        xbuf[xbufsize++] = *buf;
        if ((xbufsize >= (WRITE_BUFSIZE-1)) || (input_bytes_read == *len) ||
            (*buf == '\n'))
        {
            cyg_int32 size = xbufsize;
            res = cyg_io_write(chan, xbuf, &size);
            if (res != ENOERR) {
                *len = input_bytes_read - (xbufsize - size);
                return res;
            }
            xbufsize = 0;
        }
        buf++;
    }
    // Everything sent, so *len is correct.
    return ENOERR;
}


//==========================================================================

static Cyg_ErrNo 
termios_read(cyg_io_handle_t handle, void *_buf, cyg_uint32 *len)
{
    cyg_devtab_entry_t *dt = (cyg_devtab_entry_t *)handle;
    struct termios_private_info *priv = (struct termios_private_info *)dt->priv;
    cyg_io_handle_t chan = (cyg_io_handle_t)priv->dev_handle;
    struct termios *t = &priv->termios;
    cyg_uint32 clen;
    cyg_uint32 size;
    Cyg_ErrNo res;
    cyg_uint8 c;
    cyg_uint8 *buf = (cyg_uint8 *)_buf;
    cyg_bool discardc; // should c be discarded (not read, not printed)
    cyg_bool returnnow = false; // return back to user after this char
    
    // if receiver off
    if (0 == (t->c_cflag & CREAD) ) {
        *len = 0;
        return -EINVAL;
    }

    size = 0;
    if ( 0 == (t->c_lflag & ICANON) ) {
        // In non-canonical mode we return the min of *len and the
        // number of bytes available
        // So we query the driver for how many bytes are available - this
        // guarantees we won't block
        cyg_serial_buf_info_t dev_buf_conf;
        cyg_uint32 dbc_len = sizeof( dev_buf_conf );
        res = cyg_io_get_config( chan,
                                 CYG_IO_GET_CONFIG_SERIAL_BUFFER_INFO,
                                 &dev_buf_conf, &dbc_len );
        CYG_ASSERT( res == ENOERR, "Query buffer status failed!" );
        if (dev_buf_conf.rx_count > 0) {
            // Adjust length to be max characters currently available
            *len = *len < dev_buf_conf.rx_count ? *len : dev_buf_conf.rx_count;
        } else if (t->c_cc[VMIN] == 0) {
            // No chars available - don't block
            *len = 0;
            return ENOERR;
        }
    } // if

    while (!returnnow && size < *len) {
        clen = 1;
        discardc = false;
        res = cyg_io_read(chan, &c, &clen);
        if (res != ENOERR) {
            *len = size;
            return res;
        }

        // lock to prevent termios getting corrupted while we read from it
        cyg_drv_mutex_lock( &priv->lock );

        if ( t->c_iflag & ISTRIP )
            c &= 0x7f;

        // canonical mode: erase, kill, and newline processing
        if ( t->c_lflag & ICANON ) {
            if ( t->c_cc[ VERASE ] == c ) {
                discardc = true;
                // erase on display?
                if ( (t->c_lflag & ECHO) && (t->c_lflag & ECHOE) ) {
                    cyg_uint8 erasebuf[3];
                    erasebuf[0] = erasebuf[2] = t->c_cc[ VERASE ];
                    erasebuf[1] = ' ';
                    clen = sizeof(erasebuf);
                    // FIXME: what about error or non-blocking?
                    cyg_io_write(chan, erasebuf, &clen);
                }
                if ( size )
                    size--;
            } // if
            else if ( t->c_cc[ VKILL ] == c ) {
                // kill line on display?
                if ( (t->c_lflag & ECHO) && (t->c_lflag & ECHOK) ) {

                    // we could try and be more efficient here and 
                    // output a stream of erases, and then a stream
                    // of spaces and then more erases. But this is poor
                    // because on a slow terminal the user will see characters
                    // delete from the middle forward in chunks!
                    // But at least we try and chunk up sets of writes
                    cyg_uint8 erasebuf[30];
                    cyg_uint8 erasechunks;
                    cyg_uint8 i;

                    erasechunks = size < (sizeof(erasebuf)/3) ? 
                        size : (sizeof(erasebuf)/3);

                    for (i=0; i<erasechunks; i++) {
                        erasebuf[i*3] = erasebuf[i*3+2] = t->c_cc[ VERASE ];
                        erasebuf[i*3+1] = ' ';
                    }

                    while( size ) {
                        cyg_uint8 j;

                        j = size < (sizeof(erasebuf)/3) ? 
                            size : (sizeof(erasebuf)/3);
                        clen = (j*3);
                        // FIXME: what about error or non-blocking?
                        cyg_io_write( chan, erasebuf, &clen );
                        size -= j;
                    }
                } else
                    size = 0;
                discardc = true;
            } // else if
            // CR
            else if ( '\r' == c ) {
                if ( t->c_iflag & IGNCR )
                    discardc = true;
                else if ( t->c_iflag & ICRNL )
                    c = '\n';
            }
            // newlines or similar.
            // Note: not an else if to catch CRNL conversion
            if ( (t->c_cc[ VEOF ] == c) || (t->c_cc[ VEOL ] == c) ||
                 ('\n' == c) ) {
                if ( t->c_cc[ VEOF ] == c )
                     discardc = true;
                if ( t->c_lflag & ECHONL ) { // don't check ECHO in this case
                    clen = 1;
                    // FIXME: what about error or non-blocking?
                    // FIXME: what if INLCR is set?
                    cyg_io_write( chan, "\n", &clen );
                }
                if ( t->c_iflag & INLCR )
                    c = '\r';
                returnnow = true; // FIXME: true even for INLCR?
            } // else if
        } // if 

#ifdef CYGSEM_IO_SERIAL_TERMIOS_USE_SIGNALS
        if ( (t->c_lflag & ISIG) && (t->c_cc[ VINTR ] == c) ) {
            discardc = true;
            if ( 0 == (t->c_lflag & NOFLSH) )
                size = 0;
            // raise could be a non-local jump - we should unlock mutex
            cyg_drv_mutex_unlock( &priv->lock ); 

            // FIXME: what if raise returns != 0?
            raise( SIGINT );
            cyg_drv_mutex_lock( &priv->lock ); 
        } 

        if ( (t->c_lflag & ISIG) && (t->c_cc[ VQUIT ] == c) ) {
            discardc = true;
            if ( 0 == (t->c_lflag & NOFLSH) )
                size = 0;
            // raise could be a non-local jump - we should unlock mutex
            cyg_drv_mutex_unlock( &priv->lock ); 

            // FIXME: what if raise returns != 0?
            raise( SIGQUIT );
            cyg_drv_mutex_lock( &priv->lock ); 
        } 
#endif
        if (!discardc) {
            buf[size++] = c;
            if ( t->c_lflag & ECHO ) {
                clen = 1;
                // FIXME: what about error or non-blocking?
                termios_write( handle, &c, &clen );
            }
        }

        if ( (t->c_lflag & ICANON) == 0 ) {
            // Check to see if read has been satisfied
            if ( t->c_cc[ VMIN ] && (size >= t->c_cc[ VMIN ]) )
                returnnow = true;
        }
        cyg_drv_mutex_unlock( &priv->lock );
    } // while

    *len = size;
    return ENOERR;
}


//==========================================================================

static cyg_bool
termios_select(cyg_io_handle_t handle, cyg_uint32 which, CYG_ADDRWORD info)
{
    cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    struct termios_private_info *priv = (struct termios_private_info *)t->priv;
    cyg_io_handle_t chan = (cyg_io_handle_t)priv->dev_handle;    

    // Just pass it on to next driver level
    return cyg_io_select( chan, which, info );
}


//==========================================================================

static Cyg_ErrNo 
termios_get_config(cyg_io_handle_t handle, cyg_uint32 key, void *buf,
                   cyg_uint32 *len)
{
    cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    struct termios_private_info *priv = (struct termios_private_info *)t->priv;
    cyg_io_handle_t chan = (cyg_io_handle_t)priv->dev_handle;
    Cyg_ErrNo res = ENOERR;

    switch (key) {
    case CYG_IO_GET_CONFIG_TERMIOS:
        {
            if ( *len < sizeof(struct termios) ) {
                return -EINVAL;
            }
            cyg_drv_mutex_lock( &priv->lock );
            *(struct termios *)buf = priv->termios;
            cyg_drv_mutex_unlock( &priv->lock );
            *len = sizeof(struct termios);
        }
        break;
    default:  // Assume this is a 'serial' driver control
        res = cyg_io_get_config(chan, key, buf, len);
    } // switch
    return res;
}


//==========================================================================


static Cyg_ErrNo 
termios_set_config(cyg_io_handle_t handle, cyg_uint32 key, const void *buf, cyg_uint32 *len)
{
    cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    struct termios_private_info *priv = (struct termios_private_info *)t->priv;
    cyg_io_handle_t chan = (cyg_io_handle_t)priv->dev_handle;
    Cyg_ErrNo res = ENOERR;

    switch (key) {
    case CYG_IO_SET_CONFIG_TERMIOS:
        {
            setattr_struct *attr = (setattr_struct *)buf;
            int optact = attr->optact;

            if ( *len != sizeof( *attr ) ) {
                return -EINVAL;
            }

            CYG_ASSERT( (optact == TCSAFLUSH) || (optact == TCSADRAIN) ||
                        (optact == TCSANOW), "Invalid optact" );
                
            cyg_drv_mutex_lock( &priv->lock );
    
            if ( ( TCSAFLUSH == optact ) ||
                 ( TCSADRAIN == optact ) ) {
                res = cyg_io_get_config( chan,
                                         CYG_IO_GET_CONFIG_SERIAL_OUTPUT_DRAIN,
                                         NULL, NULL );
                CYG_ASSERT( ENOERR == res, "Drain request failed" );
            }
            if ( TCSAFLUSH == optact ) {
                res = cyg_io_get_config( chan,
                                         CYG_IO_GET_CONFIG_SERIAL_INPUT_FLUSH,
                                         NULL, NULL );
                CYG_ASSERT( ENOERR == res, "Flush request failed" );
            }
                
            res = set_attr( attr->termios_p, priv );
            cyg_drv_mutex_unlock( &priv->lock );
            return res;
        }
    default: // Pass on to serial driver
        res = cyg_io_set_config(chan, key, buf, len);
    }
    return res;
}


//==========================================================================

#endif // ifdef CYGPKG_IO_SERIAL_TERMIOS

// EOF termiostty.c
