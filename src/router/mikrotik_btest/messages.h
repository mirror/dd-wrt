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
#ifndef MESSAGES_H
#define MESSAGES_H

#include <stdint.h>

extern uint8_t MSG_OK[4];
extern uint8_t MSG_NOOK[4];

extern uint8_t MSG_TCP_DOWN[16];
extern uint8_t MSG_TCP_UP[16];
extern uint8_t MSG_TCP_BOTH[16];

extern uint8_t CHALLENGE_HEADER[4];

#define CHALLENGE_TOTAL_SIZE 20
#define CHALLENGE_SIZE 16
#define RESPONSE_SIZE 48

#endif
