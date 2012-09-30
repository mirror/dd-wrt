#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include "i2c.h"
#include "i2c_upgrader.h"

#define GSP_UPDATER_REV 5

#define GSP_DEVICE		0x20
#define GSP_UPDATER		0x21
#define GSP_PASSWORD 	0x58
#define GSP_UNLOCK 		0x01
#define GSP_PROGRAM 	0x02
#define GSP_ERASE 		0x04
#define GSP_PUC 			0x06

#define GSP2_PUC			0x1
#define GSP2_ADDR			0x2
#define GSP2_ERASE		0x3
#define GSP2_WORD			0x4
#define GSP2_PROG			0x5

static bool parse_data_file(char *filename, unsigned char data[16][16384], short *address, short *length);

static void print_banner(void)
{
	printf("Gateworks GSP Updater Rev: %i\n", GSP_UPDATER_REV);
	printf("Copyright (C) 2004-2010, Gateworks Corporation, All Rights Reserved\n");
	printf("Built %s, %s\n", __TIME__, __DATE__);
}

static void print_help(void)
{
	print_banner();
	printf("\nUsage:\n");
	printf("\t-f <filename>\tProgram flash device using [filename]\n");
}


int main(int argc, char **argv)
{
	unsigned long funcs;
	unsigned char data[16][16384] = {0};
	unsigned short address[16];
	unsigned short length[16];
	char *prog_filename=NULL;
	unsigned char buffer[16];
	signed char ret;

	int i, j, k, opt;
	int file;

	print_banner();

	while ((opt = getopt(argc, argv, "hf:")) != -1) {
		switch (opt) {
               case 'h':
					print_help();
					exit(-1);
					break;
				case 'f':
					prog_filename=optarg;
					break;
               default: /* '?' */
					print_help();
					exit(-1);
		}
	}

	if (!prog_filename) {
		print_help();
		exit(-1);
	}

	file = open("/dev/i2c-0", O_RDWR);
	if (file < 0 && errno == ENOENT) {
		file = open("/dev/i2c/0", O_RDWR);
	}

	if (file < 0) {
		printf("unable to open i2c device\n");
		exit(1);
	}

	if (ioctl(file, I2C_FUNCS, &funcs) < 0) {
		printf("i2c not functional\n");
		exit(1);
	}

	if (ioctl(file, I2C_SLAVE, GSP_DEVICE) < 0) {
		printf("couldn't set gsp address\n");
		exit(1);
	}

	ret = i2c_smbus_read_byte_data(file, 14);
	printf("Current GSP Firmware Rev: %i\n", ret & 0xff);
	if ((ret & 0xff) == 32)
	    {
	    printf("no update required\n");
	    exit(0);
	    }

	if (ioctl(file, I2C_SLAVE, GSP_UPDATER) < 0) {
		printf("couldn't set gsp_updater address\n");
		exit(1);
	}

	i2c_smbus_write_byte_data(file, 0, GSP_PASSWORD | GSP_UNLOCK);
	ret = i2c_smbus_read_byte_data(file, 0);

	if (!parse_data_file(prog_filename, data, address, length)) {
	    printf("problem with the file\n");
		exit -1;
		}

	/* ##### Stage 1 Upgrader ##### */
	// Erase all of main flash
	for (i = ((address[0] & 0xf000) | 0x400); i < 0xffff; i += 0x200) {
		buffer[0] = i & 0xff;
		buffer[1] = (i >> 8) & 0xff;
		i2c_smbus_write_i2c_block_data(file, 1, 2, buffer);
		i2c_smbus_write_byte_data(file, 0, GSP_PASSWORD | GSP_ERASE);
		while (1) {
			ret = i2c_smbus_read_byte_data(file, 2);
			if (ret != -1)
				break;
			fflush(stdout);
		}
		ret = i2c_smbus_read_byte_data(file, 1);
	}

	// Switch to Programing mode
	i2c_smbus_write_byte_data(file, 0, GSP_PASSWORD | GSP_PROGRAM);

	for (i = 1; i>= 0; i--) {
		if (i2c_upgrader_length[i]) {
			for (j = 0; j < i2c_upgrader_length[i]; j+=2) {
				// Set starting Address
				buffer[0] = (i2c_upgrader_address[i] + j) & 0xff;
				buffer[1] = ((i2c_upgrader_address[i] + j) >> 8) & 0xff;
				i2c_smbus_write_i2c_block_data(file, 1, 2, buffer);
				buffer[0] = i2c_upgrader_data[i][j];
				buffer[1] = i2c_upgrader_data[i][j+1];

				i2c_smbus_write_i2c_block_data(file, 3, 2, buffer);
				while (1) {
					ret = i2c_smbus_read_byte_data(file, 2);
					if (ret != -1)
						break;
					fflush(stdout);
				}
				if (!i) {
					printf("Program Upgrader  %2i%%\r", (int)((((double)j / 2) / i2c_upgrader_length[i]) * 100));
					fflush(stdout);
				}
			}
			if (!i) {
				printf("Program Upgrader  %i%%\n", 100);
				fflush(stdout);
			}
		}
	}
	// Turn off Programing Mode
	i2c_smbus_write_byte_data(file, 0, GSP_PASSWORD | GSP_UNLOCK);
	// Reset GSP
	i2c_smbus_write_byte_data(file, 0, GSP_PASSWORD | GSP_PUC);


	/* ##### Stage 2 Upgrader ##### */

	while (1) {
		ret = i2c_smbus_read_byte(file);
		if (ret != -1)
			break;
	}

	// Erase all of main flash
	for (i = (address[0] & 0xf000); i < 0xffff; i += 0x200) {
		buffer[0] = i & 0xff;
		buffer[1] = (i >> 8) & 0xff;
		i2c_smbus_write_i2c_block_data(file, GSP2_ADDR, 2, buffer);
		i2c_smbus_write_byte(file, GSP2_ERASE);
		while (1) {
			ret = i2c_smbus_read_byte(file);
			if (ret != -1)
				break;
		}
	}

	for (i = 15; i>= 0; i--) {
		if (length[i]) {
			for (j = 0; j < length[i]; j+=2) {
				// Set starting Address
				buffer[0] = (address[i] + j) & 0xff;
				buffer[1] = ((address[i] + j) >> 8) & 0xff;
				i2c_smbus_write_i2c_block_data(file, GSP2_ADDR, 2, buffer);
				buffer[0] = data[i][j];
				buffer[1] = data[i][j+1];

				i2c_smbus_write_i2c_block_data(file, GSP2_WORD, 2, buffer);
				i2c_smbus_write_byte(file, GSP2_PROG);
				while (1) {
					ret = i2c_smbus_read_byte(file);
					if (ret != -1)
						break;
					fflush(stdout);
				}
				printf("MSP Prg B%i   %2i%%\r", i, (int)((((double)j / 2) / length[i]) * 100));
				fflush(stdout);
			}
			printf("MSP Prg B%i  %i%%\n", i, 100);
			fflush(stdout);
		}
	}
	i2c_smbus_write_byte(file, GSP2_PUC);

	exit(0);
}

static bool parse_data_file(char *filename, unsigned char data[16][16384], short *address, short *length)
{
	FILE *fd;
	char line[1024];
	char temp[64];
	short address_loc = -1;
	char t[16][4];
	short temp_word = 0;
	int i = 0, j = 0, num_scan = 0;

	memset(t, 0, 16*4);
	memset(length, 0, 16*2);

	fd = fopen(filename, "r");
	if (!fd) return false;
	while (fgets(line, 1024, fd)) {
		if (strncmp(line, "@", 1) == 0) {
			j = 0;
			address_loc++;
			memcpy(temp, &line[1], 5);
			address[address_loc] = strtol(temp, 0, 16);
		} else if (strncmp(line, "q", 1) != 0) {
			num_scan = sscanf(line, "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
				t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7], t[8], t[9], t[10], t[11], t[12], t[13], t[14], t[15]);
			for (i = 0; i < num_scan; i++) {
				temp_word = strtol(t[i], 0, 16);
				data[address_loc][j++] = temp_word;
				length[address_loc]++;
			}
		}
	}
return true;
}
