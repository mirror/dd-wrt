/*
 *	BIRD Client
 *
 *	(c) 1999--2004 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "nest/bird.h"
#include "lib/resource.h"
#include "lib/string.h"
#include "client/client.h"
#include "sysdep/unix/unix.h"

static char *opt_list = "s:v";
static int verbose;

static char *server_path = PATH_CONTROL_SOCKET;
static int server_fd;
static int server_reply;
static byte server_read_buf[4096];
static byte *server_read_pos = server_read_buf;

static int input_initialized;
static int input_hidden;
static int input_hidden_end;

/*** Parsing of arguments ***/

static void
usage(void)
{
  fprintf(stderr, "Usage: birdc [-s <control-socket>] [-v]\n");
  exit(1);
}

static void
parse_args(int argc, char **argv)
{
  int c;

  while ((c = getopt(argc, argv, opt_list)) >= 0)
    switch (c)
      {
      case 's':
	server_path = optarg;
	break;
      case 'v':
	verbose++;
	break;
      default:
	usage();
      }
  if (optind < argc)
    usage();
}

/*** Input ***/

static void server_send(char *);
static void select_loop(int);

/* HACK: libreadline internals we need to access */
extern int _rl_vis_botlin;
extern void _rl_move_vert(int);
extern Function *rl_last_func;

static int
handle_internal_command(char *cmd)
{
  if (!strncmp(cmd, "exit", 4) || !strncmp(cmd, "quit", 4))
    {
      cleanup();
      exit(0);
    }
  if (!strncmp(cmd, "help", 4))
    {
      puts("Press `?' for context sensitive help.");
      return 1;
    }
  return 0;
}

static void
got_line(char *cmd_buffer)
{
  char *cmd;

  if (!cmd_buffer)
    {
      cleanup();
      exit(0);
    }
  if (cmd_buffer[0])
    {
      cmd = cmd_expand(cmd_buffer);
      if (cmd)
	{
	  add_history(cmd);
	  if (!handle_internal_command(cmd))
	    {
	      server_send(cmd);
	      input_hidden = -1;
	      select_loop(0);
	      input_hidden = 0;
	    }
	  free(cmd);
	}
      else
	add_history(cmd_buffer);
    }
  free(cmd_buffer);
}

void
input_start_list(void)			/* Leave the currently edited line and make space for listing */
{
  _rl_move_vert(_rl_vis_botlin);
#ifdef HAVE_RL_CRLF
  rl_crlf();
#endif
}

void
input_stop_list(void)			/* Reprint the currently edited line after listing */
{
  rl_on_new_line();
  rl_redisplay();
}

static int
input_complete(int arg UNUSED, int key UNUSED)
{
  static int complete_flag;
  char buf[256];

  if (rl_last_func != input_complete)
    complete_flag = 0;
  switch (cmd_complete(rl_line_buffer, rl_point, buf, complete_flag))
    {
    case 0:
      complete_flag = 1;
      break;
    case 1:
      rl_insert_text(buf);
      break;
    default:
      complete_flag = 1;
#ifdef HAVE_RL_DING
      rl_ding();
#endif
    }
  return 0;
}

static int
input_help(int arg, int key UNUSED)
{
  int i = 0;

  if (arg != 1)
    return rl_insert(arg, '?');
  while (i < rl_point)
    {
      if (rl_line_buffer[i++] == '"')
	do
	  {
	    if (i >= rl_point)		/* `?' inside quoted string -> insert */
	      return rl_insert(1, '?');
	  }
        while (rl_line_buffer[i++] != '"');
    }
  rl_begin_undo_group();		/* HACK: We want to display `?' at point position */
  rl_insert_text("?");
  rl_redisplay();
  rl_end_undo_group();
  input_start_list();
  cmd_help(rl_line_buffer, rl_point);
  rl_undo_command(1, 0);
  input_stop_list();
  return 0;
}

static void
input_init(void)
{
  rl_readline_name = "birdc";
  rl_add_defun("bird-complete", input_complete, '\t');
  rl_add_defun("bird-help", input_help, '?');
  rl_callback_handler_install("bird> ", got_line);
  input_initialized = 1;
  if (fcntl(0, F_SETFL, O_NONBLOCK) < 0)
    die("fcntl: %m");
}

static void
input_hide(void)
{
  if (input_hidden)
    return;
  if (rl_line_buffer)
    {
      input_hidden_end = rl_end;
      rl_end = 0;
      rl_expand_prompt("");
      rl_redisplay();
      input_hidden = 1;
    }
}

static void
input_reveal(void)
{
  if (input_hidden <= 0)
    return;
  rl_end = input_hidden_end;
  rl_expand_prompt("bird> ");
  rl_forced_update_display();
  input_hidden = 0;
}

void
cleanup(void)
{
  if (input_initialized)
    {
      input_initialized = 0;
      input_hide();
      rl_callback_handler_remove();
    }
}

/*** Communication with server ***/

static void
server_connect(void)
{
  struct sockaddr_un sa;

  server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (server_fd < 0)
    die("Cannot create socket: %m");
  bzero(&sa, sizeof(sa));
  sa.sun_family = AF_UNIX;
  strcpy(sa.sun_path, server_path);
  if (connect(server_fd, (struct sockaddr *) &sa, SUN_LEN(&sa)) < 0)
    die("Unable to connect to server control socket (%s): %m", server_path);
  if (fcntl(server_fd, F_SETFL, O_NONBLOCK) < 0)
    die("fcntl: %m");
}

static void
server_got_reply(char *x)
{
  int code;

  input_hide();
  if (*x == '+')			/* Async reply */
    printf(">>> %s\n", x+1);
  else if (x[0] == ' ')			/* Continuation */
    printf("%s%s\n", verbose ? "     " : "", x+1);
  else if (strlen(x) > 4 &&
	   sscanf(x, "%d", &code) == 1 && code >= 0 && code < 10000 &&
	   (x[4] == ' ' || x[4] == '-'))
    {
      if (code)
	printf("%s\n", verbose ? x : x+5);
      if (x[4] == ' ')
	server_reply = code;
    }
  else
    printf("??? <%s>\n", x);
  /* need this, otherwise some lib seems to eat pending output when
     the prompt is displayed */
  fflush(stdout);
  tcdrain(fileno(stdout));
}

static void
server_read(void)
{
  int c;
  byte *start, *p;

  c = read(server_fd, server_read_pos, server_read_buf + sizeof(server_read_buf) - server_read_pos);
  if (!c)
    die("Connection closed by server.");
  if (c < 0)
    die("Server read error: %m");
  start = server_read_buf;
  p = server_read_pos;
  server_read_pos += c;
  while (p < server_read_pos)
    if (*p++ == '\n')
      {
	p[-1] = 0;
	server_got_reply(start);
	start = p;
      }
  if (start != server_read_buf)
    {
      int l = server_read_pos - start;
      memmove(server_read_buf, start, l);
      server_read_pos = server_read_buf + l;
    }
  else if (server_read_pos == server_read_buf + sizeof(server_read_buf))
    {
      strcpy(server_read_buf, "?<too-long>");
      server_read_pos = server_read_buf + 11;
    }
}

static fd_set select_fds;

static void
select_loop(int mode)
{
  server_reply = -1;
  while (mode || server_reply < 0)
    {
      FD_ZERO(&select_fds);
      FD_SET(server_fd, &select_fds);
      if (mode)
	FD_SET(0, &select_fds);
      select(server_fd+1, &select_fds, NULL, NULL, NULL);
      if (FD_ISSET(server_fd, &select_fds))
	{
	  server_read();
	  if (mode)
	    input_reveal();
	}
      if (FD_ISSET(0, &select_fds))
	rl_callback_read_char();
    }
  input_reveal();
}

static void
server_send(char *cmd)
{
  int l = strlen(cmd);
  byte *z = alloca(l + 1);

  memcpy(z, cmd, l);
  z[l++] = '\n';
  while (l)
    {
      int cnt = write(server_fd, z, l);
      if (cnt < 0)
	{
	  if (errno == -EAGAIN)
	    {
	      fd_set set;
	      FD_ZERO(&set);
	      do
		{
		  FD_SET(server_fd, &set);
		  select(server_fd+1, NULL, &set, NULL, NULL);
		}
	      while (!FD_ISSET(server_fd, &set));
	    }
	  else
	    die("Server write error: %m");
	}
      else
	{
	  l -= cnt;
	  z += cnt;
	}
    }
}

int
main(int argc, char **argv)
{
#ifdef HAVE_LIBDMALLOC
  if (!getenv("DMALLOC_OPTIONS"))
    dmalloc_debug(0x2f03d00);
#endif

  parse_args(argc, argv);
  cmd_build_tree();
  server_connect();
  select_loop(0);

  input_init();

  select_loop(1);
  return 0;
}
