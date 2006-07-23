/*
 * Copyright (C) 2006  kb  <barnik@sednet.hu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

#include <sys/ioctl.h>

#include <linux/version.h>

#if KERNEL_VERSION(2,6,0) <= LINUX_VERSION_CODE
#define IS_KERNEL26 1
#else
#define IS_KERNEL26 0
#endif

#if IS_KERNEL26
#include <mtd/mtd-user.h>
#else
#include <linux/mtd/mtd.h>
#endif

#define LOADER_PART		1
#define OPENIXP_PART	3
#define LINUX_PART		6

#define MTDBLOCK_NAME "/dev/mtdblock/%d"
#define MTDDEV_NAME "/dev/mtd/%d"

#define SECTION_MAGIC 0xfeedbabe

struct section_header {
	u_int32_t	magic;		/* feedbabe */
	u_int32_t	size;		/* Length of file excluding header */
	u_int32_t	checksum;	/* checksum from magic to end of file */
	u_int32_t	counter;	/* write counter */
	u_int32_t	offset;		/* offset */
	char		name[128];	/* name of the section */
};

struct loader_header {
    u_int32_t	startup_code[4];
	u_int32_t	code_offset;
	u_int32_t	rootfs_size;
	u_int32_t	atag_list_start;
	u_int32_t	atag_list_dest;
	u_int32_t	atag_list_size;
	u_int32_t	linux_zimage_start;
	u_int32_t	linux_zimage_dest;
	u_int32_t	linux_zimage_size;
};

struct image_header {
	struct section_header sh;
	struct loader_header  lh;
};

uint32_t csumbuf( uint8_t *buf, uint32_t len )
{
	uint32_t csum = 0;
	for (; len; len--, buf++)
	  csum += *buf;
	return csum;
}

int main(int argc, char **argv)
{
	int fd;
	struct mtd_info_user mtdInfo;
	struct region_info_user mtdRegionInfo;
	struct erase_info_user mtdEraseInfo;
	
	struct image_header* phdr;
	unsigned long hdrlen;
	void* popenixp;
	unsigned long openixplen;
	void* plinux;
	unsigned long linuxlen;
    char fn_loader[sizeof(MTDBLOCK_NAME)];
    char fn_openixp[sizeof(MTDBLOCK_NAME)];
    char fn_linux[sizeof(MTDBLOCK_NAME)];
    char fd_openixp[sizeof(MTDDEV_NAME)];
    char fd_linux[sizeof(MTDDEV_NAME)];

    sprintf( fn_loader, MTDBLOCK_NAME, LOADER_PART );
	if(((fd = open(fn_loader, O_RDWR)) < 0)
			|| ((hdrlen = lseek(fd, 0, SEEK_END)) < 0)
			|| ((phdr = (struct image_header *)mmap(0, hdrlen, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == (void *) (-1))
			) {
		printf("Error reading section info %s\n",fn_loader);
		close(fd);
		return -1;
	}
	close(fd);
	
	if( phdr->sh.magic != SECTION_MAGIC )
	{
		printf("Bad section header %08X in %s\n",phdr->sh.magic,fn_loader);
		return -1;
	}

	if( phdr->sh.size <= sizeof(phdr->lh) )
	{
		printf("Partition already moved\n");
		return 1;
	}


    sprintf( fn_openixp, MTDBLOCK_NAME, OPENIXP_PART );
	if(((fd = open(fn_openixp, O_RDWR)) < 0)
			|| ((openixplen = lseek(fd, 0, SEEK_END)) < 0)
			|| ((popenixp = mmap(0, openixplen, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == (void *) (-1))
			) {
		printf("Error reading section info %s\n",fn_openixp);
		close(fd);
		return -1;
	}
	close(fd);

    sprintf( fn_linux, MTDBLOCK_NAME, LINUX_PART );
	if(((fd = open(fn_linux, O_RDWR)) < 0)
			|| ((linuxlen = lseek(fd, 0, SEEK_END)) < 0)
			|| ((plinux = mmap(0, linuxlen, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == (void *) (-1))
			) {
		printf("Error reading section info %s\n",fn_linux);
		close(fd);
		return -1;
	}
	close(fd);

	memcpy( plinux, popenixp, phdr->lh.linux_zimage_size );

    fprintf(stdout, "Syncing %s ...\n", fn_linux);
	if( msync( plinux, phdr->lh.linux_zimage_size, MS_SYNC|MS_INVALIDATE ) )
	{
		fprintf(stderr, "Could not sync %s\n", fn_linux);
		close(fd);
	}

	phdr->lh.linux_zimage_start = 0x700000 - 0x140094; //mtdRegion.offset - 0x94;
	
	phdr->sh.size = sizeof(phdr->lh);
	phdr->sh.checksum = 0;
	phdr->sh.checksum = csumbuf( (uint8_t *)phdr, sizeof(*phdr) );

    fprintf(stdout, "Syncing %s ...\n", fn_loader);
	if( msync( phdr, sizeof(phdr->sh), MS_SYNC|MS_INVALIDATE ) )
	{
		fprintf(stderr, "Could not sync %s\n", fn_loader);
		close(fd);
	}

    sprintf( fd_openixp, MTDDEV_NAME, OPENIXP_PART );
	if (((fd = open(fd_openixp, O_RDWR)) < 0)
			|| (ioctl(fd, MEMGETINFO, &mtdInfo))) {
		fprintf(stderr, "Could not get MTD device info from %s\n", fd_openixp);
		close(fd);
		return -1;
	}


    fprintf(stdout, "Unlocking %s ...\n", fd_openixp);
	mtdEraseInfo.start = 0;
	mtdEraseInfo.length = mtdInfo.size;
	if(ioctl(fd, MEMUNLOCK, &mtdEraseInfo)) {
		fprintf(stderr, "Could not unlock MTD device: %s\n", fd_openixp);
		close(fd);
		return -1;
	}
									
	fprintf(stdout, "Erasing %s ...\n", fd_openixp);
	mtdEraseInfo.length = mtdInfo.erasesize;
    for (mtdEraseInfo.start = 0;
	     mtdEraseInfo.start < mtdInfo.size;
		 mtdEraseInfo.start += mtdInfo.erasesize) {
					  
		if(ioctl(fd, MEMERASE, &mtdEraseInfo)) {
			fprintf(stderr, "Could not erase MTD device: %s\n", fd_openixp);
			close(fd);
			exit(-1);
		}
	}
	close(fd);

	fprintf(stdout,"Partition moved; please reboot.\n");

//	kill(1, 15); // send SIGTERM to init for reboot

	return 0;
}
