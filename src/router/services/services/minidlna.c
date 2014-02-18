/*
 * minidlna.c
 *
 * Copyright (C) 2012 dd-wrt
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
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
#ifdef HAVE_MINIDLNA
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <signal.h>
#include <utils.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <services.h>
#include <dlna.h>

void start_dlna(void)
{
	struct dlna_share *dlna_shares, *cs, *csnext;
	if (!nvram_match("dlna_enable", "1"))
		return;
	mkdir("/tmp/db", 0700);
	FILE *fp = fopen("/tmp/minidlna.conf", "wb");
#ifndef HAVE_VENTANA
	if (nvram_match("jffs_mounted", "1"))
	{
#endif
		mkdir("/jffs/minidlna", 0700);
		if (nvram_match("dlna_cleandb", "1")){
			eval("rm", "-f", "/jffs/minidlna/files.db");
			nvram_set("dlna_cleandb", "0");
		}
		fprintf(fp, "db_dir=/jffs/minidlna\n");
#ifndef HAVE_VENTANA
	} else {
		mkdir("/tmp/db", 0700);
		eval("rm", "-f", "/tmp/db/files.db");
	}
#endif
	fprintf(fp, "port=8200\n");
	fprintf(fp, "network_interface=br0\n");
	dlna_shares = getdlnashares();
	for (cs = dlna_shares; cs; cs = csnext) {
		if (strlen(cs->mp)) {
			if (cs->types & TYPE_VIDEO)
				fprintf(fp, "media_dir=V,%s\n", cs->mp);	// comma separted list
			if (cs->types & TYPE_AUDIO)
				fprintf(fp, "media_dir=A,%s\n", cs->mp);	// comma separted list
			if (cs->types & TYPE_IMAGES)
				fprintf(fp, "media_dir=P,%s\n", cs->mp);	// comma separted list
		}
		csnext = cs->next;
		free(cs);
	}
	fprintf(fp, "friendly_name=%s:DLNA\n", nvram_safe_get("DD_BOARD"));	//enter any name you want here, but should be unique within a network
	if (nvram_match("dlna_thumb", "1")) {
		fprintf(fp, "album_art_names=Cover.jpg/cover.jpg/AlbumArtSmall.jpg/albumartsmall.jpg/AlbumArt.jpg/albumart.jpg/Album.jpg/album.jpg/Folder.jpg/folder.jpg/Thumb.jpg/thumb.jpg\n");
	}
	fprintf(fp, "inotify=yes\n");
	fprintf(fp, "enable_tivo=no\n");
	fprintf(fp, "strict_dlna=no\n");
	fprintf(fp, "notify_interval=300\n");
	fprintf(fp, "serial=12345678\nmodel_number=1\n");
	fclose(fp);
	eval("minidlna", "-f", "/tmp/minidlna.conf");
	syslog(LOG_INFO, "minidlna : DLNA Media Server successfully started\n");

	return;
}

void stop_dlna(void)
{
	stop_process("minidlna", "DLNA Media Server");
}
#endif
