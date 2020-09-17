// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */
#ifndef __LIBFROG_PROJECTS_H__
#define __LIBFROG_PROJECTS_H__

#include "platform_defs.h"
#include "xfs.h"

extern int setprojid(const char *__name, int __fd, prid_t __id);
extern int getprojid(const char *__name, int __fd, prid_t *__id);

typedef struct fs_project {
	prid_t		pr_prid;	/* project identifier */
	char		*pr_name;	/* project name */
} fs_project_t;

extern void setprent(void);
extern void endprent(void);
extern fs_project_t *getprent(void);
extern fs_project_t *getprnam(char *__name);
extern fs_project_t *getprprid(prid_t __id);

typedef struct fs_project_path {
	prid_t		pp_prid;	/* project identifier */
	char		*pp_pathname;	/* pathname to root of project tree */
} fs_project_path_t;

extern void setprpathent(void);
extern void endprpathent(void);
extern fs_project_path_t *getprpathent(void);

extern void setprfiles(void);
extern char *projid_file;
extern char *projects_file;

#endif	/* __LIBFROG_PROJECTS_H__ */
