/* SPDX-License-Identifier: GPL-2.0-or-later */
/*****************************************************************************
  Copyright (c) 2006 EMC Corporation.
  Copyright (c) 2011 Factor-SPE

  Authors: Srinivas Aji <Aji_Srinivas@emc.com>
  Authors: Vitalii Demianets <dvitasgs@gmail.com>

******************************************************************************/

#ifndef CTL_SOCKET_CLIENT_H
#define CTL_SOCKET_CLIENT_H

#include "ctl_functions.h"

int send_ctl_message(int cmd, void *inbuf, int lin, void *outbuf, int lout,
                     LogString *log, int *res);
int ctl_client_init(void);
void ctl_client_cleanup(void);

#endif /* CTL_SOCKET_CLIENT_H */
