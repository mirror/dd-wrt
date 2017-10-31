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
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>

#include "direction.h"
#include "return_codes.h"
#include "parse_opt.h"

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t strlcpy(char *dst, const char *src, size_t size) {
	char *d = dst;
	const char *s = src;
	size_t n = size;

	/* Copy as many bytes as will fit */
	if (n != 0 && --n != 0) {
		do {
			if ((*d++ = *s++) == 0)
				break;
		} while (--n != 0);
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (size != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);	/* count does not include NUL */
}


void set_default_opt(program_options_t *options){
	memset(options, 0, sizeof(*options));
	options->time = DEFAULT_TIME;
	options->mtu = DEFAULT_MTU;
	options->direction = DEFAULT_DIRECTION;
	strcpy(options->port, DEFAULT_PORT);
}

void print_help(){
	printf("This is a bandwidth test client compatible with Mikrotik RouterOS.\n\n");
	printf("Usage: mikrotik_btest [options] HOST[:PORT]\n\n");
	printf("Options:\n");
	printf("\t-h, --help \t\t\t\t show this help message and exit.\n");
	printf("\t-t DURATION, --time=DURATION \t\t test duration in seconds. Default 8 seconds.\n");
	printf("\t-m MTU, --mtu=MTU \t\t\t MTU in bytes for the test. Default 1500 bytes.\n");
	printf("\t-d DIRECTION, --direction=DIRECTION \t Test direction (receive, send, both). Default receive.\n");
	printf("\t-u USER, --user=USER\n");
	printf("\t-p PASSWORD, --password=PASSWORD\n");
}

int parse_opt(int *argc, char **argv, program_options_t *options){
	int32_t c, option_index;
	char * pch;

	static struct option long_options[] = {
	        {"time",  required_argument, 0, 't'},
	        {"mtu",  required_argument, 0, 'm'},
	        {"direction",  required_argument, 0, 'd'},
	        {"user",    required_argument, 0, 'u'},
	        {"password",    required_argument, 0, 'p'},
	        {"help",   no_argument, 0, 'h'},
	        {0, 0, 0, 0}};

	set_default_opt(options);

	while ((c = getopt_long(*argc, argv, "p:t:m:d:u:a:h", long_options, &option_index)) != -1) {
		switch(c){
			case 't':
				options->time = atoi(optarg);
				break;
			case 'm':
				options->mtu = atoi(optarg);
				break;
			case 'd':
				strlcpy(options->direction_string, optarg, sizeof(options->direction_string));
				break;
			case 'u':
				strlcpy(options->user, optarg, sizeof(options->user));
				break;
			case 'p':
				strlcpy(options->password, optarg, sizeof(options->password));
				break;
			case 'h':
				print_help();
				return RETURN_PRINT_HELP;
				break;
			default:
				return RETURN_ERROR;
		}
	}
	if (*argc - optind == 1){
		if ((pch = strtok(argv[optind], ":")) != NULL){
			strlcpy(options->host, pch, sizeof(options->host));
			if((pch = strtok(NULL, ":")) != NULL)
				strlcpy(options->port, pch, sizeof(options->port));
		}
		else
			return RETURN_ERROR;
	}
	else
		return RETURN_ERROR;
	return RETURN_OK;
}

int check_opt(program_options_t *options){
	int32_t port;

	port = atoi(options->port);
	if (port < 1 || port >= 65536){
		fprintf(stderr, "Port %s is not valid\n", options->port);
		return RETURN_ERROR;
	}

	if (strlen(options->direction_string) > 0){
		if (strcmp(options->direction_string, "receive") == 0){
			options->direction = RECEIVE;
		}
		else if (strcmp(options->direction_string, "send") == 0){
			options->direction = SEND;
		}
		else if (strcmp(options->direction_string, "both") == 0){
			options->direction = BOTH;
		}
		else{
			fprintf(stderr, "Direction %s is not valid\n", options->direction_string);
			return RETURN_ERROR;
		}
	}

	if (options->mtu == 0){
		fprintf(stderr, "MTU is not valid\n");
		return RETURN_ERROR;
	}
	else if(options->mtu > MAX_MTU){
		fprintf(stderr, "MTU is too big\n");
		return RETURN_ERROR;
	}

	if (options->time == 0){
		fprintf(stderr, "Time is not valid\n");
		return RETURN_ERROR;
	}
	else if(options->time >= MAX_TIME){
		fprintf(stderr, "Time has to be less than 30 seconds\n");
		return RETURN_ERROR;
	}
	
	return RETURN_OK;
}

