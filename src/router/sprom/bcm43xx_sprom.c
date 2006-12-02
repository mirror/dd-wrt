/*

  Broadcom BCM43xx SPROM data modification tool

  Copyright (c) 2006 Michael Buesch <mbuesch@freenet.de>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
  Boston, MA 02110-1301, USA.

*/

#include "bcm43xx_sprom.h"
#include "utils.h"

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>


struct cmdline_args cmdargs;


static int hexdump_sprom(const uint8_t *sprom, char *buffer, size_t bsize)
{
	int i, pos = 0;

	for (i = 0; i < SPROM_SIZE; i++) {
		pos += snprintf(buffer + pos, bsize - pos - 1,
				"%02X", sprom[i] & 0xFF);
	}

	return pos + 1;
}

static uint8_t sprom_crc(const uint8_t *sprom)
{
	int i;
	uint8_t crc = 0xFF;

	for (i = 0; i < SPROM_SIZE - 1; i++)
		crc = crc8(crc, sprom[i]);
	crc ^= 0xFF;

	return crc;
}

static int write_output_binary(int fd, const uint8_t *sprom)
{
	ssize_t w;

	w = write(fd, sprom, SPROM_SIZE);
	if (w < 0)
		return -1;

	return 0;
}

static int write_output_hex(int fd, const uint8_t *sprom)
{
	ssize_t w;
	char tmp[SPROM_SIZE * 2 + 10] = { 0 };

	hexdump_sprom(sprom, tmp, sizeof(tmp));
	prinfo("Raw output:  %s\n", tmp);
	w = write(fd, tmp, SPROM_SIZE * 2);
	if (w < 0)
		return -1;

	return 0;
}

static int write_output(int fd, const uint8_t *sprom)
{
	int err;

	if (cmdargs.outfile) {
		err = ftruncate(fd, 0);
		if (err) {
			prerror("Could not truncate --outfile %s\n",
				cmdargs.outfile);
			return -1;
		}
	}

	if (cmdargs.hex_mode)
		err = write_output_hex(fd, sprom);
	else
		err = write_output_binary(fd, sprom);
	if (err)
		prerror("Could not write output data.\n");

	return err;
}

static int modify_value(uint8_t *sprom,
			struct cmdline_vparm *vparm)
{
	const uint16_t v = vparm->u.value;
	uint16_t tmp = 0;

	switch (vparm->type) {
	case VALUE_RAW:
		sprom[vparm->u.raw.offset] = vparm->u.raw.value;
		break;
	case VALUE_SUBP:
		sprom[SPROM_SUBP + 0] = (v & 0x00FF);
		sprom[SPROM_SUBP + 1] = (v & 0xFF00) >> 8;
		break;
	case VALUE_SUBV:
		sprom[SPROM_SUBV + 0] = (v & 0x00FF);
		sprom[SPROM_SUBV + 1] = (v & 0xFF00) >> 8;
		break;
	case VALUE_PPID:
		sprom[SPROM_PPID + 0] = (v & 0x00FF);
		sprom[SPROM_PPID + 1] = (v & 0xFF00) >> 8;
		break;
	case VALUE_BFLHI:
		sprom[SPROM_BFLHI + 0] = (v & 0x00FF);
		sprom[SPROM_BFLHI + 1] = (v & 0xFF00) >> 8;
		break;
	case VALUE_BFL:
		sprom[SPROM_BOARDFLAGS + 0] = (v & 0x00FF);
		sprom[SPROM_BOARDFLAGS + 1] = (v & 0xFF00) >> 8;
		break;
	case VALUE_BGMAC:
		sprom[SPROM_IL0MACADDR + 1] = vparm->u.mac[0];
		sprom[SPROM_IL0MACADDR + 0] = vparm->u.mac[1];
		sprom[SPROM_IL0MACADDR + 3] = vparm->u.mac[2];
		sprom[SPROM_IL0MACADDR + 2] = vparm->u.mac[3];
		sprom[SPROM_IL0MACADDR + 5] = vparm->u.mac[4];
		sprom[SPROM_IL0MACADDR + 4] = vparm->u.mac[5];
		break;
	case VALUE_ETMAC:
		sprom[SPROM_ET0MACADDR + 1] = vparm->u.mac[0];
		sprom[SPROM_ET0MACADDR + 0] = vparm->u.mac[1];
		sprom[SPROM_ET0MACADDR + 3] = vparm->u.mac[2];
		sprom[SPROM_ET0MACADDR + 2] = vparm->u.mac[3];
		sprom[SPROM_ET0MACADDR + 5] = vparm->u.mac[4];
		sprom[SPROM_ET0MACADDR + 4] = vparm->u.mac[5];
		break;
	case VALUE_AMAC:
		sprom[SPROM_ET1MACADDR + 1] = vparm->u.mac[0];
		sprom[SPROM_ET1MACADDR + 0] = vparm->u.mac[1];
		sprom[SPROM_ET1MACADDR + 3] = vparm->u.mac[2];
		sprom[SPROM_ET1MACADDR + 2] = vparm->u.mac[3];
		sprom[SPROM_ET1MACADDR + 5] = vparm->u.mac[4];
		sprom[SPROM_ET1MACADDR + 4] = vparm->u.mac[5];
		break;
	case VALUE_ET0PHY:
		tmp |= sprom[SPROM_ETHPHY + 0];
		tmp |= sprom[SPROM_ETHPHY + 1] << 8;
		tmp = ((tmp & 0x001F) | (v & 0x1F));
		sprom[SPROM_ETHPHY + 0] = (tmp & 0x00FF);
		sprom[SPROM_ETHPHY + 1] = (tmp & 0xFF00) >> 8;
		break;
	case VALUE_ET1PHY:
		tmp |= sprom[SPROM_ETHPHY + 0];
		tmp |= sprom[SPROM_ETHPHY + 1] << 8;
		tmp = ((tmp & 0x03E0) | ((v & 0x1F) << 5));
		sprom[SPROM_ETHPHY + 0] = (tmp & 0x00FF);
		sprom[SPROM_ETHPHY + 1] = (tmp & 0xFF00) >> 8;
		break;
	case VALUE_ET0MDC:
		sprom[SPROM_ETHPHY + 1] &= ~(1 << 14);
		if (v)
			sprom[SPROM_ETHPHY + 1] |= (1 << 14);
		break;
	case VALUE_ET1MDC:
		sprom[SPROM_ETHPHY + 1] &= ~(1 << 15);
		if (v)
			sprom[SPROM_ETHPHY + 1] |= (1 << 15);
		break;
	case VALUE_BREV:
		sprom[SPROM_BOARDREV + 0] = v;
		break;
	case VALUE_LOC:
		tmp = (sprom[SPROM_BOARDREV + 1] & 0xF0);
		tmp |= (v & 0x0F);
		sprom[SPROM_BOARDREV + 1] = (tmp & 0xFF);
		break;
	case VALUE_ANTA0:
		sprom[SPROM_BOARDREV + 1] &= ~(1 << 4);
		if (v)
			sprom[SPROM_BOARDREV + 1] |= (1 << 4);
		break;
	case VALUE_ANTA1:
		sprom[SPROM_BOARDREV + 1] &= ~(1 << 5);
		if (v)
			sprom[SPROM_BOARDREV + 1] |= (1 << 5);
		break;
	case VALUE_ANTBG0:
		sprom[SPROM_BOARDREV + 1] &= ~(1 << 6);
		if (v)
			sprom[SPROM_BOARDREV + 1] |= (1 << 6);
		break;
	case VALUE_ANTBG1:
		sprom[SPROM_BOARDREV + 1] &= ~(1 << 7);
		if (v)
			sprom[SPROM_BOARDREV + 1] |= (1 << 7);
		break;
	case VALUE_ANTGA:
		sprom[SPROM_ANTENNA_GAIN + 0] = (v & 0xFF);
		break;
	case VALUE_ANTGBG:
		sprom[SPROM_ANTENNA_GAIN + 1] = (v & 0xFF);
		break;
	case VALUE_PA0B0:
		sprom[SPROM_PA0B0 + 0] = (v & 0x00FF);
		sprom[SPROM_PA0B0 + 1] = (v & 0xFF00) >> 8;
		break;
	case VALUE_PA0B1:
		sprom[SPROM_PA0B1 + 0] = (v & 0x00FF);
		sprom[SPROM_PA0B1 + 1] = (v & 0xFF00) >> 8;
		break;
	case VALUE_PA0B2:
		sprom[SPROM_PA0B2 + 0] = (v & 0x00FF);
		sprom[SPROM_PA0B2 + 1] = (v & 0xFF00) >> 8;
		break;
	case VALUE_PA1B0:
		sprom[SPROM_PA1B0 + 0] = (v & 0x00FF);
		sprom[SPROM_PA1B0 + 1] = (v & 0xFF00) >> 8;
		break;
	case VALUE_PA1B1:
		sprom[SPROM_PA1B1 + 0] = (v & 0x00FF);
		sprom[SPROM_PA1B1 + 1] = (v & 0xFF00) >> 8;
		break;
	case VALUE_PA1B2:
		sprom[SPROM_PA1B2 + 0] = (v & 0x00FF);
		sprom[SPROM_PA1B2 + 1] = (v & 0xFF00) >> 8;
		break;
	case VALUE_WL0GPIO0:
		sprom[SPROM_WL0GPIO0 + 0] = (v & 0xFF);
		break;
	case VALUE_WL0GPIO1:
		sprom[SPROM_WL0GPIO0 + 1] = (v & 0xFF);
		break;
	case VALUE_WL0GPIO2:
		sprom[SPROM_WL0GPIO2 + 0] = (v & 0xFF);
		break;
	case VALUE_WL0GPIO3:
		sprom[SPROM_WL0GPIO2 + 1] = (v & 0xFF);
		break;
	case VALUE_MAXPA:
		sprom[SPROM_MAXPWR + 0] = (v & 0xFF);
		break;
	case VALUE_MAXPBG:
		sprom[SPROM_MAXPWR + 1] = (v & 0xFF);
		break;
	case VALUE_ITSSIA:
		sprom[SPROM_IDL_TSSI_TGT + 0] = (v & 0xFF);
		break;
	case VALUE_ITSSIBG:
		sprom[SPROM_IDL_TSSI_TGT + 1] = (v & 0xFF);
		break;
	case VALUE_SVER:
		sprom[SPROM_VERSION + 0] = (v & 0xFF);
		break;
	default:
		prerror("vparm->type internal error (0)\n");
		exit(1);
	}

	return 0;
}

static int modify_sprom(uint8_t *sprom)
{
	struct cmdline_vparm *vparm;
	int i;
	int modified = 0;
	uint8_t crc;

	for (i = 0; i < cmdargs.nr_vparm; i++) {
		vparm = &(cmdargs.vparm[i]);
		if (!vparm->set)
			continue;
		modify_value(sprom, vparm);
		modified = 1;
	}
	if (modified) {
		/* Recalculate the CRC. */
		crc = sprom_crc(sprom);
		sprom[SPROM_VERSION + 1] = crc;
	}

	return modified;
}

static void display_value(const uint8_t *sprom,
			  struct cmdline_vparm *vparm)
{
	const char *desc;
	uint8_t offset;
	uint16_t value;
	uint16_t tmp;

	switch (vparm->type) {
	case VALUE_RAW:
		desc = "RAW";
		offset = vparm->u.raw.offset;
		value = sprom[offset];
		break;
	case VALUE_SUBP:
		desc = "Subsytem product ID";
		offset = SPROM_SUBP;
		value = sprom[SPROM_SUBP + 0];
		value |= sprom[SPROM_SUBP + 1] << 8;
		break;
	case VALUE_SUBV:
		desc = "Subsystem vendor ID";
		offset = SPROM_SUBV;
		value = sprom[SPROM_SUBV + 0];
		value |= sprom[SPROM_SUBV + 1] << 8;
		break;
	case VALUE_PPID:
		desc = "PCI Product ID";
		offset = SPROM_PPID;
		value = sprom[SPROM_PPID + 0];
		value |= sprom[SPROM_PPID + 1] << 8;
		break;
	case VALUE_BFLHI:
		desc = "High 16 bits of Boardflags";
		offset = SPROM_BFLHI;
		value = sprom[SPROM_BFLHI + 0];
		value |= sprom[SPROM_BFLHI + 1] << 8;
		break;
	case VALUE_BFL:
		desc = "Low 16 bits of Boardflags";
		offset = SPROM_BOARDFLAGS;
		value = sprom[SPROM_BOARDFLAGS + 0];
		value |= sprom[SPROM_BOARDFLAGS + 1] << 8;
		break;
	case VALUE_BGMAC:
		desc = "MAC address for 802.11b/g";
		offset = SPROM_IL0MACADDR;
		value = 0;
		break;
	case VALUE_ETMAC:
		desc = "MAC address for ethernet";
		offset = SPROM_ET0MACADDR;
		value = 0;
		break;
	case VALUE_AMAC:
		desc = "MAC address for 802.11a";
		offset = SPROM_ET1MACADDR;
		value = 0;
		break;
	case VALUE_ET0PHY:
		desc = "Ethernet phy settings (0)";
		offset = SPROM_ETHPHY;
		tmp = sprom[SPROM_ETHPHY + 0];
		tmp |= sprom[SPROM_ETHPHY + 1] << 8;
		value = (tmp & 0x001F);
		break;
	case VALUE_ET1PHY:
		desc = "Ethernet phy settings (1)";
		offset = SPROM_ETHPHY;
		tmp = sprom[SPROM_ETHPHY + 0];
		tmp |= sprom[SPROM_ETHPHY + 1] << 8;
		value = (tmp & 0x03E0) >> 5;
		break;
	case VALUE_ET0MDC:
		desc = "et0mdcport";
		offset = SPROM_ETHPHY + 1;
		value = 0;
		if (sprom[SPROM_ETHPHY + 1] & (1 << 14))
			value = 1;
		break;
	case VALUE_ET1MDC:
		desc = "et1mdcport";
		offset = SPROM_ETHPHY + 1;
		value = 0;
		if (sprom[SPROM_ETHPHY + 1] & (1 << 15))
			value = 1;
		break;
	case VALUE_BREV:
		desc = "Board revision";
		offset = SPROM_BOARDREV;
		value = sprom[SPROM_BOARDREV + 0];
		break;
	case VALUE_LOC:
		desc = "Locale / Country Code";
		offset = SPROM_BOARDREV + 1;
		value = (sprom[SPROM_BOARDREV + 1] & 0x0F);
		break;
	case VALUE_ANTA0:
		desc = "A PHY antenna 0 available";
		offset = SPROM_BOARDREV + 1;
		value = 0;
		if (sprom[SPROM_BOARDREV + 1] & (1 << 4))
			value = 1;
		break;
	case VALUE_ANTA1:
		desc = "A PHY antenna 1 available";
		offset = SPROM_BOARDREV + 1;
		value = 0;
		if (sprom[SPROM_BOARDREV + 1] & (1 << 5))
			value = 1;
		break;
	case VALUE_ANTBG0:
		desc = "B/G PHY antenna 1 available";
		offset = SPROM_BOARDREV + 1;
		value = 0;
		if (sprom[SPROM_BOARDREV + 1] & (1 << 6))
			value = 1;
		break;
	case VALUE_ANTBG1:
		desc = "B/G PHY antenna 1 available";
		offset = SPROM_BOARDREV + 1;
		value = 0;
		if (sprom[SPROM_BOARDREV + 1] & (1 << 7))
			value = 1;
		break;
	case VALUE_ANTGA:
		desc = "A PHY antenna gain";
		offset = SPROM_ANTENNA_GAIN;
		value = sprom[SPROM_ANTENNA_GAIN];
		break;
	case VALUE_ANTGBG:
		desc = "A PHY antenna gain";
		offset = SPROM_ANTENNA_GAIN + 1;
		value = sprom[SPROM_ANTENNA_GAIN + 1];
		break;
	case VALUE_PA0B0:
		desc = "pa0b0";
		offset = SPROM_PA0B0;
		value = sprom[offset + 0];
		value |= sprom[offset + 1] << 8;
		break;
	case VALUE_PA0B1:
		desc = "pa0b1";
		offset = SPROM_PA0B1;
		value = sprom[offset + 0];
		value |= sprom[offset + 1] << 8;
		break;
	case VALUE_PA0B2:
		desc = "pa0b2";
		offset = SPROM_PA0B2;
		value = sprom[offset + 0];
		value |= sprom[offset + 1] << 8;
		break;
	case VALUE_PA1B0:
		desc = "pa1b0";
		offset = SPROM_PA1B0;
		value = sprom[offset + 0];
		value |= sprom[offset + 1] << 8;
		break;
	case VALUE_PA1B1:
		desc = "pa1b1";
		offset = SPROM_PA1B1;
		value = sprom[offset + 0];
		value |= sprom[offset + 1] << 8;
		break;
	case VALUE_PA1B2:
		desc = "pa1b2";
		offset = SPROM_PA1B2;
		value = sprom[offset + 0];
		value |= sprom[offset + 1] << 8;
		break;
	case VALUE_WL0GPIO0:
		desc = "LED 0 behaviour";
		offset = SPROM_WL0GPIO0 + 0;
		value = sprom[offset];
		break;
	case VALUE_WL0GPIO1:
		desc = "LED 1 behaviour";
		offset = SPROM_WL0GPIO0 + 1;
		value = sprom[offset];
		break;
	case VALUE_WL0GPIO2:
		desc = "LED 2 behaviour";
		offset = SPROM_WL0GPIO2 + 0;
		value = sprom[offset];
		break;
	case VALUE_WL0GPIO3:
		desc = "LED 3 behaviour";
		offset = SPROM_WL0GPIO2 + 1;
		value = sprom[offset];
		break;
	case VALUE_MAXPA:
		desc = "A PHY max powerout";
		offset = SPROM_MAXPWR + 0;
		value = sprom[offset];
		break;
	case VALUE_MAXPBG:
		desc = "B/G PHY max powerout";
		offset = SPROM_MAXPWR + 1;
		value = sprom[offset];
		break;
	case VALUE_ITSSIA:
		desc = "A PHY idle TSSI target";
		offset = SPROM_IDL_TSSI_TGT + 0;
		value = sprom[offset];
		break;
	case VALUE_ITSSIBG:
		desc = "B/G PHY idle TSSI target";
		offset = SPROM_IDL_TSSI_TGT + 1;
		value = sprom[offset];
		break;
	case VALUE_SVER:
		desc = "SPROM version";
		offset = SPROM_VERSION;
		value = sprom[offset];
		break;
	default:
		prerror("vparm->type internal error (1)\n");
		exit(1);
	}

	switch (vparm->bits) {
	case 1:
		prdata("SPROM(0x%02X, %s) = %s\n",
		       offset, desc, value ? "ON" : "OFF");
		break;
	case 4:
		prdata("SPROM(0x%02X, %s) = 0x%01X\n",
		       offset, desc, (value & 0xF));
		break;
	case 8:
		prdata("SPROM(0x%02X, %s) = 0x%02X\n",
		       offset, desc, (value & 0xFF));
		break;
	case 16:
		prdata("SPROM(0x%02X, %s) = 0x%04X\n",
		       offset, desc, (value & 0xFFFF));
		break;
	case -1: {
		/* MAC address. */
		const uint8_t *p = &(sprom[offset]);

		prdata("SPROM(0x%02X, %s) = %02x:%02x:%02x:%02x:%02x:%02x\n",
		       offset, desc,
		       p[1], p[0], p[3], p[2], p[5], p[4]);
		break;
	}
	default:
		prerror("vparm->bits internal error (%d)\n",
			vparm->bits);
		exit(1);
	}
}

static int display_sprom(const uint8_t *sprom)
{
	struct cmdline_vparm *vparm;
	int i;

	for (i = 0; i < cmdargs.nr_vparm; i++) {
		vparm = &(cmdargs.vparm[i]);
		if (vparm->set)
			continue;
		display_value(sprom, vparm);
	}

	return 0;
}

static int validate_input(const uint8_t *sprom)
{
	uint8_t crc, expected_crc;

	crc = sprom_crc(sprom);
	expected_crc = sprom[SPROM_VERSION + 1];
	if (crc != expected_crc) {
		prerror("Corrupt input data (crc: 0x%02X, expected: 0x%02X)\n",
			crc, expected_crc);
		if (!cmdargs.force)
			return 1;
	}

	return 0;
}

static int parse_input(uint8_t *sprom, char *buffer, size_t bsize)
{
	char *input;
	size_t inlen;
	size_t cnt;
	unsigned long parsed;
	char tmp[SPROM_SIZE * 2 + 10] = { 0 };

	if (!cmdargs.hex_mode) {
		/* The input buffer already contains
		 * the binary sprom data.
		 */
		internal_error_on(bsize != SPROM_SIZE);
		memcpy(sprom, buffer, SPROM_SIZE);
		return 0;
	}

	inlen = bsize;
	input = strchr(buffer, ':');
	if (input) {
		input++;
		inlen -= input - buffer;
	} else
		input = buffer;

	if (inlen < SPROM_SIZE * 2) {
		prerror("Input data too short\n");
		return -1;
	}
	for (cnt = 0; cnt < SPROM_SIZE; cnt++) {
		memcpy(tmp, input + cnt * 2, 2);
		parsed = strtoul(tmp, NULL, 16);
		sprom[cnt] = parsed & 0xFF;
	}

	if (cmdargs.verbose) {
		hexdump_sprom(sprom, tmp, sizeof(tmp));
		prinfo("Raw input:  %s\n", tmp);
	}

	return 0;
}

static int read_infile(int fd, char **buffer, size_t *bsize)
{
	struct stat s;
	int err;
	ssize_t r;

	err = fstat(fd, &s);
	if (err) {
		prerror("Could not stat input file.\n");
		return err;
	}
	if (s.st_size == 0) {
		prerror("No input data\n");
		return -1;
	}
	if (cmdargs.hex_mode) {
		if (s.st_size > 1024 * 1024) {
			prerror("The input data does not look "
				"like SPROM HEX data (too long).\n");
			return -1;
		}
	} else {
		if (s.st_size != SPROM_SIZE) {
			prerror("The input data is no SPROM Binary data. "
				"The size must be exactly %d bytes, "
				"but it is %u bytes\n",
				SPROM_SIZE, (unsigned int)(s.st_size));
			return -1;
		}
	}

	*bsize = s.st_size;
	if (cmdargs.hex_mode)
		(*bsize)++;
	*buffer = malloce(*bsize);
	r = read(fd, *buffer, s.st_size);
	if (r != s.st_size) {
		prerror("Could not read input data.\n");
		return -1;
	}
	if (cmdargs.hex_mode)
		(*buffer)[r] = '\0';

	return 0;
}

static void close_infile(int fd)
{
	if (cmdargs.infile)
		close(fd);
}

static void close_outfile(int fd)
{
	if (cmdargs.outfile)
		close(fd);
}

static int open_infile(int *fd)
{
	*fd = STDIN_FILENO;
	if (!cmdargs.infile)
		return 0;
	*fd = open(cmdargs.infile, O_RDONLY);
	if (*fd < 0) {
		prerror("Could not open --infile %s\n",
			cmdargs.infile);
		return -1;
	}

	return 0;
}

static int open_outfile(int *fd)
{
	*fd = STDOUT_FILENO;
	if (!cmdargs.outfile)
		return 0;
	*fd = open(cmdargs.outfile, O_RDWR | O_CREAT, 0644);
	if (*fd < 0) {
		prerror("Could not open --outfile %s\n",
			cmdargs.outfile);
		return -1;
	}

	return 0;
}

static void print_banner(int forceprint)
{
	const char *str = "BCM43xx SPROM data modification tool version " VERSION "\n";
	if (forceprint)
		prdata(str);
	else
		prinfo(str);
}

static void print_usage(int argc, char *argv[])
{
	print_banner(1);
	prdata("\nUsage: %s [OPTION]\n", argv[0]);
	prdata("  -i|--input FILE       Input file\n");
	prdata("  -o|--output FILE      Output file\n");
	prdata("  -H|--hexmode          The Input data is HEX formatted and Output will be HEX\n");
	prdata("  -V|--verbose          Be verbose\n");
	prdata("  -f|--force            Override error checks\n");
	prdata("  -v|--version          Print version\n");
	prdata("  -h|--help             Print this help\n");
	prdata("\n");
	prdata("Value Parameters:\n");
	prdata("\n");
	prdata("  -s|--rawset OFF,VAL   Set a VALue at a byte-OFFset\n");
	prdata("  -g|--rawget OFF       Get a value at a byte-OFFset\n");
	prdata("\n");
	prdata("Predefined values (for displaying (GET) or modification):\n");
	prdata("  --subp [0xFFFF]       Subsytem product ID for PCI\n");
	prdata("  --subv [0xFFFF]       Subsystem vendor ID for PCI\n");
	prdata("  --ppid [0xFFFF]       Product ID for PCI\n");
	prdata("  --bflhi [0xFFFF]      High 16 bits of boardflags (only if spromversion > 1)\n");
	prdata("  --bfl [0xFFFF]        Low 16 bits of boardflags\n");
	prdata("  --bgmac [MAC-ADDR]    MAC address for 802.11b/g\n");
	prdata("  --etmac [MAC-ADDR]    MAC address for ethernet, see b44 driver\n");
	prdata("  --amac [MAC-ADDR]     Mac address for 802.11a\n");
	prdata("  --et0phy [0xFF]\n");
	prdata("  --et1phy [0xFF]\n");
	prdata("  --et0mdc [BOOL]\n");
	prdata("  --et1mdc [BOOL]\n");
	prdata("  --brev [0xFF]         Board revision\n");
	prdata("  --loc [0xF]           Country code\n");
	prdata("  --anta0 [BOOL]        Antenna 0 available for A PHY\n");
	prdata("  --anta1 [BOOL]        Antenna 1 available for A PHY\n");
	prdata("  --antbg0 [BOOL]       Antenna 0 available for B/G PHY\n");
	prdata("  --antbg1 [BOOL]       Antenna 1 available for B/G PHY\n");
	prdata("  --antga [0xFF]        Antenna gain for A PHY\n");
	prdata("  --antgbg [0xFF]       Antenna gain for B/G PHY\n");
	prdata("  --pa0b0 [0xFFFF]\n");
	prdata("  --pa0b1 [0xFFFF]\n");
	prdata("  --pa0b2 [0xFFFF]\n");
	prdata("  --pa1b0 [0xFFFF]\n");
	prdata("  --pa1b1 [0xFFFF]\n");
	prdata("  --pa1b2 [0xFFFF]\n");
	prdata("  --wl0gpio0 [0xFF]     LED 0 behaviour\n");
	prdata("  --wl0gpio1 [0xFF]     LED 1 behaviour\n");
	prdata("  --wl0gpio2 [0xFF]     LED 2 behaviour\n");
	prdata("  --wl0gpio3 [0xFF]     LED 3 behaviour\n");
	prdata("  --maxpa [0xFF]        A PHY max power\n");
	prdata("  --maxpbg [0xFF]       B/G PHY max power\n");
	prdata("  --itssia [0xFF]       Idle tssi target for A PHY\n");
	prdata("  --itssibg [0xFF]      Idle tssi target for B/G PHY\n");
	prdata("  --sver [0xFF]         SPROM-version\n");
	prdata("\n");
	prdata(" BOOL      is a boolean value. Either 0 or 1\n");
	prdata(" 0xF..     is a hexadecimal value\n");
	prdata(" MAC-ADDR  is a MAC address in the format 00:00:00:00:00:00\n");
	prdata(" If the value parameter is \"GET\", the value will be printed;\n");
	prdata(" otherwise it is modified.\n");
}

#define ARG_MATCH		0
#define ARG_NOMATCH		1
#define ARG_ERROR		-1

static int do_cmp_arg(char **argv, int *pos,
		      const char *template,
		      int allow_merged,
		      char **param)
{
	char *arg;
	char *next_arg;
	size_t arg_len, template_len;

	arg = argv[*pos];
	next_arg = argv[*pos + 1];
	arg_len = strlen(arg);
	template_len = strlen(template);

	if (param) {
		/* Maybe we have a merged parameter here.
		 * A merged parameter is "-pfoobar" for example.
		 */
		if (allow_merged && arg_len > template_len) {
			if (memcmp(arg, template, template_len) == 0) {
				*param = arg + template_len;
				return ARG_MATCH;
			}
			return ARG_NOMATCH;
		} else if (arg_len != template_len)
			return ARG_NOMATCH;
		*param = next_arg;
	}
	if (strcmp(arg, template) == 0) {
		if (param) {
			/* Skip the parameter on the next iteration. */
			(*pos)++;
			if (*param == 0) {
				prerror("%s needs a parameter\n", arg);
				return ARG_ERROR;
			}
		}
		return ARG_MATCH;
	}

	return ARG_NOMATCH;
}

/* Simple and lean command line argument parsing. */
static int cmp_arg(char **argv, int *pos,
		   const char *long_template,
		   const char *short_template,
		   char **param)
{
	int err;

	if (long_template) {
		err = do_cmp_arg(argv, pos, long_template, 0, param);
		if (err == ARG_MATCH || err == ARG_ERROR)
			return err;
	}
	err = ARG_NOMATCH;
	if (short_template)
		err = do_cmp_arg(argv, pos, short_template, 1, param);
	return err;
}
#define arg_match(argv, i, tlong, tshort, param) \
	({						\
		int res = cmp_arg((argv), (i), (tlong),	\
				  (tshort), (param));	\
		if ((res) == ARG_ERROR)			\
	 		goto error;			\
		((res) == ARG_MATCH);			\
	})

static int parse_value(const char *str, int bits,
		       struct cmdline_vparm *vparm,
		       const char *param)
{
	unsigned long v;
	int i;

	vparm->bits = bits;
	vparm->set = 1;
	if (strcmp(str, "GET") == 0 || strcmp(str, "get") == 0) {
		vparm->set = 0;
		return 0;
	}
	if (bits == 1) {
		/* This is a boolean value. */
		if (strcmp(str, "0") == 0)
			vparm->u.value = 0;
		else if (strcmp(str, "1") == 0)
			vparm->u.value = 1;
		else
			goto error_bool;
		return 1;
	}

	if (strncmp(str, "0x", 2) != 0)
		goto error;
	str += 2;
	for (i = 0; i < bits / 4; i++) {
		if (str[i] == '\0')
			goto error;
	}
	if (str[i] != '\0')
		goto error;
	errno = 0;
	v = strtoul(str, NULL, 16);
	if (errno)
		goto error;
	vparm->u.value = v;

	return 1;
error:
	if (param) {
		prerror("%s value parsing error. Format: 0x", param);
		for (i = 0; i < bits / 4; i++)
			prerror("F");
		prerror("\n");
	}
	return -1;

error_bool:
	if (param)
		prerror("%s value parsing error. Format: 0 or 1 (boolean)\n", param);
	return -1;
}

static int parse_mac(const char *str,
		     struct cmdline_vparm *vparm,
		     const char *param)
{
	int i;
	char *delim;
	const char *in = str;
	uint8_t *out = vparm->u.mac;

	vparm->bits = -1;
	vparm->set = 1;
	if (strcmp(str, "GET") == 0 || strcmp(str, "get") == 0) {
		vparm->set = 0;
		return 0;
	}

	for (i = 0; ; i++) {
		errno = 0;
		out[i] = strtoul(in, NULL, 16);
		if (errno)
			goto error;
		if (i == 5) {
			if (in[1] != '\0' && in[2] != '\0')
				goto error;
			break;
		}
		delim = strchr(in, ':');
		if (!delim)
			goto error;
		in = delim + 1;
	}

	return 1;
error:
	prerror("%s MAC parsing error. Format: 00:00:00:00:00:00\n", param);
	return -1;
}

static int parse_rawset(const char *str,
			struct cmdline_vparm *vparm)
{
	char *delim;
	uint8_t value;
	uint8_t offset;
	int err;

	delim = strchr(str, ',');
	if (!delim)
		goto error;
	*delim = '\0';
	err = parse_value(str, 8, vparm, NULL);
	if (err != 1)
		goto error;
	offset = vparm->u.value;
	if (offset >= SPROM_SIZE) {
		prerror("--rawset offset too big (>= 0x%02X)\n",
			SPROM_SIZE);
		return -1;
	}
	err = parse_value(delim + 1, 8, vparm, NULL);
	if (err != 1)
		goto error;
	value = vparm->u.value;

	vparm->u.raw.value = value;
	vparm->u.raw.offset = offset;
	vparm->type = VALUE_RAW;
	vparm->set = 1;
	vparm->bits = 8;

	return 0;
error:
	prerror("--rawset value parsing error. Format: 0xFF,0xFF "
		"(first Offset, second Value)\n");
	return -1;
}

static int parse_rawget(const char *str,
			struct cmdline_vparm *vparm)
{
	int err;
	uint8_t offset;

	err = parse_value(str, 8, vparm, "--rawget");
	if (err != 1)
		return -1;
	offset = vparm->u.value;
	if (offset >= SPROM_SIZE) {
		prerror("--rawget offset too big (>= 0x%02X)\n",
			SPROM_SIZE);
		return -1;
	}

	vparm->u.raw.offset = offset;
	vparm->type = VALUE_RAW;
	vparm->set = 0;
	vparm->bits = 8;

	return 0;
}

static int parse_args(int argc, char *argv[])
{
	struct cmdline_vparm *vparm;
	int i, err;
	char *param;

	for (i = 1; i < argc; i++) {
		if (cmdargs.nr_vparm == MAX_VPARM) {
			prerror("Too many value parameters.\n");
			return -1;
		}

		if (arg_match(argv, &i, "--version", "-v", 0)) {
			print_banner(1);
			return 1;
		} else if (arg_match(argv, &i, "--help", "-h", 0)) {
			goto out_usage;
		} else if (arg_match(argv, &i, "--input", "-i", &param)) {
			cmdargs.infile = param;
		} else if (arg_match(argv, &i, "--output", "-o", &param)) {
			cmdargs.outfile = param;
		} else if (arg_match(argv, &i, "--verbose", "-V", 0)) {
			cmdargs.verbose = 1;
		} else if (arg_match(argv, &i, "--force", "-n", 0)) {
			cmdargs.force = 1;
		} else if (arg_match(argv, &i, "--hexmode", "-H", 0)) {
			cmdargs.hex_mode = 1;


		} else if (arg_match(argv, &i, "--rawset", "-s", &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			err = parse_rawset(param, vparm);
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--rawget", "-g", &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			err = parse_rawget(param, vparm);
			if (err < 0)
				goto error;


		} else if (arg_match(argv, &i, "--subp", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_SUBP;
			err = parse_value(param, 16, vparm, "--subp");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--subv", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_SUBV;
			err = parse_value(param, 16, vparm, "--subv");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--ppid", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_PPID;
			err = parse_value(param, 16, vparm, "--ppid");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--bflhi", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_BFLHI;
			err = parse_value(param, 16, vparm, "--bflhi");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--bfl", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_BFL;
			err = parse_value(param, 16, vparm, "--bfl");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--bgmac", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_BGMAC;
			err = parse_mac(param, vparm, "--bgmac");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--etmac", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_ETMAC;
			err = parse_mac(param, vparm, "--etmac");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--amac", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_AMAC;
			err = parse_mac(param, vparm, "--amac");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--et0phy", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_ET0PHY;
			err = parse_value(param, 8, vparm, "--et0phy");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--et1phy", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_ET1PHY;
			err = parse_value(param, 8, vparm, "--et1phy");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--et0mdc", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_ET0MDC;
			err = parse_value(param, 1, vparm, "--et0mdc");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--et1mdc", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_ET1MDC;
			err = parse_value(param, 1, vparm, "--et1mdc");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--brev", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_BREV;
			err = parse_value(param, 8, vparm, "--brev");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--loc", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_LOC;
			err = parse_value(param, 4, vparm, "--loc");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--anta0", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_ANTA0;
			err = parse_value(param, 1, vparm, "--anta0");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--anta1", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_ANTA1;
			err = parse_value(param, 1, vparm, "--anta1");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--antbg0", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_ANTBG0;
			err = parse_value(param, 1, vparm, "--antbg0");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--antbg1", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_ANTBG1;
			err = parse_value(param, 1, vparm, "--antbg1");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--antga", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_ANTGA;
			err = parse_value(param, 8, vparm, "--antga");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--antgbg", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_ANTGBG;
			err = parse_value(param, 8, vparm, "--antgbg");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--pa0b0", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_PA0B0;
			err = parse_value(param, 16, vparm, "--pa0b0");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--pa0b1", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_PA0B1;
			err = parse_value(param, 16, vparm, "--pa0b1");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--pa0b2", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_PA0B2;
			err = parse_value(param, 16, vparm, "--pa0b2");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--pa1b0", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_PA1B0;
			err = parse_value(param, 16, vparm, "--pa1b0");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--pa1b1", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_PA1B1;
			err = parse_value(param, 16, vparm, "--pa1b1");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--pa1b2", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_PA1B2;
			err = parse_value(param, 16, vparm, "--pa1b2");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--wl0gpio0", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_WL0GPIO0;
			err = parse_value(param, 8, vparm, "--wl0gpio0");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--wl0gpio1", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_WL0GPIO1;
			err = parse_value(param, 8, vparm, "--wl0gpio1");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--wl0gpio2", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_WL0GPIO2;
			err = parse_value(param, 8, vparm, "--wl0gpio2");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--wl0gpio3", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_WL0GPIO3;
			err = parse_value(param, 8, vparm, "--wl0gpio3");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--maxpa", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_MAXPA;
			err = parse_value(param, 8, vparm, "--maxpa");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--maxpbg", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_MAXPBG;
			err = parse_value(param, 8, vparm, "--maxpbg");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--itssia", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_ITSSIA;
			err = parse_value(param, 8, vparm, "--itssia");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--itssibg", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_ITSSIBG;
			err = parse_value(param, 8, vparm, "--itssibg");
			if (err < 0)
				goto error;
		} else if (arg_match(argv, &i, "--sver", 0, &param)) {
			vparm = &(cmdargs.vparm[cmdargs.nr_vparm++]);
			vparm->type = VALUE_SVER;
			err = parse_value(param, 8, vparm, "--sver");
			if (err < 0)
				goto error;
		} else {
			prerror("Unrecognized argument: %s\n", argv[i]);
			goto out_usage;
		}
	}
	if (cmdargs.nr_vparm == 0) {
		prerror("No Value parameter given. See --help.\n");
		return -1;
	}
	return 0;

out_usage:
	print_usage(argc, argv);
error:
	return -1;	
}


int main(int argc, char **argv)
{
	int err;
	int fd;
	uint8_t sprom[SPROM_SIZE];
	char *buffer = NULL;
	size_t buffer_size = 0;

	err = parse_args(argc, argv);
	if (err == 1)
		return 0;
	else if (err != 0)
		goto out;

	print_banner(0);
	prinfo("\nReading input from \"%s\"...\n",
	       cmdargs.infile ? cmdargs.infile : "stdin");

	err = open_infile(&fd);
	if (err)
		goto out;
	err = read_infile(fd, &buffer, &buffer_size);
	close_infile(fd);
	if (err)
		goto out;
	err = parse_input(sprom, buffer, buffer_size);
	free(buffer);
	if (err)
		goto out;
	err = validate_input(sprom);
	if (err)
		goto out;

	err = display_sprom(sprom);
	if (err)
		goto out;
	err = modify_sprom(sprom);
	if (err < 0)
		goto out;
	if (err) {
		err = open_outfile(&fd);
		if (err)
			goto out;
		err = write_output(fd, sprom);
		close_outfile(fd);
		if (err)
			goto out;
		prinfo("SPROM modified.\n");
	}
out:
	return err;
}
