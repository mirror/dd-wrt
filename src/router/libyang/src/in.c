/**
 * @file in.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang input functions.
 *
 * Copyright (c) 2015 - 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L /* strdup, strndup */

#include "in.h"
#include "in_internal.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "compat.h"
#include "dict.h"
#include "log.h"
#include "ly_common.h"
#include "parser_data.h"
#include "parser_internal.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"

LIBYANG_API_DEF LY_IN_TYPE
ly_in_type(const struct ly_in *in)
{
    LY_CHECK_ARG_RET(NULL, in, LY_IN_ERROR);
    return in->type;
}

LIBYANG_API_DEF LY_ERR
ly_in_reset(struct ly_in *in)
{
    LY_CHECK_ARG_RET(NULL, in, LY_EINVAL);

    in->current = in->func_start = in->start;
    in->line = 1;
    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
ly_in_new_fd(int fd, struct ly_in **in)
{
    size_t length;
    char *addr;

    LY_CHECK_ARG_RET(NULL, fd >= 0, in, LY_EINVAL);

    LY_CHECK_RET(ly_mmap(NULL, fd, &length, (void **)&addr));
    if (!addr) {
        LOGERR(NULL, LY_EINVAL, "Empty input file.");
        return LY_EINVAL;
    }

    *in = calloc(1, sizeof **in);
    LY_CHECK_ERR_RET(!*in, LOGMEM(NULL); ly_munmap(addr, length), LY_EMEM);

    (*in)->type = LY_IN_FD;
    (*in)->method.fd = fd;
    (*in)->current = (*in)->start = (*in)->func_start = addr;
    (*in)->line = 1;
    (*in)->length = length;

    return LY_SUCCESS;
}

LIBYANG_API_DEF int
ly_in_fd(struct ly_in *in, int fd)
{
    int prev_fd;
    size_t length;
    const char *addr;

    LY_CHECK_ARG_RET(NULL, in, in->type == LY_IN_FD, -1);

    prev_fd = in->method.fd;

    if (fd != -1) {
        LY_CHECK_RET(ly_mmap(NULL, fd, &length, (void **)&addr), -1);
        if (!addr) {
            LOGERR(NULL, LY_EINVAL, "Empty input file.");
            return -1;
        }

        ly_munmap((char *)in->start, in->length);

        in->method.fd = fd;
        in->current = in->start = addr;
        in->line = 1;
        in->length = length;
    }

    return prev_fd;
}

LIBYANG_API_DEF LY_ERR
ly_in_new_file(FILE *f, struct ly_in **in)
{
    LY_CHECK_ARG_RET(NULL, f, in, LY_EINVAL);

    LY_CHECK_RET(ly_in_new_fd(fileno(f), in));

    /* convert the LY_IN_FD input handler into the LY_IN_FILE */
    (*in)->type = LY_IN_FILE;
    (*in)->method.f = f;

    return LY_SUCCESS;
}

LIBYANG_API_DEF FILE *
ly_in_file(struct ly_in *in, FILE *f)
{
    FILE *prev_f;

    LY_CHECK_ARG_RET(NULL, in, in->type == LY_IN_FILE, NULL);

    prev_f = in->method.f;

    if (f) {
        /* convert LY_IN_FILE handler into LY_IN_FD to be able to update it via ly_in_fd() */
        in->type = LY_IN_FD;
        in->method.fd = fileno(prev_f);
        if (ly_in_fd(in, fileno(f)) == -1) {
            in->type = LY_IN_FILE;
            in->method.f = prev_f;
            return NULL;
        }

        /* if success, convert the result back */
        in->type = LY_IN_FILE;
        in->method.f = f;
    }

    return prev_f;
}

LIBYANG_API_DEF LY_ERR
ly_in_new_memory(const char *str, struct ly_in **in)
{
    LY_CHECK_ARG_RET(NULL, str, in, LY_EINVAL);

    *in = calloc(1, sizeof **in);
    LY_CHECK_ERR_RET(!*in, LOGMEM(NULL), LY_EMEM);

    (*in)->type = LY_IN_MEMORY;
    (*in)->start = (*in)->current = (*in)->func_start = str;
    (*in)->line = 1;

    return LY_SUCCESS;
}

LIBYANG_API_DEF const char *
ly_in_memory(struct ly_in *in, const char *str)
{
    const char *data;

    LY_CHECK_ARG_RET(NULL, in, in->type == LY_IN_MEMORY, NULL);

    data = in->current;

    if (str) {
        in->start = in->current = str;
        in->line = 1;
    }

    return data;
}

LIBYANG_API_DEF LY_ERR
ly_in_new_filepath(const char *filepath, size_t len, struct ly_in **in)
{
    LY_ERR ret;
    char *fp;
    int fd;

    LY_CHECK_ARG_RET(NULL, filepath, in, LY_EINVAL);

    if (len) {
        fp = strndup(filepath, len);
    } else {
        fp = strdup(filepath);
    }

    fd = open(fp, O_RDONLY);
    LY_CHECK_ERR_RET(fd == -1, LOGERR(NULL, LY_ESYS, "Failed to open file \"%s\" (%s).", fp, strerror(errno)); free(fp),
            LY_ESYS);

    LY_CHECK_ERR_RET(ret = ly_in_new_fd(fd, in), free(fp), ret);

    /* convert the LY_IN_FD input handler into the LY_IN_FILE */
    (*in)->type = LY_IN_FILEPATH;
    (*in)->method.fpath.fd = fd;
    (*in)->method.fpath.filepath = fp;

    return LY_SUCCESS;
}

LIBYANG_API_DEF const char *
ly_in_filepath(struct ly_in *in, const char *filepath, size_t len)
{
    int fd, prev_fd;
    char *fp = NULL;

    LY_CHECK_ARG_RET(NULL, in, in->type == LY_IN_FILEPATH, filepath ? NULL : ((void *)-1));

    if (!filepath) {
        return in->method.fpath.filepath;
    }

    if (len) {
        fp = strndup(filepath, len);
    } else {
        fp = strdup(filepath);
    }

    /* replace filepath */
    fd = open(fp, O_RDONLY);
    LY_CHECK_ERR_RET(!fd, LOGERR(NULL, LY_ESYS, "Failed to open file \"%s\" (%s).", fp, strerror(errno)); free(fp), NULL);

    /* convert LY_IN_FILEPATH handler into LY_IN_FD to be able to update it via ly_in_fd() */
    in->type = LY_IN_FD;
    prev_fd = ly_in_fd(in, fd);
    LY_CHECK_ERR_RET(prev_fd == -1, in->type = LY_IN_FILEPATH; free(fp), NULL);

    /* and convert the result back */
    in->type = LY_IN_FILEPATH;
    close(prev_fd);
    free(in->method.fpath.filepath);
    in->method.fpath.fd = fd;
    in->method.fpath.filepath = fp;

    return NULL;
}

LIBYANG_API_DEF size_t
ly_in_parsed(const struct ly_in *in)
{
    LY_CHECK_ARG_RET(NULL, in, 0);

    return in->current - in->func_start;
}

LIBYANG_API_DEF void
ly_in_free(struct ly_in *in, ly_bool destroy)
{
    if (!in) {
        return;
    } else if (in->type == LY_IN_ERROR) {
        LOGINT(NULL);
        return;
    }

    if (destroy) {
        if (in->type == LY_IN_MEMORY) {
            free((char *)in->start);
        } else {
            ly_munmap((char *)in->start, in->length);

            if (in->type == LY_IN_FILE) {
                fclose(in->method.f);
            } else {
                close(in->method.fd);

                if (in->type == LY_IN_FILEPATH) {
                    free(in->method.fpath.filepath);
                }
            }
        }
    } else if (in->type != LY_IN_MEMORY) {
        ly_munmap((char *)in->start, in->length);

        if (in->type == LY_IN_FILEPATH) {
            close(in->method.fpath.fd);
            free(in->method.fpath.filepath);
        }
    }

    free(in);
}

LIBYANG_API_DEF LY_ERR
ly_in_read(struct ly_in *in, void *buf, size_t count)
{
    LY_CHECK_ARG_RET(NULL, in, buf, LY_EINVAL);

    if (in->length && (in->length - (in->current - in->start) < count)) {
        /* EOF */
        return LY_EDENIED;
    }

    if (count) {
        memcpy(buf, in->current, count);
    }
    in->current += count;
    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
ly_in_skip(struct ly_in *in, size_t count)
{
    LY_CHECK_ARG_RET(NULL, in, LY_EINVAL);

    if (in->length && (in->length - (in->current - in->start) < count)) {
        /* EOF */
        return LY_EDENIED;
    }

    in->current += count;
    return LY_SUCCESS;
}
