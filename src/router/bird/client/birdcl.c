/*
 *	BIRD Client - Light variant I/O
 *
 *	(c) 1999--2004 Martin Mares <mj@ucw.cz>
 *      (c) 2013 Tomas Hlavacek <tomas.hlavacek@nic.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#include <sys/ioctl.h>
#include <signal.h>

#include "nest/bird.h"
#include "lib/resource.h"
#include "lib/string.h"
#include "client/client.h"
#include "sysdep/unix/unix.h"

#define INPUT_BUF_LEN 2048

struct termios tty_save;

void
input_start_list(void)
{
  /* Empty in non-ncurses version. */
}

void
input_stop_list(void)
{
  /* Empty in non-ncurses version. */
}

void
input_notify(int prompt)
{
  /* No ncurses -> no status to reveal/hide, print prompt manually. */
  if (!prompt)
    return;

  printf("bird> ");
  fflush(stdout);
}


static int
lastnb(char *str, int i)
{
  while (i--)
    if ((str[i] != ' ') && (str[i] != '\t'))
      return str[i];

  return 0;
}

void
input_read(void)
{
  char buf[INPUT_BUF_LEN];

  if ((fgets(buf, INPUT_BUF_LEN, stdin) == NULL) || (buf[0] == 0))
  {
    putchar('\n');
    cleanup();
    exit(0);
  }

  int l = strlen(buf);
  if ((l+1) == INPUT_BUF_LEN)
    {
      printf("Input too long.\n");
      return;
    }

  if (buf[l-1] == '\n')
    buf[--l] = '\0';

  if (!interactive)
    printf("%s\n", buf);

  if (l == 0)
    return;

  if (lastnb(buf, l) == '?')
    {
      cmd_help(buf, strlen(buf));
      return;
    }

  submit_command(buf);
}

static struct termios stored_tty;
static int more_active = 0;

void
more_begin(void)
{
  static struct termios tty;

  tty = stored_tty;
  tty.c_lflag &= (~ECHO);
  tty.c_lflag &= (~ICANON);

  if (tcsetattr (0, TCSANOW, &tty) < 0)
    die("tcsetattr: %m");

  more_active = 1;
}

void
more_end(void)
{
  more_active = 0;

  if (tcsetattr (0, TCSANOW, &stored_tty) < 0)
    die("tcsetattr: %m");
}

static void
sig_handler(int signal)
{
  cleanup();
  exit(0);
}

void
input_init(void)
{
  if (!interactive)
    return;

  if (tcgetattr(0, &stored_tty) < 0)
    die("tcgetattr: %m");

  if (signal(SIGINT, sig_handler) == SIG_IGN)
    signal(SIGINT, SIG_IGN);
  if (signal(SIGTERM, sig_handler) == SIG_IGN)
    signal(SIGTERM, SIG_IGN);

  struct winsize tws;
  if (ioctl(0, TIOCGWINSZ, &tws) == 0)
    {
      term_lns = tws.ws_row;
      term_cls = tws.ws_col;
    }
}

void
cleanup(void)
{
  if (more_active)
    more_end();
}
