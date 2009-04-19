#ifndef _BMF_BMF_H
#define _BMF_BMF_H

/*
 * OLSR Basic Multicast Forwarding (BMF) plugin.
 * Copyright (c) 2005 - 2007, Thales Communications, Huizen, The Netherlands.
 * Written by Erik Tromp.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of Thales, BMF nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* -------------------------------------------------------------------------
 * File       : Bmf.h
 * Description: Multicast forwarding functions
 * Created    : 29 Jun 2006
 *
 * ------------------------------------------------------------------------- */

#include "olsrd_plugin.h"       /* union set_plugin_parameter_addon */

/* BMF plugin data */
#define PLUGIN_NAME "OLSRD Basic Multicast Forwarding (BMF) plugin"
#define PLUGIN_NAME_SHORT "OLSRD BMF"
#define PLUGIN_VERSION "1.5.3 (" __DATE__ " " __TIME__ ")"
#define PLUGIN_COPYRIGHT "  (C) Thales Communications Huizen, Netherlands"
#define PLUGIN_AUTHOR "  Erik Tromp (eriktromp@users.sourceforge.net)"
#define MOD_DESC PLUGIN_NAME " " PLUGIN_VERSION "\n" PLUGIN_COPYRIGHT "\n" PLUGIN_AUTHOR
#define PLUGIN_INTERFACE_VERSION 5

/* UDP-Port on which multicast packets are encapsulated */
#define BMF_ENCAP_PORT 50698

/* Forward declaration of OLSR interface type */
struct interface;

extern int FanOutLimit;
extern int BroadcastRetransmitCount;

void BmfPError(const char *format, ...) __attribute__ ((format(printf, 1, 2)));
union olsr_ip_addr *MainAddressOf(union olsr_ip_addr *ip);
int InterfaceChange(struct interface *interf, int action);
int SetFanOutLimit(const char *value, void *data, set_plugin_parameter_addon addon);
int InitBmf(struct interface *skipThisIntf);
void CloseBmf(void);

#endif /* _BMF_BMF_H */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
