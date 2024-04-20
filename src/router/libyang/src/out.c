/**
 * @file out.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang output functions.
 *
 * Copyright (c) 2015 - 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE /* asprintf, strdup */
#include <sys/cdefs.h>

#include "out.h"
#include "out_internal.h"

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "compat.h"
#include "log.h"
#include "printer_data.h"
#include "tree_data.h"
#include "tree_schema.h"

ly_bool
ly_should_print(const struct lyd_node *node, uint32_t options)
{
    const struct lyd_node *elem;

    if (options & LYD_PRINT_WD_TRIM) {
        /* do not print default nodes */
        if (node->flags & LYD_DEFAULT) {
            /* implicit default node/NP container with only default nodes */
            return 0;
        } else if (node->schema->nodetype & LYD_NODE_TERM) {
            if (lyd_is_default(node)) {
                /* explicit default node */
                return 0;
            }
        }
    } else if ((node->flags & LYD_DEFAULT) && !(options & LYD_PRINT_WD_MASK) && !(node->schema->flags & LYS_CONFIG_R)) {
        /* LYD_PRINT_WD_EXPLICIT, find out if this is some input/output */
        if (!(node->schema->flags & (LYS_IS_INPUT | LYS_IS_OUTPUT | LYS_IS_NOTIF)) && (node->schema->flags & LYS_CONFIG_W)) {
            /* print only if it contains status data in its subtree */
            LYD_TREE_DFS_BEGIN(node, elem) {
                if ((elem->schema->nodetype != LYS_CONTAINER) || (elem->schema->flags & LYS_PRESENCE)) {
                    if (elem->schema->flags & LYS_CONFIG_R) {
                        return 1;
                    }
                }
                LYD_TREE_DFS_END(node, elem)
            }
        }
        return 0;
    } else if ((node->flags & LYD_DEFAULT) && (node->schema->nodetype == LYS_CONTAINER) && !(options & LYD_PRINT_KEEPEMPTYCONT)) {
        /* avoid empty default containers */
        LYD_TREE_DFS_BEGIN(node, elem) {
            if (elem->schema->nodetype != LYS_CONTAINER) {
                return 1;
            }
            assert(elem->flags & LYD_DEFAULT);
            LYD_TREE_DFS_END(node, elem)
        }
        return 0;
    }

    return 1;
}

API LY_OUT_TYPE
ly_out_type(const struct ly_out *out)
{
    LY_CHECK_ARG_RET(NULL, out, LY_OUT_ERROR);
    return out->type;
}

API LY_ERR
ly_out_new_clb(ly_write_clb writeclb, void *user_data, struct ly_out **out)
{
    LY_CHECK_ARG_RET(NULL, out, writeclb, LY_EINVAL);

    *out = calloc(1, sizeof **out);
    LY_CHECK_ERR_RET(!*out, LOGMEM(NULL), LY_EMEM);

    (*out)->type = LY_OUT_CALLBACK;
    (*out)->method.clb.func = writeclb;
    (*out)->method.clb.arg = user_data;

    return LY_SUCCESS;
}

API ly_write_clb
ly_out_clb(struct ly_out *out, ly_write_clb writeclb)
{
    void *prev_clb;

    LY_CHECK_ARG_RET(NULL, out, out->type == LY_OUT_CALLBACK, NULL);

    prev_clb = out->method.clb.func;

    if (writeclb) {
        out->method.clb.func = writeclb;
    }

    return prev_clb;
}

API void *
ly_out_clb_arg(struct ly_out *out, void *arg)
{
    void *prev_arg;

    LY_CHECK_ARG_RET(NULL, out, out->type == LY_OUT_CALLBACK, NULL);

    prev_arg = out->method.clb.arg;

    if (arg) {
        out->method.clb.arg = arg;
    }

    return prev_arg;
}

API LY_ERR
ly_out_new_fd(int fd, struct ly_out **out)
{
    LY_CHECK_ARG_RET(NULL, out, fd != -1, LY_EINVAL);

    *out = calloc(1, sizeof **out);
    LY_CHECK_ERR_RET(!*out, LOGMEM(NULL), LY_EMEM);
    (*out)->type = LY_OUT_FD;
    (*out)->method.fd = fd;

    return LY_SUCCESS;
}

API int
ly_out_fd(struct ly_out *out, int fd)
{
    int prev_fd;

    LY_CHECK_ARG_RET(NULL, out, out->type <= LY_OUT_FDSTREAM, -1);

    if (out->type == LY_OUT_FDSTREAM) {
        prev_fd = out->method.fdstream.fd;
    } else { /* LY_OUT_FD */
        prev_fd = out->method.fd;
    }

    if (fd != -1) {
        /* replace output stream */
        if (out->type == LY_OUT_FDSTREAM) {
            int streamfd;
            FILE *stream;

            streamfd = dup(fd);
            if (streamfd < 0) {
                LOGERR(NULL, LY_ESYS, "Unable to duplicate provided file descriptor (%d) for printing the output (%s).", fd, strerror(errno));
                return -1;
            }
            stream = fdopen(streamfd, "a");
            if (!stream) {
                LOGERR(NULL, LY_ESYS, "Unable to open provided file descriptor (%d) for printing the output (%s).", fd, strerror(errno));
                close(streamfd);
                return -1;
            }
            /* close only the internally created stream, file descriptor is returned and supposed to be closed by the caller */
            fclose(out->method.fdstream.f);
            out->method.fdstream.f = stream;
            out->method.fdstream.fd = streamfd;
        } else { /* LY_OUT_FD */
            out->method.fd = fd;
        }
    }

    return prev_fd;
}

API LY_ERR
ly_out_new_file(FILE *f, struct ly_out **out)
{
    LY_CHECK_ARG_RET(NULL, out, f, LY_EINVAL);

    *out = calloc(1, sizeof **out);
    LY_CHECK_ERR_RET(!*out, LOGMEM(NULL), LY_EMEM);

    (*out)->type = LY_OUT_FILE;
    (*out)->method.f = f;

    return LY_SUCCESS;
}

API FILE *
ly_out_file(struct ly_out *out, FILE *f)
{
    FILE *prev_f;

    LY_CHECK_ARG_RET(NULL, out, out->type == LY_OUT_FILE, NULL);

    prev_f = out->method.f;

    if (f) {
        out->method.f = f;
    }

    return prev_f;
}

API LY_ERR
ly_out_new_memory(char **strp, size_t size, struct ly_out **out)
{
    LY_CHECK_ARG_RET(NULL, out, strp, LY_EINVAL);

    *out = calloc(1, sizeof **out);
    LY_CHECK_ERR_RET(!*out, LOGMEM(NULL), LY_EMEM);

    (*out)->type = LY_OUT_MEMORY;
    (*out)->method.mem.buf = strp;
    if (!size) {
        /* buffer is supposed to be allocated */
        *strp = NULL;
    } else if (*strp) {
        /* there is already buffer to use */
        (*out)->method.mem.size = size;
    }

    return LY_SUCCESS;
}

char *
ly_out_memory(struct ly_out *out, char **strp, size_t size)
{
    char *data;

    LY_CHECK_ARG_RET(NULL, out, out->type == LY_OUT_MEMORY, NULL);

    data = *out->method.mem.buf;

    if (strp) {
        out->method.mem.buf = strp;
        out->method.mem.len = out->method.mem.size = 0;
        out->printed = 0;
        if (!size) {
            /* buffer is supposed to be allocated */
            *strp = NULL;
        } else if (*strp) {
            /* there is already buffer to use */
            out->method.mem.size = size;
        }
    }

    return data;
}

API LY_ERR
ly_out_reset(struct ly_out *out)
{
    LY_CHECK_ARG_RET(NULL, out, LY_EINVAL);

    switch (out->type) {
    case LY_OUT_ERROR:
        LOGINT(NULL);
        return LY_EINT;
    case LY_OUT_FD:
        if ((lseek(out->method.fd, 0, SEEK_SET) == -1) && (errno != ESPIPE)) {
            LOGERR(NULL, LY_ESYS, "Seeking output file descriptor failed (%s).", strerror(errno));
            return LY_ESYS;
        }
        if ((errno != ESPIPE) && (ftruncate(out->method.fd, 0) == -1)) {
            LOGERR(NULL, LY_ESYS, "Truncating output file failed (%s).", strerror(errno));
            return LY_ESYS;
        }
        break;
    case LY_OUT_FDSTREAM:
    case LY_OUT_FILE:
    case LY_OUT_FILEPATH:
        if ((fseek(out->method.f, 0, SEEK_SET) == -1) && (errno != ESPIPE)) {
            LOGERR(NULL, LY_ESYS, "Seeking output file stream failed (%s).", strerror(errno));
            return LY_ESYS;
        }
        if ((errno != ESPIPE) && (ftruncate(fileno(out->method.f), 0) == -1)) {
            LOGERR(NULL, LY_ESYS, "Truncating output file failed (%s).", strerror(errno));
            return LY_ESYS;
        }
        break;
    case LY_OUT_MEMORY:
        if (out->method.mem.buf && *out->method.mem.buf) {
            memset(*out->method.mem.buf, 0, out->method.mem.len);
        }
        out->printed = 0;
        out->method.mem.len = 0;
        break;
    case LY_OUT_CALLBACK:
        /* nothing to do (not seekable) */
        break;
    }

    return LY_SUCCESS;
}

API LY_ERR
ly_out_new_filepath(const char *filepath, struct ly_out **out)
{
    LY_CHECK_ARG_RET(NULL, out, filepath, LY_EINVAL);

    *out = calloc(1, sizeof **out);
    LY_CHECK_ERR_RET(!*out, LOGMEM(NULL), LY_EMEM);

    (*out)->type = LY_OUT_FILEPATH;
    (*out)->method.fpath.f = fopen(filepath, "w");
    if (!(*out)->method.fpath.f) {
        LOGERR(NULL, LY_ESYS, "Failed to open file \"%s\" (%s).", filepath, strerror(errno));
        free(*out);
        *out = NULL;
        return LY_ESYS;
    }
    (*out)->method.fpath.filepath = strdup(filepath);
    return LY_SUCCESS;
}

API const char *
ly_out_filepath(struct ly_out *out, const char *filepath)
{
    FILE *f;

    LY_CHECK_ARG_RET(NULL, out, out->type == LY_OUT_FILEPATH, filepath ? NULL : ((void *)-1));

    if (!filepath) {
        return out->method.fpath.filepath;
    }

    /* replace filepath */
    f = out->method.fpath.f;
    out->method.fpath.f = fopen(filepath, "w");
    if (!out->method.fpath.f) {
        LOGERR(NULL, LY_ESYS, "Failed to open file \"%s\" (%s).", filepath, strerror(errno));
        out->method.fpath.f = f;
        return (void *)-1;
    }
    fclose(f);
    free(out->method.fpath.filepath);
    out->method.fpath.filepath = strdup(filepath);

    return NULL;
}

API void
ly_out_free(struct ly_out *out, void (*clb_arg_destructor)(void *arg), ly_bool destroy)
{
    if (!out) {
        return;
    }

    switch (out->type) {
    case LY_OUT_CALLBACK:
        if (clb_arg_destructor) {
            clb_arg_destructor(out->method.clb.arg);
        }
        break;
    case LY_OUT_FDSTREAM:
        fclose(out->method.fdstream.f);
        if (destroy) {
            close(out->method.fdstream.fd);
        }
        break;
    case LY_OUT_FD:
        if (destroy) {
            close(out->method.fd);
        }
        break;
    case LY_OUT_FILE:
        if (destroy) {
            fclose(out->method.f);
        }
        break;
    case LY_OUT_MEMORY:
        if (destroy) {
            free(*out->method.mem.buf);
        }
        break;
    case LY_OUT_FILEPATH:
        free(out->method.fpath.filepath);
        fclose(out->method.fpath.f);
        break;
    case LY_OUT_ERROR:
        LOGINT(NULL);
    }

    free(out->buffered);
    free(out);
}

static LY_ERR
ly_vprint_(struct ly_out *out, const char *format, va_list ap)
{
    LY_ERR ret;
    int written = 0;
    char *msg = NULL, *aux;

    switch (out->type) {
    case LY_OUT_FD:
        written = vdprintf(out->method.fd, format, ap);
        break;
    case LY_OUT_FDSTREAM:
    case LY_OUT_FILEPATH:
    case LY_OUT_FILE:
        written = vfprintf(out->method.f, format, ap);
        break;
    case LY_OUT_MEMORY:
        if ((written = vasprintf(&msg, format, ap)) < 0) {
            break;
        }
        if (out->method.mem.len + written + 1 > out->method.mem.size) {
            aux = ly_realloc(*out->method.mem.buf, out->method.mem.len + written + 1);
            if (!aux) {
                out->method.mem.buf = NULL;
                out->method.mem.len = 0;
                out->method.mem.size = 0;
                LOGMEM(NULL);
                return LY_EMEM;
            }
            *out->method.mem.buf = aux;
            out->method.mem.size = out->method.mem.len + written + 1;
        }
        memcpy(&(*out->method.mem.buf)[out->method.mem.len], msg, written);
        out->method.mem.len += written;
        (*out->method.mem.buf)[out->method.mem.len] = '\0';
        free(msg);
        break;
    case LY_OUT_CALLBACK:
        if ((written = vasprintf(&msg, format, ap)) < 0) {
            break;
        }
        written = out->method.clb.func(out->method.clb.arg, msg, written);
        free(msg);
        break;
    case LY_OUT_ERROR:
        LOGINT(NULL);
        return LY_EINT;
    }

    if (written < 0) {
        LOGERR(NULL, LY_ESYS, "%s: writing data failed (%s).", __func__, strerror(errno));
        written = 0;
        ret = LY_ESYS;
    } else {
        if (out->type == LY_OUT_FDSTREAM) {
            /* move the original file descriptor to the end of the output file */
            lseek(out->method.fdstream.fd, 0, SEEK_END);
        }
        ret = LY_SUCCESS;
    }

    out->printed += written;
    out->func_printed += written;
    return ret;
}

LY_ERR
ly_print_(struct ly_out *out, const char *format, ...)
{
    LY_ERR ret;
    va_list ap;

    va_start(ap, format);
    ret = ly_vprint_(out, format, ap);
    va_end(ap);

    return ret;
}

API LY_ERR
ly_print(struct ly_out *out, const char *format, ...)
{
    LY_ERR ret;
    va_list ap;

    out->func_printed = 0;

    va_start(ap, format);
    ret = ly_vprint_(out, format, ap);
    va_end(ap);

    return ret;
}

API void
ly_print_flush(struct ly_out *out)
{
    switch (out->type) {
    case LY_OUT_FDSTREAM:
        /* move the original file descriptor to the end of the output file */
        lseek(out->method.fdstream.fd, 0, SEEK_END);
        fflush(out->method.fdstream.f);
        break;
    case LY_OUT_FILEPATH:
    case LY_OUT_FILE:
        fflush(out->method.f);
        break;
    case LY_OUT_FD:
        fsync(out->method.fd);
        break;
    case LY_OUT_MEMORY:
    case LY_OUT_CALLBACK:
        /* nothing to do */
        break;
    case LY_OUT_ERROR:
        LOGINT(NULL);
    }

    free(out->buffered);
    out->buf_size = out->buf_len = 0;
}

LY_ERR
ly_write_(struct ly_out *out, const char *buf, size_t len)
{
    LY_ERR ret = LY_SUCCESS;
    size_t written = 0;

    if (out->hole_count) {
        /* we are buffering data after a hole */
        if (out->buf_len + len > out->buf_size) {
            out->buffered = ly_realloc(out->buffered, out->buf_len + len);
            if (!out->buffered) {
                out->buf_len = 0;
                out->buf_size = 0;
                LOGMEM(NULL);
                return LY_EMEM;
            }
            out->buf_size = out->buf_len + len;
        }

        memcpy(&out->buffered[out->buf_len], buf, len);
        out->buf_len += len;

        out->printed += len;
        out->func_printed += len;
        return LY_SUCCESS;
    }

repeat:
    switch (out->type) {
    case LY_OUT_MEMORY:
        if (out->method.mem.len + len + 1 > out->method.mem.size) {
            *out->method.mem.buf = ly_realloc(*out->method.mem.buf, out->method.mem.len + len + 1);
            if (!*out->method.mem.buf) {
                out->method.mem.len = 0;
                out->method.mem.size = 0;
                LOGMEM(NULL);
                return LY_EMEM;
            }
            out->method.mem.size = out->method.mem.len + len + 1;
        }
        memcpy(&(*out->method.mem.buf)[out->method.mem.len], buf, len);
        out->method.mem.len += len;
        (*out->method.mem.buf)[out->method.mem.len] = '\0';

        written = len;
        break;
    case LY_OUT_FD: {
        ssize_t r;
        r = write(out->method.fd, buf, len);
        if (r < 0) {
            ret = LY_ESYS;
        } else {
            written = (size_t)r;
        }
        break;
    }
    case LY_OUT_FDSTREAM:
    case LY_OUT_FILEPATH:
    case LY_OUT_FILE:
        written = fwrite(buf, sizeof *buf, len, out->method.f);
        if (written != len) {
            ret = LY_ESYS;
        }
        break;
    case LY_OUT_CALLBACK: {
        ssize_t r;
        r = out->method.clb.func(out->method.clb.arg, buf, len);
        if (r < 0) {
            ret = LY_ESYS;
        } else {
            written = (size_t)r;
        }
        break;
    }
    case LY_OUT_ERROR:
        LOGINT(NULL);
        return LY_EINT;
    }

    if (ret) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            ret = LY_SUCCESS;
            goto repeat;
        }
        LOGERR(NULL, LY_ESYS, "%s: writing data failed (%s).", __func__, strerror(errno));
        written = 0;
    } else if ((size_t)written != len) {
        LOGERR(NULL, LY_ESYS, "%s: writing data failed (unable to write %u from %u data).", __func__,
                len - (size_t)written, len);
        ret = LY_ESYS;
    } else {
        if (out->type == LY_OUT_FDSTREAM) {
            /* move the original file descriptor to the end of the output file */
            lseek(out->method.fdstream.fd, 0, SEEK_END);
        }
        ret = LY_SUCCESS;
    }

    out->printed += written;
    out->func_printed += written;
    return ret;
}

API LY_ERR
ly_write(struct ly_out *out, const char *buf, size_t len)
{
    out->func_printed = 0;

    return ly_write_(out, buf, len);
}

API size_t
ly_out_printed(const struct ly_out *out)
{
    return out->func_printed;
}

LY_ERR
ly_write_skip(struct ly_out *out, size_t count, size_t *position)
{
    switch (out->type) {
    case LY_OUT_MEMORY:
        if (out->method.mem.len + count > out->method.mem.size) {
            *out->method.mem.buf = ly_realloc(*out->method.mem.buf, out->method.mem.len + count);
            if (!(*out->method.mem.buf)) {
                out->method.mem.len = 0;
                out->method.mem.size = 0;
                LOGMEM(NULL);
                return LY_EMEM;
            }
            out->method.mem.size = out->method.mem.len + count;
        }

        /* save the current position */
        *position = out->method.mem.len;

        /* skip the memory */
        out->method.mem.len += count;
        break;
    case LY_OUT_FD:
    case LY_OUT_FDSTREAM:
    case LY_OUT_FILEPATH:
    case LY_OUT_FILE:
    case LY_OUT_CALLBACK:
        /* buffer the hole */
        if (out->buf_len + count > out->buf_size) {
            out->buffered = ly_realloc(out->buffered, out->buf_len + count);
            if (!out->buffered) {
                out->buf_len = 0;
                out->buf_size = 0;
                LOGMEM(NULL);
                return LY_EMEM;
            }
            out->buf_size = out->buf_len + count;
        }

        /* save the current position */
        *position = out->buf_len;

        /* skip the memory */
        out->buf_len += count;

        /* increase hole counter */
        ++out->hole_count;
        break;
    case LY_OUT_ERROR:
        LOGINT(NULL);
        return LY_EINT;
    }

    /* update printed bytes counter despite we actually printed just a hole */
    out->printed += count;
    out->func_printed += count;
    return LY_SUCCESS;
}

LY_ERR
ly_write_skipped(struct ly_out *out, size_t position, const char *buf, size_t count)
{
    LY_ERR ret = LY_SUCCESS;

    switch (out->type) {
    case LY_OUT_MEMORY:
        /* write */
        memcpy(&(*out->method.mem.buf)[position], buf, count);
        break;
    case LY_OUT_FD:
    case LY_OUT_FDSTREAM:
    case LY_OUT_FILEPATH:
    case LY_OUT_FILE:
    case LY_OUT_CALLBACK:
        if (out->buf_len < position + count) {
            LOGMEM(NULL);
            return LY_EMEM;
        }

        /* write into the hole */
        memcpy(&out->buffered[position], buf, count);

        /* decrease hole counter */
        --out->hole_count;

        if (!out->hole_count) {
            /* all holes filled, we can write the buffer,
             * printed bytes counter is updated by ly_write_() */
            ret = ly_write_(out, out->buffered, out->buf_len);
            out->buf_len = 0;
        }
        break;
    case LY_OUT_ERROR:
        LOGINT(NULL);
        return LY_EINT;
    }

    if (out->type == LY_OUT_FILEPATH) {
        /* move the original file descriptor to the end of the output file */
        lseek(out->method.fdstream.fd, 0, SEEK_END);
    }
    return ret;
}
