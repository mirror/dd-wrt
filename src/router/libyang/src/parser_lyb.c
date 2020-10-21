/**
 * @file parser_lyb.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief LYB data parser for libyang
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "libyang.h"
#include "common.h"
#include "context.h"
#include "parser.h"
#include "tree_internal.h"

#define LYB_HAVE_READ_GOTO(r, d, go) if (r < 0) goto go; d += r;
#define LYB_HAVE_READ_RETURN(r, d, ret) if (r < 0) return ret; d += r;

static int
lyb_read(const char *data, uint8_t *buf, size_t count, struct lyb_state *lybs)
{
    int ret = 0, i, empty_chunk_i;
    size_t to_read;
    uint8_t meta_buf[LYB_META_BYTES];

    assert(data && lybs);

    while (1) {
        /* check for fully-read (empty) data chunks */
        to_read = count;
        empty_chunk_i = -1;
        for (i = 0; i < lybs->used; ++i) {
            /* we want the innermost chunks resolved first, so replace previous empty chunks,
             * also ignore chunks that are completely finished, there is nothing for us to do */
            if ((lybs->written[i] <= to_read) && lybs->position[i]) {
                /* empty chunk, do not read more */
                to_read = lybs->written[i];
                empty_chunk_i = i;
            }
        }

        if ((empty_chunk_i == -1) && !count) {
            break;
        }

        /* we are actually reading some data, not just finishing another chunk */
        if (to_read) {
            if (buf) {
                memcpy(buf, data + ret, to_read);
            }

            for (i = 0; i < lybs->used; ++i) {
                /* decrease all written counters */
                lybs->written[i] -= to_read;
                assert(lybs->written[i] <= LYB_SIZE_MAX);
            }
            /* decrease count/buf */
            count -= to_read;
            if (buf) {
                buf += to_read;
            }

            ret += to_read;
        }

        if (empty_chunk_i > -1) {
            /* read the next chunk meta information */
            memcpy(meta_buf, data + ret, LYB_META_BYTES);
            lybs->written[empty_chunk_i] = meta_buf[0];
            lybs->inner_chunks[empty_chunk_i] = meta_buf[1];

            /* remember whether there is a following chunk or not */
            lybs->position[empty_chunk_i] = (lybs->written[empty_chunk_i] == LYB_SIZE_MAX ? 1 : 0);

            ret += LYB_META_BYTES;
        }
    }

    return ret;
}

static int
lyb_read_number(void *num, size_t num_size, size_t bytes, const char *data, struct lyb_state *lybs)
{
    int r, ret = 0;
    uint64_t buf = 0;

    ret += (r = lyb_read(data, (uint8_t *)&buf, bytes, lybs));
    LYB_HAVE_READ_RETURN(r, data, -1);

    /* correct byte order */
    buf = le64toh(buf);

    switch (num_size) {
    case 1:
        *((uint8_t *)num) = buf;
        break;
    case 2:
        *((uint16_t *)num) = buf;
        break;
    case 4:
        *((uint32_t *)num) = buf;
        break;
    case 8:
        *((uint64_t *)num) = buf;
        break;
    default:
        LOGINT(lybs->ctx);
        return -1;
    }

    return ret;
}

static int
lyb_read_enum(uint64_t *enum_idx, uint32_t count, const char *data, struct lyb_state *lybs)
{
    size_t bytes;

    if (count < (1 << 8)) {
        bytes = 1;
    } else if (count < (1 << 16)) {
        bytes = 2;
    } else if (count < (1 << 24)) {
        bytes = 3;
    } else {
        bytes = 4;
    }

    /* enum is always read into a uint64_t buffer */
    *enum_idx = 0;
    return lyb_read_number(enum_idx, sizeof *enum_idx, bytes, data, lybs);
}

static int
lyb_read_string(const char *data, char **str, int with_length, struct lyb_state *lybs)
{
    int next_chunk = 0, r, ret = 0;
    size_t len = 0, cur_len;

    if (with_length) {
        ret += (r = lyb_read_number(&len, sizeof len, 2, data, lybs));
        LYB_HAVE_READ_GOTO(r, data, error);
    } else {
        /* read until the end of this subtree */
        len = lybs->written[lybs->used - 1];
        if (lybs->position[lybs->used - 1]) {
            next_chunk = 1;
        }
    }

    *str = malloc((len + 1) * sizeof **str);
    LY_CHECK_ERR_RETURN(!*str, LOGMEM(lybs->ctx), -1);

    ret += (r = lyb_read(data, (uint8_t *)*str, len, lybs));
    LYB_HAVE_READ_GOTO(r, data, error);

    while (next_chunk) {
        cur_len = lybs->written[lybs->used - 1];
        if (lybs->position[lybs->used - 1]) {
            next_chunk = 1;
        } else {
            next_chunk = 0;
        }

        *str = ly_realloc(*str, (len + cur_len + 1) * sizeof **str);
        LY_CHECK_ERR_RETURN(!*str, LOGMEM(lybs->ctx), -1);

        ret += (r = lyb_read(data, ((uint8_t *)*str) + len, cur_len, lybs));
        LYB_HAVE_READ_GOTO(r, data, error);

        len += cur_len;
    }

    ((char *)*str)[len] = '\0';
    return ret;

error:
    free((char *)*str);
    *str = NULL;
    return -1;
}

static void
lyb_read_stop_subtree(struct lyb_state *lybs)
{
    if (lybs->written[lybs->used - 1]) {
        LOGINT(lybs->ctx);
    }

    --lybs->used;
}

static int
lyb_read_start_subtree(const char *data, struct lyb_state *lybs)
{
    uint8_t meta_buf[LYB_META_BYTES];

    if (lybs->used == lybs->size) {
        lybs->size += LYB_STATE_STEP;
        lybs->written = ly_realloc(lybs->written, lybs->size * sizeof *lybs->written);
        lybs->position = ly_realloc(lybs->position, lybs->size * sizeof *lybs->position);
        lybs->inner_chunks = ly_realloc(lybs->inner_chunks, lybs->size * sizeof *lybs->inner_chunks);
        LY_CHECK_ERR_RETURN(!lybs->written || !lybs->position || !lybs->inner_chunks, LOGMEM(lybs->ctx), -1);
    }

    memcpy(meta_buf, data, LYB_META_BYTES);

    ++lybs->used;
    lybs->written[lybs->used - 1] = meta_buf[0];
    lybs->inner_chunks[lybs->used - 1] = meta_buf[LYB_SIZE_BYTES];
    lybs->position[lybs->used - 1] = (lybs->written[lybs->used - 1] == LYB_SIZE_MAX ? 1 : 0);

    return LYB_META_BYTES;
}

static int
lyb_parse_model(const char *data, const struct lys_module **mod, int options, struct lyb_state *lybs)
{
    int r, ret = 0;
    char *mod_name = NULL, mod_rev[11];
    uint16_t rev;

    /* model name */
    ret += (r = lyb_read_string(data, &mod_name, 1, lybs));
    LYB_HAVE_READ_GOTO(r, data, error);

    /* revision */
    ret += (r = lyb_read_number(&rev, sizeof rev, 2, data, lybs));
    LYB_HAVE_READ_GOTO(r, data, error);

    if (rev) {
        sprintf(mod_rev, "%04u-%02u-%02u", ((rev & 0xFE00) >> 9) + 2000, (rev & 0x01E0) >> 5, rev & 0x001Fu);
        *mod = ly_ctx_get_module(lybs->ctx, mod_name, mod_rev, 0);
        if ((options & LYD_OPT_LYB_MOD_UPDATE) && !(*mod)) {
            /* try to use an updated module */
            *mod = ly_ctx_get_module(lybs->ctx, mod_name, NULL, 1);
            if (*mod && (!(*mod)->implemented || !(*mod)->rev_size || (strcmp((*mod)->rev[0].date, mod_rev) < 0))) {
                /* not an implemented module in a newer revision */
                *mod = NULL;
            }
        }
    } else {
        *mod = ly_ctx_get_module(lybs->ctx, mod_name, NULL, 0);
    }
    if (lybs->ctx->data_clb) {
        if (!*mod) {
            *mod = lybs->ctx->data_clb(lybs->ctx, mod_name, NULL, 0, lybs->ctx->data_clb_data);
        } else if (!(*mod)->implemented) {
            *mod = lybs->ctx->data_clb(lybs->ctx, mod_name, (*mod)->ns, LY_MODCLB_NOT_IMPLEMENTED, lybs->ctx->data_clb_data);
        }
    }

    if (!*mod) {
        LOGERR(lybs->ctx, LY_EINVAL, "Invalid context for LYB data parsing, missing module \"%s%s%s\".",
               mod_name, rev ? "@" : "", rev ? mod_rev : "");
        goto error;
    } else if (!(*mod)->implemented) {
        LOGERR(lybs->ctx, LY_EINVAL, "Invalid context for LYB data parsing, module \"%s%s%s\" not implemented.",
               mod_name, rev ? "@" : "", rev ? mod_rev : "");
        goto error;
    }

    free(mod_name);
    return ret;

error:
    free(mod_name);
    return -1;
}

static struct lyd_node *
lyb_new_node(const struct lys_node *schema, int options)
{
    struct lyd_node *node;

    switch (schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_NOTIF:
    case LYS_RPC:
    case LYS_ACTION:
        node = calloc(sizeof(struct lyd_node), 1);
        break;
    case LYS_LEAF:
    case LYS_LEAFLIST:
        node = calloc(sizeof(struct lyd_node_leaf_list), 1);
        break;
    case LYS_ANYDATA:
    case LYS_ANYXML:
        node = calloc(sizeof(struct lyd_node_anydata), 1);
        break;
    default:
        return NULL;
    }
    LY_CHECK_ERR_RETURN(!node, LOGMEM(schema->module->ctx), NULL);

    /* fill basic info */
    node->schema = (struct lys_node *)schema;
    if (resolve_applies_when(schema, 0, NULL)) {
        node->when_status = LYD_WHEN;

        if (options & LYD_OPT_TRUSTED) {
            /* this data are considered trusted so if this node exists, it means its when must have been true */
            node->when_status |= LYD_WHEN_TRUE;
        }
    }
    node->prev = node;

    return node;
}

static int
lyb_parse_anydata(struct lyd_node *node, const char *data, struct lyb_state *lybs)
{
    int r, ret = 0;
    char *str = NULL;
    struct lyd_node_anydata *any = (struct lyd_node_anydata *)node;

    /* read value type */
    ret += (r = lyb_read(data, (uint8_t *)&any->value_type, sizeof any->value_type, lybs));
    LYB_HAVE_READ_RETURN(r, data, -1);

    /* read anydata content */
    if (any->value_type == LYD_ANYDATA_DATATREE) {
        /* invalid situation */
        LOGINT(node->schema->module->ctx);
        return -1;
    } else if (any->value_type == LYD_ANYDATA_LYB) {
        ret += (r = lyb_read_string(data, &any->value.mem, 0, lybs));
        LYB_HAVE_READ_RETURN(r, data, -1);
    } else {
        ret += (r = lyb_read_string(data, &str, 0, lybs));
        LYB_HAVE_READ_RETURN(r, data, -1);

        /* add to dictionary */
        any->value.str = lydict_insert_zc(node->schema->module->ctx, str);
    }

    return ret;
}

/* generally, fill lyd_val value union */
static int
lyb_parse_val_1(struct lys_type *type, LY_DATA_TYPE value_type, uint8_t value_flags, const char *data,
        const char **value_str, lyd_val *value, struct lyb_state *lybs)
{
    int r, ret;
    size_t i;
    char *str = NULL;
    uint8_t byte;
    uint64_t num;

    if (value_flags & LY_VALUE_USER) {
        /* just read value_str */
        ret = lyb_read_string(data, &str, 0, lybs);
        if (ret > -1) {
            *value_str = lydict_insert_zc(lybs->ctx, str);
        }
        return ret;
    }

    /* find the correct structure, go through leafrefs and typedefs */
    switch (value_type) {
    case LY_TYPE_ENUM:
        for (; type->base == LY_TYPE_LEAFREF; type = &type->info.lref.target->type);
        for (; !type->info.enums.count; type = &type->der->type);
        break;
    case LY_TYPE_BITS:
        for (; type->base == LY_TYPE_LEAFREF; type = &type->info.lref.target->type);
        for (; !type->info.bits.count; type = &type->der->type);
        break;
    default:
        break;
    }

    switch (value_type) {
    case LY_TYPE_INST:
    case LY_TYPE_IDENT:
    case LY_TYPE_UNION:
        /* we do not actually fill value now, but value_str */
        ret = lyb_read_string(data, &str, 0, lybs);
        if (ret > -1) {
            *value_str = lydict_insert_zc(lybs->ctx, str);
        }
        break;
    case LY_TYPE_BINARY:
    case LY_TYPE_STRING:
    case LY_TYPE_UNKNOWN:
        /* read string */
        ret = lyb_read_string(data, &str, 0, lybs);
        if (ret > -1) {
            value->string = lydict_insert_zc(lybs->ctx, str);
        }
        break;
    case LY_TYPE_BITS:
        value->bit = calloc(type->info.bits.count, sizeof *value->bit);
        LY_CHECK_ERR_RETURN(!value->bit, LOGMEM(lybs->ctx), -1);

        /* read values */
        ret = 0;
        for (i = 0; i < type->info.bits.count; ++i) {
            if (i % 8 == 0) {
                /* read another byte */
                ret += (r = lyb_read(data + ret, &byte, sizeof byte, lybs));
                if (r < 0) {
                    return -1;
                }
            }

            if (byte & (0x01 << (i % 8))) {
                /* bit is set */
                value->bit[i] = &type->info.bits.bit[i];
            }
        }
        break;
    case LY_TYPE_BOOL:
        /* read byte */
        ret = lyb_read(data, &byte, sizeof byte, lybs);
        if ((ret > 0) && byte) {
            value->bln = 1;
        }
        break;
    case LY_TYPE_EMPTY:
        /* nothing to read */
        ret = 0;
        break;
    case LY_TYPE_ENUM:
        num = 0;
        ret = lyb_read_enum(&num, type->info.enums.count, data, lybs);
        if (ret > 0) {
            assert(num < type->info.enums.count);
            value->enm = &type->info.enums.enm[num];
        }
        break;
    case LY_TYPE_INT8:
    case LY_TYPE_UINT8:
        ret = lyb_read_number(&value->uint8, sizeof value->uint8, 1, data, lybs);
        break;
    case LY_TYPE_INT16:
    case LY_TYPE_UINT16:
        ret = lyb_read_number(&value->uint16, sizeof value->uint16, 2, data, lybs);
        break;
    case LY_TYPE_INT32:
    case LY_TYPE_UINT32:
        ret = lyb_read_number(&value->uint32, sizeof value->uint32, 4, data, lybs);
        break;
    case LY_TYPE_DEC64:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT64:
        ret = lyb_read_number(&value->uint64, sizeof value->uint64, 8, data, lybs);
        break;
    default:
        return -1;
    }

    return ret;
}

/* generally, fill value_str */
static int
lyb_parse_val_2(struct lys_type *type, struct lyd_node_leaf_list *leaf, struct lyd_attr *attr, struct unres_data *unres)
{
    struct ly_ctx *ctx;
    struct lys_module *mod;
    struct lys_type *rtype = NULL;
    char num_str[22], *str;
    int64_t frac, num;
    uint32_t i, str_len;
    uint8_t *value_flags, dig;
    const char **value_str;
    LY_DATA_TYPE value_type;
    lyd_val *value;

    if (leaf) {
        ctx = leaf->schema->module->ctx;
        mod = lys_node_module(leaf->schema);

        value = &leaf->value;
        value_str = &leaf->value_str;
        value_flags = &leaf->value_flags;
        value_type = leaf->value_type;
    } else {
        ctx = attr->annotation->module->ctx;
        mod = lys_main_module(attr->annotation->module);

        value = &attr->value;
        value_str = &attr->value_str;
        value_flags = &attr->value_flags;
        value_type = attr->value_type;
    }

    if (*value_flags & LY_VALUE_USER) {
        /* unfortunately, we need to also fill the value properly, so just parse it again */
        *value_flags &= ~LY_VALUE_USER;
        if (!lyp_parse_value(type, value_str, NULL, leaf, attr, NULL, 1, (leaf ? leaf->dflt : 0))) {
            return -1;
        }

        if (!(*value_flags & LY_VALUE_USER)) {
            LOGWRN(ctx, "Value \"%s\" was stored as a user type, but it is not in the current context.", value_str);
        }
        return 0;
    }

    /* we are parsing leafref/ptr union stored as the target type,
     * so we first parse it into string and then resolve the leafref/ptr union */
    if ((type->base == LY_TYPE_LEAFREF) || (type->base == LY_TYPE_INST)
            || ((type->base == LY_TYPE_UNION) && type->info.uni.has_ptr_type)) {
        if ((value_type == LY_TYPE_INST) || (value_type == LY_TYPE_IDENT) || (value_type == LY_TYPE_UNION)) {
            /* we already have a string */
            goto parse_reference;
        }
    }

    /* find the correct structure, go through leafrefs and typedefs */
    switch (value_type) {
    case LY_TYPE_BITS:
        for (rtype = type; rtype->base == LY_TYPE_LEAFREF; rtype = &rtype->info.lref.target->type);
        for (; !rtype->info.bits.count; rtype = &rtype->der->type);
        break;
    case LY_TYPE_DEC64:
        for (rtype = type; rtype->base == LY_TYPE_LEAFREF; rtype = &type->info.lref.target->type);
        break;
    default:
        break;
    }

    switch (value_type) {
    case LY_TYPE_IDENT:
        /* fill the identity pointer now */
        value->ident = resolve_identref(type, *value_str, (struct lyd_node *)leaf, mod, (leaf ? leaf->dflt : 0));
        if (!value->ident) {
            return -1;
        }
        break;
    case LY_TYPE_INST:
        /* unresolved instance-identifier, keep value NULL */
        value->instance = NULL;
        break;
    case LY_TYPE_BINARY:
    case LY_TYPE_STRING:
    case LY_TYPE_UNKNOWN:
        /* just re-assign it */
        *value_str = value->string;
        break;
    case LY_TYPE_BITS:
        /* print the set bits */
        str = malloc(1);
        LY_CHECK_ERR_RETURN(!str, LOGMEM(ctx), -1);
        str[0] = '\0';
        str_len = 0;
        for (i = 0; i < rtype->info.bits.count; ++i) {
            if (value->bit[i]) {
                str = ly_realloc(str, str_len + strlen(value->bit[i]->name) + (str_len ? 1 : 0) + 1);
                LY_CHECK_ERR_RETURN(!str, LOGMEM(ctx), -1);

                str_len += sprintf(str + str_len, "%s%s", str_len ? " " : "", value->bit[i]->name);
            }
        }

        *value_str = lydict_insert_zc(ctx, str);
        break;
    case LY_TYPE_BOOL:
        *value_str = lydict_insert(ctx, (value->bln ? "true" : "false"), 0);
        break;
    case LY_TYPE_EMPTY:
        *value_str = lydict_insert(ctx, "", 0);
        break;
    case LY_TYPE_UNION:
        if (attr) {
            /* we do not support union type attribute */
            LOGINT(ctx);
            return -1;
        }

        if (resolve_union(leaf, type, 1, 2, NULL)) {
            return -1;
        }
        break;
    case LY_TYPE_ENUM:
        /* print the value */
        *value_str = lydict_insert(ctx, value->enm->name, 0);
        break;
    case LY_TYPE_INT8:
        sprintf(num_str, "%d", value->int8);
        *value_str = lydict_insert(ctx, num_str, 0);
        break;
    case LY_TYPE_UINT8:
        sprintf(num_str, "%u", value->uint8);
        *value_str = lydict_insert(ctx, num_str, 0);
        break;
    case LY_TYPE_INT16:
        sprintf(num_str, "%d", value->int16);
        *value_str = lydict_insert(ctx, num_str, 0);
        break;
    case LY_TYPE_UINT16:
        sprintf(num_str, "%u", value->uint16);
        *value_str = lydict_insert(ctx, num_str, 0);
        break;
    case LY_TYPE_INT32:
        sprintf(num_str, "%d", value->int32);
        *value_str = lydict_insert(ctx, num_str, 0);
        break;
    case LY_TYPE_UINT32:
        sprintf(num_str, "%u", value->uint32);
        *value_str = lydict_insert(ctx, num_str, 0);
        break;
    case LY_TYPE_INT64:
        sprintf(num_str, "%"PRId64, value->int64);
        *value_str = lydict_insert(ctx, num_str, 0);
        break;
    case LY_TYPE_UINT64:
        sprintf(num_str, "%"PRIu64, value->uint64);
        *value_str = lydict_insert(ctx, num_str, 0);
        break;
    case LY_TYPE_DEC64:
        num = value->dec64 / (int64_t)rtype->info.dec64.div;
        frac = value->dec64 % (int64_t)rtype->info.dec64.div;
        dig = rtype->info.dec64.dig;

        /* frac should always be positive, remove trailing zeros */
        if (frac < 0) {
            frac *= -1;
        }
        while ((dig > 1) && !(frac % 10)) {
            frac /= 10;
            --dig;
        }

        /* handle special case of int64_t not supporting printing -0 */
        sprintf(num_str, "%s%"PRId64".%.*"PRId64, (num == 0) && (value->dec64 < 0) ? "-" : "", num, dig, frac);
        *value_str = lydict_insert(ctx, num_str, 0);
        break;
    default:
        return -1;
    }

    if ((type->base == LY_TYPE_LEAFREF) || (type->base == LY_TYPE_INST)
            || ((type->base == LY_TYPE_UNION) && type->info.uni.has_ptr_type)) {
parse_reference:
        assert(*value_str);

        if (attr) {
            /* we do not support reference types of attributes */
            LOGINT(ctx);
            return -1;
        }

        if (type->base == LY_TYPE_INST) {
            if (unres_data_add(unres, (struct lyd_node *)leaf, UNRES_INSTID)) {
                return -1;
            }
        } else if (type->base == LY_TYPE_LEAFREF) {
            if (unres_data_add(unres, (struct lyd_node *)leaf, UNRES_LEAFREF)) {
                return -1;
            }
        } else {
            if (unres_data_add(unres, (struct lyd_node *)leaf, UNRES_UNION)) {
                return -1;
            }
        }
    }

    return 0;
}

static int
lyb_parse_value(struct lys_type *type, struct lyd_node_leaf_list *leaf, struct lyd_attr *attr, const char *data,
                struct unres_data *unres, struct lyb_state *lybs)
{
    int r, ret = 0;
    uint8_t start_byte;

    const char **value_str;
    lyd_val *value;
    LY_DATA_TYPE *value_type;
    uint8_t *value_flags;

    assert((leaf || attr) && (!leaf || !attr));

    if (leaf) {
        value_str = &leaf->value_str;
        value = &leaf->value;
        value_type = &leaf->value_type;
        value_flags = &leaf->value_flags;
    } else {
        value_str = &attr->value_str;
        value = &attr->value;
        value_type = &attr->value_type;
        value_flags = &attr->value_flags;
    }

    /* read value type and flags on the first byte */
    ret += (r = lyb_read(data, &start_byte, sizeof start_byte, lybs));
    LYB_HAVE_READ_RETURN(r, data, -1);

    /* fill value type, flags */
    *value_type = start_byte & 0x1F;
    if (start_byte & 0x80) {
        assert(leaf);
        leaf->dflt = 1;
    }
    if (start_byte & 0x40) {
        *value_flags |= LY_VALUE_USER;
    }
    if (start_byte & 0x20) {
        *value_flags |= LY_VALUE_UNRES;
    }

    ret += (r = lyb_parse_val_1(type, *value_type, *value_flags, data, value_str, value, lybs));
    LYB_HAVE_READ_RETURN(r, data, -1);

    /* union is handled specially */
    if ((type->base == LY_TYPE_UNION) && !(*value_flags & LY_VALUE_USER)) {
        assert(*value_type == LY_TYPE_STRING);

        *value_str = value->string;
        value->string = NULL;
        *value_type = LY_TYPE_UNION;
    }

    ret += (r = lyb_parse_val_2(type, leaf, attr, unres));
    LYB_HAVE_READ_RETURN(r, data, -1);

    return ret;
}

static int
lyb_parse_attr_name(const struct lys_module *mod, const char *data, struct lys_ext_instance_complex **ext, int options,
                    struct lyb_state *lybs)
{
    int r, ret = 0, pos, i, j, k;
    const struct lys_submodule *submod = NULL;
    char *attr_name = NULL;

    /* attr name */
    ret += (r = lyb_read_string(data, &attr_name, 1, lybs));
    LYB_HAVE_READ_RETURN(r, data, -1);

    /* search module */
    pos = -1;
    for (i = 0, j = 0; i < mod->ext_size; i = i + j + 1) {
        j = lys_ext_instance_presence(&mod->ctx->models.list[0]->extensions[0], &mod->ext[i], mod->ext_size - i);
        if (j == -1) {
            break;
        }
        if (ly_strequal(mod->ext[i + j]->arg_value, attr_name, 0)) {
            pos = i + j;
            break;
        }
    }

    /* try submodules */
    if (pos == -1) {
        for (k = 0; k < mod->inc_size; ++k) {
            submod = mod->inc[k].submodule;
            for (i = 0, j = 0; i < submod->ext_size; i = i + j + 1) {
                j = lys_ext_instance_presence(&mod->ctx->models.list[0]->extensions[0], &submod->ext[i], submod->ext_size - i);
                if (j == -1) {
                    break;
                }
                if (ly_strequal(submod->ext[i + j]->arg_value, attr_name, 0)) {
                    pos = i + j;
                    break;
                }
            }
        }
    }

    if (pos == -1) {
        *ext = NULL;
    } else {
        *ext = submod ? (struct lys_ext_instance_complex *)submod->ext[pos] : (struct lys_ext_instance_complex *)mod->ext[pos];
    }

    if (!*ext && (options & LYD_OPT_STRICT)) {
        LOGVAL(mod->ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Failed to find annotation \"%s\" in \"%s\".", attr_name, mod->name);
        free(attr_name);
        return -1;
    }

    free(attr_name);
    return ret;
}

static int
lyb_parse_attributes(struct lyd_node *node, const char *data, int options, struct unres_data *unres, struct lyb_state *lybs)
{
    int r, ret = 0;
    uint8_t i, count = 0;
    const struct lys_module *mod;
    struct lys_type **type;
    struct lyd_attr *attr = NULL;
    struct lys_ext_instance_complex *ext;

    /* read number of attributes stored */
    ret += (r = lyb_read(data, &count, 1, lybs));
    LYB_HAVE_READ_GOTO(r, data, error);

    /* read attributes */
    for (i = 0; i < count; ++i) {
        ret += (r = lyb_read_start_subtree(data, lybs));
        LYB_HAVE_READ_GOTO(r, data, error);

        /* find model */
        ret += (r = lyb_parse_model(data, &mod, options, lybs));
        LYB_HAVE_READ_GOTO(r, data, error);

        if (mod) {
            /* annotation name */
            ret += (r = lyb_parse_attr_name(mod, data, &ext, options, lybs));
            LYB_HAVE_READ_GOTO(r, data, error);
        }

        if (!mod || !ext) {
            /* unknown attribute, skip it */
            do {
                ret += (r = lyb_read(data, NULL, lybs->written[lybs->used - 1], lybs));
                LYB_HAVE_READ_GOTO(r, data, error);
            } while (lybs->written[lybs->used - 1]);
            goto stop_subtree;
        }

        /* allocate new attribute */
        if (!attr) {
            assert(!node->attr);

            attr = calloc(1, sizeof *attr);
            LY_CHECK_ERR_GOTO(!attr, LOGMEM(lybs->ctx), error);

            node->attr = attr;
        } else {
            attr->next = calloc(1, sizeof *attr);
            LY_CHECK_ERR_GOTO(!attr->next, LOGMEM(lybs->ctx), error);

            attr = attr->next;
        }

        /* attribute annotation */
        attr->annotation = ext;

        /* attribute name */
        attr->name = lydict_insert(lybs->ctx, attr->annotation->arg_value, 0);

        /* get the type */
        type = (struct lys_type **)lys_ext_complex_get_substmt(LY_STMT_TYPE, attr->annotation, NULL);
        if (!type || !(*type)) {
            goto error;
        }

        /* attribute value */
        ret += (r = lyb_parse_value(*type, NULL, attr, data, unres, lybs));
        LYB_HAVE_READ_GOTO(r, data, error);

stop_subtree:
        lyb_read_stop_subtree(lybs);
    }

    return ret;

error:
    lyd_free_attr(lybs->ctx, node, node->attr, 1);
    return -1;
}

static int
lyb_is_schema_hash_match(struct lys_node *sibling, LYB_HASH *hash, uint8_t hash_count)
{
    LYB_HASH sibling_hash;
    uint8_t i;

    /* compare all the hashes starting from collision ID 0 */
    for (i = 0; i < hash_count; ++i) {
        sibling_hash = lyb_hash(sibling, i);
        if (sibling_hash != hash[i]) {
            return 0;
        }
    }

    return 1;
}

static int
lyb_parse_schema_hash(const struct lys_node *sparent, const struct lys_module *mod, const char *data, const char *yang_data_name,
                      int options, struct lys_node **snode, struct lyb_state *lybs)
{
    int r, ret = 0;
    uint8_t i, j;
    struct lys_node *sibling;
    LYB_HASH hash[LYB_HASH_BITS - 1];

    assert((sparent || mod) && (!sparent || !mod));

    /* read the first hash */
    ret += (r = lyb_read(data, &hash[0], sizeof *hash, lybs));
    LYB_HAVE_READ_RETURN(r, data, -1);

    /* based on the first hash read all the other ones, if any */
    for (i = 0; !(hash[0] & (LYB_HASH_COLLISION_ID >> i)); ++i) {
        if (i > LYB_HASH_BITS) {
            return -1;
        }
    }

    /* move the first hash on its accurate position */
    hash[i] = hash[0];

    /* read the rest of hashes */
    for (j = i; j; --j) {
        ret += (r = lyb_read(data, &hash[j - 1], sizeof *hash, lybs));
        LYB_HAVE_READ_RETURN(r, data, -1);

        /* correct collision ID */
        assert(hash[j - 1] & (LYB_HASH_COLLISION_ID >> (j - 1)));
        /* preceded with zeros */
        assert(!(hash[j - 1] & (LYB_HASH_MASK << (LYB_HASH_BITS - (j - 1)))));
    }

    /* handle yang data templates */
    if ((options & LYD_OPT_DATA_TEMPLATE) && yang_data_name && mod) {
        sparent = lyp_get_yang_data_template(mod, yang_data_name, strlen(yang_data_name));
        if (!sparent) {
            sibling = NULL;
            goto finish;
        }
    }

    /* handle RPC/action input/output */
    if (sparent && (sparent->nodetype & (LYS_RPC | LYS_ACTION))) {
        sibling = NULL;
        while ((sibling = (struct lys_node *)lys_getnext(sibling, sparent, NULL, LYS_GETNEXT_WITHINOUT))) {
            if ((sibling->nodetype == LYS_INPUT) && (options & LYD_OPT_RPC)) {
                break;
            }
            if ((sibling->nodetype == LYS_OUTPUT) && (options & LYD_OPT_RPCREPLY)) {
                break;
            }
        }
        if (!sibling) {
            /* fail */
            goto finish;
        }

        /* use only input/output children nodes */
        sparent = sibling;
    }

    /* find our node with matching hashes */
    sibling = NULL;
    while ((sibling = (struct lys_node *)lys_getnext(sibling, sparent, mod, 0))) {
        /* skip schema nodes from models not present during printing */
        if (lyb_has_schema_model(sibling, lybs->models, lybs->mod_count) && lyb_is_schema_hash_match(sibling, hash, i + 1)) {
            /* match found */
            break;
        }
    }

finish:
    *snode = sibling;
    if (!sibling && (options & LYD_OPT_STRICT)) {
        if (mod) {
            LOGVAL(lybs->ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Failed to find matching hash for a top-level node from \"%s\".",
                    mod->name);
        } else {
            LOGVAL(lybs->ctx, LYE_SPEC, LY_VLOG_LYS, sparent, "Failed to find matching hash for a child of \"%s\".",
                    sparent->name);
        }
        return -1;
    }

    return ret;
}

static int
lyb_skip_subtree(const char *data, struct lyb_state *lybs)
{
    int r, ret = 0;

    do {
        /* first skip any meta information inside */
        r = lybs->inner_chunks[lybs->used - 1] * LYB_META_BYTES;
        data += r;
        ret += r;

        /* then read data */
        ret += (r = lyb_read(data, NULL, lybs->written[lybs->used - 1], lybs));
        LYB_HAVE_READ_RETURN(r, data, -1);
    } while (lybs->written[lybs->used - 1]);

    return ret;
}

static int
lyb_parse_subtree(const char *data, struct lyd_node *parent, struct lyd_node **first_sibling, const char *yang_data_name,
        int options, struct unres_data *unres, struct lyb_state *lybs)
{
    int r, ret = 0;
    struct lyd_node *node = NULL, *iter;
    const struct lys_module *mod;
    struct lys_node *snode;

    assert((parent && !first_sibling) || (!parent && first_sibling));

    /* register a new subtree */
    ret += (r = lyb_read_start_subtree(data, lybs));
    LYB_HAVE_READ_GOTO(r, data, error);

    if (!parent) {
        /* top-level, read module name */
        ret += (r = lyb_parse_model(data, &mod, options, lybs));
        LYB_HAVE_READ_GOTO(r, data, error);

        if (mod) {
            /* read hash, find the schema node starting from mod, possibly yang_data_name */
            r = lyb_parse_schema_hash(NULL, mod, data, yang_data_name, options, &snode, lybs);
        }
    } else {
        mod = lyd_node_module(parent);

        /* read hash, find the schema node starting from parent schema */
        r = lyb_parse_schema_hash(parent->schema, NULL, data, NULL, options, &snode, lybs);
    }
    ret += r;
    LYB_HAVE_READ_GOTO(r, data, error);

    if (!mod || !snode) {
        /* unknown data subtree, skip it whole */
        ret += (r = lyb_skip_subtree(data, lybs));
        LYB_HAVE_READ_GOTO(r, data, error);
        goto stop_subtree;
    }

    /*
     * read the node
     */
    node = lyb_new_node(snode, options);
    if (!node) {
        goto error;
    }

    ret += (r = lyb_parse_attributes(node, data, options, unres, lybs));
    LYB_HAVE_READ_GOTO(r, data, error);

    /* read node content */
    switch (snode->nodetype) {
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_NOTIF:
    case LYS_RPC:
    case LYS_ACTION:
        /* nothing to read */
        break;
    case LYS_LEAF:
    case LYS_LEAFLIST:
        ret += (r = lyb_parse_value(&((struct lys_node_leaf *)node->schema)->type, (struct lyd_node_leaf_list *)node,
                                    NULL, data, unres, lybs));
        LYB_HAVE_READ_GOTO(r, data, error);
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        ret += (r = lyb_parse_anydata(node, data, lybs));
        LYB_HAVE_READ_GOTO(r, data, error);
        break;
    default:
        goto error;
    }

    /* insert into data tree, manually */
    if (parent) {
        if (!parent->child) {
            /* only child */
            parent->child = node;
        } else {
            /* last child */
            parent->child->prev->next = node;
            node->prev = parent->child->prev;
            parent->child->prev = node;
        }
        node->parent = parent;
    } else if (*first_sibling) {
        /* last sibling */
        (*first_sibling)->prev->next = node;
        node->prev = (*first_sibling)->prev;
        (*first_sibling)->prev = node;
    } else {
        /* only sibling */
        *first_sibling = node;
    }

    /* read all descendants */
    while (lybs->written[lybs->used - 1]) {
        ret += (r = lyb_parse_subtree(data, node, NULL, NULL, options, unres, lybs));
        LYB_HAVE_READ_GOTO(r, data, error);
    }

    /* make containers default if should be */
    if ((node->schema->nodetype == LYS_CONTAINER) && !((struct lys_node_container *)node->schema)->presence) {
        LY_TREE_FOR(node->child, iter) {
            if (!iter->dflt) {
                break;
            }
        }

        if (!iter) {
            node->dflt = 1;
        }
    }

#ifdef LY_ENABLED_CACHE
    /* calculate the hash and insert it into parent (list with keys is handled when its keys are inserted) */
    if ((node->schema->nodetype != LYS_LIST) || !((struct lys_node_list *)node->schema)->keys_size) {
        lyd_hash(node);
        lyd_insert_hash(node);
    }
#endif

stop_subtree:
    /* end the subtree */
    lyb_read_stop_subtree(lybs);

    return ret;

error:
    lyd_free(node);
    if (first_sibling && (*first_sibling == node)) {
        *first_sibling = NULL;
    }
    return -1;
}

static int
lyb_parse_data_models(const char *data, int options, struct lyb_state *lybs)
{
    int i, r, ret = 0;

    /* read model count */
    ret += (r = lyb_read_number(&lybs->mod_count, sizeof lybs->mod_count, 2, data, lybs));
    LYB_HAVE_READ_RETURN(r, data, -1);

    if (lybs->mod_count) {
        lybs->models = malloc(lybs->mod_count * sizeof *lybs->models);
        LY_CHECK_ERR_RETURN(!lybs->models, LOGMEM(lybs->ctx), -1);

        /* read modules */
        for (i = 0; i < lybs->mod_count; ++i) {
            ret += (r = lyb_parse_model(data, &lybs->models[i], options, lybs));
            LYB_HAVE_READ_RETURN(r, data, -1);
        }
    }

    return ret;
}

static int
lyb_parse_magic_number(const char *data, struct lyb_state *lybs)
{
    int r, ret = 0;
    char magic_byte = 0;

    ret += (r = lyb_read(data, (uint8_t *)&magic_byte, 1, lybs));
    LYB_HAVE_READ_RETURN(r, data, -1);
    if (magic_byte != 'l') {
        LOGERR(lybs->ctx, LY_EINVAL, "Invalid first magic number byte \"0x%02x\".", magic_byte);
        return -1;
    }

    ret += (r = lyb_read(data, (uint8_t *)&magic_byte, 1, lybs));
    LYB_HAVE_READ_RETURN(r, data, -1);
    if (magic_byte != 'y') {
        LOGERR(lybs->ctx, LY_EINVAL, "Invalid second magic number byte \"0x%02x\".", magic_byte);
        return -1;
    }

    ret += (r = lyb_read(data, (uint8_t *)&magic_byte, 1, lybs));
    LYB_HAVE_READ_RETURN(r, data, -1);
    if (magic_byte != 'b') {
        LOGERR(lybs->ctx, LY_EINVAL, "Invalid third magic number byte \"0x%02x\".", magic_byte);
        return -1;
    }

    return ret;
}

static int
lyb_parse_header(const char *data, struct lyb_state *lybs)
{
    int ret = 0;
    uint8_t byte = 0;

    /* TODO version, any flags? */
    ret += lyb_read(data, (uint8_t *)&byte, sizeof byte, lybs);

    return ret;
}

struct lyd_node *
lyd_parse_lyb(struct ly_ctx *ctx, const char *data, int options, const struct lyd_node *data_tree,
              const char *yang_data_name, int *parsed)
{
    int r = 0, ret = 0;
    struct lyd_node *node = NULL, *next, *act_notif = NULL;
    struct unres_data *unres = NULL;
    struct lyb_state lybs;

    if (!ctx || !data) {
        LOGARG;
        return NULL;
    }

    lybs.written = malloc(LYB_STATE_STEP * sizeof *lybs.written);
    lybs.position = malloc(LYB_STATE_STEP * sizeof *lybs.position);
    lybs.inner_chunks = malloc(LYB_STATE_STEP * sizeof *lybs.inner_chunks);
    LY_CHECK_ERR_GOTO(!lybs.written || !lybs.position || !lybs.inner_chunks, LOGMEM(ctx), finish);
    lybs.used = 0;
    lybs.size = LYB_STATE_STEP;
    lybs.models = NULL;
    lybs.mod_count = 0;
    lybs.ctx = ctx;

    unres = calloc(1, sizeof *unres);
    LY_CHECK_ERR_GOTO(!unres, LOGMEM(ctx), finish);

    /* read magic number */
    ret += (r = lyb_parse_magic_number(data, &lybs));
    LYB_HAVE_READ_GOTO(r, data, finish);

    /* read header */
    ret += (r = lyb_parse_header(data, &lybs));
    LYB_HAVE_READ_GOTO(r, data, finish);

    /* read used models */
    ret += (r = lyb_parse_data_models(data, options, &lybs));
    LYB_HAVE_READ_GOTO(r, data, finish);

    /* read subtree(s) */
    while (data[0]) {
        ret += (r = lyb_parse_subtree(data, NULL, &node, yang_data_name, options, unres, &lybs));
        if (r < 0) {
            lyd_free_withsiblings(node);
            node = NULL;
            goto finish;
        }
        data += r;
    }

    /* read the last zero, parsing finished */
    ++ret;
    r = ret;

    if (options & LYD_OPT_DATA_ADD_YANGLIB) {
        if (lyd_merge(node, ly_ctx_info(ctx), LYD_OPT_DESTRUCT | LYD_OPT_EXPLICIT)) {
            LOGERR(ctx, LY_EINT, "Adding ietf-yang-library data failed.");
            lyd_free_withsiblings(node);
            node = NULL;
            goto finish;
        }
    }

    /* resolve any unresolved instance-identifiers */
    if (unres->count) {
        if (options & (LYD_OPT_RPC | LYD_OPT_RPCREPLY | LYD_OPT_NOTIF)) {
            LY_TREE_DFS_BEGIN(node, next, act_notif) {
                if (act_notif->schema->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)) {
                    break;
                }
                LY_TREE_DFS_END(node, next, act_notif);
            }
        }
        if (lyd_defaults_add_unres(&node, options, ctx, NULL, 0, data_tree, act_notif, unres, 0)) {
            lyd_free_withsiblings(node);
            node = NULL;
            goto finish;
        }
    }

finish:
    free(lybs.written);
    free(lybs.position);
    free(lybs.inner_chunks);
    free(lybs.models);
    if (unres) {
        free(unres->node);
        free(unres->type);
        free(unres);
    }

    if (parsed) {
        *parsed = r;
    }
    return node;
}

API int
lyd_lyb_data_length(const char *data)
{
    FUN_IN;

    struct lyb_state lybs;
    int r = 0, ret = 0, i;
    size_t len;
    uint8_t buf[LYB_SIZE_MAX];

    if (!data) {
        return -1;
    }

    lybs.written = malloc(LYB_STATE_STEP * sizeof *lybs.written);
    lybs.position = malloc(LYB_STATE_STEP * sizeof *lybs.position);
    lybs.inner_chunks = malloc(LYB_STATE_STEP * sizeof *lybs.inner_chunks);
    LY_CHECK_ERR_GOTO(!lybs.written || !lybs.position || !lybs.inner_chunks, LOGMEM(NULL), finish);
    lybs.used = 0;
    lybs.size = LYB_STATE_STEP;
    lybs.models = NULL;
    lybs.mod_count = 0;
    lybs.ctx = NULL;

    /* read magic number */
    ret += (r = lyb_parse_magic_number(data, &lybs));
    LYB_HAVE_READ_GOTO(r, data, finish);

    /* read header */
    ret += (r = lyb_parse_header(data, &lybs));
    LYB_HAVE_READ_GOTO(r, data, finish);

    /* read model count */
    ret += (r = lyb_read_number(&lybs.mod_count, sizeof lybs.mod_count, 2, data, &lybs));
    LYB_HAVE_READ_GOTO(r, data, finish);

    /* read all models */
    for (i = 0; i < lybs.mod_count; ++i) {
        /* module name length */
        len = 0;
        ret += (r = lyb_read_number(&len, sizeof len, 2, data, &lybs));
        LYB_HAVE_READ_GOTO(r, data, finish);

        /* model name */
        ret += (r = lyb_read(data, buf, len, &lybs));
        LYB_HAVE_READ_GOTO(r, data, finish);

        /* revision */
        ret += (r = lyb_read(data, buf, 2, &lybs));
        LYB_HAVE_READ_GOTO(r, data, finish);
    }

    while (data[0]) {
        /* register a new subtree */
        ret += (r = lyb_read_start_subtree(data, &lybs));
        LYB_HAVE_READ_GOTO(r, data, finish);

        /* skip it */
        ret += (r = lyb_skip_subtree(data, &lybs));
        LYB_HAVE_READ_GOTO(r, data, finish);

        /* subtree finished */
        lyb_read_stop_subtree(&lybs);
    }

    /* read the last zero, parsing finished */
    ++ret;

finish:
    free(lybs.written);
    free(lybs.position);
    free(lybs.inner_chunks);
    free(lybs.models);
    return ret;
}
