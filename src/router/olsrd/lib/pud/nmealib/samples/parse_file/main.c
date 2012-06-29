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

#include <nmea/nmea.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>


static void trace(const char *str, int str_size)
{
    printf("Trace: ");
    write(1, str, str_size);
    printf("\n");
}
static void error(const char *str, int str_size)
{
    printf("Error: ");
    write(1, str, str_size);
    printf("\n");
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
    static const char * filename = "../../samples/parse_file/gpslog.txt";
    nmeaINFO info;
    nmeaPARSER parser;
    FILE *file;
    char buff[2048];
    int size, it = 0;
    nmeaPOS dpos;

    file = fopen(filename, "rb");

    if(!file) {
        printf("Could not open file %s\n", filename);
        return -1;
    }

    nmea_property()->trace_func = &trace;
    nmea_property()->error_func = &error;

    nmea_zero_INFO(&info);
    nmea_parser_init(&parser);

    /*
    while(1)
    {
    */

    while(!feof(file))
    {
        size = (int)fread(&buff[0], 1, 100, file);

        nmea_parse(&parser, &buff[0], size, &info);

        nmea_info2pos(&info, &dpos);

        printf(
            "%03d, Lat: %f, Lon: %f, Sig: %d, Fix: %d\n",
            it++, dpos.lat, dpos.lon, info.sig, info.fix
            );
    }

    fseek(file, 0, SEEK_SET);

    /*
    }
    */

    nmea_parser_destroy(&parser);
    fclose(file);

    return 0;
}
