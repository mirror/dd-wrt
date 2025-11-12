/********************************************************************\
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652       *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                   *
 *                                                                  *
\********************************************************************/

/** @file commandline.c
    @brief Command line argument handling
    @author Copyright (C) 2004 Philippe April <papril777@yahoo.com>
    @author Copyright (C) 2015-2025 Modifications and additions by BlueWave Projects and Services <opennds@blue-wave.net>
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <sys/stat.h>

#include "debug.h"
#include "safe.h"
#include "conf.h"
#include "util.h"

static void usage(void);

/** @internal
 * @brief Print usage
 *
 * Prints usage, called when opennds is run with -h or with an unknown option
 */
static void
usage(void)
{
	printf("Usage: opennds [options]\n"
		"\n"
		"  -f          Run in foreground\n"
		"  -b          Run in background\n"
		"  -s          Log to syslog\n"
		"  -h          Print this help\n"
		"  -v          Print version\n"
		"\n"
	);
}

/** Parse the command line and set configuration values
 */
void parse_commandline(int argc, char **argv)
{
	int ret;
	int i = 1;

	s_config *config = config_get_config();

	if (argc <= i) {
		usage();
		exit(1);

	} else if (strcmp(argv[i], "-f") == 0) {

		ret = check_heartbeat();

		if (ret == 0) {
			printf("openNDS is already running, status [ %d ]. Retry later... \n", 1);
			exit(1);
		}

		config->daemon = 0;

	} else if (strcmp(argv[i], "-b") == 0) {

		ret = check_heartbeat();

		if (ret == 0) {
			printf("openNDS is already running, status [ %d ]. Retry later... \n", 1);
			exit(1);
		}

		config->daemon = -1;

	} else	if (strcmp(argv[i], "-h") == 0) {
		usage();
		exit(1);

	} else if (strcmp(argv[i], "-v") == 0) {
		printf("This is openNDS version " VERSION "\n");
		exit(1);

	} else {
		usage();
		exit(1);
	}
}
