// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

typedef int (*prfnc_t)(void *obj, int bit, int count, char *fmtstr, int size,
		       int arg, int base, int array);

extern int	fp_charns(void *obj, int bit, int count, char *fmtstr, int size,
			  int arg, int base, int array);
extern int	fp_num(void *obj, int bit, int count, char *fmtstr, int size,
		       int arg, int base, int array);
extern int	fp_sarray(void *obj, int bit, int count, char *fmtstr, int size,
			  int arg, int base, int array);
extern int	fp_time(void *obj, int bit, int count, char *fmtstr, int size,
			int arg, int base, int array);
extern int	fp_uuid(void *obj, int bit, int count, char *fmtstr, int size,
			int arg, int base, int array);
extern int	fp_crc(void *obj, int bit, int count, char *fmtstr, int size,
		       int arg, int base, int array);
