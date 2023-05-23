#ifndef _COMPRESS_H
#define _COMPRESS_H

#include <fcntl.h>
#include <apfs/types.h>

struct apfs_compress_file_data {
	struct apfs_compress_hdr hdr;
	loff_t size;
	void *data;
	u8 *buf;
	ssize_t bufblk;
	size_t bufsize;
};

struct compress {
	void *decmpfs;		/* Copy of the inline data */
	int decmpfs_len;	/* Length of the inline data */

	u64	size;		/* Size of the compressed data */
	u64	block_num;	/* Reported count of compressed blocks */
	u32	data_offs;
	u32	data_size;

	/* Data stream for the resource fork (NULL if none) */
	struct dstream *rsrc_dstream;

	/* Internal data for the decompression code */
	struct apfs_compress_file_data *compress_data;
};

extern void apfs_compress_open(struct compress *compress);
extern void apfs_compress_check(struct compress *compress);
extern ssize_t apfs_compress_read(struct compress *compress, char *buf, size_t size, loff_t *off);
extern void apfs_compress_close(struct compress *compress);
extern void read_whole_dstream(u64 oid, void *buf, loff_t size);

#endif	/* _COMPRESS_H */
