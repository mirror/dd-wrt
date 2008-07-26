
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2001, 2004  Free Software Foundation, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* command-line parameter defines */
#define RB_ASKNAME      0x01	/* ask for file name to reboot from */
#define RB_SINGLE       0x02	/* reboot to single user only */
#define RB_NOSYNC       0x04	/* dont sync before reboot */
#define RB_HALT         0x08	/* don't reboot, just halt */
#define RB_INITNAME     0x10	/* name given for /etc/init (unused) */
#define RB_DFLTROOT     0x20	/* use compiled-in rootdev */
#define RB_KDB          0x40	/* give control to kernel debugger */
#define RB_RDONLY       0x80	/* mount root fs read-only */
#define RB_DUMP         0x100	/* dump kernel memory before reboot */
#define RB_MINIROOT     0x200	/* mini-root present in memory at boot time */
#define RB_CONFIG       0x400	/* invoke user configuration routing */
#define RB_VERBOSE      0x800	/* print all potentially useful info */
#define RB_SERIAL       0x1000	/* user serial port as console */
#define RB_CDROM        0x2000	/* use cdrom as root */
#define RB_GDB		0x8000	/* use GDB remote debugger instead of DDB */
#define RB_MUTE		0x10000	/* Come up with the console muted */
#define RB_SELFTEST	0x20000 /* don't complete the boot; do selftest */
#define RB_RESERVED1	0x40000 /* reserved for internal use of boot blocks */
#define RB_RESERVED2	0x80000 /* reserved for internal use of boot blocks */
#define RB_PAUSE	0x100000 /* pause after each output line during probe */
#define RB_MULTIPLE	0x20000000	/* Use multiple consoles */

#define RB_BOOTINFO     0x80000000	/* have `struct bootinfo *' arg */

/*
 * Constants for converting boot-style device number to type,
 * adaptor (uba, mba, etc), unit number and partition number.
 * Type (== major device number) is in the low byte
 * for backward compatibility.  Except for that of the "magic
 * number", each mask applies to the shifted value.
 * Format:
 *       (4) (4) (4) (4)  (8)     (8)
 *      --------------------------------
 *      |MA | AD| CT| UN| PART  | TYPE |
 *      --------------------------------
 */
#define B_ADAPTORSHIFT          24
#define B_CONTROLLERSHIFT       20
#define B_UNITSHIFT             16
#define B_PARTITIONSHIFT        8
#define B_TYPESHIFT             0

#define B_DEVMAGIC      ((unsigned long)0xa0000000)

#define MAKEBOOTDEV(type, adaptor, controller, unit, partition) \
        (((type) << B_TYPESHIFT) | ((adaptor) << B_ADAPTORSHIFT) | \
        ((controller) << B_CONTROLLERSHIFT) | ((unit) << B_UNITSHIFT) | \
        ((partition) << B_PARTITIONSHIFT) | B_DEVMAGIC)


/* Only change the version number if you break compatibility. */
#define BOOTINFO_VERSION        1

#define N_BIOS_GEOM             8

typedef unsigned char u8_t;
typedef unsigned int u32_t;

/*
 * A zero bootinfo field often means that there is no info available.
 * Flags are used to indicate the validity of fields where zero is a
 * normal value.
 */
struct bootinfo
  {
    u32_t bi_version;
    u8_t *bi_kernelname;
    u32_t bi_nfs_diskless;
    /* End of fields that are always present. */
#define bi_endcommon            bi_n_bios_used
    u32_t bi_n_bios_used;
    u32_t bi_bios_geom[N_BIOS_GEOM];
    u32_t bi_size;
    u8_t bi_memsizes_valid;
    u8_t bi_bios_dev;
    u8_t bi_pad[2];
    u32_t bi_basemem;
    u32_t bi_extmem;
    u32_t bi_symtab;
    u32_t bi_esymtab;
    /* Items below only from advanced bootloader */
    u32_t bi_kernend;
    u32_t bi_envp;
    u32_t bi_modulep;
  };

#define MODINFO_END		0x0000		/* End of list */
#define MODINFO_NAME		0x0001		/* Name of module (string) */
#define MODINFO_TYPE		0x0002		/* Type of module (string) */
#define MODINFO_ADDR		0x0003		/* Loaded address */
#define MODINFO_SIZE		0x0004		/* Size of module */
#define MODINFO_EMPTY		0x0005		/* Has been deleted */
#define MODINFO_ARGS		0x0006		/* Parameters string */
#define MODINFO_METADATA	0x8000		/* Module-specfic */

