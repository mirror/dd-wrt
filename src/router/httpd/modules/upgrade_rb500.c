/*
 * upgrade_rb500.c
 *
 * Copyright (C) 2005 - 2021 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
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
#include <dd_defs.h>

#define MIN_BUF_SIZE 4096
#define CODE_PATTERN_ERROR 9999
static int upgrade_ret;

static int
// do_upgrade_cgi(char *url, FILE *stream)
do_upgrade_cgi(unsigned char method, struct mime_handler *handler, char *url,
	       webs_t streamm) // jimmy,
// https,
// 8/6/2003
{
#ifndef ANTI_FLASH
	int ret;
	if (upgrade_ret)
		ret = do_ej(METHOD_GET, handler, "Fail_u_s.asp", stream);
	else
		ret = do_ej(METHOD_GET, handler, "Success_u_s.asp", stream);
	if (ret)
		return ret;
	websDone(stream, 200);

	/*
	 * Reboot if successful 
	 */
	if (upgrade_ret == 0) {
		struct timespec tim, tim2;
		tim.tv_sec = 5;
		tim.tv_nsec = 0;
		nanosleep(&tim, &tim2);
		sys_reboot();
	}
#else
	int ret = do_ej(METHOD_GET, handler, "Fail_u_s.asp", stream);
	if (ret)
		return ret;
	websDone(stream, 200);
#endif
	return 0;
}

static int
// sys_upgrade(char *url, FILE *stream, int *total)
sys_upgrade(char *url, webs_t stream, size_t *total, int type) // jimmy,
	// https,
	// 8/6/2003
{
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
	if (DO_SSL(stream))
		ACTION("ACT_WEBS_UPGRADE");
	else
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
	// while (total && *total)
	{
		wfread(&buf[0], 1, 5, stream);
		*total -= 5;
		if (buf[0] != 'R' || buf[1] != 'B' || buf[2] != '5' || buf[3] != '0' || buf[4] != '0') {
			ret = -1;
			goto err;
		}
		int linuxsize;
		int fssize;

		wfread(&linuxsize, 1, 4, stream);
		wfread(&fssize, 1, 4, stream);
		*total -= 8;
		safe_fwrite(&linuxsize, 1, 4, fifo);
		safe_fwrite(&fssize, 1, 4, fifo);
		linuxsize += fssize;
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
	unsigned long fssize;

	linuxsize = 0;
	linuxsize += getc(fifo);
	linuxsize += getc(fifo) * 256;
	linuxsize += getc(fifo) * 256 * 256;
	linuxsize += getc(fifo) * 256 * 256 * 256;
	fssize = 0;
	fssize += getc(fifo);
	fssize += getc(fifo) * 256;
	fssize += getc(fifo) * 256 * 256;
	fssize += getc(fifo) * 256 * 256 * 256;
	fprintf(stderr, "Write Linux %d\n", linuxsize);
	FILE *out = fopen("/dev/cf/card0/part1", "wb");

	for (i = 0; i < linuxsize; i++)
		putc(getc(fifo), out);
	fclose(out);
	fprintf(stderr, "Write FileSys %d\n", fssize);
	out = fopen("/dev/cf/card0/part2", "wb");
	for (i = 0; i < fssize; i++)
		putc(getc(fifo), out);
	fclose(out);

	/*
	 * Wait for write to terminate 
	 */
	// waitpid (pid, &ret, 0);
	ret = 0;
err:
	if (buf)
		debug_free(buf);
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

static int
// do_upgrade_post(char *url, FILE *stream, int len, char *boundary)
do_upgrade_post(char *url, webs_t stream, size_t len, char *boundary) // jimmy,
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
	// system ("cat /www/Success_u_s.asp > /dev/null");
	// system ("cat /www/Fail_u_s.asp > /dev/null");

	/*
	 * Look for our part 
	 */
	while (len > 0) {
		if (!wfgets(buf, MIN(len + 1, sizeof(buf)), stream, NULL))
			return -1;
		len -= strlen(buf);
		if (!strncasecmp(buf, "Content-Disposition:", 20)) {
			if (strstr(buf, "name=\"erase\"")) {
				while (len > 0 && strcmp(buf, "\n") && strcmp(buf, "\r\n")) {
					if (!wfgets(buf, MIN(len + 1, sizeof(buf)), stream, NULL))
						return -1;
					len -= strlen(buf);
				}
				if (!wfgets(buf, MIN(len + 1, sizeof(buf)), stream, NULL))
					return -1;
				len -= strlen(buf);
				buf[1] = '\0'; // we only want the 1st digit
				nvram_set("sv_restore_defaults", buf);
				nvram_commit();
			} else if (strstr(buf,
					  "name=\"file\"")) // upgrade image
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
		if (!wfgets(buf, MIN(len + 1, sizeof(buf)), stream, NULL))
			return -1;
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
	if (nvram_matchi("sv_restore_defaults", 1)) {
		unlink("/usr/local/nvram/nvram.bin");
	}
	/*
	 * Slurp anything remaining in the request 
	 */
	wfgets(buf, len, stream, NULL);
	fprintf(stderr, "upgrade done()\n");
#endif
	return 0;
}
