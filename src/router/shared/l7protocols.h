/*
 * l7protocols.c - generates main.c for service handling
 *
 * Copyright (C) 2025 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */


typedef struct _l7filters {
	char *name;
	unsigned short protocol : 3; // 1=p2p, 0=l7, 2=opendpi
	unsigned short level : 13;
	char *matchdep; // for risk only
} l7filters;
#define L7_ONLY 0
#define PDPI_ONLY 1
#define NDPI_ONLY 2
#define NDPI_RISK 3
#define FILTER_CUSTOM 4

#ifdef HAVE_OPENDPI
#define DPI 2 //open dpi based
#define PDPI 2 //open dpi based
#else
#define DPI 0 //default l7
#define PDPI 1 //default p2p
#endif
//Added ,  (in extra), dazhihui, .

