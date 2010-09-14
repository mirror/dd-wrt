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
#include <curses.h>

#include "nest/bird.h"
#include "lib/resource.h"
#include "lib/string.h"
#include "client/client.h"
#include "sysdep/unix/unix.h"

static char *opt_list = "s:vr";
static int verbose;
static char *init_cmd;
static int once;

static char *server_path = PATH_CONTROL_SOCKET;
static int server_fd;
static byte server_read_buf[4096];
static byte *server_read_pos = server_read_buf;

#define STATE_PROMPT		0
#define STATE_CMD_SERVER	1
#define STATE_CMD_USER		2

static int input_initialized;
static int input_hidden_end;
static int cstate = STATE_CMD_SERVER;
static int nstate = STATE_CMD_SERVER;

static int num_lines, skip_input, interactive;

/*** Parsing of arguments ***/

static void
usage(void)
{
  fprintf(stderr, "Usage: birdc [-s <control-socket>] [-v] [-r]\n");
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
      case 'r':
	init_cmd = "restrict";
	break;
      default:
	usage();
      }

  /* If some arguments are not options, we take it as commands */
  if (optind < argc)
    {
      char *tmp;
      int i;
      int len = 0;

      if (init_cmd)
	usage();

      for (i = optind; i < argc; i++)
	len += strlen(argv[i]) + 1;

      tmp = init_cmd = malloc(len);
      for (i = optind; i < argc; i++)
	{
	  strcpy(tmp, argv[i]);
	  tmp += strlen(tmp);
	  *tmp++ = ' ';
	}

      once = 1;
    }
}

/*** Input ***/

static void server_send(char *);

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

void
submit_server_command(char *cmd)
{
  server_send(cmd);
  nstate = STATE_CMD_SERVER;
  num_lines = 2;
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
	    submit_server_command(cmd);

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
  int i, in_string, in_bracket;

  if (arg != 1)
    return rl_insert(arg, '?');

  in_string = in_bracket = 0;
  for (i = 0; i < rl_point; i++)
    {
   
      if (rl_line_buffer[i] == '"')
	in_string = ! in_string;
      else if (! in_string)
        {
	  if (rl_line_buffer[i] == '[')
	    in_bracket++;
	  else if (rl_line_buffer[i] == ']')
	    in_bracket--;
        }
    }

  /* `?' inside string or path -> insert */
  if (in_string || in_bracket)
    return rl_insert(1, '?');

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
//  readline library does strange things when stdin is nonblocking.
//  if (fcntl(0, F_SETFL, O_NONBLOCK) < 0)
//    die("fcntl: %m");
}

static void
input_hide(void)
{
  input_hidden_end = rl_end;
  rl_end = 0;
  rl_expand_prompt("");
  rl_redisplay();
}

static void
input_reveal(void)
{
  /* need this, otherwise some lib seems to eat pending output when
     the prompt is displayed */
  fflush(stdout);
  tcdrain(fileno(stdout));

  rl_end = input_hidden_end;
  rl_expand_prompt("bird> ");
  rl_forced_update_display();
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

void
update_state(void)
{
  if (nstate == cstate)
    return;

  if (init_cmd)
    {
      /* First transition - client received hello from BIRD
	 and there is waiting initial command */
      submit_server_command(init_cmd);
      init_cmd = NULL;
      return;
    }

  if (!init_cmd && once)
    {
      /* Initial command is finished and we want to exit */
      cleanup();
      exit(0);
    }

  if (nstate == STATE_PROMPT)
    {
      if (input_initialized)
	input_reveal();
      else
	input_init();
    }

  if (nstate != STATE_PROMPT)
    input_hide();

  cstate = nstate;
}

void
more(void)
{
  printf("--More--\015");
  fflush(stdout);

 redo:
  switch (getchar())
    {
    case 32:
      num_lines = 2;
      break;
    case 13:
      num_lines--;
      break;
    case 'q':
      skip_input = 1;
      break;
    default:
      goto redo;
    }

  printf("        \015");
  fflush(stdout);
}


/*** Communication with server ***/

static void
server_connect(void)
{
  struct sockaddr_un sa;

  server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (server_fd < 0)
    die("Cannot create socket: %m");

  if (strlen(server_path) >= sizeof(sa.sun_path))
    die("server_connect: path too long");

  bzero(&sa, sizeof(sa));
  sa.sun_family = AF_UNIX;
  strcpy(sa.sun_path, server_path);
  if (connect(server_fd, (struct sockaddr *) &sa, SUN_LEN(&sa)) < 0)
    die("Unable to connect to server control socket (%s): %m", server_path);
  if (fcntl(server_fd, F_SETFL, O_NONBLOCK) < 0)
    die("fcntl: %m");
}

#define PRINTF(LEN, PARGS...) do { if (!skip_input) len = printf(PARGS); } while(0)

static void
server_got_reply(char *x)
{
  int code;
  int len = 0;

  if (*x == '+')			/* Async reply */
    PRINTF(len, ">>> %s\n", x+1);
  else if (x[0] == ' ')			/* Continuation */
    PRINTF(len, "%s%s\n", verbose ? "     " : "", x+1);
  else if (strlen(x) > 4 &&
	   sscanf(x, "%d", &code) == 1 && code >= 0 && code < 10000 &&
	   (x[4] == ' ' || x[4] == '-'))
    {
      if (code)
	PRINTF(len, "%s\n", verbose ? x : x+5);
      if (x[4] == ' ')
      {
	nstate = STATE_PROMPT;
	skip_input = 0;
	return;
      }
    }
  else
    PRINTF(len, "??? <%s>\n", x);

  if (skip_input)
    return;

  if (interactive && input_initialized && (len > 0))
    {
      int lns = LINES ? LINES : 25;
      int cls = COLS ? COLS : 80;
      num_lines += (len + cls - 1) / cls; /* Divide and round up */
      if ((num_lines >= lns)  && (cstate == STATE_CMD_SERVER))
	more();
    }
}

static void
server_read(void)
{
  int c;
  byte *start, *p;

 redo:
  c = read(server_fd, server_read_pos, server_read_buf + sizeof(server_read_buf) - server_read_pos);
  if (!c)
    die("Connection closed by server.");
  if (c < 0)
    {
      if (errno == EINTR)
	goto redo;
      else
	die("Server read error: %m");
    }

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
select_loop(void)
{
  int rv;
  while (1)
    {
      FD_ZERO(&select_fds);

      if (cstate != STATE_CMD_USER)
	FD_SET(server_fd, &select_fds);
      if (cstate != STATE_CMD_SERVER)
	FD_SET(0, &select_fds);

      rv = select(server_fd+1, &select_fds, NULL, NULL, NULL);
      if (rv < 0)
	{
	  if (errno == EINTR)
	    continue;
	  else
	    die("select: %m");
	}

      if (FD_ISSET(server_fd, &select_fds))
	{
	  server_read();
	  update_state();
	}

      if (FD_ISSET(0, &select_fds))
	{
	  rl_callback_read_char();
	  update_state();
	}
    }
}

static void
wait_for_write(int fd)
{
  while (1)
    {
      int rv;
      fd_set set;
      FD_ZERO(&set);
      FD_SET(fd, &set);

      rv = select(fd+1, NULL, &set, NULL, NULL);
      if (rv < 0)
	{
	  if (errno == EINTR)
	    continue;
	  else
	    die("select: %m");
	}

      if (FD_ISSET(server_fd, &set))
	return;
    }
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
	  if (errno == EAGAIN)
	    wait_for_write(server_fd);
	  else if (errno == EINTR)
	    continue;
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

  interactive = isatty(0);
  parse_args(argc, argv);
  cmd_build_tree();
  server_connect();
  select_loop();
  return 0;
}
