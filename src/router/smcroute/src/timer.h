/* Timer helper functions
 *
 * Copyright (C) 2017-2021  Joachim Wiberg <troglobit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef SMCROUTE_TIMER_H_
#define SMCROUTE_TIMER_H_

int  timer_init (void);
void timer_exit (void);

int  timer_add  (int period, void (*cb)(void *), void *arg);
int  timer_del  (void (*cb)(void *), void *arg);

#endif /* SMCROUTE_TIMER_H_ */
