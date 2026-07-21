/* SPDX-License-Identifier: GPL-2.0-or-later */
/*****************************************************************************
  Copyright (c) 2006 EMC Corporation.
  Copyright (c) 2011 Factor-SPE

  Authors: Srinivas Aji <Aji_Srinivas@emc.com>
  Authors: Vitalii Demianets <dvitasgs@gmail.com>

******************************************************************************/

#ifndef CTL_SOCKET_SERVER_H
#define CTL_SOCKET_SERVER_H

int ctl_socket_init(void);
void ctl_socket_cleanup(void);

extern int ctl_in_handler;
void _ctl_err_log(char *fmt, ...);

#define ctl_err_log(_fmt...) ({ if (ctl_in_handler) _ctl_err_log(_fmt); })

#endif /* CTL_SOCKET_SERVER_H */
