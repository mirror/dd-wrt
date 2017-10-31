/*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef PARSE_OPT_H
#define PARSE_OPT_H

#include <stdint.h>

#include "direction.h"

struct program_options {
	char host[256];
	char port[16];
	uint16_t time;
	uint16_t mtu;
	char direction_string[8];
	direction_t direction;
	char user[32];
	char password[32];
};
typedef struct program_options  program_options_t;


int parse_opt(int *argc, char **argv, program_options_t *options);

int check_opt(program_options_t *options);

#define DEFAULT_MTU 1500
#define DEFAULT_TIME 8.0
#define MAX_TIME 30.0
#define MAX_MTU 9000
#define DEFAULT_DIRECTION RECEIVE
#define DEFAULT_PORT "2000"

#endif
