// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  Copyright (C) 2012-2013 Samsung Electronics Co., Ltd.
 *
 *  misc.c: Helper function for checksum and handling exFAT errors
 */

/*
 *  linux/fs/fat/misc.c
 *
 *  Written 1992,1993 by Werner Almesberger
 *  22/11/2000 - Fixed fat_date_unix2dos for dates earlier than 01/01/1980
 *		 and date_dos2unix for date==0 by Igor Zhbanov(bsg@uniyar.ac.ru)
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/time.h>
#include "exfat.h"
#include "version.h"

#ifdef CONFIG_EXFAT_UEVENT
static struct kobject exfat_uevent_kobj;

int exfat_uevent_init(struct kset *exfat_kset)
{
	int err;
	struct kobj_type *ktype = get_ktype(&exfat_kset->kobj);

	exfat_uevent_kobj.kset = exfat_kset;
	err = kobject_init_and_add(&exfat_uevent_kobj, ktype, NULL, "uevent");
	if (err)
		pr_err("[EXFAT] Unable to create exfat uevent kobj\n");

	return err;
}

void exfat_uevent_uninit(void)
{
	kobject_del(&exfat_uevent_kobj);
	memset(&exfat_uevent_kobj, 0, sizeof(struct kobject));
}

void exfat_uevent_ro_remount(struct super_block *sb)
{
	struct block_device *bdev = sb->s_bdev;
	dev_t bd_dev = bdev ? bdev->bd_dev : 0;

	char major[16], minor[16];
	char *envp[] = { major, minor, NULL };

	snprintf(major, sizeof(major), "MAJOR=%d", MAJOR(bd_dev));
	snprintf(minor, sizeof(minor), "MINOR=%d", MINOR(bd_dev));

	kobject_uevent_env(&exfat_uevent_kobj, KOBJ_CHANGE, envp);
}
#endif

/*
 * exfat_fs_error reports a file system problem that might indicate fa data
 * corruption/inconsistency. Depending on 'errors' mount option the
 * panic() is called, or error message is printed FAT and nothing is done,
 * or filesystem is remounted read-only (default behavior).
 * In case the file system is remounted read-only, it can be made writable
 * again by remounting it.
 */
void __exfat_fs_error(struct super_block *sb, int report, const char *fmt, ...)
{
	struct exfat_mount_options *opts = &EXFAT_SB(sb)->options;
	va_list args;
	struct va_format vaf;
	struct block_device *bdev = sb->s_bdev;
	dev_t bd_dev = bdev ? bdev->bd_dev : 0;

	if (report) {
		va_start(args, fmt);
		vaf.fmt = fmt;
		vaf.va = &args;
		pr_err("exFAT-fs (%s[%d:%d]): ERR: %pV\n",
			sb->s_id, MAJOR(bd_dev), MINOR(bd_dev), &vaf);
		va_end(args);
	}

	if (opts->errors == EXFAT_ERRORS_PANIC) {
		panic("exFAT-fs (%s[%d:%d]): fs panic from previous error\n",
			sb->s_id, MAJOR(bd_dev), MINOR(bd_dev));
	} else if (opts->errors == EXFAT_ERRORS_RO && !EXFAT_IS_SB_RDONLY(sb)) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
		sb->s_flags |= MS_RDONLY;
#else
		sb->s_flags |= SB_RDONLY;
#endif
		pr_err("exFAT-fs (%s[%d:%d]): file-system has been set to "
			"read-only\n", sb->s_id, MAJOR(bd_dev), MINOR(bd_dev));
		exfat_uevent_ro_remount(sb);
	}
}
EXPORT_SYMBOL(__exfat_fs_error);

/**
 * __exfat_msg() - print preformated EXFAT specific messages.
 * All logs except what uses exfat_fs_error() should be written by __exfat_msg()
 */
void __exfat_msg(struct super_block *sb, const char *level, int st, const char *fmt, ...)
{
	struct va_format vaf;
	va_list args;
	struct block_device *bdev = sb->s_bdev;
	dev_t bd_dev = bdev ? bdev->bd_dev : 0;

	va_start(args, fmt);
	vaf.fmt = fmt;
	vaf.va = &args;
	/* level means KERN_ pacility level */
	printk("%sexFAT-fs (%s[%d:%d]): %pV\n", level,
			sb->s_id, MAJOR(bd_dev), MINOR(bd_dev), &vaf);
	va_end(args);
}
EXPORT_SYMBOL(__exfat_msg);

void exfat_log_version(void)
{
	pr_info("exFAT: file-system version %s\n", EXFAT_VERSION);
}
EXPORT_SYMBOL(exfat_log_version);

/* <linux/time.h> externs sys_tz
 * extern struct timezone sys_tz;
 */
#define UNIX_SECS_1980    315532800L

#if BITS_PER_LONG == 64
#define UNIX_SECS_2108    4354819200L
#endif

/* days between 1970/01/01 and 1980/01/01 (2 leap days) */
#define DAYS_DELTA_DECADE    (365 * 10 + 2)
/* 120 (2100 - 1980) isn't leap year */
#define NO_LEAP_YEAR_2100    (120)
#define IS_LEAP_YEAR(y)    (!((y) & 0x3) && (y) != NO_LEAP_YEAR_2100)

#define SECS_PER_MIN    (60)
#define SECS_PER_HOUR   (60 * SECS_PER_MIN)
#define SECS_PER_DAY    (24 * SECS_PER_HOUR)

#define MAKE_LEAP_YEAR(leap_year, year)                         \
	do {                                                    \
		/* 2100 isn't leap year */                      \
		if (unlikely(year > NO_LEAP_YEAR_2100))         \
			leap_year = ((year + 3) / 4) - 1;       \
		else                                            \
			leap_year = ((year + 3) / 4);           \
	} while (0)

/* Linear day numbers of the respective 1sts in non-leap years. */
static time_t accum_days_in_year[] = {
	/* Month : N 01  02  03  04  05  06  07  08  09  10  11  12 */
	0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 0, 0, 0,
};

/* Convert a FAT time/date pair to a UNIX date (seconds since 1 1 70). */
void exfat_time_fat2unix(struct exfat_sb_info *sbi, struct timespec_compat *ts,
		DATE_TIME_T *tp)
{
	time_t year = tp->Year;
	time_t ld; /* leap day */

	MAKE_LEAP_YEAR(ld, year);

	if (IS_LEAP_YEAR(year) && (tp->Month) > 2)
		ld++;

	ts->tv_sec =  tp->Second  + tp->Minute * SECS_PER_MIN
			+ tp->Hour * SECS_PER_HOUR
			+ (year * 365 + ld + accum_days_in_year[tp->Month]
			+ (tp->Day - 1) + DAYS_DELTA_DECADE) * SECS_PER_DAY;

	if (!sbi->options.tz_utc)
		ts->tv_sec += sys_tz.tz_minuteswest * SECS_PER_MIN;

	ts->tv_nsec = 0;
}

/* Convert linear UNIX date to a FAT time/date pair. */
void exfat_time_unix2fat(struct exfat_sb_info *sbi, struct timespec_compat *ts,
		DATE_TIME_T *tp)
{
	time_t second = ts->tv_sec;
	time_t day, month, year;
	time_t ld; /* leap day */

	if (!sbi->options.tz_utc)
		second -= sys_tz.tz_minuteswest * SECS_PER_MIN;

	/* Jan 1 GMT 00:00:00 1980. But what about another time zone? */
	if (second < UNIX_SECS_1980) {
		tp->Second  = 0;
		tp->Minute  = 0;
		tp->Hour = 0;
		tp->Day  = 1;
		tp->Month  = 1;
		tp->Year = 0;
		return;
	}
#if (BITS_PER_LONG == 64)
	if (second >= UNIX_SECS_2108) {
		tp->Second  = 59;
		tp->Minute  = 59;
		tp->Hour = 23;
		tp->Day  = 31;
		tp->Month  = 12;
		tp->Year = 127;
		return;
	}
#endif

	day = second / SECS_PER_DAY - DAYS_DELTA_DECADE;
	year = day / 365;

	MAKE_LEAP_YEAR(ld, year);
	if (year * 365 + ld > day)
		year--;

	MAKE_LEAP_YEAR(ld, year);
	day -= year * 365 + ld;

	if (IS_LEAP_YEAR(year) && day == accum_days_in_year[3]) {
		month = 2;
	} else {
		if (IS_LEAP_YEAR(year) && day > accum_days_in_year[3])
			day--;
		for (month = 1; month < 12; month++) {
			if (accum_days_in_year[month + 1] > day)
				break;
		}
	}
	day -= accum_days_in_year[month];

	tp->Second  = second % SECS_PER_MIN;
	tp->Minute  = (second / SECS_PER_MIN) % 60;
	tp->Hour = (second / SECS_PER_HOUR) % 24;
	tp->Day  = day + 1;
	tp->Month  = month;
	tp->Year = year;
}

TIMESTAMP_T *exfat_tm_now(struct exfat_sb_info *sbi, TIMESTAMP_T *tp)
{
	struct timespec_compat ts;
	DATE_TIME_T dt;

	KTIME_GET_REAL_TS(&ts);
	exfat_time_unix2fat(sbi, &ts, &dt);

	tp->year = dt.Year;
	tp->mon = dt.Month;
	tp->day = dt.Day;
	tp->hour = dt.Hour;
	tp->min = dt.Minute;
	tp->sec = dt.Second;

	return tp;
}

u16 exfat_calc_chksum_2byte(void *data, s32 len, u16 chksum, s32 type)
{
	s32 i;
	u8 *c = (u8 *) data;

	for (i = 0; i < len; i++, c++) {
		if (((i == 2) || (i == 3)) && (type == CS_DIR_ENTRY))
			continue;
		chksum = (((chksum & 1) << 15) | ((chksum & 0xFFFE) >> 1)) + (u16) *c;
	}
	return chksum;
}

#ifdef CONFIG_EXFAT_DBG_MSG
void __exfat_dmsg(int level, const char *fmt, ...)
{
	va_list args;

	/* should check type */
	if (level > EXFAT_MSG_LEVEL)
		return;

	va_start(args, fmt);
	/* fmt already includes KERN_ pacility level */
	vprintk(fmt, args);
	va_end(args);
}
#endif
