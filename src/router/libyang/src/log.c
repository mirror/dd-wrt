/**
 * @file log.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang logger implementation
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE
#define _BSD_SOURCE
#define _DEFAULT_SOURCE
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <assert.h>

#include "common.h"
#include "parser.h"
#include "context.h"
#include "tree_internal.h"

volatile uint8_t ly_log_level = LY_LLWRN;
volatile uint8_t ly_log_opts = LY_LOLOG | LY_LOSTORE_LAST;
static void (*ly_log_clb)(LY_LOG_LEVEL level, const char *msg, const char *path);
static volatile int path_flag = 1;
volatile int ly_log_dbg_groups = 0;

API LY_LOG_LEVEL
ly_verb(LY_LOG_LEVEL level)
{
    LY_LOG_LEVEL prev = ly_log_level;

    ly_log_level = level;
    return prev;
}

API int
ly_log_options(int opts)
{
    uint8_t prev = ly_log_opts;

    ly_log_opts = opts;
    return prev;
}

API void
ly_verb_dbg(int dbg_groups)
{
#ifndef NDEBUG
    ly_log_dbg_groups = dbg_groups;
#else
    (void)dbg_groups;
#endif
}

API void
ly_set_log_clb(void (*clb)(LY_LOG_LEVEL level, const char *msg, const char *path), int path)
{
    ly_log_clb = clb;
    path_flag = path;
}

API void
(*ly_get_log_clb(void))(LY_LOG_LEVEL, const char *, const char *)
{
    return ly_log_clb;
}

/* !! spends all string parameters !! */
static int
log_store(const struct ly_ctx *ctx, LY_LOG_LEVEL level, LY_ERR no, LY_VECODE vecode, char *msg, char *path, char *apptag)
{
    struct ly_err_item *eitem, *last;

    assert(ctx && (level < LY_LLVRB));

    eitem = pthread_getspecific(ctx->errlist_key);
    if (!eitem) {
        /* if we are only to fill in path, there must have been an error stored */
        assert(msg);
        eitem = malloc(sizeof *eitem);
        if (!eitem) {
            goto mem_fail;
        }
        eitem->prev = eitem;
        eitem->next = NULL;

        pthread_setspecific(ctx->errlist_key, eitem);
    } else if (!msg) {
        /* only filling the path */
        assert(path);

        /* find last error */
        eitem = eitem->prev;
        do {
            if (eitem->level == LY_LLERR) {
                /* fill the path */
                free(eitem->path);
                eitem->path = path;
                return 0;
            }
            eitem = eitem->prev;
        } while (eitem->prev->next);
        /* last error was not found */
        assert(0);
    } else if ((log_opt != ILO_STORE) && ((ly_log_opts & LY_LOSTORE_LAST) == LY_LOSTORE_LAST)) {
        /* overwrite last message */
        free(eitem->msg);
        free(eitem->path);
        free(eitem->apptag);
    } else {
        /* store new message */
        last = eitem->prev;
        eitem->prev = malloc(sizeof *eitem);
        if (!eitem->prev) {
            goto mem_fail;
        }
        eitem = eitem->prev;
        eitem->prev = last;
        eitem->next = NULL;
        last->next = eitem;
    }

    /* fill in the information */
    eitem->level = level;
    eitem->no = no;
    eitem->vecode = vecode;
    eitem->msg = msg;
    eitem->path = path;
    eitem->apptag = apptag;
    return 0;

mem_fail:
    LOGMEM(NULL);
    free(msg);
    free(path);
    free(apptag);
    return -1;
}

/* !! spends path !! */
static void
log_vprintf(const struct ly_ctx *ctx, LY_LOG_LEVEL level, LY_ERR no, LY_VECODE vecode, char *path,
            const char *format, va_list args)
{
    char *msg = NULL;
    int free_strs;

    if ((log_opt == ILO_ERR2WRN) && (level == LY_LLERR)) {
        /* change error to warning */
        level = LY_LLWRN;
    }

    if ((log_opt == ILO_IGNORE) || (level > ly_log_level)) {
        /* do not print or store the message */
        free(path);
        return;
    }

    /* set global errno on normal logging, but do not erase */
    if ((log_opt != ILO_STORE) && no) {
        ly_errno = no;
    }

    if ((no == LY_EVALID) && (vecode == LYVE_SUCCESS)) {
        /* assume we are inheriting the error, so inherit vecode as well */
        vecode = ly_vecode(ctx);
    }

    /* store the error/warning (if we need to store errors internally, it does not matter what are the user log options) */
    if ((level < LY_LLVRB) && ctx && ((ly_log_opts & LY_LOSTORE) || (log_opt == ILO_STORE))) {
        if (!format) {
            assert(path);
            /* postponed print of path related to the previous error, do not rewrite stored original message */
            if (log_store(ctx, level, no, vecode, NULL, path, NULL)) {
                return;
            }
            msg = "Path is related to the previous error message.";
        } else {
            if (vasprintf(&msg, format, args) == -1) {
                LOGMEM(ctx);
                free(path);
                return;
            }
            if (log_store(ctx, level, no, vecode, msg, path, NULL)) {
                return;
            }
        }
        free_strs = 0;
    } else {
        if (vasprintf(&msg, format, args) == -1) {
            LOGMEM(ctx);
            free(path);
            return;
        }
        free_strs = 1;
    }

    /* if we are only storing errors internally, never print the message (yet) */
    if ((ly_log_opts & LY_LOLOG) && (log_opt != ILO_STORE)) {
        if (ly_log_clb) {
            ly_log_clb(level, msg, path);
        } else {
            fprintf(stderr, "libyang[%d]: %s%s", level, msg, path ? " " : "\n");
            if (path) {
                fprintf(stderr, "(path: %s)\n", path);
            }
        }
    }

    if (free_strs) {
        free(path);
        free(msg);
    }
}

void
ly_log(const struct ly_ctx *ctx, LY_LOG_LEVEL level, LY_ERR no, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    log_vprintf(ctx, level, no, 0, NULL, format, ap);
    va_end(ap);
}

#ifndef NDEBUG

void
ly_log_dbg(int group, const char *format, ...)
{
    char *dbg_format;
    const char *str_group;
    va_list ap;

    if (!(ly_log_dbg_groups & group)) {
        return;
    }

    switch (group) {
    case LY_LDGDICT:
        str_group = "DICT";
        break;
    case LY_LDGYANG:
        str_group = "YANG";
        break;
    case LY_LDGYIN:
        str_group = "YIN";
        break;
    case LY_LDGXPATH:
        str_group = "XPATH";
        break;
    case LY_LDGDIFF:
        str_group = "DIFF";
        break;
    case LY_LDGAPI:
        str_group = "API";
        break;
    case LY_LDGHASH:
        str_group = "HASH";
        break;
    default:
        LOGINT(NULL);
        return;
    }

    if (asprintf(&dbg_format, "%s: %s", str_group, format) == -1) {
        LOGMEM(NULL);
        return;
    }

    va_start(ap, format);
    log_vprintf(NULL, LY_LLDBG, 0, 0, NULL, dbg_format, ap);
    va_end(ap);
    free(dbg_format);
}

#endif

API void
lyext_log(const struct ly_ctx *ctx, LY_LOG_LEVEL level, const char *plugin, const char *function, const char *format, ...)
{
    va_list ap;
    char *plugin_msg;
    int ret;

    if (ly_log_level < level) {
        return;
    }

    if (plugin)
        ret = asprintf(&plugin_msg, "%s (reported by plugin %s, %s())", format, plugin, function);
    else
        ret = asprintf(&plugin_msg, "%s", format);

    if (ret == -1) {
        LOGMEM(ctx);
        return;
    }

    va_start(ap, format);
    log_vprintf(ctx, level, (level == LY_LLERR ? LY_EPLUGIN : 0), 0, NULL, plugin_msg, ap);
    va_end(ap);

    free(plugin_msg);
}

static enum LY_VLOG_ELEM extvelog2velog[] = {
    LY_VLOG_NONE, /* LYEXT_VLOG_NONE */
    LY_VLOG_XML, /* LYEXT_VLOG_XML */
    LY_VLOG_LYS, /* LYEXT_VLOG_LYS */
    LY_VLOG_LYD, /* LYEXT_VLOG_LYD */
    LY_VLOG_STR, /* LYEXT_VLOG_STR */
    LY_VLOG_PREV, /* LYEXT_VLOG_PREV */
};

API void
lyext_vlog(const struct ly_ctx *ctx, LY_VECODE vecode, const char *plugin, const char *function,
           LYEXT_VLOG_ELEM elem_type, const void *elem, const char *format, ...)
{
    enum LY_VLOG_ELEM etype = extvelog2velog[elem_type];
    char *plugin_msg, *path = NULL;
    va_list ap;
    int ret;

    if (path_flag && (etype != LY_VLOG_NONE)) {
        if (etype == LY_VLOG_PREV) {
            /* use previous path */
            const struct ly_err_item *first = ly_err_first(ctx);
            if (first && first->prev->path) {
                path = strdup(first->prev->path);
            }
        } else {
            /* print path */
            if (!elem) {
                /* top-level */
                path = strdup("/");
            } else {
                ly_vlog_build_path(etype, elem, &path, 0, 0);
            }
        }
    }

    if (plugin)
        ret = asprintf(&plugin_msg, "%s (reported by plugin %s, %s())", format, plugin, function);
    else
        ret = asprintf(&plugin_msg, "%s", format);

    if (ret == -1) {
        LOGMEM(ctx);
        free(path);
        return;
    }

    va_start(ap, format);
    /* path is spent and should not be freed! */
    log_vprintf(ctx, LY_LLERR, LY_EVALID, vecode, path, plugin_msg, ap);
    va_end(ap);

    free(plugin_msg);
}

const char *ly_errs[] = {
/* LYE_SUCCESS */      "",
/* LYE_XML_MISS */     "Missing %s \"%s\".",
/* LYE_XML_INVAL */    "Invalid %s.",
/* LYE_XML_INCHAR */   "Encountered invalid character sequence \"%.10s\".",

/* LYE_EOF */          "Unexpected end of input data.",
/* LYE_INSTMT */       "Invalid keyword \"%s\".",
/* LYE_INCHILDSTMT */  "Invalid keyword \"%s\" as a child to \"%s\".",
/* LYE_INPAR */        "Invalid ancestor \"%s\" of \"%s\".",
/* LYE_INID */         "Invalid identifier \"%s\" (%s).",
/* LYE_INDATE */       "Invalid date \"%s\", valid date in format \"YYYY-MM-DD\" expected.",
/* LYE_INARG */        "Invalid value \"%s\" of \"%s\".",
/* LYE_MISSSTMT */     "Missing keyword \"%s\".",
/* LYE_MISSCHILDSTMT */ "Missing keyword \"%s\" as a child to \"%s\".",
/* LYE_MISSARG */      "Missing argument \"%s\" to keyword \"%s\".",
/* LYE_TOOMANY */      "Too many instances of \"%s\" in \"%s\".",
/* LYE_DUPID */        "Duplicated %s identifier \"%s\".",
/* LYE_DUPLEAFLIST */  "Duplicated instance of \"%s\" leaf-list (\"%s\").",
/* LYE_DUPLIST */      "Duplicated instance of \"%s\" list.",
/* LYE_NOUNIQ */       "Unique data leaf(s) \"%s\" not satisfied in \"%s\" and \"%s\".",
/* LYE_ENUM_INVAL */   "Invalid value \"%d\" of \"%s\" enum, restricted enum value does not match the base type value \"%d\".",
/* LYE_ENUM_INNAME */  "Adding new enum name \"%s\" in restricted enumeration type is not allowed.",
/* LYE_ENUM_DUPVAL */  "The value \"%d\" of \"%s\" enum has already been assigned to \"%s\" enum.",
/* LYE_ENUM_DUPNAME */ "The enum name \"%s\" has already been assigned to another enum.",
/* LYE_ENUM_WS */      "The enum name \"%s\" includes invalid leading or trailing whitespaces.",
/* LYE_BITS_INVAL */   "Invalid position \"%d\" of \"%s\" bit, restricted bits position does not match the base type position \"%d\".",
/* LYE_BITS_INNAME */  "Adding new bit name \"%s\" in restricted bits type is not allowed.",
/* LYE_BITS_DUPVAL */  "The position \"%d\" of \"%s\" bit has already been assigned to \"%s\" bit.",
/* LYE_BITS_DUPNAME */ "The bit name \"%s\" has already been assigned to another bit.",
/* LYE_INMOD */        "Module name \"%s\" refers to an unknown module.",
/* LYE_INMOD_LEN */    "Module name \"%.*s\" refers to an unknown module.",
/* LYE_KEY_NLEAF */    "Key \"%s\" is not a leaf.",
/* LYE_KEY_TYPE */     "Key \"%s\" must not be the built-in type \"empty\".",
/* LYE_KEY_CONFIG */   "The \"config\" value of the \"%s\" key differs from its list config value.",
/* LYE_KEY_MISS */     "Leaf \"%s\" defined as key in a list not found.",
/* LYE_KEY_DUP */      "Key identifier \"%s\" is not unique.",
/* LYE_INREGEX */      "Regular expression \"%s\" is not valid (\"%s\": %s).",
/* LYE_INRESOLV */     "Failed to resolve %s \"%s\".",
/* LYE_INSTATUS */     "A %s definition \"%s\" %s %s definition \"%s\".",
/* LYE_CIRC_LEAFREFS */"A circular chain of leafrefs detected.",
/* LYE_CIRC_FEATURES */"A circular chain features detected in \"%s\" feature.",
/* LYE_CIRC_IMPORTS */ "A circular dependency (import) for module \"%s\".",
/* LYE_CIRC_INCLUDES */"A circular dependency (include) for submodule \"%s\".",
/* LYE_INVER */        "Different YANG versions of a submodule and its main module.",
/* LYE_SUBMODULE */    "Unable to parse submodule, parse the main module instead.",

/* LYE_OBSDATA */      "Obsolete data \"%s\" instantiated.",
/* LYE_OBSTYPE */      "Data node \"%s\" with obsolete type \"%s\" instantiated.",
/* LYE_NORESOLV */     "No resolvents found for %s \"%s\".",
/* LYE_INELEM */       "Unknown element \"%s\".",
/* LYE_INELEM_LEN */   "Unknown element \"%.*s\".",
/* LYE_MISSELEM */     "Missing required element \"%s\" in \"%s\".",
/* LYE_INVAL */        "Invalid value \"%s\" in \"%s\" element.",
/* LYE_INMETA */       "Invalid \"%s:%s\" metadata with value \"%s\".",
/* LYE_INATTR */       "Invalid attribute \"%s\".",
/* LYE_MISSATTR */     "Missing attribute \"%s\" in \"%s\" element.",
/* LYE_NOCONSTR */     "Value \"%s\" does not satisfy the constraint \"%s\" (range, length, or pattern).",
/* LYE_INCHAR */       "Unexpected character(s) '%c' (%.15s).",
/* LYE_INPRED */       "Predicate resolution failed on \"%s\".",
/* LYE_MCASEDATA */    "Data for more than one case branch of \"%s\" choice present.",
/* LYE_NOMUST */       "Must condition \"%s\" not satisfied.",
/* LYE_NOWHEN */       "When condition \"%s\" not satisfied.",
/* LYE_INORDER */      "Invalid order of elements \"%s\" and \"%s\".",
/* LYE_INWHEN */       "Irresolvable when condition \"%s\".",
/* LYE_NOMIN */        "Too few \"%s\" elements.",
/* LYE_NOMAX */        "Too many \"%s\" elements.",
/* LYE_NOREQINS */     "Required instance of \"%s\" does not exist.",
/* LYE_NOLEAFREF */    "Leafref \"%s\" of value \"%s\" points to a non-existing leaf.",
/* LYE_NOMANDCHOICE */ "Mandatory choice \"%s\" missing a case branch.",

/* LYE_XPATH_INTOK */  "Unexpected XPath token %s (%.15s).",
/* LYE_XPATH_EOF */    "Unexpected XPath expression end.",
/* LYE_XPATH_INOP_1 */ "Cannot apply XPath operation %s on %s.",
/* LYE_XPATH_INOP_2 */ "Cannot apply XPath operation %s on %s and %s.",
/* LYE_XPATH_INCTX */  "Invalid context type %s in %s.",
/* LYE_XPATH_INMOD */  "Unknown module \"%.*s\".",
/* LYE_XPATH_INFUNC */ "Unknown XPath function \"%.*s\".",
/* LYE_XPATH_INARGCOUNT */ "Invalid number of arguments (%d) for the XPath function %.*s.",
/* LYE_XPATH_INARGTYPE */ "Wrong type of argument #%d (%s) for the XPath function %s.",
/* LYE_XPATH_DUMMY */   "Accessing the value of the dummy node \"%s\".",
/* LYE_XPATH_NOEND */   "Unterminated string delimited with %c (%.15s).",

/* LYE_PATH_INCHAR */  "Unexpected character(s) '%c' (\"%s\").",
/* LYE_PATH_INMOD */   "Module not found or not implemented.",
/* LYE_PATH_MISSMOD */ "Missing module name.",
/* LYE_PATH_INNODE */  "Schema node not found.",
/* LYE_PATH_INKEY */   "List key not found or on incorrect position (\"%s\").",
/* LYE_PATH_MISSKEY */ "List keys or position missing (\"%s\").",
/* LYE_PATH_INIDENTREF */ "Identityref predicate value \"%.*s\" missing module name.",
/* LYE_PATH_EXISTS */  "Node already exists.",
/* LYE_PATH_MISSPAR */ "Parent does not exist.",
/* LYE_PATH_PREDTOOMANY */ "Too many predicates.",
};

static const LY_VECODE ecode2vecode[] = {
    LYVE_SUCCESS,      /* LYE_SUCCESS */

    LYVE_XML_MISS,     /* LYE_XML_MISS */
    LYVE_XML_INVAL,    /* LYE_XML_INVAL */
    LYVE_XML_INCHAR,   /* LYE_XML_INCHAR */

    LYVE_EOF,          /* LYE_EOF */
    LYVE_INSTMT,       /* LYE_INSTMT */
    LYVE_INSTMT,       /* LYE_INCHILDSTMT */
    LYVE_INPAR,        /* LYE_INPAR */
    LYVE_INID,         /* LYE_INID */
    LYVE_INDATE,       /* LYE_INDATE */
    LYVE_INARG,        /* LYE_INARG */
    LYVE_MISSSTMT,     /* LYE_MISSCHILDSTMT */
    LYVE_MISSSTMT,     /* LYE_MISSSTMT */
    LYVE_MISSARG,      /* LYE_MISSARG */
    LYVE_TOOMANY,      /* LYE_TOOMANY */
    LYVE_DUPID,        /* LYE_DUPID */
    LYVE_DUPLEAFLIST,  /* LYE_DUPLEAFLIST */
    LYVE_DUPLIST,      /* LYE_DUPLIST */
    LYVE_NOUNIQ,       /* LYE_NOUNIQ */
    LYVE_ENUM_INVAL,   /* LYE_ENUM_INVAL */
    LYVE_ENUM_INNAME,  /* LYE_ENUM_INNAME */
    LYVE_ENUM_INVAL,   /* LYE_ENUM_DUPVAL */
    LYVE_ENUM_INNAME,  /* LYE_ENUM_DUPNAME */
    LYVE_ENUM_WS,      /* LYE_ENUM_WS */
    LYVE_BITS_INVAL,   /* LYE_BITS_INVAL */
    LYVE_BITS_INNAME,  /* LYE_BITS_INNAME */
    LYVE_BITS_INVAL,   /* LYE_BITS_DUPVAL */
    LYVE_BITS_INNAME,  /* LYE_BITS_DUPNAME */
    LYVE_INMOD,        /* LYE_INMOD */
    LYVE_INMOD,        /* LYE_INMOD_LEN */
    LYVE_KEY_NLEAF,    /* LYE_KEY_NLEAF */
    LYVE_KEY_TYPE,     /* LYE_KEY_TYPE */
    LYVE_KEY_CONFIG,   /* LYE_KEY_CONFIG */
    LYVE_KEY_MISS,     /* LYE_KEY_MISS */
    LYVE_KEY_DUP,      /* LYE_KEY_DUP */
    LYVE_INREGEX,      /* LYE_INREGEX */
    LYVE_INRESOLV,     /* LYE_INRESOLV */
    LYVE_INSTATUS,     /* LYE_INSTATUS */
    LYVE_CIRC_LEAFREFS,/* LYE_CIRC_LEAFREFS */
    LYVE_CIRC_FEATURES,/* LYE_CIRC_FEATURES */
    LYVE_CIRC_IMPORTS, /* LYE_CIRC_IMPORTS */
    LYVE_CIRC_INCLUDES,/* LYE_CIRC_INCLUDES */
    LYVE_INVER,        /* LYE_INVER */
    LYVE_SUBMODULE,    /* LYE_SUBMODULE */

    LYVE_OBSDATA,      /* LYE_OBSDATA */
    LYVE_OBSDATA,      /* LYE_OBSTYPE */
    LYVE_NORESOLV,     /* LYE_NORESOLV */
    LYVE_INELEM,       /* LYE_INELEM */
    LYVE_INELEM,       /* LYE_INELEM_LEN */
    LYVE_MISSELEM,     /* LYE_MISSELEM */
    LYVE_INVAL,        /* LYE_INVAL */
    LYVE_INMETA,       /* LYE_INMETA */
    LYVE_INATTR,       /* LYE_INATTR */
    LYVE_MISSATTR,     /* LYE_MISSATTR */
    LYVE_NOCONSTR,     /* LYE_NOCONSTR */
    LYVE_INCHAR,       /* LYE_INCHAR */
    LYVE_INPRED,       /* LYE_INPRED */
    LYVE_MCASEDATA,    /* LYE_MCASEDATA */
    LYVE_NOMUST,       /* LYE_NOMUST */
    LYVE_NOWHEN,       /* LYE_NOWHEN */
    LYVE_INORDER,      /* LYE_INORDER */
    LYVE_INWHEN,       /* LYE_INWHEN */
    LYVE_NOMIN,        /* LYE_NOMIN */
    LYVE_NOMAX,        /* LYE_NOMAX */
    LYVE_NOREQINS,     /* LYE_NOREQINS */
    LYVE_NOLEAFREF,    /* LYE_NOLEAFREF */
    LYVE_NOMANDCHOICE, /* LYE_NOMANDCHOICE */

    LYVE_XPATH_INTOK,  /* LYE_XPATH_INTOK */
    LYVE_XPATH_EOF,    /* LYE_XPATH_EOF */
    LYVE_XPATH_INOP,   /* LYE_XPATH_INOP_1 */
    LYVE_XPATH_INOP,   /* LYE_XPATH_INOP_2 */
    LYVE_XPATH_INCTX,  /* LYE_XPATH_INCTX */
    LYVE_XPATH_INMOD,  /* LYE_XPATH_INMOD */
    LYVE_XPATH_INFUNC, /* LYE_XPATH_INFUNC */
    LYVE_XPATH_INARGCOUNT, /* LYE_XPATH_INARGCOUNT */
    LYVE_XPATH_INARGTYPE, /* LYE_XPATH_INARGTYPE */
    LYVE_XPATH_DUMMY,  /* LYE_XPATH_DUMMY */
    LYVE_XPATH_NOEND,  /* LYE_XPATH_NOEND */

    LYVE_PATH_INCHAR,  /* LYE_PATH_INCHAR */
    LYVE_PATH_INMOD,   /* LYE_PATH_INMOD */
    LYVE_PATH_MISSMOD, /* LYE_PATH_MISSMOD */
    LYVE_PATH_INNODE,  /* LYE_PATH_INNODE */
    LYVE_PATH_INKEY,   /* LYE_PATH_INKEY */
    LYVE_PATH_MISSKEY, /* LYE_PATH_MISSKEY */
    LYVE_PATH_INIDENTREF, /* LYE_PATH_INIDENTREF */
    LYVE_PATH_EXISTS,  /* LYE_PATH_EXISTS */
    LYVE_PATH_MISSPAR, /* LYE_PATH_MISSPAR */
    LYVE_PATH_PREDTOOMANY, /* LYE_PATH_PREDTOOMANY */
};

static int
ly_vlog_build_path_print(char **path, uint16_t *index, const char *str, uint16_t str_len, uint16_t *length)
{
    void *mem;
    uint16_t step;

    if ((*index) < str_len) {
        /* enlarge buffer */
        step = (str_len < LY_BUF_STEP) ? LY_BUF_STEP : str_len;
        mem = realloc(*path, *length + *index + step + 1);
        LY_CHECK_ERR_RETURN(!mem, LOGMEM(NULL), -1);
        *path = mem;

        /* move data, lengths */
        memmove(&(*path)[*index + step], &(*path)[*index], *length);
        (*index) += step;
    }

    (*index) -= str_len;
    memcpy(&(*path)[*index], str, str_len);
    *length += str_len;

    return 0;
}

int
ly_vlog_build_path(enum LY_VLOG_ELEM elem_type, const void *elem, char **path, int schema_all_prefixes, int data_no_last_predicate)
{
    int i, j, yang_data_extension = 0;
    struct lys_node_list *slist;
    struct lys_node *sparent = NULL;
    struct lyd_node *dlist, *diter;
    const struct lys_module *top_smodule = NULL;
    const char *name, *prefix = NULL, *val_end, *val_start, *ext_name;
    char *str;
    uint16_t length, index;
    size_t len;

    length = 0;
    *path = malloc(1);
    LY_CHECK_ERR_RETURN(!(*path), LOGMEM(NULL), -1);
    index = 0;

    while (elem) {
        switch (elem_type) {
        case LY_VLOG_XML:
            name = ((struct lyxml_elem *)elem)->name;
            prefix = ((struct lyxml_elem *)elem)->ns ? ((struct lyxml_elem *)elem)->ns->prefix : NULL;
            elem = ((struct lyxml_elem *)elem)->parent;
            break;
        case LY_VLOG_LYS:
            if (!top_smodule) {
                /* remember the top module, it will act as the current module */
                for (sparent = (struct lys_node *)elem; lys_parent(sparent); sparent = lys_parent(sparent));
                top_smodule = lys_node_module(sparent);
            }

            /* skip uses */
            sparent = lys_parent((struct lys_node *)elem);
            while (sparent && (sparent->nodetype == LYS_USES)) {
                sparent = lys_parent(sparent);
            }
            if (!sparent || (lys_node_module((struct lys_node *)elem) != top_smodule) || schema_all_prefixes) {
                prefix = lys_node_module((struct lys_node *)elem)->name;
            } else {
                prefix = NULL;
            }

            if (((struct lys_node *)elem)->nodetype & (LYS_AUGMENT | LYS_GROUPING)) {
                if (ly_vlog_build_path_print(path, &index, "]", 1, &length)) {
                    return -1;
                }

                name = ((struct lys_node *)elem)->name;
                if (ly_vlog_build_path_print(path, &index, name, strlen(name), &length)) {
                    return -1;
                }

                if (((struct lys_node *)elem)->nodetype == LYS_GROUPING) {
                    name = "{grouping}[";
                } else { /* augment */
                    name = "{augment}[";
                }
            } else if (((struct lys_node *)elem)->nodetype == LYS_EXT) {
                name = ((struct lys_ext_instance *)elem)->def->name;
                if (!strcmp(name, "yang-data")) {
                    yang_data_extension = 1;
                    name = ((struct lys_ext_instance *)elem)->arg_value;
                    prefix = lys_node_module((struct lys_node *)elem)->name;
                }
            } else {
                name = ((struct lys_node *)elem)->name;
            }

            if (((struct lys_node *)elem)->nodetype == LYS_EXT) {
                if (((struct lys_ext_instance*)elem)->parent_type == LYEXT_PAR_NODE) {
                    elem = (struct lys_node*)((struct lys_ext_instance*)elem)->parent;
                } else {
                    sparent = NULL;
                    elem = NULL;
                }
                break;
            }

            /* need to find the parent again because we don't want to skip augments */
            do {
                sparent = ((struct lys_node *)elem)->parent;
                elem = lys_parent((struct lys_node *)elem);
            } while (elem && (((struct lys_node *)elem)->nodetype == LYS_USES));
            break;
        case LY_VLOG_LYD:
            name = ((struct lyd_node *)elem)->schema->name;
            if (!((struct lyd_node *)elem)->parent ||
                    lyd_node_module((struct lyd_node *)elem) != lyd_node_module(((struct lyd_node *)elem)->parent)) {
                prefix = lyd_node_module((struct lyd_node *)elem)->name;
            } else {
                prefix = NULL;
            }

            /* handle predicates (keys) in case of lists */
            if (!data_no_last_predicate || index) {
                if (((struct lyd_node *)elem)->schema->nodetype == LYS_LIST) {
                    dlist = (struct lyd_node *)elem;
                    slist = (struct lys_node_list *)((struct lyd_node *)elem)->schema;
                    if (slist->keys_size) {
                        /* schema list with keys - use key values in predicates */
                        for (i = slist->keys_size - 1; i > -1; i--) {
                            LY_TREE_FOR(dlist->child, diter) {
                                if (diter->schema == (struct lys_node *)slist->keys[i]) {
                                    break;
                                }
                            }
                            if (diter && ((struct lyd_node_leaf_list *)diter)->value_str) {
                                if (strchr(((struct lyd_node_leaf_list *)diter)->value_str, '\'')) {
                                    val_start = "=\"";
                                    val_end = "\"]";
                                } else {
                                    val_start = "='";
                                    val_end = "']";
                                }

                                /* print value */
                                if (ly_vlog_build_path_print(path, &index, val_end, 2, &length)) {
                                    return -1;
                                }
                                len = strlen(((struct lyd_node_leaf_list *)diter)->value_str);
                                if (ly_vlog_build_path_print(path, &index,
                                        ((struct lyd_node_leaf_list *)diter)->value_str, len, &length)) {
                                    return -1;
                                }

                                /* print schema name */
                                if (ly_vlog_build_path_print(path, &index, val_start, 2, &length)) {
                                    return -1;
                                }
                                len = strlen(diter->schema->name);
                                if (ly_vlog_build_path_print(path, &index, diter->schema->name, len, &length)) {
                                    return -1;
                                }

                                if (lyd_node_module(dlist) != lyd_node_module(diter)) {
                                    if (ly_vlog_build_path_print(path, &index, ":", 1, &length)) {
                                        return -1;
                                    }
                                    len = strlen(lyd_node_module(diter)->name);
                                    if (ly_vlog_build_path_print(path, &index, lyd_node_module(diter)->name, len, &length)) {
                                        return -1;
                                    }
                                }

                                if (ly_vlog_build_path_print(path, &index, "[", 1, &length)) {
                                    return -1;
                                }
                            }
                        }
                    } else {
                        /* schema list without keys - use instance position */
                        i = j = lyd_list_pos(dlist);
                        len = 1;
                        while (j > 9) {
                            ++len;
                            j /= 10;
                        }

                        if (ly_vlog_build_path_print(path, &index, "]", 1, &length)) {
                            return -1;
                        }

                        str = malloc(len + 1);
                        LY_CHECK_ERR_RETURN(!str, LOGMEM(NULL), -1);
                        sprintf(str, "%d", i);

                        if (ly_vlog_build_path_print(path, &index, str, len, &length)) {
                            free(str);
                            return -1;
                        }
                        free(str);

                        if (ly_vlog_build_path_print(path, &index, "[", 1, &length)) {
                            return -1;
                        }
                    }
                } else if (((struct lyd_node *)elem)->schema->nodetype == LYS_LEAFLIST &&
                        ((struct lyd_node_leaf_list *)elem)->value_str) {
                    if (strchr(((struct lyd_node_leaf_list *)elem)->value_str, '\'')) {
                        val_start = "[.=\"";
                        val_end = "\"]";
                    } else {
                        val_start = "[.='";
                        val_end = "']";
                    }

                    if (ly_vlog_build_path_print(path, &index, val_end, 2, &length)) {
                        return -1;
                    }
                    len = strlen(((struct lyd_node_leaf_list *)elem)->value_str);
                    if (ly_vlog_build_path_print(path, &index, ((struct lyd_node_leaf_list *)elem)->value_str, len, &length)) {
                        return -1;
                    }
                    if (ly_vlog_build_path_print(path, &index, val_start, 4, &length)) {
                        return -1;
                    }
                }
            }

            /* check if it is yang-data top element */
            if (!((struct lyd_node *)elem)->parent) {
                ext_name = lyp_get_yang_data_template_name(elem);
                if (ext_name) {
                    if (ly_vlog_build_path_print(path, &index, name, strlen(name), &length)) {
                        return -1;
                    }
                    if (ly_vlog_build_path_print(path, &index, "/", 1, &length)) {
                        return -1;
                    }
                    yang_data_extension = 1;
                    name = ext_name;
               }
            }

            elem = ((struct lyd_node *)elem)->parent;
            break;
        case LY_VLOG_STR:
            len = strlen((const char *)elem);
            if (ly_vlog_build_path_print(path, &index, (const char *)elem, len, &length)) {
                return -1;
            }
            goto success;
        default:
            /* shouldn't be here */
            LOGINT(NULL);
            return -1;
        }
        if (name) {
            if (ly_vlog_build_path_print(path, &index, name, strlen(name), &length)) {
                return -1;
            }
            if (prefix) {
                if (yang_data_extension && ly_vlog_build_path_print(path, &index, "#", 1, &length)) {
                    return -1;
                }
                if (ly_vlog_build_path_print(path, &index, ":", 1, &length)) {
                    return -1;
                }
                if (ly_vlog_build_path_print(path, &index, prefix, strlen(prefix), &length)) {
                    return -1;
                }
            }
        }
        if (ly_vlog_build_path_print(path, &index, "/", 1, &length)) {
            return -1;
        }
        if ((elem_type == LY_VLOG_LYS) && !elem && sparent && (sparent->nodetype == LYS_AUGMENT)) {
            len = strlen(((struct lys_node_augment *)sparent)->target_name);
            if (ly_vlog_build_path_print(path, &index, ((struct lys_node_augment *)sparent)->target_name, len, &length)) {
                return -1;
            }
        }
    }

success:
    memmove(*path, (*path) + index, length);
    (*path)[length] = '\0';
    return 0;
}

void
ly_vlog(const struct ly_ctx *ctx, LY_ECODE ecode, enum LY_VLOG_ELEM elem_type, const void *elem, ...)
{
    va_list ap;
    const char *fmt;
    char* path = NULL;
    const struct ly_err_item *first;

    if ((ecode == LYE_PATH) && !path_flag) {
        return;
    }

    if (path_flag && (elem_type != LY_VLOG_NONE)) {
        if (elem_type == LY_VLOG_PREV) {
            /* use previous path */
            first = ly_err_first(ctx);
            if (first && first->prev->path) {
                path = strdup(first->prev->path);
            }
        } else {
            /* print path */
            if (!elem) {
                /* top-level */
                path = strdup("/");
            } else {
                ly_vlog_build_path(elem_type, elem, &path, 0, 0);
            }
        }
    }

    va_start(ap, elem);
    /* path is spent and should not be freed! */
    switch (ecode) {
    case LYE_SPEC:
        fmt = va_arg(ap, char *);
        log_vprintf(ctx, LY_LLERR, LY_EVALID, LYVE_SUCCESS, path, fmt, ap);
        break;
    case LYE_PATH:
        assert(path);
        log_vprintf(ctx, LY_LLERR, LY_EVALID, LYVE_SUCCESS, path, NULL, ap);
        break;
    default:
        log_vprintf(ctx, LY_LLERR, LY_EVALID, ecode2vecode[ecode], path, ly_errs[ecode], ap);
        break;
    }
    va_end(ap);
}

void
ly_vlog_str(const struct ly_ctx *ctx, enum LY_VLOG_ELEM elem_type, const char *str, ...)
{
    va_list ap;
    char *path = NULL, *fmt, *ptr;
    const struct ly_err_item *first;

    assert((elem_type == LY_VLOG_NONE) || (elem_type == LY_VLOG_PREV));

    if (elem_type == LY_VLOG_PREV) {
        /* use previous path */
        first = ly_err_first(ctx);
        if (first && first->prev->path) {
            path = strdup(first->prev->path);
        }
    }

    if (strchr(str, '%')) {
        /* must be enough */
        fmt = malloc(2 * strlen(str) + 1);
        strcpy(fmt, str);
        for (ptr = strchr(fmt, '%'); ptr; ptr = strchr(ptr + 2, '%')) {
            memmove(ptr + 1, ptr, strlen(ptr) + 1);
            ptr[0] = '%';
        }
    } else {
        fmt = strdup(str);
    }

    va_start(ap, str);
    /* path is spent and should not be freed! */
    log_vprintf(ctx, LY_LLERR, LY_EVALID, LYVE_SUCCESS, path, fmt, ap);
    va_end(ap);

    free(fmt);
}

API void
ly_err_print(struct ly_err_item *eitem)
{
    if (ly_log_opts & LY_LOLOG) {
        if (ly_log_clb) {
            ly_log_clb(eitem->level, eitem->msg, eitem->path);
        } else {
            fprintf(stderr, "libyang[%d]: %s%s", eitem->level, eitem->msg, eitem->path ? " " : "\n");
            if (eitem->path) {
                fprintf(stderr, "(path: %s)\n", eitem->path);
            }
        }
    }
}

static void
err_print(struct ly_ctx *ctx, struct ly_err_item *last_eitem)
{
    if (!last_eitem) {
        last_eitem = pthread_getspecific(ctx->errlist_key);
    } else {
        /* this last was already stored before, do not write it again */
        last_eitem = last_eitem->next;
    }

    if ((log_opt != ILO_STORE) && (log_opt != ILO_IGNORE)) {
        for (; last_eitem; last_eitem = last_eitem->next) {
            ly_err_print(last_eitem);

            /* also properly update ly_errno */
            if (last_eitem->level == LY_LLERR) {
                ly_errno = last_eitem->no;
            }
        }
    }
}

/**
 * @brief Make \p last_eitem the last error item ignoring any logging options.
 */
void
ly_err_free_next(struct ly_ctx *ctx, struct ly_err_item *last_eitem)
{
    if (!last_eitem) {
        ly_err_clean(ctx, NULL);
    } else if (last_eitem->next) {
        ly_err_clean(ctx, last_eitem->next);
    }
}

/**
 * @brief Properly clean errors from \p ctx based on the user and internal logging options
 * after resolving schema/data unres.
 *
 * @param[in] ctx Context used.
 * @param[in] prev_eitem Most recent error item before resolving data unres.
 * @param[in] keep Whether to keep the stored errors.
 */
static void
err_clean(struct ly_ctx *ctx, struct ly_err_item *prev_eitem, int keep)
{
    struct ly_err_item *first;

    /* internal options take precedence */
    if (log_opt == ILO_STORE) {
        /* keep all the new errors */
    } else if ((log_opt == ILO_IGNORE) || !keep || !(ly_log_opts & LY_LOSTORE)) {
        /* throw away all the new errors */
        ly_err_free_next(ctx, prev_eitem);
    } else if ((ly_log_opts & LY_LOSTORE_LAST) == LY_LOSTORE_LAST) {
        /* keep only the most recent error */
        first = pthread_getspecific(ctx->errlist_key);
        if (!first) {
            /* no errors whatsoever */
            return;
        }
        prev_eitem = first->prev;

        /* put the context errlist in order */
        pthread_setspecific(ctx->errlist_key, prev_eitem);
        assert(!prev_eitem->prev->next || (prev_eitem->prev->next == prev_eitem));
        prev_eitem->prev->next = NULL;
        prev_eitem->prev = prev_eitem;

        /* free all the errlist items except the last one, do not free any if there is only one */
        if (prev_eitem != first) {
            ly_err_free(first);
        }
    }
}

void
ly_ilo_change(struct ly_ctx *ctx, enum int_log_opts new_ilo, enum int_log_opts *prev_ilo, struct ly_err_item **prev_last_eitem)
{
    assert(prev_ilo);

    *prev_ilo = log_opt;
    if (new_ilo == ILO_STORE) {
        /* only in this case the errors are only temporarily stored */
        assert(ctx && prev_last_eitem);
        *prev_last_eitem = (struct ly_err_item *)ly_err_first(ctx);
        if (*prev_last_eitem) {
            *prev_last_eitem = (*prev_last_eitem)->prev;
        }
    }

    if (log_opt != ILO_IGNORE) {
        log_opt = new_ilo;
    } /* else we can just keep it, useless to change it */
}

void
ly_ilo_restore(struct ly_ctx *ctx, enum int_log_opts prev_ilo, struct ly_err_item *prev_last_eitem, int keep_and_print)
{
    assert(log_opt != ILO_LOG);
    if (log_opt != ILO_STORE) {
        /* nothing to print or free */
        assert(log_opt == prev_ilo || (!ctx && !prev_last_eitem && !keep_and_print));
        log_opt = prev_ilo;
        return;
    }

    assert(ctx);

    log_opt = prev_ilo;
    if (keep_and_print) {
        err_print(ctx, prev_last_eitem);
    }
    err_clean(ctx, prev_last_eitem, keep_and_print);
}

void
ly_err_last_set_apptag(const struct ly_ctx *ctx, const char *apptag)
{
    struct ly_err_item *i;

    if (log_opt != ILO_IGNORE) {
        i = ly_err_first(ctx);
        if (i) {
            i = i->prev;
            i->apptag = strdup(apptag);
        }
    }
}
