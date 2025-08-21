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
	s += strlen(begin);
	strlcpy(buf, s, e - s + 1);
	return buf;
}

struct hikvision_dispatch {
	const char *filename;
	int (*handler)(const char *filename, const char *buf, size_t len);
};

static int hik_generic(const char *filename, const char *mem, size_t len)
{
	char s_date[128];
	char s_name[128];
	char s_desc[128];
	char s_addr[128],
	char *date = getXMLTag(mem, "dateTime", s_date);
	char *name = getXMLTag(mem, "channelName", s_name);
	char *desc = getXMLTag(mem, "eventDescription", s_desc);
	char *addr = getXMLTag(mem, "ipAddress", s_addr);
	if (filename && date && name && desc && addr)
		sysprintf("%s \\\"%s\\\" \\\"%s\\\" \\\"%s\\\" \\\"%s\\\" \\\"%s\\\"", nvram_safe_get("alarmserver_cmd"), filename, date, addr, name, desc);
	else if (date && name && desc && addr)
		sysprintf("%s \\\"unspecified\\\" \\\"%s\\\" \\\"%s\\\" \\\"%s\\\" \\\"%s\\\"", nvram_safe_get("alarmserver_cmd"), date, addr, name, desc);
	return 0;
}

static struct hikvision_dispatch dispatch[] = {
	/* v2, still need a nanddump of newer cams for getting all events */
	{ "MoveDetection.xml", hik_generic },
	{ "AudioException.xml", hik_generic },
	{ "hderror.xml", hik_generic },
	/* v1 */
	{ "Motion alarm", hik_generic },
	{ "disc error alarm", hik_generic },
	{ "shelteralarm alarm", hik_generic },
	{ "facedetection alarm", hik_generic },
	{ "defocus alarm", hik_generic },
	{ "scenchangedetection alarm", hik_generic },
	{ "fielddetection alarm", hik_generic },
	{ "linedetection alarm", hik_generic },
	{ "regionEntrance alarm", hik_generic },
	{ "regionExit alarm", hik_generic },
	{ "loitering alarm", hik_generic },
	{ "group alarm", hik_generic },
	{ "rapidMove alarm", hik_generic },
	{ "parking alarm", hik_generic },
	{ "unattendedBaggage alarm", hik_generic },
	{ "videoloss alarm", hik_generic },
	{ "vehicledetection alarm", hik_generic },
	{ "audioexception alarm", hik_generic },
	{ "nicBroken alarm", hik_generic },
	{ "ipConflict alarm", hik_generic },
	{ "illAccess alarm", hik_generic },
};

static int alarmserver_in(char *url, webs_t wp, size_t len, char *boundary)
{
	char buf[1024];
	wp->restore_ret = EINVAL;
	int i;
	if (nvram_match("alarmserver", "1")) {
		/*
	 * Look for our part 
	 */
		int v2 = 0;
		while (len > 0) {
			if (!wfgets(buf, MIN(len + 1, sizeof(buf)), wp, NULL))
				return -1;
			len -= strlen(buf);
			/* there are 2 different protocol versions out there */
			if (!strncasecmp(buf, "Content-Disposition:", 20)) {
				v2 = 1;
				break;
			}
			if (!strncasecmp(buf, "<?xml version=\"1.0\"", 18)) {
				break;
			}
		}
/* extract filename */ #
		char *filename = strstr(buf, "name=\"");
		if (filename) {
			filename += 6;
			char *p = strchr(filename, '"');
			if (p)
				*p = 0;
		}

		/*
	 * Skip boundary and headers 
	 */
		if (v2) {
			while (len > 0) {
				if (!wfgets(buf, sizeof(buf), wp, NULL))
					return -1;
				len -= strlen(buf);
				if (!strcmp(buf, "\n") || !strcmp(buf, "\r\n"))
					break;
			}
		}

		unsigned short count;
		char *mem = malloc(len);
		if (!mem) {
			return -1;
		}
		wfread(mem, len, 1, wp);
		mem[len] = 0;
		char s_desc[128];
		char *desc = getXMLTag(mem, "eventDescription", s_desc);

		if (filename) {
			for (i = 0; i < sizeof(dispatch) / sizeof(dispatch[0]); i++)
				if (!strcmp(dispatch[i].filename, filename)) {
					dispatch[i].handler(filename, mem, len);
					goto out;
				}

		} else {
			for (i = 0; i < sizeof(dispatch) / sizeof(dispatch[0]); i++)
				if (!strcmp(dispatch[i].filename, desc)) {
					dispatch[i].handler(desc, mem, len);
					goto out;
				}
			hik_generic(desc, mem, len);
		}
out:;

		debug_free(mem);
	}
	return 0;
}

int do_ej(unsigned char method, struct mime_handler *handler, char *path, webs_t stream);
