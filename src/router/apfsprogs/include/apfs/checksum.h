/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#ifndef _CHECKSUM_H
#define _CHECKSUM_H

#include <apfs/types.h>

extern u32 crc32c(u32 crc, const void *buf, int size);
extern u64 fletcher64(void *addr, unsigned long len);

#endif /* _CHECKSUM_H */
