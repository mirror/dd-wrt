/**
 * @file json.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Generic JSON format parser routines.
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_JSON_H_
#define LY_JSON_H_

#include <stddef.h>
#include <stdint.h>

#include "log.h"
#include "set.h"

struct ly_ctx;
struct ly_in;

/* Macro to test if character is whitespace */
#define is_jsonws(c) (c == 0x20 || c == 0x9 || c == 0xa || c == 0xd)

/* Macro to test if character is valid string character */
#define is_jsonstrchar(c) (c == 0x20 || c == 0x21 || (c >= 0x23 && c <= 0x5b) || (c >= 0x5d && c <= 0x10ffff))

/**
 * @brief Status of the parser providing information what is expected next (which function is supposed to be called).
 */
enum LYJSON_PARSER_STATUS {
    LYJSON_ERROR,          /* JSON parser error - value is used as an error return code */
    LYJSON_ROOT,           /* JSON document root, used internally */
    LYJSON_OBJECT,         /* JSON object */
    LYJSON_OBJECT_CLOSED,  /* JSON object closed */
    LYJSON_OBJECT_EMPTY,   /* empty JSON object { }*/
    LYJSON_ARRAY,          /* JSON array */
    LYJSON_ARRAY_CLOSED,   /* JSON array closed */
    LYJSON_ARRAY_EMPTY,    /* empty JSON array */
    LYJSON_NUMBER,         /* JSON number value */
    LYJSON_STRING,         /* JSON string value */
    LYJSON_FALSE,          /* JSON false value */
    LYJSON_TRUE,           /* JSON true value */
    LYJSON_NULL,           /* JSON null value */
    LYJSON_END             /* end of input data */
};

struct lyjson_ctx {
    const struct ly_ctx *ctx;
    struct ly_in *in;       /* input structure */

    struct ly_set status;   /* stack of LYJSON_PARSER_STATUS values corresponding to the JSON items being processed */

    const char *value;      /* LYJSON_STRING, LYJSON_NUMBER, LYJSON_OBJECT */
    size_t value_len;       /* LYJSON_STRING, LYJSON_NUMBER, LYJSON_OBJECT */
    ly_bool dynamic;        /* LYJSON_STRING, LYJSON_NUMBER, LYJSON_OBJECT */
    uint32_t depth;         /* current number of nested blocks, see ::LY_MAX_BLOCK_DEPTH */

    struct {
        enum LYJSON_PARSER_STATUS status;
        uint32_t status_count;
        const char *value;
        size_t value_len;
        ly_bool dynamic;
        uint32_t depth;
        const char *input;
    } backup;
};

/**
 * @brief Create a new JSON parser context and start parsing.
 *
 * @param[in] ctx libyang context.
 * @param[in] in JSON string data to parse.
 * @param[out] jsonctx New JSON context with status ::LYJSON_VALUE.
 * @return LY_ERR value.
 */
LY_ERR lyjson_ctx_new(const struct ly_ctx *ctx,  struct ly_in *in, struct lyjson_ctx **jsonctx);

/**
 * @brief Get status of the parser as the last/previous parsed token
 *
 * @param[in] jsonctx JSON context to check.
 * @param[in] index Index of the token, starting by 0 for the last token
 * @return LYJSON_ERROR in case of invalid index, other LYJSON_PARSER_STATUS corresponding to the token.
 */
enum LYJSON_PARSER_STATUS lyjson_ctx_status(struct lyjson_ctx *jsonctx, uint32_t index);

/**
 * @brief Get string representation of the JSON context status (token).
 *
 * @param[in] status Context status (aka JSON token)
 * @return String representation of the @p status.
 */
const char *lyjson_token2str(enum LYJSON_PARSER_STATUS status);

/**
 * @brief Move to the next JSON artefact and update parser status.
 *
 * @param[in] jsonctx XML context to move.
 * @param[out] status Optional parameter to provide new parser status
 * @return LY_ERR value.
 */
LY_ERR lyjson_ctx_next(struct lyjson_ctx *jsonctx, enum LYJSON_PARSER_STATUS *status);

/**
 * @brief Backup the JSON parser context's state To restore the backup, use ::lyjson_ctx_restore().
 * @param[in] jsonctx JSON parser context to backup.
 */
void lyjson_ctx_backup(struct lyjson_ctx *jsonctx);

/**
 * @brief REstore the JSON parser context's state from the backup created by ::lyjson_ctx_backup().
 * @param[in] jsonctx JSON parser context to restore.
 */
void lyjson_ctx_restore(struct lyjson_ctx *jsonctx);

/**
 * @brief Remove the allocated working memory of the context.
 *
 * @param[in] jsonctx JSON context to clear.
 */
void lyjson_ctx_free(struct lyjson_ctx *jsonctx);

#endif /* LY_JSON_H_ */
