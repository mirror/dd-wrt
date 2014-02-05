
/*
 * Broadcom Home Gateway Reference Design
 * Web Page Configuration Support Routines
 *
 * Copyright 2001-2003, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 * $Id: upgrade.c,v 1.4 2005/11/30 11:53:42 seg Exp $
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <broadcom.h>
#include <cyutils.h>

#define MIN_BUF_SIZE    4096
#define CODE_PATTERN_ERROR 9999
static int upgrade_ret;

#ifdef HAVE_X86
static char *getdisc(void)	// works only for squashfs 
{
	int i;
	static char ret[4];
	unsigned char *disks[] = { "sda2", "sdb2", "sdc2", "sdd2", "sde2", "sdf2", "sdg2", "sdh2", "sdi2" };
	for (i = 0; i < 9; i++) {
		char dev[64];

		sprintf(dev, "/dev/%s", disks[i]);
		FILE *in = fopen(dev, "rb");

		if (in == NULL)
			continue;	// no second partition or disc does not
		// exist, skipping
		char buf[4];

		fread(buf, 4, 1, in);
		if (buf[0] == 'h' && buf[1] == 's' && buf[2] == 'q' && buf[3] == 't') {
			fclose(in);
			// filesystem detected
			strncpy(ret, disks[i], 3);
			return ret;
		}
		fclose(in);
	}
	return NULL;
}
#endif
void
// do_upgrade_cgi(char *url, FILE *stream)
do_upgrade_cgi(struct mime_handler *handler, char *url, webs_t stream, char *query)	// jimmy, https,
							// 8/6/2003
{
#ifndef ANTI_FLASH
	fprintf(stderr, "do post\n");
	if (upgrade_ret)
		do_ej(handler, "Fail_u_s.asp", stream, NULL);
	else
		do_ej(handler, "Success_u_s.asp", stream, NULL);
	fprintf(stderr, "websdone\n");

	websDone(stream, 200);
	fprintf(stderr, "reboot\n");

	/*
	 * Reboot if successful 
	 */
	if (upgrade_ret == 0) {
		sleep(4);
		eval("umount", "/usr/local");
		sys_reboot();
	}
#else
	do_ej(handler, "Fail_u_s.asp", stream, NULL);
	websDone(stream, 200);
#endif
}

#ifdef HAVE_RB600
#define swap(x) \
	((unsigned int )( \
			(((unsigned int )(x) & (unsigned int )0x000000ffUL) << 24) | \
			(((unsigned int )(x) & (unsigned int )0x0000ff00UL) <<  8) | \
			(((unsigned int )(x) & (unsigned int )0x00ff0000UL) >>  8) | \
			(((unsigned int )(x) & (unsigned int )0xff000000UL) >> 24) ))

#endif

int
// sys_upgrade(char *url, FILE *stream, int *total)
sys_upgrade(char *url, webs_t stream, int *total, int type)	// jimmy,
								// https,
								// 8/6/2003
{
	lcdmessage("System Upgrade");
#ifndef ANTI_FLASH
	char upload_fifo[] = "/tmp/uploadXXXXXX";
	FILE *fifo = NULL;
	char *write_argv[4];
	pid_t pid;
	char *buf = NULL;
	int count, ret = 0;
	long flags = -1;
	int size = BUFSIZ;
	int i = 0;

	{
		write_argv[0] = "write";
		write_argv[1] = upload_fifo;
		write_argv[2] = "linux";
		write_argv[3] = NULL;
	}

	// diag_led(DIAG, START_LED); // blink the diag led
	C_led(1);
#ifdef HAVE_HTTPS
	if (do_ssl)
		ACTION("ACT_WEBS_UPGRADE");
	else
#endif
		ACTION("ACT_WEB_UPGRADE");

	/*
	 * Feed write from a temporary FIFO 
	 */
	if (!mktemp(upload_fifo) || !(fifo = fopen(upload_fifo, "w"))) {
		if (!ret)
			ret = errno;
		goto err;
	}

	/*
	 * Set nonblock on the socket so we can timeout 
	 */

	/*
	 ** The buffer must be at least as big as what the stream file is
	 ** using so that it can read all the data that has been buffered
	 ** in the stream file. Otherwise it would be out of sync with fn
	 ** select specially at the end of the data stream in which case
	 ** the select tells there is no more data available but there in
	 ** fact is data buffered in the stream file's buffer. Since no
	 ** one has changed the default stream file's buffer size, let's
	 ** use the constant BUFSIZ until someone changes it.
	 **/

	if (size < MIN_BUF_SIZE)
		size = MIN_BUF_SIZE;
	if ((buf = safe_malloc(size)) == NULL) {
		ret = ENOMEM;
		goto err;
	}

	/*
	 * Pipe the rest to the FIFO 
	 */
	cprintf("Upgrading\n");
	// while (total && *total)
	{
		wfread(&buf[0], 1, 5, stream);
		*total -= 5;
		if (buf[0] != 'W' || buf[1] != 'R' || buf[2] != 'A' || buf[3] != 'P' || buf[4] != '1') {
			ret = -1;
			goto err;
		}
		int linuxsize;

		wfread(&linuxsize, 1, 4, stream);
		*total -= 4;
		safe_fwrite(&linuxsize, 1, 4, fifo);
#ifdef HAVE_RB600
		linuxsize = swap(linuxsize);
#endif
		for (i = 0; i < linuxsize / MIN_BUF_SIZE; i++) {
			wfread(&buf[0], 1, MIN_BUF_SIZE, stream);
			fwrite(&buf[0], 1, MIN_BUF_SIZE, fifo);
		}

		wfread(&buf[0], 1, linuxsize % MIN_BUF_SIZE, stream);
		fwrite(&buf[0], 1, linuxsize % MIN_BUF_SIZE, fifo);
		*total -= linuxsize;

	}
	fclose(fifo);
	fifo = NULL;
	fifo = fopen(upload_fifo, "rb");
	unsigned long linuxsize;

	linuxsize = 0;
	linuxsize += getc(fifo);
	linuxsize += getc(fifo) * 256;
	linuxsize += getc(fifo) * 256 * 256;
	linuxsize += getc(fifo) * 256 * 256 * 256;
	char dev[128];

	char drive[64];
#ifdef HAVE_RB600
	sprintf(drive, "/dev/sda");
#else
	sprintf(drive, "/dev/%s", getdisc());
#endif
	fprintf(stderr, "Write Linux %d to %s\n", linuxsize, dev);
	//backup nvram
	fprintf(stderr, "backup nvram\n");
	FILE *in = fopen("/usr/local/nvram/nvram.bin", "rb");
	if (in) {
		char *mem = malloc(65536);
		fread(mem, 65536, 1, in);
		fclose(in);
		in = fopen(drive, "r+b");
		fseeko(in, 0, SEEK_END);
		off_t mtdlen = ftello(in);
		fseeko(in, mtdlen - (65536 * 2), SEEK_SET);
		fwrite(mem, 65536, 1, in);
		fflush(in);
		fsync(fileno(in));
		fclose(in);
		free(mem);
	}
	fprintf(stderr, "write system\n");
	FILE *out = fopen(drive, "r+b");
	char *flashbuf = (char *)malloc(linuxsize);
	if (!flashbuf)		// not enough memory, use direct way
	{
		for (i = 0; i < linuxsize; i++)
			putc(getc(fifo), out);
	} else {
		//read into temp buffer
		fread(flashbuf, linuxsize, 1, fifo);
		fwrite(flashbuf, linuxsize, 1, out);
		free(flashbuf);
	}
	fflush(out);
	fsync(fileno(out));
	fclose(out);
	/*
	 * Wait for write to terminate 
	 */
	// waitpid (pid, &ret, 0);
	cprintf("done\n");
	ret = 0;
err:
	if (buf)
		free(buf);
	if (fifo)
		fclose(fifo);
	unlink(upload_fifo);

	// diag_led(DIAG, STOP_LED);
	// C_led (0);
	fprintf(stderr, "Idle\n");
	ACTION("ACT_IDLE");

	return ret;
#else
	return 0;
#endif
}

void
// do_upgrade_post(char *url, FILE *stream, int len, char *boundary)
do_upgrade_post(char *url, webs_t stream, int len, char *boundary)	// jimmy, 
									// https, 
									// 8/6/2003
{

	killall("udhcpc", SIGKILL);
#ifndef ANTI_FLASH
	char buf[1024];
	int type = 0;

	upgrade_ret = EINVAL;

	// Let below files loaded to memory
	// To avoid the successful screen is blank after web upgrade.

	/*
	 * Look for our part 
	 */
	while (len > 0) {
		if (!wfgets(buf, MIN(len + 1, sizeof(buf)), stream))
			return;
		len -= strlen(buf);
		if (!strncasecmp(buf, "Content-Disposition:", 20)) {
			if (strstr(buf, "name=\"erase\"")) {
				while (len > 0 && strcmp(buf, "\n")
				       && strcmp(buf, "\r\n")) {
					if (!wfgets(buf, MIN(len + 1, sizeof(buf)), stream))
						return;
					len -= strlen(buf);
				}
				if (!wfgets(buf, MIN(len + 1, sizeof(buf)), stream))
					return;
				len -= strlen(buf);
				buf[1] = '\0';	// we only want the 1st digit
				nvram_set("sv_restore_defaults", buf);
				nvram_commit();
			} else if (strstr(buf, "name=\"file\""))	// upgrade image
			{
				type = 0;
				break;
			}
		}
	}

	/*
	 * Skip boundary and headers 
	 */
	while (len > 0) {
		if (!wfgets(buf, MIN(len + 1, sizeof(buf)), stream))
			return;
		len -= strlen(buf);
		if (!strcmp(buf, "\n") || !strcmp(buf, "\r\n"))
			break;
	}
	upgrade_ret = sys_upgrade(NULL, stream, &len, type);
	fprintf(stderr, "core upgrade done() %d\n", len);
	/*
	 * Restore factory original settings if told to. This will also cause a
	 * restore defaults on reboot of a Sveasoft firmware. 
	 */
	if (nvram_match("sv_restore_defaults", "1")) {
		system2("rm -f /usr/local/nvram/nvram.bin");
		char drive[64];
#ifdef HAVE_RB600
		sprintf(drive, "/dev/sda");
#else
		sprintf(drive, "/dev/%s", getdisc());
#endif
		FILE *in = fopen(drive, "r+b");
		fseeko(in, 0, SEEK_END);
		off_t mtdlen = ftell(in);
		fseeko(in, mtdlen - (65536 * 2), SEEK_SET);
		int i;
		for (i = 0; i < 65536; i++)
			putc(0, in);	// erase backup area
		fflush(in);
		fsync(fileno(in));
		fclose(in);
	}
	/*
	 * Slurp anything remaining in the request 
	 */
	while (len--) {
#ifdef HAVE_HTTPS
		if (do_ssl) {
			wfgets(buf, 1, stream);
		} else {
			(void)fgetc(stream);
		}
#else
		(void)fgetc(stream);
#endif

	}
#endif
	fprintf(stderr, "upgrade done()\n");

}
