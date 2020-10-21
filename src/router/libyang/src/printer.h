/**
 * @file printer.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Printers for libyang
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PRINTER_H_
#define LY_PRINTER_H_

#include "libyang.h"
#include "tree_schema.h"
#include "tree_internal.h"

typedef enum LYOUT_TYPE {
    LYOUT_FD,          /**< file descriptor */
    LYOUT_STREAM,      /**< FILE stream */
    LYOUT_MEMORY,      /**< memory */
    LYOUT_CALLBACK     /**< print via provided callback */
} LYOUT_TYPE;

struct lyout {
    LYOUT_TYPE type;
    union {
        int fd;
        FILE *f;
        struct {
            char *buf;
            size_t len;
            size_t size;
        } mem;
        struct {
            ssize_t (*f)(void *arg, const void *buf, size_t count);
            void *arg;
        } clb;
    } method;

    /* buffer for holes */
    char *buffered;
    size_t buf_len;
    size_t buf_size;

    /* hole counter */
    size_t hole_count;
};

struct ext_substmt_info_s {
    const char *name;
    const char *arg;
    int flags;
#define SUBST_FLAG_YIN 0x1 /**< has YIN element */
#define SUBST_FLAG_ID 0x2  /**< the value is identifier -> no quotes */
};

#define LY_PRINT_SET errno = 0

#define LY_PRINT_RET(ctx) if (errno) { LOGERR(ctx, LY_ESYS, "Print error (%s).", strerror(errno)); return EXIT_FAILURE; } else \
        { return EXIT_SUCCESS; }

/* filled in printer.c */
extern struct ext_substmt_info_s ext_substmt_info[];

/**
 * @brief Generic printer, replacement for printf() / write() / etc
 */
int ly_print(struct lyout *out, const char *format, ...);
void ly_print_flush(struct lyout *out);
int ly_write(struct lyout *out, const char *buf, size_t count);
int ly_write_skip(struct lyout *out, size_t count, size_t *position);
int ly_write_skipped(struct lyout *out, size_t position, const char *buf, size_t count);

/* prefix_kind: 0 - print import prefixes for foreign features, 1 - print module names, 2 - print prefixes (tree printer), 3 - print module names including revisions (JSONS printer) */
int ly_print_iffeature(struct lyout *out, const struct lys_module *module, struct lys_iffeature *expr, int prefix_kind);

int yang_print_model(struct lyout *out, const struct lys_module *module);
int yin_print_model(struct lyout *out, const struct lys_module *module);
int tree_print_model(struct lyout *out, const struct lys_module *module, const char *target_schema_path, int line_length, int options);
int info_print_model(struct lyout *out, const struct lys_module *module, const char *target_schema_path);
int jsons_print_model(struct lyout *out, const struct lys_module *module, const char *target_schema_path);

int json_print_data(struct lyout *out, const struct lyd_node *root, int options);
int xml_print_data(struct lyout *out, const struct lyd_node *root, int options);
int xml_print_node(struct lyout *out, int level, const struct lyd_node *node, int toplevel, int options);
int lyb_print_data(struct lyout *out, const struct lyd_node *root, int options);

int lys_print_target(struct lyout *out, const struct lys_module *module, const char *target_schema_path,
                     void (*clb_print_typedef)(struct lyout*, const struct lys_tpdf*, int*),
                     void (*clb_print_identity)(struct lyout*, const struct lys_ident*, int*),
                     void (*clb_print_feature)(struct lyout*, const struct lys_feature*, int*),
                     void (*clb_print_type)(struct lyout*, const struct lys_type*, int*),
                     void (*clb_print_grouping)(struct lyout*, const struct lys_node*, int*),
                     void (*clb_print_container)(struct lyout*, const struct lys_node*, int*),
                     void (*clb_print_choice)(struct lyout*, const struct lys_node*, int*),
                     void (*clb_print_leaf)(struct lyout*, const struct lys_node*, int*),
                     void (*clb_print_leaflist)(struct lyout*, const struct lys_node*, int*),
                     void (*clb_print_list)(struct lyout*, const struct lys_node*, int*),
                     void (*clb_print_anydata)(struct lyout*, const struct lys_node*, int*),
                     void (*clb_print_case)(struct lyout*, const struct lys_node*, int*),
                     void (*clb_print_notif)(struct lyout*, const struct lys_node*, int*),
                     void (*clb_print_rpc)(struct lyout*, const struct lys_node*, int*),
                     void (*clb_print_action)(struct lyout*, const struct lys_node*, int*),
                     void (*clb_print_input)(struct lyout*, const struct lys_node*, int*),
                     void (*clb_print_output)(struct lyout*, const struct lys_node*, int*));

/* 0 - same, 1 - different */
int nscmp(const struct lyd_node *node1, const struct lyd_node *node2);

#endif /* LY_PRINTER_H_ */
