/* make_bad_sector.c v3.03 by Mark Lord */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/fs.h>
#include <scsi/scsi.h>
#include <scsi/sg.h>

#include "hdparm.h"
#include "sgio.h"

#define READ	0
#define WRITE	1

int verbose = 0;	// used by sgio.c

#if 0
static void init_hdio_taskfile (struct hdio_taskfile *r, __u8 ata_op, int rw, int force_lba48,
				__u64 lba, unsigned int nsect, int data_bytes)
{
	const __u64 lba28_mask = 0x0fffffff;

	memset(r, 0, sizeof(struct hdio_taskfile) + data_bytes);
	r->xfer_method	= TASKFILE_XFER_METHOD_PIO_OUT;
	if (!data_bytes) {
		r->cmd_req = TASKFILE_CMD_REQ_NODATA;
	} else {
		r->cmd_req = rw ? TASKFILE_CMD_REQ_OUT : TASKFILE_CMD_REQ_IN;
		if (rw)
			r->out_bytes = data_bytes;
		else
			r->in_bytes  = data_bytes;
	}
	r->lob.command           = ata_op;
	r->out_flags.lob.b.command = 1;
	r->out_flags.lob.b.dev     = 1;
	r->out_flags.lob.b.lbal    = 1;
	r->out_flags.lob.b.lbam    = 1;
	r->out_flags.lob.b.lbah    = 1;
	r->out_flags.lob.b.nsect   = 1;

	r->lob.nsect = nsect;
	r->lob.lbal  = lba;
	r->lob.lbam  = lba >>  8;
	r->lob.lbah  = lba >> 16;
	r->lob.dev   = ATA_USING_LBA;

	if ((lba & ~lba28_mask) == 0 && nsect <= 256 && !force_lba48) {
		r->lob.dev |= lba >> 24;
	} else {
		r->hob.nsect = nsect >>  8;
		r->hob.lbal = lba    >> 24;
		r->hob.lbam = lba    >> 32;
		r->hob.lbah = lba    >> 40;
		r->out_flags.hob.b.nsect = 1;
		r->out_flags.hob.b.lbal  = 1;
		r->out_flags.hob.b.lbam  = 1;
		r->out_flags.hob.b.lbah  = 1;
	}
}
#endif

int main (int argc, char *argv[])
{
	/*
	 * Note: the extra bytes are transfered ONE-PER_WORD after the sector data,
	 * so for 4 extra bytes, we must transfer 4 extra WORDs.
	 */
	const char *devpath, *myname = argv[0];;
	int rc = 0, fd, do_rewrite = 0, do_readback = 0;
	__u8 ata_op;
	__u64 lba;
	unsigned char bad_pattern = 0x00;
	struct hdio_taskfile *r;
	const int ten_seconds = 10;

	r = malloc(sizeof(struct hdio_taskfile) + 520);
	if (!r) {
		perror("malloc()");
		exit(1);
	}

	while (argc-- > 1 && **++argv == '-') {
		if (!strcmp("--readback", argv[0])) {
			do_readback = 1;
		} else if (!strcmp("--rewrite", argv[0])) {
			do_rewrite = 1;
		} else {
			fprintf(stderr, "%s: unknown flag: %s\n", myname, argv[0]);
			exit(1);
		}
	}
	if (argc != 2) {
		fprintf(stderr, "%s: bad/missing parms: expected [--rewrite|--readback] <devpath> <lba>\n", myname);
		exit(1);
	}
	devpath = argv[0];
	lba = strtol(argv[1], NULL, 0);

	fd = open(devpath, O_RDWR);
	if (fd == -1) {
		perror(devpath);
		exit(1);
	}

	// Try and ensure that the system doesn't have our sector in cache:
	(void) ioctl(fd, BLKFLSBUF, NULL);

	if (do_rewrite) {
		fprintf(stderr, "%s: overwriting LBA=%llu (this should succeed!)\n",
			 devpath, (unsigned long long)lba);
		ata_op = (lba >> 28) ? ATA_OP_WRITE_PIO_EXT : ATA_OP_WRITE_PIO;
		init_hdio_taskfile(r, ata_op, WRITE, 0, lba, 1, 512);
		rc = do_taskfile_cmd(fd, r, ten_seconds);
		fprintf(stderr, "%s: %s\n", devpath, rc ? "error" : "success");
	}
	if (rc == 0 || do_readback) {
		fprintf(stderr, "%s: readback test LBA=%llu%s\n",
			 devpath, (unsigned long long)lba,
			 do_rewrite ? " (this should succeed!)" : "");
		ata_op = (lba >> 28) ? ATA_OP_READ_PIO_EXT : ATA_OP_READ_PIO;
		init_hdio_taskfile(r, ata_op, READ, 0, lba, 1, 512);
		rc = do_taskfile_cmd(fd, r, ten_seconds);
		fprintf(stderr, "%s: %s\n", devpath, rc ? "error" : "success");
	}
	if (do_rewrite || do_readback)
		exit(rc);

	do {
		int i;

		init_hdio_taskfile(r, ATA_OP_WRITE_LONG_ONCE, WRITE, 0, lba, 1, 520);
		--bad_pattern;
		// Corrupt and rewrite the sector:
		for (i = 0; i < 520; ++i)
			r->data[i] = bad_pattern;
		fprintf(stderr, "%s: writing LBA=%llu\n", devpath, (unsigned long long)lba);
		rc = do_taskfile_cmd(fd, r, ten_seconds);
		if (rc) {
			fprintf(stderr, "%s: WRITE_LONG failed\n", devpath);
		} else {
			fprintf(stderr, "%s: readback test LBA=%llu (this should fail!)\n",
				 devpath, (unsigned long long)lba);
			//init_hdio_taskfile(r, ATA_OP_READ_VERIFY_ONCE, READ, 0, lba, 1, 0);
			init_hdio_taskfile(r, ATA_OP_READ_PIO_EXT, READ, 1, lba, 1, 512);
			rc = do_taskfile_cmd(fd, r, ten_seconds);
			if (rc)
				fprintf(stderr, "%s: readback failed\n", devpath);
		}
	} while (rc == 0);
	exit (0);
}
