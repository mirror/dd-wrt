/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#ifndef __SMBD_TRANSPORT_TCP_H__
#define __SMBD_TRANSPORT_TCP_H__

int smbd_tcp_set_interfaces(char *ifc_list, int ifc_list_sz);
int smbd_tcp_init(void);
void smbd_tcp_destroy(void);

#endif /* __SMBD_TRANSPORT_TCP_H__ */
