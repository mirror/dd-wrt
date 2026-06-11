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

//#include <ctype.h>
//#include <osal_common.h>
#include <shell_entity.h>

#include <vty.h>
#include <cli.h>

typedef struct {
	sa_u32_t key;
	void (*func) (VTY_T *);
} VTY_KET_BIND_T;

#define CHAR_CTRL(X)    ((X) - 64)
#define CHAR_BS         8
#define CHAR_ESC        27
#define CHAR_SPACE      ' '
#define CHAR_TAB        '\t'
#define CHAR_HELP       '?'
#define CHAR_LF         '\r'
#define CHAR_CR         '\n'

#define CHAR_PRT_MIN    32
#define CHAR_PRT_MAX    126
#define CHAR_PRINTABLE(c) ((c) >= CHAR_PRT_MIN && (c) <= CHAR_PRT_MAX)

#define VTY_WELCOME_MESSAGE "\r\n"\
"************************************************************************\r\n"\
"*                                                                      *\r\n"\
"*                Welcome to MACsec Console                             *\r\n"\
"*                                                                      *\r\n"\
"************************************************************************\r\n"

void vty_write(VTY_T *pVty, sa_ch_t *buf, int nbytes)
{
	switch (pVty->writeMode) {
	case VTY_MODE_FORCED_FLUSH:
	case VTY_MODE_FLUSH:
		shell_write(pVty->fd, buf, nbytes);
		break;

	case VTY_MODE_MORE:
		/* MORE not yet implemented in this engine */
		shell_write(pVty->fd, buf, nbytes);
		break;

	case VTY_MODE_MORE_DROP:
		break;

	default:
		break;
	}
}

static void _vty_putchar(VTY_T *pVty, sa_ch_t ch, int times)
{
	sa_ch_t *pt = NULL;

	if (times < 1) {
		return;
	}

	pt = osal_malloc(times);
	if (!(pt)) {
		return;
	}

	osal_memset(pt, ch, times);

	vty_write(pVty, pt, times);

	osal_free(pt);
}

int vty_output(VTY_T *pVty, sa_ch_t *format, ...)
{
	va_list args;
	int size, ptLen;
	sa_ch_t buf[512] = { 0 };
	sa_ch_t *pt;

	pt = buf;
	ptLen = sizeof buf;

	va_start(args, format);

	size = vsnprintf(pt, ptLen, format, args);

	if (size < 0 || size > ptLen) {
		size = ptLen;
	}

	vty_write(pVty, pt, size);

	va_end(args);

	return size;
}

void _vty_welcome(VTY_T *pVty)
{
	vty_output(pVty, VTY_WELCOME_MESSAGE);
}

static void _vty_prompt(VTY_T *pVty)
{
	vty_output(pVty, "%s ", pVty->prompt);
}

static void _vty_redraw_line(VTY_T *pVty)
{
	vty_write(pVty, pVty->line.data, pVty->line.len);

	pVty->line.cp = pVty->line.len;
}

static void _vty_cmd_insert_char(VTY_T *pVty, sa_ch_t c)
{
	int length;
	VTY_BUF_T *pBuf = &pVty->line;
	sa_ch_t *tpt = NULL;
	int tmax = 0;

	if (pBuf->max <= pBuf->len + 1) {
		tpt = pBuf->data;
		tmax = pBuf->max;

		pBuf->max *= 2;

		pBuf->data = osal_malloc(pBuf->max);
		if (pBuf->data == NULL) {
			return;
		}

		osal_memset(pBuf->data, 0, pBuf->max);
		osal_memcpy(pBuf->data, tpt, tmax);
		osal_free(tpt);
		tpt = NULL;
	}

	length = pBuf->len - pBuf->cp;

	memmove(&VTY_BUF_VAL_NEXT_CP(pBuf), &VTY_BUF_VAL_CP(pBuf), length);

	VTY_BUF_VAL_CP(pBuf) = c;

	vty_write(pVty, &VTY_BUF_VAL_CP(pBuf), length + 1);

	_vty_putchar(pVty, CHAR_BS, length);

	pBuf->cp++;
	pBuf->len++;
}

static void _vty_cmd_forward_char(VTY_T *pVty)
{
	VTY_BUF_T *pBuf = &pVty->line;

	if (pBuf->cp < pBuf->len) {
		vty_write(pVty, &VTY_BUF_VAL_CP(pBuf), 1);

		pBuf->cp++;
	}
}

static void _vty_cmd_backward_char(VTY_T *pVty)
{
	VTY_BUF_T *pBuf = &pVty->line;

	if (pBuf->cp > 0) {
		pBuf->cp--;

		_vty_putchar(pVty, CHAR_BS, 1);
	}
}

static void _vty_cmd_delete_char(VTY_T *pVty)
{
	int size;
	VTY_BUF_T *pBuf = &pVty->line;

	if (pBuf->len == 0) {
		return;
	}

	if (pBuf->cp == pBuf->len)
		return;

	size = pBuf->len - pBuf->cp;

	pBuf->len--;

	memmove(&VTY_BUF_VAL_CP(pBuf), &VTY_BUF_VAL_NEXT_CP(pBuf), size - 1);

	pBuf->data[pBuf->len] = '\0';

	vty_write(pVty, &VTY_BUF_VAL_CP(pBuf), size - 1);

	_vty_putchar(pVty, CHAR_SPACE, 1);

	_vty_putchar(pVty, CHAR_BS, size);
}

static void _vty_cmd_backspace_char(VTY_T *pVty)
{
	VTY_BUF_T *pBuf = &pVty->line;

	if (pBuf->cp == 0)
		return;

	_vty_cmd_backward_char(pVty);

	_vty_cmd_delete_char(pVty);
}

static void _vty_cmd_line_begin(VTY_T *pVty)
{
	VTY_BUF_T *pBuf = &pVty->line;

	_vty_putchar(pVty, CHAR_BS, pBuf->cp);

	pVty->line.cp = 0;
}

static void _vty_cmd_line_end(VTY_T *pVty)
{
	VTY_BUF_T *pBuf = &pVty->line;

	vty_write(pVty, &VTY_BUF_VAL_CP(pBuf), (pBuf->len - pBuf->cp));

	pBuf->cp = pBuf->len;
}

static void _vty_cmd_kill_line(VTY_T *pVty)
{
	VTY_BUF_T *pBuf = &pVty->line;

	_vty_cmd_line_begin(pVty);

	_vty_putchar(pVty, CHAR_SPACE, pBuf->len);
	_vty_putchar(pVty, CHAR_BS, pBuf->len);

	VTY_BUF_ZERO(pBuf);
}

static void _vty_print_history(VTY_T *pVty)
{
	VTY_HIST_T *pHist = &pVty->hist;
	VTY_BUF_T *pBuf = &pVty->line;
	int length;

	_vty_cmd_kill_line(pVty);

	length = osal_strlen(pHist->list[pHist->cp]);

	osal_memcpy(pBuf->data, pHist->list[pHist->cp], length);

	pBuf->cp = pBuf->len = length;

	_vty_redraw_line(pVty);
}

static void _vty_cmd_next_history(VTY_T *pVty)
{
	VTY_HIST_T *pHist = &pVty->hist;
	int next;

	if (pHist->cp == pHist->insert) {
		return;
	}

	next = (pHist->cp + 1) % VTY_HIST_MAX_SIZE;

	if (pHist->list[next]) {
		pHist->cp = next;

		_vty_print_history(pVty);
	}
}

static void _vty_cmd_prev_history(VTY_T *pVty)
{
	VTY_HIST_T *pHist = &pVty->hist;
	int prev;

	prev = (pHist->cp == 0) ? (VTY_HIST_MAX_SIZE - 1) : (pHist->cp - 1);

	if (prev == pHist->insert) {
		return;
	}

	if (pHist->list[prev]) {
		pHist->cp = prev;

		_vty_print_history(pVty);
	}
}

sa_ch_t *__vty_trim_left(sa_ch_t *s)
{
	sa_ch_t *start = NULL;
	sa_ch_t *end = NULL;
	sa_ch_t *p = NULL;

	start = s;
	end = s + strlen(s);

	while (*start == ' ') {
		start++;
	}

	p = s;
	while (start < end) {
		*p++ = *start++;
	}

	*p = '\0';
	return s;
}

void _vty_cmd_add_history(VTY_T *pVty)
{
	VTY_HIST_T *pHist = &pVty->hist;
	VTY_BUF_T *pBuf = &pVty->line;
	sa_ch_t *pBufTrim = pBuf->data;
	int prev;

	pBufTrim = __vty_trim_left(pBuf->data);
	if (osal_strlen(pBufTrim) == 0) {
		return;
	}

	prev =
	    (pHist->insert ==
	     0) ? (VTY_HIST_MAX_SIZE - 1) : (pHist->insert - 1);
	if (pHist->list[prev] && osal_strcmp(pBufTrim, pHist->list[prev]) == 0) {
		pHist->cp = pHist->insert;
		return;
	}

	if (pHist->list[pHist->insert]) {
		osal_free(pHist->list[pHist->insert]);
	}

	pHist->list[pHist->insert] = osal_malloc((osal_strlen(pBufTrim) + 1));
	if (pHist->list[pHist->insert] == NULL) {
		return;
	}
	strcpy(pHist->list[pHist->insert], pBufTrim);

	pHist->insert = (pHist->insert + 1) % VTY_HIST_MAX_SIZE;
	pHist->cp = pHist->insert;
}

static void _vty_cmd_exit(VTY_T *pVty)
{
	VTY_BUF_ZERO(&pVty->line);

	vty_output(pVty, "%s", VTY_LINEBREAK);

	_vty_prompt(pVty);

	pVty->hist.cp = pVty->hist.insert;
}

static void _vty_cmd_output_error(VTY_T *pVty)
{
	/* point out first error position */
	if (pVty->errPosition != -1) {
		int errPosition;
		char errPositionStr[512] = { 0 };

		errPosition = strlen(pVty->prompt) + 1 + pVty->errPosition;
		osal_memset(errPositionStr, ' ', errPosition);
		errPositionStr[errPosition] = '^';
		errPositionStr[errPosition + 1] = '\0';

		vty_print("%s\r\n", errPositionStr);
	}

	/* output error string */
	if (strlen(pVty->errStr) != 0) {
		vty_print("%% %s\r\n", pVty->errStr);
	}

	return;
}

static void _vty_cmd_execute(VTY_T *pVty)
{
	vty_output(pVty, VTY_LINEBREAK);

	/* reset error position and error string */
	pVty->errPosition = -1;
	pVty->errStr[0] = '\0';

	cli_execute_cmd(pVty);

	_vty_cmd_output_error(pVty);

	_vty_cmd_add_history(pVty);

	VTY_BUF_ZERO(&(pVty->line));	/* Clear command line buffer. */

	_vty_prompt(pVty);
}

static void _vty_cmd_describe(VTY_T *pVty)
{
	vty_output(pVty, VTY_LINEBREAK);

	/* reset error position and error string */
	pVty->errPosition = -1;
	pVty->errStr[0] = '\0';

	cli_describe_cmd(pVty);

	_vty_cmd_output_error(pVty);

	_vty_prompt(pVty);

	_vty_redraw_line(pVty);
}

static void _vty_cmd_complete(VTY_T *pVty)
{
	vty_output(pVty, VTY_LINEBREAK);

	cli_complete_cmd(pVty);

	_vty_prompt(pVty);

	_vty_redraw_line(pVty);
}

/* key bind for escape characters */
static VTY_KET_BIND_T gKeyBindEsc[] = {
	{'A', _vty_cmd_prev_history},
	{'B', _vty_cmd_next_history},
	{'C', _vty_cmd_forward_char},
	{'D', _vty_cmd_backward_char}
};
static int gKeyBindEscLen = sizeof(gKeyBindEsc) / sizeof(gKeyBindEsc[0]);

/* key bind for normal characters */
static VTY_KET_BIND_T gKeyBindNorm[] = {
	{CHAR_CTRL('A'), _vty_cmd_line_begin},
	{CHAR_CTRL('B'), _vty_cmd_backward_char},
	{CHAR_CTRL('D'), _vty_cmd_delete_char},
	{CHAR_CTRL('E'), _vty_cmd_line_end},
	{CHAR_CTRL('F'), _vty_cmd_forward_char},
	{CHAR_CTRL('N'), _vty_cmd_next_history},
	{CHAR_CTRL('P'), _vty_cmd_prev_history},
	{CHAR_CTRL('U'), _vty_cmd_kill_line},
	{CHAR_CTRL('H'), _vty_cmd_backspace_char},
	{0x7f, _vty_cmd_backspace_char},
	{CHAR_CTRL('C'), _vty_cmd_exit},
	{CHAR_CR, _vty_cmd_execute},
	{CHAR_LF, _vty_cmd_execute},
	{CHAR_TAB, _vty_cmd_complete},
	{CHAR_HELP, _vty_cmd_describe}
};
static int gKeyBindNormLen = sizeof(gKeyBindNorm) / sizeof(gKeyBindNorm[0]);

static sa_bool_t _vty_traverse_keybind(VTY_T *pVty, VTY_KET_BIND_T *keyBind,
				       int keyBindLen, sa_u8_t ch)
{
	sa_bool_t flag = 0;
	int i;

	for (i = 0; i < keyBindLen; i++) {
		if (ch == keyBind[i].key) {
			keyBind[i].func(pVty);
			flag = 1;
			break;
		}
	}

	return flag;
}

static void _vty_input_esc(VTY_T *pVty, sa_ch_t ch)
{
	_vty_traverse_keybind(pVty, gKeyBindEsc, gKeyBindEscLen, ch);

	pVty->esc = VTY_ESC_NONE;
}

void _vty_input_normal(VTY_T *pVty, sa_u8_t ch)
{
	if (!_vty_traverse_keybind(pVty, gKeyBindNorm, gKeyBindNormLen, ch)) {
		if (CHAR_PRINTABLE(ch)) {
			_vty_cmd_insert_char(pVty, ch);
		}
	}
}

/******************************************************************************
 *
 * description:
 *   send input output to vty.
 *   also see vty_write.
 *
 * input:
 *   VTY_T * - vty
 *   sa_ch_t * - input buffer
 *   int - input bytes
 *
 * output:
 *
 * return:
 *   int - 0 success else failure
 *
 ******************************************************************************/
int vty_input(VTY_T *pVty, sa_u8_t *buf, int nbytes)
{
	int i = 0;

	if (pVty == NULL) {
		return 0;
	}

	if (nbytes <= 0) {
		pVty->state = VTY_STATE_CLOSE;
	}

	for (i = 0; i < nbytes; i++) {
		if (pVty->esc == VTY_ESC_CH2) {
			_vty_input_esc(pVty, buf[i]);
			continue;
		}

		if (pVty->esc == VTY_ESC_CH1) {
			if (buf[i] == '[') {
				pVty->esc = VTY_ESC_CH2;
			} else {
				pVty->esc = VTY_ESC_NONE;
			}
			continue;
		}

		if (buf[i] == CHAR_ESC) {
			pVty->esc = VTY_ESC_CH1;
			continue;
		}

		_vty_input_normal(pVty, buf[i]);
	}

	return 0;
}

/******************************************************************************
 *
 * description:
 *   free vty allocated memory, and close vty.
 *   also see vty_output.
 *
 * input:
 *   VTY_T * - vty
 *
 * output:
 *
 * return:
 *
 ******************************************************************************/
void vty_close(VTY_T *pVty)
{
	int i;

	if (pVty == NULL) {
		return;
	}

	for (i = 0; i < VTY_HIST_MAX_SIZE; i++) {
		if (pVty->hist.list[i]) {
			osal_free(pVty->hist.list[i]);
		}
	}

	if (pVty->line.data) {
		osal_free(pVty->line.data);
	}

	osal_free(pVty);

	return;
}

/******************************************************************************
 *
 * description:
 *   complete vty's line by supplied subCmdStr, it's provided for cli engine.
 *
 * input:
 *   VTY_T * - vty
 *   const char * - subCmdStr
 *   sa_bool_t - if append backspace to end
 *
 * output:
 *
 * return:
 *   int - 0 success else failure
 *
 ******************************************************************************/
int vty_complete_cmd(VTY_T *pVty, const char *subCmdStr, sa_bool_t addSpace)
{
	VTY_BUF_T *pBuf = &(pVty->line);
	int subCmdStrLen = strlen(subCmdStr);
	int i;

	/* backspace to where the last word start */
	if (pBuf->len != 0) {
		for (i = pBuf->len - 1; i >= 0; i--) {
			if (CHAR_SPACE == pBuf->data[i]) {
				pBuf->len = i + 1;
				goto complete_handle;
			}
		}

		pBuf->len = 0;
	}

      complete_handle:
	osal_memcpy(pBuf->data + pBuf->len, (void *)subCmdStr, subCmdStrLen);
	pBuf->len += subCmdStrLen;

	if (addSpace) {
		pBuf->data[pBuf->len] = CHAR_SPACE;
		pBuf->len++;
	}

	pBuf->cp = pBuf->len;

	return 0;
}

VTY_T *vty_create(int fd)
{
	VTY_T *pVty = osal_malloc(sizeof(VTY_T));
	if (pVty == NULL) {
		return NULL;
	}

	osal_memset(pVty, 0, sizeof(VTY_T));

	pVty->fd = fd;
	pVty->type = VTY_TYPE_CONSOLE;
	pVty->state = VTY_STATE_NORMAL;
	pVty->esc = VTY_ESC_NONE;
	pVty->writeMode = VTY_MODE_FLUSH;
	pVty->cliMode = CLI_MODE_ENABLE;
	osal_strcpy(pVty->prompt, PROMPT_BASE ">");

	pVty->line.data = osal_malloc(VTY_CMDBUF_MAX_SIZE);
	if (pVty->line.data == NULL)
		return NULL;

	pVty->line.max = VTY_CMDBUF_MAX_SIZE;
	VTY_BUF_ZERO(&pVty->line);

	pVty->lines = VTY_DEFUALT_LINES;
	pVty->width = VTY_DEFUALT_WIDTH;

	_vty_welcome(pVty);
	_vty_prompt(pVty);

	return pVty;
}
