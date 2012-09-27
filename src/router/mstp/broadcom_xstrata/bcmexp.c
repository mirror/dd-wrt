/*
 * Based on Bill Evans's getmale.c
 *
 * Copyright (C) 2012 by Vladimir Cotfas <unix_router@yahoo.com>
 *
 * Released under GPLv2
 */
#include <sys/select.h>
#include <sys/types.h>
#include <errno.h>
#include <pty.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <utmp.h>
#include <ctype.h>
#include <pty.h>

#define MPROMPT "BCM.0> "

int pty_phyle;

char recent_input[8];

struct termios tp1;
struct termios tp2;

int at_main_prompt(void);

void log_pty_data(int the_character);

int send_command(char *the_string, int add_cr, int expect_prompt);

int send_constant(char *the_string);

int wait_for_main_prompt(int bell);

int wait_for_one_character(void);

int wait_for_password_prompt(int bell);

int wait_for_string(char *the_string, int bell);

void wait_for_zero_status(int bell);

int at_main_prompt(void)
{
//  printf("[%s]\b", recent_input); fflush(stdout);
  return strstr(recent_input, MPROMPT) != NULL;
}				/* at_main_prompt() */

char outbuf[81920] = {0};

void p_printable(const char* buf, const int len)
{
  if(buf == NULL || len < 1) return;

  int i;
  for(i = 0; i < len; i++) {
    if(buf[i] == '\n' || isprint(buf[i])) {
      strncat(outbuf, (buf+i), 1);
      if(buf[i] == '\n') {
	do {
	  if(!strncmp(outbuf, "[H[J", 4)) continue;
          if(!strncmp(outbuf, MPROMPT, strlen(MPROMPT))) continue;
          write(STDOUT_FILENO, outbuf, strlen(outbuf));
        } while(0);
	outbuf[0] = '\0';
      }
    }
  }
}

void log_pty_data(int the_character)
{
  int jndex;

  char the_buffer[4];

  the_buffer[0] = the_character;

  p_printable(the_buffer, 1);
  //write(STDOUT_FILENO, the_buffer, 1);

  for (jndex = 0; jndex < sizeof(recent_input) - 1; jndex++) {
    recent_input[jndex] = recent_input[jndex + 1];
  }

  recent_input[jndex] = the_buffer[0];

}				/* log_pty_data() */

int send_command(char *the_string, int add_cr, int expect_prompt)
{
  int function_result;

  function_result = send_constant(the_string);

  if (function_result < 1) {
    return function_result;
  }

  if (add_cr) {
    function_result = send_constant("\r");

    if (function_result < 1) {
      return function_result;
    }
  }

  if (expect_prompt) {
    function_result = wait_for_one_character();

    if (function_result < 1) {
      return function_result;
    }

    function_result = wait_for_main_prompt(expect_prompt - 1);

    if (function_result < 1) {
      return function_result;
    }
  }

  return function_result;

}				/* send_command() */

int send_constant(char *the_string)
{
  int input_index;
  int max_fd;
  int select_result;

  char the_buffer[4];

  ssize_t read_result;

  struct timeval timeout;

  fd_set read_fds;
  fd_set write_fds;

  input_index = 0;

  for (;;) {
    FD_ZERO(&read_fds);
    if (0) {
      FD_SET(pty_phyle, &read_fds);
    }

    FD_ZERO(&write_fds);
    FD_SET(pty_phyle, &write_fds);

    max_fd = pty_phyle;

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    select_result = select(max_fd + 1, &read_fds, &write_fds, NULL, &timeout);

    if (select_result < 1) {
      if (0) {
	the_buffer[0] = '.';

	//write(STDOUT_FILENO, the_buffer, 1);
	p_printable(the_buffer, 1);
      }

      continue;			/* <--------- */
    }

    if (FD_ISSET(pty_phyle, &read_fds)) {
      read_result = read(pty_phyle, the_buffer, 1);

      if (read_result < 0) {
	return -1;		/* <--------- */
      }

      if (read_result == 0) {
	fprintf(stderr, "read(pty) returned 0\n");

	return 0;
      }

      log_pty_data(the_buffer[0]);
    } else if (FD_ISSET(pty_phyle, &write_fds)) {
      if (the_string[input_index] == 0) {
	return 1;		/* <--------- */
      }

      the_buffer[0] = the_string[input_index++];

      write(pty_phyle, the_buffer, 1);
    }
  }

}				/* send_constant() */

int wait_for_main_prompt(int bell)
{
  int max_fd;
  int select_result;

  char the_buffer[4];

  ssize_t read_result;

  struct timeval timeout;

  fd_set read_fds;

  for (;;) {
    FD_ZERO(&read_fds);
    FD_SET(pty_phyle, &read_fds);

    max_fd = pty_phyle;

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    select_result = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);

    if (select_result < 1) {
      if (0) {
	the_buffer[0] = '.';

	//write(STDOUT_FILENO, the_buffer, 1);
	p_printable(the_buffer, 1);
      }

      continue;			/* <--------- */
    }

    if (FD_ISSET(pty_phyle, &read_fds)) {
      read_result = read(pty_phyle, the_buffer, 1);

      if (read_result < 0) {
	return -1;		/* <--------- */
      }

      if (read_result == 0) {
	fprintf(stderr, "read(pty) returned 0\n");

	return 0;
      }

      log_pty_data(the_buffer[0]);

      if (at_main_prompt()) {
	if (bell) {
	  the_buffer[0] = 7;

	  //write(STDOUT_FILENO, the_buffer, 1);
	  p_printable(the_buffer, 1);
	}

	return 1;
      }
    }
  }

}				/* wait_for_main_prompt() */

int wait_for_one_character(void)
{
  int max_fd;
  int select_result;

  char the_buffer[4];

  ssize_t read_result;

  struct timeval timeout;

  fd_set read_fds;

  for (;;) {
    FD_ZERO(&read_fds);
    FD_SET(pty_phyle, &read_fds);

    max_fd = pty_phyle;

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    select_result = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);

    if (select_result < 1) {
      if (0) {
	the_buffer[0] = '.';

	//write(STDOUT_FILENO, the_buffer, 1);
	p_printable(the_buffer, 1);
      }

      continue;			/* <--------- */
    }

    if (FD_ISSET(pty_phyle, &read_fds)) {
      read_result = read(pty_phyle, the_buffer, 1);

      if (read_result < 0) {
	return -1;		/* <--------- */
      }

      if (read_result == 0) {
	fprintf(stderr, "read(pty) returned 0\n");

	return 0;
      }

      log_pty_data(the_buffer[0]);

      return 1;
    }
  }
}				/* wait_for_one_character() */

int wait_for_string(char *the_string, int bell)
{
  int found_it;
  int jndex;
  int kndex;
  int max_fd;
  int select_result;

  char the_buffer[4];

  ssize_t read_result;

  struct timeval timeout;

  fd_set read_fds;

  for (;;) {
    FD_ZERO(&read_fds);
    FD_SET(pty_phyle, &read_fds);

    max_fd = pty_phyle;

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    select_result = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);

    if (select_result < 1) {
      if (0) {
	the_buffer[0] = '.';

	//write(STDOUT_FILENO, the_buffer, 1);
	p_printable(the_buffer, 1);
      }

      continue;			/* <--------- */
    }

    if (FD_ISSET(pty_phyle, &read_fds)) {
      read_result = read(pty_phyle, the_buffer, 1);

      if (read_result < 0) {
	return -1;		/* <--------- */
      }

      if (read_result == 0) {
	fprintf(stderr, "read(pty) returned 0\n");

	return 0;
      }

      log_pty_data(the_buffer[0]);

      found_it = 1;

      jndex = sizeof(recent_input) - 1;
      kndex = strlen(the_string) - 1;

      for (;;) {
	if ((jndex < 0) || (kndex < 0)) {
	  break;		/* <--------- */
	}

	if (recent_input[jndex] != the_string[kndex]) {
	  found_it = 0;

	  break;		/* <--------- */
	}

	jndex--;
	kndex--;
      }

      if (found_it) {
	if (bell) {
	  the_buffer[0] = 7;

	  //write(STDOUT_FILENO, the_buffer, 1);
	  p_printable(the_buffer, 1);
	}

	return 1;
      }
    }
  }

}				/* wait_for_string() */

/*--------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
  int i;
  pid_t the_child;
  struct winsize window_size;

  if(argc < 2) return 0;

  char cmd[256] = {0};
  for(i = 1; i < argc; i++) {
	if(i != 1) strncat(cmd, " ", 255);
	strncat(cmd, argv[i], 255);
  }
  strncat(cmd, "\n", 255);
	
  recent_input[sizeof(recent_input) - 1] = 0;

  ioctl(STDIN_FILENO, TIOCGWINSZ, &window_size);

  the_child = forkpty(&pty_phyle, NULL, NULL, &window_size);

  if (the_child == 0) {
    execl("/bin/bcm", "bcm", NULL);

    exit(0);
  }

  tcgetattr(0, &tp1);

  tp2 = tp1;

  tp2.c_iflag &= ~ICRNL;
  tp2.c_lflag &= ~ICANON;
  tp2.c_lflag &= ~ECHO;
  tp2.c_cc[VMIN] = 1;
  tp2.c_cc[VTIME] = 0;
  tp2.c_cc[VINTR] = 0xFF;
  tp2.c_cc[VSUSP] = 0xFF;
  tp2.c_cc[VQUIT] = 0xFF;

  if (tcsetattr(0, TCSANOW, &tp2) != 0) {
    fprintf(stderr, "setting attributes failed\n");
  }

  //sleep(1);

  wait_for_main_prompt(0);

  send_constant(cmd);
  wait_for_string(MPROMPT,0);
  wait_for_string(MPROMPT,0);

  tcsetattr(0, TCSANOW, &tp1);

  return 0;
}
