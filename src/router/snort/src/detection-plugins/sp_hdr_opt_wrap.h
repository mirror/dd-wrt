/****************************************************************************
 * Copyright (C) 2008-2011 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************/

/* Necessary hash/wrapper functions to put a .so rule's HdrOptCheck option
 * directly on the rule option tree. */

#ifndef __SP_HDR_OPT_WRAP_H__
#define __SP_HDR_OPT_WRAP_H__

#include "sf_engine/sf_snort_plugin_api.h"
#include "sfhashfcn.h"
#include "detection_options.h"

uint32_t HdrOptCheckHash(void *d);
int HdrOptCheckCompare(void *l, void *r);
int HdrOptEval(void *option_data, Packet *p);

#endif /* __SP_HDR_OPT_WRAP_H__ */
