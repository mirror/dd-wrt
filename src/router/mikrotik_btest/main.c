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
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

#include "parse_opt.h"
#include "tcptest.h"
#include "return_codes.h"

int main (int argc, char **argv) {
	program_options_t options;
	int16_t rv;

	rv = parse_opt(&argc, argv, &options);
	if ( rv == RETURN_ERROR){
		fprintf(stderr, "Error parsing commands\n");
		exit(EXIT_FAILURE);
	}
	else if (rv == RETURN_PRINT_HELP)
		exit(EXIT_SUCCESS);

	if (check_opt(&options) != RETURN_OK)
		exit(EXIT_FAILURE);

	tcptest(options.host, options.port, options.user, options.password, options.direction, options.mtu, options.time);
	
	exit(EXIT_SUCCESS);
}
