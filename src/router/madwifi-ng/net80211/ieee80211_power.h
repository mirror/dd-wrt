/*-
 * Copyright (c) 2001 Atsushi Onoe
 * Copyright (c) 2002-2005 Sam Leffler, Errno Consulting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: ieee80211_power.h 1441 2006-02-06 16:03:21Z mrenzmann $
 */
#ifndef _NET80211_IEEE80211_POWER_H_
#define _NET80211_IEEE80211_POWER_H_

struct ieee80211com;
struct ieee80211vap;

void	ieee80211_power_attach(struct ieee80211com *);
void	ieee80211_power_detach(struct ieee80211com *);
void	ieee80211_power_vattach(struct ieee80211vap *);
void	ieee80211_power_latevattach(struct ieee80211vap *);
void	ieee80211_power_vdetach(struct ieee80211vap *);

struct ieee80211_node;

int	ieee80211_node_saveq_drain(struct ieee80211_node *);
int	ieee80211_node_saveq_age(struct ieee80211_node *);
void	ieee80211_pwrsave(struct ieee80211_node *, struct sk_buff *);
void	ieee80211_node_pwrsave(struct ieee80211_node *, int);
void	ieee80211_sta_pwrsave(struct ieee80211vap *, int);
#endif /* _NET80211_IEEE80211_POWER_H_ */
