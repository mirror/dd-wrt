/* $Id: gpspipe.c 4125 2006-12-12 22:50:23Z ckuethe $ */
/*
 * gpspipe
 *
 * a simple program to connect to a gpsd daemon and dump the received data
 * to stdout
 *
 * This will dump the raw NMEA from gpsd to stdout
 *      gpspipe -r
 *
 * This will dump the super-raw data (gps binary) from gpsd to stdout
 *      gpspipe -R
 *
 * This will dump the GPSD sentences from gpsd to stdout
 *      gpspipe -w
 *
 * This will dump the GPSD and the NMEA sentences from gpsd to stdout
 *      gpspipe -wr
 *
 * Original code by: Gary E. Miller <gem@rellim.com>.  Cleanup by ESR.
 * All rights given to the gpsd project to release under whatever open source
 * license they use.  A thank you would be nice if you use this code.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <termios.h>
#include "gpsd_config.h"
#include "gpsd.h"

static int fd_out = 1;		/* output initially goes to standard output */ 
static void spinner(unsigned int, unsigned int);

/* NMEA-0183 standard baud rate */
#define BAUDRATE B4800

/* Serial port variables */
static struct termios oldtio, newtio;
static char serbuf[255];

/* open the serial port and set it up */
static void open_serial(char* device) 
{
    /* 
     * Open modem device for reading and writing and not as controlling
     * tty.
     */
    if ((fd_out = open(device, O_RDWR|O_NOCTTY)) < 0) {
	fprintf(stderr, "gpspipe: error opening serial port\n");
	exit(1);
    }

    /* Save current serial port settings for later */
    if (tcgetattr(fd_out, &oldtio) != 0) {
	fprintf(stderr, "gpspipe: error reading serial port settings\n");
	exit(1);
    }

    /* Clear struct for new port settings. */
    /*@i@*/bzero(&newtio, sizeof(newtio));

    /* make it raw */
    (void)cfmakeraw(&newtio);
    /* set speed */
    /*@i@*/(void)cfsetospeed(&newtio, BAUDRATE);
	 
    /* Clear the modem line and activate the settings for the port. */
    (void)tcflush(fd_out,TCIFLUSH);
    if (tcsetattr(fd_out,TCSANOW,&newtio) != 0) {
	(void)fprintf(stderr, "gspipe: error configuring serial port\n");
	exit(1);
    }
}

static void usage(void)
{
    (void)fprintf(stderr, "Usage: gpspipe [OPTIONS] [server[:port]]\n\n"
		  "SVN ID: $Id: gpspipe.c 4125 2006-12-12 22:50:23Z ckuethe $ \n"
		  "-h Show this help.\n"
		  "-r Dump raw NMEA.\n"
		  "-R Dump super-raw mode (GPS binary).\n"
		  "-w Dump gpsd native data.\n"
		  "-j Turn on server-side buffering.\n"
		  "-t Time stamp the data.\n"
		  "-s [serial dev] emulate a 4800bps NMEA GPS on serial port (use with '-r').\n"
		  "-n [count] exit after count packets.\n"
		  "-v Print a little spinner.\n"
		  "-V Print version and exit.\n\n"
		  "You must specify one, or both, of -r/-w.\n"
	);
}

int main( int argc, char **argv) 
{
    int sock = 0;
    char buf[4096];
    ssize_t wrote = 0;
    bool timestamp = false;
    bool new_line = true;
    long count = -1;
    int option;
    unsigned int vflag = 0, l = 0;

    char *arg = NULL, *colon1, *colon2, *device = NULL; 
    char *port = DEFAULT_GPSD_PORT, *server = "127.0.0.1";
    char *serialport = NULL;

    buf[0] = '\0';
    while ((option = getopt(argc, argv, "?hrRwjtvVn:s:")) != -1) {
	switch (option) {
	case 'n':
	    count = strtol(optarg, 0, 0);
	    break;
	case 'r':
	    (void)strlcat(buf, "r=1;", sizeof(buf));
	    break;
	case 'R':
	    (void)strlcat(buf, "r=2;", sizeof(buf));
	    break;
	case 't':
	    timestamp = true;
	    break;
	case 'v':
	    vflag++;
	    break;
	case 'w':
	    (void)strlcat(buf, "w=1;", sizeof(buf));
	    break;
	case 'j':
	    (void)strlcat(buf, "j=1;", sizeof(buf));
	    break;
	case 'V':
	    (void)fprintf(stderr, "%s: SVN ID: $Id: gpspipe.c 4125 2006-12-12 22:50:23Z ckuethe $ \n", argv[0]);
	    exit(0);
	case 's':
	    serialport = optarg;
	    break;
	case '?':
	case 'h':
	default:
	    usage();
	    exit(1);
	}
    }

    if (serialport!=NULL && strstr(buf, "r=1")==NULL) {
	(void)fprintf(stderr, "gpsipipe: use of '-s' requires '-r'.\n");
	exit(1);
    }

    if (strstr(buf, "r=")==NULL && strstr(buf, "w=1")==NULL) {
	(void)fprintf(stderr, "gpspipe: one of '-R', '-r' or '-w' is required.\n");
	exit(1);
    }
    /* Grok the server, port, and device. */
    /*@ -branchstate @*/
    if (optind < argc) {
	arg = strdup(argv[optind]);
	/*@i@*/colon1 = strchr(arg, ':');
	server = arg;
	if (colon1 != NULL) {
	    if (colon1 == arg) {
		server = NULL;
	    } else {
		*colon1 = '\0';
	    }
	    port = colon1 + 1;
	    colon2 = strchr(port, ':');
	    if (colon2 != NULL) {
		if (colon2 == port) {
		    port = NULL;
		} else {
		    *colon2 = '\0';
		}
		device = colon2 + 1;
	    }
	}
	colon1 = colon2 = NULL;
    }
    /*@ +branchstate @*/

    /* Open the serial port and set it up. */
    if (serialport)
	open_serial(serialport);

    /*@ -nullpass @*/
    sock = netlib_connectsock( server, port, "tcp");
    if (sock < 0) {
	(void)fprintf(stderr, 
		      "gpspipe: could not connect to gpsd %s:%s, %s(%d)\n",
		      server, port, strerror(errno), errno);
	exit(1);
    }
    /*@ +nullpass @*/

    /* ship the assembled options */
    wrote = write(sock, buf, strlen(buf));
    if ((ssize_t)strlen(buf) != wrote) {
	(void)fprintf(stderr, "gpspipe: write error, %s(%d)\n", 
		      strerror(errno), errno);
	exit(1);
    }

    for(;;) {
	int i = 0;
	int j = 0;
	int readbytes = 0;

	if (vflag)
	    spinner(vflag, l++);
	readbytes = (int)read(sock, buf, sizeof(buf));
	if (readbytes > 0) {
	    for (i = 0 ; i < readbytes ; i++) {
		char c = buf[i];
		if (j < (int)(sizeof(serbuf) - 1)) {
		    serbuf[j++] = buf[i];
		}
		if (new_line && timestamp) {
		    time_t now = time(NULL);

		    new_line = 0;
		    if (fprintf(stdout, "%.24s :", ctime(&now)) <= 0) {
			(void)fprintf(stderr,
				      "gpspipe: write error, %s(%d)\n",
				      strerror(errno), errno);
			exit(1);
		    }
		}
		if (fputc(c, stdout) == EOF) {
		    fprintf( stderr, "gpspipe: Write Error, %s(%d)\n",
			     strerror(errno), errno);
		    exit(1);
		}

		if (c == '\n') {
		    if (serialport != NULL) {
			if (write(fd_out, serbuf, (size_t)j) == -1) {
			    fprintf(stderr, 
				    "gpspipe: Serial port write Error, %s(%d)\n",
				     strerror(errno), errno);
			    exit(1);
			}
			j = 0;
		    }

		    new_line = true;
		    /* flush after every good line */
		    if (fflush(stdout)) {
			(void)fprintf(stderr, "gpspipe: fflush Error, %s(%d)\n",
				strerror(errno), errno);
			exit(1);
		    }
		    if (count > 0) {
			if (0 >= --count) {
			    /* completed count */
			    exit(0);
			}
		    }
		}
	    }
	} else if (readbytes < 0) {
	    (void) fprintf(stderr, "gpspipe: read error %s(%d)\n",
			    strerror(errno), errno);
	    exit(1);
	}
    }

#ifdef __UNUSED__
    if (serialport != NULL) {
	/* Restore the old serial port settings. */
	if (tcsetattr(fd, TCSANOW, &oldtio) != 0) {
	    (void)fprintf(stderr, "Error restoring serial port settings\n");
	    exit(1);
	}
    }

    exit(0);
#endif /* __UNUSED__ */  
}

static void spinner (unsigned int v, unsigned int num) {
    char *spin = "|/-\\";

    (void)fprintf(stderr, "\010%c", spin[(num/(1<<(v-1))) % 4]);
    (void)fflush(stderr);
    return;
}
