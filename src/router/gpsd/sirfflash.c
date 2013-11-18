/* $Id: sirfflash.c 3770 2006-11-02 05:07:11Z esr $ */
/*
 * This is the SiRF-dependent part of the gpsflash program.
 *
 * If we ever compose our own S-records, dlgsp2.bin looks for this header
 * unsigned char hdr[] = "S00600004844521B\r\n";
 *
 * Here's what Carl Carter at SiRF told us when he sent us informattion
 * on how to build one of these:
 *
 * --------------------------------------------------------------------------
 * Regarding programming the flash, I will attach 2 things for you -- a
 * program called SiRFProg, the source for an older flash programming
 * utility, and a description of the ROM operation.  Note that while the
 * ROM description document is for SiRFstarIII, the interface applies to
 * SiRFstarII systems like you are using.  Here is a little guide to how
 * things work:
 * 
 * 1.  The receiver is put into "internal boot" mode -- this means that it
 * is running off the code contained in the internal ROM rather than the
 * external flash.  You do this by either putting a pull-up resistor on
 * data line 0 and cycling power or by giving a message ID 148.
 * 2.  The internal ROM provides a very primitive boot loader that permits
 * you to load a program into RAM and then switch to it.
 * 3.  The program in RAM is used to handle the erasing and programming
 * chores, so theoretically you could create any program of your own
 * choosing to handle things.  SiRFProg gives you an example of how to do
 * it using Motorola S record files as the programming source.  The program
 * that resides on the programming host handles sending down the RAM
 * program, then communicating with it to transfer the data to program.
 * 4.  Once the programming is complete, you transfer to it by switching to
 * "external boot" mode -- generally this requires a pull-down resistor on
 * data line 0 and either a power cycle or toggling the reset line low then
 * back high.  There is no command that does this.
 * 
 * Our standard utility operates much faster than SiRFProg by using a
 * couple tricks.  One, it transfers a binary image rather than S records
 * (which are ASCII and about 3x the size of the image).  Two, it
 * compresses the binary image using some standard compression algorithm.
 * Three, when transferring the file we boost the port baud rate.  Normally
 * we use 115200 baud as that is all the drivers in most receivers handle.
 * But when supported, we can boost up to 900 kbaud.  Programming at 38400
 * takes a couple minutes.  At 115200 it takes usually under 30 seconds.
 * At 900 k it takes about 6 seconds.
 * --------------------------------------------------------------------------
 *
 * Copyright (c) 2005 Chris Kuethe <chris.kuethe@gmail.com>
 */

#include <sys/types.h>
#include "gpsd_config.h"
#include "gpsd.h"
#include "gpsflash.h"

#if defined(SIRF_ENABLE) && defined(BINARY_ENABLE)

/* From the SiRF protocol manual... may as well be consistent */
#define PROTO_SIRF 0
#define PROTO_NMEA 1

#define BOOST_38400 0
#define BOOST_57600 1
#define BOOST_115200 2

static int
sirfSendUpdateCmd(int pfd){
	bool status;
	/*@ +charint @*/
	static unsigned char msg[] =	{
	    			0xa0,0xa2,	/* header */
				0x00,0x01,	/* message length */
				0x94,		/* 0x94: firmware update */
				0x00,0x00,	/* checksum */
				0xb0,0xb3};	/* trailer */
	/*@ -charint @*/
	status = sirf_write(pfd, msg);
	/* wait a moment for the receiver to switch to boot rom */
	(void)sleep(2);
	return status ? 0 : -1;
}

static int
sirfSendLoader(int pfd, struct termios *term, char *loader, size_t ls){
	unsigned int x;
	int r, speed = 38400;
	/*@i@*/unsigned char boost[] = {'S', BOOST_38400};
	unsigned char *msg;

	if((msg = malloc(ls+10)) == NULL){
		return -1; /* oops. bail out */
	}

	/*@ +charint @*/
#ifdef B115200
	speed = 115200;
	boost[1] = BOOST_115200;
#else
#ifdef B57600
	speed = 57600;
	boost[1] = BOOST_57600;
#endif
#endif
	/*@ -charint @*/

	x = (unsigned)htonl(ls);
	msg[0] = 'S';
	msg[1] = (unsigned char)0;
	memcpy(msg+2, &x, 4); /* length */
	memcpy(msg+6, loader, ls); /* loader */
	memset(msg+6+ls, 0, 4); /* reset vector */
	
	/* send the command to jack up the speed */
	if((r = (int)write(pfd, boost, 2)) != 2) {
		free(msg);
		return -1; /* oops. bail out */
	}

	/* wait for the serial speed change to take effect */
	(void)tcdrain(pfd);
	(void)usleep(1000);

	/* now set up the serial port at this higher speed */
	(void)serialSpeed(pfd, term, speed);

	/* ship the actual data */
	r = binary_send(pfd, (char *)msg, ls+10);
	free(msg);
	return r;
}

static int
sirfSetProto(int pfd, struct termios *term, unsigned int speed, unsigned int proto){
	int i;
	int spd[8] = {115200, 57600, 38400, 28800, 19200, 14400, 9600, 4800};
	/*@ +charint @*/
	static unsigned char sirf[] =	{
				0xa0,0xa2,	/* header */
				0x00,0x31,	/* message length */
				0xa5,		/* message 0xa5: UART config */
				0x00,0,0, 0,0,0,0, 8,1,0, 0,0, /* port 0 */
				0xff,0,0, 0,0,0,0, 0,0,0, 0,0, /* port 1 */
				0xff,0,0, 0,0,0,0, 0,0,0, 0,0, /* port 2 */
				0xff,0,0, 0,0,0,0, 0,0,0, 0,0, /* port 3 */
				0x00,0x00,	/* checksum */
				0xb0,0xb3};	/* trailer */
	/*@ -charint @*/

	if (serialConfig(pfd, term, 38400) == -1)
		return -1;

	sirf[7] = sirf[6] = (unsigned char)proto;
	/*@i@*/i = htonl(speed); /* borrow "i" to put speed into proper byte order */
	/*@i@*/bcopy(&i, sirf+8, 4);

	/* send at whatever baud we're currently using */
	(void)sirf_write(pfd, sirf);
	(void)nmea_send(pfd, "$PSRF100,%u,%u,8,1,0", speed, proto);

	/* now spam the receiver with the config messages */
	for(i = 0; i < (int)(sizeof(spd)/sizeof(spd[0])); i++) {
		(void)serialSpeed(pfd, term, spd[i]);
		(void)sirf_write(pfd, sirf);
		(void)nmea_send(pfd, "$PSRF100,%u,%u,8,1,0", speed, proto);
		(void)tcdrain(pfd);
		(void)usleep(100000);
	}

	(void)serialSpeed(pfd, term, (int)speed);
	(void)tcflush(pfd, TCIOFLUSH);

	return 0;
}

/*@ -nullstate @*/
static int sirfProbe(int fd, char **version)
/* try to elicit a return packet with the firmware version in it */
{
    /*@ +charint @*/
    static unsigned char versionprobe[] = {
				    0xa0, 0xa2, 0x00, 0x02,
				    0x84, 0x00,
				    0x00, 0x84, 0xb0, 0xb3};
    /*@ -charint @*/
    char buf[MAX_PACKET_LENGTH];
    ssize_t status, want;

    gpsd_report(LOG_PROG, "probing with %s\n", 
		gpsd_hexdump(versionprobe, sizeof(versionprobe)));
    if ((status = write(fd, versionprobe, sizeof(versionprobe))) != 10)
	return -1;
    /*
     * Older SiRF chips had a 21-character version message.  Newer 
     * ones (GSW 2.3.2 or later) have an 81-character version message.
     * Accept either.
     */
    want = 0;
    if (expect(fd,"\xa0\xa2\x00\x15\x06", 5, 1))
	want = 21;
    else if (expect(fd,"\xa0\xa2\x00\x51\x06", 5, 1)) 
	want = 81;

    if (want) {
	ssize_t len;
	memset(buf, 0, sizeof(buf));
	for (len = 0; len < want; len += status) {
	    status = read(fd, buf+len, sizeof(buf));
	    if (status == -1)
		return -1;
	}
	gpsd_report(LOG_PROG, "%d bytes = %s\n", len, gpsd_hexdump(buf, (size_t)len));
	*version = strdup(buf);
	return 0;
    } else {
	*version = NULL;
	return -1;
    }
}
/*@ +nullstate @*/

static int sirfPortSetup(int fd, struct termios *term)
{
    /* the firware upload defaults to 38k4, so let's go there */
    return sirfSetProto(fd, term, PROTO_SIRF, 38400);
}

static int sirfVersionCheck(int fd UNUSED, const char *version UNUSED,
			    const char *loader UNUSED, size_t ls UNUSED,
			    const char *firmware UNUSED, size_t fs UNUSED)
{
    /*
     * This implies that any SiRF loader and firmware image is good for 
     * any SiRF chip.  We really want to do more checking here...
     */
    return 0;
}

static int wait2seconds(int fd UNUSED)
{
    /* again we wait, this time for our uploaded code to start running */
    gpsd_report(LOG_PROG, "waiting 2 seconds...\n");
    return (int)sleep(2);
}

static int wait5seconds(int fd UNUSED)
{
    /* wait for firmware upload to settle in */
    gpsd_report(LOG_PROG, "waiting 5 seconds...\n");
    return (int)sleep(5);
}

static int sirfPortWrapup(int fd, struct termios *term)
{
    /* waitaminnit, and drop back to NMEA@4800 for luser apps */
    return sirfSetProto(fd, term, PROTO_NMEA, 4800);
}

struct flashloader_t sirf_type = {
    .name = "SiRF binary",

    /* name of default flashloader */
    .flashloader = "dlgsp2.bin",
    /*
     * I can't imagine a GPS firmware less than 256KB / 2Mbit. The
     * latest build that I have (2.3.2) is 296KB. So 256KB is probably
     * low enough to allow really old firmwares to load.
     *
     * As far as I know, USB receivers have 512KB / 4Mbit of
     * flash. Application note APNT00016 (Alternate Flash Programming
     * Algorithms) says that the S2AR reference design supports 4, 8
     * or 16 Mbit flash memories, but with current firmwares not even
     * using 60% of a 4Mbit flash on a commercial receiver, I'm not
     * going to stress over loading huge images. The define below is
     * 524288 bytes, but that blows up nearly 3 times as S-records.
     * 928K srec -> 296K binary
     */
    .min_firmware_size = 262144,
    .max_firmware_size = 1572864,

    /* a reasonable loader is probably 15K - 20K */
    .min_loader_size = 15440,
    .max_loader_size = 20480,

    /* the command methods */
    .probe = sirfProbe,
    .port_setup = sirfPortSetup,	/* before signal blocking */
    .version_check = sirfVersionCheck,
    .stage1_command = sirfSendUpdateCmd,
    .loader_send  = sirfSendLoader,
    .stage2_command = wait2seconds,
    .firmware_send  = srecord_send,
    .stage3_command = wait5seconds,
    .port_wrapup = sirfPortWrapup,	/* after signals unblock */
};
#endif /* defined(SIRF_ENABLE) && defined(BINARY_ENABLE) */
