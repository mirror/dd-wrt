/*
 * alarmserver.c
 *
 * Copyright (C) 2005 - 2025 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

static int wfsendfile(int fd, off_t offset, size_t nbytes, webs_t wp);
static char *wfgets(char *buf, int len, webs_t fp, int *eof);
size_t wfwrite(void *buf, size_t size, size_t n, webs_t fp);
static size_t wfread(void *buf, size_t size, size_t n, webs_t fp);
static int wfclose(webs_t fp);
int wfflush(webs_t fp);

/*
 * "w"); \ if (fp) { \ fprintf(fp, fmt, ## args); \ fclose(fp); \ } \ } while 
 * (0) 
 */
static char *getXMLTag(const char *p, const char *tag, char *buf)
{
	char begin[64];
	char end[64];
	sprintf(begin, "<%s>", tag);
	sprintf(end, "</%s>", tag);
	char *s = strstr(p, begin);
	char *e = strstr(p, end);
	if (!s || !e)
		return NULL;
	strlcpy(buf, s, e - s + strlen(begin));
	return buf;
}

static int alarmserver_in(char *url, webs_t wp, size_t len, char *boundary)
{
	char buf[1024];
	wp->restore_ret = EINVAL;
	int force = 0;
	int keepip = 0;
	int keepsettings = 0;
	dd_loginfo("alarmserver", "%s:%d len %d\n", __func__, __LINE__,len);
	if (nvram_match("alarmserver", "1")) {
		/*
	 * Look for our part 
	 */
		while (len > 0) {
			if (!wfgets(buf, MIN(len + 1, sizeof(buf)), wp, NULL))
				return -1;
			fprintf(stderr, "slurp %s\n" buf);
			len -= strlen(buf);
			if (!strncasecmp(buf, "Content-Disposition:", 20)) {
				break;
			}
		}
	dd_loginfo("alarmserver", "%s:%d\n", __func__, __LINE__);
		/*
	 * Skip boundary and headers 
	 */
		while (len > 0) {
			if (!wfgets(buf, sizeof(buf), wp, NULL))
				return -1;
			fprintf(stderr, "slurp2 %s\n" buf);
			len -= strlen(buf);
			if (!strcmp(buf, "\n") || !strcmp(buf, "\r\n"))
				break;
		}

		unsigned short count;
		char *mem = malloc(len);
		if (!mem) {
			return -1;
		}
	dd_loginfo("alarmserver", "%s:%d\n", __func__, __LINE__);
		wfread(mem, len, 1, wp);
		mem[len] = 0;
		char s_date[128];
		char s_name[128];
		char s_desc[128];
		char *date = getXMLTag(mem, "dateTime", s_date);
		char *name = getXMLTag(mem, "channelName", s_name);
		char *desc = getXMLTag(mem, "eventDescription", s_desc);
		fprintf(stderr, "event %s\n", mem);
		fprintf(stderr, "date %s\n", date);
		fprintf(stderr, "date %s\n", name);
		fprintf(stderr, "desc %s\n", desc);
	dd_loginfo("alarmserver", "%s:%d\n", __func__, __LINE__);
		debug_free(mem);
		//		if (date && name && desc)
		//		sysprintf("%s \"%s\" \"%s\" \"%s\"", nvram_safe_get("alarmserver_cmd"), date, name, desc);
	}
	return 0;
}

int do_ej(unsigned char method, struct mime_handler *handler, char *path, webs_t stream);
