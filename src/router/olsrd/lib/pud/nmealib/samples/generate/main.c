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
#include <unistd.h>

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
    nmeaINFO info;
    char buff[2048];
    int gen_sz;
    int it;

    nmea_zero_INFO(&info);

    info.sig = 3;
    info.fix = 3;
    info.lat = 5000.0;
    info.lon = 3600.0;
    info.speed = 2.14 * NMEA_TUS_MS;
    info.elv = 10.86;

    info.satinfo.inuse = 1;
    info.satinfo.inview = 1;

    /*
    info.satinfo.sat[0].id = 1;
    info.satinfo.sat[0].in_use = 1;
    info.satinfo.sat[0].elv = 50;
    info.satinfo.sat[0].azimuth = 0;
    info.satinfo.sat[0].sig = 99;
    */

    for(it = 0; it < 10; ++it)
    {
        gen_sz = nmea_generate(
            &buff[0], 2048, &info,
            GPGGA | GPGSA | GPGSV | GPRMC | GPVTG
            );

        buff[gen_sz] = 0;
        printf("%s\n", &buff[0]);

        usleep(500000);

        info.speed += .1;
    }

    return 0;
}
