/*
 * Copyright (C) 2010 Robert Lougher <rob@jamvm.org.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#if 1
#define TRACE(fmt, ...)
#else
#define TRACE(fmt, ...) { printf(fmt "\n", ## __VA_ARGS__); fflush(stdout); }
#endif

#if 0
#define TBD(fmt, ...)
#define UNUSED(fmt, ...)
#define DEPRECATED(fmt, ...)
#define UNIMPLEMENTED(fmt, ...)
#else
#define TBD(fmt, ...) { printf(fmt "\n", ## __VA_ARGS__); fflush(stdout); }
#define UNUSED(fmt, ...) { printf(fmt "\n", ## __VA_ARGS__); fflush(stdout); }
#define DEPRECATED(fmt, ...) { printf(fmt "\n", ## __VA_ARGS__); fflush(stdout); }
#define UNIMPLEMENTED(fmt, ...) { printf(fmt "\n", ## __VA_ARGS__); fflush(stdout); }
#endif

#define IGNORED(fmt, ...)
