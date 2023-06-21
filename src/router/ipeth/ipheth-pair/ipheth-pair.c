/*
 *  Apple iPhone USB Ethernet pairing program
 *
 *  Copyright (c) 2009, 2010, 2011 Daniel Borca  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libimobiledevice/lockdown.h>


int ipeth_main(int argc, char **argv)
{
    const char *myself = argv[0];
    const char *uuid = NULL;
    int list = 0;
    int oldc = 0;

    idevice_error_t rv;
    idevice_t device;
    lockdownd_client_t client;

    while (--argc > 0) {
	const char *p = *++argv;
	if (!strcmp(p, "--help") || !strcmp(p, "-h")) {
	    printf("usage: %s [--list] [--uuid UUID] [--oldc]\n", myself);
	    return 0;
	}
	if (!strcmp(p, "--list") || !strcmp(p, "-l")) {
	    list = !0;
	    break;
	}
	if (!strcmp(p, "--oldc") || !strcmp(p, "-o")) {
	    oldc = !0;
	    break;
	}
	if (!strcmp(p, "--uuid") || !strcmp(p, "-u")) {
	    if (argc < 2) {
		fprintf(stderr, "%s: argument to '%s' is missing\n", myself, p);
		return -1;
	    }
	    argc--;
	    uuid = *++argv;
	    continue;
	}
    }

    if (list) {
	int err = 0;
	int i, count;
	char **devices;

	rv = idevice_get_device_list(&devices, &count);
	if (rv || !count) {
	    fprintf(stderr, "%s: %d: no devices\n", myself, rv);
	    exit(-1);
	}
	for (i = 0; i < count; i++) {
	    char *device_name = NULL;
	    rv = idevice_new(&device, devices[i]);
	    if (rv == 0) {
		rv = lockdownd_client_new(device, &client, "ipheth-pair");
		if (rv == 0) {
		    rv = lockdownd_get_device_name(client, &device_name);
		    lockdownd_client_free(client);
		}
		idevice_free(device);
	    }
	    printf("%s %s\n", devices[i], device_name ? device_name : "N/A");
	    if (device_name)
		free(device_name);
	    if (rv) {
		err = rv;
	    }
	}

	idevice_device_list_free(devices);
	exit (err);
    }

    rv = idevice_new(&device, uuid);
    if (rv) {
	fprintf(stderr, "%s: %d: cannot get %s device\n", myself, rv, uuid ? uuid : "default");
	return -1;
    }
    rv = oldc ? lockdownd_client_new(device, &client, "ipheth-pair"):
		lockdownd_client_new_with_handshake(device, &client, "ipheth-pair"); /* TrustedHostAttached */
    if (rv) {
	fprintf(stderr, "%s: %d: cannot get lockdown\n", myself, rv);
	idevice_free(device);
	return -1;
    }

    /* Send NULL for lockdownd_pair_record and let the library generate it. */
    rv = lockdownd_validate_pair(client, NULL);
    if (rv) {
	const char *verb = "Pair";
	rv = lockdownd_pair(client, NULL);
	if (rv == 0) {
	    verb = "ValidatePair";
	    rv = lockdownd_validate_pair(client, NULL);
	}
	if (rv) {
	    fprintf(stderr, "%s: %d: cannot %s\n", myself, rv, verb);
	    lockdownd_client_free(client);
	    idevice_free(device);
	    return -1;
	}
    }

    /* Is it ok to say Goodbye? */
    lockdownd_client_free(client);
    idevice_free(device);

    return 0;
}

int main(int argc, char *argv[])
{
system("ifconfig iph0 up");
return ipeth_main(argc,argv);
}
