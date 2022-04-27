/*
 * Schmutztool um Karten EEPROM Daten zu verschluesseln, Dumpen und zu Recovern, sowie locken
 * syntax: eepromtool [command] [param] 
 * 
 * commands:
 * 		dump dumpfile.card - saves eeprom content to file
 *		recover dumpfile.card - recovers the eeprom data from a file
 *		crypt dumpfile.crypt - encrypts calibration eeprom and writes the encrypted data to file (just for debugging)
 *		lock - locks the eeprom content for any future write access. do only use this command if you know what you're doing. the card will be write protected on next reboot
 * 
 * (c) 2008 - Sebastian Gottschall / Stac-Wireless Ltd.
 */

#include <string.h>
#include <stdlib.h>
#include <err.h>

#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <stdio.h>

#include "if_athioctl.h"

#ifndef ATH_DEFAULT
#define	ATH_DEFAULT	"wifi0"
#endif

#define HAL_DIAG_EEREAD	17	/* Read EEPROM word */
#define HAL_DIAG_EEWRITE 18	/* Write EEPROM word */

struct ath_diag atd;

#define ARC4_MAX_BYTES	0x40000000

typedef struct {
	u_int16_t ee_off;	/* eeprom offset */
	u_int16_t ee_data;	/* write data */
} HAL_DIAG_EEVAL;

typedef struct {
	unsigned char state[256];
	unsigned int byteCount;
	unsigned char x;
	unsigned char y;
} rc4_key;

static void Arc4Init(rc4_key * arc4, unsigned char *key, int keylen)
{
	unsigned char index1, index2, tmp, *state;
	short counter;

	arc4->byteCount = 0;
	state = &arc4->state[0];

	for (counter = 0; counter < 256; counter++) {
		state[counter] = (unsigned char)counter;
	}
	arc4->x = 0;
	arc4->y = 0;
	index1 = 0;
	index2 = 0;

	for (counter = 0; counter < 256; counter++) {
		index2 = (key[index1] + state[counter] + index2) & 0xff;

		tmp = state[counter];
		state[counter] = state[index2];
		state[index2] = tmp;

		index1 = (index1 + 1) % keylen;
	}
}

static int Arc4(rc4_key * arc4, unsigned char *in, unsigned char *out, int len)
{
	unsigned char x, y, *state, xorIndex, tmp;
	int counter;
	arc4->byteCount += len;
	if (arc4->byteCount > ARC4_MAX_BYTES) {
		return -1;
	}

	x = arc4->x;
	y = arc4->y;
	state = &arc4->state[0];
	for (counter = 0; counter < len; counter++) {
		x = (x + 1) & 0xff;
		y = (state[x] + y) & 0xff;

		tmp = state[x];
		state[x] = state[y];
		state[y] = tmp;

		xorIndex = (state[x] + state[y]) & 0xff;

		tmp = in[counter];
		tmp ^= state[xorIndex];
		out[counter] = tmp;
	}
	arc4->x = x;
	arc4->y = y;
	return len;
}

static rc4_key uprc4;

void Cinit(void)
{
	Arc4Init(&uprc4, "WRasdf44a!!!dkJP", strlen("WRasdf44a!!!dkJP"));
}

int main(int argc, char *argv[])
{

	struct ifreq ifr;
	int s;
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		err(1, "socket");
	if (argc > 1 && strcmp(argv[1], "-i") == 0) {
		if (argc < 2) {
			fprintf(stderr, "%s: missing interface name for -i\n", argv[0]);
			exit(-1);
		}
		strncpy(atd.ad_name, argv[2], sizeof(atd.ad_name));
		argc -= 2, argv += 2;
	} else
		strncpy(atd.ad_name, ATH_DEFAULT, sizeof(atd.ad_name));
	strncpy(ifr.ifr_name, atd.ad_name, sizeof(ifr.ifr_name));

	atd.ad_id = HAL_DIAG_EEREAD | ATH_DIAG_DYN | ATH_DIAG_IN;
	unsigned short res;
	unsigned short i;
	if (!strcmp(argv[1], "dump")) {
		FILE *fp = fopen(argv[2], "wb");
		for (i = 0; i < 0x400; i++) {
			atd.ad_in_data = &i;
			atd.ad_in_size = sizeof(unsigned short);
			atd.ad_out_data = (caddr_t) & res;
			atd.ad_out_size = sizeof(unsigned short);
			if (ioctl(s, SIOCGATHDIAG, &atd) < 0) {
				fprintf(stderr, "error while reading eeprom on %X\n", i);
				return;
			}
//fprintf(stderr,"dumping %X=%X\r",i,res);
			fwrite(&res, 2, 1, fp);
		}
		fprintf(stderr, "\n\ndone              \n");
		fclose(fp);
	}
	if (!strcmp(argv[1], "crypt")) {
		unsigned short buffer[0x400 - 0xc0];
		fprintf(stderr, "reading\n");
		unsigned short protect;
		i = 0x3f;
		atd.ad_in_data = &i;
		atd.ad_in_size = sizeof(unsigned short);
		atd.ad_out_data = (caddr_t) & protect;
		atd.ad_out_size = sizeof(unsigned short);
		ioctl(s, SIOCGATHDIAG, &atd);
		fprintf(stderr, "protect bit %X\n", protect);
/*#define	AR_EEPROM_PROTECT_RP_0_31	0x0001
#define	AR_EEPROM_PROTECT_WP_0_31	0x0002
#define	AR_EEPROM_PROTECT_RP_32_63	0x0004
#define	AR_EEPROM_PROTECT_WP_32_63	0x0008
#define	AR_EEPROM_PROTECT_RP_64_127	0x0010
#define	AR_EEPROM_PROTECT_WP_64_127	0x0020
#define	AR_EEPROM_PROTECT_RP_128_191	0x0040
#define	AR_EEPROM_PROTECT_WP_128_191	0x0080
#define	AR_EEPROM_PROTECT_RP_192_207	0x0100
#define	AR_EEPROM_PROTECT_WP_192_207	0x0200
#define	AR_EEPROM_PROTECT_RP_208_223	0x0400
#define	AR_EEPROM_PROTECT_WP_208_223	0x0800
#define	AR_EEPROM_PROTECT_RP_224_239	0x1000
#define	AR_EEPROM_PROTECT_WP_224_239	0x2000
#define	AR_EEPROM_PROTECT_RP_240_255	0x4000
#define	AR_EEPROM_PROTECT_WP_240_255	0x8000
*/
		for (i = 1; i < 17; i++) {
			if (protect & 1 << i)
				fprintf(stderr, "protect bits %X\n", 1 << i);
		}
		for (i = 0xc0; i < 0x400; i++) {
			atd.ad_in_data = &i;
			atd.ad_in_size = sizeof(unsigned short);
			atd.ad_out_data = (caddr_t) & res;
			atd.ad_out_size = sizeof(unsigned short);
			if (ioctl(s, SIOCGATHDIAG, &atd) < 0) {
				fprintf(stderr, "error while reading eeprom on %X\n", i);
				return;
			}
			buffer[i - 0xc0] = res;
		}

		fprintf(stderr, "encrypting\n");
		Cinit();
		unsigned short out[0x400 - 0xc0];
		Arc4(&uprc4, buffer, out, (0x400 - 0xc0) * 2);
		fprintf(stderr, "writing\n");
		atd.ad_id = HAL_DIAG_EEWRITE | ATH_DIAG_IN;
		HAL_DIAG_EEVAL ee;

		ee.ee_off = 0x3f;
		ee.ee_data = 0;
		atd.ad_in_data = &ee;
		atd.ad_in_size = sizeof(HAL_DIAG_EEVAL);
		if (ioctl(s, SIOCGATHDIAG, &atd) < 0) {
			fprintf(stderr, "error while writing eeprom on %X (protect bit)\n", 0x3f);
		}
		FILE *fp = fopen(argv[2], "wb");
		for (i = 0xc0; i < 0x400; i++) {
			ee.ee_off = i;
			ee.ee_data = out[i - 0xc0];
			atd.ad_in_data = &ee;
			atd.ad_in_size = sizeof(HAL_DIAG_EEVAL);
			if (ioctl(s, SIOCGATHDIAG, &atd) < 0) {
				fprintf(stderr, "error while writing eeprom on %X\n", i);
				return;
			}
			fwrite(&out[i - 0xc0], 2, 1, fp);
		}
		fclose(fp);
		fprintf(stderr, "\n\ndone              \n");
	}

	if (!strcmp(argv[1], "recover")) {
		fprintf(stderr, "writing\n");
		atd.ad_id = HAL_DIAG_EEWRITE | ATH_DIAG_IN;
		HAL_DIAG_EEVAL ee;
		FILE *fp = fopen(argv[2], "rb");

		for (i = 0; i < 0x400; i++) {
			unsigned short res;
			fread(&res, 2, 1, fp);
			ee.ee_off = i;
			ee.ee_data = res;
			atd.ad_in_data = &ee;
			atd.ad_in_size = sizeof(HAL_DIAG_EEVAL);
			if (ioctl(s, SIOCGATHDIAG, &atd) < 0) {
				fprintf(stderr, "error while writing eeprom on %X\n", i);
				return;
			}
		}
		fclose(fp);
		fprintf(stderr, "\n\ndone              \n");
	}

	if (!strcmp(argv[1], "lock")) {
		fprintf(stderr, "locking (warning, unrecoverable write protection delay it for 10 sec)\n");
		sleep(10);

		atd.ad_id = HAL_DIAG_EEWRITE | ATH_DIAG_IN;
		HAL_DIAG_EEVAL ee;
		ee.ee_off = 0x3f;
		ee.ee_data = 0x0002 | 0x0008 | 0x0020 | 0x0080 | 0x0200 | 0x0800 | 0x2000 | 0x8000;
		fprintf(stderr, "locking (protection value %X)\n", ee.ee_data);
		atd.ad_in_data = &ee;
		atd.ad_in_size = sizeof(HAL_DIAG_EEVAL);
		if (ioctl(s, SIOCGATHDIAG, &atd) < 0) {
			fprintf(stderr, "error while writing eeprom on %X\n", i);
			return;
		}
		fprintf(stderr, "\n\ndone              \n");
	}

}
