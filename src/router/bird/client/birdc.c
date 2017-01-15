/*
 *	BIRD Client - Readline variant I/O
 *
 *	(c) 1999--2004 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#include <readline/readline.h>
#include <readline/history.h>
#include <curses.h>

#include "nest/bird.h"
#include "lib/resource.h"
#include "lib/string.h"
#include "client/client.h"

static int input_hidden_end;
static int prompt_active;

/*** Input ***/

/* HACK: libreadline internals we need to access */
extern int _rl_vis_botlin;
extern void _rl_move_vert(int);

static void
add_history_dedup(char *cmd)
{
  /* Add history line if it differs from the last one */
  HIST_ENTRY *he = history_get(history_length);
  if (!he || strcmp(he->line, cmd))
    add_history(cmd);
}

static void
input_got_line(char *cmd_buffer)
{
  if (!cmd_buffer)
    {
      cleanup();
      exit(0);
    }

  if (cmd_buffer[0])
    {
      add_history_dedup(cmd_buffer);
      submit_command(cmd_buffer);
    }

  free(cmd_buffer);
}

void
input_start_list(void)
{
  /* Leave the currently edited line and make space for listing */
  _rl_move_vert(_rl_vis_botlin);
#ifdef HAVE_RL_CRLF
  rl_crlf();
#endif
}

void
input_stop_list(void)
{
  /* Reprint the currently edited line after listing */
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

void
input_init(void)
{
  rl_readline_name = "birdc";
  rl_add_defun("bird-complete", input_complete, '\t');
  rl_add_defun("bird-help", input_help, '?');
  rl_callback_handler_install("bird> ", input_got_line);

  // rl_get_screen_size();
  term_lns = LINES;
  term_cls = COLS;

  prompt_active = 1;

  // readline library does strange things when stdin is nonblocking.
  // if (fcntl(0, F_SETFL, O_NONBLOCK) < 0)
  //   DIE("fcntl");
}

static void
input_reveal(void)
{
  /* need this, otherwise some lib seems to eat pending output when
     the prompt is displayed */
  fflush(stdout);
  tcdrain(STDOUT_FILENO);

  rl_end = input_hidden_end;
  rl_expand_prompt("bird> ");
  rl_forced_update_display();

  prompt_active = 1;
}

static void
input_hide(void)
{
  input_hidden_end = rl_end;
  rl_end = 0;
  rl_expand_prompt("");
  rl_redisplay();

  prompt_active = 0;
}

void
input_notify(int prompt)
{
  if (prompt == prompt_active)
    return;

  if (prompt)
    input_reveal();
  else
    input_hide();
}

void
input_read(void)
{
  rl_callback_read_char();
}

void
more_begin(void)
{
}

void
more_end(void)
{
}

void
cleanup(void)
{
  if (init)
    return;

  input_hide();
  rl_callback_handler_remove();
}
