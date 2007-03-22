
/*
 * getpath.c  - directory search routines for configuration and support
 *                files.
 *
 *      Contributed by Stefan Luethje <luethje@sl-gw.lake.de>
 *
 *      IPTraf 1.4.0 Copyright (c) Gerard Paul Java 1997, 1998
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include "dirs.h"

char *get_path(int dirtype, char *file)
{
    static char path[PATH_MAX];
    char *ptr = NULL;
    char *dir, *env = NULL;

    switch (dirtype) {
    case T_WORKDIR:
        dir = WORKDIR;
        env = WORKDIR_ENV;
        break;
    case T_LOGDIR:
        dir = LOGDIR;
        env = LOGDIR_ENV;
        break;
    case T_EXECDIR:
        dir = EXECDIR;
        break;
    case T_LOCKDIR:
        dir = LOCKDIR;
        break;
    default:
        return file;
    }

    if ((dirtype != T_EXECDIR) && (dirtype != T_LOCKDIR) &&
        (ptr = getenv(env)) != NULL)
        dir = ptr;

    if (dir == NULL || *dir == '\0')
        return file;

    snprintf(path, PATH_MAX - 1, "%s/%s", dir, file);

    return path;
}
