/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2005-2013 Sourcefire, Inc.
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
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "app_forecast.h"

static AFActKey master_key;

static inline void rekeyMasterAFActKey (SFSnortPacket *p, int dir, tAppId forecast)
{
    sfaddr_t *src;

    src = dir ? GET_DST_IP(p) : GET_SRC_IP(p);
    memcpy(master_key.ip, sfaddr_get_ip6_ptr(src), sizeof(master_key.ip));
    master_key.forecast = forecast;
}

void checkSessionForAFIndicator(SFSnortPacket *p, int dir, const tAppIdConfig *pConfig, tAppId indicator)
{
    AFElement *ind_element;
    if (!(ind_element = (AFElement*)sfxhash_find(pConfig->AF_indicators, &indicator)))
        return;

    rekeyMasterAFActKey(p, dir, ind_element->forecast);

    AFActVal *test_active_value;
    if ((test_active_value = (AFActVal*)sfxhash_find(pConfig->AF_actives, &master_key)))
    {
        test_active_value->last = GetPacketRealTime;
        test_active_value->target = ind_element->target;
        return;
    }

    AFActVal new_active_value;
    new_active_value.target = ind_element->target;
    new_active_value.last = GetPacketRealTime;

    sfxhash_add(pConfig->AF_actives, &master_key, &new_active_value);
}

tAppId checkSessionForAFForecast(tAppIdData *session, SFSnortPacket *p, int dir, const tAppIdConfig *pConfig, tAppId forecast)
{
    AFActVal *check_act_val;

    rekeyMasterAFActKey(p, dir, forecast);

    //get out if there is no value
    if (!(check_act_val = (AFActVal*)sfxhash_find(pConfig->AF_actives, &master_key)))
        return APP_ID_UNKNOWN;

    //if the value is older than 5 minutes, remove it and get out
    time_t age;
    age = GetPacketRealTime - check_act_val->last;
    if (age < 0 || age > 300)
    {
        sfxhash_remove(pConfig->AF_actives, &master_key);
        return APP_ID_UNKNOWN;
    }

    session->payloadAppId = check_act_val->target;
    return forecast;
}
