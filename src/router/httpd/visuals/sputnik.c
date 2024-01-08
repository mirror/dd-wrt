/*
 * sputnik.c
 *
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#define VISUALSOURCE 1
/*
 * $Id:$
 *
 * Copyright (c) 2005 Sputnik, Inc.
 * All rights reserved
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include <broadcom.h>

EJ_VISIBLE void ej_sputnik_apd_status(webs_t wp, int argc, char_t **argv)
{
	char *key = argv[0];
	FILE *fh;

	if ((fh = fopen("/var/run/apd.status", "r"))) {
		char s[128];

		/*
		 * The first line is always the PPID.  If it doesn't exist, apd
		 *   isn't running.
		 */
		if (fgets(s, sizeof(s), fh)) {
			int apd_pid = (pid_t)atol(s);

			if (!kill(apd_pid, 0)) {
				if (!strcmp(key, "pid")) {
					websWrite(wp, "%d", apd_pid);
				} else {
					/*
					 * We're good, read the rest.
					 */
					while (fgets(s, sizeof(s), fh)) {
						int len = strlen(s);
						char *eqloc;

						if (len > 0 &&
						    s[len - 1] == '\n') {
							s[len - 1] = '\0';
						}

						if ((eqloc = index(s, '='))) {
							char *v;

							*eqloc = '\0';
							v = eqloc + 1;

							if (!strcmp(key, s)) {
								websWrite(wp,
									  "%s",
									  v);
								break;
							}
						}
					} /* End while */
				}
			}
		}

		fclose(fh);
	}

	return;
} /* End ej_sputnik_apd_status() */
