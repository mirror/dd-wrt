/* testdevinfo - Display device info findings for debugging

   Copyright (C) 2015 Andreas Bombe <aeb@debian.org>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "device_info.h"


int main(int argc, char **argv)
{
    struct device_info info;
    int fd;

    if (argc != 2) {
	printf("Usage: testdevinfo FILENAME\n");
	return 1;
    }

    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
	perror("open device");
	return 1;
    }

    device_info_verbose = 100;
    get_device_info(fd, &info);
    close(fd);

    printf("\nfound information:\n");

    printf("device type: ");
    switch (info.type) {
	case TYPE_UNKNOWN:
	    printf("unknown\n");
	    break;

	case TYPE_BAD:
	    printf("unusable\n");
	    break;

	case TYPE_FILE:
	    printf("image file\n");
	    break;

	case TYPE_VIRTUAL:
	    printf("virtual\n");
	    break;

	case TYPE_REMOVABLE:
	    printf("removable\n");
	    break;

	case TYPE_FIXED:
	    printf("fixed\n");
	    break;

	default:
	    printf("internal error! invalid value\n");
	    break;
    }

    printf("is partition: ");
    if (info.partition < 0)
	printf("unknown\n");
    else if (info.partition == 0)
	printf("no, full disk\n");
    else
	printf("number %d\n", info.partition);

    printf("has children: ");
    if (info.has_children < 0)
	printf("unknown\n");
    else if (info.has_children == 0)
	printf("no\n");
    else
	printf("yes\n");

    printf("heads: ");
    if (info.geom_heads < 0)
	printf("unknown\n");
    else
	printf("%d\n", info.geom_heads);

    printf("sectors: ");
    if (info.geom_sectors < 0)
	printf("unknown\n");
    else
	printf("%d\n", info.geom_sectors);

    printf("start: ");
    if (info.geom_start < 0)
	printf("unknown\n");
    else
	printf("%ld\n", info.geom_start);

    printf("sector size: ");
    if (info.sector_size < 0)
	printf("unknown\n");
    else
	printf("%d\n", info.sector_size);

    printf("size: ");
    if (info.size < 0)
	printf("unknown\n");
    else
	printf("%lld\n", info.size);

    return 0;
}
