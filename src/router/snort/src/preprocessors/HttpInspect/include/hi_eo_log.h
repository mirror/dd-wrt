/****************************************************************************
 *
 * Copyright (C) 2003-2011 Sourcefire, Inc.
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
 
#ifndef __HI_EO_LOG_H__
#define __HI_EO_LOG_H__

#include "hi_include.h"
#include "hi_si.h"
#include "hi_return_codes.h"

static INLINE int hi_eo_generate_event(HI_SESSION *Session, int iAlert)
{
    if(iAlert && !(Session->norm_flags & HI_BODY) &&
       !Session->server_conf->no_alerts)
    {
        return HI_BOOL_TRUE;
    }

    return HI_BOOL_FALSE;
}

int hi_eo_client_event_log(HI_SESSION *Session, int iEvent, void *data,
        void (*free_data)(void *));

int hi_eo_server_event_log(HI_SESSION *Session, int iEvent, void *data,
        void (*free_data)(void *));
int hi_eo_anom_server_event_log(HI_SESSION *Session, int iEvent, void *data,
        void (*free_data)(void *));

#endif
