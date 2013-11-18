/* $Id: report.c 3666 2006-10-26 23:11:51Z ckuethe $ */
#include <sys/types.h>
#include <stdio.h>
#include <stdarg.h>
#include "gpsd_config.h"
#include "gpsd.h"

void gpsd_report(int errlevel UNUSED, const char *fmt, ... )
/* stub logger for clients that don't supply one */
{
    va_list ap;

    va_start(ap, fmt);
    (void)vfprintf(stderr, fmt, ap);
    va_end(ap);
}

