/*
 * misc.h	All that didn't fit elsewhere.
 *
 * Copyright (C) 1995 Olaf Kirch <okir@monad.swb.de>
 */

#ifndef MISC_H
#define MISC_H

/*
 * Generate random key, returning the length of the result. Currently,
 * weakrandomkey generates a maximum of 20 bytes are generated, but this
 * may change with future implementations.
 */
int	randomkey(unsigned char *keyout, int len);
int	weakrandomkey(unsigned char *keyout, int len);

char *generic_make_pathname(const char *, const char *);
_Bool generic_setup_basedir(const char *, const char *, char *, const size_t);

struct stat;

extern int check_is_mountpoint(const char *path,
		int (mystat)(const char *, struct stat *));
#define is_mountpoint(path) \
	check_is_mountpoint(path, NULL)

/* size of the file pointer buffers for rpc procfs files */
#define RPC_CHAN_BUF_SIZE 32768

#endif /* MISC_H */
