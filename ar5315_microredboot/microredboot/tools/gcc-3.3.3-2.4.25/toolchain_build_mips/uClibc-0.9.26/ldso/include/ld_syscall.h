#ifndef _LD_SYSCALL_H_
#define _LD_SYSCALL_H_

/* Pull in the arch specific syscall implementation */
#include <ld_syscalls.h>
/*  For MAP_ANONYMOUS -- differs between platforms */
#include <asm/mman.h>			
/* Pull in whatever this particular arch's kernel thinks the kernel version of
 * struct stat should look like.  It turns out that each arch has a different
 * opinion on the subject, and different kernel revs use different names... */
#define kernel_stat stat
#include <bits/kernel_stat.h>


/* Encoding of the file mode.  */
#define	S_IFMT		0170000	/* These bits determine file type.  */

/* File types.  */
#define	S_IFDIR		0040000	/* Directory.  */
#define	S_IFCHR		0020000	/* Character device.  */
#define	S_IFBLK		0060000	/* Block device.  */
#define	S_IFREG		0100000	/* Regular file.  */
#define	S_IFIFO		0010000	/* FIFO.  */
#define	S_IFLNK		0120000	/* Symbolic link.  */
#define	S_IFSOCK	0140000	/* Socket.  */

/* Protection bits.  */

#define	S_ISUID		04000	/* Set user ID on execution.  */
#define	S_ISGID		02000	/* Set group ID on execution.  */
#define	S_ISVTX		01000	/* Save swapped text after use (sticky).  */
#define	S_IREAD		0400	/* Read by owner.  */
#define	S_IWRITE	0200	/* Write by owner.  */
#define	S_IEXEC		0100	/* Execute by owner.  */


/* Here are the definitions for some syscalls that are used
   by the dynamic linker.  The idea is that we want to be able
   to call these before the errno symbol is dynamicly linked, so
   we use our own version here.  Note that we cannot assume any
   dynamic linking at all, so we cannot return any error codes.
   We just punt if there is an error. */


#define __NR__dl_exit __NR_exit
static inline _syscall1(void, _dl_exit, int, status);


#define __NR__dl_close __NR_close
static inline _syscall1(int, _dl_close, int, fd);


#if defined(__powerpc__) || defined(__mips__) || defined(__sh__)
/* PowerPC, MIPS and SuperH have a different calling convention for mmap(). */
#define __NR__dl_mmap __NR_mmap
static inline _syscall6(void *, _dl_mmap, void *, start, size_t, length,
		int, prot, int, flags, int, fd, off_t, offset);
#else
#define __NR__dl_mmap_real __NR_mmap
static inline _syscall1(void *, _dl_mmap_real, unsigned long *, buffer);

static inline void * _dl_mmap(void * addr, unsigned long size, int prot,
		int flags, int fd, unsigned long offset)
{
	unsigned long buffer[6];

	buffer[0] = (unsigned long) addr;
	buffer[1] = (unsigned long) size;
	buffer[2] = (unsigned long) prot;
	buffer[3] = (unsigned long) flags;
	buffer[4] = (unsigned long) fd;
	buffer[5] = (unsigned long) offset;
	return (void *) _dl_mmap_real(buffer);
}
#endif

#ifndef _dl_MAX_ERRNO
#define _dl_MAX_ERRNO 4096
#endif
#define _dl_mmap_check_error(__res)	\
	(((int)__res) < 0 && ((int)__res) >= -_dl_MAX_ERRNO)
#ifndef MAP_ANONYMOUS
#ifdef __sparc__
#define MAP_ANONYMOUS 0x20
#else
#error MAP_ANONYMOUS not defined and suplementary value not known
#endif
#endif


#define __NR__dl_open __NR_open
#define O_RDONLY        0x0000
#define O_WRONLY	     01
#define O_RDWR		     02
#define O_CREAT		   0100	/* not fcntl */
static inline _syscall2(int, _dl_open, const char *, fn, int, flags);

#define __NR__dl_write __NR_write
static inline _syscall3(unsigned long, _dl_write, int, fd, 
	    const void *, buf, unsigned long, count);


#define __NR__dl_read __NR_read
static inline _syscall3(unsigned long, _dl_read, int, fd, 
	    const void *, buf, unsigned long, count);

#define __NR__dl_mprotect __NR_mprotect
static inline _syscall3(int, _dl_mprotect, const void *, addr, unsigned long, len, int, prot);



#define __NR__dl_stat __NR_stat
static inline _syscall2(int, _dl_stat, const char *, file_name, struct stat *, buf);


#define __NR__dl_munmap __NR_munmap
static inline _syscall2(int, _dl_munmap, void *, start, unsigned long, length);

#define __NR__dl_getuid __NR_getuid
static inline _syscall0(uid_t, _dl_getuid);

#define __NR__dl_geteuid __NR_geteuid
static inline _syscall0(uid_t, _dl_geteuid);

#define __NR__dl_getgid __NR_getgid
static inline _syscall0(gid_t, _dl_getgid);

#define __NR__dl_getegid __NR_getegid
static inline _syscall0(gid_t, _dl_getegid);

#define __NR__dl_getpid __NR_getpid
static inline _syscall0(gid_t, _dl_getpid);

/*
 * Not an actual syscall, but we need something in assembly to say whether
 * this is OK or not.
 */
static inline int _dl_suid_ok(void)
{
    uid_t uid, euid, gid, egid;

    uid = _dl_getuid();
    euid = _dl_geteuid();
    gid = _dl_getgid();
    egid = _dl_getegid();

    if(uid == euid && gid == egid)
	return 1;
    else
	return 0;
}

#define __NR__dl_readlink __NR_readlink
static inline _syscall3(int, _dl_readlink, const char *, path, char *, buf, size_t, bufsiz);

#endif /* _LD_SYSCALL_H_ */

