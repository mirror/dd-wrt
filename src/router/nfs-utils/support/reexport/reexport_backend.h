#ifndef REEXPORT_BACKEND_H
#define REEXPORT_BACKEND_H

extern struct reexpdb_backend_plugin sqlite_plug_ops;

struct reexpdb_backend_plugin {
	/*
	 * Find or allocate a fsidnum for a given path.
	 *
	 * @path: Path to look for
	 * @fsidnum: Pointer to an uint32_t variable
	 * @may_create: If non-zero, a fsidnum will be allocated if none was found
	 *
	 * Returns true if either an fsidnum was found or successfully allocated,
	 * false otherwise.
	 * On success, the fsidnum will be stored into @fsidnum.
	 * Upon errors, false is returned and errors are logged.
	 */
	bool (*fsidnum_by_path)(char *path, uint32_t *fsidnum, int may_create, bool *found);

	/*
	 * Lookup path by a given fsidnum
	 *
	 * @fsidnum: fsidnum to look for
	 * @path: address of a char pointer
	 *
	 * Returns true if a path was found, false otherwise.
	 * Upon errors, false is returned and errors are logged.
	 * In case of success, the function returns the found path
	 * via @path, @path will point to a freshly allocated buffer
	 * which is free()'able.
	 */
	bool (*path_by_fsidnum)(uint32_t fsidnum, char **path, bool *found);

	/*
	 * Init database connection, can get called multiple times.
	 * Returns true on success, false otherwise.
	 */
	bool (*initdb)(void);

	/*
	 * Undoes initdb().
	 */
	void (*destroydb)(void);
};

#endif /* REEXPORT_BACKEND_H */
