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
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdint.h>

#include "tcptest.h"
#include "tcptest_thread.h"
#include "direction.h"
#include "return_codes.h"
#include "utils.h"

int16_t tcptest(char *host, char *port, char *user, char *password, direction_t direction, uint16_t mtu, uint16_t time){
	int32_t sockfd;
	double Mbits, bits, asleep_time, max_time = time, elapsed_time = 0;
	struct timespec t0, t1;
	pthread_t threads[2];
	thread_args_t threads_arg[2];
	pthread_mutex_t mutexes[2];
	uint64_t total_bytes[2];
	pthread_attr_t attr;

	if ((sockfd = open_socket(host, port)) == RETURN_ERROR)
		return RETURN_ERROR;

	if (init_test(sockfd, user, password, direction, mtu) == RETURN_ERROR){
		close(sockfd);
		return RETURN_ERROR;
	}
	close(sockfd);

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_mutex_init(&mutexes[RECEIVE], NULL);
	pthread_mutex_init(&mutexes[SEND], NULL);

	if (direction == BOTH){
		init_thread_args(&threads_arg[RECEIVE], &mutexes[RECEIVE], mtu, RECEIVE, host, port, user, password);
		total_bytes[RECEIVE] = 0;
		pthread_create(&threads[RECEIVE], &attr, tcptest_thread, (void *) &threads_arg[RECEIVE]);
		init_thread_args(&threads_arg[SEND], &mutexes[SEND], mtu, SEND, host, port, user, password);
		total_bytes[SEND] = 0;
		pthread_create(&threads[SEND], &attr, tcptest_thread, (void *) &threads_arg[SEND]);
	}
	else{
		init_thread_args(&threads_arg[direction], &mutexes[direction], mtu, direction, host, port, user, password);
		total_bytes[direction] = 0;
		pthread_create(&threads[direction], NULL, tcptest_thread, (void *) &threads_arg[direction]);
	}

	do{
		clock_gettime(CLOCK_MONOTONIC, &t0);
		sleep(1);
		clock_gettime(CLOCK_MONOTONIC, &t1);
		asleep_time = ((double) (t1.tv_sec - t0.tv_sec)) + (((double) (t1.tv_nsec - t0.tv_nsec)) * 1.0e-9);

		if (direction == RECEIVE || direction == BOTH){
			pthread_mutex_lock(&mutexes[RECEIVE]);
			if (!threads_arg[RECEIVE].alive){
				pthread_mutex_unlock(&mutexes[RECEIVE]);
				printf("\r");
				break;
			}
			total_bytes[RECEIVE] += threads_arg[RECEIVE].bytes;
			bits = threads_arg[RECEIVE].bytes * 8.0;
			Mbits = bits / 1048576.0;
			threads_arg[RECEIVE].bytes = 0;
			pthread_mutex_unlock(&mutexes[RECEIVE]);
			printf("Rx: %7.2f Mb/s", Mbits / asleep_time);
		}
		if (direction == BOTH){
			printf("\t");
		}
		if (direction == SEND || direction == BOTH){
			pthread_mutex_lock(&mutexes[SEND]);
			if (!threads_arg[SEND].alive){
				pthread_mutex_unlock(&mutexes[SEND]);
				printf("\r");
				break;
			}
			total_bytes[SEND] += threads_arg[SEND].bytes;
			bits = threads_arg[SEND].bytes * 8.0;
			Mbits = bits / 1048576.0;
			threads_arg[SEND].bytes = 0;
			pthread_mutex_unlock(&mutexes[SEND]);
			printf("Tx: %7.2f Mb/s", Mbits / asleep_time);
		}
		printf("\r");
		fflush(stdout);
		clock_gettime(CLOCK_MONOTONIC, &t1);
		elapsed_time += ((double) (t1.tv_sec - t0.tv_sec)) + (((double) (t1.tv_nsec - t0.tv_nsec)) * 1.0e-9);
	}while (elapsed_time <= max_time);

	pthread_mutex_lock(&mutexes[RECEIVE]);
	threads_arg[RECEIVE].alive = 0;
	pthread_mutex_unlock(&mutexes[RECEIVE]);

	pthread_mutex_lock(&mutexes[SEND]);
	threads_arg[SEND].alive = 0;
	pthread_mutex_unlock(&mutexes[SEND]);

	if (direction == RECEIVE || direction == BOTH){
		pthread_join(threads[RECEIVE], NULL);
		bits = total_bytes[RECEIVE] * 8.0;
		Mbits = bits / 1048576.0;
		printf("Rx: %7.2f Mb/s", Mbits / elapsed_time);
	}
	if (direction == BOTH)
		printf("\t");
	if (direction == SEND || direction == BOTH){
		pthread_join(threads[SEND], NULL);
		bits = total_bytes[SEND] * 8.0;
		Mbits = bits / 1048576.0;
		printf("Tx: %7.2f Mb/s", Mbits / elapsed_time);
	}
	printf("\n");
	fflush(stdout);

	pthread_mutex_destroy(&mutexes[RECEIVE]);
	pthread_mutex_destroy(&mutexes[SEND]);

	return RETURN_OK;
}
