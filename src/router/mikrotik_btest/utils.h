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
#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#include "direction.h"

#define MAX_RETRY 60

int16_t init_test(int32_t sockfd, char *user, char *password,  direction_t direction, uint16_t mtu);
int16_t open_socket(char *host, char *port);
void craft_response(char *user, char *password, uint8_t *challenge, uint8_t *response);
int16_t recv_msg(int32_t sockfd, unsigned char *buf, uint16_t bufsize, uint8_t *msg, int32_t *recvbytes);
int16_t send_msg(int32_t sockfd, uint8_t *msg, uint16_t len);

#endif
