/*
** Copyright (C) 2002-2011 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/* $Id$ */

#ifndef __SPO_UNIFIED2_H__
#define __SPO_UNIFIED2_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef WIN32
#include <netinet/in.h>
#endif
#include "decode.h" /* for struct in6_addr -- maybe move to sf_types.h? */
#include "sf_types.h"

void Unified2Setup(void);

#endif  /* __SPO_UNIFIED_H__ */
