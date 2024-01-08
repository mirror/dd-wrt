/* Taken from Ted's losetup.c - Mitch <m.dsouza@mrc-apu.cam.ac.uk> */
/* Added vfs mount options - aeb - 960223 */
/* Removed lomount - aeb - 960224 */

/*
 * 1999-02-22 Arkadiusz Mi¶kiewicz <misiek@pld.ORG.PL>
 * - added Native Language Support
 * 1999-03-21 Arnaldo Carvalho de Melo <acme@conectiva.com.br>
 * - fixed strerr(errno) in gettext calls
 * 2001-04-11 Jari Ruusu
 * - added AES support
 */

#define LOOPMAJOR	7

/*
 * losetup.c - setup and control loop devices
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/sysmacros.h>
#include <sys/wait.h>
#include <limits.h>
#include <fcntl.h>
#include <mntent.h>
#include <locale.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <signal.h>

#include "loop.h"
#include "nls.h"
#include "../libmount/src/sha512.h"
#include "../libmount/src/rmd160.h"
#include "../libmount/src/aes.h"

#if !defined(BLKGETSIZE64)
# define BLKGETSIZE64 _IOR(0x12,114,size_t)
#endif

int verbose = 0;

#if !defined(LOOP_PASSWORD_MIN_LENGTH)
# define  LOOP_PASSWORD_MIN_LENGTH   20
#endif

char    *passFDnumber = (char *)0;
char    *passAskTwice = (char *)0;
char    *passSeedString = (char *)0;
char    *passHashFuncName = (char *)0;
char    *passIterThousands = (char *)0;
char    *loInitValue = (char *)0;
char    *gpgKeyFile = (char *)0;
char    *gpgHomeDir = (char *)0;
char    *clearTextKeyFile = (char *)0;
char    *loopOffsetBytes = (char *)0;
char    *loopSizeBytes = (char *)0;
char    *loopEncryptionType = (char *)0;

static int  multiKeyMode = 0;   /* 0=single-key 64=multi-key-v2 65=multi-key-v3 1000=any */
static char *multiKeyPass[66];
static char *loopFileName;

static char *xstrdup(const char *s)
{
    char *t;
    if(!s) return NULL;
    t = strdup(s);
    if(!t) {
        fprintf(stderr, "not enough memory\n");
        exit(2); /* EX_SYSERR */
    }
    return t;
}

static char *
crypt_name (int id, int *flags) {
	int i;

	for (i = 0; loop_crypt_type_tbl[i].id != -1; i++)
		if(id == loop_crypt_type_tbl[i].id) {
			*flags = loop_crypt_type_tbl[i].flags;
			return loop_crypt_type_tbl[i].name;
		}
	*flags = 0;
	if(id == 18)
		return "CryptoAPI";
	return "undefined";
}

static int
show_loop(char *device) {
	struct loop_info64 loopinfo;
	int fd;

	if ((fd = open(device, O_RDONLY)) < 0) {
		int errsv = errno;
		fprintf(stderr, _("loop: can't open device %s: %s\n"),
			device, strerror (errsv));
		return 2;
	}
	if (loop_get_status64_ioctl(fd, &loopinfo) < 0) {
		int errsv = errno;
		close (fd);
		fprintf(stderr, _("loop: can't get info on device %s: %s\n"),
			device, strerror (errsv));
		return 1;
	}
	close (fd);
	loopinfo.lo_file_name[LO_NAME_SIZE-1] = 0;
	loopinfo.lo_crypt_name[LO_NAME_SIZE-1] = 0;
	printf("%s: [%04llx]:%llu (%s)", device, (unsigned long long)loopinfo.lo_device,
		(unsigned long long)loopinfo.lo_inode, loopinfo.lo_file_name);
	if (loopinfo.lo_offset) {
		if ((long long)loopinfo.lo_offset < 0) {
			printf(_(" offset=@%llu"), -((unsigned long long)loopinfo.lo_offset));
		} else {
			printf(_(" offset=%llu"), (unsigned long long)loopinfo.lo_offset);
		}
	}
	if (loopinfo.lo_sizelimit)
		printf(_(" sizelimit=%llu"), (unsigned long long)loopinfo.lo_sizelimit);
	if (loopinfo.lo_encrypt_type) {
		int flags;
		char *s = crypt_name (loopinfo.lo_encrypt_type, &flags);

		printf(_(" encryption=%s"), s);
		/* type 18 == LO_CRYPT_CRYPTOAPI */
		if (loopinfo.lo_encrypt_type == 18) {
			printf("/%s", loopinfo.lo_crypt_name);
		} else {
			if(flags & 2)
				printf("-");
			if(flags & 1)
				printf("%u", (unsigned int)loopinfo.lo_encrypt_key_size << 3);
		}
	}
	switch(loopinfo.lo_flags & 0x180000) {
	case 0x180000:
		printf(_(" multi-key-v3"));
		break;
	case 0x100000:
		printf(_(" multi-key-v2"));
		break;
	}
	/* type 2 == LO_CRYPT_DES */
	if (loopinfo.lo_init[0] && (loopinfo.lo_encrypt_type != 2))
		printf(_(" loinit=%llu"), (unsigned long long)loopinfo.lo_init[0]);
	if (loopinfo.lo_flags & 0x200000)
		printf(_(" read-only"));
	printf("\n");

	return 0;
}

#define SIZE(a) (sizeof(a)/sizeof(a[0]))

static char *
find_unused_loop_device (void) {
	/* Just creating a device, say in /tmp, is probably a bad idea -
	   people might have problems with backup or so.
	   So, we just try /dev/loop[0-7]. */
	char dev[20];
	char *loop_formats[] = { "/dev/loop%d", "/dev/loop/%d" };
	int i, j, fd, somedev = 0, someloop = 0;
	struct stat statbuf;

	for (j = 0; j < (int)SIZE(loop_formats); j++) {
	    for(i = 0; i < 256; i++) {
		sprintf(dev, loop_formats[j], i);
		if (stat (dev, &statbuf) == 0 && S_ISBLK(statbuf.st_mode)) {
			somedev++;
			fd = open (dev, O_RDONLY);
			if (fd >= 0) {
				if (is_unused_loop_device(fd) == 0)
					someloop++;		/* in use */
				else if (errno == ENXIO) {
					close (fd);
					return xstrdup(dev);/* probably free */
				}
				close (fd);
			}
			continue;/* continue trying as long as devices exist */
		}
		break;
	    }
	}

	if (!somedev)
		fprintf(stderr, _("Error: could not find any loop device\n"));
	else if (!someloop)
		fprintf(stderr, _("Error: could not open any loop device\n"));
	else
		fprintf(stderr, _("Error: could not find any free loop device\n"));
	return 0;
}

static int rd_wr_retry(int fd, char *buf, int cnt, int w)
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

static char *get_FD_pass(int fd)
{
	char *p = NULL, *n;
	int x = 0, y = 0;

	do {
		if(y >= (x - 1)) {
			x += 128;
			/* Must enforce some max limit here -- this code   */
			/* runs as part of mount, and mount is setuid root */
			/* and has used mlockall(MCL_CURRENT | MCL_FUTURE) */
			if(x > (4*1024)) return(NULL);
			n = malloc(x);
			if(!n) return(NULL);
			if(p) {
				memcpy(n, p, y);
				memset(p, 0, y);
				free(p);
			}
			p = n;
		}
		if(rd_wr_retry(fd, p + y, 1, 0) != 1) break;
		if((p[y] == '\n') || !p[y]) break;
		y++;
	} while(1);
	if(p) p[y] = 0;
	return p;
}

static unsigned long long mystrtoull(char *s, int acceptAT)
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

static void warnAboutBadKeyData(int x)
{
	if((x > 1) && (x != 64) && (x != 65)) {
		fprintf(stderr, _("Warning: Unknown key data format - using it anyway\n"));
	}
}

static int are_these_files_same(const char *name1, const char *name2)
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

static char *do_GPG_pipe(char *pass)
{
	int     x, pfdi[2], pfdo[2];
	char    str[10], *a[16], *e[2], *h;
	pid_t   gpid;
	struct passwd *p;
	void    *oldSigPipeHandler;

	if((getuid() == 0) && gpgHomeDir && gpgHomeDir[0]) {
		h = gpgHomeDir;
	} else {
		if(!(p = getpwuid(getuid()))) {
			fprintf(stderr, _("Error: Unable to detect home directory for uid %d\n"), (int)getuid());
			return NULL;
		}
		h = p->pw_dir;
	}
	if(!(e[0] = malloc(strlen(h) + 6))) {
		nomem1:
		fprintf(stderr, _("Error: Unable to allocate memory\n"));
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
	 * When this code is run as part of losetup, normal read permissions
	 * affect the open() below because losetup is not setuid-root.
	 *
	 * When this code is run as part of mount, only root can set
	 * 'gpgKeyFile' and as such, only root can decide what file is opened
	 * below. However, since mount is usually setuid-root all non-root
	 * users can also open() the file too, but that file's contents are
	 * only piped to gpg. This readable-for-all is intended behaviour,
	 * and is very useful in situations where non-root users mount loop
	 * devices with their own gpg private key, and yet don't have access
	 * to the actual key used to encrypt loop device.
	 */
	if((x = open(gpgKeyFile, O_RDONLY)) == -1) {
		fprintf(stderr, _("Error: unable to open %s for reading\n"), gpgKeyFile);
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
	if(loopOffsetBytes && are_these_files_same(loopFileName, gpgKeyFile)) {
		FILE *f;
		char b[1024];
		long long cnt;
		int cnt2, cnt3;

		cnt = mystrtoull(loopOffsetBytes, 1);
		if(cnt < 0) cnt = -cnt;
		if(cnt > (1024 * 1024)) cnt = 1024 * 1024; /* sanity check */
		f = tmpfile();
		if(!f) {
			fprintf(stderr, _("Error: unable to create temp file\n"));
			close(x);
			goto nomem3;
		}
		while(cnt > 0) {
			cnt2 = sizeof(b);
			if(cnt < cnt2) cnt2 = cnt;
			cnt3 = rd_wr_retry(x, b, cnt2, 0);
			if(cnt3 && (fwrite(b, cnt3, 1, f) != 1)) {
				tmpWrErr:
				fprintf(stderr, _("Error: unable to write to temp file\n"));
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
		if(gpgHomeDir && gpgHomeDir[0]) {
			a[x++] = "--homedir";
			a[x++] = gpgHomeDir;
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
	rd_wr_retry(pfdi[1], pass, x, 1);
	rd_wr_retry(pfdi[1], "\n", 1, 1);
	if(oldSigPipeHandler != SIG_ERR) signal(SIGPIPE, oldSigPipeHandler);

	close(pfdi[1]);
	memset(pass, 0, x);
	x = 0;
	while(x < 66) {
		multiKeyPass[x] = get_FD_pass(pfdo[0]);
		if(!multiKeyPass[x]) {
			/* mem alloc failed - abort */
			multiKeyPass[0] = 0;
			break;
		}
		if(strlen(multiKeyPass[x]) < LOOP_PASSWORD_MIN_LENGTH) break;
		x++;
	}
	warnAboutBadKeyData(x);
	if(x >= 65)
		multiKeyMode = 65;
	if(x == 64)
		multiKeyMode = 64;
	close(pfdo[0]);
	waitpid(gpid, &x, 0);
	if(!multiKeyPass[0]) goto nomem1;
	return multiKeyPass[0];
}

static char *sGetPass(int minLen, int warnLen)
{
	char *p, *s, *seed;
	int i, ask2, close_i_fd = 0;

	if(!passFDnumber) {
		if(clearTextKeyFile) {
			if((i = open(clearTextKeyFile, O_RDONLY)) == -1) {
				fprintf(stderr, _("Error: unable to open %s for reading\n"), clearTextKeyFile);
				return NULL;
			}
			close_i_fd = 1;
			goto contReadFrom_i;
		}
		p = getpass(_("Password: "));
		ask2 = passAskTwice ? 1 : 0;
	} else {
		i = atoi(passFDnumber);
		contReadFrom_i:
		if(gpgKeyFile && gpgKeyFile[0]) {
			p = get_FD_pass(i);
			if(close_i_fd) close(i);
		} else {
			int x = 0;
			while(x < 66) {
				multiKeyPass[x] = get_FD_pass(i);
				if(!multiKeyPass[x]) goto nomem;
				if(strlen(multiKeyPass[x]) < LOOP_PASSWORD_MIN_LENGTH) break;
				x++;
			}
			if(close_i_fd) close(i);
			warnAboutBadKeyData(x);
			if(x >= 65) {
				multiKeyMode = 65;
				return multiKeyPass[0];
			}
			if(x == 64) {
				multiKeyMode = 64;
				return multiKeyPass[0];
			}
			p = multiKeyPass[0];
		}
		ask2 = 0;
	}
	if(!p) goto nomem;
	if(gpgKeyFile && gpgKeyFile[0]) {
		if(ask2) {
			i = strlen(p);
			s = malloc(i + 1);
			if(!s) goto nomem;
			strcpy(s, p);
			p = getpass(_("Retype password: "));
			if(!p) goto nomem;
			if(strcmp(s, p)) goto compareErr;
			memset(s, 0, i);
			free(s);
			ask2 = 0;
		}
		p = do_GPG_pipe(p);
		if(!p) return(NULL);
		if(!p[0]) {
			fprintf(stderr, _("Error: gpg key file decryption failed\n"));
			return(NULL);
		}
		if(multiKeyMode) return(p);
	}
	i = strlen(p);
	if(i < minLen) {
		fprintf(stderr, _("Error: Password must be at least %d characters.\n"), minLen);
		return(NULL);
	}
	seed = passSeedString;
	if(!seed) seed = "";
	s = malloc(i + strlen(seed) + 1);
	if(!s) {
		nomem:
		fprintf(stderr, _("Error: Unable to allocate memory\n"));
		return(NULL);
	}
	strcpy(s, p);
	memset(p, 0, i);
	if(ask2) {
		p = getpass(_("Retype password: "));
		if(!p) goto nomem;
		if(strcmp(s, p)) {
			compareErr:
			fprintf(stderr, _("Error: Passwords are not identical\n"));
			return(NULL);
		}
		memset(p, 0, i);
	}
	if(i < warnLen) {
		fprintf(stderr, _("WARNING - Please use longer password (%d or more characters)\n"), LOOP_PASSWORD_MIN_LENGTH);
	}
	strcat(s, seed);
	return(s);
}

/* this is for compatibility with historic loop-AES version */
static void unhashed1_key_setup(unsigned char *keyStr, int ile, unsigned char *keyBuf, int bufSize)
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
static void unhashed2_key_setup(unsigned char *keyStr, int ile __attribute__((__unused__)), unsigned char *keyBuf, int bufSize)
{
	memset(keyBuf, 0, bufSize);
	strncpy((char *)keyBuf, (char *)keyStr, bufSize - 1);
	keyBuf[bufSize - 1] = 0;
}

static void rmd160HashTwiceWithA(unsigned char *ib, int ile, unsigned char *ob, int ole)
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

static long long xx_lseek(int fd, long long offset, int whence)
{
	if(sizeof(off_t) >= 8) {
		return lseek(fd, offset, whence);
	} else {
		return llseek(fd, offset, whence);
	}
}

static int loop_create_random_keys(char *partition, long long offset, long long sizelimit, int loopro, unsigned char *k)
{
	int x, y, fd;
	sha512_context s;
	unsigned char b[4096];

	if(loopro) {
		fprintf(stderr, _("Error: read-only device %s\n"), partition);
		return 1;
	}

	/*
	 * Compute SHA-512 over first 40 KB of old fs data. SHA-512 hash
	 * output is then used as entropy for new fs encryption key.
	 */
	if((fd = open(partition, O_RDWR)) == -1) {
		seekFailed:
		fprintf(stderr, _("Error: unable to open/seek device %s\n"), partition);
		return 1;
	}
	if(offset < 0) offset = -offset;
	if(xx_lseek(fd, offset, SEEK_SET) == -1) {
		close(fd);
		goto seekFailed;
	}
	__loDev_sha512_init(&s);
	for(x = 1; x <= 10; x++) {
		if((sizelimit > 0) && ((long long)(sizeof(b) * x) > sizelimit)) break;
		if(rd_wr_retry(fd, (char *) &b[0], sizeof(b), 0) != sizeof(b)) break;
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
		if(xx_lseek(fd, offset, SEEK_SET) == -1) break;
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
			if(rd_wr_retry(fd, (char *) &b[0], sizeof(b), 1) != sizeof(b)) break;
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
		fprintf(stderr, _("Error: unable to open /dev/urandom\n"));
		return 1;
	}
	rd_wr_retry(fd, (char *) &b[64], 32, 0);

	/* generate multi-key hashes */
	x = 0;
	while(x < 65) {
		rd_wr_retry(fd, (char *) &b[64+32], 16, 0);
		__loDev_sha512_hash_buffer(&b[0], 64+32+16, k, 32);
		k += 32;
		x++;
	}

	close(fd);
	memset(&b[0], 0, sizeof(b));
	return 0;
}

static int
set_loop(const char *device, const char *file, int *loopro, int busyRetVal) {
	struct loop_info64 loopinfo;
	int fd, ffd, mode, i, errRetVal = 1;
	char *pass, *apiName = NULL;
	void (*hashFunc)(unsigned char *, int, unsigned char *, int);
	unsigned char multiKeyBits[65][32];
	int minPassLen = LOOP_PASSWORD_MIN_LENGTH;

	sync();
	loopFileName = (char *)file;
	multiKeyMode = 0;
	mode = (*loopro ? O_RDONLY : O_RDWR);
	if ((ffd = open(file, mode)) < 0) {
		if (!*loopro && errno == EROFS)
			ffd = open(file, mode = O_RDONLY);
		if (ffd < 0) {
			perror(file);
			return 1;
		}
	}
	if ((fd = open(device, mode)) < 0) {
		perror (device);
		goto close_ffd_return1;
	}
	*loopro = (mode == O_RDONLY);

	if (ioctl(fd, LOOP_SET_FD, ffd) < 0) {
		if(errno == EBUSY)
			errRetVal = busyRetVal;
		if((errRetVal != 2) || verbose)
			perror("ioctl: LOOP_SET_FD");
keyclean_close_fd_ffd_return1:
		memset(loopinfo.lo_encrypt_key, 0, sizeof(loopinfo.lo_encrypt_key));
		memset(&multiKeyBits[0][0], 0, sizeof(multiKeyBits));
		close (fd);
close_ffd_return1:
		close (ffd);
		return errRetVal;
	}

	memset (&loopinfo, 0, sizeof (loopinfo));
	strncpy ((char *)loopinfo.lo_file_name, file, LO_NAME_SIZE - 1);
	loopinfo.lo_file_name[LO_NAME_SIZE - 1] = 0;
	if (loopEncryptionType)
		loopinfo.lo_encrypt_type = loop_crypt_type (loopEncryptionType, &loopinfo.lo_encrypt_key_size, &apiName);
	if (loopOffsetBytes)
		loopinfo.lo_offset = mystrtoull(loopOffsetBytes, 1);
	if (loopSizeBytes)
		loopinfo.lo_sizelimit = mystrtoull(loopSizeBytes, 0);

#ifdef MCL_FUTURE
	/*
	 * Oh-oh, sensitive data coming up. Better lock into memory to prevent
	 * passwd etc being swapped out and left somewhere on disk.
	 */

	if(loopinfo.lo_encrypt_type && mlockall(MCL_CURRENT | MCL_FUTURE)) {
		perror("memlock");
		ioctl (fd, LOOP_CLR_FD, 0);
		fprintf(stderr, _("Couldn't lock into memory, exiting.\n"));
		exit(1);
	}
#endif

	switch (loopinfo.lo_encrypt_type) {
	case LO_CRYPT_NONE:
		loopinfo.lo_encrypt_key_size = 0;
		break;
	case LO_CRYPT_XOR:
		pass = sGetPass (1, 0);
		if(!pass) goto loop_clr_fd_out;
		strncpy ((char *)loopinfo.lo_encrypt_key, pass, LO_KEY_SIZE - 1);
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
		if(passHashFuncName) {
			if(!strcasecmp(passHashFuncName, "sha256")) {
				hashFunc = __loDev_sha256_hash_buffer;
			} else if(!strcasecmp(passHashFuncName, "sha384")) {
				hashFunc = __loDev_sha384_hash_buffer;
			} else if(!strcasecmp(passHashFuncName, "sha512")) {
				hashFunc = __loDev_sha512_hash_buffer;
			} else if(!strcasecmp(passHashFuncName, "rmd160")) {
				hashFunc = rmd160HashTwiceWithA;
				minPassLen = 1;
			} else if(!strcasecmp(passHashFuncName, "unhashed1")) {
				hashFunc = unhashed1_key_setup;
			} else if(!strcasecmp(passHashFuncName, "unhashed2")) {
				hashFunc = unhashed2_key_setup;
				minPassLen = 1;
			} else if(!strncasecmp(passHashFuncName, "random", 6) && ((passHashFuncName[6] == 0) || (passHashFuncName[6] == '/'))) {
				/* random hash type sets up 65 random keys */
				/* WARNING! DO NOT USE RANDOM HASH TYPE ON PARTITION WITH EXISTING */
				/* IMPORTANT DATA ON IT. RANDOM HASH TYPE WILL DESTROY YOUR DATA.  */
				if(loop_create_random_keys((char*)file, loopinfo.lo_offset, loopinfo.lo_sizelimit, *loopro, &multiKeyBits[0][0])) {
					goto loop_clr_fd_out;
				}
				memcpy(&loopinfo.lo_encrypt_key[0], &multiKeyBits[0][0], sizeof(loopinfo.lo_encrypt_key));
				multiKeyMode = 1000;
				break; /* out of switch(loopinfo.lo_encrypt_type) */
			}
		}
		pass = sGetPass (minPassLen, LOOP_PASSWORD_MIN_LENGTH);
		if(!pass) goto loop_clr_fd_out;
		i = strlen(pass);
		if(hashFunc == unhashed1_key_setup) {
			/* this is for compatibility with historic loop-AES version */
			loopinfo.lo_encrypt_key_size = 16;             /* 128 bits */
			if(i >= 32) loopinfo.lo_encrypt_key_size = 24; /* 192 bits */
			if(i >= 43) loopinfo.lo_encrypt_key_size = 32; /* 256 bits */
		}
		(*hashFunc)((unsigned char *)pass, i, &loopinfo.lo_encrypt_key[0], sizeof(loopinfo.lo_encrypt_key));
		if(multiKeyMode) {
			int r = 0, t;
			while(r < multiKeyMode) {
				t = strlen(multiKeyPass[r]);
				(*hashFunc)((unsigned char *)multiKeyPass[r], t, &multiKeyBits[r][0], 32);
				memset(multiKeyPass[r], 0, t);
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
				if(multiKeyMode == 65) {
					multiKeyBits[r][0] ^= 0xF4; /* version 3 */
				} else {
					multiKeyBits[r][0] ^= 0x55; /* version 2 */
				}
				r++;
			}
		} else if(passIterThousands) {
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
			sscanf(passIterThousands, "%lu", &iter);
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
		fprintf (stderr, _("Error: don't know how to get key for encryption system %d\n"), loopinfo.lo_encrypt_type);
		goto loop_clr_fd_out;
	}

	if(loInitValue) {
		/* cipher modules are free to do whatever they want with this value */
		i = 0;
		sscanf(loInitValue, "%d", &i);
		loopinfo.lo_init[0] = i;
	}

	/* type 18 == LO_CRYPT_CRYPTOAPI */
	if ((loopinfo.lo_encrypt_type == 18) || (loop_set_status64_ioctl(fd, &loopinfo) < 0)) {
		/* direct cipher interface failed - try CryptoAPI interface now */
		if(!apiName || (try_cryptoapi_loop_interface(fd, &loopinfo, apiName) < 0)) {
			fprintf(stderr, _("ioctl: LOOP_SET_STATUS: %s, requested cipher or key length (%d bits) not supported by kernel\n"), strerror(errno), loopinfo.lo_encrypt_key_size << 3);
			loop_clr_fd_out:
			(void) ioctl (fd, LOOP_CLR_FD, 0);
			goto keyclean_close_fd_ffd_return1;
		}
	}
	if(multiKeyMode >= 65) {
		if(ioctl(fd, LOOP_MULTI_KEY_SETUP_V3, &multiKeyBits[0][0]) < 0) {
			if(multiKeyMode == 1000) goto try_v2_setup;
			perror("ioctl: LOOP_MULTI_KEY_SETUP_V3");
			goto loop_clr_fd_out;
		}
	} else if(multiKeyMode == 64) {
		try_v2_setup:
		if((ioctl(fd, LOOP_MULTI_KEY_SETUP, &multiKeyBits[0][0]) < 0) && (multiKeyMode != 1000)) {
			perror("ioctl: LOOP_MULTI_KEY_SETUP");
			goto loop_clr_fd_out;
		}
	}

	memset(loopinfo.lo_encrypt_key, 0, sizeof(loopinfo.lo_encrypt_key));
	memset(&multiKeyBits[0][0], 0, sizeof(multiKeyBits));
	close (fd);
	close (ffd);

	if (verbose > 1)
		printf(_("set_loop(%s,%s): success\n"), device, file);
	return 0;
}

#include <getopt.h>
#include <stdarg.h>

static char *progname;

static void
usage(void) {
	fprintf(stderr, _("usage:\n\
  %s [options] loop_device file        # setup\n\
  %s -F [options] loop_device [file]   # setup, read /etc/fstab\n\
  %s loop_device                       # give info\n\
  %s -a                                # give info of all loops\n\
  %s -f                                # show next free loop device\n\
  %s -d loop_device                    # delete\n\
  %s -R loop_device                    # resize\n\
options:  -e encryption  -o offset  -s sizelimit  -p passwdfd  -T  -S pseed\n\
          -H phash  -I loinit  -K gpgkey  -G gpghome  -C itercountk  -v  -r\n\
          -P cleartextkey\n"),
		progname, progname, progname, progname, progname, progname, progname);
	exit(1);
}

static void
show_all_loops(void)
{
	char dev[20];
	char *lfmt[] = { "/dev/loop%d", "/dev/loop/%d" };
	int i, j, fd, x;
	struct stat statbuf;

	for(i = 0; i < 256; i++) {
		for(j = (sizeof(lfmt) / sizeof(lfmt[0])) - 1; j >= 0; j--) {
			sprintf(dev, lfmt[j], i);
			if(stat(dev, &statbuf) == 0 && S_ISBLK(statbuf.st_mode)) {
				fd = open(dev, O_RDONLY);
				if(fd >= 0) {
					x = is_unused_loop_device(fd);
					close(fd);
					if(x == 0) {
						show_loop(dev);
						j = 0;
					}
				}
			}
		}
	}
}

static int
read_options_from_fstab(char *loopToFind, char **partitionPtr)
{
	FILE *f;
	struct mntent *m;
	int y, foundMatch = 0;
	char *opt, *fr1, *fr2;
	struct options {
		char *name;	/* name of /etc/fstab option */
		char **dest;	/* destination where it is written to */
		char *line;	/* temp */
	};
	struct options tbl[] = {
		{ "device/file name ",	partitionPtr },	/* must be index 0 */
		{ "loop=",		&loopToFind },	/* must be index 1 */
		{ "offset=",		&loopOffsetBytes },
		{ "sizelimit=",		&loopSizeBytes },
		{ "encryption=",	&loopEncryptionType },
		{ "pseed=",		&passSeedString },
		{ "phash=",		&passHashFuncName },
		{ "loinit=",		&loInitValue },
		{ "gpgkey=",		&gpgKeyFile },
		{ "gpghome=",		&gpgHomeDir },
		{ "cleartextkey=",	&clearTextKeyFile },
		{ "itercountk=",	&passIterThousands },
	};
	struct options *p;

	if (!(f = setmntent("/etc/fstab", "r"))) {
		fprintf(stderr, _("Error: unable to open /etc/fstab for reading\n"));
		return 0;
	}
	while ((m = getmntent(f)) != NULL) {
		tbl[0].line = fr1 = xstrdup(m->mnt_fsname);
		p = &tbl[1];
		do {
			p->line = NULL;
		} while (++p < &tbl[sizeof(tbl) / sizeof(struct options)]);
		opt = fr2 = xstrdup(m->mnt_opts);
		for (opt = strtok(opt, ","); opt != NULL; opt = strtok(NULL, ",")) {
			p = &tbl[1];
			do {
				y = strlen(p->name);
				if (!strncmp(opt, p->name, y))
					p->line = opt + y;
			} while (++p < &tbl[sizeof(tbl) / sizeof(struct options)]);
		}
		if (tbl[1].line && !strcmp(loopToFind, tbl[1].line)) {
			if (++foundMatch > 1) {
				fprintf(stderr, _("Error: multiple loop=%s options found in /etc/fstab\n"), loopToFind);
				endmntent(f);
				return 0;
			}
			p = &tbl[0];
			do {
				if (!*p->dest && p->line) {
					*p->dest = p->line;
					if (verbose)
						printf(_("using %s%s from /etc/fstab\n"), p->name, p->line);
				}
			} while (++p < &tbl[sizeof(tbl) / sizeof(struct options)]);
			fr1 = fr2 = NULL;
		}
		if(fr1) free(fr1);
		if(fr2) free(fr2);
	}
	endmntent(f);
	if (foundMatch == 0) {
		fprintf(stderr, _("Error: loop=%s option not found in /etc/fstab\n"), loopToFind);
	}
	return foundMatch;
}

static int
recompute_loop_dev_size(char *device)
{
	int fd, err1 = 0, err2, err3;
	long long oldBytes = -1, newBytes = -1;

	fd = open(device, O_RDONLY);
	if(fd < 0) {
		perror(device);
		return 1;
	}
	if(verbose) {
		err1 = ioctl(fd, BLKGETSIZE64, &oldBytes);
	}
	err2 = ioctl(fd, LOOP_RECOMPUTE_DEV_SIZE, 0);
	if(err2) {
		perror(device);
		goto done1;
	}
	if(verbose) {
		err3 = ioctl(fd, BLKGETSIZE64, &newBytes);
		if(!err1 && (oldBytes >= 0)) {
			printf("%s: old size %lld bytes\n", device, oldBytes);
		}
		if(!err3 && (newBytes >= 0)) {
			printf("%s: new size %lld bytes\n", device, newBytes);
		}
	}
done1:
	close(fd);
	return err2;
}

static int 
del_loop (const char *device) {
	int fd;

	sync();
	if ((fd = open (device, O_RDONLY)) < 0) {
		int errsv = errno;
		fprintf(stderr, _("loop: can't delete device %s: %s\n"),
			device, strerror (errsv));
		return 1;
	}
	if (ioctl (fd, LOOP_CLR_FD, 0) < 0) {
		perror ("ioctl: LOOP_CLR_FD");
		return 1;
	}
	close (fd);
	if (verbose > 1)
		printf(_("del_loop(%s): success\n"), device);
	return 0;
}

int
main(int argc, char **argv) {
	char *partitionName = NULL;
	char *device = NULL;
	int delete,find,c,option_a=0,option_F=0,option_R=0,setup_o=0;
	int res = 0;
	int ro = 0;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	delete = find = 0;
	progname = argv[0];
	while ((c = getopt(argc,argv,"aC:de:fFG:H:I:K:o:p:P:rRs:S:Tv")) != -1) {
		switch (c) {
		case 'a':		/* show status of all loops */
			option_a = 1;
			break;
		case 'C':
			passIterThousands = optarg;
			setup_o = 1;
			break;
		case 'd':
			delete = 1;
			break;
		case 'e':
			loopEncryptionType = optarg;
			setup_o = 1;
			break;
		case 'f':		/* find free loop */
			find = 1;
			break;
		case 'F':		/* read loop related options from /etc/fstab */
			option_F = 1;
			setup_o = 1;
			break;
		case 'G':               /* GnuPG home dir */
			gpgHomeDir = optarg;
			setup_o = 1;
			break;
		case 'H':               /* passphrase hash function name */
			passHashFuncName = optarg;
			setup_o = 1;
			break;
		case 'I':               /* lo_init[0] value (in string form)  */
			loInitValue = optarg;
			setup_o = 1;
			break;
		case 'K':               /* GnuPG key file name */
			gpgKeyFile = optarg;
			setup_o = 1;
			break;
		case 'o':
			loopOffsetBytes = optarg;
			setup_o = 1;
			break;
		case 'p':               /* read passphrase from given fd */
			passFDnumber = optarg;
			setup_o = 1;
			break;
		case 'P':               /* read passphrase from given file */
			clearTextKeyFile = optarg;
			setup_o = 1;
			break;
		case 'r':               /* read-only */
			ro = 1;
			setup_o = 1;
			break;
		case 'R':               /* recompute loop dev size */
			option_R = 1;
			break;
		case 's':
			loopSizeBytes = optarg;
			setup_o = 1;
			break;
		case 'S':               /* optional seed for passphrase */
			passSeedString = optarg;
			setup_o = 1;
			break;
		case 'T':               /* ask passphrase _twice_ */
			passAskTwice = "T";
			setup_o = 1;
			break;
		case 'v':
			verbose++;
			break;
		default:
			usage();
		}
	}
	if (option_a + delete + option_R + setup_o + find > 1) usage();
	if (option_a) {
		/* show all loops */
		if (argc != optind) usage();
		show_all_loops();
		res = 0;
	} else if (find) {
		if (argc != optind)
			usage();
		device = find_unused_loop_device();
		if (device == NULL)
			return -1;
		if (verbose)
			printf("Loop device is %s\n", device);
		printf("%s\n", device);
		res = 0;
	} else if (delete) {
		/* delete loop */
		if (argc != optind+1) usage();
		res = del_loop(argv[optind]);
	} else if (option_R) {
		/* resize existing loop */
		if (argc != optind+1) usage();
		res = recompute_loop_dev_size(argv[optind]);
	} else if ((argc == optind+1) && !setup_o) {
		/* show one loop */
		res = show_loop(argv[optind]);
	} else {
		/* set up new loop */
		if ((argc < optind+1) || ((argc == optind+1) && !option_F) || (argc > optind+2))
			usage();
		if (argc > optind+1)
			partitionName = argv[optind+1];
		if (option_F && (read_options_from_fstab(argv[optind], &partitionName) != 1))
			exit(1);
		res = set_loop(argv[optind],partitionName,&ro, 1);
	}
	return res;
}
