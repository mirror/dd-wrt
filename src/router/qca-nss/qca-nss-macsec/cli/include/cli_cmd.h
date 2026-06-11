/*
 * Copyright (c) 2014, 2019 The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _CLI_CMD_H
#define _CLI_CMD_H

#include <sys_base.h>
#include "cli_lib.h"

#define CLI_CMD_ERR_STR_LEN    800

#define phyId (pVty->data[0])
#define secyId (pVty->data[0])

#define CLI_EXEC_API(func)          \
do{                                 \
    int rc = func;                  \
    if (rc != 0){                   \
        char *emsg = cli_emsg_get(rc); \
        if(emsg){                   \
            vty_print_error("Shell Execute API fail.(MSG=%s)\n", emsg);\
        }else{                      \
            vty_print_error("Shell Execute Fun fail.(RET=%d)\n", rc);\
        }                           \
        return CLI_FAIL;            \
    }                               \
}while(0)

void cli_cmd_init(void);

int cli_cmd_basic_mode_init(void);

int cli_cmd_basic_init(void);

int cli_get_value_by_fun_name(const char *funcName, sa_u32_t *pValue);

int cli_install_mode_basic_cmds(int mode);

int cli_cmd_secy_init(void);

int cli_cmd_phy_init(void);

int cli_cmd_app_init(void);

int cli_cmd_optimus_init(void);

char *cli_emsg_get(int rc);

#endif /* _CLI_CMD_H */
