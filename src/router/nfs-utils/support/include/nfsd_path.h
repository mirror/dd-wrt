/*
 * Copyright (C) 2019 Trond Myklebust <trond.myklebust@hammerspace.com>
 */
#ifndef NFSD_PATH_H
#define NFSD_PATH_H

void 		nfsd_path_init(void);

const char *	nfsd_path_nfsd_rootdir(void);
char *		nfsd_path_strip_root(char *pathname);
char *		nfsd_path_prepend_dir(const char *dir, const char *pathname);

int 		nfsd_path_stat(const char *pathname, struct stat *statbuf);
int 		nfsd_path_lstat(const char *pathname, struct stat *statbuf);

char *		nfsd_realpath(const char *path, char *resolved_path);

ssize_t		nfsd_path_read(int fd, char *buf, size_t len);
ssize_t		nfsd_path_write(int fd, const char *buf, size_t len);

#endif
