#define VSTR_SC_POSIX_C
/*
 *  Copyright (C) 2002, 2003, 2004, 2005  James Antill
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  email: james@and.org
 */
/* functions which are shortcuts */
#include "main.h"

/* Linux gives EINVAL on x86 > SSIZE_MAX, but gives same on > 2GB for
 * x86-64 etc. ... Ie. INT_MAX limit, in theory this stops signed overflow
 * problems in the kernel, however they don't have a constant so we have
 * to assume INT_MAX ... retards. */
#define VSTR__UIO_MAXDATA INT_MAX


#if !(USE_FD_CLOSE_CHECK) /* hack to test code path on close error */
# define VSTR__POSIX_OPEN3(x, y, z) open64(x, y, z)
# define VSTR__POSIX_CLOSE(x) close(x)
#else
 /* this is all non threadsafe ... */
# define VSTR__POSIX_OPEN3(x, y, z)  vstr__sc_posix_open(x, y, z)
# define VSTR__POSIX_CLOSE(x) vstr__sc_posix_close(x)
static int vstr__sc_posix_open(const char *pathname, int flags,
                               VSTR_AUTOCONF_mode_t mode)
{
  int ret = open64(pathname, flags, mode);
  
  ASSERT((ret == -1) || ++vstr__options.fd_count);

  return (ret);
}

static int vstr__sc_posix_close(int fd)
{
  int ret = close(fd);

  ASSERT(vstr__options.fd_count-- > 0);
  
  if ((ret != -1) &&
      vstr__options.fd_close_fail_num && !--vstr__options.fd_close_fail_num)
  {
    errno = EIO;
    return (-1);
  }

  return (ret);
}
#endif

#define VSTR__POSIX_OPEN1(x)                                    \
    VSTR__POSIX_OPEN3(x, O_RDONLY | O_NOCTTY | O_NONBLOCK, 0)

#ifndef HAVE_MMAP
# define VSTR__SC_ENOSYS(x) \
  if (err) \
  { \
    errno = ENOSYS; \
    *err = (x); \
  } \
  \
  return (FALSE)
#else
static void vstr__sc_ref_munmap(Vstr_ref *passed_ref)
{ /* debugging is all non threadsafe ... */
  Vstr__sc_mmap_ref *mmap_ref = (Vstr__sc_mmap_ref *)passed_ref;

  munmap(mmap_ref->ref.ptr, mmap_ref->mmap_len); /* this _can't_ be -1 */

  VSTR__F(mmap_ref);

  ASSERT(vstr__options.mmap_count-- > 0);
}
#endif

static int vstr__sc_get_size(size_t base_len,
                             int fd, size_t *len, VSTR_AUTOCONF_off64_t off,
                             unsigned int *err,
                             unsigned int err_fstat, unsigned int err_size)
    VSTR__COMPILE_ATTR_NONNULL_A();
static int vstr__sc_get_size(size_t base_len,
                             int fd, size_t *len, VSTR_AUTOCONF_off64_t off,
                             unsigned int *err,
                             unsigned int err_fstat, unsigned int err_size)
{
  struct stat64 stat_buf;

  ASSERT(len && err);

  if (*len)
    return (TRUE);

  if (fstat64(fd, &stat_buf) == -1)
  {
    *err = err_fstat;
    return (FALSE);
  }

  if (!stat_buf.st_size)
    return (TRUE);
  
  if (stat_buf.st_size <= off)
  {
    *err = err_fstat;
    errno = ENOSPC;
    return (FALSE);
  }

  *len = (stat_buf.st_size - off);
  if (*len > (SIZE_MAX - base_len))
  {
    *err = err_size;
    errno = EFBIG;
    return (FALSE);
  }

  return (TRUE);
}


#ifndef HAVE_MMAP
int vstr_sc_mmap_fd(Vstr_base *VSTR__ATTR_UNUSED(base),
                    size_t VSTR__ATTR_UNUSED(pos),
                    int VSTR__ATTR_UNUSED(fd),
                    VSTR_AUTOCONF_off64_t VSTR__ATTR_UNUSED(off),
                    size_t VSTR__ATTR_UNUSED(len),
                    unsigned int *err)
{ VSTR__SC_ENOSYS(VSTR_TYPE_SC_MMAP_FD_ERR_MMAP_ERRNO); }
#else
int vstr_sc_mmap_fd(Vstr_base *base, size_t pos, int fd,
                    VSTR_AUTOCONF_off64_t off, size_t len, unsigned int *err)
{
  unsigned int dummy_err;
  void *addr = NULL;
  Vstr__sc_mmap_ref *mmap_ref = NULL;

  assert(off >= 0); /* off is offset from the start of the file,
                     * not as in seek */

  if (!err)
    err = &dummy_err;
  *err = 0;

  ASSERT_GOTO(base && (pos <= base->len), inval_args);
  
  if (!vstr__sc_get_size(base->len, fd, &len, off, err,
                         VSTR_TYPE_SC_MMAP_FD_ERR_FSTAT_ERRNO,
                         VSTR_TYPE_SC_MMAP_FD_ERR_TOO_LARGE))
    return (FALSE);

  addr = mmap64(NULL, len, PROT_READ, MAP_PRIVATE, fd, off);
  if (addr == MAP_FAILED)
  {
    *err = VSTR_TYPE_SC_MMAP_FD_ERR_MMAP_ERRNO;
    return (FALSE);
  }

  if (!(mmap_ref = VSTR__MK(sizeof(Vstr__sc_mmap_ref))))
    goto malloc_mmap_ref_failed;
  mmap_ref->mmap_len = len;
  mmap_ref->ref.func = vstr__sc_ref_munmap;
  mmap_ref->ref.ptr = (void *)addr;
  mmap_ref->ref.ref = 0;

  if (!vstr_add_ref(base, pos, &mmap_ref->ref, 0, len))
    goto add_ref_failed;

  ASSERT(++vstr__options.mmap_count);

  return (TRUE);

 add_ref_failed:
  VSTR__F(mmap_ref);
 malloc_mmap_ref_failed:
  munmap(addr, len);
  *err = VSTR_TYPE_SC_MMAP_FILE_ERR_MEM;
  errno = ENOMEM;
  base->conf->malloc_bad = TRUE;

  return (FALSE);
  
 inval_args:
  *err = VSTR_TYPE_SC_MMAP_FD_ERR_MMAP_ERRNO;
  errno = EINVAL;
  return (FALSE);
}
#endif

#ifndef HAVE_MMAP
int vstr_sc_mmap_file(Vstr_base *VSTR__ATTR_UNUSED(base),
                      size_t VSTR__ATTR_UNUSED(pos),
                      const char *VSTR__ATTR_UNUSED(filename),
                      VSTR_AUTOCONF_off64_t VSTR__ATTR_UNUSED(off),
                      size_t VSTR__ATTR_UNUSED(len),
                      unsigned int *err)
{ VSTR__SC_ENOSYS(VSTR_TYPE_SC_MMAP_FD_ERR_MMAP_ERRNO); }
#else
int vstr_sc_mmap_file(Vstr_base *base, size_t pos, const char *filename,
                      VSTR_AUTOCONF_off64_t off, size_t len,
                      unsigned int *err)
{
  int fd = -1;
  unsigned int dummy_err;
  int ret = 0;
  int saved_errno = 0;

  if (!err)
    err = &dummy_err;
  *err = 0;

  if ((fd = VSTR__POSIX_OPEN1(filename)) == -1)
  {
    *err = VSTR_TYPE_SC_MMAP_FILE_ERR_OPEN_ERRNO;
    return (FALSE);
  }

  ret = vstr_sc_mmap_fd(base, pos, fd, off, len, err);

  if (*err)
    saved_errno = errno;

  if ((VSTR__POSIX_CLOSE(fd) == -1) && !*err)
  {
    *err = VSTR_TYPE_SC_MMAP_FILE_ERR_CLOSE_ERRNO;
    return (FALSE);
  }
  
  if (*err)
    errno = saved_errno;

  return (ret);
}
#endif

static int vstr__sc_read_slow_len_fd(Vstr_base *base, size_t pos, int fd,
                                     size_t len, unsigned int *err)
{
  size_t orig_pos = pos;
  ssize_t bytes = -1;
  unsigned int num = 0;
  Vstr_ref *ref = NULL;

  ASSERT_GOTO(base && (pos <= base->len), inval_args);

  if (pos && !vstr__add_setup_pos(base, &pos, &num, NULL))
    goto mem_fail;
  pos = orig_pos;

  if (!(ref = vstr_ref_make_malloc(len)))
    goto mem_fail;

  if (!vstr_cntl_conf(base->conf, VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_REF,
                      1, UINT_MAX))
    goto mem_ref_fail;

  do
  {
    bytes = read(fd, ref->ptr, len);
  } while ((bytes == -1) && (errno == EINTR));

  if (bytes == -1)
  {
    *err = VSTR_TYPE_SC_READ_FD_ERR_READ_ERRNO;
    goto mem_ref_fail;
  }
  if (!bytes)
  {
    *err = VSTR_TYPE_SC_READ_FD_ERR_EOF;
    errno = ENOSPC;
    goto mem_ref_fail;
  }

  num = vstr_add_ref(base, pos, ref, 0, bytes); /* must work */
  ASSERT(num);
  
  vstr_ref_del(ref);

  return (TRUE);

 mem_ref_fail:
  assert(ref);
  vstr_ref_del(ref);
  
 mem_fail:
  if (!*err)
  {
    *err = VSTR_TYPE_SC_READ_FD_ERR_MEM;
    errno = ENOMEM;
  }
  
  return (FALSE);

 inval_args:
  *err = VSTR_TYPE_SC_READ_FD_ERR_READ_ERRNO;
  errno = EINVAL;
  return (FALSE);
}

static int vstr__sc_read_fast_iov_fd(Vstr_base *base, size_t pos, int fd,
                                     struct iovec *iovs, unsigned int num,
                                     unsigned int *err)
{
  ssize_t bytes = -1;

  if (num > UIO_MAXIOV)
    num = UIO_MAXIOV;

  do
  {
    bytes = readv(fd, iovs, num);
  } while ((bytes == -1) && (errno == EINTR));

  if (bytes == -1)
  {
    vstr_add_iovec_buf_end(base, pos, 0);
    *err = VSTR_TYPE_SC_READ_FD_ERR_READ_ERRNO;
    return (FALSE);
  }

  vstr_add_iovec_buf_end(base, pos, (size_t)bytes);

  if (!bytes)
  {
    *err = VSTR_TYPE_SC_READ_FD_ERR_EOF;
    errno = ENOSPC;
    return (FALSE);
  }

  return (TRUE);
}

int vstr_sc_read_iov_fd(Vstr_base *base, size_t pos, int fd,
                        unsigned int min, unsigned int max,
                        unsigned int *err)
{
  struct iovec *iovs = NULL;
  unsigned int num = 0;
  unsigned int dummy_err;

  if (!err)
    err = &dummy_err;
  *err = 0;

  assert(max >= min);

  ASSERT_GOTO(base && (pos <= base->len), inval_args);
  
  if (!min)
    return (TRUE);

  if (min > ((VSTR__UIO_MAXDATA / base->conf->buf_sz) / 2))
    min = ((VSTR__UIO_MAXDATA / base->conf->buf_sz) / 2);
  
  if (!base->cache_available)
    return (vstr__sc_read_slow_len_fd(base, pos, fd,
                                      min * base->conf->buf_sz, err));

  /* use iovec internal add -- much quicker, one syscall no double copy  */
  if (!vstr_add_iovec_buf_beg(base, pos, min, max, &iovs, &num))
  {
    *err = VSTR_TYPE_SC_READ_FD_ERR_MEM;
    errno = ENOMEM;
    return (FALSE);
  }

  return (vstr__sc_read_fast_iov_fd(base, pos, fd, iovs, num, err));
  
 inval_args:
  *err = VSTR_TYPE_SC_READ_FD_ERR_READ_ERRNO;
  errno = EINVAL;
  return (FALSE);
}

#define IOV_SZ(iovs, num) \
 (iovs[0].iov_len + ((num > 1) ? iovs[num - 1].iov_len : 0) + \
                    ((num > 2) ? ((num - 2) * base->conf->buf_sz) : 0))

static int vstr__sc_read_len_fd(Vstr_base *base, size_t pos, int fd,
                                size_t len, unsigned int *err)
{
  struct iovec *iovs = NULL;
  unsigned int num = 0;
  unsigned int ios = 0;

  if (!base->cache_available)
    return (vstr__sc_read_slow_len_fd(base, pos, fd, len, err));

  /* guess at 2 over size, as we usually lose some in the division
   * and we can lose a lot on iovs[0] */
  ios = len / base->conf->buf_sz;
  ios += 2;
  if (!vstr_add_iovec_buf_beg(base, pos, ios, ios, &iovs, &num))
  {
    *err = VSTR_TYPE_SC_READ_FD_ERR_MEM;
    errno = ENOMEM;
    return (FALSE);
  }

  /* fixup iovs for exact size of len, if bigger */
  assert(num && ((num == 1) || (iovs[num - 1].iov_len == base->conf->buf_sz)));

  assert(IOV_SZ(iovs, num) >= len);
  while (IOV_SZ(iovs, num) >  len)
  {
    size_t tmp = 0;

    tmp = (IOV_SZ(iovs, num) -  len);
    if (tmp >= iovs[num - 1].iov_len)
    {
      --num;
      continue;
    }

    iovs[num - 1].iov_len  -= tmp;
    assert(IOV_SZ(iovs, num) == len);
  }
  assert(IOV_SZ(iovs, num) == len);

  return (vstr__sc_read_fast_iov_fd(base, pos, fd, iovs, num, err));
}
#undef IOV_SZ

int vstr_sc_read_len_fd(Vstr_base *base, size_t pos, int fd,
                        size_t len, unsigned int *err)
{
  unsigned int dummy_err;
  const size_t off = 0;

  if (!err)
    err = &dummy_err;
  *err = 0;

  ASSERT_GOTO(base && (pos <= base->len), inval_args);
  
  if (!vstr__sc_get_size(base->len, fd, &len, off, err,
                         VSTR_TYPE_SC_READ_FD_ERR_FSTAT_ERRNO,
                         VSTR_TYPE_SC_READ_FD_ERR_TOO_LARGE))
    return (FALSE);

  return (vstr__sc_read_len_fd(base, pos, fd, len, err));
  
 inval_args:
  *err = VSTR_TYPE_SC_READ_FD_ERR_READ_ERRNO;
  errno = EINVAL;
  return (FALSE);
}

int vstr_sc_read_iov_file(Vstr_base *base, size_t pos,
                          const char *filename, VSTR_AUTOCONF_off64_t off,
                          unsigned int min, unsigned int max,
                          unsigned int *err)
{
  int fd = -1;
  unsigned int dummy_err;
  int ret = 0;
  int saved_errno = 0;

  if (!err)
    err = &dummy_err;
  *err = 0;
  
  ASSERT_GOTO(base && (pos <= base->len), inval_args);

  if ((fd = VSTR__POSIX_OPEN1(filename)) == -1)
  {
    *err = VSTR_TYPE_SC_READ_FILE_ERR_OPEN_ERRNO;
    return (FALSE);
  }

  if (off && (lseek64(fd, off, SEEK_SET) == -1))
    *err = VSTR_TYPE_SC_READ_FILE_ERR_SEEK_ERRNO;

  if (!*err)
  {
    size_t orig_pos = pos;
    size_t orig_len = base->len;

    while (!*err && min)
    {
      unsigned int num = base->num;
      size_t tmp = base->len;
      
      ret = vstr_sc_read_iov_fd(base, pos, fd, min, max, err);
      
      num = base->num - num;
      if (num > min) num = min;
      
      min -= num;
      max -= num;
      pos += base->len - tmp;
    }
    if (*err) /* shouldn't change errno */
    {
      ASSERT((saved_errno =  errno));
      vstr_del(base, orig_pos, base->len - orig_len);
      ASSERT( saved_errno == errno);
    }
  }

  if (*err)
    saved_errno = errno;

  if ((VSTR__POSIX_CLOSE(fd) == -1) && !*err)
  {
    *err = VSTR_TYPE_SC_READ_FILE_ERR_CLOSE_ERRNO;
    return (FALSE);
  }
  
  if (*err)
    errno = saved_errno;

  return (ret);
  
 inval_args:
  *err = VSTR_TYPE_SC_READ_FD_ERR_READ_ERRNO;
  errno = EINVAL;
  return (FALSE);
}

int vstr_sc_read_len_file(Vstr_base *base, size_t pos,
                          const char *filename,
                          VSTR_AUTOCONF_off64_t off, size_t len,
                          unsigned int *err)
{
  int fd = -1;
  unsigned int dummy_err;
  int ret = FALSE;
  int saved_errno = 0;

  if (!err)
    err = &dummy_err;
  *err = 0;

  ASSERT_GOTO(base && (pos <= base->len), inval_args);
  
  if ((fd = VSTR__POSIX_OPEN1(filename)) == -1)
  {
    *err = VSTR_TYPE_SC_READ_FILE_ERR_OPEN_ERRNO;
    return (FALSE);
  }

  if (vstr__sc_get_size(base->len, fd, &len, off, err,
                        VSTR_TYPE_SC_READ_FILE_ERR_FSTAT_ERRNO,
                        VSTR_TYPE_SC_READ_FILE_ERR_TOO_LARGE))
  {
    if (off && (lseek64(fd, off, SEEK_SET) == -1))
      *err = VSTR_TYPE_SC_READ_FILE_ERR_SEEK_ERRNO;
  }

  if (!*err)
  {
    size_t orig_pos = pos;
    size_t orig_len = base->len;
    
    while (!*err && len)
    {
      size_t tmp = base->len;
      
      ret = vstr__sc_read_len_fd(base, pos, fd, len, err);
      len -= base->len - tmp;
      pos += base->len - tmp;
    }
    
    if (*err)
    {
      ASSERT((saved_errno =  errno));
      vstr_del(base, orig_pos, base->len - orig_len);
      ASSERT( saved_errno == errno);
    }
  }
  
  if (*err)
    saved_errno = errno;

  if ((VSTR__POSIX_CLOSE(fd) == -1) && !*err)
  {
    *err = VSTR_TYPE_SC_READ_FILE_ERR_CLOSE_ERRNO;
    return (FALSE);
  }
  
  if (*err)
    errno = saved_errno;

  return (ret);
  
 inval_args:
  *err = VSTR_TYPE_SC_READ_FD_ERR_READ_ERRNO;
  errno = EINVAL;
  return (FALSE);
}

int vstr_sc_write_fd(Vstr_base *base, size_t pos, size_t len, int fd,
                     unsigned int *err)
{
  unsigned int dummy_err;

  if (!err)
    err = &dummy_err;
  *err = 0;

  ASSERT_GOTO(base && pos && (((pos <= base->len) &&
                               (vstr_sc_poslast(pos, len) <= base->len)) ||
                              !len), inval_args);
  
  if (!len)
    return (TRUE);

  while (len)
  {
    struct iovec cpy_vec[VSTR__SC_VEC_SZ];
    size_t sz = VSTR__SC_VEC_SZ;
    struct iovec *vec;
    unsigned int num = 0;
    size_t  clen  = 0;
    ssize_t bytes = 0;

    if ((pos == 1) && (len == base->len) && base->cache_available)
    {
      if (!(clen = vstr_export_iovec_ptr_all(base, &vec, &num)))
      {
        *err = VSTR_TYPE_SC_WRITE_FD_ERR_MEM;
        errno = ENOMEM;
        return (FALSE);
      }
    }
    else
    { /* could opt. anything with cache_avail ... but probably not worth it */
      vec = cpy_vec;
      clen = vstr_export_iovec_cpy_ptr(base, pos, len, vec, sz, &num);
    }

    if (num > UIO_MAXIOV)
    {
      unsigned int scan = num;

      while (scan-- > UIO_MAXIOV)
        clen -= vec[scan].iov_len;
      num = UIO_MAXIOV;
    }

    /* cap so we don't get EINVAL */
    if (clen > VSTR__UIO_MAXDATA)
    {
      unsigned int scan = num;

      ASSERT(VSTR_MAX_NODE_ALL <= VSTR__UIO_MAXDATA);
      
      while (clen > VSTR__UIO_MAXDATA)
      {
        ASSERT(scan >= 1);
        clen -= vec[--scan].iov_len;
      }

      ASSERT(clen && scan);
      
      num = scan;
    }
    
    do
    {
      bytes = writev(fd, vec, num);
    } while ((bytes == -1) && (errno == EINTR));

    if (bytes == -1)
    {
      *err = VSTR_TYPE_SC_WRITE_FD_ERR_WRITE_ERRNO;
      return (FALSE);
    }

    ASSERT((size_t)bytes <= len);

    vstr_del(base, pos, bytes);
    
    ASSERT((size_t)bytes <= clen);
    if (clen != (size_t)bytes)
      break;
    
    len -= bytes;
  }

  return (TRUE);

 inval_args:
  *err = VSTR_TYPE_SC_WRITE_FD_ERR_WRITE_ERRNO;
  errno = EINVAL;
  return (FALSE);
}

int vstr_sc_write_file(Vstr_base *base, size_t pos, size_t len,
                       const char *filename, int open_flags,
                       VSTR_AUTOCONF_mode_t mode,
                       VSTR_AUTOCONF_off64_t off, unsigned int *err)
{
  int fd = -1;
  unsigned int dummy_err;
  int ret = 0;
  int saved_errno = 0;

  if (!err)
    err = &dummy_err;
  *err = 0;
  
  ASSERT_GOTO(base && pos && (((pos <= base->len) &&
                               (vstr_sc_poslast(pos, len) <= base->len)) ||
                              !len), inval_args);
  
  if (!len)
    return (TRUE);

  if (!open_flags) /* O_RDONLY isn't valid, for obvious reasons */
    open_flags = (O_WRONLY | O_CREAT | O_EXCL | O_NONBLOCK);

  if ((fd = VSTR__POSIX_OPEN3(filename, open_flags, mode)) == -1)
  {
    *err = VSTR_TYPE_SC_WRITE_FILE_ERR_OPEN_ERRNO;
    return (FALSE);
  }

  if (off && (lseek64(fd, off, SEEK_SET) == -1))
    *err = VSTR_TYPE_SC_WRITE_FILE_ERR_SEEK_ERRNO;

  while (!*err && len)
  {
    size_t tmp = base->len;
    
    ret = vstr_sc_write_fd(base, pos, len, fd, err);
    len -= tmp - base->len;
  }

  if (*err)
    saved_errno = errno;

  if ((VSTR__POSIX_CLOSE(fd) == -1) && !*err)
  {
    *err = VSTR_TYPE_SC_WRITE_FILE_ERR_CLOSE_ERRNO;
    return (FALSE);
  }
  
  if (*err)
    errno = saved_errno;

  return (ret);
  
 inval_args:
  *err = VSTR_TYPE_SC_WRITE_FD_ERR_WRITE_ERRNO;
  errno = EINVAL;
  return (FALSE);
}

static int vstr__sc_fmt_add_cb_ipv4_ptr(Vstr_base *base, size_t pos,
                                        Vstr_fmt_spec *spec)
{
  struct in_addr *ipv4 = VSTR_FMT_CB_ARG_PTR(spec, 0);
  size_t obj_len = 0;
  char buf[1024];
  const char *ptr = NULL;

  assert(ipv4);
  assert(sizeof(buf) >= INET_ADDRSTRLEN);

  ptr = inet_ntop(AF_INET, ipv4, buf, sizeof(buf));
  if (!ptr) ptr = "0.0.0.0";

  obj_len = strlen(ptr);

  if (!vstr_sc_fmt_cb_beg(base, &pos, spec, &obj_len,
                          VSTR_FLAG_SC_FMT_CB_BEG_OBJ_STR))
    return (FALSE);

  if (!vstr_add_buf(base, pos, ptr, obj_len))
    return (FALSE);

  if (!vstr_sc_fmt_cb_end(base, pos, spec, obj_len))
    return (FALSE);

  return (TRUE);
}

static int vstr__sc_fmt_add_cb_ipv6_ptr(Vstr_base *base, size_t pos,
                                        Vstr_fmt_spec *spec)
{
  struct in6_addr *ipv6 = VSTR_FMT_CB_ARG_PTR(spec, 0);
  size_t obj_len = 0;
  char buf[1024];
  const char *ptr = NULL;

  assert(ipv6);
  assert(sizeof(buf) >= INET6_ADDRSTRLEN);

  ptr = inet_ntop(AF_INET6, ipv6, buf, sizeof(buf));
  if (!ptr) ptr = "::";

  obj_len = strlen(ptr);

  if (!vstr_sc_fmt_cb_beg(base, &pos, spec, &obj_len,
                          VSTR_FLAG_SC_FMT_CB_BEG_OBJ_STR))
    return (FALSE);

  if (!vstr_add_buf(base, pos, ptr, obj_len))
    return (FALSE);

  if (!vstr_sc_fmt_cb_end(base, pos, spec, obj_len))
    return (FALSE);

  return (TRUE);
}

int vstr_sc_fmt_add_ipv4_ptr(Vstr_conf *conf, const char *name)
{
  return (vstr_fmt_add(conf, name, vstr__sc_fmt_add_cb_ipv4_ptr,
                       VSTR_TYPE_FMT_PTR_VOID,
                       VSTR_TYPE_FMT_END));
}

int vstr_sc_fmt_add_ipv6_ptr(Vstr_conf *conf, const char *name)
{
  return (vstr_fmt_add(conf, name, vstr__sc_fmt_add_cb_ipv6_ptr,
                       VSTR_TYPE_FMT_PTR_VOID,
                       VSTR_TYPE_FMT_END));
}

#define VSTR__SC_FMT_ADD(x, n, nchk)                                    \
    if (ret &&                                                          \
        !VSTR_SC_FMT_ADD(conf, vstr_sc_fmt_add_ ## x, "{" n, nchk, "}")) \
      ret = FALSE

int vstr__sc_fmt_add_posix(Vstr_conf *conf)
{
  int ret = TRUE;
  
  VSTR__SC_FMT_ADD(ipv4_ptr, "ipv4.p", "p");
  VSTR__SC_FMT_ADD(ipv6_ptr, "ipv6.p", "p");

  return (ret);
}
#include "internal_syms_generated/vstr-cpp-symbols_rev.h"
VSTR__SYM_ALIAS(sc_fmt_add_ipv4_ptr);
VSTR__SYM_ALIAS(sc_fmt_add_ipv6_ptr);
VSTR__SYM_ALIAS(sc_mmap_fd);
VSTR__SYM_ALIAS(sc_mmap_file);
VSTR__SYM_ALIAS(sc_read_iov_fd);
VSTR__SYM_ALIAS(sc_read_iov_file);
VSTR__SYM_ALIAS(sc_read_len_fd);
VSTR__SYM_ALIAS(sc_read_len_file);
VSTR__SYM_ALIAS(sc_write_fd);
VSTR__SYM_ALIAS(sc_write_file);
