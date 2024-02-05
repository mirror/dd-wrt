// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

void	xfs_init(libxfs_init_t *args);

int	verify_sb(char			*sb_buf,
		xfs_sb_t		*sb,
		int			is_primary_sb);
int	verify_set_primary_sb(xfs_sb_t	*root_sb,
			int		sb_index,
			int		*sb_modified);
int	get_sb(xfs_sb_t			*sbp,
		xfs_off_t			off,
		int			size,
		xfs_agnumber_t		agno);
int retain_primary_sb(struct xfs_mount *mp);
void	write_primary_sb(xfs_sb_t	*sbp,
			int		size);

int	find_secondary_sb(xfs_sb_t	*sb);

struct fs_geometry;
void	get_sb_geometry(struct fs_geometry	*geo,
			xfs_sb_t	*sbp);

char	*alloc_ag_buf(int size);

void	print_inode_list(xfs_agnumber_t i);
char	*err_string(int err_code);

void	thread_init(void);

void	phase1(struct xfs_mount *);
void	phase2(struct xfs_mount *, int);
void	phase3(struct xfs_mount *, int);
void	phase4(struct xfs_mount *);
void	check_rtmetadata(struct xfs_mount *mp);
void	phase5(struct xfs_mount *);
void	phase6(struct xfs_mount *);
void	phase7(struct xfs_mount *, int);

int	verify_set_agheader(struct xfs_mount *, struct xfs_buf *,
		struct xfs_sb *, struct xfs_agf *, struct xfs_agi *,
		xfs_agnumber_t);
