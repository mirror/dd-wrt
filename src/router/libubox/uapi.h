/**
 *   uapi - Common API macros
 *   Copyright (C) 2010 Steven Barth <steven@midlink.org>
 *   Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
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
 */

#ifndef UAPI_H_
#define UAPI_H_

#define API_CTOR			__attribute__((constructor))
#define API_DTOR			__attribute__((destructor))
#define API_HIDDEN			__attribute__((visibility("hidden")))
#define API_INTERNAL		__attribute__((visibility("internal")))
#define API_DEFAULT			__attribute__((visibility("default")))
#define API_ALLOC			__attribute__((malloc))
#define API_NONNULL(...)	__attribute__((nonnull(__VA_ARGS_)))
#define API_FORCEINLINE		__attribute__((always_inline)) inline

#define API_PACKED			__attribute__((packed))
#define API_CLEANUP(gc)		__attribute__((cleanup(gc)))
#endif /* UAPI_H_ */
