//==========================================================================
//
//      profile.c
//
//      Application profiling support
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2002 Gary Thomas
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Gary Thomas
// Contributors: 
// Date:         2002-11-14
// Purpose:      Application profiling support
// Description:  
//
//####DESCRIPTIONEND####
//
//===========================================================================
#include <pkgconf/profile_gprof.h>

#include <stdlib.h>
#include <string.h>
#include <cyg/infra/diag.h>
#include <network.h>
#include <tftp_support.h>
#include <cyg/profile/profile.h>
#include <cyg/profile/gmon_out.h>

static int num_buckets;
static unsigned short *profile;
static int bucket_size, bucket_shift;
static unsigned long start_addr, end_addr;
static int enabled;
static int tftp_server_id;
static int prof_rate;

static int profile_open(const char *, int);
static int profile_close(int);
static int profile_write(int, const void *, int);
static int profile_read(int, void *, int);

struct tftpd_fileops profile_fileops = {
    profile_open, profile_close, profile_write, profile_read
};

struct _file {
    unsigned char      *pos, *eof, *data;
    int                 flags;
    int                 mode;
};
#define FILE_OPEN 0x0001

#define NUM_FILES 1
static struct _file files[NUM_FILES];

static inline struct _file *
profile_fp(int fd)
{
    struct _file *fp;
    if ((fd < 0) || (fd >= NUM_FILES)) return (struct _file *)0;
    fp = &files[fd];
    if (!(fp->flags & FILE_OPEN)) return (struct _file *)0;
    return fp;
}

static int
profile_open(const char *fn, int flags)
{
    int fd = 0;
    struct _file *fp;
    int len = sizeof(struct gmon_hdr) + sizeof(struct gmon_hist_hdr) + (num_buckets*2) + 1;
    struct gmon_hdr _hdr;
    struct gmon_hist_hdr _hist_hdr;
    unsigned char *bp;
    int version = GMON_VERSION;
    int hist_size = num_buckets;
    int file_type = -1;

    fp = files;
    if (fp->flags & FILE_OPEN) {
        // File already open
        return -1;
    }
    if (!(flags & O_RDONLY)) {
        // Only read supported
        return -1;
    }
    if (strcmp(fn, "PROFILE.DAT") == 0) {
        file_type = 0;
    }
    if (file_type < 0) {
        // No supported file
        return -1;
    }
    // Set up file info
    enabled = 0;
    switch (file_type) { // In case another type ever supported
    case 0: // profile data
        fp->data = malloc(len);
        if (!fp->data) {
            diag_printf("Can't allocate buffer for profile data!\n");
            return -1;
        }
        fp->flags = FILE_OPEN;
        fp->mode = flags;
        fp->pos = fp->data;
        fp->eof = fp->pos + len;
        // Fill in profile data
        bp = fp->data;
        memset(&_hdr, 0, sizeof(_hdr));
        strcpy(_hdr.cookie, GMON_MAGIC);
        memcpy(_hdr.version, &version, sizeof(version));
        memcpy(bp, &_hdr, sizeof(_hdr));
        bp += sizeof(_hdr);
        memcpy(&_hist_hdr.low_pc, &start_addr, sizeof(start_addr));
        memcpy(&_hist_hdr.high_pc, &end_addr, sizeof(end_addr));    
        memcpy(&_hist_hdr.hist_size, &hist_size, sizeof(hist_size));    
        memcpy(&_hist_hdr.prof_rate, &prof_rate, sizeof(prof_rate));    
        strcpy(_hist_hdr.dimen, "seconds");
        _hist_hdr.dimen_abbrev = 's';
        *bp++ = GMON_TAG_TIME_HIST;
        memcpy(bp, &_hist_hdr, sizeof(_hist_hdr));
        bp += sizeof(_hist_hdr);
        memcpy(bp, profile, (num_buckets*2));
        memset(profile, 0, (num_buckets*2));
        break;    
    }
    return fd;
}

static int
profile_close(int fd)
{
    struct _file *fp = profile_fp(fd);
    if (!fp) return -1;
    if (fp->data) free(fp->data);
    fp->flags = 0;  // No longer open
    enabled = 1;
    return 0;
}

static int 
profile_write(int fd, const void *buf, int len)
{
    return -1;
}

static int
profile_read(int fd, void *buf, int len)
{
    struct _file *fp = profile_fp(fd);
    int res;
    if (!fp) return -1;
    res = fp->eof - fp->pos;  // Number of bytes left in "file"
    if (res > len) res = len;
    if (res <= 0) return 0;  // End of file
    bcopy(fp->pos, buf, res);
    fp->pos += res;
    return res;
}

void
__profile_hit(unsigned long pc)
{
    int bucket;
    if (enabled) {
        if ((pc >= start_addr) && (pc <= end_addr)) {
            bucket = (pc - start_addr) >> bucket_shift;
            if (profile[bucket] < (unsigned short)0xFFFF) {
                profile[bucket]++;
            }
        }
    }
}

void 
profile_on(void *_start, void *_end, int _bucket_size, int resolution)
{    
    start_addr = (unsigned long)_start;
    end_addr = (unsigned long)_end;
    // Adjust bucket size to be a power of 2
    bucket_size = 1;
    bucket_shift = 0;
    while (bucket_size < _bucket_size) {
        bucket_size <<= 1;
        bucket_shift++;
    }
    if (bucket_size != _bucket_size) {
        bucket_size <<= 1;
        bucket_shift++;
    }
    // Calculate number of buckets
    num_buckets = ((end_addr - start_addr) + (bucket_size - 1)) / bucket_size;
    // Adjust end address so calculations come out
    end_addr = start_addr + (bucket_size * num_buckets);
    prof_rate = 1000000 / resolution;
    // Allocate buffer for profile data
    if (!(profile = malloc(num_buckets*2))) {
        diag_printf("Can't allocate profile buffer - ignored\n");
        return;
    }
    memset(profile, 0, (num_buckets*2));
    enabled = 1;
    diag_printf("Profile from %p..%p[%p], in %d buckets of size %d\n", 
                start_addr, end_addr, _end, num_buckets, bucket_size);

    hal_enable_profile_timer(resolution);

    // Create a TFTP server to provide the data
    tftp_server_id = tftpd_start(CYGNUM_PROFILE_TFTP_PORT, &profile_fileops);
}

// EOF profile.c
