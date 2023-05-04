/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2012-2013 Sourcefire, Inc.
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

/**************************************************************************
 *
 * ssl_ha.h
 *
 * Authors: Michael Altizer <mialtize@cisco.com>, Russ Combs <rucombs@cisco.com>, Bhagyashree Bantwal <bbantwal@cisco.com>
 *
 * Description:
 *
 * SSL high availability exported functionality.
 *
 **************************************************************************/

#ifndef __SSL_HA_H__
#define __SSL_HA_H__

#ifdef ENABLE_HA

#include "ssl_config.h"

typedef enum
{
    HA_EVENT_UPDATE,
    HA_EVENT_DELETE,
    HA_EVENT_MAX
} HA_Event;

typedef uint32_t (*SSLHAProducerFunc)(uint8_t *buf, void *data_to_copy, uint32_t size);
typedef int (*SSLHAConsumerFunc)(const uint8_t *data, uint32_t length);

int RegisterSSLHAFuncs(uint32_t preproc_id, uint8_t subcode, uint32_t size, SSLHAProducerFunc produce, SSLHAConsumerFunc consume, SSLHAConsumerFunc deletion);
void UnregisterStreamHAFuncs(uint32_t preproc_id, uint8_t subcode);
//void SSLSetHAPendingBit(void *ssnptr, int bit);

void SSLHAInit(struct _SnortConfig *sc, char *args);
int SSLVerifyHAConfig(struct _SnortConfig *sc, void *config);
#if defined(SNORT_RELOAD)
void SSLHAReload(struct _SnortConfig *sc, char *args, void **new_config);
void *SSLHASwapReload( struct _SnortConfig *sc, void *data );
#endif
void SSLHAConfigFree(void *config);
void SSLHAPostConfigInit(struct _SnortConfig *sc, int unused, void *arg);
void SSLCleanHA(void);
void SSLPrintHAStats(void);
int DisplaySSLHAStats(char *buffer);
void SSLResetHAStats(void);
void SSLProcessHA(int ssl_index, bool update, void *data, uint32_t ssl_size);

#endif /* ENABLE_HA */

#endif /* __SSL_HA_H__ */
