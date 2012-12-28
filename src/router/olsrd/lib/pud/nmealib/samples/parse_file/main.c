/*
 * This file is part of nmealib.
 *
 * Copyright (c) 2008 Timur Sinitsyn
 * Copyright (c) 2011 Ferry Huberts
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <nmea/info.h>
#include <nmea/context.h>
#include <nmea/parser.h>
#include <nmea/gmath.h>

#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>

static const char * traceStr = "Trace: ";
static const char * errorStr = "Error: ";
static const char * eol = "\n";

static void trace(const char *str, int str_size) {
	write(1, traceStr, strlen(traceStr));
	write(1, str, str_size);
	write(1, eol, strlen(eol));
}
static void error(const char *str, int str_size) {
	write(1, errorStr, strlen(errorStr));
	write(1, str, str_size);
	write(1, eol, strlen(eol));
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
	char fn[2048];
	const char * filename = &fn[0];
	const char * deffile;
	const char * dn;
	nmeaINFO info;
	nmeaPARSER parser;
	FILE *file;
	char buff[2048];
	int size, it = 0;
	nmeaPOS dpos;

	if (argc <= 1) {
		dn = dirname(argv[0]);
		deffile="/../../samples/parse_file/gpslog.txt";
	} else {
		dn="";
		deffile = argv[1];
	}
	snprintf(&fn[0], sizeof(fn),"%s%s", dn, deffile);
	printf("Using file %s\n", filename);

	file = fopen(filename, "rb");

	if (!file) {
		printf("Could not open file %s\n", filename);
		return -1;
	}

	nmea_context_set_trace_func(&trace);
	nmea_context_set_error_func(&error);

	nmea_zero_INFO(&info);
	nmea_parser_init(&parser);

	while (!feof(file)) {
		size = (int) fread(&buff[0], 1, 100, file);

		nmea_parse(&parser, &buff[0], size, &info);
		nmea_info2pos(&info, &dpos);

		printf("*** %03d, Lat: %f, Lon: %f, Sig: %d, Fix: %d\n", it++, dpos.lat, dpos.lon, info.sig, info.fix);
	}

	fseek(file, 0, SEEK_SET);

	/*
	 }
	 */

	nmea_parser_destroy(&parser);
	fclose(file);

	return 0;
}
