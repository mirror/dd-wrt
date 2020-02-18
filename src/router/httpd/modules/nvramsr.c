
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
static int wfprintf(webs_t fp, char *fmt, ...);
static size_t wfwrite(void *buf, size_t size, size_t n, webs_t fp);
static size_t wfread(void *buf, size_t size, size_t n, webs_t fp);
static int wfclose(webs_t fp);
static int wfflush(webs_t fp);

/*
 * #define cprintf(fmt, args...) do { \ FILE *fp = fopen("/dev/console",
 * "w"); \ if (fp) { \ fprintf(fp, fmt, ## args); \ fclose(fp); \ } \ } while 
 * (0) 
 */

static int nv_file_in(char *url, webs_t wp, size_t len, char *boundary)
{

	char buf[1024];
	wp->restore_ret = EINVAL;
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
			if (strstr(buf, "name=\"file\"")) {
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
#if defined(HAVE_FONERA) || defined(HAVE_WHRAG108) || defined(HAVE_GATEWORX) || defined(HAVE_MAGICBOX) || defined(HAVE_X86) || defined(HAVE_LS2) || defined(HAVE_MERAKI) || defined(HAVE_CA8) || defined(HAVE_TW6600)  || defined(HAVE_LS5)
	eval("rm", "-f", "/tmp/nvram/*");	// delete nvram database
	unlink("/tmp/nvram/.lock");	// delete nvram database
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
	int ret = nvram_restore("/tmp/restore.bin", 0);
	if (ret < 0)
		wp->restore_ret = 99;
	else
		wp->restore_ret = 0;
	unlink("/tmp/restore.bin");
	chdir("/www");
	return 0;
}

static void do_ej(unsigned char method, struct mime_handler *handler, char *path, webs_t stream);

static void sr_config_cgi(unsigned char method, struct mime_handler *handler, char *path, webs_t wp)
{
	if (wp->restore_ret != 0)
		do_ej(METHOD_GET, handler, "Fail.asp", wp);
	else
		do_ej(METHOD_GET, handler, "Success_rest.asp", wp);

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
}

static void do_file_attach(struct mime_handler *handler, char *path, webs_t stream, char *attachment);

static void nv_file_out(unsigned char method, struct mime_handler *handler, char *path, webs_t wp)
{

#ifdef HAVE_REGISTER
	if (!wp->isregistered_real) {
		return;
	}
#endif
	char *name = nvram_safe_get("router_name");
	char fname[128];
#if defined(HAVE_ONNET) || defined(HAVE_ANTAIRA)
	snprintf(fname, sizeof(fname), "nvrambak_r%s%s%s_%s.bin", SVN_REVISION, *name ? "_" : "", *name ? name : "", nvram_safe_get(NVROUTER_ALT));
#else
	snprintf(fname, sizeof(fname), "nvrambak_r%s%s%s_%s.bin", SVN_REVISION, *name ? "_" : "", *name ? name : "", nvram_safe_get("DD_BOARD"));
#endif
	nvram_backup("/tmp/nvrambak.bin");
	do_file_attach(handler, "/tmp/nvrambak.bin", wp, fname);
	unlink("/tmp/nvrambak.bin");
	return;
}

static int td_file_in(char *url, webs_t wp, size_t len, char *boundary)	//load and set traffic data from config file
{
	char *buf = malloc(2048);
	char *name = NULL;
	char *data = NULL;

	/*
	 * Look for our part 
	 */
	while (len > 0) {
		if (!wfgets(buf, MIN(len + 1, 2048), wp, NULL)) {
			free(buf);
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
			free(buf);
			return -1;
		}
		len -= strlen(buf);
		if (!strcmp(buf, "\n") || !strcmp(buf, "\r\n"))
			break;
	}

	if (wfgets(buf, 2048, wp, NULL) != NULL) {
		len -= strlen(buf);
		if (strncmp(buf, "TRAFF-DATA", 10) == 0)	//sig OK
		{
			while (wfgets(buf, 2048, wp, NULL) != NULL) {
				len -= strlen(buf);
				if (startswith(buf, "traff-")) {
					name = strtok(buf, "=");
					if (strlen(name) == 13)	//only set ttraf-XX-XXXX
					{
						data = strtok(NULL, "");
						strtrim_right(data, '\n');	//strip all LF+CR+spaces
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
	return 0;
}

static void td_config_cgi(unsigned char method, struct mime_handler *handler, char *path, webs_t wp)
{
	do_ej(METHOD_GET, handler, "Traff_admin.asp", wp);
	websDone(wp, 200);
}
