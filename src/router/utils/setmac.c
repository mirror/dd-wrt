/****************************************************************************/

/*
 *	setmac.c --  Set MAC addresses for eth devices from FLASH
 *
 *	(C) Copyright 2004, Greg Ungerer <gerg@snapgear.com>
 */

/****************************************************************************/
#define NEED_PRINTF 1

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>

/****************************************************************************/

/*
 *	Define the maxiumum number of ethernet devices we should try
 *	and configure. Also define the default number we try to configure.
 */
#define	MAXETHS		16
#define	DEFAULTETHS	2

#ifndef ETHPREFIX
#define ETHPREFIX "ixp"
#endif

/*
 *	Define the default flash device to use to get MAC addresses from.
 */
#define	DEFAULTFLASH	"/dev/flash/ethmac"

/****************************************************************************/

/*
 *	Define the table of default MAC addresses. What to use if we can't
 *	find any other good MAC addresses.
 */
unsigned char mactable[MAXETHS * 6] = {
	0x00, 0xd0, 0xcf, 0x00, 0x00, 0x01,
	0x00, 0xd0, 0xcf, 0x00, 0x00, 0x02,
	0x00, 0xd0, 0xcf, 0x00, 0x00, 0x03,
	0x00, 0xd0, 0xcf, 0x00, 0x00, 0x04,
	0x00, 0xd0, 0xcf, 0x00, 0x00, 0x05,
	0x00, 0xd0, 0xcf, 0x00, 0x00, 0x06,
	0x00, 0xd0, 0xcf, 0x00, 0x00, 0x07,
	0x00, 0xd0, 0xcf, 0x00, 0x00, 0x08,
	0x00, 0xd0, 0xcf, 0x00, 0x00, 0x09,
	0x00, 0xd0, 0xcf, 0x00, 0x00, 0x0a,
	0x00, 0xd0, 0xcf, 0x00, 0x00, 0x0b,
	0x00, 0xd0, 0xcf, 0x00, 0x00, 0x0c,
	0x00, 0xd0, 0xcf, 0x00, 0x00, 0x0d,
	0x00, 0xd0, 0xcf, 0x00, 0x00, 0x0e,
	0x00, 0xd0, 0xcf, 0x00, 0x00, 0x0f,
	0x00, 0xd0, 0xcf, 0x00, 0x00, 0x10,
};

int numeths = DEFAULTETHS;

/****************************************************************************/

/*
 *	Search for a mtd partition in /proc/mtd.
 *	Assumes that each line starts with the device name followed
 *	by a ':', and the partition name is enclosed by quotes.
 */
char *findmtddevice(char *mtdname)
{
	FILE *f;
	char buf[80];
	int found;
	static char device[80];
	char *p, *q;

	f = fopen("/proc/mtd", "r");
	if (!f) {
		perror("setmac: open /proc/mtd failed");
		return NULL;
	}

	found = 0;
	while (!found && fgets(buf, sizeof(buf), f)) {
		p = strchr(buf, ':');
		if (!p)
			continue;
		*p++ = '\0';

		p = strchr(p, '"');
		if (!p)
			continue;
		p++;

		q = strchr(p, '"');
		if (!q)
			continue;
		*q = '\0';

		if (strcmp(p, mtdname) == 0) {
			found = 1;
			break;
		}
	}
	fclose(f);
	int num;
	sscanf(buf,"mtd%d",&num);
	if (found) {
		sprintf(device, "/dev/mtdblock/%d", num);
		return device;
	} else {
		fprintf(stderr, "setmac: mtd device '%s' not found\n", mtdname);
		return NULL;
	}
}

/****************************************************************************/

void *memstr(void *m, const char *s, size_t n)
{
	int slen;
	void *end;

	slen = strlen(s);
	if (!slen || slen > n)
		return NULL;

	for (end=m+n-slen; m<=end; m++)
		if (memcmp(m, s, slen)==0)
			return m;

	return NULL;

}

/****************************************************************************/

#define REDBOOTSIZE 4096

void readmacredboot(char *flash, char *redbootconfig)
{
	int fd, i;
	off_t flashsize;
	void *m, *mac;
	char name[32];

	if ((fd = open(flash, O_RDONLY)) < 0) {
		perror("setmac: failed to open MAC flash");
		return;
	}

	m = malloc(REDBOOTSIZE);
	if (!m) {
		fprintf(stderr, "setmac: malloc failed\n");
		close(fd);
		return;
	}

	flashsize = read(fd, m, REDBOOTSIZE);
	if (flashsize < 0) {
		perror("setmac: failed to read MAC flash");
		close(fd);
		free(m);
		return;
	}

	for (i = 0; (i < numeths); i++) {
		snprintf(name, sizeof(name), redbootconfig, i);
		mac = memstr(m, name, flashsize);
		if (!mac) {
			fprintf(stderr, "setmac: redboot config '%s' not found\n",
					name);
			continue;
		}
		mac += strlen(name)+1;
		memcpy(&mactable[i*6], mac, 6);
	}

	free(m);
	close(fd);
}

/****************************************************************************/

void readmacflash(char *flash, off_t macoffset)
{
	int fd, i;
	off_t off;
	unsigned char mac[6];


	/*
	 *	Not that many possible MAC addresses, so lets just
	 *	read them all at once and cache them locally.
	 */
	if ((fd = open(flash, O_RDONLY)) < 0) {
		perror("setmac: failed to read MAC flash");
		return;
	}

	for (i = 0; (i < numeths); i++) {
		off = macoffset + (i * 6);
		if (lseek(fd, off, SEEK_SET) != off) {
			perror("setmac: failed to find eth MACS");
			break;
		}

		if (read(fd, &mac[0], 6) < 0) {
			perror("setmac: failed to read eth MACS");
			break;
		}

		/* Do simple checks for a valid MAC address */
		if ((mac[0] == 0) && (mac[1] == 0) && (mac[2] == 0) &&
		    (mac[3] == 0) && (mac[4] == 0) && (mac[5] == 0))
			continue;
		if ((mac[0] == 0xff) && (mac[1] == 0xff) && (mac[2] == 0xff) &&
		    (mac[3] == 0xff) && (mac[4] == 0xff) && (mac[5] == 0xff))
			continue;

		memcpy(&mactable[i*6], &mac[0], 6);
	}

	close(fd);
}

/****************************************************************************/

void getmac(int port, unsigned char *mac)
{
	memcpy(mac, &mactable[port*6], 6);
}

/****************************************************************************/

void setmac(int port, unsigned char *mac)
{
	int pid, status;
	char eths[32];
	char macs[32];

	sprintf(eths, "%s%d", ETHPREFIX, port);
	sprintf(macs, "%02x:%02x:%02x:%02x:%02x:%02x",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	if ((pid = fork()) < 0) {
		perror("setmac: failed to fork()");
		return;
	}

	if (pid == 0) {
		execlp("ifconfig", "ifconfig", eths, "hw", "ether", macs, NULL);
		exit(1);
	}

	waitpid(pid, &status, 0);

	if (WIFEXITED(status))
		printf("Set %s to MAC address %s\n", eths, macs);
	else
		printf("FAILED to set %s to MAC address %s\n", eths, macs);
}

/****************************************************************************/

void usage(int rc)
{
	printf("usage: setmac [-hs?] [OPTION]...\n"
		"\t-s\n"
		"\t-f <flash-device>\n"
		"\t-m <mtd-name>\n"
		"\t-n <num-eth-interfaces>\n"
		"\t-o <offset>\n"
		"\t-r <redboot-config-name>\n");
	exit(rc);
}

/****************************************************************************/

int main(int argc, char *argv[])
{
	int i, p, c;
	unsigned char mac[6];
	char *flash = DEFAULTFLASH;
	char *mtdname = NULL;
	off_t macoffset = 0x24000;
	char *redboot = NULL;
	int swapmacs = 0;
	int offset = 0;

	while ((c = getopt(argc, argv, "h?sm:n:o:i:r:f:")) > 0) {
		switch (c) {
		case '?':
		case 'h':
			usage(0);
		case 's':
			swapmacs++;
			break;
		case 'f':
			flash = optarg;
			break;
		case 'm':
			mtdname = optarg;
			break;
		case 'n':
			numeths = atoi(optarg);
			if ((numeths < 0) || (numeths > MAXETHS)) {
				printf("ERROR: bad number of ethernets?\n");
				exit(1);
			}
			break;
		case 'o':
			macoffset = strtoul(optarg, NULL, 0);
			break;
		case 'r':
			redboot = optarg;
			break;
		case 'i':
			offset = strtoul(optarg, NULL, 0);
			break;
		default:
			usage(1);
		}
	}

	if (mtdname)
		flash = findmtddevice(mtdname);

	if (flash) {
		if (redboot)
			readmacredboot(flash, redboot);
		else
			readmacflash(flash, macoffset);
	}

	getmac(0, &mac[0]);
	setmac(offset, &mac[0]);

	return 0;
}

/****************************************************************************/
