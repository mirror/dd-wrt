/*
 * nvram_backup.c
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

#ifdef HAVE_EROUTER
#define HAVE_RB600
#endif

#define MIN_BUF_SIZE 4096
#define CODE_PATTERN_ERROR 9999
#define HAVE_NEW_UPGRADE

static int
// do_upgrade_cgi(char *url, FILE *stream)
do_upgrade_cgi(unsigned char method, struct mime_handler *handler, char *url,
	       webs_t stream) // jimmy, https,
	// 8/6/2003
{
	int ret;
#ifndef ANTI_FLASH
	fprintf(stderr, "do post\n");
	if (stream->upgrade_ret)
		ret = do_ej(METHOD_GET, handler, "Fail_u_s.asp", stream);
	else
		ret = do_ej(METHOD_GET, handler, "Success_u_s.asp", stream);
	if (ret)
		return ret;

	websDone(stream, 200);

	/*
	 * Reboot if successful 
	 */
	if (stream->upgrade_ret == 0) {
		eval("umount", "/usr/local");
		nvram_set("shutdown", "fast");
		sys_reboot();
	}
#else
	ret = do_ej(METHOD_GET, handler, "Fail_u_s.asp", stream);
	if (ret)
		return ret;
	websDone(stream, 200);
#endif
	return 0;
}

#ifdef HAVE_RB600
#define swap(x)                                                              \
	((unsigned int)((((unsigned int)(x) & (unsigned int)0x000000ffUL)    \
			 << 24) |                                            \
			(((unsigned int)(x) & (unsigned int)0x0000ff00UL)    \
			 << 8) |                                             \
			(((unsigned int)(x) & (unsigned int)0x00ff0000UL) >> \
			 8) |                                                \
			(((unsigned int)(x) & (unsigned int)0xff000000UL) >> \
			 24)))

#endif

static int
// sys_upgrade(char *url, FILE *stream, int *total)
sys_upgrade(char *url, webs_t stream, size_t *total, int type) // jimmy,
	// https,
	// 8/6/2003
{
	lcdmessage("System Upgrade");
#ifndef ANTI_FLASH
	FILE *fifo = NULL;
	char *write_argv[4];
	pid_t pid;
	char *buf = NULL;
	int count, ret = 0;
	long flags = -1;
	int size = BUFSIZ;
	int i = 0;

	// diag_led(DIAG, START_LED); // blink the diag led
	if (DO_SSL(stream))
		ACTION("ACT_WEBS_UPGRADE");
	else
		ACTION("ACT_WEB_UPGRADE");

	char *drv = getdisc();
	char drive[64];
	sprintf(drive, "/dev/%s", drv);

#ifndef HAVE_EROUTER
	fprintf(stderr, "backup nvram\n");
	FILE *in = fopen("/usr/local/nvram/nvram.bin", "rb");
	if (in) {
		int size = nvram_size();
		char *mem = malloc(size);
		fread(mem, size, 1, in);
		fclose(in);
		FILE *in = fopen(drive, "r+b");
		int f_flags = fcntl(fileno(in), F_GETFL);
		f_flags |= O_SYNC;
		fcntl(fileno(in), F_SETFL, f_flags);
		fseeko(in, 0, SEEK_END);
		off_t mtdlen = ftello(in);
		fseeko(in, mtdlen - (size + 65536), SEEK_SET);
		fwrite(mem, size, 1, in);
		fflush(in);
		fsync(fileno(in));
		fclose(in);
		debug_free(mem);
	}
#endif

	/*
	 * Feed write from a temporary FIFO 
	 */
#ifdef HAVE_NEW_UPGRADE
	char *write_argv_buf[8];
	eval("mkdir", "-p", "/tmp/new_root");
	eval("mount", "-n", "-t", "tmpfs", "none", "/tmp/new_root");
	eval("mkdir", "-p", "/tmp/new_root/tmp");
	write_argv_buf[0] = "update-prepare.sh";
	write_argv_buf[1] = "uploadfile.bin";
	write_argv_buf[2] = drive;
	write_argv_buf[3] = "nomount";
	write_argv_buf[4] = "noreboot";
	write_argv_buf[5] = "usedd";
	write_argv_buf[6] = NULL;
	if (!drv || !(fifo = fopen("/tmp/new_root/tmp/uploadfile.bin", "wb"))) {
#else
	if (!drv || !(fifo = fopen(drive, "wb"))) {
#endif
		if (!ret)
			ret = errno;
		goto err;
	}
	int f_flags = fcntl(fileno(fifo), F_GETFL);
	f_flags |= O_SYNC;
	fcntl(fileno(fifo), F_SETFL, f_flags);

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
#ifdef HAVE_NEWPORT
		if (memcmp(buf, "NEWP1", 5)) {
			ret = -1;
			goto err;
		}
#else
		if (memcmp(buf, "WRAP1", 5)) {
			ret = -1;
			goto err;
		}
#endif
		int linuxsize;

		wfread(&linuxsize, 1, 4, stream);
		*total -= 4;
//              safe_fwrite(&linuxsize, 1, 4, fifo);
#ifdef HAVE_RB600
		linuxsize = swap(linuxsize);
#endif
		for (i = 0; i < linuxsize / MIN_BUF_SIZE; i++) {
			wfread(&buf[0], 1, MIN_BUF_SIZE, stream);
			fwrite(&buf[0], 1, MIN_BUF_SIZE, fifo);
			// fprintf(stderr, "%d bytes written\n", i * MIN_BUF_SIZE);
			fsync(fileno(fifo));
		}

		wfread(&buf[0], 1, linuxsize % MIN_BUF_SIZE, stream);
		fwrite(&buf[0], 1, linuxsize % MIN_BUF_SIZE, fifo);
		fsync(fileno(fifo));
		*total -= linuxsize;
	}
	fclose(fifo);
#ifdef HAVE_NEW_UPGRADE
	ret = _evalpid(write_argv_buf, NULL, 0, &pid);
	waitpid(pid, &ret, 0);
#else
	killall("watchdog", SIGKILL);
	/*
	 * Wait for write to terminate 
	 */
	// waitpid (pid, &ret, 0);
#endif
	cprintf("done\n");
	ret = 0;
err:
	free(drv);
	if (buf)
		debug_free(buf);

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

	stream->upgrade_ret = EINVAL;

	// Let below files loaded to memory
	// To avoid the successful screen is blank after web upgrade.

	/*
	 * Look for our part 
	 */
	while (len > 0) {
		if (!wfgets(buf, MIN(len + 1, sizeof(buf)), stream, NULL))
			return -1;
		len -= strlen(buf);
		if (!strncasecmp(buf, "Content-Disposition:", 20)) {
			if (strstr(buf, "name=\"erase\"")) {
				while (len > 0 && strcmp(buf, "\n") &&
				       strcmp(buf, "\r\n")) {
					if (!wfgets(buf,
						    MIN(len + 1, sizeof(buf)),
						    stream, NULL))
						return -1;
					len -= strlen(buf);
				}
				if (!wfgets(buf, MIN(len + 1, sizeof(buf)),
					    stream, NULL))
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
	stream->upgrade_ret = sys_upgrade(NULL, stream, &len, type);
	fprintf(stderr, "core upgrade done() %zu\n", len);
	/*
	 * Restore factory original settings if told to. This will also cause a
	 * restore defaults on reboot of a Sveasoft firmware. 
	 */
#ifndef HAVE_EROUTER
	if (nvram_matchi("sv_restore_defaults", 1)) {
		unlink("/usr/local/nvram/nvram.bin");
		char drive[64];
		char *drv = getdisc();
		if (!drv)
			return -1;
		int size = nvram_size();
		sprintf(drive, "/dev/%s", drv);
		free(drv);
		FILE *in = fopen(drive, "r+b");
		fseeko(in, 0, SEEK_END);
		off_t mtdlen = ftell(in);
		fseeko(in, mtdlen - (size + 65536), SEEK_SET);
		int i;
		for (i = 0; i < size; i++)
			putc(0, in); // erase backup area
		fflush(in);
		fsync(fileno(in));
		fclose(in);
	}
#else
	if (nvram_matchi("sv_restore_defaults", 1)) {
		eval("erase", "nvram");
	}
#endif
	/*
	 * Slurp anything remaining in the request 
	 */
	wfgets(buf, len, stream, NULL);
	fprintf(stderr, "upgrade done()\n");
#endif
	return 0;
}
