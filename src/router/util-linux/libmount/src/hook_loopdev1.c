/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * This file is part of libmount from util-linux project.
 *
 * Copyright (C) 2011-2022 Karel Zak <kzak@redhat.com>
 *
 * libmount is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Please, see the comment in libmount/src/hooks.c to understand how hooks work.
 */
#include <blkid.h>
#include <stdbool.h>
#include <sys/types.h>
#include <pwd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <ctype.h>

#include "mountP.h"
#include "strutils.h"
#include "linux_version.h"

#include "sha512.h"
#include "rmd160.h"
#include "aes.h"

struct hook_data {
	int chmodVal;
};

#define LO_CRYPT_NONE   0
#define LO_CRYPT_XOR    1
#define LO_CRYPT_DES    2
#define LO_CRYPT_CRYPTOAPI 18

#define LOOP_SET_FD		0x4C00
#define LOOP_CLR_FD		0x4C01
#define LOOP_SET_STATUS		0x4C02
#define LOOP_GET_STATUS		0x4C03
#define LOOP_SET_STATUS64	0x4C04
#define LOOP_GET_STATUS64	0x4C05
#define LOOP_MULTI_KEY_SETUP 	0x4C4D
#define LOOP_MULTI_KEY_SETUP_V3	0x4C4E
#define LOOP_RECOMPUTE_DEV_SIZE 0x4C52

#define LO_NAME_SIZE    64
#define LO_KEY_SIZE     32

struct loop_info {
	int		lo_number;
#if LINUX_VERSION_CODE >= 0x20600
	__kernel_old_dev_t lo_device;
#else
	__kernel_dev_t	lo_device;
#endif
	unsigned long	lo_inode;
#if LINUX_VERSION_CODE >= 0x20600
	__kernel_old_dev_t lo_rdevice;
#else
	__kernel_dev_t	lo_rdevice;
#endif
	int		lo_offset;
	int		lo_encrypt_type;
	int		lo_encrypt_key_size;
	int		lo_flags;
	char		lo_name[LO_NAME_SIZE];
	unsigned char	lo_encrypt_key[LO_KEY_SIZE];
	unsigned long	lo_init[2];
	char		reserved[4];
};

struct loop_info64 {
	u_int64_t	lo_device; 		/* ioctl r/o */
	u_int64_t	lo_inode; 		/* ioctl r/o */
	u_int64_t	lo_rdevice; 		/* ioctl r/o */
	u_int64_t	lo_offset;		/* bytes */
	u_int64_t	lo_sizelimit;		/* bytes, 0 == max available */
	u_int32_t	lo_number;		/* ioctl r/o */
	u_int32_t	lo_encrypt_type;
	u_int32_t	lo_encrypt_key_size; 	/* ioctl w/o */
	u_int32_t	lo_flags;		/* ioctl r/o */
	unsigned char	lo_file_name[LO_NAME_SIZE];
	unsigned char	lo_crypt_name[LO_NAME_SIZE];
	unsigned char	lo_encrypt_key[LO_KEY_SIZE]; /* ioctl w/o */
	u_int64_t	lo_init[2];
};

#if !defined(LOOP_PASSWORD_MIN_LENGTH)
# define  LOOP_PASSWORD_MIN_LENGTH   20
#endif

typedef struct {
	char *multiKeyPass[66];
	int multiKeyMode;
	char *loopFileName;
	char *pass_cb_string;
	char *extraPtrToFree;

	char *loopDevName;
	char *loopOffsetBytes;
	char *loopSizeBytes;
	char *loopEncryptionType;
	char *passSeedString;
	char *passHashFuncName;
	char *passIterThousands;
	char *loInitValue;
	char *gpgKeyFile;
	char *gpgHomeDir;
	char *clearTextKeyFile;
} loDev_passInfo;

static int loDev_rd_wr_retry(int fd, char *buf, int cnt, int w)
{
	int x, y, z;

	x = 0;
	while(x < cnt) {
		y = cnt - x;
		if(w) {
			z = write(fd, buf + x, y);
		} else {
			z = read(fd, buf + x, y);
			if (!z) return x;
		}
		if(z < 0) {
			if ((errno == EAGAIN) || (errno == ENOMEM) || (errno == ERESTART) || (errno == EINTR)) {
				continue;
			}
			return x;
		}
		x += z;
	}
	return x;
}

static char *loDev_get_FD_pass(int fd)
{
	char *p = NULL, *n;
	int x = 0, y = 0;

	do {
		if(y >= (x - 1)) {
			x += 128;
			/* Must enforce some max limit here -- this code   */
			/* runs as part of mount, and mount is setuid root */
			/* and has used mlockall(MCL_CURRENT | MCL_FUTURE) */
			if(x > (4*1024)) {
				error_out:
				if(p) {
					memset(p, 0, y);
					free(p);
				}
				return NULL;
			}
			n = malloc(x);
			if(!n) goto error_out;
			if(p) {
				memcpy(n, p, y);
				memset(p, 0, y);
				free(p);
			}
			p = n;
		}
		if(loDev_rd_wr_retry(fd, p + y, 1, 0) != 1) break;
		if((p[y] == '\n') || !p[y]) break;
		y++;
	} while(1);
	if(p) p[y] = 0;
	return p;
}

static unsigned long long loDev_mystrtoull(char *s, int acceptAT)
{
	unsigned long long v = 0;
	int negative = 0;

	while ((*s == ' ') || (*s == '\t'))
		s++;
	if (acceptAT && (*s == '@')) {
		s++;
		negative = 1;
	}
	if (*s == '0') {
		s++;
		if ((*s == 'x') || (*s == 'X')) {
			s++;
			sscanf(s, "%llx", &v);
		} else {
			sscanf(s, "%llo", &v);
		}
	} else {
		sscanf(s, "%llu", &v);
	}
	return negative ? -v : v;
}

static void loDev_warnAboutBadKeyData(struct libmnt_context *cxt, int x)
{
	if((x > 1) && (x != 64) && (x != 65)) {
		DBG(CXT, ul_debugobj(cxt, "Warning: Unknown key data format - using it anyway"));
	}
}

static int loDev_are_these_files_same(const char *name1, const char *name2)
{
	struct stat statbuf1;
	struct stat statbuf2;

	if(!name1 || !*name1 || !name2 || !*name2) return 0;
	if(stat(name1, &statbuf1)) return 0;
	if(stat(name2, &statbuf2)) return 0;
	if(statbuf1.st_dev != statbuf2.st_dev) return 0;
	if(statbuf1.st_ino != statbuf2.st_ino) return 0;
	return 1;   /* are same */
}

static char *loDev_do_GPG_pipe(struct libmnt_context *cxt, loDev_passInfo *pi, char *pass)
{
	int     x, pfdi[2], pfdo[2], failed = 0;
	char    str[10], *a[16], *e[2], *h;
	pid_t   gpid;
	struct passwd *p;
	void    *oldSigPipeHandler;

	if((getuid() == 0) && pi->gpgHomeDir && pi->gpgHomeDir[0]) {
		h = pi->gpgHomeDir;
	} else {
		if(!(p = getpwuid(getuid()))) {
			DBG(CXT, ul_debugobj(cxt, "Error: Unable to detect home directory"));
			return NULL;
		}
		h = p->pw_dir;
	}
	if(!(e[0] = malloc(strlen(h) + 6))) {
		nomem1:
		DBG(CXT, ul_debugobj(cxt, "Error: Unable to allocate memory"));
		return NULL;
	}
	sprintf(e[0], "HOME=%s", h);
	e[1] = 0;

	if(pipe(&pfdi[0])) {
		nomem2:
		free(e[0]);
		goto nomem1;
	}
	if(pipe(&pfdo[0])) {
		close(pfdi[0]);
		close(pfdi[1]);
		goto nomem2;
	}

	/*
	 * When this code is run as part of mount, only root can set
	 * 'gpgKeyFile' and as such, only root can decide what file is opened
	 * below. However, since mount is usually setuid-root all non-root
	 * users can also open() the file too, but that file's contents are
	 * only piped to gpg. This readable-for-all is intended behaviour,
	 * and is very useful in situations where non-root users mount loop
	 * devices with their own gpg private key, and yet don't have access
	 * to the actual key used to encrypt loop device.
	 */
	if((x = open(pi->gpgKeyFile, O_RDONLY)) == -1) {
		DBG(CXT, ul_debugobj(cxt, "Error: unable to open gpg key file for reading"));
		nomem3:
		free(e[0]);
		close(pfdo[0]);
		close(pfdo[1]);
		close(pfdi[0]);
		close(pfdi[1]);
		return NULL;
	}

	/*
	 * If someone puts a gpg key file at beginning of device and
	 * puts the real file system at some offset into the device,
	 * this code extracts that gpg key file into a temp file so gpg
	 * won't end up reading whole device when decrypting the key file.
	 *
	 * Example of encrypted cdrom mount with 8192 bytes reserved for gpg key file:
	 * mount -t iso9660 /dev/cdrom /cdrom -o loop=/dev/loop0,encryption=AES128,gpgkey=/dev/cdrom,offset=8192
	 *                  ^^^^^^^^^^                                                    ^^^^^^^^^^        ^^^^
	 */
	if(pi->loopOffsetBytes && loDev_are_these_files_same(pi->loopFileName, pi->gpgKeyFile)) {
		FILE *f;
		char b[1024];
		long long cnt;
		int cnt2, cnt3;

		cnt = loDev_mystrtoull(pi->loopOffsetBytes, 1);
		if(cnt < 0) cnt = -cnt;
		if(cnt > (1024 * 1024)) cnt = 1024 * 1024; /* sanity check */
		f = tmpfile();
		if(!f) {
			DBG(CXT, ul_debugobj(cxt, "Error: unable to create temp file"));
			close(x);
			goto nomem3;
		}
		while(cnt > 0) {
			cnt2 = sizeof(b);
			if(cnt < cnt2) cnt2 = cnt;
			cnt3 = loDev_rd_wr_retry(x, b, cnt2, 0);
			if(cnt3 && (fwrite(b, cnt3, 1, f) != 1)) {
				tmpWrErr:
				DBG(CXT, ul_debugobj(cxt, "Error: unable to write to temp file"));
				fclose(f);
				close(x);
				goto nomem3;
			}
			if(cnt2 != cnt3) break;
			cnt -= cnt3;
		}
		if(fflush(f)) goto tmpWrErr;
		close(x);
		x = dup(fileno(f));
		fclose(f);
		lseek(x, 0L, SEEK_SET);
	}

	sprintf(str, "%d", pfdi[0]);
	if(!(gpid = fork())) {
		dup2(x, 0);
		dup2(pfdo[1], 1);
		close(x);
		close(pfdi[1]);
		close(pfdo[0]);
		close(pfdo[1]);
		if((x = open("/dev/null", O_WRONLY)) >= 0) {
			dup2(x, 2);
			close(x);
		}
		x = 0;
		a[x++] = "gpg";
		if(pi->gpgHomeDir && pi->gpgHomeDir[0]) {
			a[x++] = "--homedir";
			a[x++] = pi->gpgHomeDir;
		}
		a[x++] = "--no-options";
		a[x++] = "--quiet";
		a[x++] = "--batch";
		a[x++] = "--no-tty";
		a[x++] = "--passphrase-fd";
		a[x++] = str;
		a[x++] = "--decrypt";
		a[x] = 0;
		if(setgid(getgid())) exit(1);
		if(setuid(getuid())) exit(1);
		for(x = 3; x < 1024; x++) {
			if(x == pfdi[0]) continue;
			close(x);
		}
		execve("/bin/gpg", &a[0], &e[0]);
		execve("/usr/bin/gpg", &a[0], &e[0]);
		execve("/usr/local/bin/gpg", &a[0], &e[0]);
		exit(1);
	}
	free(e[0]);
	close(x);
	close(pfdi[0]);
	close(pfdo[1]);
	if(gpid == -1) {
		close(pfdi[1]);
		close(pfdo[0]);
		goto nomem1;
	}

	x = strlen(pass);

	/* ignore possible SIGPIPE signal while writing to gpg */
	oldSigPipeHandler = signal(SIGPIPE, SIG_IGN);
	loDev_rd_wr_retry(pfdi[1], pass, x, 1);
	loDev_rd_wr_retry(pfdi[1], "\n", 1, 1);
	if(oldSigPipeHandler != SIG_ERR) signal(SIGPIPE, oldSigPipeHandler);

	close(pfdi[1]);
	memset(pass, 0, x);
	x = 0;
	while(x < 66) {
		if(pi->multiKeyPass[x]) {
			free(pi->multiKeyPass[x]);
			pi->multiKeyPass[x] = NULL;
		}
		x++;
	}
	x = 0;
	while(x < 66) {
		pi->multiKeyPass[x] = loDev_get_FD_pass(pfdo[0]);
		if(!pi->multiKeyPass[x]) {
			/* mem alloc failed - abort */
			failed = 1;
			break;
		}
		if(strlen(pi->multiKeyPass[x]) < LOOP_PASSWORD_MIN_LENGTH) break;
		x++;
	}
	loDev_warnAboutBadKeyData(cxt, x);
	if(x >= 65)
		pi->multiKeyMode = 65;
	if(x == 64)
		pi->multiKeyMode = 64;
	close(pfdo[0]);
	waitpid(gpid, &x, 0);
	if(failed || !pi->multiKeyPass[0]) goto nomem1;
	return pi->multiKeyPass[0];
}

/* password returned by this function must not be free()ed directly, because it
   came from either cxt->pwd_get_cb() or is a duplicate of pi->multiKeyPass[0].
   cxt->pwd_release_cb() free()s the one that came from cxt->pwd_get_cb() and
   pi->multiKeyPass[0] gets free()d with rest of pi->multiKeyPass[] pointers.
   If this function actually malloc()s a pointer, a copy is at pi->extraPtrToFree */
static char *loDev_sGetPass(struct libmnt_context *cxt, loDev_passInfo *pi, int minLen, int warnLen)
{
	char *p, *s, *seed;
	int i, x;

	if(pi->clearTextKeyFile) {
		if((i = open(pi->clearTextKeyFile, O_RDONLY)) == -1) {
			DBG(CXT, ul_debugobj(cxt, "Error: unable to open cleartext key file for reading"));
			return NULL;
		}
		x = 0;
		while(x < 66) {
			pi->multiKeyPass[x] = loDev_get_FD_pass(i);
			if(!pi->multiKeyPass[x]) {
				close(i);
				goto nomem;
			}
			if(strlen(pi->multiKeyPass[x]) < LOOP_PASSWORD_MIN_LENGTH) break;
			x++;
		}
		close(i);
		loDev_warnAboutBadKeyData(cxt, x);
		if(x >= 65) {
			pi->multiKeyMode = 65;
			return pi->multiKeyPass[0];
		}
		if(x == 64) {
			pi->multiKeyMode = 64;
			return pi->multiKeyPass[0];
		}
		p = pi->multiKeyPass[0];
	} else {
		if(!cxt->pwd_get_cb)
			return NULL;
		DBG(CXT, ul_debugobj(cxt, "asking for pass"));
		p = pi->pass_cb_string = cxt->pwd_get_cb(cxt);
	}
	
	if(!p) goto nomem;
	if(pi->gpgKeyFile && pi->gpgKeyFile[0]) {
		p = loDev_do_GPG_pipe(cxt, pi, p);
		if(!p) return NULL;
		if(!p[0]) {
			DBG(CXT, ul_debugobj(cxt, "Error: gpg key file decryption failed"));
			return NULL;
		}
		if(pi->multiKeyMode) return p;
	}
	i = strlen(p);
	if(i < minLen) {
		DBG(CXT, ul_debugobj(cxt, "Error: Password is too short"));
		return NULL;
	}
	seed = pi->passSeedString;
	if(!seed) seed = "";
	s = pi->extraPtrToFree = malloc(i + strlen(seed) + 1);
	if(!s) {
		memset(p, 0, i);
		nomem:
		DBG(CXT, ul_debugobj(cxt, "Error: Unable to allocate memory"));
		return NULL;
	}
	strcpy(s, p);
	memset(p, 0, i);
	if(i < warnLen) {
		DBG(CXT, ul_debugobj(cxt, "WARNING - Please use longer password"));
	}
	strcat(s, seed);
	return(s);
}

/* this is for compatibility with historic loop-AES version */
static void loDev_unhashed1_key_setup(unsigned char *keyStr, int ile, unsigned char *keyBuf, int bufSize)
{
	register int    x, y, z, cnt = ile;
	unsigned char   *kp;

	memset(keyBuf, 0, bufSize);
	kp = keyStr;
	for(x = 0; x < (bufSize * 8); x += 6) {
		y = *kp++;
		if(--cnt <= 0) {
			kp = keyStr;
			cnt = ile;
		}
		if((y >= '0') && (y <= '9')) y -= '0';
		else if((y >= 'A') && (y <= 'Z')) y -= ('A' - 10);
		else if((y >= 'a') && (y <= 'z')) y -= ('a' - 36);
		else if((y == '.') || (y == '/')) y += (62 - '.');
		else y &= 63;
		z = x >> 3;
		if(z < bufSize) {
			keyBuf[z] |= y << (x & 7);
		}
		z++;
		if(z < bufSize) {
			keyBuf[z] |= y >> (8 - (x & 7));
		}
	}
}

/* this is for compatibility with mainline mount */
static void loDev_unhashed2_key_setup(unsigned char *keyStr, int ile __attribute__((__unused__)), unsigned char *keyBuf, int bufSize)
{
	memset(keyBuf, 0, bufSize);
	strncpy((char *)keyBuf, (char *)keyStr, bufSize - 1);
	keyBuf[bufSize - 1] = 0;
}

static void loDev_rmd160HashTwiceWithA(unsigned char *ib, int ile, unsigned char *ob, int ole)
{
	char tmpBuf[20 + 20];
	char pwdCopy[130];

	if(ole < 1) return;
	memset(ob, 0, ole);
	if(ole > 40) ole = 40;
	__loDev_rmd160_hash_buffer(&tmpBuf[0], (char *)ib, ile);
	pwdCopy[0] = 'A';
	if(ile > (int)sizeof(pwdCopy) - 1) ile = sizeof(pwdCopy) - 1;
	memcpy(pwdCopy + 1, ib, ile);
	__loDev_rmd160_hash_buffer(&tmpBuf[20], pwdCopy, ile + 1);
	memcpy(ob, tmpBuf, ole);
	memset(tmpBuf, 0, sizeof(tmpBuf));
	memset(pwdCopy, 0, sizeof(pwdCopy));
}

extern long long llseek(int, long long, int);

static long long loDev_xx_lseek(int fd, long long offset, int whence)
{
	if(sizeof(off_t) >= 8) {
		return lseek(fd, offset, whence);
	} else {
		return llseek(fd, offset, whence);
	}
}

static int loDev_create_random_keys(struct libmnt_context *cxt, char *partition, long long offset, long long sizelimit, int loopro, unsigned char *k)
{
	int x, y, fd;
	sha512_context s;
	unsigned char b[4096];

	if(loopro) {
		DBG(CXT, ul_debugobj(cxt, "Error: read-only device"));
		return 1;
	}

	/*
	 * Compute SHA-512 over first 40 KB of old fs data. SHA-512 hash
	 * output is then used as entropy for new fs encryption key.
	 */
	if((fd = open(partition, O_RDWR)) == -1) {
		seekFailed:
		DBG(CXT, ul_debugobj(cxt, "Error: unable to open/seek device"));
		return 1;
	}
	if(offset < 0) offset = -offset;
	if(loDev_xx_lseek(fd, offset, SEEK_SET) == -1) {
		close(fd);
		goto seekFailed;
	}
	__loDev_sha512_init(&s);
	for(x = 1; x <= 10; x++) {
		if((sizelimit > 0) && ((long long)(sizeof(b) * x) > sizelimit)) break;
		if(loDev_rd_wr_retry(fd, (char *) &b[0], sizeof(b), 0) != sizeof(b)) break;
		__loDev_sha512_write(&s, &b[0], sizeof(b));
	}
	__loDev_sha512_final(&s);

	/*
	 * Overwrite 40 KB of old fs data 20 times so that recovering
	 * SHA-512 output beyond this point is difficult and expensive.
	 */
	for(y = 0; y < 20; y++) {
		int z;
		struct {
			struct timeval tv;
			unsigned char h[64];
			int x,y,z;
		} j;
		if(loDev_xx_lseek(fd, offset, SEEK_SET) == -1) break;
		memcpy(&j.h[0], &s.sha_out[0], 64);
		gettimeofday(&j.tv, NULL);
		j.y = y;
		for(x = 1; x <= 10; x++) {
			j.x = x;
			for(z = 0; z < (int)sizeof(b); z += 64) {
				j.z = z;
				__loDev_sha512_hash_buffer((unsigned char *)&j, sizeof(j), &b[z], 64);
			}
			if((sizelimit > 0) && ((long long)(sizeof(b) * x) > sizelimit)) break;
			if(loDev_rd_wr_retry(fd, (char *) &b[0], sizeof(b), 1) != sizeof(b)) break;
		}
		memset(&j, 0, sizeof(j));
		if(fsync(fd)) break;
	}
	close(fd);

	/*
	 * Use all 512 bits of hash output
	 */
	memcpy(&b[0], &s.sha_out[0], 64);
	memset(&s, 0, sizeof(s));

	/*
	 * Read 32 bytes of random entropy from kernel's random
	 * number generator. This code may be executed early on startup
	 * scripts and amount of random entropy may be non-existent.
	 * SHA-512 of old fs data is used as workaround for missing
	 * entropy in kernel's random number generator.
	 */
	if((fd = open("/dev/urandom", O_RDONLY)) == -1) {
		DBG(CXT, ul_debugobj(cxt, "Error: unable to open /dev/urandom"));
		return 1;
	}
	loDev_rd_wr_retry(fd, (char *) &b[64], 32, 0);

	/* generate multi-key hashes */
	x = 0;
	while(x < 65) {
		loDev_rd_wr_retry(fd, (char *) &b[64+32], 16, 0);
		__loDev_sha512_hash_buffer(&b[0], 64+32+16, k, 32);
		k += 32;
		x++;
	}

	close(fd);
	memset(&b[0], 0, sizeof(b));
	return 0;
}

static int loDev_fork_mkfs_command(struct libmnt_context *cxt, char *device, char *fstype)
{
	int x, y = 0;
	char *a[10], *e[2];

	sync();
	if(!(x = fork())) {
		if((x = open("/dev/null", O_WRONLY)) >= 0) {
			dup2(x, 0);
			dup2(x, 1);
			dup2(x, 2);
			close(x);
		}
		x = 0;
		a[x++] = "mkfs";
		a[x++] = "-t";
		a[x++] = fstype;
		/* mkfs.reiserfs and mkfs.xfs need -f option */
		if(!strcmp(fstype, "reiserfs") || !strcmp(fstype, "xfs")) {
			a[x++] = "-f";
		}
		a[x++] = device;
		a[x] = 0;
		e[0] = "PATH=/sbin:/usr/sbin";
		e[1] = 0;
		if(setgid(getgid())) exit(1);
		if(setuid(getuid())) exit(1);
		for(x = 3; x < 1024; x++) {
			close(x);
		}
		execve("/sbin/mkfs", &a[0], &e[0]);
		exit(1);
	}
	if(x == -1) {
		DBG(CXT, ul_debugobj(cxt, "Error: fork failed"));
		return 1;
	}
	waitpid(x, &y, 0);
	sync();
	if(!WIFEXITED(y) || (WEXITSTATUS(y) != 0)) {
		DBG(CXT, ul_debugobj(cxt, "Error: encrypted file system mkfs failed"));
		return 1;
	}
	return 0;
}

static void loDev_convert_info_to_info64(struct loop_info *info, struct loop_info64 *info64)
{
	memset(info64, 0, sizeof(*info64));
	info64->lo_number = info->lo_number;
	info64->lo_device = info->lo_device;
	info64->lo_inode = info->lo_inode;
	info64->lo_rdevice = info->lo_rdevice;
	info64->lo_offset = info->lo_offset;
	info64->lo_encrypt_type = info->lo_encrypt_type;
	info64->lo_encrypt_key_size = info->lo_encrypt_key_size;
	info64->lo_flags = info->lo_flags;
	info64->lo_init[0] = info->lo_init[0];
	info64->lo_init[1] = info->lo_init[1];
	info64->lo_sizelimit = 0;
	if (info->lo_encrypt_type == 18) /* LO_CRYPT_CRYPTOAPI */
		memcpy(info64->lo_crypt_name, info->lo_name, sizeof(info64->lo_crypt_name));
	else
		memcpy(info64->lo_file_name, info->lo_name, sizeof(info64->lo_file_name));
	memcpy(info64->lo_encrypt_key, info->lo_encrypt_key, sizeof(info64->lo_encrypt_key));
}

static int loDev_convert_info64_to_info(struct loop_info64 *info64, struct loop_info *info)
{
	memset(info, 0, sizeof(*info));
	info->lo_number = info64->lo_number;
	info->lo_device = info64->lo_device;
	info->lo_inode = info64->lo_inode;
	info->lo_rdevice = info64->lo_rdevice;
	info->lo_offset = info64->lo_offset;
	info->lo_encrypt_type = info64->lo_encrypt_type;
	info->lo_encrypt_key_size = info64->lo_encrypt_key_size;
	info->lo_flags = info64->lo_flags;
	info->lo_init[0] = info64->lo_init[0];
	info->lo_init[1] = info64->lo_init[1];
	if (info->lo_encrypt_type == 18) /* LO_CRYPT_CRYPTOAPI */
		memcpy(info->lo_name, info64->lo_crypt_name, sizeof(info->lo_name));
	else
		memcpy(info->lo_name, info64->lo_file_name, sizeof(info->lo_name));
	memcpy(info->lo_encrypt_key, info64->lo_encrypt_key, sizeof(info->lo_encrypt_key));

	/* error in case values were truncated */
	if (info->lo_device != info64->lo_device ||
	    info->lo_rdevice != info64->lo_rdevice ||
	    info->lo_inode != info64->lo_inode ||
	    (u_int64_t) info->lo_offset != info64->lo_offset ||
	    info64->lo_sizelimit) {
		return -1;
	}
	return 0;
}

static int loDev_set_status64_ioctl(int fd, struct loop_info64 *info64)
{
	struct loop_info info;
	struct loop_info64 tmp;
	int r;

	/*
	 * This ugly work around is needed because some
	 * Red Hat kernels are using same ioctl code:
	 *  	#define LOOP_CHANGE_FD 0x4C04
	 * vs.
	 *	#define LOOP_SET_STATUS64 0x4C04
	 * that is used by modern loop driver.
	 *
	 * Attempt to detect presense of LOOP_GET_STATUS64
	 * ioctl before issuing LOOP_SET_STATUS64 ioctl.
	 * Red Hat kernels with above LOOP_CHANGE_FD damage
	 * should return -1 and set errno to EINVAL.
	 */
	r = ioctl(fd, LOOP_GET_STATUS64, &tmp);
	memset(&tmp, 0, sizeof(tmp));
	if ((r == 0) || (errno != EINVAL)) {
		r = ioctl(fd, LOOP_SET_STATUS64, info64);
		if (!r)
			return 0;
	}
	r = loDev_convert_info64_to_info(info64, &info);
	if (!r)
		r = ioctl(fd, LOOP_SET_STATUS, &info);

	/* don't leave copies of encryption key on stack */
	memset(&info, 0, sizeof(info));
	return r;
}

static int loDev_get_status64_ioctl(int fd, struct loop_info64 *info64)
{
	struct loop_info info;
	int r;

	memset(info64, 0, sizeof(*info64));
	r = ioctl(fd, LOOP_GET_STATUS64, info64);
	if (!r)
		return 0;
	r = ioctl(fd, LOOP_GET_STATUS, &info);
	if (!r)
		loDev_convert_info_to_info64(&info, info64);

	/* don't leave copies of encryption key on stack */
	memset(&info, 0, sizeof(info));
	return r;
}

/* returns: 1=unused 0=busy */
static int loDev_is_unused_loop_device(int fd)
{
	struct loop_info64 info64;
	struct loop_info info;
	int r;

	r = ioctl(fd, LOOP_GET_STATUS64, &info64);
	memset(&info64, 0, sizeof(info64));
	if (!r)
		return 0;
	if (errno == ENXIO)
		return 1;

	r = ioctl(fd, LOOP_GET_STATUS, &info);
	memset(&info, 0, sizeof(info));
	if (!r)
		return 0;
	if (errno == ENXIO)
		return 1;
	if (errno == EOVERFLOW)
		return 0;
	return 1;
}

struct loDev_crypt_type_struct {
	short int id;
	unsigned char flags; /* bit0 = show keybits, bit1 = add '-' before keybits */
	unsigned char keyBytes;
	char *name;
};

static struct loDev_crypt_type_struct loDev_crypt_type_tbl[] = {
	{  0, 0,  0, "no" },
	{  0, 0,  0, "none" },
	{  1, 0,  0, "xor" },
	{  3, 1, 16, "twofish" },
	{  4, 1, 16, "blowfish" },
	{  7, 1, 16, "serpent" },
	{  8, 1, 16, "mars" },
	{ 11, 3, 16, "rc6" },
	{ 12, 0, 21, "tripleDES" },
	{ 12, 0, 24, "3des" },
	{ 12, 0, 24, "des3_ede" },
	{ 16, 1, 16, "AES" },
	{ -1, 0,  0, NULL }
};

static char *loDev_getApiName(char *e, int *len)
{
	int x, y, z = 1, q = -1;
	unsigned char *s;

	*len = y = 0;
	s = (unsigned char *)strdup(e);
	if(!s)
		return NULL;
	x = strlen((char *)s);
	while(x > 0) {
		x--;
		if(!isdigit(s[x]))
			break;
		y += (s[x] - '0') * z;
		z *= 10;
		q = x;
	}
	while(x >= 0) {
		s[x] = tolower(s[x]);
		if(s[x] == '-')
			s[x] = 0;
		x--;
	}
	if(y >= 40) {
		if(q >= 0)
			s[q] = 0;
		*len = y;
	}
	return((char *)s);
}

static int loDev_crypt_type_fn(const char *name, u_int32_t *kbyp, char **apiName)
{
	int i, k;
	char *s;

	*apiName = s = loDev_getApiName((char *)name, &k);
	if(!s) s = "";
	if(k < 0)
		k = 0;
	if(k > 256)
		k = 256;
	for (i = 0; loDev_crypt_type_tbl[i].id != -1; i++) {
		if (!strcasecmp (s , loDev_crypt_type_tbl[i].name)) {
			*kbyp = k ? k >> 3 : loDev_crypt_type_tbl[i].keyBytes;
			return loDev_crypt_type_tbl[i].id;
		}
	}
	*kbyp = 16; /* 128 bits */
	return 18; /* LO_CRYPT_CRYPTOAPI */
}

static int loDev_try_cryptoapi_interface(int fd, struct loop_info64 *loopinfo, char *apiName)
{
	if(!apiName) apiName = "";
	snprintf((char *)loopinfo->lo_crypt_name, sizeof(loopinfo->lo_crypt_name), "%s-cbc", apiName);
	loopinfo->lo_crypt_name[LO_NAME_SIZE - 1] = 0;
	loopinfo->lo_encrypt_type = 18; /* LO_CRYPT_CRYPTOAPI */
	return(loDev_set_status64_ioctl(fd, loopinfo));
}

static int loDev_is_loop_device(const char *device)
{
	struct stat statbuf;

	return (stat(device, &statbuf) == 0 &&
		S_ISBLK(statbuf.st_mode) &&
		major(statbuf.st_rdev) == 7);
}

int __loDev_is_loop_active_same_back(char *, char *, char *, char *);
int __loDev_is_loop_active_same_back(char *dev, char *backdev, char *offsetStr, char *sizelimitStr)
{
	int fd;
	int ret = 0;
	struct stat statbuf;
	struct loop_info64 loopinfo;
	uint64_t offset = 0, sizelimit = 0;

	if(offsetStr)
		offset = loDev_mystrtoull(offsetStr, 1);
	if(sizelimitStr)
		sizelimit = loDev_mystrtoull(sizelimitStr, 0);

	if (stat (dev, &statbuf) == 0 && S_ISBLK(statbuf.st_mode) && major(statbuf.st_rdev) == 7) {
		if(stat (backdev, &statbuf) != 0)
			return 0;
		fd = open (dev, O_RDONLY);
		if (fd < 0)
			return 0;
		if ((loDev_get_status64_ioctl(fd, &loopinfo) == 0)
		    && (loopinfo.lo_offset == offset)
		    && (loopinfo.lo_sizelimit == sizelimit)
		    && (statbuf.st_dev == loopinfo.lo_device)
		    && (statbuf.st_ino == loopinfo.lo_inode))
			ret = 1; /* backing device matches */
		memset(&loopinfo, 0, sizeof(loopinfo));
		close(fd);
	}
	return ret;
}

#define SIZE(a) ((int)(sizeof(a)/sizeof(a[0])))

static char * loDev_find_unused_loop_device(void)
{
	/* Just creating a device, say in /tmp, is probably a bad idea -
	   people might have problems with backup or so.
	   So, we just try /dev/loop[0-7]. */
	char dev[20];
	char *loop_formats[] = { "/dev/loop%d", "/dev/loop/%d" };
	int i, j, fd, somedev = 0, someloop = 0;
	struct stat statbuf;

	for (j = 0; j < SIZE(loop_formats); j++) {
	    for(i = 0; i < 256; i++) {
		sprintf(dev, loop_formats[j], i);
		if (stat (dev, &statbuf) == 0 && S_ISBLK(statbuf.st_mode)) {
			somedev++;
			fd = open (dev, O_RDONLY);
			if (fd >= 0) {
				if (loDev_is_unused_loop_device(fd) == 0)
					someloop++;		/* in use */
				else if (errno == ENXIO) {
					close (fd);
					return strdup(dev);/* probably free */
				}
				close (fd);
			}
			continue;/* continue trying as long as devices exist */
		}
		break;
	    }
	}
	return 0;
}

/* de-initiallize this module */
static int hookset_deinit(struct libmnt_context *cxt, const struct libmnt_hookset *hs)
{
	void *data;

	DBG(HOOK, ul_debugobj(hs, "deinit '%s'", hs->name));

	/* remove all our hooks */
	while (mnt_context_remove_hook(cxt, hs, 0, &data) == 0) {
		free(data);
		data = NULL;
	}

	return 0;
}

static inline struct hook_data *new_hook_data(void)
{
	struct hook_data *hd = calloc(1, sizeof(*hd));

	if (!hd)
		return NULL;

	hd->chmodVal = -1;
	return hd;
}

/* Check, if there already exists a mounted loop device on the mountpoint node
 * with the same parameters.
 */
static int is_mounted_same_loopfile(struct libmnt_context *cxt,
				    const char *target,
				    char *backing_file,
				    char *offsetStr, char *sizelimitStr)
{
	struct libmnt_table *tb;
	struct libmnt_iter itr;
	struct libmnt_fs *fs;
	struct libmnt_cache *cache;
	unsigned long flags = 0;

	assert(cxt);
	assert(cxt->fs);
	assert((cxt->flags & MNT_FL_MOUNTFLAGS_MERGED));

	if (!target || !backing_file || mnt_context_get_mtab(cxt, &tb))
		return 0;

	DBG(CXT, ul_debugobj(cxt, "checking if %s mounted on %s",
				backing_file, target));

	if(mnt_context_get_user_mflags(cxt, &flags))
		return 0;

	cache = mnt_context_get_cache(cxt);
	mnt_reset_iter(&itr, MNT_ITER_BACKWARD);

	/* Search for mountpoint node in mtab, procceed if any of these has the
	 * loop option set or the device is a loop device
	 */
	while (mnt_table_next_fs(tb, &itr, &fs) == 0) {
		const char *src = mnt_fs_get_source(fs);
		const char *opts = mnt_fs_get_user_options(fs);
		char *val;
		size_t len;
		int res = 0;

		if (!src || !mnt_fs_match_target(fs, target, cache))
			continue;

		if (strncmp(src, "/dev/loop", 9) == 0) {
			res = __loDev_is_loop_active_same_back((char *) src, backing_file, offsetStr, sizelimitStr);

		} else if (opts && (flags & MNT_MS_LOOP) &&
		    mnt_optstr_get_option(opts, "loop", &val, &len) == 0 && val) {

			val = strndup(val, len);
			if(val) {
				res = __loDev_is_loop_active_same_back((char *) val, backing_file, offsetStr, sizelimitStr);
				free(val);
			}
		}

		if (res) {
			DBG(CXT, ul_debugobj(cxt, "%s already mounted", backing_file));
			return 1;
		}
	}

	return 0;
}

static int setup_loopdev(struct libmnt_context *cxt, struct hook_data *hd)
{
	int loop_dev_fd = -1, backing_fi_fd = -1;
	loDev_passInfo pi;
	struct loop_info64 loopinfo;
        int i;
        char *pass = NULL, *apiName = NULL;
        void (*hashFunc)(unsigned char *, int, unsigned char *, int);
        unsigned char multiKeyBits[65][32];
        int minPassLen = LOOP_PASSWORD_MIN_LENGTH;
        int run_mkfs_command = 0;
	int mode = O_RDWR;
	int fixedLoopName = 0;
	unsigned int chmodVal = 0;
	const char *optstr;
	char *val = NULL;
	size_t len = 0;
	int rc = 0, myErrno = 0;
	uint64_t offset = 0, sizelimit = 0;
	unsigned long mflags = 0;

	memset(&pi, 0, sizeof(pi));
	assert(cxt);
	assert(cxt->fs);
	assert((cxt->flags & MNT_FL_MOUNTFLAGS_MERGED));
	hd->chmodVal = -1;

	pi.loopFileName = (char *) mnt_fs_get_srcpath(cxt->fs);
	if (!pi.loopFileName)
		return -EINVAL;

	DBG(CXT, ul_debugobj(cxt, "trying to setup loopdev for %s", pi.loopFileName));

	mnt_context_get_mflags(cxt, &mflags);
	if (mflags & MS_RDONLY) {
		DBG(CXT, ul_debugobj(cxt, "enabling READ-ONLY flag"));
		mode = O_RDONLY;
	}

	optstr = mnt_fs_get_options(cxt->fs);

#define MM(a,b)	if(mnt_optstr_get_option(optstr, a, &val, &len) == 0 && val && len) { 	\
			if(!(b = strndup(val, len))) rc = -ENOMEM;			\
		}

	MM("loop",         pi.loopDevName);
	MM("offset",       pi.loopOffsetBytes);
	MM("sizelimit",    pi.loopSizeBytes);
	MM("encryption",   pi.loopEncryptionType);
	MM("pseed",        pi.passSeedString);
	MM("phash",        pi.passHashFuncName);
	MM("itercountk",   pi.passIterThousands);
	MM("loinit",       pi.loInitValue);
	MM("gpgkey",       pi.gpgKeyFile);
	MM("gpghome",      pi.gpgHomeDir);
	MM("cleartextkey", pi.clearTextKeyFile);

#undef MM

	if(pi.loopOffsetBytes)
		offset = loDev_mystrtoull(pi.loopOffsetBytes, 1);
	if(pi.loopSizeBytes)
		sizelimit = loDev_mystrtoull(pi.loopSizeBytes, 0);

	if (rc == 0 && is_mounted_same_loopfile(cxt, mnt_context_get_target(cxt), pi.loopFileName, pi.loopOffsetBytes, pi.loopSizeBytes))
		rc = -EBUSY;
	if (rc)
		goto clean_up_out;
	if(pi.loopDevName) {
		fixedLoopName = 1;
		if(__loDev_is_loop_active_same_back(pi.loopDevName, pi.loopFileName, pi.loopOffsetBytes, pi.loopSizeBytes)) {
			/* loop device appears to be already set up to same backing file, so skip most of loop setup */
			/* this is useful for encrypted losetup -> fsck -> mount, where passphrase needs to be typed only once */
			rc = mnt_fs_set_source(cxt->fs, pi.loopDevName);
			if(rc) goto clean_up_out;
			goto skip_setup_ok_out;
		}
	} else if(cxt->fs && mnt_optstr_get_option(optstr, "loop", &val, &len) != 0) {
		/* Looks like no "loop" option at all. Add one so it gets cleaned up by umount */
		mnt_optstr_append_option(&cxt->fs->user_optstr, "loop", NULL);
	}
	if((backing_fi_fd = open(pi.loopFileName, mode)) < 0) {
		DBG(CXT, ul_debugobj(cxt, "can't open backing device/file"));
		myErrno = ENODEV;
		rc = -MNT_ERR_LOOPDEV;
		goto clean_up_out;
	}

try_again_another_loopdev:
	sync();
	if(!pi.loopDevName) {
		pi.loopDevName = loDev_find_unused_loop_device();
		if(!pi.loopDevName) {
			DBG(CXT, ul_debugobj(cxt, "can't find free loop device"));
			myErrno = ENODEV;
			rc = -MNT_ERR_LOOPDEV;
			goto clean_up_out;
		}
	}
	if((loop_dev_fd = open(pi.loopDevName, mode)) < 0) {
		DBG(CXT, ul_debugobj(cxt, "can't open loop device"));
		myErrno = ENODEV;
		rc = -MNT_ERR_LOOPDEV;
		goto clean_up_out;
	}
	if(ioctl(loop_dev_fd, LOOP_SET_FD, backing_fi_fd) < 0) {
		if(errno == EBUSY && !fixedLoopName) {
			close(loop_dev_fd);
			loop_dev_fd = -1;
			free(pi.loopDevName);
			pi.loopDevName = NULL;
			DBG(CXT, ul_debugobj(cxt, "loopdev stolen...trying again"));
			goto try_again_another_loopdev;
		}
		close(loop_dev_fd);
		loop_dev_fd = -1;
		DBG(CXT, ul_debugobj(cxt, "loop set-fd ioctl failed"));
		myErrno = EBUSY;
		rc = -MNT_ERR_LOOPDEV;
		goto clean_up_out;
	}

	memset(&loopinfo, 0, sizeof (loopinfo));
	strncpy((char *)loopinfo.lo_file_name, pi.loopFileName, LO_NAME_SIZE - 1);
	loopinfo.lo_file_name[LO_NAME_SIZE - 1] = 0;
	if(pi.loopEncryptionType)
		loopinfo.lo_encrypt_type = loDev_crypt_type_fn(pi.loopEncryptionType, &loopinfo.lo_encrypt_key_size, &apiName);
	loopinfo.lo_offset = offset;
	loopinfo.lo_sizelimit = sizelimit;

	switch(loopinfo.lo_encrypt_type) {
	case 0:	  /* LO_CRYPT_NONE */
		loopinfo.lo_encrypt_key_size = 0;
		break;
	case 1:   /* LO_CRYPT_XOR */
		pass = loDev_sGetPass(cxt, &pi, 1, 0);
		if(!pass) {
			myErrno = ENOKEY;
			goto loop_clr_fd_out;
		}
		strncpy((char *)loopinfo.lo_encrypt_key, pass, LO_KEY_SIZE - 1);
		loopinfo.lo_encrypt_key[LO_KEY_SIZE - 1] = 0;
		loopinfo.lo_encrypt_key_size = strlen((char*)loopinfo.lo_encrypt_key);
		break;
	case 3:   /* LO_CRYPT_FISH2 */
	case 4:   /* LO_CRYPT_BLOW */
	case 7:   /* LO_CRYPT_SERPENT */
	case 8:   /* LO_CRYPT_MARS */
	case 11:  /* LO_CRYPT_RC6 */
	case 12:  /* LO_CRYPT_DES_EDE3 */
	case 16:  /* LO_CRYPT_AES */
	case 18:  /* LO_CRYPT_CRYPTOAPI */
		/* set default hash function */
		hashFunc = __loDev_sha256_hash_buffer;
		if(loopinfo.lo_encrypt_key_size == 24) hashFunc = __loDev_sha384_hash_buffer;
		if(loopinfo.lo_encrypt_key_size == 32) hashFunc = __loDev_sha512_hash_buffer;
		/* possibly override default hash function */
		if(pi.passHashFuncName) {
			if(!strcasecmp(pi.passHashFuncName, "sha256")) {
				hashFunc = __loDev_sha256_hash_buffer;
			} else if(!strcasecmp(pi.passHashFuncName, "sha384")) {
				hashFunc = __loDev_sha384_hash_buffer;
			} else if(!strcasecmp(pi.passHashFuncName, "sha512")) {
				hashFunc = __loDev_sha512_hash_buffer;
			} else if(!strcasecmp(pi.passHashFuncName, "rmd160")) {
				hashFunc = loDev_rmd160HashTwiceWithA;
				minPassLen = 1;
			} else if(!strcasecmp(pi.passHashFuncName, "unhashed1")) {
				hashFunc = loDev_unhashed1_key_setup;
			} else if(!strcasecmp(pi.passHashFuncName, "unhashed2")) {
				hashFunc = loDev_unhashed2_key_setup;
				minPassLen = 1;
			} else if(!strncasecmp(pi.passHashFuncName, "random", 6) && ((pi.passHashFuncName[6] == 0) || (pi.passHashFuncName[6] == '/'))) {
				/* random hash type sets up 65 random keys */
				/* WARNING! DO NOT USE RANDOM HASH TYPE ON PARTITION WITH EXISTING */
				/* IMPORTANT DATA ON IT. RANDOM HASH TYPE WILL DESTROY YOUR DATA.  */
				if(loDev_create_random_keys(cxt, pi.loopFileName, loopinfo.lo_offset, loopinfo.lo_sizelimit, mflags & MS_RDONLY, &multiKeyBits[0][0])) {
					myErrno = ENOKEY;
					goto loop_clr_fd_out;
				}
				memcpy(&loopinfo.lo_encrypt_key[0], &multiKeyBits[0][0], sizeof(loopinfo.lo_encrypt_key));
				run_mkfs_command = pi.multiKeyMode = 1000;
				break; /* out of switch(loopinfo.lo_encrypt_type) */
			}
		}
		pass = loDev_sGetPass(cxt, &pi, minPassLen, LOOP_PASSWORD_MIN_LENGTH);
		if(!pass) {
			myErrno = ENOKEY;
			goto loop_clr_fd_out;
		}
		i = strlen(pass);
		if(hashFunc == loDev_unhashed1_key_setup) {
			/* this is for compatibility with historic loop-AES version */
			loopinfo.lo_encrypt_key_size = 16;             /* 128 bits */
			if(i >= 32) loopinfo.lo_encrypt_key_size = 24; /* 192 bits */
			if(i >= 43) loopinfo.lo_encrypt_key_size = 32; /* 256 bits */
		}
		(*hashFunc)((unsigned char *)pass, i, &loopinfo.lo_encrypt_key[0], sizeof(loopinfo.lo_encrypt_key));
		if(pi.multiKeyMode) {
			int r = 0, t;
			while(r < pi.multiKeyMode) {
				t = strlen(pi.multiKeyPass[r]);
				(*hashFunc)((unsigned char *)pi.multiKeyPass[r], t, &multiKeyBits[r][0], 32);
				memset(pi.multiKeyPass[r], 0, t);
				/*
				 * MultiKeyMode uses md5 IV. One key mode uses sector IV. Sector IV
				 * and md5 IV v2 and v3 are all computed differently. This first key
				 * byte XOR with 0x55/0xF4 is needed to cause complete decrypt failure
				 * in cases where data is encrypted with one type of IV and decrypted
				 * with another type IV. If identical key was used but only IV was
				 * computed differently, only first plaintext block of 512 byte CBC
				 * chain would decrypt incorrectly and rest would decrypt correctly.
				 * Partially correct decryption is dangerous. Decrypting all blocks
				 * incorrectly is safer because file system mount will simply fail.
				 */
				if(pi.multiKeyMode == 65) {
					multiKeyBits[r][0] ^= 0xF4; /* version 3 */
				} else {
					multiKeyBits[r][0] ^= 0x55; /* version 2 */
				}
				r++;
			}
		} else if(pi.passIterThousands) {
			aes_context ctx;
			unsigned long iter = 0;
			unsigned char tempkey[32];
			/*
			 * Set up AES-256 encryption key using same password and hash function
			 * as before but with password bit 0 flipped before hashing. That key
			 * is then used to encrypt actual loop key 'itercountk' thousand times.
			 */
			pass[0] ^= 1;
			(*hashFunc)((unsigned char *)pass, i, &tempkey[0], 32);
			__loDev_aes_set_key(&ctx, &tempkey[0], 32, 0);
			sscanf(pi.passIterThousands, "%lu", &iter);
			iter *= 1000;
			while(iter > 0) {
				/* encrypt both 128bit blocks with AES-256 */
				__loDev_aes_encrypt(&ctx, &loopinfo.lo_encrypt_key[ 0], &loopinfo.lo_encrypt_key[ 0]);
				__loDev_aes_encrypt(&ctx, &loopinfo.lo_encrypt_key[16], &loopinfo.lo_encrypt_key[16]);
				/* exchange upper half of first block with lower half of second block */
				memcpy(&tempkey[0], &loopinfo.lo_encrypt_key[8], 8);
				memcpy(&loopinfo.lo_encrypt_key[8], &loopinfo.lo_encrypt_key[16], 8);
				memcpy(&loopinfo.lo_encrypt_key[16], &tempkey[0], 8);
				iter--;
			}
			memset(&ctx, 0, sizeof(ctx));
			memset(&tempkey[0], 0, sizeof(tempkey));
		}
		memset(pass, 0, i);   /* erase original password */
		break;
	default:
		DBG(CXT, ul_debugobj(cxt, "don't know how to set up this type encryption"));
		myErrno = ENOKEY;
		goto loop_clr_fd_out;
	}

	if(pi.loInitValue) {
		/* cipher modules are free to do whatever they want with this value */
		i = 0;
		sscanf(pi.loInitValue, "%d", &i);
		loopinfo.lo_init[0] = i;
	}

	/* type 18 == LO_CRYPT_CRYPTOAPI */
	if ((loopinfo.lo_encrypt_type == 18) || (loDev_set_status64_ioctl(loop_dev_fd, &loopinfo) < 0)) {
		/* direct cipher interface failed - try CryptoAPI interface now */
		if(!apiName || (loDev_try_cryptoapi_interface(loop_dev_fd, &loopinfo, apiName) < 0)) {
			DBG(CXT, ul_debugobj(cxt, "loop set-status ioctl failed"));
			myErrno = ENOTSUP;
			loop_clr_fd_out:
			(void) ioctl(loop_dev_fd, LOOP_CLR_FD, 0);
			rc = -MNT_ERR_LOOPDEV;
			goto clean_up_out;
		}
	}
	if(pi.multiKeyMode >= 65) {
		if(ioctl(loop_dev_fd, LOOP_MULTI_KEY_SETUP_V3, &multiKeyBits[0][0]) < 0) {
			if(pi.multiKeyMode == 1000) goto try_v2_setup;
			DBG(CXT, ul_debugobj(cxt, "loop multi-key-v3 ioctl failed"));
			myErrno = ENOTSUP;
			goto loop_clr_fd_out;
		}
	} else if(pi.multiKeyMode == 64) {
		try_v2_setup:
		if((ioctl(loop_dev_fd, LOOP_MULTI_KEY_SETUP, &multiKeyBits[0][0]) < 0) && (pi.multiKeyMode != 1000)) {
			DBG(CXT, ul_debugobj(cxt, "loop multi-key-v2 ioctl failed"));
			myErrno = ENOTSUP;
			goto loop_clr_fd_out;
		}
	}

	if(run_mkfs_command && cxt->fs && cxt->fs->fstype && cxt->fs->fstype[0] && (getuid() == 0)) {
		if(!loDev_fork_mkfs_command(cxt, pi.loopDevName, cxt->fs->fstype)) {
			/* !strncasecmp(pi.passHashFuncName, "random", 6) test matched */
			/* This reads octal mode for newly created file system root */
			/* directory node from '-o phash=random/1777' mount option. */
			/*                          octal mode--^^^^                */
			if(sscanf(pi.passHashFuncName + 6, "/%o", &chmodVal) == 1) {
				/* bits 31...24 set to magic value, so that if something */
                                /* interprets hd->chmodVal as a file descriptor, it will fail */
				chmodVal &= 0x00FFFFFF;
				chmodVal |= 0x77000000;
			}

		} else {
			myErrno = ENOMEM;
			goto loop_clr_fd_out;
		}
	}

	rc = mnt_fs_set_source(cxt->fs, pi.loopDevName);
	if(rc) {
		myErrno = ENOMEM;
		goto loop_clr_fd_out;
	}

skip_setup_ok_out:
	/* success */
	if(chmodVal) {
		hd->chmodVal = (int)chmodVal;
	}

clean_up_out:
	if(loop_dev_fd != -1) close(loop_dev_fd);
	if(backing_fi_fd != -1) close(backing_fi_fd);

	memset(loopinfo.lo_encrypt_key, 0, sizeof(loopinfo.lo_encrypt_key));
	memset(&multiKeyBits[0][0], 0, sizeof(multiKeyBits));

	for(i = 0; i < 66; i++) {
		if(pi.multiKeyPass[i]) free(pi.multiKeyPass[i]);
	}
	if(pi.loopDevName) free(pi.loopDevName);
	if(pi.loopOffsetBytes) free(pi.loopOffsetBytes);
	if(pi.loopSizeBytes) free(pi.loopSizeBytes);
	if(pi.loopEncryptionType) free(pi.loopEncryptionType);
	if(pi.passSeedString) free(pi.passSeedString);
	if(pi.passHashFuncName) free(pi.passHashFuncName);
	if(pi.passIterThousands) free(pi.passIterThousands);
	if(pi.loInitValue) free(pi.loInitValue);
	if(pi.gpgKeyFile) free(pi.gpgKeyFile);
	if(pi.gpgHomeDir) free(pi.gpgHomeDir);
	if(pi.clearTextKeyFile) free(pi.clearTextKeyFile);

	if(apiName) free(apiName);
	if(pi.extraPtrToFree) free(pi.extraPtrToFree);

	if(pi.pass_cb_string && cxt->pwd_release_cb) {
		DBG(CXT, ul_debugobj(cxt, "release pass"));
		cxt->pwd_release_cb(cxt, pi.pass_cb_string);
	}
	if(myErrno) errno = myErrno;
	return rc;
}

static int delete_loopdev(struct libmnt_context *cxt)
{
	const char *src;
	int rc = 0, fd;

	assert(cxt);
	assert(cxt->fs);

	src = mnt_fs_get_srcpath(cxt->fs);
	if (!src)
		return -EINVAL;

	if(!loDev_is_loop_device(src))
		return -EINVAL;
		
	sync();
	if((fd = open(src, O_RDONLY)) < 0) {
		DBG(CXT, ul_debugobj(cxt, "can't open loop device"));
		rc = -ENODEV;
	} else {
		if(ioctl(fd, LOOP_CLR_FD, 0) < 0) {
			DBG(CXT, ul_debugobj(cxt, "loop crl-fd ioctl failed"));
			rc = -EINVAL;
		}
		close(fd);
	}

	DBG(CXT, ul_debugobj(cxt, "loopdev deleted [rc=%d]", rc));
	return rc;
}

/* Now used by umount until context_umount.c will use hooks toosee  */
int mnt_context_delete_loopdev(struct libmnt_context *cxt)
{
	return delete_loopdev(cxt);
}

static int is_loopdev_required(struct libmnt_context *cxt, struct libmnt_optlist *ol)
{
	const char *src, *type;
	unsigned long flags = 0;

	if (cxt->action != MNT_ACT_MOUNT)
		return 0;
	if (!cxt->fs)
		return 0;
	if (mnt_optlist_is_bind(ol)
	    || mnt_optlist_is_move(ol)
	    || mnt_context_propagation_only(cxt))
		return 0;

	src = mnt_fs_get_srcpath(cxt->fs);
	if (!src)
		return 0;		/* backing file not set */

	/* userspace flags */
	if (mnt_context_get_user_mflags(cxt, &flags))
		return 0;

	if (flags & (MNT_MS_LOOP | MNT_MS_OFFSET | MNT_MS_SIZELIMIT)) {
		DBG(LOOP, ul_debugobj(cxt, "loopdev specific options detected"));
		return 1;
	}

	/* Automatically create a loop device from a regular file if a
	 * filesystem is not specified or the filesystem is known for libblkid
	 * (these filesystems work with block devices only). The file size
	 * should be at least 1KiB, otherwise we will create an empty loopdev with
	 * no mountable filesystem...
	 *
	 * Note that there is no restriction (on kernel side) that would prevent a regular
	 * file as a mount(2) source argument. A filesystem that is able to mount
	 * regular files could be implemented.
	 */
	type = mnt_fs_get_fstype(cxt->fs);

	if (mnt_fs_is_regularfs(cxt->fs) &&
	    (!type || strcmp(type, "auto") == 0 || blkid_known_fstype(type))) {
		struct stat st;

		if (stat(src, &st) == 0 && S_ISREG(st.st_mode) &&
		    st.st_size > 1024) {

			DBG(LOOP, ul_debugobj(cxt, "automatically enabling loop= option"));
			mnt_optlist_append_flags(ol, MNT_MS_LOOP, cxt->map_userspace);
			return 1;
		}
	}

	return 0;
}

/* call after mount(2) */
static int hook_cleanup_loopdev(
			struct libmnt_context *cxt,
			const struct libmnt_hookset *hs __attribute__((__unused__)),
			void *data)
{
	unsigned int chmodVal;
	struct hook_data *hd = (struct hook_data *) data;

	if (mnt_context_get_status(cxt) == 0) {
		/*
		 * mount(2) failed, delete loopdev
		 */
		delete_loopdev(cxt);
	} else {
		/*
		 * mount(2) success
		 */
		if(hd && (hd->chmodVal != -1)) {
			chmodVal = (unsigned int) hd->chmodVal;
			if((chmodVal & 0xFF000000) == 0x77000000) { /* check magic value */
				chmodVal &= 0x00FFFFFF;
				if(cxt->fs && cxt->fs->target && cxt->fs->target[0]) {
					/*
					 * If loop was set up using random keys and new file system
					 * was created on the loop device, initial permissions for
					 * file system root directory need to be set here.
					 */
					DBG(CXT, ul_debugobj(cxt, "doing chmod() on mountpoint"));
					if(chmod(cxt->fs->target, chmodVal)) {
						DBG(CXT, ul_debugobj(cxt, "chmod() on mountpoint failed"));
					}
				}
			}
		}
	}
	return 0;
}

/* call to prepare mount source */
static int hook_prepare_loopdev(
                        struct libmnt_context *cxt,
                        const struct libmnt_hookset *hs,
                        void *data __attribute__((__unused__)))
{
	struct libmnt_optlist *ol;
	struct hook_data *hd;
	int rc;

	assert(cxt);

	ol = mnt_context_get_optlist(cxt);
	if (!ol)
		return -ENOMEM;
	if (!is_loopdev_required(cxt, ol))
		return 0;
	hd = new_hook_data();
	if (!hd)
		return -ENOMEM;

	rc = setup_loopdev(cxt, hd);
	if (!rc)
		rc = mnt_context_append_hook(cxt, hs,
				MNT_STAGE_MOUNT_POST,
				hd, hook_cleanup_loopdev);
	if (rc) {
		delete_loopdev(cxt);
		free(hd);
	}
	return rc;
}

const struct libmnt_hookset hookset_loopdev =
{
        .name = "__loopdev",

        .firststage = MNT_STAGE_PREP_SOURCE,
        .firstcall = hook_prepare_loopdev,

        .deinit = hookset_deinit
};
