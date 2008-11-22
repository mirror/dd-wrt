/* Decoder for password encoding of Cisco VPN client.
   Copyright (C) 2005 Maurice Massar
   
   Thanks to HAL-9000@evilscientists.de for decoding and posting the algorithm!

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "decrypt-utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <gcrypt.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	int i, len, ret = 0;
	char *bin, *pw = NULL;
	
	gcry_check_version(NULL);

	if (argc == 1 || *argv[1] == '-') {
		fprintf(stderr,
			"\nUsage: %s DEADBEEF...012345678 424242...7261\n"
			"    Print decoded result to stdout\n\n",
			argv[0]);
		exit(1);
	}
	/* Hack for use in pcf2vpnc */
	if (*argv[1] == 'q') {
		exit(1);
	}
	
	for (i = 1; i < argc; i++) {
		ret = hex2bin(argv[i], &bin, &len);
		if (ret != 0) {
			perror("decoding input");
			continue;
		}
		ret = deobfuscate(bin, len, (const char **)&pw, NULL);
		free(bin);
		if (ret != 0) {
			perror("decrypting input");
			continue;
		}
		printf("%s\n", pw);
		free(pw);
	}
	
	exit(ret != 0);
}
