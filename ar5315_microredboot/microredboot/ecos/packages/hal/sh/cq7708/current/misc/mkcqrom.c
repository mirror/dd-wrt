/* Split ROM data Program for CqREEK SH-3 (Thanks Yamanaka-san) */
/* Nihon Cygnus Solutions  KASHIWAYA Haruki  '00-4-13 */

/* This program separate ROM data for one ROM socket.
   The cq board has two ROM sockets, and those bus size can
   switch to 8bit or 16bit with JP6.
   If your program is smaller than 32kByte, you can use
   a 256k bit ROM with 8bit bus mode.
   However, the board curcuit connection is peculiar.
   A0 is connected to A14 in 8bit bus mode (See the curcuit diagram),
   this mean's all odd data are assigned from 0x4000 offset.
   You must use this program and separate ROM data when you use
   a 256k bit ROM with 8bit bus mode.

   If your program is bigger than 32kByte, simply, you can use
   two 256k bit(or bigger) ROM with 16bit bus mode.
   You do *not* need to use this program.
   (Details, separate ROM data to odd and even, and write each data to
   each ROMs, and attach odd's ROM to IC4, even's ROM to IC8.)

*/

#include <stdio.h>
#include <errno.h>

#define ROMSIZE (32 * 1024)

main(int argc, char **argv)
{
	FILE *fpr, *fpw;
	int i, c;
	char *s, *p;

	if(argc != 3) {
		printf("Usage: mkcqrom <input-filename> <output-filename>\n Please see comments in mkcqrom.c.\n");
		exit(1);
	}

	p = (char *)malloc(ROMSIZE);
	if(p == NULL) {
		printf("malloc error\n");
		exit(1);
	}
	fpr = fopen(*++argv, "rb");
	if(fpr == NULL) {
		printf("open error %s %d\n", *argv, errno);
		exit(1);
	}
	memset(p, 0xff, ROMSIZE);

    /* split even data to 0x4000 offset (A0 is assigned to A14) */
	i = 0;
	while(1) {
		c = getc(fpr);
		if(c == EOF) break;
		s = p + (i / 2) + (i % 2) * 0x4000;
		*s = (char)c;
		i++;
	}
	fclose(fpr);

	fpw = fopen(*++argv, "wb");
	if(fpw == NULL) {
		printf("open error %s %d\n", *argv, errno);
		exit(1);
	}
	fwrite(p, ROMSIZE, sizeof(char), fpw);
	fclose(fpw);
}
