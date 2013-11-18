/* $Id: serial.c 4080 2006-12-05 13:35:45Z esr $ */
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include "gpsd_config.h"

#if defined(HAVE_SYS_MODEM_H)
#include <sys/modem.h>
#endif /* HAVE_SYS_MODEM_H */
#include "gpsd.h"
/* Workaround for HP-UX 11.23, which is missing CRTSCTS */
#ifndef CRTSCTS
#  ifdef CNEW_RTSCTS
#    define CRTSCTS CNEW_RTSCTS
#  else
#    define CRTSCTS 0
#  endif /* CNEW_RTSCTS */
#endif /* !CRTSCTS */

void gpsd_tty_init(struct gps_device_t *session)
/* to be called on allocating a device */
{
    /* mark GPS fd closed and its baud rate unknown */
    session->gpsdata.gps_fd = -1;
    session->saved_baud = -1;
}

#if defined(__CYGWIN__)
/* Workaround for Cygwin, which is missing cfmakeraw */
/* Pasted from man page; added in serial.c arbitrarily */
void cfmakeraw(struct termios *termios_p)
{
    termios_p->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
    termios_p->c_oflag &= ~OPOST;
    termios_p->c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
    termios_p->c_cflag &= ~(CSIZE|PARENB);
    termios_p->c_cflag |= CS8;
}
#endif /* defined(__CYGWIN__) */

speed_t gpsd_get_speed(struct termios* ttyctl)
{
    speed_t code = cfgetospeed(ttyctl);
    switch (code) {
    case B0:     return(0);
    case B300:   return(300);
    case B1200:  return(1200);
    case B2400:  return(2400);
    case B4800:  return(4800);
    case B9600:  return(9600);
    case B19200: return(19200);
    case B38400: return(38400);
    case B57600: return(57600);
    default: return(115200);
    }
}

bool gpsd_set_raw(struct gps_device_t *session)
{
    (void)cfmakeraw(&session->ttyset);
    if (tcsetattr(session->gpsdata.gps_fd, TCIOFLUSH, &session->ttyset) < 0) {
 	gpsd_report(LOG_ERROR,
		    "error changing port attributes: %s\n",strerror(errno));
 	return false;
    }

    return true;
}

void gpsd_set_speed(struct gps_device_t *session, 
		   speed_t speed, unsigned char parity, unsigned int stopbits)
{
    speed_t	rate;

    if (speed < 300)
	rate = B0;
    else if (speed < 1200)
      rate =  B300;
    else if (speed < 2400)
      rate =  B1200;
    else if (speed < 4800)
      rate =  B2400;
    else if (speed < 9600)
      rate =  B4800;
    else if (speed < 19200)
      rate =  B9600;
    else if (speed < 38400)
      rate =  B19200;
    else if (speed < 57600)
      rate =  B38400;
    else if (speed < 115200)
      rate =  B57600;
    else
      rate =  B115200;

    if (rate!=cfgetispeed(&session->ttyset) || (unsigned int)parity!=session->gpsdata.parity || stopbits!=session->gpsdata.stopbits) {

	/*@ignore@*/
	(void)cfsetispeed(&session->ttyset, rate);
	(void)cfsetospeed(&session->ttyset, rate);
	/*@end@*/
 	session->ttyset.c_iflag &=~ (PARMRK | INPCK);
 	session->ttyset.c_cflag &=~ (CSIZE | CSTOPB | PARENB | PARODD);
 	session->ttyset.c_cflag |= (stopbits==2 ? CS7|CSTOPB : CS8);
 	switch (parity)
 	{
 	case 'E':
 	    session->ttyset.c_iflag |= INPCK;
 	    session->ttyset.c_cflag |= PARENB;
 	    break;
 	case 'O':
 	    session->ttyset.c_iflag |= INPCK;
 	    session->ttyset.c_cflag |= PARENB | PARODD;
 	    break;
 	}
	if (tcsetattr(session->gpsdata.gps_fd, TCSANOW, &session->ttyset) != 0)
	    return;

	/*
	 * Serious black magic begins here.  Getting this code wrong can cause
	 * failures to lock to a correct speed, and not clean reproducible 
	 * failures but flukey hardware- and timing-dependent ones.  So
	 * be very sure you know what you're doing before hacking it, and
	 * test thoroughly.
	 *
	 * The fundamental problem here is that serial devices take time 
	 * to settle into a new baud rate after tcsetattr() is issued. Until
	 * they do so, input will be arbitarily garbled.  Normally this
	 * is not a big problem, but in our hunt loop the garbling can trash 
	 * a long enough prefix of each sample to prevent detection of a 
	 * packet header.  We could address the symptom by making the sample
	 * size enough larger that subtracting the maximum length of garble
	 * would still leave a sample longer than the maximum packet size.  
	 * But it's better (and more efficient) to address the disease.
	 *
	 * In theory, one might think that not even a tcflush() call would 
	 * be needed, with tcsetattr() delaying its return until the device
	 * is in a good state.  For simple devices like a 14550 UART that 
	 * have fixed response timings this may even work, if the driver
	 * writer was smart enough to delay the return by the right number
	 * of milliseconds after poking the device port(s).
	 *
	 * Problems may arise if the driver's timings are off.  Or we may
         * be talking to a USB device like the pl2303 commonly used in GPS
	 * mice; on these, the change will not happen immediately because
	 * it has to be sent as a message to the external processor that 
	 * has to act upon it, and that processor may still have buffered 
	 * data in its own FIFO.  In this case the expected delay may be
	 * too large and too variable (depending on the details of how the
	 * USB device is integrated with its symbiont hardware) to be put
	 * in the driver.
	 *
	 * So, somehow, we have to introduce a delay after tcsatattr() 
	 * returns sufficient to allow *any* device to settle.  On the other
	 * hand, a really long delay will make gpsd device registration
	 * unpleasantly laggy.
	 *
	 * The classic way to address this is with a tcflush(), counting
	 * on it to clear the device FIFO. But that call may clear only the
	 * kernel buffers, not the device's hardware FIFO, so it may not
	 * be sufficient by itself.
	 *
	 * flush followed by a 200-millisecond delay followed by flush has
	 * been found to work reliably on the pl2303.  It is also known
	 * from testing that a 100-millisec delay is too short, allowing
	 * occasional failure to lock.
	 */
	(void)tcflush(session->gpsdata.gps_fd, TCIOFLUSH);
	(void)usleep(200000);
	(void)tcflush(session->gpsdata.gps_fd, TCIOFLUSH);
    }
    gpsd_report(LOG_INF, "speed %d, %d%c%d\n", speed, 9-stopbits, parity, stopbits);

    session->gpsdata.baudrate = (unsigned int)speed;
    session->gpsdata.parity = (unsigned int)parity;
    session->gpsdata.stopbits = stopbits;

    /*
     * The device might need a wakeup string before it will send data.
     * If we don't know the device type, ship it every driver's wakeup
     * in hopes it will respond.
     */
    if (isatty(session->gpsdata.gps_fd)!=0) {
	struct gps_type_t **dp;
	if (session->device_type == NULL) {
	    for (dp = gpsd_drivers; *dp; dp++)
		if ((*dp)->probe_wakeup != NULL)
		    (*dp)->probe_wakeup(session);
	} else if (session->device_type->probe_wakeup != NULL)
	    session->device_type->probe_wakeup(session);
    }
    packet_reset(&session->packet);
}

int gpsd_open(struct gps_device_t *session)
{
    gpsd_report(LOG_INF, "opening GPS data source at '%s'\n", session->gpsdata.gps_device);
    if ((session->gpsdata.gps_fd = open(session->gpsdata.gps_device, O_RDWR|O_NONBLOCK|O_NOCTTY)) < 0) {
	gpsd_report(LOG_ERROR, "read-write device open failed: %s - trying read-only\n", strerror(errno));
	if ((session->gpsdata.gps_fd = open(session->gpsdata.gps_device, O_RDONLY|O_NONBLOCK|O_NOCTTY)) < 0) {
	    gpsd_report(LOG_ERROR, "read-only device open failed: %s\n", strerror(errno));
	    return -1;
	}
    }

#ifdef FIXED_PORT_SPEED
    session->saved_baud = FIXED_PORT_SPEED;
#endif

    if (session->saved_baud != -1) {
        /*@i@*/(void)cfsetispeed(&session->ttyset, (speed_t)session->saved_baud);
        /*@i@*/(void)cfsetospeed(&session->ttyset, (speed_t)session->saved_baud);
	(void)tcsetattr(session->gpsdata.gps_fd, TCSANOW, &session->ttyset);
	(void)tcflush(session->gpsdata.gps_fd, TCIOFLUSH);
    }

    session->packet.type = BAD_PACKET;
    if (isatty(session->gpsdata.gps_fd)!=0) {
	/* Save original terminal parameters */
	if (tcgetattr(session->gpsdata.gps_fd,&session->ttyset_old) != 0)
	  return -1;
	(void)memcpy(&session->ttyset,
		     &session->ttyset_old, sizeof(session->ttyset));
	/*
	 * Only block until we get at least one character, whatever the
	 * third arg of read(2) says.
	 */
	/*@ ignore @*/
	memset(session->ttyset.c_cc,0,sizeof(session->ttyset.c_cc));
	session->ttyset.c_cc[VMIN] = 1;
	/*@ end @*/
	/*
	 * Tip from Chris Kuethe: the FIDI chip used in the Trip-Nav
	 * 200 (and possibly other USB GPSes) gets completely hosed
	 * in the presence of flow control.  Thus, turn off CRTSCTS.
	 */
	session->ttyset.c_cflag &= ~(PARENB | PARODD | CRTSCTS);
	session->ttyset.c_cflag |= CREAD | CLOCAL;
	session->ttyset.c_iflag = session->ttyset.c_oflag = session->ttyset.c_lflag = (tcflag_t) 0;

	session->baudindex = 0;
	gpsd_set_speed(session, 
		       gpsd_get_speed(&session->ttyset_old), 'N', 1);
    }
    return session->gpsdata.gps_fd;
}

bool gpsd_write(struct gps_device_t *session, void const *buf, size_t len)
{
     ssize_t status;
     bool ok;
     status = write(session->gpsdata.gps_fd, buf, len);	
     ok = (status == (ssize_t)len);
     (void)tcdrain(session->gpsdata.gps_fd);
     /* no test here now, always print as hex */
     gpsd_report(LOG_IO, "=> GPS: %s%s\n", gpsd_hexdump(buf, len), ok?"":" FAILED");
     return ok;
}

/*
 * This constant controls how long the packet sniffer will spend looking
 * for a packet leader before it gives up.  It *must* be larger than
 * MAX_PACKET_LENGTH or we risk never syncing up at all.  Large values
 * will produce annoying startup lag.
 */
#define SNIFF_RETRIES	256

bool gpsd_next_hunt_setting(struct gps_device_t *session)
/* advance to the next hunt setting  */
{
#ifdef FIXED_PORT_SPEED
    /* just the one fixed port speed... */
    static unsigned int rates[] = {FIXED_PORT_SPEED};
#else /* FIXED_PORT_SPEED not defined */
    /* every rate we're likely to see on a GPS */
    static unsigned int rates[] = {0, 4800, 9600, 19200, 38400, 57600};
#endif /* FIXED_PORT_SPEED defined */

    if (session->packet.retry_counter++ >= SNIFF_RETRIES) {
	session->packet.retry_counter = 0;
	if (session->baudindex++ >= (unsigned int)(sizeof(rates)/sizeof(rates[0]))-1) {
	    session->baudindex = 0;
	    if (session->gpsdata.stopbits++ >= 2)
		return false;			/* hunt is over, no sync */
	}
	gpsd_set_speed(session, 
		       rates[session->baudindex],
		       'N', session->gpsdata.stopbits);
    }

    return true;	/* keep hunting */

}

void gpsd_assert_sync(struct gps_device_t *session)
/* to be called when we want to register that we've synced with a device */
{
    /*
     * We've achieved first sync with the device. Remember the
     * baudrate so we can try it first next time this device
     * is opened.
     */
    if (session->saved_baud == -1)
	session->saved_baud = (int)cfgetispeed(&session->ttyset);
}

void gpsd_close(struct gps_device_t *session)
{
    if (session->gpsdata.gps_fd != -1) {
	if (isatty(session->gpsdata.gps_fd)!=0) {
	    /* force hangup on close on systems that don't do HUPCL properly */
	    /*@ ignore @*/
	    (void)cfsetispeed(&session->ttyset, (speed_t)B0);
	    (void)cfsetospeed(&session->ttyset, (speed_t)B0);
	    /*@ end @*/
	    (void)tcsetattr(session->gpsdata.gps_fd,TCSANOW, &session->ttyset);
	}
	/* this is the clean way to do it */
	session->ttyset_old.c_cflag |= HUPCL;
	(void)tcsetattr(session->gpsdata.gps_fd,TCSANOW,&session->ttyset_old);
	(void)close(session->gpsdata.gps_fd);
	session->gpsdata.gps_fd = -1;
    }
}

