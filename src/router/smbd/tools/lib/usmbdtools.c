// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#include <syslog.h>
#include <iconv.h>

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#include <stdio.h>
#include <stdarg.h>
#include <usmbdtools.h>

/*
 * special simple linked list implementation, just made for the need of usmbd.
 * not yet optimized
 */

struct LIST *list_init(struct LIST **list)
{
	*list = malloc(sizeof(struct LIST));
	if (!*list)
		return NULL;
	(*list)->prev = NULL;
	(*list)->next = NULL;
	return *list;
}

long long list_maxid(struct LIST **list)
{
	long long id = -1;
	struct LIST *head = *list;

	while ((head = head->next)) {
		if (((long long)head->id) >= id)
			id = head->id;
	}
	return id;
}

int list_foreach(struct LIST **list,
		 void (*func)(void *item, unsigned long long id,
			      void *user_data), void *user_data)
{
	struct LIST *head = *list;

	while ((head = head->next)) {
		if (head->type == KEY_STRING)
			func(head->item, list_tokey(head->keystr), user_data);
		else
			func(head->item, head->id, user_data);
	}
}

struct LIST *head_get(struct LIST **list, unsigned long long id)
{
	struct LIST *head = *list;
	struct LIST *last = NULL;

	head = *list;
	while ((head = head->next)) {
		if (head == last) {
			/* should not happen. if this triggers we have a bug */
			pr_debug("fixup list\n");
			head->next = NULL;
			break;
		}
		last = head;
		if (head->type == KEY_STRING) {
			char *c = (char *)list_fromkey(id);

			if (!strcmp(head->keystr, c))
				return head;
		} else {
			if (head->id == id)
				return head;
		}
	}
	return NULL;
}

int _list_add(struct LIST **list, void *item, unsigned long long id, char *str)
{
	int ret = 1;
	struct LIST *new;

	if (!*list)
		list_init(list);
	new = head_get(list, str ? list_tokey(str) : id);
	if (new)
		ret = 0;
	if (!new)
		new = malloc(sizeof(struct LIST));
	if (!new)
		return 0;

	new->item = item;
	if (ret) {
		if (str) {
			new->keystr = str;
			new->type = KEY_STRING;
		} else {
			new->id = id;
			new->type = KEY_ID;
		}
		new->next = NULL;
		struct LIST *head = *list;
		struct LIST *last = head;

		while ((head = head->next))
			last = head;

		last->next = new;
		new->prev = last;
	}
	return ret;
}

int list_add(struct LIST **list, void *item, unsigned long long id)
{
	return _list_add(list, item, id, NULL);
}

int list_add_str(struct LIST **list, void *item, char *str)
{
	return _list_add(list, item, 0, str);
}

void list_append(struct LIST **list, void *item)
{
	_list_add(list, item, list_maxid(list) + 1, NULL);
}

int _list_remove(struct LIST **list, unsigned long long id, int dec)
{
	int ret = 0;
	struct LIST *head = *list;
	struct LIST *next = NULL;

	while ((head = head->next)) {
		if ((head->type == KEY_ID && head->id == id)
		    || (head->type == KEY_STRING
			&& !strcmp(head->keystr, list_fromkey(id)))) {
			if (head->prev)
				head->prev->next = head->next;

			if (head->next)
				next = head->next;
			head->next->prev = head->prev;

			free(head);
			ret = 1;
			goto out;
		}
	}
out:
	if (dec && !ret && next) {
		/* reorder all following ids after removing slot */
		while (next) {
			next->id = next->prev->id + 1;
			next = next->next;
		}
	}
	return ret;
}

int list_remove_dec(struct LIST **list, unsigned long long id)
{
	return _list_remove(list, id, 1);
}

int list_remove(struct LIST **list, unsigned long long id)
{
	return _list_remove(list, id, 0);
}

void *list_get(struct LIST **list, unsigned long long id)
{
	struct LIST *head = head_get(list, id);

	if (head)
		return head->item;
	return NULL;
}

void list_clear(struct LIST **list)
{
	struct LIST *head = *list;

	if (head)
		return;

	while (head) {
		struct LIST *h = head->next;

		free(head);
		head = h;
	}
	*list = NULL;
}

static const char *app_name = "unknown";
static int log_open;

typedef void (*logger)(int level, const char *fmt, va_list list);

char *usmbd_conv_charsets[USMBD_CHARSET_MAX + 1] = {
	"UTF-8",
	"UTF-16LE",
	"UCS-2LE",
	"UTF-16BE",
	"UCS-2BE",
	"OOPS"
};

static int syslog_level(int level)
{
	if (level == PR_ERROR)
		return LOG_ERR;
	if (level == PR_INFO)
		return LOG_INFO;
	if (level == PR_DEBUG)
		return LOG_DEBUG;

	return LOG_ERR;
}

static void __pr_log_stdio(int level, const char *fmt, va_list list)
{
	char buf[1024];

	vsnprintf(buf, sizeof(buf), fmt, list);
	printf("%s", buf);
}

static void __pr_log_syslog(int level, const char *fmt, va_list list)
{
	vsyslog(syslog_level(level), fmt, list);
}

static logger __logger = __pr_log_stdio;

void set_logger_app_name(const char *an)
{
	app_name = an;
}

const char *get_logger_app_name(void)
{
	return app_name;
}

char *strerr(int err)
{
	static char __thread buf[64];

	strerror_r(err, buf, sizeof(buf));
	buf[sizeof(buf) - 1] = 0x00;
	return buf;
}

void __pr_log(int level, const char *fmt, ...)
{
	va_list list;

	va_start(list, fmt);
	__logger(level, fmt, list);
	va_end(list);
}

void pr_logger_init(int flag)
{
	if (flag == PR_LOGGER_SYSLOG) {
		if (log_open) {
			closelog();
			log_open = 0;
		}
		openlog("usmbd", LOG_NDELAY, LOG_LOCAL5);
		__logger = __pr_log_syslog;
		log_open = 1;
	}
}

#if TRACING_DUMP_NL_MSG
#define PR_HEX_DUMP_WIDTH	160
void pr_hex_dump(const void *mem, size_t sz)
{
	char xline[PR_HEX_DUMP_WIDTH];
	char sline[PR_HEX_DUMP_WIDTH];
	int xi = 0, si = 0, mi = 0;

	while (mi < sz) {
		char c = *((char *)mem + mi);

		mi++;
		xi += sprintf(xline + xi, "%02X ", 0xff & c);
		if (c > ' ' && c < '~')
			si += sprintf(sline + si, "%c", c);
		else
			si += sprintf(sline + si, ".");
		if (xi >= PR_HEX_DUMP_WIDTH / 2) {
			pr_err("%s         %s\n", xline, sline);
			xi = 0;
			si = 0;
		}
	}

	if (xi) {
		int sz = PR_HEX_DUMP_WIDTH / 2 - xi + 1;

		if (sz > 0) {
			memset(xline + xi, ' ', sz);
			xline[PR_HEX_DUMP_WIDTH / 2 + 1] = 0x00;
		}
		pr_err("%s         %s\n", xline, sline);
	}
}
#else
void pr_hex_dump(const void *mem, size_t sz)
{
}
#endif

void base64_init_encodestate(base64_encodestate *state_in)
{
	state_in->step = step_A;
	state_in->result = 0;
	state_in->stepcount = 0;
}

char base64_encode_value(char value_in)
{
	static const char *encoding =
	    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	if (value_in > 63)
		return '=';
	return encoding[(int)value_in];
}

int base64_encode_block(const char *plaintext_in, int length_in, char *code_out,
			base64_encodestate *state_in)
{
	const char *plainchar = plaintext_in;
	const char *const plaintextend = plaintext_in + length_in;
	char *codechar = code_out;
	char result;
	char fragment;

	result = state_in->result;

	switch (state_in->step) {
		while (1) {
		case step_A:
			if (plainchar == plaintextend) {
				state_in->result = result;
				state_in->step = step_A;
				return codechar - code_out;
			}
			fragment = *plainchar++;
			result = (fragment & 0x0fc) >> 2;
			*codechar++ = base64_encode_value(result);
			result = (fragment & 0x003) << 4;
		case step_B:
			if (plainchar == plaintextend) {
				state_in->result = result;
				state_in->step = step_B;
				return codechar - code_out;
			}
			fragment = *plainchar++;
			result |= (fragment & 0x0f0) >> 4;
			*codechar++ = base64_encode_value(result);
			result = (fragment & 0x00f) << 2;
		case step_C:
			if (plainchar == plaintextend) {
				state_in->result = result;
				state_in->step = step_C;
				return codechar - code_out;
			}
			fragment = *plainchar++;
			result |= (fragment & 0x0c0) >> 6;
			*codechar++ = base64_encode_value(result);
			result = (fragment & 0x03f) >> 0;
			*codechar++ = base64_encode_value(result);

			++(state_in->stepcount);
		}
	}
	/* control should not reach here */
	return codechar - code_out;
}

int base64_encode_blockend(char *code_out, base64_encodestate *state_in)
{
	char *codechar = code_out;

	switch (state_in->step) {
	case step_B:
		*codechar++ = base64_encode_value(state_in->result);
		*codechar++ = '=';
		*codechar++ = '=';
		break;
	case step_C:
		*codechar++ = base64_encode_value(state_in->result);
		*codechar++ = '=';
		break;
	case step_A:
		break;
	}

	return codechar - code_out;
}

char *base64_encode(unsigned char *src, size_t srclen)
{
	char *out = malloc((srclen / 3 + 1) * 4 + 1);
	base64_encodestate state;

	base64_init_encodestate(&state);
	int len = base64_encode_block(src, srclen, out, &state);

	base64_encode_blockend(out + len, &state);
	return out;
}

int base64_decode_value(char value_in)
{
	static const char decoding[] = {
		62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1,
		-1, -1, -2, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		    12, 13, 14, 15, 16, 17,
		18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27,
		    28, 29, 30, 31, 32, 33,
		34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
		    50, 51
	};
	static const char decoding_size = sizeof(decoding);

	value_in -= 43;
	if (value_in < 0 || value_in > decoding_size)
		return -1;
	return decoding[(int)value_in];
}

void base64_init_decodestate(base64_decodestate *state_in)
{
	state_in->step = step_a;
	state_in->plainchar = 0;
}

int base64_decode_block(const char *code_in, int length_in, char *plaintext_out,
			const int outlen, base64_decodestate *state_in)
{
	const char *codechar = code_in;
	char *plainchar = plaintext_out;
	char fragment;
	int count = 0;

	*plainchar = state_in->plainchar;

	switch (state_in->step) {
		while (1) {
		case step_a:
			do {
				if (codechar == code_in + length_in) {
					state_in->step = step_a;
					state_in->plainchar = *plainchar;
					return plainchar - plaintext_out;
				}
				fragment =
				    (char)base64_decode_value(*codechar++);
			} while (fragment < 0);
			if (count > outlen)
				return plainchar - plaintext_out;
			*plainchar = (fragment & 0x03f) << 2;
		case step_b:
			do {
				if (codechar == code_in + length_in) {
					state_in->step = step_b;
					state_in->plainchar = *plainchar;
					return plainchar - plaintext_out;
				}
				fragment =
				    (char)base64_decode_value(*codechar++);
			} while (fragment < 0);
			count += 1;
			if (count > outlen)
				return plainchar - plaintext_out;
			*plainchar++ |= (fragment & 0x030) >> 4;
			*plainchar = (fragment & 0x00f) << 4;
		case step_c:
			do {
				if (codechar == code_in + length_in) {
					state_in->step = step_c;
					state_in->plainchar = *plainchar;
					return plainchar - plaintext_out;
				}
				fragment =
				    (char)base64_decode_value(*codechar++);
			} while (fragment < 0);
			count += 1;
			if (count > outlen)
				return plainchar - plaintext_out;
			*plainchar++ |= (fragment & 0x03c) >> 2;
			*plainchar = (fragment & 0x003) << 6;
		case step_d:
			do {
				if (codechar == code_in + length_in) {
					state_in->step = step_d;
					state_in->plainchar = *plainchar;
					return plainchar - plaintext_out;
				}
				fragment =
				    (char)base64_decode_value(*codechar++);
			} while (fragment < 0);
			count++;
			if (count > outlen)
				return plainchar - plaintext_out;
			*plainchar++ |= (fragment & 0x03f);
		}
	}
	/* control should not reach here */
	return plainchar - plaintext_out;
}

unsigned char *base64_decode(char const *src, size_t *dstlen)
{
	base64_decodestate state;

	base64_init_decodestate(&state);
	int len = ((strlen(src) / 4) * 3) + 1;
	char *out = malloc(len);
	*dstlen = base64_decode_block(src, strlen(src), out, len, &state);
	return out;
}

static int codeset_has_altname(int codeset)
{
	if (codeset == USMBD_CHARSET_UTF16LE ||
	    codeset == USMBD_CHARSET_UTF16BE)
		return 1;
	return 0;
}

char *usmbd_gconvert(const char *str, size_t str_len, int to_codeset,
		     int from_codeset, size_t *bytes_read,
		     size_t *bytes_written)
{
	char *converted, *buf;
	int err;

retry:
	err = 0;
	if (from_codeset >= USMBD_CHARSET_MAX) {
		pr_err("Unknown source codeset: %d\n", from_codeset);
		return NULL;
	}

	if (to_codeset >= USMBD_CHARSET_MAX) {
		pr_err("Unknown target codeset: %d\n", to_codeset);
		return NULL;
	}
	buf = converted = malloc((str_len * 2) + 1);
	memset(converted, 0, (str_len * 2) + 1);
	iconv_t conv = iconv_open(usmbd_conv_charsets[to_codeset],
				  usmbd_conv_charsets[from_codeset]);
	*bytes_read = str_len;
	*bytes_written = str_len * 2;
	err = iconv(conv, (char **)&str, bytes_read, &converted, bytes_written);
	*bytes_read = 0;
	err = iconv(conv, NULL, bytes_read, &converted, bytes_written);
	iconv_close(conv);
	*bytes_written = (str_len * 2) - *bytes_written;
	*bytes_read = str_len - *bytes_read;
	if (err < 0) {
		int has_altname = 0;

		if (codeset_has_altname(to_codeset)) {
			to_codeset++;
			has_altname = 1;
		}

		if (codeset_has_altname(from_codeset)) {
			from_codeset++;
			has_altname = 1;
		}

		if (has_altname) {
			pr_info("Will try '%s' and '%s'\n",
				usmbd_conv_charsets[to_codeset],
				usmbd_conv_charsets[from_codeset]);
			goto retry;
		}

		return NULL;
	}

	return buf;
}

static pthread_mutex_t atomic_lock = PTHREAD_MUTEX_INITIALIZER;

int atomic_int_add(volatile int *atomic, int val)
{
	int oldval;

	pthread_mutex_lock(&atomic_lock);
	oldval = *atomic;
	*atomic = oldval + val;
	pthread_mutex_unlock(&atomic_lock);
	return oldval;
}

void atomic_int_inc(volatile int *atomic)
{
	pthread_mutex_lock(&atomic_lock);
	(*atomic)++;
	pthread_mutex_unlock(&atomic_lock);
}

int atomic_int_compare_and_exchange(volatile int *atomic, int oldval,
				    int newval)
{
	int success;

	pthread_mutex_lock(&atomic_lock);
	success = (*atomic == oldval);
	if (success)
		*atomic = newval;

	pthread_mutex_unlock(&atomic_lock);

	return success;
}

char *ascii_strdown(char *str, size_t len)
{
	char *result, *s;

	if (!str)
		return NULL;

	if (len < 0)
		len = strlen(str);

	result = strndup(str, len);
	for (s = result; *s; s++)
		*s = tolower(*s);
	return result;
}

void notify_usmbd_daemon(void)
{
	char manager_pid[10] = { 0, };
	int pid = 0;
	int lock_fd;

	lock_fd = open(USMBD_LOCK_FILE, O_RDONLY);
	if (lock_fd < 0)
		return;

	if (read(lock_fd, &manager_pid, sizeof(manager_pid)) == -1) {
		pr_debug("Unable to read main PID: %s\n", strerr(errno));
		close(lock_fd);
		return;
	}

	close(lock_fd);

	pid = strtol(manager_pid, NULL, 10);

	pr_debug("Send SIGHUP to pid %d\n", pid);
	if (kill(pid, SIGHUP))
		pr_debug("Unable to send signal to pid %d: %s\n",
			 pid, strerr(errno));
}

int test_file_access(char *conf)
{
	int fd = open(conf, O_RDWR | O_CREAT, S_IRWXU | S_IRGRP);

	if (fd != -1) {
		close(fd);
		return 0;
	}

	pr_err("%s %s\n", conf, strerr(errno));
	return -EINVAL;
}
