/* This file is used for a trick to determine the version of the kernel
 * build tree. Simply grepping <linux/version.h> doesn't work, since
 * some distributions have multiple UTS_RELEASE definitions in that
 * file.
 * Taken from the lm_sensors project.
 *
 * $Id: kernelversion.c 1669 2006-07-05 02:21:30Z proski $
 */
#include <linux/version.h>

/* Linux 2.6.18+ uses <linux/utsrelease.h> */
#ifndef UTS_RELEASE
#include <linux/utsrelease.h>
#endif

char *uts_release = UTS_RELEASE;
