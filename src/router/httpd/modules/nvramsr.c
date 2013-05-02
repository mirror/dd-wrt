
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
#include <cyutils.h>

#define MIN_BUF_SIZE    4096

#define SERVICE_MODULE "/lib/services.so"
#define cprintf(fmt, args...)

/*
 * #define cprintf(fmt, args...) do { \ FILE *fp = fopen("/dev/console",
 * "w"); \ if (fp) { \ fprintf(fp, fmt, ## args); \ fclose(fp); \ } \ } while 
 * (0) 
 */

static int restore_ret;

void nv_file_in(char *url, webs_t wp, int len, char *boundary)
{

	char buf[1024];
	restore_ret = EINVAL;
#ifdef HAVE_REGISTER
	if (!isregistered_real()) {
		return;
	}
#endif

	/*
	 * Look for our part 
	 */
	while (len > 0) {
		if (!wfgets(buf, MIN(len + 1, sizeof(buf)), wp))
			return;
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
		if (!wfgets(buf, sizeof(buf), wp))
			return;
		len -= strlen(buf);
		if (!strcmp(buf, "\n") || !strcmp(buf, "\r\n"))
			break;
	}
#if defined(HAVE_FONERA) || defined(HAVE_WHRAG108) || defined(HAVE_GATEWORX) || defined(HAVE_MAGICBOX) || defined(HAVE_X86) || defined(HAVE_LS2) || defined(HAVE_MERAKI) || defined(HAVE_CA8) || defined(HAVE_TW6600)  || defined(HAVE_LS5)
	eval("rm", "-f", "/tmp/nvram/*");	// delete nvram database
	eval("rm", "-f", "/tmp/nvram/.lock");	// delete nvram database
#endif
	// fprintf (stderr, "file write");
	unsigned short count;
	FILE *fp = fopen("/tmp/restore.bin", "wb");
	char *mem = malloc(len);
	wfread(mem, len, 1, wp);
	fwrite(mem, len, 1, fp);
	fclose(fp);
	int ret = nvram_restore("/tmp/restore.bin");
	if (ret < 0)
		restore_ret = 99;
	else
		restore_ret = 0;
	eval("rm", "-f", "/tmp/restore.bin");
	chdir("/www");
}

void sr_config_cgi(struct mime_handler *handler, char *path, webs_t wp, char *query)
{
	if (restore_ret != 0)
		do_ej(handler, "Fail.asp", wp, NULL);
	else
		do_ej(handler, "Success_rest.asp", wp, NULL);

	websDone(wp, 200);

	/*
	 * Reboot if successful 
	 */
	if (restore_ret == 0) {
		nvram_commit();
		sleep(5);
		sys_reboot();
	}
}

void nv_file_out(struct mime_handler *handler, char *path, webs_t wp, char *query)
{

#ifdef HAVE_REGISTER
	if (!isregistered_real()) {
		return;
	}
#endif
	nvram_backup("/tmp/nvrambak.bin");
	do_file_attach(handler, "/tmp/nvrambak.bin", wp, query, "nvrambak.bin");
	eval("rm", "-f", "/tmp/nvrambak.bin");
	return;
}

void td_file_in(char *url, webs_t wp, int len, char *boundary)	//load and set traffic data from config file
{
	char buf[2048];
	char *name = NULL;
	char *data = NULL;

	/*
	 * Look for our part 
	 */
	while (len > 0) {
		if (!wfgets(buf, MIN(len + 1, sizeof(buf)), wp))
			return;
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
		if (!wfgets(buf, sizeof(buf), wp))
			return;
		len -= strlen(buf);
		if (!strcmp(buf, "\n") || !strcmp(buf, "\r\n"))
			break;
	}

	if (wfgets(buf, sizeof(buf), wp) != NULL) {
		len -= strlen(buf);
		if (strncmp(buf, "TRAFF-DATA", 10) == 0)	//sig OK
		{
			while (wfgets(buf, sizeof(buf), wp) != NULL) {
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
	while (len--) {
#ifdef HAVE_HTTPS
		if (do_ssl) {
			wfgets(buf, 1, wp);
		} else
#endif
			(void)fgetc(wp->fp);
	}

	nvram_commit();
}

void td_config_cgi(struct mime_handler *handler, char *path, webs_t wp, char *query)
{
	do_ej(handler, "Traff_admin.asp", wp, NULL);
	websDone(wp, 200);
}
