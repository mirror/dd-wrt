/*
 * Copyright (c) 2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xfs/project.h>

#define PROJID		"/etc/projid"
#define PROJECT_PATHS	"/etc/projects"
char *projid_file;
char *projects_file;

static FILE *projects;
static fs_project_t p;
static char projects_buffer[512];

static FILE *project_paths;
static fs_project_path_t pp;
static char project_paths_buffer[1024];

void
setprfiles(void)
{
	if (!projid_file)
		projid_file = PROJID;
	if (!projects_file)
		projects_file = PROJECT_PATHS;
}

void
setprent(void)
{
	setprfiles();
	projects = fopen(projid_file, "r");
}

void
setprpathent(void)
{
	setprfiles();
	project_paths = fopen(projects_file, "r");
}

void
endprent(void)
{
	if (projects)
		fclose(projects);
	projects = NULL;
}

void
endprpathent(void)
{
	if (project_paths)
		fclose(project_paths);
	project_paths = NULL;
}

fs_project_t *
getprent(void)
{
	char	*idstart, *idend;
	size_t	size = sizeof(projects_buffer) - 1;

	if (!projects)
		return NULL;
	for (;;) {
		if (!fgets(projects_buffer, size, projects))
			break;
		/*
		 * /etc/projid file format -- "name:id\n", ignore "^#..."
		 */
		if (projects_buffer[0] == '#')
			continue;
		idstart = strchr(projects_buffer, ':');
		if (!idstart)
			continue;
		if ((idstart + 1) - projects_buffer >= size)
			continue;
		idend = strchr(idstart+1, ':');
		if (idend)
			*idend = '\0';
		*idstart = '\0';
		p.pr_prid = atoi(idstart+1);
		p.pr_name = &projects_buffer[0];
		return &p;
	}

	return NULL;
}

fs_project_t *
getprnam(
	char		*name)
{
	fs_project_t	*p = NULL;

	setprent();
	while ((p = getprent()) != NULL)
		if (strcmp(p->pr_name, name) == 0)
			break;
	endprent();
	return p;
}

fs_project_t *
getprprid(
	prid_t		prid)
{
	fs_project_t	*p = NULL;

	setprent();
	while ((p = getprent()) != NULL)
		if (p->pr_prid == prid)
			break;
	endprent();
	return p;
}

fs_project_path_t *
getprpathent(void)
{
	char		*nmstart, *nmend;
	size_t		size = sizeof(project_paths_buffer) - 1;

	if (!project_paths)
		return NULL;
	for (;;) {
		if (!fgets(project_paths_buffer, size, project_paths))
			break;
		/*
		 * /etc/projects format -- "id:pathname\n", ignore "^#..."
		 */
		if (project_paths_buffer[0] == '#')
			continue;
		nmstart = strchr(project_paths_buffer, ':');
		if (!nmstart)
			continue;
		if ((nmstart + 1) - project_paths_buffer >= size)
			continue;
		nmend = strchr(nmstart + 1, '\n');
		if (nmend)
			*nmend = '\0';
		*nmstart = '\0';
		pp.pp_pathname = nmstart + 1;
		pp.pp_prid = atoi(&project_paths_buffer[0]);
		return &pp;
	}

	return NULL;
}


int
getprojid(
	const char	*name,
	int		fd,
	prid_t		*projid)
{
	struct fsxattr	fsx;

	if (xfsctl(name, fd, XFS_IOC_FSGETXATTR, &fsx)) {
		perror("XFS_IOC_FSGETXATTR");
		return -1;
	}
	*projid = fsx.fsx_projid;
	return 0;
}

int
setprojid(
	const char	*name,
	int		fd,
	prid_t		projid)
{
	struct fsxattr	fsx;
	int		error;

	if ((error = xfsctl(name, fd, XFS_IOC_FSGETXATTR, &fsx)) == 0) {
		fsx.fsx_projid = projid;
		error = xfsctl(name, fd, XFS_IOC_FSSETXATTR, &fsx);
	}
	return error;
}
