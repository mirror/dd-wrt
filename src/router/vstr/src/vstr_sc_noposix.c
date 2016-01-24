#define VSTR_SC_NOPOSIX_C
/*
 *  Copyright (C) 2002, 2003, 2004  James Antill
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
/* STUBS for - functions which are POSIX shortcuts */
#include "main.h"

# define VSTR__SC_ENOSYS(x) \
  if (err) \
  { \
    errno = ENOSYS; \
    *err = (x); \
  } \
  \
  return (FALSE)

int vstr_sc_mmap_fd(Vstr_base *VSTR__ATTR_UNUSED(base),
                    size_t VSTR__ATTR_UNUSED(pos),
                    int VSTR__ATTR_UNUSED(fd),
                    VSTR_AUTOCONF_off64_t VSTR__ATTR_UNUSED(off),
                    size_t VSTR__ATTR_UNUSED(len),
                    unsigned int *err)
{ VSTR__SC_ENOSYS(VSTR_TYPE_SC_MMAP_FD_ERR_MMAP_ERRNO); }

int vstr_sc_mmap_file(Vstr_base *VSTR__ATTR_UNUSED(base),
                      size_t VSTR__ATTR_UNUSED(pos),
                      const char *VSTR__ATTR_UNUSED(filename),
                      VSTR_AUTOCONF_off64_t VSTR__ATTR_UNUSED(off),
                      size_t VSTR__ATTR_UNUSED(len),
                      unsigned int *err)
{ VSTR__SC_ENOSYS(VSTR_TYPE_SC_MMAP_FD_ERR_MMAP_ERRNO); }

int vstr_sc_read_iov_fd(Vstr_base *VSTR__ATTR_UNUSED(base),
                        size_t VSTR__ATTR_UNUSED(pos),
                        int VSTR__ATTR_UNUSED(fd),
                        unsigned int VSTR__ATTR_UNUSED(min),
                        unsigned int VSTR__ATTR_UNUSED(max),
                        unsigned int *err)
{ VSTR__SC_ENOSYS(VSTR_TYPE_SC_READ_FD_ERR_READ_ERRNO); }

int vstr_sc_read_len_fd(Vstr_base *VSTR__ATTR_UNUSED(base),
                        size_t VSTR__ATTR_UNUSED(pos),
                        int VSTR__ATTR_UNUSED(fd),
                        size_t VSTR__ATTR_UNUSED(len),
                        unsigned int *err)
{ VSTR__SC_ENOSYS(VSTR_TYPE_SC_READ_FD_ERR_READ_ERRNO); }

int vstr_sc_read_iov_file(Vstr_base *VSTR__ATTR_UNUSED(base),
                          size_t VSTR__ATTR_UNUSED(pos),
                          const char *VSTR__ATTR_UNUSED(filename),
                          VSTR_AUTOCONF_off64_t VSTR__ATTR_UNUSED(off),
                          unsigned int VSTR__ATTR_UNUSED(min),
                          unsigned int VSTR__ATTR_UNUSED(max),
                          unsigned int *err)
{ VSTR__SC_ENOSYS(VSTR_TYPE_SC_READ_FILE_ERR_READ_ERRNO); }

int vstr_sc_read_len_file(Vstr_base *VSTR__ATTR_UNUSED(base),
                          size_t VSTR__ATTR_UNUSED(pos),
                          const char *VSTR__ATTR_UNUSED(filename),
                          VSTR_AUTOCONF_off64_t VSTR__ATTR_UNUSED(off),
                          size_t VSTR__ATTR_UNUSED(len),
                          unsigned int *err)
{ VSTR__SC_ENOSYS(VSTR_TYPE_SC_READ_FILE_ERR_READ_ERRNO); }

int vstr_sc_write_fd(Vstr_base *VSTR__ATTR_UNUSED(base),
                     size_t VSTR__ATTR_UNUSED(pos),
                     size_t VSTR__ATTR_UNUSED(len),
                     int VSTR__ATTR_UNUSED(fd),
                     unsigned int *err)
{ VSTR__SC_ENOSYS(VSTR_TYPE_SC_WRITE_FD_ERR_WRITE_ERRNO); }

int vstr_sc_write_file(Vstr_base *VSTR__ATTR_UNUSED(base),
                       size_t VSTR__ATTR_UNUSED(pos),
                       size_t VSTR__ATTR_UNUSED(len),
                       const char *VSTR__ATTR_UNUSED(filename),
                       int VSTR__ATTR_UNUSED(open_flags),
                       VSTR_AUTOCONF_mode_t VSTR__ATTR_UNUSED(mode),
                       VSTR_AUTOCONF_off64_t VSTR__ATTR_UNUSED(off),
                       unsigned int *err)
{ VSTR__SC_ENOSYS(VSTR_TYPE_SC_WRITE_FD_ERR_WRITE_ERRNO); }

int vstr_sc_fmt_add_ipv4_ptr(Vstr_conf *VSTR__ATTR_UNUSED(conf),
                             const char *VSTR__ATTR_UNUSED(name))
{
  return (FALSE);
}

int vstr_sc_fmt_add_ipv6_ptr(Vstr_conf *VSTR__ATTR_UNUSED(conf),
                             const char *VSTR__ATTR_UNUSED(name))
{
  return (FALSE);
}

int vstr__sc_fmt_add_posix(Vstr_conf *VSTR__ATTR_UNUSED(conf))
{
  return (TRUE);
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
