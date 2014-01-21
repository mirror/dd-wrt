/*
    Mac-Telnet - Connect to RouterOS or mactelnetd devices via MAC address
    Copyright (C) 2010, Håkon Nessjøen <haakon.nessjoen@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_term;

int raw_term() {
	struct termios new;

	if (tcgetattr(STDIN_FILENO, &orig_term) < 0) {
		perror("tcgetattr");
		return -1;
	}

	memcpy(&new, &orig_term, sizeof(struct termios) );

	/* raw mode, from tcsetattr man page */
	new.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
	new.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
	new.c_cflag &= ~(CSIZE|PARENB);
	new.c_cflag |= CS8;

	if (tcsetattr(STDIN_FILENO, TCSANOW, &new) < 0) {
		perror("tcsetattr");
		return -1;
	}
	return 0;
}

int reset_term() {
	if (tcsetattr(STDIN_FILENO, TCSANOW, &orig_term) < 0) {
		perror("tcsetattr");
		return -1;
	}
	return 0;
}

int get_terminal_size(unsigned short *width, unsigned short *height) {
	struct winsize ws;

	if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) != 0) {
		perror("TIOCGWINSZ");
		return -1;
	}

	*width = ws.ws_col;
	*height = ws.ws_row;

	return 0;
}

int set_terminal_size(int fd, unsigned short width, unsigned short height) {
	struct winsize ws;

	ws.ws_col = width;
	ws.ws_row = height;

	if (ioctl(fd, TIOCSWINSZ, &ws) != 0) {
		perror("TIOCSWINSZ");
		return -1;
	}

	return 0;
}

