/**
 * @file xml.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Generic XML parser implementation for libyang
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

#include "xml.h"

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compat.h"
#include "in_internal.h"
#include "out_internal.h"
#include "tree.h"
#include "tree_schema_internal.h"

/* Move input p by s characters, if EOF log with lyxml_ctx c */
#define move_input(c, s) \
    ly_in_skip(c->in, s); \
    LY_CHECK_ERR_RET(!c->in->current[0], LOGVAL(c->ctx, LY_VCODE_EOF), LY_EVALID)

/* Ignore whitespaces in the input string p */
#define ign_xmlws(c) \
    while (is_xmlws(*(c)->in->current)) { \
        if (*(c)->in->current == '\n') {  \
            LY_IN_NEW_LINE((c)->in);      \
        }                                 \
        ly_in_skip(c->in, 1);             \
    }

static LY_ERR lyxml_next_attr_content(struct lyxml_ctx *xmlctx, const char **value, size_t *value_len, ly_bool *ws_only,
        ly_bool *dynamic);

/**
 * @brief Ignore and skip any characters until the delim of the size delim_len is read, including the delim
 *
 * @param[in] xmlctx XML parser context to provide input handler and libyang context
 * @param[in] in input handler to read the data, it is updated only in case the section is correctly terminated.
 * @param[in] delim Delimiter to detect end of the section.
 * @param[in] delim_len Length of the delimiter string to use.
 * @param[in] sectname Section name to refer in error message.
 */
LY_ERR
skip_section(struct lyxml_ctx *xmlctx, const char *delim, size_t delim_len, const char *sectname)
{
    size_t i;
    register const char *input, *a, *b;
    uint64_t parsed = 0, newlines = 0;

    for (input = xmlctx->in->current; *input; ++input, ++parsed) {
        if (*input != *delim) {
            if (*input == '\n') {
                ++newlines;
            }
            continue;
        }
        a = input;
        b = delim;
        for (i = 0; i < delim_len; ++i) {
            if (*a++ != *b++) {
                break;
            }
        }
        if (i == delim_len) {
            /* delim found */
            xmlctx->in->line += newlines;
            ly_in_skip(xmlctx->in, parsed + delim_len);
            return LY_SUCCESS;
        }
    }

    /* delim not found,
     * do not update input handler to refer to the beginning of the section in error message */
    LOGVAL(xmlctx->ctx, LY_VCODE_NTERM, sectname);
    return LY_EVALID;
}

/**
 * @brief Check/Get an XML identifier from the input string.
 *
 * The identifier must have at least one valid character complying the name start character constraints.
 * The identifier is terminated by the first character, which does not comply to the name character constraints.
 *
 * See https://www.w3.org/TR/xml-names/#NT-NCName
 *
 * @param[in] xmlctx XML context.
 * @param[out] start Pointer to the start of the identifier.
 * @param[out] end Pointer ot the end of the identifier.
 * @return LY_ERR value.
 */
static LY_ERR
lyxml_parse_identifier(struct lyxml_ctx *xmlctx, const char **start, const char **end)
{
    const char *s, *in;
    uint32_t c;
    size_t parsed;
    LY_ERR rc;

    in = s = xmlctx->in->current;

    /* check NameStartChar (minus colon) */
    LY_CHECK_ERR_RET(ly_getutf8(&in, &c, &parsed),
            LOGVAL(xmlctx->ctx, LY_VCODE_INCHAR, in[0]),
            LY_EVALID);
    LY_CHECK_ERR_RET(!is_xmlqnamestartchar(c),
            LOGVAL(xmlctx->ctx, LYVE_SYNTAX, "Identifier \"%s\" starts with an invalid character.", in - parsed),
            LY_EVALID);

    /* check rest of the identifier */
    do {
        /* move only successfully parsed bytes */
        ly_in_skip(xmlctx->in, parsed);

        rc = ly_getutf8(&in, &c, &parsed);
        LY_CHECK_ERR_RET(rc, LOGVAL(xmlctx->ctx, LY_VCODE_INCHAR, in[0]), LY_EVALID);
    } while (is_xmlqnamechar(c));

    *start = s;
    *end = xmlctx->in->current;
    return LY_SUCCESS;
}

/**
 * @brief Add namespace definition into XML context.
 *
 * Namespaces from a single element are supposed to be added sequentially together (not interleaved by a namespace from other
 * element). This mimic namespace visibility, since the namespace defined in element E is not visible from its parents or
 * siblings. On the other hand, namespace from a parent element can be redefined in a child element. This is also reflected
 * by lyxml_ns_get() which returns the most recent namespace definition for the given prefix.
 *
 * When leaving processing of a subtree of some element (after it is removed from xmlctx->elements), caller is supposed to call
 * lyxml_ns_rm() to remove all the namespaces defined in such an element from the context.
 *
 * @param[in] xmlctx XML context to work with.
 * @param[in] prefix Pointer to the namespace prefix. Can be NULL for default namespace.
 * @param[in] prefix_len Length of the prefix.
 * @param[in] uri Namespace URI (value) to store directly. Value is always spent.
 * @return LY_ERR values.
 */
LY_ERR
lyxml_ns_add(struct lyxml_ctx *xmlctx, const char *prefix, size_t prefix_len, char *uri)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxml_ns *ns;

    ns = malloc(sizeof *ns);
    LY_CHECK_ERR_RET(!ns, LOGMEM(xmlctx->ctx), LY_EMEM);

    /* we need to connect the depth of the element where the namespace is defined with the
     * namespace record to be able to maintain (remove) the record when the parser leaves
     * (to its sibling or back to the parent) the element where the namespace was defined */
    ns->depth = xmlctx->elements.count;

    ns->uri = uri;
    if (prefix) {
        ns->prefix = strndup(prefix, prefix_len);
        LY_CHECK_ERR_RET(!ns->prefix, LOGMEM(xmlctx->ctx); free(ns->uri); free(ns), LY_EMEM);
    } else {
        ns->prefix = NULL;
    }

    ret = ly_set_add(&xmlctx->ns, ns, 1, NULL);
    LY_CHECK_ERR_RET(ret, free(ns->prefix); free(ns->uri); free(ns), ret);

    return LY_SUCCESS;
}

/**
 * @brief Remove all the namespaces defined in the element recently closed (removed from the xmlctx->elements).
 *
 * @param[in] xmlctx XML context to work with.
 */
void
lyxml_ns_rm(struct lyxml_ctx *xmlctx)
{
    for (uint32_t u = xmlctx->ns.count - 1; u + 1 > 0; --u) {
        if (((struct lyxml_ns *)xmlctx->ns.objs[u])->depth != xmlctx->elements.count + 1) {
            /* we are done, the namespaces from a single element are supposed to be together */
            break;
        }
        /* remove the ns structure */
        free(((struct lyxml_ns *)xmlctx->ns.objs[u])->prefix);
        free(((struct lyxml_ns *)xmlctx->ns.objs[u])->uri);
        free(xmlctx->ns.objs[u]);
        --xmlctx->ns.count;
    }

    if (!xmlctx->ns.count) {
        /* cleanup the xmlctx's namespaces storage */
        ly_set_erase(&xmlctx->ns, NULL);
    }
}

const struct lyxml_ns *
lyxml_ns_get(const struct ly_set *ns_set, const char *prefix, size_t prefix_len)
{
    struct lyxml_ns *ns;

    for (uint32_t u = ns_set->count - 1; u + 1 > 0; --u) {
        ns = (struct lyxml_ns *)ns_set->objs[u];
        if (prefix && prefix_len) {
            if (ns->prefix && !ly_strncmp(ns->prefix, prefix, prefix_len)) {
                return ns;
            }
        } else if (!ns->prefix) {
            /* default namespace */
            return ns;
        }
    }

    return NULL;
}

/**
 * @brief Skip in the input until EOF or just after the opening tag.
 * Handles special XML constructs (comment, cdata, doctype).
 *
 * @param[in] xmlctx XML context to use.
 * @return LY_ERR value.
 */
static LY_ERR
lyxml_skip_until_end_or_after_otag(struct lyxml_ctx *xmlctx)
{
    const struct ly_ctx *ctx = xmlctx->ctx; /* shortcut */
    const char *endtag, *sectname;
    size_t endtag_len;

    while (1) {
        ign_xmlws(xmlctx);

        if (xmlctx->in->current[0] == '\0') {
            /* EOF */
            if (xmlctx->elements.count) {
                LOGVAL(ctx, LY_VCODE_EOF);
                return LY_EVALID;
            }
            return LY_SUCCESS;
        } else if (xmlctx->in->current[0] != '<') {
            LOGVAL(ctx, LY_VCODE_INSTREXP, LY_VCODE_INSTREXP_len(xmlctx->in->current),
                    xmlctx->in->current, "element tag start ('<')");
            return LY_EVALID;
        }
        move_input(xmlctx, 1);

        if (xmlctx->in->current[0] == '!') {
            move_input(xmlctx, 1);
            /* sections to ignore */
            if (!strncmp(xmlctx->in->current, "--", 2)) {
                /* comment */
                move_input(xmlctx, 2);
                sectname = "Comment";
                endtag = "-->";
                endtag_len = ly_strlen_const("-->");
            } else if (!strncmp(xmlctx->in->current, "[CDATA[", ly_strlen_const("[CDATA["))) {
                /* CDATA section */
                move_input(xmlctx, ly_strlen_const("[CDATA["));
                sectname = "CData";
                endtag = "]]>";
                endtag_len = ly_strlen_const("]]>");
            } else if (!strncmp(xmlctx->in->current, "DOCTYPE", ly_strlen_const("DOCTYPE"))) {
                /* Document type declaration - not supported */
                LOGVAL(ctx,  LY_VCODE_NSUPP, "Document Type Declaration");
                return LY_EVALID;
            } else {
                LOGVAL(ctx, LYVE_SYNTAX, "Unknown XML section \"%.20s\".", &xmlctx->in->current[-2]);
                return LY_EVALID;
            }
            LY_CHECK_RET(skip_section(xmlctx, endtag, endtag_len, sectname));
        } else if (xmlctx->in->current[0] == '?') {
            LY_CHECK_RET(skip_section(xmlctx, "?>", 2, "Declaration"));
        } else {
            /* other non-WS character */
            break;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse QName.
 *
 * @param[in] xmlctx XML context to use.
 * @param[out] prefix Parsed prefix, may be NULL.
 * @param[out] prefix_len Length of @p prefix.
 * @param[out] name Parsed name.
 * @param[out] name_len Length of @p name.
 * @return LY_ERR value.
 */
static LY_ERR
lyxml_parse_qname(struct lyxml_ctx *xmlctx, const char **prefix, size_t *prefix_len, const char **name, size_t *name_len)
{
    const char *start, *end;

    *prefix = NULL;
    *prefix_len = 0;

    LY_CHECK_RET(lyxml_parse_identifier(xmlctx, &start, &end));
    if (end[0] == ':') {
        /* we have prefixed identifier */
        *prefix = start;
        *prefix_len = end - start;

        move_input(xmlctx, 1);
        LY_CHECK_RET(lyxml_parse_identifier(xmlctx, &start, &end));
    }

    *name = start;
    *name_len = end - start;
    return LY_SUCCESS;
}

/**
 * @brief Parse XML text content (value).
 *
 * @param[in] xmlctx XML context to use.
 * @param[in] endchar Expected character to mark value end.
 * @param[out] value Parsed value.
 * @param[out] length Length of @p value.
 * @param[out] ws_only Whether the value is empty/white-spaces only.
 * @param[out] dynamic Whether the value was dynamically allocated.
 * @return LY_ERR value.
 */
static LY_ERR
lyxml_parse_value(struct lyxml_ctx *xmlctx, char endchar, char **value, size_t *length, ly_bool *ws_only, ly_bool *dynamic)
{
#define BUFSIZE 24
#define BUFSIZE_STEP 128

    const struct ly_ctx *ctx = xmlctx->ctx; /* shortcut */
    const char *in = xmlctx->in->current, *start, *in_aux;
    char *buf = NULL;
    size_t offset;   /* read offset in input buffer */
    size_t len;      /* length of the output string (write offset in output buffer) */
    size_t size = 0; /* size of the output buffer */
    void *p;
    uint32_t n;
    size_t u;
    ly_bool ws = 1;

    assert(xmlctx);

    /* init */
    start = in;
    offset = len = 0;

    /* parse */
    while (in[offset]) {
        if (in[offset] == '&') {
            /* non WS */
            ws = 0;

            if (!buf) {
                /* prepare output buffer */
                buf = malloc(BUFSIZE);
                LY_CHECK_ERR_RET(!buf, LOGMEM(ctx), LY_EMEM);
                size = BUFSIZE;
            }

            /* allocate enough for the offset and next character,
             * we will need 4 bytes at most since we support only the predefined
             * (one-char) entities and character references */
            while (len + offset + 4 >= size) {
                buf = ly_realloc(buf, size + BUFSIZE_STEP);
                LY_CHECK_ERR_RET(!buf, LOGMEM(ctx), LY_EMEM);
                size += BUFSIZE_STEP;
            }

            if (offset) {
                /* store what we have so far */
                memcpy(&buf[len], in, offset);
                len += offset;
                in += offset;
                offset = 0;
            }

            ++offset;
            if (in[offset] != '#') {
                /* entity reference - only predefined references are supported */
                if (!strncmp(&in[offset], "lt;", ly_strlen_const("lt;"))) {
                    buf[len++] = '<';
                    in += ly_strlen_const("&lt;");
                } else if (!strncmp(&in[offset], "gt;", ly_strlen_const("gt;"))) {
                    buf[len++] = '>';
                    in += ly_strlen_const("&gt;");
                } else if (!strncmp(&in[offset], "amp;", ly_strlen_const("amp;"))) {
                    buf[len++] = '&';
                    in += ly_strlen_const("&amp;");
                } else if (!strncmp(&in[offset], "apos;", ly_strlen_const("apos;"))) {
                    buf[len++] = '\'';
                    in += ly_strlen_const("&apos;");
                } else if (!strncmp(&in[offset], "quot;", ly_strlen_const("quot;"))) {
                    buf[len++] = '\"';
                    in += ly_strlen_const("&quot;");
                } else {
                    LOGVAL(ctx, LYVE_SYNTAX, "Entity reference \"%.*s\" not supported, only predefined references allowed.",
                            10, &in[offset - 1]);
                    goto error;
                }
                offset = 0;
            } else {
                p = (void *)&in[offset - 1];
                /* character reference */
                ++offset;
                if (isdigit(in[offset])) {
                    for (n = 0; isdigit(in[offset]); offset++) {
                        n = (LY_BASE_DEC * n) + (in[offset] - '0');
                    }
                } else if ((in[offset] == 'x') && isxdigit(in[offset + 1])) {
                    for (n = 0, ++offset; isxdigit(in[offset]); offset++) {
                        if (isdigit(in[offset])) {
                            u = (in[offset] - '0');
                        } else if (in[offset] > 'F') {
                            u = LY_BASE_DEC + (in[offset] - 'a');
                        } else {
                            u = LY_BASE_DEC + (in[offset] - 'A');
                        }
                        n = (LY_BASE_HEX * n) + u;
                    }
                } else {
                    LOGVAL(ctx, LYVE_SYNTAX, "Invalid character reference \"%.*s\".", 12, p);
                    goto error;

                }

                LY_CHECK_ERR_GOTO(in[offset] != ';',
                        LOGVAL(ctx, LY_VCODE_INSTREXP,
                        LY_VCODE_INSTREXP_len(&in[offset]), &in[offset], ";"),
                        error);
                ++offset;
                LY_CHECK_ERR_GOTO(ly_pututf8(&buf[len], n, &u),
                        LOGVAL(ctx, LYVE_SYNTAX, "Invalid character reference \"%.*s\" (0x%08x).", 12, p, n),
                        error);
                len += u;
                in += offset;
                offset = 0;
            }
        } else if (in[offset] == endchar) {
            /* end of string */
            if (buf) {
                /* realloc exact size string */
                buf = ly_realloc(buf, len + offset + 1);
                LY_CHECK_ERR_RET(!buf, LOGMEM(ctx), LY_EMEM);
                size = len + offset + 1;
                memcpy(&buf[len], in, offset);

                /* set terminating NULL byte */
                buf[len + offset] = '\0';
            }
            len += offset;
            in += offset;
            goto success;
        } else {
            if (!is_xmlws(in[offset])) {
                /* non WS */
                ws = 0;
            }

            /* log lines */
            if (in[offset] == '\n') {
                LY_IN_NEW_LINE(xmlctx->in);
            }

            /* continue */
            in_aux = &in[offset];
            LY_CHECK_ERR_GOTO(ly_getutf8(&in_aux, &n, &u),
                    LOGVAL(ctx, LY_VCODE_INCHAR, in[offset]), error);
            offset += u;
        }
    }

    /* EOF reached before endchar */
    LOGVAL(ctx, LY_VCODE_EOF);

error:
    free(buf);
    return LY_EVALID;

success:
    if (buf) {
        *value = buf;
        *dynamic = 1;
    } else {
        *value = (char *)start;
        *dynamic = 0;
    }
    *length = len;
    *ws_only = ws;

    xmlctx->in->current = in;
    return LY_SUCCESS;

#undef BUFSIZE
#undef BUFSIZE_STEP
}

/**
 * @brief Parse XML closing element and match it to a stored starting element.
 *
 * @param[in] xmlctx XML context to use.
 * @param[in] prefix Expected closing element prefix.
 * @param[in] prefix_len Length of @p prefix.
 * @param[in] name Expected closing element name.
 * @param[in] name_len Length of @p name.
 * @param[in] empty Whether we are parsing a special "empty" element (with joined starting and closing tag) with no value.
 * @return LY_ERR value.
 */
static LY_ERR
lyxml_close_element(struct lyxml_ctx *xmlctx, const char *prefix, size_t prefix_len, const char *name, size_t name_len,
        ly_bool empty)
{
    struct lyxml_elem *e;

    /* match opening and closing element tags */
    if (!xmlctx->elements.count) {
        LOGVAL(xmlctx->ctx, LYVE_SYNTAX, "Stray closing element tag (\"%.*s\").",
                (int)name_len, name);
        return LY_EVALID;
    }

    e = (struct lyxml_elem *)xmlctx->elements.objs[xmlctx->elements.count - 1];
    if ((e->prefix_len != prefix_len) || (e->name_len != name_len) ||
            (prefix_len && strncmp(prefix, e->prefix, e->prefix_len)) || strncmp(name, e->name, e->name_len)) {
        LOGVAL(xmlctx->ctx, LYVE_SYNTAX, "Opening (\"%.*s%s%.*s\") and closing (\"%.*s%s%.*s\") elements tag mismatch.",
                (int)e->prefix_len, e->prefix ? e->prefix : "", e->prefix ? ":" : "", (int)e->name_len, e->name,
                (int)prefix_len, prefix ? prefix : "", prefix ? ":" : "", (int)name_len, name);
        return LY_EVALID;
    }

    /* opening and closing element tags matches, remove record from the opening tags list */
    ly_set_rm_index(&xmlctx->elements, xmlctx->elements.count - 1, free);

    /* remove also the namespaces connected with the element */
    lyxml_ns_rm(xmlctx);

    /* skip WS */
    ign_xmlws(xmlctx);

    /* special "<elem/>" element */
    if (empty && (xmlctx->in->current[0] == '/')) {
        move_input(xmlctx, 1);
    }

    /* parse closing tag */
    if (xmlctx->in->current[0] != '>') {
        LOGVAL(xmlctx->ctx, LY_VCODE_INSTREXP, LY_VCODE_INSTREXP_len(xmlctx->in->current),
                xmlctx->in->current, "element tag termination ('>')");
        return LY_EVALID;
    }

    /* move after closing tag without checking for EOF */
    ly_in_skip(xmlctx->in, 1);

    return LY_SUCCESS;
}

/**
 * @brief Store parsed opening element and parse any included namespaces.
 *
 * @param[in] xmlctx XML context to use.
 * @param[in] prefix Parsed starting element prefix.
 * @param[in] prefix_len Length of @p prefix.
 * @param[in] name Parsed starting element name.
 * @param[in] name_len Length of @p name.
 * @return LY_ERR value.
 */
static LY_ERR
lyxml_open_element(struct lyxml_ctx *xmlctx, const char *prefix, size_t prefix_len, const char *name, size_t name_len)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxml_elem *e;
    const char *prev_input;
    char *value;
    size_t parsed, value_len;
    ly_bool ws_only, dynamic, is_ns;
    uint32_t c;

    /* store element opening tag information */
    e = malloc(sizeof *e);
    LY_CHECK_ERR_RET(!e, LOGMEM(xmlctx->ctx), LY_EMEM);
    e->name = name;
    e->prefix = prefix;
    e->name_len = name_len;
    e->prefix_len = prefix_len;

    LY_CHECK_RET(ly_set_add(&xmlctx->elements, e, 1, NULL));
    if (xmlctx->elements.count > LY_MAX_BLOCK_DEPTH) {
        LOGERR(xmlctx->ctx, LY_EINVAL,
                "The maximum number of open elements has been exceeded.");
        ret = LY_EINVAL;
        goto cleanup;
    }

    /* skip WS */
    ign_xmlws(xmlctx);

    /* parse and store all namespaces */
    prev_input = xmlctx->in->current;
    is_ns = 1;
    while ((xmlctx->in->current[0] != '\0') && !(ret = ly_getutf8(&xmlctx->in->current, &c, &parsed))) {
        if (!is_xmlqnamestartchar(c)) {
            break;
        }
        xmlctx->in->current -= parsed;

        /* parse attribute name */
        LY_CHECK_GOTO(ret = lyxml_parse_qname(xmlctx, &prefix, &prefix_len, &name, &name_len), cleanup);

        /* parse the value */
        LY_CHECK_GOTO(ret = lyxml_next_attr_content(xmlctx, (const char **)&value, &value_len, &ws_only, &dynamic), cleanup);

        /* store every namespace */
        if ((prefix && !ly_strncmp("xmlns", prefix, prefix_len)) || (!prefix && !ly_strncmp("xmlns", name, name_len))) {
            ret = lyxml_ns_add(xmlctx, prefix ? name : NULL, prefix ? name_len : 0,
                    dynamic ? value : strndup(value, value_len));
            dynamic = 0;
            LY_CHECK_GOTO(ret, cleanup);
        } else {
            /* not a namespace */
            is_ns = 0;
        }
        if (dynamic) {
            free(value);
        }

        /* skip WS */
        ign_xmlws(xmlctx);

        if (is_ns) {
            /* we can actually skip all the namespaces as there is no reason to parse them again */
            prev_input = xmlctx->in->current;
        }
    }

cleanup:
    if (!ret) {
        xmlctx->in->current = prev_input;
    }
    return ret;
}

/**
 * @brief Move parser to the attribute content and parse it.
 *
 * @param[in] xmlctx XML context to use.
 * @param[out] value Parsed attribute value.
 * @param[out] value_len Length of @p value.
 * @param[out] ws_only Whether the value is empty/white-spaces only.
 * @param[out] dynamic Whether the value was dynamically allocated.
 * @return LY_ERR value.
 */
static LY_ERR
lyxml_next_attr_content(struct lyxml_ctx *xmlctx, const char **value, size_t *value_len, ly_bool *ws_only, ly_bool *dynamic)
{
    char quot;

    /* skip WS */
    ign_xmlws(xmlctx);

    /* skip '=' */
    if (xmlctx->in->current[0] == '\0') {
        LOGVAL(xmlctx->ctx, LY_VCODE_EOF);
        return LY_EVALID;
    } else if (xmlctx->in->current[0] != '=') {
        LOGVAL(xmlctx->ctx, LY_VCODE_INSTREXP, LY_VCODE_INSTREXP_len(xmlctx->in->current),
                xmlctx->in->current, "'='");
        return LY_EVALID;
    }
    move_input(xmlctx, 1);

    /* skip WS */
    ign_xmlws(xmlctx);

    /* find quotes */
    if (xmlctx->in->current[0] == '\0') {
        LOGVAL(xmlctx->ctx, LY_VCODE_EOF);
        return LY_EVALID;
    } else if ((xmlctx->in->current[0] != '\'') && (xmlctx->in->current[0] != '\"')) {
        LOGVAL(xmlctx->ctx, LY_VCODE_INSTREXP, LY_VCODE_INSTREXP_len(xmlctx->in->current),
                xmlctx->in->current, "either single or double quotation mark");
        return LY_EVALID;
    }

    /* remember quote */
    quot = xmlctx->in->current[0];
    move_input(xmlctx, 1);

    /* parse attribute value */
    LY_CHECK_RET(lyxml_parse_value(xmlctx, quot, (char **)value, value_len, ws_only, dynamic));

    /* move after ending quote (without checking for EOF) */
    ly_in_skip(xmlctx->in, 1);

    return LY_SUCCESS;
}

/**
 * @brief Move parser to the next attribute and parse it.
 *
 * @param[in] xmlctx XML context to use.
 * @param[out] prefix Parsed attribute prefix.
 * @param[out] prefix_len Length of @p prefix.
 * @param[out] name Parsed attribute name.
 * @param[out] name_len Length of @p name.
 * @return LY_ERR value.
 */
static LY_ERR
lyxml_next_attribute(struct lyxml_ctx *xmlctx, const char **prefix, size_t *prefix_len, const char **name, size_t *name_len)
{
    const char *in;
    char *value;
    uint32_t c;
    size_t parsed, value_len;
    ly_bool ws_only, dynamic;

    /* skip WS */
    ign_xmlws(xmlctx);

    /* parse only possible attributes */
    while ((xmlctx->in->current[0] != '>') && (xmlctx->in->current[0] != '/')) {
        in = xmlctx->in->current;
        if (in[0] == '\0') {
            LOGVAL(xmlctx->ctx, LY_VCODE_EOF);
            return LY_EVALID;
        } else if ((ly_getutf8(&in, &c, &parsed) || !is_xmlqnamestartchar(c))) {
            LOGVAL(xmlctx->ctx, LY_VCODE_INSTREXP, LY_VCODE_INSTREXP_len(in - parsed), in - parsed,
                    "element tag end ('>' or '/>') or an attribute");
            return LY_EVALID;
        }

        /* parse attribute name */
        LY_CHECK_RET(lyxml_parse_qname(xmlctx, prefix, prefix_len, name, name_len));

        if ((!*prefix || ly_strncmp("xmlns", *prefix, *prefix_len)) && (*prefix || ly_strncmp("xmlns", *name, *name_len))) {
            /* standard attribute */
            break;
        }

        /* namespace, skip it */
        LY_CHECK_RET(lyxml_next_attr_content(xmlctx, (const char **)&value, &value_len, &ws_only, &dynamic));
        if (dynamic) {
            free(value);
        }

        /* skip WS */
        ign_xmlws(xmlctx);
    }

    return LY_SUCCESS;
}

/**
 * @brief Move parser to the next element and parse it.
 *
 * @param[in] xmlctx XML context to use.
 * @param[out] prefix Parsed element prefix.
 * @param[out] prefix_len Length of @p prefix.
 * @param[out] name Parse element name.
 * @param[out] name_len Length of @p name.
 * @param[out] closing Flag if the element is closing (includes '/').
 * @return LY_ERR value.
 */
static LY_ERR
lyxml_next_element(struct lyxml_ctx *xmlctx, const char **prefix, size_t *prefix_len, const char **name, size_t *name_len,
        ly_bool *closing)
{
    /* skip WS until EOF or after opening tag '<' */
    LY_CHECK_RET(lyxml_skip_until_end_or_after_otag(xmlctx));
    if (xmlctx->in->current[0] == '\0') {
        /* set return values */
        *prefix = *name = NULL;
        *prefix_len = *name_len = 0;
        return LY_SUCCESS;
    }

    if (xmlctx->in->current[0] == '/') {
        move_input(xmlctx, 1);
        *closing = 1;
    } else {
        *closing = 0;
    }

    /* skip WS */
    ign_xmlws(xmlctx);

    /* parse element name */
    LY_CHECK_RET(lyxml_parse_qname(xmlctx, prefix, prefix_len, name, name_len));

    return LY_SUCCESS;
}

LY_ERR
lyxml_ctx_new(const struct ly_ctx *ctx, struct ly_in *in, struct lyxml_ctx **xmlctx_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxml_ctx *xmlctx;
    ly_bool closing;

    /* new context */
    xmlctx = calloc(1, sizeof *xmlctx);
    LY_CHECK_ERR_RET(!xmlctx, LOGMEM(ctx), LY_EMEM);
    xmlctx->ctx = ctx;
    xmlctx->in = in;

    LOG_LOCINIT(NULL, NULL, NULL, in);

    /* parse next element, if any */
    LY_CHECK_GOTO(ret = lyxml_next_element(xmlctx, &xmlctx->prefix, &xmlctx->prefix_len, &xmlctx->name,
            &xmlctx->name_len, &closing), cleanup);

    if (xmlctx->in->current[0] == '\0') {
        /* update status */
        xmlctx->status = LYXML_END;
    } else if (closing) {
        LOGVAL(ctx, LYVE_SYNTAX, "Stray closing element tag (\"%.*s\").", (int)xmlctx->name_len, xmlctx->name);
        ret = LY_EVALID;
        goto cleanup;
    } else {
        /* open an element, also parses all enclosed namespaces */
        LY_CHECK_GOTO(ret = lyxml_open_element(xmlctx, xmlctx->prefix, xmlctx->prefix_len, xmlctx->name, xmlctx->name_len), cleanup);

        /* update status */
        xmlctx->status = LYXML_ELEMENT;
    }

cleanup:
    if (ret) {
        lyxml_ctx_free(xmlctx);
    } else {
        *xmlctx_p = xmlctx;
    }
    return ret;
}

LY_ERR
lyxml_ctx_next(struct lyxml_ctx *xmlctx)
{
    LY_ERR ret = LY_SUCCESS;
    ly_bool closing;
    struct lyxml_elem *e;

    /* if the value was not used, free it */
    if (((xmlctx->status == LYXML_ELEM_CONTENT) || (xmlctx->status == LYXML_ATTR_CONTENT)) && xmlctx->dynamic) {
        free((char *)xmlctx->value);
        xmlctx->value = NULL;
        xmlctx->dynamic = 0;
    }

    switch (xmlctx->status) {
    case LYXML_ELEM_CONTENT:
        /* content |</elem> */

        /* handle special case when empty content for "<elem/>" was returned */
        if (xmlctx->in->current[0] == '/') {
            assert(xmlctx->elements.count);
            e = (struct lyxml_elem *)xmlctx->elements.objs[xmlctx->elements.count - 1];

            /* close the element (parses closing tag) */
            ret = lyxml_close_element(xmlctx, e->prefix, e->prefix_len, e->name, e->name_len, 1);
            LY_CHECK_GOTO(ret, cleanup);

            /* update status */
            xmlctx->status = LYXML_ELEM_CLOSE;
            break;
        }
    /* fall through */
    case LYXML_ELEM_CLOSE:
        /* </elem>| <elem2>* */

        /* parse next element, if any */
        ret = lyxml_next_element(xmlctx, &xmlctx->prefix, &xmlctx->prefix_len, &xmlctx->name, &xmlctx->name_len, &closing);
        LY_CHECK_GOTO(ret, cleanup);

        if (xmlctx->in->current[0] == '\0') {
            /* update status */
            xmlctx->status = LYXML_END;
        } else if (closing) {
            /* close an element (parses also closing tag) */
            ret = lyxml_close_element(xmlctx, xmlctx->prefix, xmlctx->prefix_len, xmlctx->name, xmlctx->name_len, 0);
            LY_CHECK_GOTO(ret, cleanup);

            /* update status */
            xmlctx->status = LYXML_ELEM_CLOSE;
        } else {
            /* open an element, also parses all enclosed namespaces */
            ret = lyxml_open_element(xmlctx, xmlctx->prefix, xmlctx->prefix_len, xmlctx->name, xmlctx->name_len);
            LY_CHECK_GOTO(ret, cleanup);

            /* update status */
            xmlctx->status = LYXML_ELEMENT;
        }
        break;

    case LYXML_ELEMENT:
    /* <elem| attr='val'* > content  */
    case LYXML_ATTR_CONTENT:
        /* attr='val'| attr='val'* > content */

        /* parse attribute name, if any */
        ret = lyxml_next_attribute(xmlctx, &xmlctx->prefix, &xmlctx->prefix_len, &xmlctx->name, &xmlctx->name_len);
        LY_CHECK_GOTO(ret, cleanup);

        if (xmlctx->in->current[0] == '>') {
            /* no attributes but a closing tag */
            ly_in_skip(xmlctx->in, 1);
            if (!xmlctx->in->current[0]) {
                LOGVAL(xmlctx->ctx, LY_VCODE_EOF);
                ret = LY_EVALID;
                goto cleanup;
            }

            /* parse element content */
            ret = lyxml_parse_value(xmlctx, '<', (char **)&xmlctx->value, &xmlctx->value_len, &xmlctx->ws_only,
                    &xmlctx->dynamic);
            LY_CHECK_GOTO(ret, cleanup);

            if (!xmlctx->value_len) {
                /* empty value should by alocated staticaly, but check for in any case */
                if (xmlctx->dynamic) {
                    free((char *) xmlctx->value);
                }
                /* use empty value, easier to work with */
                xmlctx->value = "";
                xmlctx->dynamic = 0;
            }

            /* update status */
            xmlctx->status = LYXML_ELEM_CONTENT;
        } else if (xmlctx->in->current[0] == '/') {
            /* no content but we still return it */
            xmlctx->value = "";
            xmlctx->value_len = 0;
            xmlctx->ws_only = 1;
            xmlctx->dynamic = 0;

            /* update status */
            xmlctx->status = LYXML_ELEM_CONTENT;
        } else {
            /* update status */
            xmlctx->status = LYXML_ATTRIBUTE;
        }
        break;

    case LYXML_ATTRIBUTE:
        /* attr|='val' */

        /* skip formatting and parse value */
        ret = lyxml_next_attr_content(xmlctx, &xmlctx->value, &xmlctx->value_len, &xmlctx->ws_only, &xmlctx->dynamic);
        LY_CHECK_GOTO(ret, cleanup);

        /* update status */
        xmlctx->status = LYXML_ATTR_CONTENT;
        break;

    case LYXML_END:
        /* </elem>   |EOF */
        /* nothing to do */
        break;
    }

cleanup:
    if (ret) {
        /* invalidate context */
        xmlctx->status = LYXML_END;
    }
    return ret;
}

LY_ERR
lyxml_ctx_peek(struct lyxml_ctx *xmlctx, enum LYXML_PARSER_STATUS *next)
{
    LY_ERR ret = LY_SUCCESS;
    const char *prefix, *name, *prev_input;
    size_t prefix_len, name_len;
    ly_bool closing;

    prev_input = xmlctx->in->current;

    switch (xmlctx->status) {
    case LYXML_ELEM_CONTENT:
        if (xmlctx->in->current[0] == '/') {
            *next = LYXML_ELEM_CLOSE;
            break;
        }
    /* fall through */
    case LYXML_ELEM_CLOSE:
        /* parse next element, if any */
        ret = lyxml_next_element(xmlctx, &prefix, &prefix_len, &name, &name_len, &closing);
        LY_CHECK_GOTO(ret, cleanup);

        if (xmlctx->in->current[0] == '\0') {
            *next = LYXML_END;
        } else if (closing) {
            *next = LYXML_ELEM_CLOSE;
        } else {
            *next = LYXML_ELEMENT;
        }
        break;
    case LYXML_ELEMENT:
    case LYXML_ATTR_CONTENT:
        /* parse attribute name, if any */
        ret = lyxml_next_attribute(xmlctx, &prefix, &prefix_len, &name, &name_len);
        LY_CHECK_GOTO(ret, cleanup);

        if ((xmlctx->in->current[0] == '>') || (xmlctx->in->current[0] == '/')) {
            *next = LYXML_ELEM_CONTENT;
        } else {
            *next = LYXML_ATTRIBUTE;
        }
        break;
    case LYXML_ATTRIBUTE:
        *next = LYXML_ATTR_CONTENT;
        break;
    case LYXML_END:
        *next = LYXML_END;
        break;
    }

cleanup:
    xmlctx->in->current = prev_input;
    return ret;
}

void
lyxml_ctx_free(struct lyxml_ctx *xmlctx)
{
    uint32_t u;

    if (!xmlctx) {
        return;
    }

    LOG_LOCBACK(0, 0, 0, 1);

    if (((xmlctx->status == LYXML_ELEM_CONTENT) || (xmlctx->status == LYXML_ATTR_CONTENT)) && xmlctx->dynamic) {
        free((char *)xmlctx->value);
    }
    ly_set_erase(&xmlctx->elements, free);
    for (u = xmlctx->ns.count - 1; u + 1 > 0; --u) {
        /* remove the ns structure */
        free(((struct lyxml_ns *)xmlctx->ns.objs[u])->prefix);
        free(((struct lyxml_ns *)xmlctx->ns.objs[u])->uri);
        free(xmlctx->ns.objs[u]);
    }
    ly_set_erase(&xmlctx->ns, NULL);
    free(xmlctx);
}

LY_ERR
lyxml_dump_text(struct ly_out *out, const char *text, ly_bool attribute)
{
    LY_ERR ret;

    if (!text) {
        return 0;
    }

    for (uint64_t u = 0; text[u]; u++) {
        switch (text[u]) {
        case '&':
            ret = ly_print_(out, "&amp;");
            break;
        case '<':
            ret = ly_print_(out, "&lt;");
            break;
        case '>':
            /* not needed, just for readability */
            ret = ly_print_(out, "&gt;");
            break;
        case '"':
            if (attribute) {
                ret = ly_print_(out, "&quot;");
                break;
            }
        /* fall through */
        default:
            ret = ly_write_(out, &text[u], 1);
            break;
        }
        LY_CHECK_RET(ret);
    }

    return LY_SUCCESS;
}

LY_ERR
lyxml_value_compare(const struct ly_ctx *ctx1, const char *value1, void *val_prefix_data1,
        const struct ly_ctx *ctx2, const char *value2, void *val_prefix_data2)
{
    const char *value1_iter, *value2_iter;
    const char *value1_next, *value2_next;
    uint32_t value1_len, value2_len;
    ly_bool is_prefix1, is_prefix2;
    const struct lys_module *mod1, *mod2;
    LY_ERR ret;

    if (!value1 && !value2) {
        return LY_SUCCESS;
    }
    if ((value1 && !value2) || (!value1 && value2)) {
        return LY_ENOT;
    }

    if (!ctx2) {
        ctx2 = ctx1;
    }

    ret = LY_SUCCESS;
    for (value1_iter = value1, value2_iter = value2;
            value1_iter && value2_iter;
            value1_iter = value1_next, value2_iter = value2_next) {
        if ((ret = ly_value_prefix_next(value1_iter, NULL, &value1_len, &is_prefix1, &value1_next))) {
            break;
        }
        if ((ret = ly_value_prefix_next(value2_iter, NULL, &value2_len, &is_prefix2, &value2_next))) {
            break;
        }

        if (is_prefix1 != is_prefix2) {
            ret = LY_ENOT;
            break;
        }

        if (!is_prefix1) {
            if (value1_len != value2_len) {
                ret = LY_ENOT;
                break;
            }
            if (strncmp(value1_iter, value2_iter, value1_len)) {
                ret = LY_ENOT;
                break;
            }
            continue;
        }

        mod1 = mod2 = NULL;
        if (val_prefix_data1) {
            /* find module of the first prefix, if any */
            mod1 = ly_resolve_prefix(ctx1, value1_iter, value1_len, LY_VALUE_XML, val_prefix_data1);
        }
        if (val_prefix_data2) {
            mod2 = ly_resolve_prefix(ctx2, value2_iter, value2_len, LY_VALUE_XML, val_prefix_data2);
        }
        if (!mod1 || !mod2) {
            /* not a prefix or maps to different namespaces */
            ret = LY_ENOT;
            break;
        }

        if (mod1->ctx == mod2->ctx) {
            /* same contexts */
            if ((mod1->name != mod2->name) || (mod1->revision != mod2->revision)) {
                ret = LY_ENOT;
                break;
            }
        } else {
            /* different contexts */
            if (strcmp(mod1->name, mod2->name)) {
                ret = LY_ENOT;
                break;
            }

            if (mod1->revision || mod2->revision) {
                if (!mod1->revision || !mod2->revision) {
                    ret = LY_ENOT;
                    break;
                }
                if (strcmp(mod1->revision, mod2->revision)) {
                    ret = LY_ENOT;
                    break;
                }
            }
        }
    }

    if (value1_iter || value2_iter) {
        ret = LY_ENOT;
    }

    return ret;
}
