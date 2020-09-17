// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */
#ifndef __LIBFROG_CONVERT_H__
#define __LIBFROG_CONVERT_H__

extern int64_t	cvt_s64(char *s, int base);
extern int32_t	cvt_s32(char *s, int base);
extern int16_t	cvt_s16(char *s, int base);

extern uint64_t	cvt_u64(char *s, int base);
extern uint32_t	cvt_u32(char *s, int base);
extern uint16_t	cvt_u16(char *s, int base);

extern long long cvtnum(size_t blocksize, size_t sectorsize, const char *s);
extern void cvtstr(double value, char *str, size_t sz);
extern unsigned long cvttime(char *s);

extern uid_t	uid_from_string(char *user);
extern gid_t	gid_from_string(char *group);
extern prid_t	prid_from_string(char *project);

#endif	/* __LIBFROG_CONVERT_H__ */
