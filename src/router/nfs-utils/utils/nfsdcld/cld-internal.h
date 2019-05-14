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

struct cld_client {
	int			cl_fd;
	struct event		cl_event;
	struct cld_msg	cl_msg;
};

uint64_t current_epoch;
uint64_t recovery_epoch;
int first_time;
int num_cltrack_records;
int num_legacy_records;

#endif /* _CLD_INTERNAL_H_ */
