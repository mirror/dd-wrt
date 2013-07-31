
#include <sys/prctl.h>
#include <linux/capability.h>

#ifndef _LINUX_CAPABILITY_VERSION_3
#define _LINUX_CAPABILITY_VERSION_3  0x20080522
#define _LINUX_CAPABILITY_U32S_3     2
#endif

/* CAP_TO_MASK is missing in CentOS header files */
#ifndef CAP_TO_MASK
#define CAP_TO_MASK(x)      (1 << ((x) & 31))
#endif

/* capset() prototype is missing ... */
int capset(cap_user_header_t hdrp, const cap_user_data_t datap);

static inline int
set_capabilities(u32 caps)
{
  struct __user_cap_header_struct cap_hdr;
  struct __user_cap_data_struct cap_dat[_LINUX_CAPABILITY_U32S_3];
  int err;

  cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
  cap_hdr.pid = 0;

  memset(cap_dat, 0, sizeof(cap_dat));
  cap_dat[0].effective = cap_dat[0].permitted = caps;

  err = capset(&cap_hdr, cap_dat);
  if (!err)
    return 0;

  /* Kernel may support do not support our version of capability interface.
       The last call returned supported version so we just retry it. */
  if (errno == EINVAL)
  {
    err = capset(&cap_hdr, cap_dat);
    if (!err)
      return 0;
  }

  return -1;
}

static void
drop_uid(uid_t uid)
{
  u32 caps =
    CAP_TO_MASK(CAP_NET_BIND_SERVICE) |
    CAP_TO_MASK(CAP_NET_BROADCAST) |
    CAP_TO_MASK(CAP_NET_ADMIN) |
    CAP_TO_MASK(CAP_NET_RAW);

  /* change effective user ID to be able to switch to that
     user ID completely after dropping CAP_SETUID */
  if (seteuid(uid) < 0)
    die("seteuid: %m");

  /* restrict the capabilities */
  if (set_capabilities(caps) < 0)
    die("capset: %m");

  /* keep the capabilities after dropping root ID */
  if (prctl(PR_SET_KEEPCAPS, 1) < 0)
    die("prctl: %m");

  /* completely switch to the unprivileged user ID */
  if (setresuid(uid, uid, uid) < 0)
    die("setresuid: %m");
}
