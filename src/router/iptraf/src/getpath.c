/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

// TODO: full rewrite

#include "iptraf-ng-compat.h"

#include "dirs.h"

char *get_path(int dirtype, char *file)
{
	static char path[PATH_MAX];
	char *ptr = NULL;
	char *dir, *env = NULL;

	switch (dirtype) {
	case T_WORKDIR:
		dir = WORKDIR;
		env = WORKDIR_ENV;
		break;
	case T_LOGDIR:
		dir = LOGDIR;
		env = LOGDIR_ENV;
		break;
	case T_LOCKDIR:
		dir = LOCKDIR;
		break;
	default:
		return file;
	}

	if ((dirtype != T_LOCKDIR) && (ptr = getenv(env)) != NULL)
		dir = ptr;

	if (dir == NULL || *dir == '\0')
		return file;

	snprintf(path, PATH_MAX - 1, "%s/%s", dir, file);

	return path;
}
