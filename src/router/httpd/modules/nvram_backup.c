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
#include <sys/sysinfo.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <utils.h>
#include <broadcom.h>
#include <dd_defs.h>
#include <revision.h>

#define cprintf(fmt, args...)

static int wfsendfile(int fd, off_t offset, size_t nbytes, webs_t wp);
static char *wfgets(char *buf, int len, webs_t fp, int *eof);
size_t wfwrite(void *buf, size_t size, size_t n, webs_t fp);
static size_t wfread(void *buf, size_t size, size_t n, webs_t fp);
static int wfclose(webs_t fp);
int wfflush(webs_t fp);

/*
 * #define cprintf(fmt, args...) do { \ FILE *fp = fopen("/dev/console",
 * "w"); \ if (fp) { \ fprintf(fp, fmt, ## args); \ fclose(fp); \ } \ } while 
 * (0) 
 */

#ifdef HAVE_ANTAIRA
static inline void xorBuffer(char *buffer, int len, char key)
{
	int i = 0;
	for (i = 0; i < len; i++)
		buffer[i] ^= key;
}

static int xorFile(const char *src, const char *dst, char key)
{
	FILE *srcfp = fopen(src, "r");
	if (!srcfp)
		return -1;

	FILE *dstfp = fopen(dst, "w+");
	if (!dstfp)
		return -1;

	int readlen = 32;
	int didread = 0;
	char buffer[readlen];
	while ((didread = fread(buffer, 1, readlen, srcfp)) > 0) {
		xorBuffer(buffer, didread, key);
		fwrite(buffer, 1, didread, dstfp);
	}

	fclose(srcfp);
	fclose(dstfp);

	return 0;
}

static int xorFileMove(const char *src, char key)
{
	const char *dst = "/tmp/.tmp.xor";
	int ret = xorFile(src, dst, key);
	if (ret != 0)
		return ret;

	return rename(dst, src);
}
#endif /*HAVE_ANTAIRA */

static int nv_file_in(char *url, webs_t wp, size_t len, char *boundary)
{
	char buf[1024];
	wp->restore_ret = EINVAL;
	int force = 0;
#ifdef HAVE_REGISTER
	if (!wp->isregistered_real) {
		return -1;
	}
#endif
	/*
	 * Look for our part 
	 */
	while (len > 0) {
		if (!wfgets(buf, MIN(len + 1, sizeof(buf)), wp, NULL))
			return -1;
		len -= strlen(buf);
		if (!strncasecmp(buf, "Content-Disposition:", 20)) {

		if (strstr(buf, "name=\"force\"")) {
				while (len > 0 && strcmp(buf, "\n") && strcmp(buf, "\r\n")) {
					if (!wfgets(buf, MIN(len + 1, 1024), stream, NULL)) {
						debug_free(buf);
						return -1;
					}

					len -= strlen(buf);
				}
				if (!wfgets(buf, MIN(len + 1, 1024), stream, NULL)) {
					debug_free(buf);
					return -1;
				}
				len -= strlen(buf);
				buf[1] = '\0'; // we only want the 1st digit
				force = atoi(buf);		
		} else 			if (strstr(buf, "name=\"file\"")) {
				break;
			}
		}
	}
	/*
	 * Skip boundary and headers 
	 */
	while (len > 0) {
		if (!wfgets(buf, sizeof(buf), wp, NULL))
			return -1;
		len -= strlen(buf);
		if (!strcmp(buf, "\n") || !strcmp(buf, "\r\n"))
			break;
	}
#if defined(HAVE_FONERA) || defined(HAVE_WHRAG108) || defined(HAVE_GATEWORX) || defined(HAVE_MAGICBOX) || defined(HAVE_X86) || \
	defined(HAVE_LS2) || defined(HAVE_MERAKI) || defined(HAVE_CA8) || defined(HAVE_TW6600) || defined(HAVE_LS5)
	eval("rm", "-f", "/tmp/nvram/*"); // delete nvram database
	unlink("/tmp/nvram/.lock"); // delete nvram database
#endif

	unsigned short count;
	FILE *fp = fopen("/tmp/restore.bin", "wb");
	if (!fp)
		return -1;
	char *mem = malloc(len);
	if (!mem) {
		fclose(fp);
		return -1;
	}
	wfread(mem, len, 1, wp);
	fwrite(mem, len, 1, fp);
	fclose(fp);
	debug_free(mem);

#ifdef HAVE_ANTAIRA
	if (nvram_matchi("xor_backup", 1))
		xorFileMove("/tmp/restore.bin", 'K');
#endif /*HAVE_ANTAIRA */

	int ret = nvram_restore("/tmp/restore.bin", force);
	if (ret < 0)
		wp->restore_ret = 99;
	else
		wp->restore_ret = 0;
	unlink("/tmp/restore.bin");
	eval("sync");
	eval("umount", "/usr/local");
	chdir("/www");
	nvram_set("shutdown", "fast");
	return 0;
}

int do_ej(unsigned char method, struct mime_handler *handler, char *path, webs_t stream);

static int sr_config_cgi(unsigned char method, struct mime_handler *handler, char *path, webs_t wp)
{
	int ret;
	if (wp->restore_ret != 0)
		ret = do_ej(METHOD_GET, handler, "Fail.asp", wp);
	else
		ret = do_ej(METHOD_GET, handler, "Success_rest.asp", wp);
	if (ret)
		return ret;
	websDone(wp, 200);

	/*
	 * Reboot if successful 
	 */
	if (wp->restore_ret == 0) {
		nvram_commit();
		struct timespec tim, tim2;
		tim.tv_sec = 5;
		tim.tv_nsec = 0;
		nanosleep(&tim, &tim2);
		sys_reboot();
	}
	return 0;
}

static int do_file_attach(struct mime_handler *handler, char *path, webs_t stream, char *attachment);

#define getRouterName() nvram_exists(NVROUTER_ALT) ? nvram_safe_get(NVROUTER_ALT) : nvram_safe_get(NVROUTER)

static int nv_file_out(unsigned char method, struct mime_handler *handler, char *path, webs_t wp)
{
	int ret;
#ifdef HAVE_REGISTER
	if (!wp->isregistered_real) {
		return -1;
	}
#endif
	char *name = nvram_safe_get("router_name");
	char fname[128];
	snprintf(fname, sizeof(fname), "nvrambak_r%s%s%s_%s.bin", SVN_REVISION, *name ? "_" : "", *name ? name : "",
		 getRouterName());
	nvram_backup("/tmp/nvrambak.bin");

#ifdef HAVE_ANTAIRA
	if (nvram_matchi("xor_backup", 1))
		xorFileMove("/tmp/nvrambak.bin", 'K');
#endif /*HAVE_ANTAIRA */

	ret = do_file_attach(handler, "/tmp/nvrambak.bin", wp, fname);
	unlink("/tmp/nvrambak.bin");
	return ret;
}

static int td_file_in(char *url, webs_t wp, size_t len,
		      char *boundary) //load and set traffic data from config file
{
	char *buf = malloc(2048);
	char *name = NULL;
	char *data = NULL;
	/*
	 * Look for our part 
	 */
	while (len > 0) {
		if (!wfgets(buf, MIN(len + 1, 2048), wp, NULL)) {
			debug_free(buf);
			return -1;
		}
		len -= strlen(buf);
		if (!strncasecmp(buf, "Content-Disposition:", 20)) {
			if (strstr(buf, "name=\"file\"")) {
				break;
			}
		}
	}
	/*
	 * Skip boundary and headers 
	 */
	while (len > 0) {
		if (!wfgets(buf, 2048, wp, NULL)) {
			debug_free(buf);
			return -1;
		}
		len -= strlen(buf);
		if (!strcmp(buf, "\n") || !strcmp(buf, "\r\n"))
			break;
	}

	if (wfgets(buf, 2048, wp, NULL) != NULL) {
		len -= strlen(buf);
		if (strncmp(buf, "TRAFF-DATA", 10) == 0) //sig OK
		{
			while (wfgets(buf, 2048, wp, NULL) != NULL) {
				len -= strlen(buf);
				if (startswith(buf, "traff-")) {
					name = strtok(buf, "=");
					if (strlen(name) == 13) //only set ttraf-XX-XXXX
					{
						data = strtok(NULL, "");
						strtrim_right(data,
							      '\n'); //strip all LF+CR+spaces
						strtrim_right(data, '\r');
						strtrim_right(data, ' ');
						nvram_set(name, data);
					}
				}
			}
		}
	}

	/*
	 * Slurp anything remaining in the request 
	 */
	wfgets(buf, len, wp, NULL);
	nvram_commit();
	debug_free(buf);
	return 0;
}

static int td_config_cgi(unsigned char method, struct mime_handler *handler, char *path, webs_t wp)
{
	int ret = do_ej(METHOD_GET, handler, "Traff_admin.asp", wp);
	if (ret)
		return ret;
	websDone(wp, 200);
	return 0;
}
