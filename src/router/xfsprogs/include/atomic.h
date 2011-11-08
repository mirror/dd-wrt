/*
 * Copyright (c) 2011 RedHat, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef __ATOMIC_H__
#define __ATOMIC_H__

/*
 * Warning: These are not really atomic at all. They are wrappers around the
 * kernel atomic variable interface. If we do need these variables to be atomic
 * (due to multithreading of the code that uses them) we need to add some
 * pthreads magic here.
 */
typedef	int32_t	atomic_t;
typedef	int64_t	atomic64_t;

#define atomic_inc_return(x)	(++(*(x)))
#define atomic_dec_return(x)	(--(*(x)))

#define atomic64_read(x)	*(x)
#define atomic64_set(x, v)	(*(x) = v)

#endif /* __ATOMIC_H__ */

