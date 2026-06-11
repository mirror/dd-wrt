/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
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

#ifndef __VTY__
#define __VTY__

#include <sys_base.h>
#include <stdarg.h>

#define VTY_CMDBUF_MAX_SIZE 512
#define VTY_HIST_MAX_SIZE   10
#define VTY_LINEBREAK       "\r\n"

#define VTY_DEFUALT_LINES   24
#define VTY_DEFUALT_WIDTH   80

#define VTY_PROMPT_MAX      64
#define VTY_ERR_STR_MAX     128

#define VTY_BUF_VAL_CP(self)        ((self)->data[(self)->cp])
#define VTY_BUF_VAL_PREV_CP(self)   ((self)->data[(self)->cp - 1])
#define VTY_BUF_VAL_NEXT_CP(self)   ((self)->data[(self)->cp + 1])

#define VTY_BUF_ASSIGN(self, buf, nbytes)   \
do {                                        \
    (self)->cp = 0;                         \
    (self)->len = (self)->max = (nbytes);   \
    (self)->data = (buf);                   \
} while(0)

#define VTY_BUF_ZERO(self)                  \
do {                                        \
    osal_memset((self)->data,0,(self)->max);\
    (self)->cp = (self)->len = 0;           \
} while(0)

#define VTY_BUF_FREE(self)                  \
do {                                        \
    if ((self)->max){                       \
        osal_free((self)->data);        \
    }                                       \
    (self)->data = NULL;                    \
    (self)->len = (self)->max = 0;          \
    (self)->cp = 0;                         \
} while(0)

#define vty_print(fmt, ...) \
    vty_output(pVty, fmt, ##__VA_ARGS__);

#define vty_print_warn(fmt, ...) \
    vty_output(pVty, "Warning: "fmt, ##__VA_ARGS__);

#define vty_print_error(fmt, ...) \
    vty_output(pVty, "%% "fmt, ##__VA_ARGS__);

#define vty_err_str(fmt, ...) \
    sprintf(pVty->errStr, fmt, ##__VA_ARGS__);

typedef enum {
	VTY_TYPE_CONSOLE,
	VTY_TYPE_TELNET,	/* Telnet not supported */
} VTY_TYPE_T;

typedef enum {
	VTY_STATE_NORMAL,
	VTY_STATE_CLOSE,
} VTY_STATE_T;

typedef enum {
	VTY_ESC_NONE,		/* Not an escape charactor */
	VTY_ESC_CH1,		/* an escape charactor of first charactor */
	VTY_ESC_CH2,		/* an escape charactor of second charactor */
} VTY_ESC_T;

typedef struct {
	sa_ch_t *data;
	int max;
	int cp;
	int len;
} VTY_BUF_T;

typedef struct {
	sa_ch_t *list[VTY_HIST_MAX_SIZE];
	int cp;
	int insert;
} VTY_HIST_T;

typedef enum {
	VTY_MODE_FLUSH,		/* flush outputs, not consider vty window size */
	VTY_MODE_MORE,		/* if vty_output_more, once reach window size, stop output an wait for user-input */
	VTY_MODE_MORE_DROP,	/* user-input is exit, then other write more will be ignored */

	VTY_MODE_FORCED_FLUSH,	/* user defined, turn off VTY_MORE_MODE, just flush. */
} VTY_WRITE_MODE_T;

typedef struct vty {
	int fd;			/* output fd */
	int cliMode;		/* shell cli mode */
	VTY_TYPE_T type;	/* identify vty is console or telnet */
	VTY_STATE_T state;	/* vty's state, if quit, set state to close */
	VTY_ESC_T esc;		/* escape character */
	VTY_BUF_T line;		/* input line, command string */
	VTY_HIST_T hist;	/* command history */
	VTY_WRITE_MODE_T writeMode;	/* vty write mode */
	char prompt[VTY_PROMPT_MAX];	/* vty prompt string */
	void *pExecCmd;		/* pointer to command which is being executed */
	int errPosition;	/* pointer to first error position in line buffer */
	char errStr[VTY_ERR_STR_MAX];	/* vty prompt string */

	/* vty writed position */
	int writeLines;
	int writeWidth;
	int writeEndLine;

	/* vty window size */
	int lines;
	int width;

	/* data[0] = unitId, data[1]~data[3] = cli-temp data */
	sa_u32_t data[4];
} VTY_T;

VTY_T *vty_create(int fd);

void vty_close(VTY_T *pVty);

int vty_complete_cmd(VTY_T *, const char *, sa_bool_t);

int vty_output(VTY_T *, sa_ch_t *, ...);

int vty_input(VTY_T *, sa_u8_t *, int);

void vty_write(VTY_T *, sa_ch_t *, int);

#endif /* __VTY_H__ */
