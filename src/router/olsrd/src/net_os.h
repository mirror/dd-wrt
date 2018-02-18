/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
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
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

/*
 * This file defines the OS dependent network related functions
 * that MUST be available to olsrd.
 * The implementations of the functions should be found in
 * <OS>/net.c (e.g. linux/net.c)
 */

#ifndef _OLSR_NET_OS_H
#define _OLSR_NET_OS_H

#include "olsr_types.h"
#include "interfaces.h"

/* OS dependent functions */
ssize_t olsr_sendto(int, const void *, size_t, int, const struct sockaddr *, socklen_t);

ssize_t olsr_recvfrom(int, void *, size_t, int, struct sockaddr *, socklen_t *);

int olsr_select(int, fd_set *, fd_set *, fd_set *, struct timeval *);

int bind_socket_to_device(int, char *);

int convert_ip_to_mac(union olsr_ip_addr *, struct sockaddr *, char *);

void net_os_set_global_ifoptions(void);
int net_os_set_ifoptions(const char *if_name, struct interface_olsr *iface);
int net_os_restore_ifoptions(void);

int gethemusocket(struct sockaddr_in *pin);

int getsocket(int, struct interface_olsr *);

int getsocket6(int, struct interface_olsr *);

int get_ipv6_address(char *, struct sockaddr_in6 *, struct olsr_ip_prefix *);

int calculate_if_metric(char *);

int check_wireless_interface(char *);

bool is_if_link_up(char *);

int join_mcast(struct interface_olsr *, int);

bool olsr_if_isup(const char * dev);
int olsr_if_set_state(const char *dev, bool up);
#endif /* _OLSR_NET_OS_H */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
