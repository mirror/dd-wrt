// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2004-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */
#ifndef MKFS_PROTO_H_
#define MKFS_PROTO_H_

char *setup_proto(char *fname);
void parse_proto(struct xfs_mount *mp, struct fsxattr *fsx, char **pp,
		int proto_slashes_are_spaces);
void res_failed(int err);

#endif /* MKFS_PROTO_H_ */
