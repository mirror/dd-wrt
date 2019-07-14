/*
 * fix_standby.c  by Mark Lord, March 2006.
 *
 * This frees a drive from the "power up in standby" condition.
 * It tries to be safe, but is still quite risky when run
 * on a preemptive kernel, because it could preempt something
 * that was in the middle of issuing an IDE/SATA command.
 *
 * Hardcoded to work only on the first (master) drive
 * on the first (primary) IDE/SATA interface.
 */
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/io.h>

typedef unsigned char  u8;
typedef unsigned short u16;

enum {
	BUSY_STAT	= 0x80,
	READY_STAT	= 0x40,
	WRERR_STAT	= 0x20,
	SEEK_STAT	= 0x10,
	DRQ_STAT	= 0x08,
	ECC_STAT	= 0x04,
	INDEX_STAT	= 0x02,
	ERR_STAT	= 0x01,
};

enum {
	CHECKPOWERMODE	= 0xE5,
	SETFEATURES	= 0xEF,
	SF_SPINUP_NOW	= 0x07,
	SF_NO_STANDBY	= 0x86,
};

static unsigned int ide_base, ide_control, ide_altstatus;

enum {
	ide_data	= 0,
	ide_feature	= 1,
	ide_error	= ide_feature,
	ide_nsectors	= 2,
	ide_lbal	= 3,
	ide_lbam	= 4,
	ide_lbah	= 5,
	ide_select	= 6,
	ide_command	= 7,
	ide_status	= ide_command,
};

static inline void
cli (void)
{
	__asm__ __volatile__("cli" : : : "memory");
}

static inline void
sti (void)
{
	__asm__ __volatile__("sti" : : : "memory");
}

static void
wait_400ns (void)
{
	inb(ide_altstatus);
	inb(ide_altstatus);
	inb(ide_altstatus);
	inb(ide_altstatus);
	inb(ide_altstatus);
}

static inline void
OUTB (u8 val, unsigned int reg)
{
	outb(val, ide_base + reg);
}

static inline u8
INB (unsigned int reg)
{
	return inb(ide_base + reg);
}

static inline u16
INW (unsigned int reg)
{
	return inw(ide_base + reg);
}

static inline void
OUTW (u16 val, unsigned int reg)
{
	outw(val, ide_base + reg);
}

static void
dump_regs (void)
{
	printf("ide_data      = 0x%04x\n", INW(ide_data));
	printf("ide_error     = 0x%02x\n", INB(ide_error));
	printf("ide_nsectors  = 0x%02x\n", INB(ide_nsectors));
	printf("ide_lbal      = 0x%02x\n", INB(ide_lbal));
	printf("ide_lbam      = 0x%02x\n", INB(ide_lbam));
	printf("ide_lbah      = 0x%02x\n", INB(ide_lbah));
	printf("ide_select    = 0x%02x\n", INB(ide_select));
	printf("ide_status    = 0x%02x\n", INB(ide_status));
	printf("ide_altstatus = 0x%02x\n", INB(ide_altstatus));
}

static int
wait_for_status (u8 ones, u8 zeros, u8 *stat_r, unsigned int timeout)
{
	u8 stat;
	int result;

	wait_400ns();
	timeout *= 1000000;
	do {
		stat = INB(ide_status);
		result = (stat & (ones|zeros)) != ones;
	} while (result && --timeout);
	if (stat_r)
		*stat_r = stat;
	return result ? EIO : 0;
}

static int
require_status (u8 ones, u8 zeros, u8 *stat_r, unsigned int timeout, const char *msg)
{
	int result = 0;
	u8 stat;

	if (wait_for_status(ones, zeros, &stat, timeout)) {
		fprintf(stderr, "status timeout: %s, stat=0x%02x\n", msg, stat);
		dump_regs();
		result = EIO;
	}
	if (stat_r)
		*stat_r = stat;
	return result;
}

static int
do_drive_select (int master_slave)
{
	if (require_status(READY_STAT, BUSY_STAT|DRQ_STAT, NULL, 1, "do_drive_select1"))
		return EIO;
	OUTB(0xe0 | ((master_slave & 1) << 4), ide_select);
	if (require_status(READY_STAT, BUSY_STAT|DRQ_STAT, NULL, 1, "do_drive_select2"))
		return EIO;
	return 0;
}

static int
do_nondata_command (u8 command)
{
	u8 stat, err;

	if (require_status(READY_STAT, BUSY_STAT|DRQ_STAT, NULL, 1, "do_nondata_command1"))
		return EIO;
	OUTB(command, ide_command);
	if (require_status(READY_STAT, BUSY_STAT|DRQ_STAT, &stat, 40, "do_nondata_command2"))
		return EIO;
	if ((stat & ERR_STAT)) {
		err = INB(ide_error);
		fprintf(stderr, "command 0x%02x failed, status=0x%02x, error=0x%02x\n", command, stat, err);
		dump_regs();
		return EIO;
	}
	printf("command 0x%02x succeeded\n", command);
	return 0;
}

static int
do_setfeatures (u8 subcommand, u8 parameter)
{
	if (require_status(READY_STAT, BUSY_STAT|DRQ_STAT, NULL, 1, "do_setfeatures1"))
		return EIO;
	OUTB(subcommand, ide_feature);
	OUTB(parameter,  ide_nsectors);
	return do_nondata_command(SETFEATURES);
}

static int
do_rw_test (void)
{
	OUTW(0x1234, ide_data);
	wait_400ns();
	if (INW(ide_data) == 0x1234) {
		OUTW(0x4321, ide_data);
		wait_400ns();
		if (INW(ide_data) == 0x4321)
			return 0; /* success */
	}
	fprintf(stderr, "register read/write test failed\n");
	return EIO;
}

static int
do_fix_drive (void)
{
	u8 stat;

	printf("\nTrying with ide_base=0x%x:\n", ide_base);

	ide_control = (ide_base < 0x1000) ? 0x206 : 0x102;
	ide_altstatus = ide_control;

	stat = INB(ide_altstatus);
	if (stat == 0xff || stat == 0x00) {
		fprintf(stderr, "No drive detected, try a different ide_base?\n");
		return ENODEV;
	}

	if ((stat & (BUSY_STAT|DRQ_STAT))) {
		fprintf(stderr, "IDE interface is busy, try again later\n");
		return EAGAIN;
	}
	if (do_drive_select(0))
		return EIO;
	do_rw_test();

	OUTB(0x02, ide_control);		/* set NIEN */
	wait_400ns();

	if (do_setfeatures(SF_SPINUP_NOW, 0))	/* spin-up the drive */
		return EIO;
	if (do_setfeatures(SF_NO_STANDBY, 0))	/* disable powerup-in-standby feature */
		return EIO;
	if (do_nondata_command(CHECKPOWERMODE))	/* clear any leftover errors */
		return EIO;
	dump_regs();

	INB(ide_status);			/* clear IRQ */
	OUTB(0x00, ide_control);		/* reeanble NIEN */
	wait_400ns();
	INB(ide_status);			/* clear IRQ */

	printf("Done; the drive may take another 30 seconds to come to life now.\n");
	return 0;
}

int  main (int argc, char **argv)
{
	int result;

	ide_base = 0x1f0;
	if (argc > 1) {
		char *end = NULL;
		ide_base = strtol(argv[1], &end, 0);
		if (argc > 2 || !ide_base || ide_base > 0xffff || !end || *end) {
			fprintf(stderr, "Bad argument; expected ide_base, Eg:  0x1f0\n");
			return EINVAL;
		}
	}

	sync();
	sleep(2);
	iopl(3);
	cli();

	result = do_fix_drive();

	sti();
	sync();
	return result;
}

