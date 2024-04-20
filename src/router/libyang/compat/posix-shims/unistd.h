#ifndef _UNISTD_H
#define _UNISTD_H    1

/* headers are broken on Windows, which means that some of them simply *have* to come first */
# include <winsock2.h>
# include <ws2tcpip.h>

/* This is intended as a drop-in replacement for unistd.h on Windows.
 * Please add functionality as neeeded.
 * https://stackoverflow.com/a/826027/1202830
 */

#include <stdlib.h>
#include <io.h>
#include <process.h> /* for getpid() and the exec..() family */
#include <direct.h> /* for _getcwd() and _chdir() */

#define srandom srand
#define random rand

/* Values for the second argument to access.
   These may be OR'd together.  */
#define R_OK    4       /* Test for read permission.  */
#define W_OK    2       /* Test for write permission.  */
#define X_OK    0       /* jkt: unsupported on Windows, so we don't really care */
#define F_OK    0       /* Test for existence.  */

#define access _access
#define dup2 _dup2
#define execve _execve
#define ftruncate _chsize
#define unlink _unlink
#define fileno _fileno
#define getcwd _getcwd
#define chdir _chdir
#define isatty _isatty
#define lseek _lseek
#define fsync _commit
#define timegm _mkgmtime
/* read, write, and close are NOT being #defined here, because while there are file handle specific versions for Windows, they probably don't work for sockets. You need to look at your app and consider whether to call e.g. closesocket(). */

#define ssize_t SSIZE_T

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
/* should be in some equivalent to <sys/types.h> */
typedef __int8            int8_t;
typedef __int16           int16_t;
typedef __int32           int32_t;
typedef __int64           int64_t;
typedef unsigned __int8   uint8_t;
typedef unsigned __int16  uint16_t;
typedef unsigned __int32  uint32_t;
typedef unsigned __int64  uint64_t;

#include <windows.h>
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif
#ifndef _POSIX_PATH_MAX
#define _POSIX_PATH_MAX 256
#endif

#ifndef S_ISREG
#  define S_ISREG(m) (((m) & _S_IFMT) == _S_IFREG)
#endif
#ifndef S_ISDIR
#  define S_ISDIR(m) (((m) & _S_IFMT) == _S_IFDIR)
#endif
#ifndef S_IRUSR
# define S_IRUSR _S_IREAD
#endif
#ifndef S_IWUSR
# define S_IWUSR _S_IWRITE
#endif

#endif /* unistd.h  */
