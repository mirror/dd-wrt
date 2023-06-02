#ifndef REEXPORT_H
#define REEXPORT_H

enum {
	REEXP_NONE = 0,
	REEXP_AUTO_FSIDNUM,
	REEXP_PREDEFINED_FSIDNUM,
};

int reexpdb_init(void);
void reexpdb_destroy(void);
int reexpdb_fsidnum_by_path(char *path, uint32_t *fsidnum, int may_create);
int reexpdb_apply_reexport_settings(struct exportent *ep, char *flname, int flline);
void reexpdb_uncover_subvolume(uint32_t fsidnum);

#define FSID_SOCKET_NAME "fsid.sock"

#endif /* REEXPORT_H */
