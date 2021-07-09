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
static size_t wfwrite(void *buf, size_t size, size_t n, webs_t fp);
static size_t wfread(void *buf, size_t size, size_t n, webs_t fp);
static int wfclose(webs_t fp);
static int wfflush(webs_t fp);
static void do_file_attach(struct mime_handler *handler, char *path, webs_t stream, char *attachment);


static void download_wireguard_config(unsigned char method, struct mime_handler *handler, char *path, webs_t wp)
{
	char fname[128];
	snprintf(fname, sizeof(fname), "%s", path+1);
	char dname[128];
	snprintf(dname,sizeof(dname),"%s", path+1);
	char *p = strstr(dname, "..");
	if (p)
	    return;
	p = strstr(dname, "/");
	if (p)
	    return;

	p = strstr(dname, ".");
	if (p)
	    *p = '_';
	char location[128];
	snprintf(location,sizeof(location,"/tmp/wireguard/%s", dname);
	do_file_attach(handler, location, wp, fname);
	return;
}

