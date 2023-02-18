/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _CLD_INTERNAL_H_
#define _CLD_INTERNAL_H_

#if CLD_UPCALL_VERSION >= 2
#define UPCALL_VERSION		2
#else
#define UPCALL_VERSION		1
#endif

struct cld_client {
	int			cl_fd;
	struct event		*cl_event;
	union {
		struct cld_msg		cl_msg;
#if UPCALL_VERSION >= 2
		struct cld_msg_v2	cl_msg_v2;
#endif
	} cl_u;
};

extern uint64_t current_epoch;
extern uint64_t recovery_epoch;
extern int first_time;
extern int num_cltrack_records;
extern int num_legacy_records;

#endif /* _CLD_INTERNAL_H_ */
