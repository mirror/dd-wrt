/* ISC license. */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <sys/sysmacros.h>  /* makedev, major, minor */
#include <sys/stat.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>
#include <regex.h>
#include <libgen.h>  /* basename */
#include <stdio.h>  /* rename */
#include <sys/socket.h>
#include <linux/netlink.h>

#include <skalibs/posixplz.h>
#include <skalibs/types.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/bytestr.h>
#include <skalibs/strerr.h>
#include <skalibs/sgetopt.h>
#include <skalibs/sig.h>
#include <skalibs/selfpipe.h>
#include <skalibs/tai.h>
#include <skalibs/env.h>
#include <skalibs/cspawn.h>
#include <skalibs/djbunix.h>
#include <skalibs/iopause.h>
#include <skalibs/socket.h>
#include <skalibs/skamisc.h>
#include <skalibs/unix-transactional.h>

#include <mdevd/config.h>
#include "mdevd-internal.h"

#define USAGE "mdevd [ -v verbosity ] [ -D notif ] [ -I intake ] [ -o outputfd ] [ -O nlgroups ] [ -b kbufsz ] [ -f conffile ] [ -n | -N ] [ -s slashsys ] [ -d slashdev ] [ -F fwbase ] [ -C ]"
#define dieusage() strerr_dieusage(100, USAGE)

#define CONFBUFSIZE 8192
#define UEVENT_MAX_VARS 63
#define UEVENT_MAX_SIZE 8192

#define ACTION_NONE 0x0
#define ACTION_ADD 0x1
#define ACTION_REMOVE 0x2
#define ACTION_ANY 0x3

static int dryrun = 0 ;
static int cont = 1 ;
static unsigned int verbosity = 1 ;
static char const *slashsys = "/sys" ;
static char const *fwbase = "/lib/firmware" ;
static unsigned int root_maj, root_min ;

struct envmatch_s
{
  size_t var ;
  regex_t re ;
} ;

struct majmin_s
{
  unsigned int maj ;
  unsigned int minlo ;
  unsigned int minhi ;
} ;

union devmatch_u
{
  regex_t devre ;
  struct majmin_s majmin ;
} ;

#define DEVMATCH_NOTHING 0
#define DEVMATCH_CATCHALL 1
#define DEVMATCH_DEVRE 2
#define DEVMATCH_MAJMIN 3

#define MOVEINFO_NOTHING 0
#define MOVEINFO_NOCREATE 1
#define MOVEINFO_MOVE 2
#define MOVEINFO_MOVEANDLINK 3

typedef struct scriptelem_s scriptelem, *scriptelem_ref ;
struct scriptelem_s
{
  uid_t uid ;
  gid_t gid ;
  mode_t mode ;
  size_t movepath ;
  size_t command ;
  union devmatch_u devmatch ;
  unsigned int envmatchlen : 11 ;
  unsigned int devmatchtype : 2 ;
  unsigned int movetype : 2 ;
  unsigned int cmdtype : 2 ;
  unsigned int flagcont : 1 ;
  unsigned int flagexecline : 1 ;
  unsigned short envmatchs ;
} ;

static scriptelem const scriptelem_catchall =
{
  .uid = 0,
  .gid = 0,
  .mode = 0660,
  .movepath = 0,
  .command = 0,
  .envmatchlen = 0,
  .devmatchtype = DEVMATCH_CATCHALL,
  .movetype = MOVEINFO_NOTHING,
  .cmdtype = ACTION_NONE,
  .flagcont = 0,
  .flagexecline = 0,
  .envmatchs = 0
} ;

typedef struct udata_s udata, *udata_ref ;
struct udata_s
{
  char const *devname ;
  mode_t devtype ;
  unsigned int action ;
  int mmaj ;
  int mmin ;
  pid_t pid ;
  unsigned short i ;
  char sysdevpath[PATH_MAX] ;
  char buf[UEVENT_MAX_SIZE] ;
} ;
#define UDATA_ZERO { .devname = 0, .devtype = 0, .action = 0, .mmaj = -1, .mmin = -1, .pid = 0, .i = 0, .buf = "" }


 /* Utility functions */

static inline void script_free (scriptelem *script, unsigned short scriptlen, struct envmatch_s *envmatch, unsigned short envmatchlen)
{
  unsigned short i = 0 ;
  for (; i < scriptlen ; i++)
    if (script[i].devmatchtype == DEVMATCH_DEVRE)
      regfree(&script[i].devmatch.devre) ;
  for (i = 0 ; i < envmatchlen ; i++) regfree(&envmatch[i].re) ;
}

static inline int mdevd_mkdirp (char const *path)
{
  size_t n = strlen(path) ;
  if (!n) return 1 ;
  char s[n + 1] ;
  size_t i = 0 ;
  memcpy(s, path, n+1) ;
  for (; i < n ; i++)
  {
    if (s[i] == '/')
    {
      int r = 0 ;
      s[i] = 0 ;
      if (dryrun) strerr_warni2x("dry run: mkdir ", s) ;
      else r = mkdir(s, 0755) ;
      s[i] = '/' ;
      if (r < 0 && errno != EEXIST) break ;
    }
  }
  return i >= n ;
}

static int makesubdirs (char const *path)
{
  if (strrchr(path, '/') && !mdevd_mkdirp(path))
  {
    if (verbosity) strerr_warnwu2sys("create subdirectories for ", path) ;
    return 0 ;
  }
  return 1 ;
}

static inline int rebc_init (unsigned int groups, unsigned int kbufsz)
{
  struct sockaddr_nl nl = { .nl_family = AF_NETLINK, .nl_pad = 0, .nl_groups = groups, .nl_pid = 0 } ;
  int fd = socket_internal(AF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT, O_CLOEXEC) ;
  if (fd == -1) return -1 ;
  if (connect(fd, (struct sockaddr *)&nl, sizeof nl) == -1) goto err ;
  if (setsockopt(fd, SOL_SOCKET, SO_SNDBUFFORCE, &kbufsz, sizeof(unsigned int)) < 0)
  {
    if (errno != EPERM
     || setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &kbufsz, sizeof(unsigned int)) < 0) goto err ;
  }
  return fd ;

 err:
  fd_close(fd) ;
  return -1 ;
}


 /* mdev.conf parsing. See PARSING.txt for details. */

 /* The first pass is simple. The goal is just to compute scriptlen and envmatchlen. */

static inline unsigned char firstpass_cclass (char c)
{
  static unsigned char const classtable[65] = "08888888817881888888888888888888188438888888858888888888888288886" ;
  return (unsigned char)c < 65 ? classtable[(unsigned char)c] - '0' : 8 ;
}

static inline void script_firstpass (char *s, unsigned short *scriptlen, unsigned short *envmatchlen)
{
  static unsigned char const table[5][9] =
  {
    { 0x05, 0x00, 0x06, 0x34, 0x04, 0x01, 0x24, 0x40, 0x22 },
    { 0x06, 0x06, 0x06, 0x34, 0x06, 0x06, 0x24, 0x06, 0x22 },
    { 0x06, 0x04, 0x13, 0x02, 0x06, 0x02, 0x02, 0x06, 0x02 },
    { 0x06, 0x06, 0x06, 0x02, 0x06, 0x02, 0x02, 0x06, 0x02 },
    { 0x05, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x40, 0x04 }
  } ;
  size_t i = 0 ;
  size_t col0 = 0 ;
  unsigned int line = 1 ;
  unsigned short n = 0, m = 0 ;
  unsigned int state = 0 ;
  while (state < 5)
  {
    unsigned char what = table[state][firstpass_cclass(s[i])] ;
    state = what & 0x07 ;
    if (what & 0x10) m++ ;
    if (what & 0x20) n++ ;
    if (what & 0x40) { line++ ; col0 = i ; }
    i++ ;
  }
  if (state == 6)
  {
    char fmtline[UINT_FMT] ;
    char fmtcol[UINT_FMT] ;
    fmtline[uint_fmt(fmtline, line)] = 0 ;
    fmtcol[uint_fmt(fmtcol, i - col0)] = 0 ;
    strerr_dief6x(2, "syntax error during ", "first", " pass: line ", fmtline, " column ", fmtcol) ;
  }

  *scriptlen = n ;
  *envmatchlen = m ;
}

 /* The second pass is the real, complete mdev.conf parsing. */

static inline unsigned char secondpass_cclass (char c)
{
  static unsigned char const classtable[65] = "`rrrrrrrragrrarrrrrrrrrrrrrrrrrramrdcrqrrrlpherrnnnnnnnnookbrijrf" ;
  return (unsigned char)c < 65 ? classtable[(unsigned char)c] - '`' : 18 ;
}

static inline void script_secondpass (char *s, scriptelem *script, struct envmatch_s *envmatch)
{
  static uint32_t const table[30][19] =
  {
    { 0x0000001e, 0x00000000, 0x0000001f, 0x40000003, 0x00000001, 0x08000002, 0x20000007, 0x00000040, 0x1400000d, 0x0000001f, 0x0000001f, 0x1400000d, 0x1400000d, 0x1400000d, 0x1400000d, 0x1400000d, 0x1400000d, 0x1400000d, 0x1400000d },
    { 0x0000001e, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000040, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001 },
    { 0x0000001f, 0x0000001f, 0x0000001f, 0x40000003, 0x0000001f, 0x0000001f, 0x20000007, 0x0000001f, 0x1400000d, 0x0000001f, 0x0000001f, 0x1400000d, 0x1400000d, 0x1400000d, 0x1400000d, 0x1400000d, 0x1400000d, 0x1400000d, 0x1400000d },
    { 0x0000001f, 0x0000001f, 0x0000001f, 0x04040004, 0x0000001f, 0x04040004, 0x04040004, 0x0000001f, 0x04040004, 0x0000001f, 0x04040004, 0x04040004, 0x04040004, 0x04040004, 0x04040004, 0x04040004, 0x04040004, 0x04040004, 0x04040004 },
    { 0x0000001f, 0x0000001f, 0x0000001f, 0x00000004, 0x0000001f, 0x00000004, 0x00000004, 0x0000001f, 0x00000004, 0x02000005, 0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004 },
    { 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x04000006, 0x04000006, 0x0000001f, 0x04000006, 0x0000001f, 0x04000006, 0x04000006, 0x04000006, 0x04000006, 0x04000006, 0x04000006, 0x04000006, 0x04000006, 0x04000006 },
    { 0x0000001f, 0x03000012, 0x0000001f, 0x00000006, 0x0000001f, 0x00000006, 0x00000006, 0x0000001f, 0x00000006, 0x00000006, 0x00000006, 0x00000006, 0x00000006, 0x00000006, 0x00000006, 0x00000006, 0x00000006, 0x00000006, 0x00000006 },
    { 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x04000008, 0x04000008, 0x0000001f, 0x0000001f, 0x0000001f },
    { 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x02800009, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x00000008, 0x00000008, 0x0000001f, 0x0000001f, 0x0000001f },
    { 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0400000a, 0x0400000a, 0x0000001f, 0x0000001f, 0x0000001f },
    { 0x0000001f, 0x02600012, 0x0000001f, 0x0000001f, 0x0000001f, 0x0240000b, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000000a, 0x0000000a, 0x0000001f, 0x0000001f, 0x0000001f },
    { 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0400000c, 0x0400000c, 0x0000001f, 0x0000001f, 0x0000001f },
    { 0x0000001f, 0x02100012, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000000c, 0x0000000c, 0x0000001f, 0x0000001f, 0x0000001f },
    { 0x0000001f, 0x02080012, 0x0000001f, 0x0000000d, 0x0000001f, 0x0000000d, 0x0000000d, 0x0000001f, 0x0000000d, 0x0204000e, 0x0000001f, 0x0000000d, 0x0000000d, 0x0000000d, 0x0000000d, 0x0000000d, 0x0000000d, 0x0000000d, 0x0000000d },
    { 0x0000001f, 0x0000001f, 0x0000001f, 0x0400000f, 0x0000001f, 0x0400000f, 0x0400000f, 0x0000001f, 0x0400000f, 0x0400000f, 0x0400000f, 0x0400000f, 0x0400000f, 0x0400000f, 0x0400000f, 0x0400000f, 0x0400000f, 0x0400000f, 0x0400000f },
    { 0x0000001f, 0x0000001f, 0x03000010, 0x0000000f, 0x0000001f, 0x0000000f, 0x0000000f, 0x0000001f, 0x0000000f, 0x0000000f, 0x0000000f, 0x0000000f, 0x0000000f, 0x0000000f, 0x0000000f, 0x0000000f, 0x0000000f, 0x0000000f, 0x0000000f },
    { 0x0000001f, 0x0000001f, 0x0000001f, 0x04000011, 0x0000001f, 0x0400000d, 0x0400000d, 0x0000001f, 0x0400000d, 0x0000001f, 0x0000001f, 0x0400000d, 0x0400000d, 0x0400000d, 0x0400000d, 0x0400000d, 0x0400000d, 0x0400000d, 0x0400000d },
    { 0x0000001f, 0x02080012, 0x0000001f, 0x00000011, 0x0000001f, 0x00000011, 0x00000011, 0x0000001f, 0x00000011, 0x00000011, 0x00000011, 0x00000011, 0x00000011, 0x00000011, 0x00000011, 0x00000011, 0x00000011, 0x00000011, 0x00000011 },
    { 0x0000001f, 0x00000012, 0x0000001f, 0x0000001f, 0x0000001f, 0x04000013, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x04000013, 0x04000013, 0x04000013, 0x04000013, 0x04000013 },
    { 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x00000013, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x02020014, 0x0000001f, 0x0000001f, 0x00000013, 0x00000013, 0x00000013, 0x00000013, 0x00000013 },
    { 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x04000015, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x04000015, 0x04000015, 0x04000015, 0x04000015, 0x04000015 },
    { 0x0000001f, 0x02010016, 0x0000001f, 0x0000001f, 0x0000001f, 0x00000015, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x00000015, 0x00000015, 0x00000015, 0x00000015, 0x00000015 },
    { 0x0000001f, 0x00000016, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x04000017, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f },
    { 0x0000001f, 0x02008018, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0200c040, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x00000017, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f },
    { 0x0000401e, 0x00000018, 0x0000001f, 0x0000201c, 0x00004001, 0x0000203c, 0x0000101c, 0x00004040, 0x0000001f, 0x00000819, 0x00000419, 0x0000001f, 0x0000301c, 0x0000021b, 0x0000001f, 0x0000001f, 0x0000103c, 0x0000303c, 0x0000001f },
    { 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000011a, 0x0000011a, 0x0000001f, 0x0000011a, 0x0000011a, 0x0000001f, 0x0000011a, 0x0000001f, 0x0000011a, 0x0000011a, 0x0000011a, 0x0000011a, 0x0000011a, 0x0000011a },
    { 0x0000401e, 0x0200001b, 0x0000001f, 0x0000001f, 0x02004001, 0x0000001a, 0x0000001a, 0x02004040, 0x0000001a, 0x0000001a, 0x0000001f, 0x0000001a, 0x0000001f, 0x0000001a, 0x0000001a, 0x0000001a, 0x0000001a, 0x0000001a, 0x0000001a },
    { 0x0000401e, 0x0000001b, 0x0000001f, 0x0000201c, 0x00004001, 0x0000203c, 0x0000101c, 0x00004040, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000301c, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000103c, 0x0000303c, 0x0000001f },
    { 0x0000001f, 0x0000009d, 0x0000001f, 0x0000009d, 0x0000001f, 0x0000009d, 0x0000009d, 0x0000001f, 0x0000009d, 0x0000009d, 0x0000009d, 0x0000009d, 0x0000009d, 0x0000009d, 0x0000009d, 0x0000009d, 0x0000009d, 0x0000009d, 0x0000009d },
    { 0x0000401e, 0x0000001d, 0x0000001d, 0x0000001d, 0x0000001d, 0x0000001d, 0x0000001d, 0x02004040, 0x0000001d, 0x0000001d, 0x0000001d, 0x0000001d, 0x0000001d, 0x0000001d, 0x0000001d, 0x0000001d, 0x0000001d, 0x0000001d, 0x0000001d }
  } ;
  size_t mark = 0 ;
  size_t col0 = 0 ;
  size_t pos = 0 ;
  unsigned int line = 1 ;
  uint32_t state = 0 ;
  unsigned short i = 0 ; /* current scriptelem index */
  unsigned short j = 0 ; /* current envmatch index */
  while (state < 0x1e)
  {
    uint32_t what = table[state][secondpass_cclass(s[pos])] ;
    state = what & 0x1f ;
    if (what & 0x40000000)
    {
      script[i].devmatchtype = DEVMATCH_NOTHING ;
      script[i].envmatchs = j ;
    }
    if (what & 0x20000000) script[i].devmatchtype = DEVMATCH_MAJMIN ;
    if (what & 0x10000000)
    {
      script[i].devmatchtype = DEVMATCH_DEVRE ;
      script[i].envmatchs = j ;
    }
    if (what & 0x08000000) script[i].flagcont = 1 ;
    if (what & 0x04000000) mark = pos ;
    if (what & 0x02000000) s[pos] = 0 ;
    if (what & 0x01000000)
    {
      int r = regcomp(&envmatch[j].re, s + mark, REG_EXTENDED) ;
      if (r)
      {
        char errbuf[256] ;
        char fmtline[UINT_FMT] ;
        fmtline[uint_fmt(fmtline, line)] = 0 ;
        regerror(r, &envmatch[j].re, errbuf, 256) ;
        strerr_diefu6x(2, "compile regular expression ", s + mark, " for envmatch at line ", fmtline, ": ", errbuf) ;
      }
      j++ ;
      script[i].envmatchlen++ ;
    }
    if (what & 0x00800000)
    {
      if (!uint0_scan(s + mark, &script[i].devmatch.majmin.maj))
      {
        char fmtline[UINT_FMT] ;
        fmtline[uint_fmt(fmtline, line)] = 0 ;
        strerr_diefu4x(2, "get major from string ", s + mark, " at line ", fmtline) ;
      }
    }
    if (what & 0x00400000)
    {
      if (!uint0_scan(s + mark, &script[i].devmatch.majmin.minlo))
      {
        char fmtline[UINT_FMT] ;
        fmtline[uint_fmt(fmtline, line)] = 0 ;
        strerr_diefu4x(2, "get minor from string ", s + mark, " at line ", fmtline) ;
      }
    }
    if (what & 0x00200000) script[i].devmatch.majmin.minhi = script[i].devmatch.majmin.minlo ;
    if (what & 0x00100000)
    {
      if (!uint0_scan(s + mark, &script[i].devmatch.majmin.minhi))
      {
        char fmtline[UINT_FMT] ;
        fmtline[uint_fmt(fmtline, line)] = 0 ;
        strerr_diefu4x(2, "get minor from string ", s + mark, " at line ", fmtline) ;
      }
    }
    if (what & 0x00080000)
    {
      int r = regcomp(&script[i].devmatch.devre, s + mark, REG_EXTENDED) ;
      if (r)
      {
        char errbuf[256] ;
        char fmtline[UINT_FMT] ;
        fmtline[uint_fmt(fmtline, line)] = 0 ;
        regerror(r, &envmatch[j].re, errbuf, 256) ;
        strerr_diefu6x(2, "compile regular expression ", s + mark, " for devmatch at line ", fmtline, ": ", errbuf) ;
      }
    }
    if (what & 0x00040000) envmatch[j].var = mark ;
    if (what & 0x00020000)
    {
      struct passwd *pw = getpwnam(s + mark) ;
      if (pw) script[i].uid = pw->pw_uid ;
      else if (!uid0_scan(s + mark, &script[i].uid))
      {
        char fmtline[UINT_FMT] ;
        fmtline[uint_fmt(fmtline, line)] = 0 ;
        strerr_diefu4x(2, "get uid from string ", s + mark, " at line ", fmtline) ;
      }
    }
    if (what & 0x00010000)
    {
      struct group *gr = getgrnam(s + mark) ;
      if (gr) script[i].gid = gr->gr_gid ;
      else if (!gid0_scan(s + mark, &script[i].gid))
      {
        char fmtline[UINT_FMT] ;
        fmtline[uint_fmt(fmtline, line)] = 0 ;
        strerr_diefu4x(2, "get gid from string ", s + mark, " at line ", fmtline) ;
      }
    }
    if (what & 0x00008000)
    {
      unsigned int m ;
      if (!uint0_oscan(s + mark, &m))
      {
        char fmtline[UINT_FMT] ;
        fmtline[uint_fmt(fmtline, line)] = 0 ;
        strerr_diefu4x(2, "get mode from string ", s + mark, " at line ", fmtline) ;
      }
      script[i].mode = m ;
    }
    if (what & 0x00002000) { script[i].cmdtype |= ACTION_REMOVE ; script[i].flagexecline = 0 ; }
    if (what & 0x00001000) { script[i].cmdtype |= ACTION_ADD ; script[i].flagexecline = 0 ; }
    if (what & 0x00000800) script[i].movetype = MOVEINFO_MOVE ;
    if (what & 0x00000400) script[i].movetype = MOVEINFO_MOVEANDLINK ;
    if (what & 0x00000200) script[i].movetype = MOVEINFO_NOCREATE ;
    if (what & 0x00000100) script[i].movepath = pos ;
    if (what & 0x00000080) script[i].command = pos ;
    if (what & 0x00000020) script[i].flagexecline = 1 ;
    if (what & 0x00000040) { line++ ; col0 = pos ; }
    if (what & 0x00004000) i++ ;
    pos++ ;
  }

  if (state == 0x1f)
  {
    char fmtline[UINT_FMT] ;
    char fmtcol[UINT_FMT] ;
    fmtline[uint_fmt(fmtline, line)] = 0 ;
    fmtcol[uint_fmt(fmtcol, pos - col0)] = 0 ;
    strerr_dief6x(2, "syntax error during ", "second", " pass: line ", fmtline, " column ", fmtcol) ;
  }
}


 /* Firmware management */

static inline int wait_for_loading (char const *sysdevpath, size_t sysdevpathlen)
{
  int lfd = -1 ;
  unsigned int n = 150 ;
  static tain const period = { .sec = TAI_ZERO, .nano = 200000000 } ;
  char loadingfn[sysdevpathlen + 9] ;
  memcpy(loadingfn, sysdevpath, sysdevpathlen) ;
  memcpy(loadingfn + sysdevpathlen, "/loading", 9) ;
  tain_now_g() ;
  while (n--)  /* sysfs doesn't support inotify, so we have to poll -_- */
  {
    tain deadline ;
    lfd = open_write(loadingfn) ;
    if (lfd >= 0) break ;
    tain_add_g(&deadline, &period) ;
    deepsleepuntil_g(&deadline) ;
  }
  if (lfd >= 0 && ndelay_off(lfd) < 0)
  {
    fd_close(lfd) ;
    return -1 ;
  }
  return lfd ;
}

static inline void load_firmware (char const *fw, char const *sysdevpath)
{
  size_t fwbaselen = strlen(fwbase) ;
  size_t fwlen = strlen(fw) ;
  size_t sysdevpathlen = strlen(sysdevpath) ;
  int fwfd, loadingfd, datafd ;
  char fwfn[fwbaselen + fwlen + 2] ;
  memcpy(fwfn, fwbase, fwbaselen) ;
  fwfn[fwbaselen] = '/' ;
  memcpy(fwfn + fwbaselen + 1, fw, fwlen + 1) ;
  if (dryrun)
  {
    strerr_warni5x("dry run: copy ", fwfn, " to ", sysdevpath, "/data") ;
    return ;
  }
  fwfd = open_readb(fwfn) ;
  if (fwfd < 0 && verbosity >= 2) strerr_warnwu3sys("open ", fwfn, " for reading") ;
  loadingfd = wait_for_loading(sysdevpath, sysdevpathlen) ;
  if (loadingfd < 0)
  {
    if (verbosity >= 2) strerr_warnwu3sys("open ", sysdevpath, "/loading for writing") ;
    goto errclosef ;
  }
  if (fwfd < 0) goto errclosel ;
  if (fd_write(loadingfd, "1", 1) < 1)
  {
    if (verbosity >= 2) strerr_warnwu3sys("write 1 to ", sysdevpath, "/loading") ;
    goto errclosel ;
  }
  {
    char datafn[sysdevpathlen + 6] ;
    memcpy(datafn, sysdevpath, sysdevpathlen) ;
    memcpy(datafn + sysdevpathlen, "/data", 6) ;
    datafd = open_write(datafn) ;
    if (datafd < 0)
    {
      if (verbosity >= 2) strerr_warnwu3sys("open ", datafn, " for writing") ;
      goto errload ;
    }
    if (ndelay_off(datafd) < 0)
    {
      if (verbosity >= 2) strerr_warnwu2sys("ndelay_off ", datafn) ;
      goto errdata ;
    }
    if (fd_cat(fwfd, datafd) < 0)
    {
      if (verbosity >= 2) strerr_warnwu4sys("copy ", fwfn, " to ", datafn) ;
      goto errdata ;
    }
    fd_close(datafd) ;
    fd_write(loadingfd, "0", 1) ;
  }
  fd_close(loadingfd) ;
  fd_close(fwfd) ;
  return ;

 errdata:
  fd_close(datafd) ;
 errload:
  allwrite(loadingfd, "-1", 2) ;
 errclosel:
  fd_close(loadingfd) ;
 errclosef:
  if (fwfd >= 0) fd_close(fwfd) ;
}


 /* uevent management */

static inline unsigned char format_cclass (char c)
{
  static unsigned char const classtable[58] = "0333333333333333333333333333333333333133333333332222222222" ;
  return (unsigned char)c < 58 ? classtable[(unsigned char)c] - '0' : 3 ;
}

static inline ssize_t alias_format (char *out, size_t outmax, char const *in, char const *data, regmatch_t const *off)
{
  static unsigned char const table[2][4] = { { 0x12, 0x01, 0x10, 0x10 }, { 0x03, 0x10, 0x20, 0x03 } } ;
  size_t w = 0 ;
  unsigned int state = 0 ;
  while (state < 2)
  {
    char next = *in++ ;
    unsigned char what = table[state][format_cclass(next)] ;
    state = what & 0x03 ;
    if (what & 0x10)
    {
      if (w >= outmax) return (errno = ENAMETOOLONG, -1) ;
      if (out) out[w] = next ;
      w++ ;
    }
    if (what & 0x20)
    {
      unsigned int i = next - '0' ;
      size_t len = off[i].rm_eo - off[i].rm_so ;
      if (w + len > outmax) return -1 ;
      if (out) memcpy(out + w, data + off[i].rm_so, len) ;
      w += len ;
    }
  }
  if (state == 3) return (errno = EINVAL, -1) ;
  return w ;
}

static inline void spawn_command (char const *command, struct uevent_s const *event, int isel, udata *ud, char const *extra)
{
  char const *argv[4] = { isel ? "execlineb" : "/bin/sh", isel ? "-Pc" : "-c", command, 0 } ;
  size_t envlen = env_len((char const **)environ) ;
  size_t n ;
  char const *envp[envlen + event->varn + 1 + !!extra] ;
  n = env_merge(envp, envlen + event->varn + 1, (char const **)environ, envlen, event->buf + event->vars[1], event->len - event->vars[1]) ;
  if (extra)
  {
    envp[n - 1] = extra ;
    envp[n++] = 0 ;
  }
  ud->pid = cspawn(argv[0], argv, envp, CSPAWN_FLAGS_SELFPIPE_FINISH, 0, 0) ;
  if (!ud->pid)
  {
    if (verbosity) strerr_warnwu2sys("spawn ", argv[0]) ;
  }
}

static inline int run_scriptelem (struct uevent_s const *event, scriptelem const *elem, char const *storage, struct envmatch_s const *envmatch, udata *ud)
{
  size_t devnamelen = strlen(ud->devname) ;
  size_t nodelen = 0 ;
  char nodebuf[PATH_MAX] = "MDEV=" ;
  char *node = nodebuf + 5 ;
  regmatch_t off[10] ;
  unsigned short i = 0 ;
  for (; i < elem->envmatchlen ; i++)
  {
    char const *x = mdevd_uevent_getvar(event, storage + envmatch[elem->envmatchs + i].var) ;
    if (!x) return 0 ;
    if (regexec(&envmatch[elem->envmatchs + i].re, x, 0, 0, 0)) return 0 ;
  }

  switch (elem->devmatchtype)
  {
    case DEVMATCH_NOTHING:
    case DEVMATCH_CATCHALL: break ;
    case DEVMATCH_MAJMIN:
      if (ud->mmaj >= 0 && ud->mmin >= 0 && ud->mmaj == elem->devmatch.majmin.maj && ud->mmin >= elem->devmatch.majmin.minlo && ud->mmin <= elem->devmatch.majmin.minhi) break ;
      return 0 ;
    case DEVMATCH_DEVRE:
      if (!regexec(&elem->devmatch.devre, ud->devname, 10, off, 0)
       && !off[0].rm_so && off[0].rm_eo == strlen(ud->devname))
        break ;
      return 0 ;
  }

  switch (elem->movetype)
  {
    case MOVEINFO_NOTHING :
    case MOVEINFO_NOCREATE :
      memcpy(node, ud->devname, devnamelen + 1) ;
      nodelen = devnamelen ;
      break ;
    case MOVEINFO_MOVE :
    case MOVEINFO_MOVEANDLINK :
    {
      ssize_t r = alias_format(node, PATH_MAX - 5, storage + elem->movepath, ud->devname, off) ;
      if (r <= 1)
      {
        if (verbosity) strerr_warnwu5sys("process expression \"", storage + elem->movepath, "\" with devname \"", ud->devname, "\"") ;
        return -1 ;
      }
      nodelen = r - 1 ;
      if (node[nodelen - 1] == '/')
      {
        if (nodelen + devnamelen >= PATH_MAX - 6)
        {
          errno = ENAMETOOLONG ;
          if (verbosity) strerr_warnwu2sys("create alias for ", ud->devname) ;
          return -1 ;
        }
        memcpy(node + nodelen, ud->devname, devnamelen + 1) ;
        nodelen += devnamelen ;
      }
      break ;
    }
  }
  if (elem->movetype != MOVEINFO_NOCREATE && nodelen && ud->action == ACTION_ADD && ud->mmaj >= 0)
  {
    if (!makesubdirs(node)) return -1 ;
    if (dryrun)
    {
      char fmtmaj[UINT_FMT] ;
      char fmtmin[UINT_FMT] ;
      char fmtmode[UINT_OFMT] ;
      fmtmaj[uint_fmt(fmtmaj, ud->mmaj)] = 0 ;
      fmtmin[uint_fmt(fmtmin, ud->mmin)] = 0 ;
      fmtmode[uint_ofmt(fmtmode, elem->mode)] = 0 ;
      strerr_warni6x("dry run: mknod ", node, S_ISBLK(ud->devtype) ? " b " : " c ", fmtmaj, " ", fmtmin) ;
      strerr_warni4x("dry run: chmod ", fmtmode, " ", node) ;
    }
    else if (mknod(node, elem->mode | ud->devtype, makedev(ud->mmaj, ud->mmin)) < 0)
    {
      if (errno != EEXIST)
      {
        if (verbosity) strerr_warnwu2sys("mknod ", node) ;
        return -1 ;
      }
      if (chmod(node, elem->mode) < 0 && verbosity >= 2)
        strerr_warnwu2sys("chmod ", node) ;
    }
    if (elem->uid || elem->gid)
    {
      if (dryrun)
      {
        char fmtuid[UID_FMT] ;
        char fmtgid[GID_FMT] ;
        fmtuid[uid_fmt(fmtuid, elem->uid)] = 0 ;
        fmtgid[gid_fmt(fmtgid, elem->gid)] = 0 ;
        strerr_warni6x("dry run: chown ", fmtuid, ":", fmtgid, " ", node) ;
      }
      else if (chown(node, elem->uid, elem->gid) < 0 && verbosity >= 2)
        strerr_warnwu2sys("chown ", node) ;
    }
    if (ud->mmaj == root_maj && ud->mmin == root_min)
    {
      if (dryrun) strerr_warni3x("dry run: symlink ", node, " to root") ;
      else symlink(node, "root") ;
    }
    if (elem->movetype == MOVEINFO_MOVEANDLINK)
    {
      if (!makesubdirs(ud->devname)) return -1 ;
      if (dryrun) strerr_warni4x("dry run: symlink ", node, " to ", ud->devname) ;
      else if (atomic_symlink4(node, ud->devname, 0, 0) < 0)
      {
        if (verbosity) strerr_warnwu4sys("symlink ", node, " to ", ud->devname) ;
        return -1 ;
      }
    }
  }

  if (elem->cmdtype == ACTION_ANY || ud->action == elem->cmdtype)
  {
    if (dryrun)
    {
      strerr_warni4x("dry run: spawn ", elem->flagexecline ? "execlineb" : "/bin/sh", " for: ", storage + elem->command) ;
      if (verbosity >= 2)
      {
        char buf[UEVENT_MAX_SIZE + PATH_MAX + 5] ;
        size_t j = 0 ;
        unsigned short i = 1 ;
        for (; i < event->varn ; i++)
        {
          size_t len = strlen(event->buf + event->vars[i]) ;
          memcpy(buf + j, event->buf + event->vars[i], len) ;
          buf[j+len] = ' ' ;
          j += len+1 ;
        }
        if (nodelen)
        {
          memcpy(buf + j, nodebuf, nodelen + 5) ;
          j += nodelen + 6 ;
        }
        buf[j-1] = 0 ;
        strerr_warni2x("dry run: added variables: ", buf) ;
      }
    }
    else spawn_command(storage + elem->command, event, elem->flagexecline, ud, nodelen ? nodebuf : 0) ;
  }

  if (elem->movetype != MOVEINFO_NOCREATE && ud->action == ACTION_REMOVE && ud->mmaj >= 0)
  {
    if (elem->movetype == MOVEINFO_MOVEANDLINK)
    {
      if (dryrun) strerr_warni2x("dry run: unlink ", ud->devname) ;
      else unlink_void(ud->devname) ;
    }
    if (dryrun) strerr_warni2x("dry run: unlink ", node) ;
    else unlink_void(node) ;
  }

  return !elem->flagcont ;
}

static int run_script (struct uevent_s const *event, scriptelem const *script, unsigned short scriptlen, char const *storage, struct envmatch_s const *envmatch, udata *ud)
{
  for (; ud->i < scriptlen && !ud->pid ; ud->i++)
    if (run_scriptelem(event, script + ud->i, storage, envmatch, ud)) ud->i = scriptlen - 1 ;
  return ud->i >= scriptlen && !ud->pid ;
}

static inline int act_on_event (struct uevent_s const *event, unsigned int action, scriptelem const *script, unsigned short scriptlen, char const *storage, struct envmatch_s const *envmatch, udata *ud)
{
  ssize_t hasmajmin = 0 ;
  size_t sysdevpathlen = strlen(ud->sysdevpath) ;
  unsigned int mmaj, mmin ;
  char const *x = mdevd_uevent_getvar(event, "MAJOR") ;
  ud->devtype = S_IFCHR ;
  ud->action = action ;
  if (action == ACTION_ADD)
  {
    if (x && uint0_scan(x, &mmaj))
    {
      x = mdevd_uevent_getvar(event, "MINOR") ;
      if (x && uint0_scan(x, &mmin)) hasmajmin = 1 ;
    }
    if (!hasmajmin)
    {
      memcpy(ud->sysdevpath + sysdevpathlen, "/dev", 5) ;
      hasmajmin = openreadnclose(ud->sysdevpath, ud->buf, UINT_FMT << 1) ;
      ud->sysdevpath[sysdevpathlen] = 0 ;
      if (hasmajmin > 0)
      {
        size_t i = uint_scan(ud->buf, &mmaj) ;
        if (i > 0 && ud->buf[i] == ':')
        {
          size_t j = uint_scan(ud->buf + i + 1, &mmin) ;
          if (j > 0 && ud->buf[i+1+j] == '\n') ;
          else hasmajmin = 0 ;
        }
        else hasmajmin = 0 ;
      }
    }
  }
  ud->mmaj = hasmajmin > 0 ? mmaj : -1 ;
  ud->mmin = hasmajmin > 0 ? mmin : -1 ;

  ud->devname = mdevd_uevent_getvar(event, "DEVNAME") ;
  if (!ud->devname)
  {
    ssize_t r ;
    memcpy(ud->sysdevpath + sysdevpathlen, "/uevent", 8) ;
    r = openreadnclose(ud->sysdevpath, ud->buf, UEVENT_MAX_SIZE-1) ;
    ud->sysdevpath[sysdevpathlen] = 0 ;
    if (r > 0)
    {
      ud->buf[r] = 0 ;
      ud->devname = strstr(ud->buf, "\nDEVNAME=") ;
      if (ud->devname)
      {
        ud->devname += 9 ;
        *(char *)strchr(ud->devname, '\n') = 0 ;
      }
    }
    if (!ud->devname) ud->devname = basename(ud->sysdevpath) ;
  }
  if (strlen(ud->devname) >= PATH_MAX - 1)
  {
    if (verbosity) strerr_warnwu2x("device name too long: ", ud->devname) ;
    return 1 ;
  }
  if (strstr(ud->sysdevpath, "/block/")) ud->devtype = S_IFBLK ;
  else
  {
    x = mdevd_uevent_getvar(event, "SUBSYSTEM") ;
    if (x && str_start(x, "block")) ud->devtype = S_IFBLK ;
  }
  ud->i = 0 ;
  return run_script(event, script, scriptlen, storage, envmatch, ud) ;
}

static inline int on_event (struct uevent_s const *event, scriptelem const *script, unsigned short scriptlen, char const *storage, struct envmatch_s const *envmatch, udata *ud)
{
  int done = 1 ;
  char const *x = mdevd_uevent_getvar(event, "ACTION") ;
  unsigned int action ;
  if (!x) return 1 ;
  if (!strcmp(x, "add")) action = ACTION_ADD ;
  else if (!strcmp(x, "remove")) action = ACTION_REMOVE ;
  else action = ACTION_ANY ;
  x = mdevd_uevent_getvar(event, "DEVPATH") ;
  if (!x) return 1 ;
  {
    size_t devpathlen = strlen(x) ;
    size_t slashsyslen = strlen(slashsys) ;
    memcpy(ud->sysdevpath, slashsys, slashsyslen) ;
    memcpy(ud->sysdevpath + slashsyslen, x, devpathlen + 1) ;
  }
  x = mdevd_uevent_getvar(event, "FIRMWARE") ;
  if (action == ACTION_ADD || !x) done = act_on_event(event, action, script, scriptlen, storage, envmatch, ud) ;
  if (action == ACTION_ADD && x) load_firmware(x, ud->sysdevpath) ;
  return done ;
}


 /* Tying it all together */

static inline int handle_signals (struct uevent_s *event, scriptelem const *script, unsigned short scriptlen, char const *storage, struct envmatch_s const *envmatch, udata *ud)
{
  int e = 0 ;
  for (;;)
  {
    int c = selfpipe_read() ;
    switch (c)
    {
      case -1 : strerr_diefu1sys(111, "selfpipe_read") ;
      case 0 : return e ;
      case SIGTERM : cont = 0 ; break ;
      case SIGHUP : cont = 1 ; break ;
      case SIGCHLD :
        if (!ud->pid) wait_reap() ;
        else
        {
          int wstat ;
          int r = wait_pid_nohang(ud->pid, &wstat) ;
          if (r < 0)
            if (errno != ECHILD) strerr_diefu1sys(111, "wait_pid_nohang") ;
            else break ;
          else if (!r) break ;
          ud->pid = 0 ;
          e = run_script(event, script, scriptlen, storage, envmatch, ud) ;
        }
        break ;
      default :
        strerr_dief1x(101, "internal error: inconsistent signal handling. Please submit a bug-report.") ;
    }
  }
}

static inline int handle_event (int fd, struct uevent_s *event, scriptelem const *script, unsigned short scriptlen, char const *storage, struct envmatch_s const *envmatch, udata *ud)
{
  if (!mdevd_uevent_read(fd, event, 1, verbosity) || event->varn <= 1) return 0 ;
  return on_event(event, script, scriptlen, storage, envmatch, ud) ;
}

static inline int output_event (unsigned int outputfd, struct uevent_s *event)
{
  static char const c = 0 ;
  struct iovec v[2] = { { .iov_base = event->buf, .iov_len = event->len }, { .iov_base = (char *)&c, .iov_len = 1 } } ;
  if (fd_writev(outputfd, v, 2) < event->len + 1)
  {
    char fmt[UINT_FMT] ;
    fmt[uint_fmt(fmt, outputfd)] = 0 ;
    fd_close(outputfd) ;
    strerr_warnwu3sys("write to descriptor ", fmt, " (closing it)") ;
    return 0 ;
  }
  return 1 ;
}

static inline void rebc_event (int fd, struct uevent_s const *event)
{
  if (fd_send(fd, event->buf, event->len, 0) == -1)
    strerr_warnwu1sys("rebroadcast uevent") ;
}

int main (int argc, char const *const *argv)
{
  char const *configfile = "/etc/mdev.conf" ;
  iopause_fd x[2] = { { .events = IOPAUSE_READ }, { .events = IOPAUSE_READ } } ;
  unsigned int notif = 0 ;
  unsigned int kbufsz = 1048576 ;
  char const *slashdev = "/dev" ;
  int docoldplug = 0 ;
  unsigned int intake = 1 ;
  unsigned int outputfd = 0 ;
  unsigned int rebc = 0 ;
  PROG = "mdevd" ;
  {
    subgetopt l = SUBGETOPT_ZERO ;
    for (;;)
    {
      int opt = subgetopt_r(argc, argv, "nNv:D:I:o:O:b:f:s:d:F:C", &l) ;
      if (opt == -1) break ;
      switch (opt)
      {
        case 'n' : dryrun = 1 ; break ;
        case 'N' : dryrun = 2 ; break ;
        case 'v' : if (!uint0_scan(l.arg, &verbosity)) dieusage() ; break ;
        case 'D' : if (!uint0_scan(l.arg, &notif)) dieusage() ; break ;
        case 'I' : if (!uint0_scan(l.arg, &intake)) dieusage() ; break ;
        case 'o' : if (!uint0_scan(l.arg, &outputfd)) dieusage() ; break ;
        case 'O' : if (!uint0_scan(l.arg, &rebc)) dieusage() ; break ;
        case 'b' : if (!uint0_scan(l.arg, &kbufsz)) dieusage() ; break ;
        case 'f' : configfile = l.arg ; break ;
        case 's' : slashsys = l.arg ; break ;
        case 'd' : slashdev = l.arg ; break ;
        case 'F' : fwbase = l.arg ; break ;
        case 'C' : docoldplug = 1 ; break ;
        default : dieusage() ;
      }
    }
    argc -= l.ind ; argv += l.ind ;
  }

  if (configfile[0] != '/') strerr_dief2x(100, configfile, " is not an absolute path") ;
  if (slashsys[0] != '/') strerr_dief2x(100, slashsys, " is not an absolute path") ;
  if (slashdev[0] != '/') strerr_dief2x(100, slashdev, " is not an absolute path") ;
  if (fwbase[0] != '/') strerr_dief2x(100, fwbase, " is not an absolute path") ;
  if (chdir(slashdev) < 0) strerr_diefu2sys(111, "chdir to ", slashdev) ;
  if (strlen(slashsys) >= PATH_MAX - 1) strerr_dief1x(100, "paths too long") ;
  if (!fd_sanitize()) strerr_diefu1sys(111, "sanitize standard fds") ;
  if (notif)
  {
    if (notif < 3) strerr_dief1x(100, "notification fd must be 3 or more") ;
    if (fcntl(notif, F_GETFD) < 0) strerr_dief1sys(100, "invalid notification fd") ;
  }
  {
    unsigned int loop = rebc & (intake | 1) ;
    if (loop)
    {
      char fmt[UINT_FMT] ;
      fmt[uint_fmt(fmt, loop)] = 0 ;
      strerr_warnw3x("ignoring rebroadcast request on nlgroups ", fmt, " to avoid loops") ;
      rebc &= ~loop ;
    }
  }
  if (outputfd)
  {
    if (outputfd < 3) strerr_dief1x(100, "output fd must be 3 or more") ;
    if (fcntl(outputfd, F_GETFD) < 0) strerr_dief1sys(100, "invalid output fd") ;
    if (outputfd == notif) strerr_dief1x(100, "output fd and notification fd must not be the same") ;
    if (ndelay_on(outputfd) < 0) strerr_diefu1sys(111, "set output fd non-blocking") ;
    if (coe(outputfd) < 0) strerr_diefu1sys(111, "set output fd close-on-exec") ;
  }
  {
    struct stat st ;
    if (stat("/", &st) < 0) strerr_diefu1sys(111, "stat /") ;
    root_maj = major(st.st_dev) ;
    root_min = minor(st.st_dev) ;
  }
  x[1].fd = mdevd_netlink_init(intake, kbufsz) ;
  if (x[1].fd < 0) strerr_diefu1sys(111, "init netlink") ;
  if (rebc)
  {
    int fd = rebc_init(rebc, kbufsz) ;
    if (fd == -1) strerr_diefu2sys(111, "init netlink", " rebroadcast socket") ;
    rebc = fd ;
  }

  x[0].fd = selfpipe_init() ;
  if (x[0].fd < 0) strerr_diefu1sys(111, "init selfpipe") ;
  if (!sig_altignore(SIGPIPE))
    strerr_diefu1sys(111, "ignore SIGPIPE") ;
  {
    sigset_t set ;
    sigemptyset(&set) ;
    sigaddset(&set, SIGTERM) ;
    sigaddset(&set, SIGCHLD) ;
    sigaddset(&set, SIGHUP) ;
    if (!selfpipe_trapset(&set))
      strerr_diefu1sys(111, "trap signals") ;
  }

  tain_now_set_stopwatch_g() ;
  umask(0) ;

  while (cont)
  {
    ssize_t len ;
    unsigned short scriptlen = 0 ;
    unsigned short envmatchlen = 0 ;
    char storage[CONFBUFSIZE] ;
    len = openreadnclose(configfile, storage, CONFBUFSIZE - 1) ;
    if (len < 0)
    {
      if (errno != ENOENT) strerr_diefu2sys(111, "read ", configfile) ;
      if (verbosity) strerr_warnwu2sys("read ", configfile) ;
      len = 0 ;
    }
    storage[len++] = 0 ;
    script_firstpass(storage, &scriptlen, &envmatchlen) ;

    {
      struct uevent_s event = UEVENT_ZERO ;
      udata ud = UDATA_ZERO ;
      struct envmatch_s envmatch[envmatchlen ? envmatchlen : 1] ;
      scriptelem script[scriptlen + 1] ;
      memset(script, 0, scriptlen * sizeof(scriptelem)) ;
      script[scriptlen++] = scriptelem_catchall ;
      script_secondpass(storage, script, envmatch) ;
      if (dryrun == 2) break ;
      cont = 2 ;
      if (docoldplug)
      {
        char const *cargv[2] = { MDEVD_BINPREFIX "mdevd-coldplug", 0 } ;
        char const *cenv = 0 ;
        if (!cspawn(cargv[0], cargv, &cenv, CSPAWN_FLAGS_SELFPIPE_FINISH, 0, 0))
          strerr_warnwu2sys("spawn ", cargv[0]) ;
        docoldplug = 0 ;
      }
      if (notif)
      {
        fd_write(notif, "\n", 1) ;
        fd_close(notif) ;
        notif = 0 ;
      }

      while (ud.pid || cont == 2)
      {
        if (iopause_stamp(x, 1 + (!ud.pid && cont == 2), 0, 0) < 0) strerr_diefu1sys(111, "iopause") ;
        if (x[0].revents & IOPAUSE_READ && handle_signals(&event, script, scriptlen, storage, envmatch, &ud))
          goto dorebc ;
        if (!ud.pid && cont == 2 && x[1].revents & IOPAUSE_READ && handle_event(x[1].fd, &event, script, scriptlen, storage, envmatch, &ud))
          goto dorebc ;
        continue ;
       dorebc:
        if (outputfd && !output_event(outputfd, &event)) outputfd = 0 ;
        if (rebc) rebc_event(rebc, &event) ;
      }

      script_free(script, scriptlen, envmatch, envmatchlen) ;
    }
  }
  return 0 ;
}
