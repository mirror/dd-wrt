/*
 * Copyright 2002 Damien Miller <djm@mindrot.org> All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "common.h"

static void
usage(void)
{
	fprintf(stderr, "Usage: [-c ctlsock] softflowctl [command]\n");
}

int
main(int argc, char **argv)
{
	const char *ctlsock_path;
	char buf[8192], *command;
	struct sockaddr_un ctl;
	socklen_t ctllen;
	int ctlsock, ch;
	FILE *ctlf;
	extern char *optarg;
	extern int optind;

	ctlsock_path = DEFAULT_CTLSOCK;
	while ((ch = getopt(argc, argv, "hc:")) != -1) {
		switch (ch) {
		case 'h':
			usage();
			return (0);
		case 'c':
			ctlsock_path = optarg;
			break;
		default:
			fprintf(stderr, "Invalid commandline option.\n");
			usage();
			exit(1);
		}
	}
	
	/* Accept only one argument */
	if (optind != argc - 1) {
		usage();
		exit(1);
	}
	command = argv[optind];

	memset(&ctl, '\0', sizeof(ctl));
	if (strlcpy(ctl.sun_path, ctlsock_path, sizeof(ctl.sun_path)) >= 
	    sizeof(ctl.sun_path)) {
		fprintf(stderr, "Control socket path too long.\n");
		exit(1);
	}
	ctl.sun_path[sizeof(ctl.sun_path) - 1] = '\0';
	ctl.sun_family = AF_UNIX;
	ctllen = offsetof(struct sockaddr_un, sun_path) +
            strlen(ctlsock_path) + 1;
#ifdef SOCK_HAS_LEN 
	ctl.sun_len = ctllen;
#endif
	if ((ctlsock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "ctl socket() error: %s\n", 
		    strerror(errno));
		exit(1);
	}
	if (connect(ctlsock, (struct sockaddr*)&ctl, sizeof(ctl)) == -1) {
		fprintf(stderr, "ctl connect(\"%s\") error: %s\n",
		    ctl.sun_path, strerror(errno));
		exit(1);
	}
	
	if ((ctlf = fdopen(ctlsock, "r+")) == NULL) {
		fprintf(stderr, "fdopen: %s\n", strerror(errno));
		exit(1);
	}
	setlinebuf(ctlf);
	
	/* Send command */
	if (fprintf(ctlf, "%s\n", command) < 0) {
		fprintf(stderr, "write: %s\n", strerror(errno));
		exit(1);
	}

	/* Write out reply */
	while((fgets(buf, sizeof(buf), ctlf)) != NULL)
		fputs(buf, stdout);

	fclose(ctlf);
	close(ctlsock);

	exit(0);
}
