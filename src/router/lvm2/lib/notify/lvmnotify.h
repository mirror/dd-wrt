/*
 * Copyright (C) 2015 Red Hat, Inc.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 */

#ifndef _LVMNOTIFY_H
#define _LVMNOTIFY_H

int lvmnotify_is_supported(void);
void lvmnotify_send(struct cmd_context *cmd);
void set_vg_notify(struct cmd_context *cmd);
void set_lv_notify(struct cmd_context *cmd);
void set_pv_notify(struct cmd_context *cmd);

#endif

