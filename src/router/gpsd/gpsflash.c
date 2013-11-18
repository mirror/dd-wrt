/* $Id: gpsflash.c 4078 2006-12-05 05:00:54Z ckuethe $ */
/*
 * This is the GPS-type-independent part of the gpsflash program.
 *
 * Copyright (c) 2005 Chris Kuethe <chris.kuethe@gmail.com>
 */
#include <sys/types.h>
#include <stdarg.h>
#include "gpsd_config.h"
#include "gpsd.h"
#include "gpsflash.h"

/* block size when writing to the serial port. related to FIFO size */
#define WRBLK 128

static char *progname;
static int verbosity = 0;

static bool srec_check(const char *data);

void gpsd_report(int errlevel, const char *fmt, ... )
/* assemble command in printf(3) style, use stderr or syslog */
{
    if (errlevel <= verbosity) {
	char buf[BUFSIZ];
	va_list ap;

	(void)strlcpy(buf, progname, BUFSIZ);
	(void)strlcat(buf, ": ", BUFSIZ);
	va_start(ap, fmt) ;
	(void)vsnprintf(buf + strlen(buf), sizeof(buf)-strlen(buf), fmt, ap);
	va_end(ap);
	(void)fputs(buf, stdout);
    }
}

static void
usage(void){
    fprintf(stderr, "Usage: %s [-v d] [-n] [-l <loader_file>] -f <firmware_file> {<tty>}\n", progname);
}

int
serialSpeed(int pfd, struct termios *term, int speed){
	int rv;
	int r = 0;

	switch(speed){
#ifdef B115200
	case 115200:
		speed = B115200;
		break;
#endif
#ifdef B57600
	case 57600:
		speed = B57600;
		break;
#endif
	case 38400:
		speed = B38400;
		break;
#ifdef B28800
	case 28800:
		speed = B28800;
		break;
#endif
	case 19200:
		speed = B19200;
		break;
#ifdef B14400
	case 14400:
		speed = B14400;
		break;
#endif
	case 9600:
		speed = B9600;
		break;
	case 4800:
		speed = B9600;
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	/* set UART speed */
	(int)tcgetattr(pfd, term);
	/*@ ignore @*/
	cfsetispeed(term, speed);
	cfsetospeed(term, speed);
	/*@ end @*/
	while (((rv = tcsetattr(pfd, TCSAFLUSH, term)) == -1) && \
	    (errno == EINTR) && (r < 3)) {
		/* retry up to 3 times on EINTR */
		(void)usleep(1000);
		r++;
	}

	if(rv == -1)
		return -1;
	else
		return 0;
}


int
serialConfig(int pfd, struct termios *term, int speed){
	int rv;
	int r = 0;

	/* get the current terminal settings */
	(void)tcgetattr(pfd, term);
	/* set the port into "raw" mode. */
	/*@i@*/cfmakeraw(term);
	term->c_lflag &=~ (ICANON);
	/* Enable serial I/O, ignore modem lines */
	term->c_cflag |= (CLOCAL | CREAD);
	/* No output postprocessing */
	term->c_oflag &=~ (OPOST);
	/* 8 data bits */
	term->c_cflag |= CS8;
	term->c_iflag &=~ (ISTRIP);
	/* No parity */
	term->c_iflag &=~ (INPCK);
	term->c_cflag &=~ (PARENB | PARODD);
	/* 1 Stop bit */
	term->c_cflag &=~ (CSIZE | CSTOPB);
	/* No flow control */
	term->c_iflag &=~ (IXON | IXOFF);
#if defined(CCTS_OFLOW) && defined(CRTS_IFLOW) && defined(MDMBUF)
	term->c_oflag &=~ (CCTS_OFLOW | CRTS_IFLOW | MDMBUF);
#endif
#if defined(CRTSCTS)
	term->c_oflag &=~ (CRTSCTS);
#endif

	/* we'd like to read back at least 2 characters in .2sec */
	/*@i@*/term->c_cc[VMIN] = 2;
	/*@i@*/term->c_cc[VTIME] = 2;

	/* apply all the funky control settings */
	while (((rv = tcsetattr(pfd, TCSAFLUSH, term)) == -1) && \
	    (errno == EINTR) && (r < 3)) {
		/* retry up to 3 times on EINTR */
		(void)usleep(1000);
		r++;
	}

	if(rv == -1)
		return -1;
	
	/* and if that all worked, try change the UART speed */
	return serialSpeed(pfd, term, speed);
}

int
binary_send(int pfd, char *data UNUSED , size_t ls){
	unsigned char *msg;
	size_t nbr, nbs, nbx;
	ssize_t r;
	static int count;
	double start = timestamp();

	/*@ -compdef @*/
	if((msg = malloc(ls+10)) == NULL){
		return -1; /* oops. bail out */
	}

	fprintf(stderr, "gpsflash: transferring binary... \010");
	count = 0;

	nbr = ls+10; nbs = WRBLK ; nbx = 0;
	while(nbr){
		if(nbr > WRBLK )
			nbs = WRBLK ;
		else
			nbs = nbr;

r0:		if((r = write(pfd, msg+nbx, nbs)) == -1){
			if (errno == EAGAIN){ /* retry */
				(void)tcdrain(pfd); /* wait a moment */
				errno = 0; /* clear errno */
				nbr -= r; /* number bytes remaining */
				nbx += r; /* number bytes sent */
				goto r0;
			} else {
			        (void)free(msg);
				return -1; /* oops. bail out */
			}
		}
		nbr -= r;
		nbx += r;

		(void)fputc("-/|\\"[count % 4], stderr);
		(void)fputc('\010', stderr);
		(void)fflush(stdout);
	}
	/*@ +compdef @*/

	(void)fprintf(stderr, "...done (%2.2f sec).\n", timestamp()-start);

	(void)free(msg);
	return 0;
}


int
srecord_send(int pfd, char *data, size_t len){
	int r, i;
	size_t tl;
	char sendbuf[85], recvbuf[8];
	static int count;
	double start = timestamp();

	/* srecord loading is interactive. send line, get reply */
	/* when sending S-records, check for SA/S5 or SE */

	fprintf(stderr, "gpsflash: transferring S-records... \010");
	count = 0;

	memset(recvbuf, 0, 8);
	i = 0;

	while(strlen(data)){
		/* grab a line of firmware, ignore line endings */
		if ((r = (int)strlen(data))){
			memset(sendbuf,0,85);
			if((r = sscanf(data, "%80s", sendbuf)) == EOF)
				return 0;

			tl = strlen(sendbuf);
			if ((tl < 1) || (tl > 80))
				return -1;

			data += tl;
			len -= tl;

			while((data[0] != 'S') && (data[0] != '\0'))
				data++;

			sendbuf[tl] = '\r';
			sendbuf[tl+1] = '\n';
			tl += 2;

			if ((++i % 1000) == 0)
				printf ("%6d\n", i);

			(void)tcflush(pfd, TCIFLUSH);
			if((r = (int)write(pfd, sendbuf, tl+2)) != (int)tl+2)
				return -1; /* oops. bail out */

			(void)tcdrain(pfd);
			if((r = (int)read(pfd, recvbuf, 7)) == -1)
				return -1; /* oops. bail out */

			if (!((recvbuf[0] == 'S') && ((recvbuf[1] == 'A') || (recvbuf[1] == '5'))))
				return -1; /* oops. bail out */
		}

		(void)fputc("-/|\\"[count % 4], stderr);
		(void)fputc('\010', stderr);
		(void)fflush(stdout);
	}

	(void)fprintf(stderr, "...done (%2.2f sec).\n", timestamp()-start);
	return 0;
}

bool 
expect(int pfd, const char *str, size_t len, time_t timeout)
/* keep reading till we see a specified expect string or time out */
{
    size_t got = 0;
    char ch;
    double start = timestamp();

    gpsd_report(LOG_PROG, "expect(%s, %d)\n", 
		gpsd_hexdump((char *)str, len),
		timeout);

    for (;;) {
	if (read(pfd, &ch, 1) != 1)
	    return false;		/* I/O failed */
	gpsd_report(LOG_RAW, "I see %d: %02x\n", got, (unsigned)(ch & 0xff));
	if (timestamp() - start > (double)timeout)
	    return false;		/* we're timed out */
	else if (got == len)
	    return true;		/* we're done */
	else if (ch == str[got])
	    got++;			/* match continues */
	else
	    got = 0;			/* match fails, retry */
    }
}


#if defined(SIRF_ENABLE) && defined(BINARY_ENABLE)
/* add new types by adding pointers to their driver blocks to this list */
/*@ -nullassign @*/
static struct flashloader_t *types[] = {&sirf_type, NULL};
/*@ +nullassign @*/
#else
/* add new types by adding pointers to their driver blocks to this list */
/*@ -nullassign @*/
static struct flashloader_t *types[] = {NULL, NULL};
/*@ +nullassign @*/
#endif

int
main(int argc, char **argv){

	int ch;
	int lfd, ffd, pfd;
	size_t ls,  fs;
	bool fflag = false, lflag = false, nflag = false;
	struct stat sb;
	struct flashloader_t *gpstype, **gp;
	char *fname = NULL;
	char *lname = NULL;
	char *port = NULL;
	char *warning;
	struct termios term;
	sigset_t sigset;
	char *firmware = NULL;
	char *loader = NULL;
	char *version = NULL;

	progname = argv[0];

	while ((ch = getopt(argc, argv, "f:l:nVv:")) != -1)
		switch (ch) {
		case 'f':
			fname = optarg;
			fflag = true;
			break;
		case 'l':
			lname = optarg;
			lflag = true;
			break;
		case 'n':
			nflag = true;
			break;
		case 'v':
			verbosity = atoi(optarg);
			break;
		case 'V':
			(void)fprintf(stderr, "SVN ID: $Id: gpsflash.c 4078 2006-12-05 05:00:54Z ckuethe $ \n");
			exit(0);
		default:
			usage();
			exit(0);
			/* NOTREACHED */
		}

	argc -= optind;
	argv += optind;

	/* there is exactly one required argument, a tty device */
	if (argc == 1) 
	    port = argv[0];
	else {
	    usage();
	    exit(0);
	}

	if (!nflag && 
	    (((warning = getenv("I_READ_THE_WARNING")) == NULL) ||
	     (strcmp(warning, "why oh why didn't i take the blue pill")!=0))){
	    printf("\nThis program rewrites your receiver's flash ROM.\n");
	    printf("If done improperly this will permanently ruin your\n");
	    printf("receiver. We insist you read the gpsflash manpage\n");
	    printf("before you break something.\n\n");
	    nflag = true;
	}

	if (!nflag) {
	    /* make sure we have meaningful flags */
	    if (!fflag || fname == NULL) {
		usage();
		return 1;
	    }
	}

	/* Open the serial port, blocking is OK */
	if((pfd = open(port, O_RDWR | O_NOCTTY , 0600)) == -1) {
		gpsd_report(LOG_ERROR, "open(%s)\n", port);
		return 1;
	}

	/* try to get an identification string out of the firmware */
	gpstype = NULL;
	for (gp = types; *gp; gp++) {
	    gpstype = *gp;
	    gpsd_report(LOG_PROG, "probing for %s\n", gpstype->name);
	    if (gpstype->probe(pfd, &version) == 0)
		break;
	}
	if (gpstype == NULL || version == NULL) {
	    gpsd_report(LOG_ERROR, "not a known GPS type\n");
	    return 1;
	}

	/* OK, we have a known type */
	gpsd_report(LOG_SHOUT, "GPS is %s, version '%s'.\n", gpstype->name, version);
	if (lname == NULL)
	    lname = (char *)gpstype->flashloader;

	if (nflag) {
	    gpsd_report(LOG_PROG, "probe finished.\n");
	    return 0;
	}

	/* there may be a type-specific setup method */
	memset(&term, 0, sizeof(term));
	if(gpstype->port_setup(pfd, &term) == -1) {
		gpsd_report(LOG_ERROR, "port_setup()\n");
		return 1;
	}

	gpsd_report(LOG_PROG, "port set up...\n");

	/* Open the loader file */
	if((lfd = open(lname, O_RDONLY, 0444)) == -1) {
	    gpsd_report(LOG_ERROR, "open(%s)\n", lname);
	    return 1;
	}

	/* fstat() its file descriptor. Need the size, and avoid races */
	if(fstat(lfd, &sb) == -1) {
	    gpsd_report(LOG_ERROR, "fstat(%s)\n", lname);
	    return 1;
	}

	/* minimal sanity check on loader size. also prevents bad malloc() */
	ls = (size_t)sb.st_size;
	if ((ls < gpstype->min_loader_size)||(ls > gpstype->max_loader_size)){
	    gpsd_report(LOG_ERROR, "preposterous loader size: %d\n", ls);
	    return 1;
	}

	gpsd_report(LOG_PROG, "passed sanity checks...\n");

	/* malloc a loader buffer */
	if ((loader = malloc(ls)) == NULL) {
	    gpsd_report(LOG_ERROR, "malloc(%d)\n", ls);
	    return 1;
	}

	if (read(lfd, loader, ls) != (ssize_t)ls) {
	    (void)free(loader);
	    gpsd_report(LOG_ERROR, "read(%d)\n", ls);
	    return 1;
	}

	/* don't care if close fails - kernel will force close on exit() */
	(void)close(lfd);

	gpsd_report(LOG_PROG, "loader read in...\n");

	/* Open the firmware image file */
	/*@ -nullpass @*/
	if((ffd = open(fname, O_RDONLY, 0444)) == -1) {
	    (void)free(loader);
	    gpsd_report(LOG_ERROR, "open(%s)]n", fname);
	    return 1;
	}
	if(fstat(ffd, &sb) == -1) {
	    (void)free(loader);
	    gpsd_report(LOG_ERROR, "fstat(%s)\n", fname);
	    return 1;
	}

	/* minimal sanity check on firmware size. also prevents bad malloc() */
	fs = (size_t)sb.st_size;
	if ((fs < gpstype->min_firmware_size) || (fs > gpstype->max_firmware_size)){
	    (void)free(loader);
	    gpsd_report(LOG_ERROR, "preposterous firmware size: %d\n", fs);
	    return 1;
	}

	/* malloc an image buffer */
	if ((firmware = malloc(fs+1)) == NULL) {
	    (void)free(loader);
	    gpsd_report(LOG_ERROR, "malloc(%u)\n", (unsigned)fs);
	    return 1;
	}

	/* get the firmware */
	if (read(ffd, firmware, fs) != (ssize_t)fs) {
	    (void)free(loader);
	    (void)free(firmware);
	    gpsd_report(LOG_ERROR, "read(%u)\n", (unsigned)fs);
	    return 1;
	}

	firmware[fs] = '\0';

	/* don't care if close fails - kernel will force close on exit() */
	(void)close(ffd);

	gpsd_report(LOG_PROG, "firmware read in...\n");

	/* did we just read some S-records? */
	if (!((firmware[0] == 'S') && ((firmware[1] >= '0') && (firmware[1] <= '9')))){ /* srec? */
	    (void)free(loader);
	    (void)free(firmware);
	    gpsd_report(LOG_ERROR, "%s: not an S-record file\n", fname);
	    return 1;
	}

	if(gpstype->version_check(pfd, version, loader, ls, firmware, fs)==-1){
		(void)free(loader);
		(void)free(firmware);
		gpsd_report(LOG_ERROR, "version_check()\n");
		return 1;
	}

	/*
	 * dlgps2.bin starts flashing when it sees valid srecords.
	 * validate the entire image before we flash. shooting self
	 * in foot is bad, mmmkay?
	 */
	gpsd_report(LOG_PROG, "validating firmware\n");
	if (srec_check(firmware)){
	    (void)free(loader);
	    (void)free(firmware);
	    gpsd_report(LOG_ERROR, "%s: corrupted firmware image\n", fname);
	    return 1;
	}
	/*@ +nullpass @*/
	gpsd_report(LOG_PROG, "firmware validated\n");

	gpsd_report(LOG_PROG, "version checked...\n");

	gpsd_report(LOG_PROG, "blocking signals...\n");

	/* once we get here, we are uninterruptable. handle signals */
	(void)sigemptyset(&sigset);
	(void)sigaddset(&sigset, SIGINT);
	(void)sigaddset(&sigset, SIGHUP);
	(void)sigaddset(&sigset, SIGQUIT);
	(void)sigaddset(&sigset, SIGTSTP);
	(void)sigaddset(&sigset, SIGSTOP);
	(void)sigaddset(&sigset, SIGKILL);

	if(sigprocmask(SIG_BLOCK, &sigset, NULL) == -1) {
		(void)free(loader);
		(void)free(firmware);
		gpsd_report(LOG_ERROR,"sigprocmask\n");
		return 1;
	}

	/* send the command to begin the update */
	if(gpstype->stage1_command!=NULL && (gpstype->stage1_command(pfd) == -1)) {
		(void)free(loader);
		(void)free(firmware);
		gpsd_report(LOG_ERROR, "Stage 1 update command\n");
		return 1;
	}

	gpsd_report(LOG_PROG, "sending loader...\n");

	/* send the bootstrap/flash programmer */
	if(gpstype->loader_send(pfd, &term, loader, ls) == -1) {
		(void)free(loader);
		(void)free(firmware);
		gpsd_report(LOG_ERROR, "Loader send\n");
		return 1;
	}
	(void)free(loader);

	gpsd_report(LOG_PROG, "initializing firmware load...\n");

	/* send any command needed to demarcate the two loads */
	if(gpstype->stage2_command!=NULL && (gpstype->stage2_command(pfd) == -1)) {
	    (void)free(firmware);
	    gpsd_report(LOG_ERROR, "Stage 2 update command\n");
	    return 1;
	}

	gpsd_report(LOG_PROG, "performing firmware load...\n");

	/* and now, poke the actual firmware over */
	if(gpstype->firmware_send(pfd, firmware, fs) == -1) {
	    (void)free(firmware);
	    gpsd_report(LOG_ERROR, "Firmware send\n");
	    return 1;
	}
	(void)free(firmware);

	gpsd_report(LOG_PROG, "finishing firmware load...\n");

	/* send any command needed to finish the firmware load */
	if(gpstype->stage3_command!=NULL && (gpstype->stage3_command(pfd) == -1)) {
	    gpsd_report(LOG_ERROR, "Stage 3 update command\n");
	    return 1;
	}

	gpsd_report(LOG_PROG, "unblocking signals...\n");

	if(sigprocmask(SIG_UNBLOCK, &sigset, NULL) == -1) {
		gpsd_report(LOG_ERROR,"sigprocmask\n");
		return 1;
	}

	/* type-defined wrapup, take our tty to GPS's post-flash settings */
	if(gpstype->port_wrapup(pfd, &term) == -1) {
		gpsd_report(LOG_ERROR, "port_wrapup()\n");
		return 1;
	}

	gpsd_report(LOG_PROG, "finished.\n");

	/* return() from main(), to take advantage of SSP compilers */
	return 0;
}

static bool
srec_check(const char *data){
	int i, l, n, x, y, z;
	char buf[85];

	l = 0;
	while(strlen(data)){
		/* grab a line of firmware, ignore line endings */
		memset(buf,0,85);
		l++;
		if(sscanf(data, "%80s", buf) == EOF){
			gpsd_report(LOG_ERROR, "line %d read failed\n", l);
			return true;
		}

		n = (int)strlen(buf);
		if ((n < 1) || (n > 80)){
			gpsd_report(LOG_ERROR, "firmware line %d invalid length %d\n", l, n);
			return true;
		}

		/* advance to the next srecord */
		data += n;
		while((data[0] != 'S') && (data[0] != '\0'))
			data++;

		if (buf[0] != 'S'){
			gpsd_report(LOG_INF, "%s\n", buf);
			gpsd_report(LOG_ERROR, "firmware line %d doesn't begin with 'S'.\n", l);
			return true;
		}

		x = hex2bin(buf+2);
		y = (n - 4)/2;
		if (x != y){
			gpsd_report(LOG_INF, "buf: '%s'\n", buf);
			gpsd_report(LOG_ERROR, "firmware line %d length error: %d != %d\n", l, x, y);
			return true;
		}

		x = hex2bin(buf+n-2);
		y = 0;
		for(i = 2; i < n-2; i+=2){
			z = hex2bin(buf+i);
			y += z;
		}
		y &= 0xff; y ^= 0xff;
		if (x != y){
			gpsd_report(LOG_INF, "buf: '%s'\n", buf);
			gpsd_report(LOG_ERROR, "firmware line %d checksum error: %x != %x\n", l, x, y);
			return true;
		}
	}
	return false;
}
