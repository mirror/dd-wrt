/*
 * Definitions for the IPWireless 3G UMTS TDD, USB connected modem
 */

/* vendor/product pairs that are known work with this driver*/
/* to add a supported device, define the id's here and modify the */
/* id_table in ipw.c */

#define IPW_VID		0x0bc3
#define IPW_PID1	0x0001 // modem uses this product id in "normal" mode
#define IPW_PID2	0x0002 // modem uses this product id in "composite" mode


/* Vendor commands: */



/* baud rates */

enum {
  ipw_sio_b256000 = 0x000e,
  ipw_sio_b128000 = 0x001d,
  ipw_sio_b115200 = 0x0020,
  ipw_sio_b57600  = 0x0040,
  ipw_sio_b56000  = 0x0042,
  ipw_sio_b38400  = 0x0060,
  ipw_sio_b19200  = 0x00c0,
  ipw_sio_b14400  = 0x0100,
  ipw_sio_b9600   = 0x0180,
  ipw_sio_b4800   = 0x0300,   
  ipw_sio_b2400   = 0x0600,   
  ipw_sio_b1200   = 0x0c00, 
  ipw_sio_b600    = 0x1800
};

/* data bits */
#define ipw_dtb_7   0x700
#define ipw_dtb_8   0x810	// ok so the define is misleading, I know, but forces 8,n,1
				// I mean, is there a point to any other setting these days? :)	



/* usb control request types : */
#define IPW_SIO_RXCTL	  0x00 // control bulk rx channel transmissions, value=1/0 (on/off)
#define IPW_SIO_SET_BAUD  0x01 // set baud, value=requested ipw_sio_bxxxx
#define IPW_SIO_SET_LINE  0x03 // set databits, parity. value=ipw_dtb_x
#define IPW_SIO_SET_PIN   0x07 // set/clear dtr/rts value=ipw_pin_xxx
#define IPW_SIO_POLL	  0x08 // get serial port status byte, call with value=0

#define IPW_SIO_FETCH_UNK 0x10 // fetches some unknown data, typicall 32 bytes of gumph

#define IPW_SIO_INIT	  0x11 // initializes ? value=0 (appears as first thing todo on open)
#define IPW_SIO_PURGE	  0x12 // purge all transmissions?, call with value=numchar_to_purge
#define IPW_SIO_HANDFLOW  0x13 // set xon/xoff limits value=0, and a buffer of 0x10 bytes
#define IPW_SIO_SETCHARS  0x19 // set the flowcontrol special chars, value=0, buf=6 bytes, 
			       // last 2 bytes contain flowcontrol chars e.g. 00 00 00 00 11 13


/* values used for request IPW_SIO_SET_PIN*/
#define IPW_PIN_SETDTR	0x101
#define IPW_PIN_SETRTS	0x202
#define IPW_PIN_CLRDTR	0x100
#define IPW_PIN_CLRRTS	0x200 // unconfirmed

/* values used for request IPW_SIO_RXCTL*/
#define IPW_RXBULK_ON		1
#define IPW_RXBULK_OFF		0


/* various 16 byte hardcoded transferbuffers used by flow control */
#define IPW_BYTES_FLOWINIT	{ 0x01, 0, 0, 0, 0x40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
#define IPW_BYTES_SETCHARSINIT_XONXOFF	{ 0, 0, 0, 0, 0x11, 0x13 } //set flow control chars to STX/ETX
#define IPW_BYTES_SETCHARSINIT_NONE	{ 0, 0, 0, 0, 0, 0 } //no flow control characters please

/* Interpretation of modem status lines */
/* These need sorting out by individually connecting pins and checking
 * results. FIXME!
 * When data is being sent we see 0x30 in the lower byte; this must
 * contain DSR and CTS ...
 */
#define IPW_DSR			((1<<4) | (1<<5))
#define IPW_CTS			((1<<5) | (1<<4))

#define IPW_WANTS_TO_SEND	0x30
//#define IPW_DTR			/* Data Terminal Ready */
//#define IPW_CTS			/* Clear To Send */
//#define IPW_CD			/* Carrier Detect */
//#define IPW_DSR			/* Data Set Ready */
//#define IPW_RxD			/* Receive pin */

//#define IPW_LE
//#define IPW_RTS		
//#define IPW_ST		
//#define IPW_SR		
//#define IPW_RI			/* Ring Indicator */

/* vim: set ts=8 sts=8: */

