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

#include <sys_base.h>
#include <termios.h>
#include <signal.h>
#include <cli.h>
#include <shell_entity.h>

typedef struct {
	int sig;
	char *desc;
} sig_desc_t;

static struct termios old_termios;

static sig_desc_t sig_desc_table[] = {
	{SIGABRT, "[SIGABRT] abort(), exception."},
	{SIGBUS, "[SIGBUS] memory access caused hardware error"},
	//{SIGEMT,    "[SIGEMT] hardware exception."},
	{SIGFPE, "[SIGFPE] math calculate exception."},
	{SIGHUP, "[SIGHUP] ctrolling process's terminal disconnet."},
	{SIGILL, "[SIGILL] illegal instructions exception."},
	{SIGINT, "[SIGINT] User pressed Interrupt Key(Ctrl-C), exit."},
	{SIGTSTP, "[SIGTSTP] User pressed Suspend Key(Ctrl-Z), exit."},
	{SIGIOT, "[SIGIOT] some hardware error, similar to SIGABRT."},
	{SIGKILL, "[SIGKILL] can't be handled, exit."},
	{SIGSEGV, "[SIGSEGV] segment fault."},
	{SIGSTOP, "[SIGSTOP] terminate process."},
	{SIGSYS, "[SIGSYS] illegal system call."},
	{SIGTERM, "[SIGTERM] exit and send kill command."},
	{SIGTRAP, "[SIGTRAP] hardware debug exception."},
	{SIGUSR1, "[SIGUSR1] shell defined signal exit."},
};

static int _shell_serial_init()
{
	int fildes = fileno(stdin);
	struct termios new_termios;

	tcgetattr(fildes, &old_termios);
	tcgetattr(fildes, &new_termios);

    /** ctrl(c) ctrl(\) ctrl(?) ctrl(u) ctrl(d) ctrl(q) ctrl(s) ctrl(z) */
	new_termios.c_cc[VINTR] = 3;
	new_termios.c_cc[VEOF] = 4;
	new_termios.c_cc[VSTART] = 17;
	new_termios.c_cc[VSTOP] = 19;
	new_termios.c_cc[VKILL] = 21;
	new_termios.c_cc[VSUSP] = 26;
	new_termios.c_cc[VQUIT] = 28;
	new_termios.c_cc[VERASE] = 127;

	new_termios.c_cflag &= CBAUD;
	new_termios.c_cflag |= CSIZE | CREAD | HUPCL | CLOCAL;
	new_termios.c_iflag = ICRNL | IXON | IXOFF;
	new_termios.c_oflag = ONLCR | OPOST;
	new_termios.c_lflag = ECHOK | ECHOKE | ECHOCTL | TOSTOP;

	tcsetattr(fildes, TCSANOW, &new_termios);

	return 0;
}

static int _shell_serial_fini()
{
	int fildes = fileno(stdin);

	tcsetattr(fildes, TCSANOW, &old_termios);

	return 0;
}

static void _shell_sig_handler(int sig)
{
	int i;
	int sigCount = (sizeof(sig_desc_table) / sizeof(sig_desc_t));

	for (i = 0; i < sigCount; i++) {
		if (sig_desc_table[i].sig == sig) {
			osal_print("%s\n", sig_desc_table[i].desc);

			_shell_serial_fini();

			exit(0);
		}
	}

	return;
}

static void _shell_sig_init(void)
{
	int i;
	int sigCount = (sizeof(sig_desc_table) / sizeof(sig_desc_t));

	for (i = 0; i < sigCount; i++) {
		signal(sig_desc_table[i].sig, _shell_sig_handler);
	}

	return;
}

static void _shell_sig_fini(void)
{
	int i;
	int sigCount = (sizeof(sig_desc_table) / sizeof(sig_desc_t));

	for (i = 0; i < sigCount; i++) {
		signal(sig_desc_table[i].sig, SIG_DFL);
	}

	return;
}

int main(int argc, sa_ch_t *argv[])
{
	_shell_sig_init();

	_shell_serial_init();

	shell_loop();

	_shell_serial_fini();

	_shell_sig_fini();

	return 0;
}
