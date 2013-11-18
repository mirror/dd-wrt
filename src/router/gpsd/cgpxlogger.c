/* $Id: cgpxlogger.c 3666 2006-10-26 23:11:51Z ckuethe $ */
/*
 * Copyright (c) 2005,2006 Chris Kuethe <chris.kuethe@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <sys/cdefs.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gpsd_config.h"
#include "gps.h"

#define BS 512

#define NUM 8
char *pollstr = "SPAMDQTV\n";
char *host = "127.0.0.1";
char *port = "2947";
unsigned int sl = 5;

struct gps_data_t *gpsdata;

char *progname;

void process(struct gps_data_t *, char *, size_t, int);
void usage(void);
void bye(int);
void write_record(struct gps_data_t *gpsdata);
void header(void);
void footer(void);
void track_start(void);
void track_end(void);
int tracking = 0;

struct {
	double latitude;
	double longitude;
	float altitude;
	float speed;
	float course;
	float hdop;
	short svs;
	char status;
	char mode;
	char time[32];
} gps_ctx;


int
main(int argc, char **argv){
	int ch;
	fd_set fds;
	int casoc = 0;

	progname = argv[0];
	while ((ch = getopt(argc, argv, "hVi:j:s:p:")) != -1){
	switch (ch) {
	case 'i':
		sl = (unsigned int)atoi(optarg);
		if (sl < 1)
			sl = 1;
		if (sl >= 3600)
			fprintf(stderr,
			    "WARNING: polling interval is an hour or more!\n");
		break;
	case 'j':
		casoc = (unsigned int)atoi(optarg);
		casoc = casoc ? 1 : 0;
		break;
	case 's':
		host = optarg;
		break;
	case 'p':
		port = optarg;
		break;
	case 'V':
		(void)fprintf(stderr, "SVN ID: $Id: cgpxlogger.c 3666 2006-10-26 23:11:51Z ckuethe $ \n");
		exit(0);
	default:
		usage();
		/* NOTREACHED */
	}
	}

	gpsdata = gps_open(host, port);
	if (!gpsdata) {
		char *err_str;
		switch (errno) {
		case NL_NOSERVICE:
			err_str = "can't get service entry";
			break;
		case NL_NOHOST:
			err_str = "can't get host entry";
			break;
		case NL_NOPROTO:
			err_str = "can't get protocol entry";
			break;
		case NL_NOSOCK:
			err_str = "can't create socket";
			break;
		case NL_NOSOCKOPT:
			err_str = "error SETSOCKOPT SO_REUSEADDR";
			break;
		case NL_NOCONNECT:
			err_str = "can't connect to host";
			break;
		default:
			err_str = "Unknown";
			break;
		}
		fprintf(stderr,
		    "cgpxlogger: no gpsd running or network error: %d, %s\n",
		    errno, err_str);
		exit(1);
	}

	signal(SIGINT, bye);
	signal(SIGTERM, bye);
	signal(SIGQUIT, bye);
	signal(SIGHUP, bye);

	header();

	if (casoc)
		gps_query(gpsdata, "j1\n");

	gps_set_raw_hook(gpsdata, process);

	for(;;){
		int data;
		struct timeval tv;

		FD_ZERO(&fds);
		FD_SET(gpsdata->gps_fd, &fds);

		gps_query(gpsdata, pollstr);

		tv.tv_usec = 250000;
		tv.tv_sec = 0;
		data = select(gpsdata->gps_fd + 1, &fds, NULL, NULL, &tv);

		if (data < 0) {
			fprintf(stderr,"%s\n", strerror(errno));
			exit(2);
		}
		else if (data)
			gps_poll(gpsdata);

		sleep(sl);
	}
}

void usage(){
	fprintf(stderr,
	    "Usage: %s [-h] [-s server] [-p port] [-i interval] [-j casoc]\n",
	    progname);
	fprintf(stderr,
	    "\tdefaults to '%s -s 127.0.0.1 -p 2947 -i 5 -j 0'\n",
	    progname);
	exit(1);
}

void bye(int signum){
	footer();
	fprintf(stderr, "Exiting on signal %d!\n", signum);
	gps_close(gpsdata);
	exit(0);
}

void process(struct gps_data_t *gpsdata,
	     char *buf UNUSED, size_t len UNUSED, int level UNUSED){

	if ((gpsdata->fix.mode > 1) && (gpsdata->status > 0))
		write_record(gpsdata);
	else
		track_end();
}

void write_record(struct gps_data_t *gpsdata){
	track_start();
	printf("      <trkpt lat=\"%.6f\" ", gpsdata->fix.latitude );
	printf("lon=\"%.6f\">\n", gpsdata->fix.longitude );

	if ((gpsdata->status >= 2) && (gpsdata->fix.mode >= 3)){
		/* dgps or pps */
		if (gpsdata->fix.mode == 4) { /* military pps */
			printf("        <fix>pps</fix>\n");
		} else { /* civilian dgps or sbas */
			printf("        <fix>dgps</fix>\n");
		}
	} else { /* no dgps or pps */
		if (gpsdata->fix.mode == 3) {
			printf("        <fix>3d</fix>\n");
		} else if (gpsdata->fix.mode == 2) {
			printf("        <fix>2d</fix>\n");
		} else if (gpsdata->fix.mode == 1) {
			printf("        <fix>none</fix>\n");
		} /* don't print anything if no fix indicator */
	}

	/* print altitude if we have a fix and it's 3d of some sort */
	if ((gpsdata->fix.mode >= 3) && (gpsdata->status >= 1))
		printf("        <ele>%.2f</ele>\n", gpsdata->fix.altitude);

	/* print # satellites used in fix, if reasonable to do so */
	if (gps_ctx.mode >= 2) {
		printf("        <hdop>%.1f</hdop>\n", gpsdata->hdop);
		printf("        <sat>%d</sat>\n", gpsdata->satellites_used);
	}

	if (gpsdata->satellites_used) { /* plausible timestamp */
		char scr[128];
		printf("        <time>%s</time>\n",
		    unix_to_iso8601(gpsdata->fix.time, scr, sizeof(scr)));
	}
	printf("      </trkpt>\n");
	fflush(stdout);
}

void header(){
	printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	printf("<gpx version=\"1.1\" creator=\"GPX GPSD client\"\n");
	printf("        xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
	printf("        xmlns=\"http://www.topografix.com/GPX/1.1\"\n");
	printf("        xsi:schemaLocation=\"http://www.topografix.com/GPS/1/1\n");
	printf("        http://www.topografix.com/GPX/1/1/gpx.xsd\">\n");
	printf("  <metadata>\n");
	printf("    <name>GPX GPSD client</name>\n");
	printf("    <author>Chris Kuethe (chris.kuethe@gmail.com)</author>\n");
	printf("    <copyright>2-clause BSD License</copyright>\n");
	printf("  </metadata>\n");
	printf("\n");
	printf("\n");
}

void footer(){
	track_end();
	printf("</gpx>\n");
}

void track_start(){
	if (tracking != 0)
		return;
	printf("<!-- track start -->\n  <trk>\n    <trkseg>\n");
	tracking = 1;
}


void track_end(){
	if (tracking == 0)
		return;
	printf("    </trkseg>\n  </trk>\n<!-- track end -->\n");
	tracking = 0;
}

