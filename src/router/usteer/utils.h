/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Copyright (C) 2020 embedd.ch 
 *   Copyright (C) 2020 Felix Fietkau <nbd@nbd.name> 
 *   Copyright (C) 2020 John Crispin <john@phrozen.org> 
 */

#ifndef __APMGR_UTILS_H
#define __APMGR_UTILS_H

#define MSG(_nr, _format, ...) debug_msg(MSG_##_nr, __func__, __LINE__, _format, ##__VA_ARGS__)
#define MSG_CONT(_nr, _format, ...) debug_msg_cont(MSG_##_nr, _format, ##__VA_ARGS__)

#define MAC_ADDR_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_ADDR_DATA(_a) \
	((const uint8_t *)(_a))[0], \
	((const uint8_t *)(_a))[1], \
	((const uint8_t *)(_a))[2], \
	((const uint8_t *)(_a))[3], \
	((const uint8_t *)(_a))[4], \
	((const uint8_t *)(_a))[5]

enum usteer_debug {
	MSG_FATAL,
	MSG_INFO,
	MSG_VERBOSE,
	MSG_DEBUG,
	MSG_NETWORK,
	MSG_DEBUG_ALL,
};

extern void log_msg(char *msg);
extern void debug_msg(int level, const char *func, int line, const char *format, ...);
extern void debug_msg_cont(int level, const char *format, ...);

#define __usteer_init __attribute__((constructor))

#endif
