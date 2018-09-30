/*
 * probe_roms - scan for Adapter ROMS
 *
 * (based on linux-2.6:arch/x86/kernel/probe_roms_32.c)
 *
 * Copyright (C) 2008 Intel Corporation
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "probe_roms.h"
#include "mdadm.h"
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <asm/types.h>

static void *rom_mem = MAP_FAILED;
static int rom_fd = -1;
static const int rom_len = 0xf0000 - 0xc0000; /* option-rom memory region */
static int _sigbus;
static unsigned long rom_align;

static void sigbus(int sig)
{
	_sigbus = 1;
}

static int probe_address8(const __u8 *ptr, __u8 *val)
{
	int rc = 0;

	*val = *ptr;
	if (_sigbus)
		rc = -1;
	_sigbus = 0;

	return rc;
}

static int probe_address16(const __u16 *ptr, __u16 *val)
{
	int rc = 0;

	*val = *ptr;
	if (_sigbus)
		rc = -1;
	_sigbus = 0;

	return rc;
}

void probe_roms_exit(void)
{
	signal(SIGBUS, SIG_DFL);
	if (rom_fd >= 0) {
		close(rom_fd);
		rom_fd = -1;
	}
	if (rom_mem != MAP_FAILED) {
		munmap(rom_mem, rom_len);
		rom_mem = MAP_FAILED;
	}
}

int probe_roms_init(unsigned long align)
{
	int fd = -1;
	int rc = 0;

	/* valid values are 2048 and 512.  512 is for PCI-3.0 compliant
	 * systems, or systems that do not have dangerous/legacy ISA
	 * devices.  2048 should always be safe
	 */
	if (align == 512 || align == 2048)
		rom_align = align;
	else
		return -1;

	if (signal(SIGBUS, sigbus) == SIG_ERR)
		rc = -1;
	if (rc == 0) {
		fd = open("/dev/mem", O_RDONLY);
		if (fd < 0)
			rc = -1;
	}
	if (rc == 0) {
		rom_mem = mmap(NULL, rom_len, PROT_READ, MAP_PRIVATE, fd, 0xc0000);
		if (rom_mem == MAP_FAILED)
			rc = -1;
	}

	if (rc == 0)
		rom_fd = fd;
	else {
		if (fd >= 0)
			close(fd);
		probe_roms_exit();
	}
	return rc;
}

/**
 * isa_bus_to_virt - convert physical address to mmap'd region
 * @addr - address to convert
 *
 * Only valid between a successful call to probe_roms_init and the
 * corresponding probe_roms_exit
 */
static void *isa_bus_to_virt(unsigned long addr)
{
	return rom_mem + (addr - 0xc0000);
}

struct resource {
	unsigned long start;
	unsigned long end;
	unsigned long data;
	const char *name;
};

static struct resource system_rom_resource = {
	.name	= "System ROM",
	.start	= 0xf0000,
	.data   = 0,
	.end	= 0xfffff,
};

static struct resource extension_rom_resource = {
	.name	= "Extension ROM",
	.start	= 0xe0000,
	.data   = 0,
	.end	= 0xeffff,
};

static struct resource adapter_rom_resources[] = { {
	.name	= "Adapter ROM",
	.start	= 0xc8000,
	.data   = 0,
	.end	= 0,
}, {
	.name	= "Adapter ROM",
	.start	= 0,
	.data   = 0,
	.end	= 0,
}, {
	.name	= "Adapter ROM",
	.start	= 0,
	.data   = 0,
	.end	= 0,
}, {
	.name	= "Adapter ROM",
	.start	= 0,
	.data   = 0,
	.end	= 0,
}, {
	.name	= "Adapter ROM",
	.start	= 0,
	.data   = 0,
	.end	= 0,
}, {
	.name	= "Adapter ROM",
	.start	= 0,
	.data   = 0,
	.end	= 0,
} };

static struct resource video_rom_resource = {
	.name	= "Video ROM",
	.start	= 0xc0000,
	.data   = 0,
	.end	= 0xc7fff,
};

#define ROMSIGNATURE 0xaa55

static int romsignature(const unsigned char *rom)
{
	const unsigned short * const ptr = (const unsigned short *)rom;
	unsigned short sig = 0;

	return probe_address16(ptr, &sig) == 0 && sig == ROMSIGNATURE;
}

static int romchecksum(const unsigned char *rom, unsigned long length)
{
	unsigned char sum, c;

	for (sum = 0; length && probe_address8(rom++, &c) == 0; length--)
		sum += c;
	return !length && !sum;
}

int scan_adapter_roms(scan_fn fn)
{
	/* let scan_fn examing each of the adapter roms found by probe_roms */
	unsigned int i;
	int found;

	if (rom_fd < 0)
		return 0;

	found = 0;
	for (i = 0; i < ARRAY_SIZE(adapter_rom_resources); i++) {
		struct resource *res = &adapter_rom_resources[i];

		if (res->start) {
			found = fn(isa_bus_to_virt(res->start),
				   isa_bus_to_virt(res->end),
				   isa_bus_to_virt(res->data));
			if (found)
				break;
		} else
			break;
	}

	return found;
}

static unsigned long align(unsigned long addr, unsigned long alignment)
{
	return (addr + alignment - 1) & ~(alignment - 1);
}

void probe_roms(void)
{
	const void *rom;
	unsigned long start, length, upper;
	unsigned char c;
	unsigned int i;
	__u16 val=0;

	if (rom_fd < 0)
		return;

	/* video rom */
	upper = adapter_rom_resources[0].start;
	for (start = video_rom_resource.start; start < upper; start += rom_align) {
		rom = isa_bus_to_virt(start);
		if (!romsignature(rom))
			continue;

		video_rom_resource.start = start;

		if (probe_address8(rom + 2, &c) != 0)
			continue;

		/* 0 < length <= 0x7f * 512, historically */
		length = c * 512;

		/* if checksum okay, trust length byte */
		if (length && romchecksum(rom, length))
			video_rom_resource.end = start + length - 1;
		break;
	}

	start = align(video_rom_resource.end + 1, rom_align);
	if (start < upper)
		start = upper;

	/* system rom */
	upper = system_rom_resource.start;

	/* check for extension rom (ignore length byte!) */
	rom = isa_bus_to_virt(extension_rom_resource.start);
	if (romsignature(rom)) {
		length = extension_rom_resource.end - extension_rom_resource.start + 1;
		if (romchecksum(rom, length))
			upper = extension_rom_resource.start;
	}

	/* check for adapter roms on 2k boundaries */
	for (i = 0; i < ARRAY_SIZE(adapter_rom_resources) && start < upper; start += rom_align) {
		rom = isa_bus_to_virt(start);
		if (!romsignature(rom))
			continue;

		if (probe_address8(rom + 2, &c) != 0)
			continue;

		/* 0 < length <= 0x7f * 512, historically */
		length = c * 512;

		/* Retrieve 16-bit pointer to PCI Data Structure (offset 18h-19h)
		 * The data can be within 64KB forward of the first location
		 * of this code image. The pointer is in little-endian order
		 */

		if (probe_address16(rom + 0x18, &val) != 0)
			continue;
		val = __le16_to_cpu(val);

		/* but accept any length that fits if checksum okay */
		if (!length || start + length > upper || !romchecksum(rom, length))
			continue;

		adapter_rom_resources[i].start = start;
		adapter_rom_resources[i].data = start + (unsigned long) val;
		adapter_rom_resources[i].end = start + length - 1;

		start = adapter_rom_resources[i++].end & ~(rom_align - 1);
	}
}
