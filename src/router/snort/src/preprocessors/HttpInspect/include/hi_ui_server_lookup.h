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
 
#ifndef __HI_UI_SERVER_LOOKUP_H__
#define __HI_UI_SERVER_LOOKUP_H__

#include "hi_include.h"
#include "hi_ui_config.h"

int hi_ui_server_lookup_init(SERVER_LOOKUP **ServerLookup);
int hi_ui_server_lookup_add(SERVER_LOOKUP *ServerLookup, sfip_t *Ip, 
                            HTTPINSPECT_CONF *ServerConf);

HTTPINSPECT_CONF *hi_ui_server_lookup_find(SERVER_LOOKUP *ServerLookup, 
                                            snort_ip_p Ip, int *iError);
HTTPINSPECT_CONF *hi_ui_server_lookup_first(SERVER_LOOKUP *ServerLookup,
                                            int *iError);
HTTPINSPECT_CONF *hi_ui_server_lookup_next(SERVER_LOOKUP *ServerLookup,
                                           int *iError);
void  hi_ui_server_lookup_destroy(SERVER_LOOKUP *ServerLookup);
void hi_ui_server_iterate(
        SERVER_LOOKUP *ServerLookup, 
        void (*userfunc)(void *)
        );
#endif
