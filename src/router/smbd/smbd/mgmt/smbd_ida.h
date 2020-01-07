/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#ifndef __SMBD_IDA_MANAGEMENT_H__
#define __SMBD_IDA_MANAGEMENT_H__

#include <linux/slab.h>
#include <linux/idr.h>

struct smbd_ida {
	struct ida	map;
};

struct smbd_ida *smbd_ida_alloc(void);
void smbd_ida_free(struct smbd_ida *ida);

/*
 * 2.2.1.6.7 TID Generation
 *    The value 0xFFFF MUST NOT be used as a valid TID. All other
 *    possible values for TID, including zero (0x0000), are valid.
 *    The value 0xFFFF is used to specify all TIDs or no TID,
 *    depending upon the context in which it is used.
 */
int smbd_acquire_smb1_tid(struct smbd_ida *ida);
int smbd_acquire_smb2_tid(struct smbd_ida *ida);

/*
 * 2.2.1.6.8 UID Generation
 *    The value 0xFFFE was declared reserved in the LAN Manager 1.0
 *    documentation, so a value of 0xFFFE SHOULD NOT be used as a
 *    valid UID.<21> All other possible values for a UID, excluding
 *    zero (0x0000), are valid.
 */
int smbd_acquire_smb1_uid(struct smbd_ida *ida);
int smbd_acquire_smb2_uid(struct smbd_ida *ida);
int smbd_acquire_async_msg_id(struct smbd_ida *ida);

int smbd_acquire_id(struct smbd_ida *ida);

void smbd_release_id(struct smbd_ida *ida, int id);
#endif /* __SMBD_IDA_MANAGEMENT_H__ */
