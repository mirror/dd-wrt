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
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "tcptest_thread.h"
#include "return_codes.h"
#include "utils.h"

void init_thread_args(thread_args_t *args, pthread_mutex_t *mutex, uint16_t bufsize, direction_t direction, char *host, char *port, char *user, char *password){
	args->bufsize = bufsize;
	args->direction = direction;
	args->mutex = mutex;
	args->bytes = 0;
	args->alive = 1;
	args->host = host;
	args->port = port;
	args->user = user;
	args->password = password;
}


void *tcptest_thread(void *argument){
	int32_t sockfd, poll_ctrl;
	thread_args_t *args;
	uint8_t *buffer;
	uint16_t failed = 0;
	int32_t bytes = 0;
	struct pollfd pfd;

	args = (thread_args_t *) argument;

	if ((sockfd = open_socket(args->host, args->port)) == RETURN_ERROR)
			goto exit;

	if (init_test(sockfd, args->user, args->password, args->direction, args->bufsize) == RETURN_ERROR)
			goto close_and_exit;

	fcntl(sockfd, F_SETFL, O_NONBLOCK);
	pfd.fd = sockfd;
	if (args->direction == RECEIVE)
		pfd.events = POLLIN;
	else
		pfd.events = POLLOUT;

	buffer = (uint8_t *) malloc(args->bufsize);
	memset(buffer, 0, args->bufsize);

	pthread_mutex_lock(args->mutex);
	while (args->alive){
		pthread_mutex_unlock(args->mutex);
		poll_ctrl = poll(&pfd, 1, POLL_TIMEOUT);
		switch (poll_ctrl){
			case (-1 | 0):
				++failed;
				bytes = 0;
				break;
			default:
				switch (pfd.revents) {
					case POLLIN:
						if (args->direction == RECEIVE)
							bytes = recv(sockfd, buffer, args->bufsize, 0);
						break;
					case POLLOUT:
						if (args->direction == SEND)
							bytes = send(sockfd, buffer, args->bufsize, MSG_NOSIGNAL);
						break;
					default:
						++failed;
						bytes = 0;
						break;
					}
					if (bytes == -1)
						++failed;
					else
						failed = 0;
					if(failed >= MAX_RETRY){
						goto free_and_exit;
				}
				break;
		}
		pthread_mutex_lock(args->mutex);
		args->bytes += bytes;
	}
free_and_exit:
	free(buffer);
close_and_exit:
	close(sockfd);
exit:
	pthread_mutex_trylock(args->mutex);
	args->alive = 0;
	pthread_mutex_unlock(args->mutex);
	return NULL;
}
