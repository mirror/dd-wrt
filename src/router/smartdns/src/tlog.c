#ifdef NEED_PRINTF
#include <stdio.h>
#include "tlog.h"
/*
 * tinylog
 * Copyright (C) 2018-2024 Nick Peng <pymumu@gmail.com>
 * https://github.com/pymumu/tinylog
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
static int tlog_set_level = TLOG_INFO;

int tlog_setlevel(tlog_level level)
{
    if (level >= TLOG_END) {
        return -1;
    }

    tlog_set_level = level;
    return 0;
}


tlog_level tlog_getlevel(void)
{
    return tlog_set_level;
}

#endif