// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2013 Red Hat, Inc.
 * All Rights Reserved.
 */
#ifndef __XFS_DB_SYMLINK_H
#define __XFS_DB_SYMLINK_H

extern const struct field	symlink_crc_hfld[];
extern const struct field	symlink_crc_flds[];

extern int	symlink_size(void *obj, int startoff, int idx);

#endif /* __XFS_DB_SYMLINK_H */
