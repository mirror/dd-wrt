//==========================================================================
//
//      fatfs_supp.c
//
//      FAT file system support functions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):           Savin Zlobec <savin@elatec.si> 
// Date:                2003-06-30
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/fs_fat.h>
#include <pkgconf/infra.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/cyg_trac.h>
#include <cyg/infra/diag.h>
#include <cyg/io/io.h>
#include <cyg/fs/fatfs.h>
#include <blib/blib.h>

#include <sys/types.h>
#include <ctype.h>

#include "fatfs.h"

//==========================================================================
// FAT defines & macros

// -------------------------------------------------------------------------
// FAT dir entry attributes macros

#define DENTRY_IS_RDONLY(_dentry_)  (S_FATFS_ISRDONLY((_dentry_)->attr))
#define DENTRY_IS_HIDDEN(_dentry_)  (S_FATFS_ISHIDDEN((_dentry_)->attr))
#define DENTRY_IS_SYSTEM(_dentry_)  (S_FATFS_ISSYSTEM((_dentry_)->attr))
#define DENTRY_IS_VOLUME(_dentry_)  (S_FATFS_ISVOLUME((_dentry_)->attr))
#define DENTRY_IS_DIR(_dentry_)     (S_FATFS_ISDIR((_dentry_)->attr))
#define DENTRY_IS_ARCHIVE(_dentry_) (S_FATFS_ISARCHIVE((_dentry_)->attr))

#define DENTRY_IS_DELETED(_dentry_) \
    (0xE5 == (cyg_uint8)((_dentry_)->name[0]))

#define DENTRY_IS_ZERO(_dentry_) \
    (0x00 == (cyg_uint8)((_dentry_)->name[0]))

// -------------------------------------------------------------------------
// FAT disk data access macros

// FIXME: support big endian machines!
   
#define GET_BYTE(_data_, _var_, _off_) \
    (_var_ = *( ((cyg_uint8 *)_data_) + (_off_) ) )

#define GET_WORD(_data_, _var_, _off_)                      \
    (_var_ = *( ((cyg_uint8 *)_data_) + (_off_) ) |         \
             *( ((cyg_uint8 *)_data_) + (_off_) + 1 ) << 8)

#define GET_DWORD(_data_, _var_, _off_)                         \
    (_var_ = *( ((cyg_uint8 *)_data_) + (_off_))             |  \
             *( ((cyg_uint8 *)_data_) + (_off_) + 1 ) << 8   |  \
             *( ((cyg_uint8 *)_data_) + (_off_) + 2 ) << 16  |  \
             *( ((cyg_uint8 *)_data_) + (_off_) + 3 ) << 24)

#define GET_BYTES(_data_, _var_, _size_, _off_) \
    memcpy((void *)(_var_), (void*)(((cyg_uint8 *)_data_)+(_off_)),_size_)

#define SET_BYTE(_data_, _val_, _off_) \
    (*( ((cyg_uint8 *)_data_) + (_off_) ) = _val_)

#define SET_WORD(_data_, _val_, _off_)                                   \
    do {                                                                 \
        *( ((cyg_uint8 *)_data_) + (_off_) )     = _val_         & 0xFF; \
        *( ((cyg_uint8 *)_data_) + (_off_) + 1 ) = (_val_ >> 8)  & 0xFF; \
    } while (0)

#define SET_DWORD(_data_, _val_, _off_)                                  \
    do {                                                                 \
        *( ((cyg_uint8 *)_data_) + (_off_) )     = _val_         & 0xFF; \
        *( ((cyg_uint8 *)_data_) + (_off_) + 1 ) = (_val_ >> 8)  & 0xFF; \
        *( ((cyg_uint8 *)_data_) + (_off_) + 2 ) = (_val_ >> 16) & 0xFF; \
        *( ((cyg_uint8 *)_data_) + (_off_) + 3 ) = (_val_ >> 24) & 0xFF; \
    } while (0)

#define SET_BYTES(_data_, _var_, _size_, _off_) \
    memcpy((void *)(((cyg_uint8 *)_data_)+(_off_)), (void *)(_var_), _size_)

// -------------------------------------------------------------------------
// FAT table entries types 

#define TENTRY_REGULAR  0 // Used when entry points to next file cluster 
#define TENTRY_FREE     1 // Free cluster
#define TENTRY_LAST     2 // Last cluster of file 
#define TENTRY_RESERVED 3 // Reserved cluster
#define TENTRY_BAD      4 // Bad cluster 

// -------------------------------------------------------------------------
// FAT table structures size 

#define DENTRY_SIZE 0x20 // Dir entry size

// -------------------------------------------------------------------------
// Time & date defines 

#define JD_1_JAN_1970 2440588 // 1 Jan 1970 in Julian day number

// -------------------------------------------------------------------------
// Code tracing defines 

#ifdef FATFS_TRACE_DIR_ENTRY
# define TDE 1
#else
# define TDE 0
#endif

#ifdef FATFS_TRACE_CLUSTER
# define TCL 1
#else
# define TCL 0
#endif

#ifdef FATFS_TRACE_DATA
# define TDO 1
#else
# define TDO 0
#endif
    
//==========================================================================
// FAT structures 

// -------------------------------------------------------------------------
// FAT table boot record structure 
   
typedef struct fat_boot_record_s
{
    cyg_uint16    jump;           // 00h : Jump code
//  cyg_uint8     jump0;          //                 + NOP
    char          oem_name[8+1];  // 03h : OEM name
    cyg_uint16    bytes_per_sec;  // 0Bh : cyg_bytes per sector
    cyg_uint8     sec_per_clu;    // 0Dh : Sectors per cluster
    cyg_uint16    res_sec_num;    // 0Eh : Number of reserved sectors
    cyg_uint8     fat_tbls_num;   // 10h : Number of copies of fat
    cyg_uint16    max_root_dents; // 11h : Maximum number of root dir entries
    cyg_uint16    sec_num_32;     // 13h : Number of sectors in partition < 32MB
    cyg_uint8     media_desc;     // 15h : Media descriptor
    cyg_uint16    sec_per_fat;    // 16h : Sectors per FAT
    cyg_uint16    sec_per_track;  // 18h : Sectors per track
    cyg_uint16    heads_num;      // 1Ah : Number of heads
    cyg_uint32    hsec_num;       // 1Ch : Number of hidden sectors
    cyg_uint32    sec_num;        // 20h : Number of sectors in partition
    cyg_uint8     exe_marker[2];  // 1FEh: Executable marker (55h AAh)

// FAT32 specific fields
 
    cyg_uint32    sec_per_fat_32; // 24h : Sectors per FAT
    cyg_uint16    ext_flags;      // 28h : Flags
    cyg_uint16    fs_ver;         // 2Ah : FS version
    cyg_uint32    root_cluster;   // 2Ch : Root dir cluster
    cyg_uint16    fs_info_sec;    // 30h : Sector number of FSINFO structure
    cyg_uint16    bk_boot_sec;    // 32h : Sector number of backup boot record
//  cyg_uint8     reserved[12];   // 34h : Reserved

// Fields with different locations on FAT12/16 and FAT32 
        
    cyg_uint8     drv_num;        // 24h (40h) : Drive number of partition 
//  cyg_uint8     reserved1;      // 25h (41h) : Reserved 1
    cyg_uint8     ext_sig;        // 26h (42h) : Extended signature
    cyg_uint32    ser_num;        // 27h (43h) : Serial number of partition
    char          vol_name[11+1]; // 2Bh (47h) : Volume name of partition
    char          fat_name[8+1];  // 36h (52h) : FAT name
   
} fat_boot_record_t;

// -------------------------------------------------------------------------
// FAT dir entry structure 
 
typedef struct fat_raw_dir_entry_s
{
    char       name[8+1];   // 00h : Name
    char       ext[3+1];    // 08h : Extension
    cyg_uint8  attr;        // 0Bh : Attribute
    cyg_uint8  nt_reserved; // 0Ch : Win NT Reserved field
    cyg_uint8  crt_sec_100; // 0Dh : Creation time ms stamp 0 - 199
    cyg_uint16 crt_time;    // 0Eh : Creation time
    cyg_uint16 crt_date;    // 10h : Creation date
    cyg_uint16 acc_date;    // 12h : Last access date
    cyg_uint16 cluster_HI;  // 14h : Starting cluster HI WORD (FAT32)
    cyg_uint16 wrt_time;    // 16h : Time    
    cyg_uint16 wrt_date;    // 18h : Date
    cyg_uint16 cluster;     // 1Ah : Starting cluster 
    cyg_uint32 size;        // 1Ch : Size of the file    
} fat_raw_dir_entry_t;

// -------------------------------------------------------------------------
// FAT cluster opts 
 
typedef enum cluster_opts_e
{
    CO_NONE       = 0x00, // NULL option
    CO_EXTEND     = 0x01, // Extend cluster chain if one cluster too short
    CO_ERASE_NEW  = 0x02, // Erase newly allocated cluster
    CO_MARK_LAST  = 0x04  // Mark  newly allocated cluster as last
} cluster_opts_t;

//==========================================================================
// Utility functions 

// -------------------------------------------------------------------------
// get_val_log2()
// Gets the log2 of given value or returns 0 if value is not a power of 2. 
 
static cyg_uint32 
get_val_log2(cyg_uint32 val)
{
    cyg_uint32 i, log2;
    
    i    = val;
    log2 = 0;
    
    while (0 == (i & 1))
    {
        i >>= 1;
        log2++;
    }

    if (i != 1) return 0;
    else        return log2;
}

// -------------------------------------------------------------------------
// cluster_to_block_pos()
// Converts cluster position to blib block position.

static void
cluster_to_block_pos(fatfs_disk_t *disk,
                     cyg_uint32    cluster,
                     cyg_uint32    cluster_pos,
                     cyg_uint32   *block, 
                     cyg_uint32   *block_pos)
{
    cyg_uint32 block_size      = cyg_blib_get_block_size(&disk->blib);
    cyg_uint32 block_size_log2 = cyg_blib_get_block_size_log2(&disk->blib);
    
    *block = (cluster - 2) << (disk->cluster_size_log2 - block_size_log2);
    
    *block_pos = disk->fat_data_pos + cluster_pos;  

    if (*block_pos > block_size)
    {
        *block     += *block_pos >> block_size_log2;
        *block_pos  = *block_pos & (block_size - 1);
    }
}

// -------------------------------------------------------------------------
// disk_write()
// Writes data to disk.

static __inline__ int
disk_write(fatfs_disk_t *disk, 
           void         *buf,
           cyg_uint32   *len,
           cyg_uint32    pos)
{
    return cyg_blib_write(&disk->blib, buf, len, 0, pos);
}

// -------------------------------------------------------------------------
// disk_read()
// Reads data from disk.

static __inline__ int
disk_read(fatfs_disk_t *disk, 
          void         *buf,
          cyg_uint32   *len,
          cyg_uint32    pos)
{
    return cyg_blib_read(&disk->blib, buf, len, 0, pos);
}

// -------------------------------------------------------------------------
// disk_cluster_write()
// Writes data to disk at specified cluster position.

static __inline__ int
disk_cluster_write(fatfs_disk_t *disk,
                   void         *buf,
                   cyg_uint32   *len,
                   cyg_uint32    cluster,
                   cyg_uint32    cluster_pos)
{
    cyg_uint32 block, block_pos;

    cluster_to_block_pos(disk, cluster, cluster_pos, &block, &block_pos);

    return cyg_blib_write(&disk->blib, buf, len, block, block_pos);
}

// -------------------------------------------------------------------------
// disk_cluster_read()
// Reads data from disk at specified cluster position.

static __inline__ int
disk_cluster_read(fatfs_disk_t *disk,
                  void         *buf,
                  cyg_uint32   *len,
                  cyg_uint32    cluster,
                  cyg_uint32    cluster_pos)
{
    cyg_uint32 block, block_pos;

    cluster_to_block_pos(disk, cluster, cluster_pos, &block, &block_pos);

    return cyg_blib_read(&disk->blib, buf, len, block, block_pos);
}

// -------------------------------------------------------------------------
// jdays_to_gdate()
// Converts juilan days into gregorian date.
 
static void
jdays_to_gdate(cyg_uint32 jd, int *day, int *month, int *year)
{
    cyg_uint32 l, n, i, j;

    l = jd + 68569;
    n = (4 * l) / 146097;
    l = l - (146097 * n + 3) / 4;
    i = (4000 * (l + 1)) / 1461001;
    l = l - (1461 * i) / 4 + 31;
    j = (80 * l) / 2447;
    *day = l - (2447 * j) / 80;

    l = j / 11;
    *month = j + 2 - (12 * l);
    *year = 100 * (n - 49) + i + l;
}

// -------------------------------------------------------------------------
// gdate_to_jdays()
// Converts gregorian date to juilan days.
 
static void
gdate_to_jdays(int day, int month, int year, cyg_uint32 *jd)
{
    *jd = day - 32075 + 1461*(year + 4800 + (month - 14)/12)/4 +
          367*(month - 2 - (month - 14)/12*12)/12 - 
          3*((year + 4900 + (month - 14)/12)/100)/4;
}
 
// -------------------------------------------------------------------------
// date_unix2dos()
// Converts unix timestamp to dos time and date. 
                 
static void
date_unix2dos(cyg_uint32  unix_timestamp, 
              cyg_uint16 *dos_time,
              cyg_uint16 *dos_date)
{
    cyg_uint32 jd;
    cyg_uint16 dtime, ddate;
    int        hour, min, sec;
    int        day, month, year;
    
    hour = (unix_timestamp / 3600) % 24;
    min  = (unix_timestamp / 60) % 60;
    sec  =  unix_timestamp % 60;

    jd = JD_1_JAN_1970 + unix_timestamp / (3600 * 24);
    jdays_to_gdate(jd, &day, &month, &year);

    CYG_TRACE7(TDE, "timestamp=%d date=%d:%d:%d %d-%d-%d",
               unix_timestamp, hour, min, sec, year, month, day);

    if (year < 1980)
        year = 1980;

    dtime = (hour << 11) | (min << 5) | (sec >> 1);
    ddate = ((year - 1980) << 9) | (month << 5) | day;
 
    CYG_TRACE2(TDE, "dos time=%d date=%d", dtime, ddate);
    
    if (NULL != dos_time) *dos_time = dtime;
    if (NULL != dos_date) *dos_date = ddate;
}

// -------------------------------------------------------------------------
// date_dos2unix()
// Converts dos time and date to unix timestamp. 
 
static void
date_dos2unix(cyg_uint16  dos_time, 
              cyg_uint16  dos_date, 
              cyg_uint32 *unix_timestamp)
{
    cyg_uint32 ts; 
    int        hour, min, sec;
    int        day, month, year;
    
    sec        = (dos_time & ((1<<5)-1)) * 2;
    dos_time >>= 5;
    min        = (dos_time & ((1<<6)-1));
    dos_time >>= 6;
    hour       = dos_time;
    
    day        = (dos_date & ((1<<5)-1));
    dos_date >>= 5;
    month      = (dos_date & ((1<<4)-1));
    dos_date >>= 4;
    year       = dos_date + 1980;

    gdate_to_jdays(day, month, year, &ts);

    ts -= JD_1_JAN_1970;
    ts  = (ts * 24 * 3600) + (sec + min * 60 + hour * 3600);
    
    *unix_timestamp = ts;

    CYG_TRACE2(TDE, "dos time=%d date=%d", dos_time, dos_date);
    CYG_TRACE7(TDE, "timestamp=%d date=%d:%d:%d %d-%d-%d",
                    ts, hour, min, sec, year, month, day);
}

//==========================================================================
// FAT boot record functions 

#if TDE 

// -------------------------------------------------------------------------
// print_boot_record()
// Prints FAT boot record.

static void
print_boot_record(fat_boot_record_t* fbr)
{
    diag_printf("FAT: FBR jump code:       0x%02X\n", fbr->jump);
    diag_printf("FAT: FBR oem name:       '%.8s'\n",  fbr->oem_name);
    diag_printf("FAT: FBR bytes per sec:   %u\n",     fbr->bytes_per_sec);
    diag_printf("FAT: FBR sec per cluster: %u\n",     fbr->sec_per_clu);
    diag_printf("FAT: FBR reserved sec:    %u\n",     fbr->res_sec_num);
    diag_printf("FAT: FBR fat tbls num:    %u\n",     fbr->fat_tbls_num);
    diag_printf("FAT: FBR max root dents:  %u\n",     fbr->max_root_dents);
    diag_printf("FAT: FBR sec num (32):    %u\n",     fbr->sec_num_32);
    diag_printf("FAT: FBR media desc:      0x%02X\n", fbr->media_desc);
    diag_printf("FAT: FBR sec per fat:     %u\n",     fbr->sec_per_fat);
    diag_printf("FAT: FBR sec per track:   %u\n",     fbr->sec_per_track);
    diag_printf("FAT: FBR heads num:       %u\n",     fbr->heads_num);
    diag_printf("FAT: FBR hidden sec num:  %u\n",     fbr->hsec_num);
    diag_printf("FAT: FBR sec num:         %u\n",     fbr->sec_num);

    if (0 == fbr->sec_per_fat)
    {
        diag_printf("FAT: FBR sec per fat32:   %u\n",     fbr->sec_per_fat_32);
        diag_printf("FAT: FBR ext flags:       0x%04X\n", fbr->ext_flags);
        diag_printf("FAT: FBR fs ver:          %u\n",     fbr->fs_ver);
        diag_printf("FAT: FBR root cluster:    %u\n",     fbr->root_cluster);
        diag_printf("FAT: FBR fs info sec:     %u\n",     fbr->fs_info_sec);
    }
    
    diag_printf("FAT: FBR drv num:         %u\n",     fbr->drv_num);
    diag_printf("FAT: FBR ext sig:         0x%02X\n", fbr->ext_sig);
    diag_printf("FAT: FBR ser num:         0x%08X\n", fbr->ser_num);
    diag_printf("FAT: FBR vol name:       '%.11s'\n", fbr->vol_name);
    diag_printf("FAT: FBR fat name:       '%.8s'\n",  fbr->fat_name);
    diag_printf("FAT: FBR exe mark:        0x%02X 0x%02X\n", 
                fbr->exe_marker[0], fbr->exe_marker[1]);
}

#endif // TDE

// -------------------------------------------------------------------------
// read_boot_record()
// Reads FAT boot record from disk.
 
static int 
read_boot_record(fatfs_disk_t *disk, fat_boot_record_t *fbr)
{
    cyg_uint8 data[0x5A];
    int       len, err;
    
    len = 0x5A;
    err = disk_read(disk, (void*)data, &len, 0);
    if (err != ENOERR)
        return err;
   
    GET_WORD(data,  fbr->jump,           0x00);
    GET_BYTES(data, fbr->oem_name, 8,    0x03);
    GET_WORD(data,  fbr->bytes_per_sec,  0x0B);
    GET_BYTE(data,  fbr->sec_per_clu,    0x0D);
    GET_WORD(data,  fbr->res_sec_num,    0x0E);
    GET_BYTE(data,  fbr->fat_tbls_num,   0x10);
    GET_WORD(data,  fbr->max_root_dents, 0x11);
    GET_WORD(data,  fbr->sec_num_32,     0x13);
    GET_BYTE(data,  fbr->media_desc,     0x15);
    GET_WORD(data,  fbr->sec_per_fat,    0x16);
    GET_WORD(data,  fbr->sec_per_track,  0x18);
    GET_WORD(data,  fbr->heads_num,      0x1A);
    GET_DWORD(data, fbr->hsec_num,       0x1C);
    GET_DWORD(data, fbr->sec_num,        0x20);

    // This is a quick check for FAT12/16 or FAT32 boot record.
    // The sec_per_fat field must be 0 on FAT32, since this
    // field plays a crucial role in detection of the FAT type 
    // (12,16,32) it is quite safe to make this assumption.
    if (0 == fbr->sec_per_fat)
    {
        GET_DWORD(data, fbr->sec_per_fat_32, 0x24);
        GET_WORD(data,  fbr->ext_flags,      0x28);
        GET_WORD(data,  fbr->fs_ver,         0x2A);
        GET_DWORD(data, fbr->root_cluster,   0x2C);
        GET_WORD(data,  fbr->fs_info_sec,    0x30);
        GET_WORD(data,  fbr->bk_boot_sec,    0x32);
        GET_BYTE(data,  fbr->drv_num,        0x40);
        GET_BYTE(data,  fbr->ext_sig,        0x42);
        GET_DWORD(data, fbr->ser_num,        0x43);
        GET_BYTES(data, fbr->vol_name, 11,   0x47);
        GET_BYTES(data, fbr->fat_name, 8,    0x52);
    }
    else
    {
        GET_BYTE(data,  fbr->drv_num,        0x24);
        GET_BYTE(data,  fbr->ext_sig,        0x26);
        GET_DWORD(data, fbr->ser_num,        0x27);
        GET_BYTES(data, fbr->vol_name, 11,   0x2B);
        GET_BYTES(data, fbr->fat_name, 8,    0x36);
    }
    
    // Read the end marker
    len = 0x02;
    err = disk_read(disk, (void*)data, &len, 0x1FE);
    if (err != ENOERR)
        return err;

    GET_BYTES(data, fbr->exe_marker, 2,  0);

    // Zero terminate strings
    fbr->oem_name[8]  = '\0';
    fbr->vol_name[11] = '\0';
    fbr->fat_name[8]  = '\0';
 
#if TDE 
    print_boot_record(fbr);
#endif
    
    return ENOERR;
}

//==========================================================================
// FAT table entry functions 

// -------------------------------------------------------------------------
// read_tentry()
// Reads FAT table entry from disk.

static int
read_tentry(fatfs_disk_t *disk, cyg_uint32 num, cyg_uint32 *entry)
{
    cyg_uint8  data[4];
    cyg_uint32 pos, num3;
    cyg_uint32 e;
    int        len, err;

    switch (disk->fat_type)
    {
        case FATFS_FAT12:
            num3 = num * 3;
            pos  = disk->fat_tbl_pos + (num3 >> 1);
            len  = 2;
    
            err = disk_read(disk, (void*)data, &len, pos);
            if (err != ENOERR)
                return err;

            GET_WORD(data, e, 0x00);

            if (0 == (num3 & 1)) *entry = e        & 0x0FFF;
            else                 *entry = (e >> 4) & 0x0FFF;

            break;
            
        case FATFS_FAT16:
            pos = disk->fat_tbl_pos + (num << 1);
            len = 2;
    
            err = disk_read(disk, (void*)data, &len, pos);
            if (err != ENOERR)
                return err;

            GET_WORD(data, e, 0x00);
            *entry = e;

            break; 
            
        case FATFS_FAT32:
            pos = disk->fat_tbl_pos + (num << 2);
            len = 4;
    
            err = disk_read(disk, (void*)data, &len, pos);
            if (err != ENOERR)
                return err;

            GET_DWORD(data, e, 0x00);
            *entry = e & 0x0FFFFFFF;

            break;

        default:
            CYG_ASSERT(false, "Unknown FAT type");
    }
    return ENOERR;
}

// -------------------------------------------------------------------------
// write_tentry()
// Writes FAT table entry to disk (to all copies of FAT).
 
static int
write_tentry(fatfs_disk_t *disk, cyg_uint32 num, cyg_uint32 *entry)
{
    cyg_uint8  data[4];
    cyg_uint32 pos=0, num3; 
    cyg_uint32 e;
    int        i, len, err;

    switch (disk->fat_type)
    {
        case FATFS_FAT12:
            num3 = num * 3;
            pos  = disk->fat_tbl_pos + (num3 >> 1);
            len  = 2;
   
            err = disk_read(disk, (void*)data, &len, pos);
            if (err != ENOERR)
                return err;

            GET_WORD(data, e, 0x00);
  
            if (0 == (num3 & 1)) e = (e & 0xF000) | (*entry & 0x0FFF);
            else                 e = (e & 0x000F) | ((*entry & 0x0FFF) << 4);
    
            SET_WORD(data, e, 0x00);

            break;

        case FATFS_FAT16:
            pos = disk->fat_tbl_pos + (num << 1);
            len = 2;
    
            e = *entry;
            SET_WORD(data, e, 0x00);

            break;  
            
        case FATFS_FAT32:
            pos = disk->fat_tbl_pos + (num << 2);
            len = 4;
    
            err = disk_read(disk, (void*)data, &len, pos);
            if (err != ENOERR)
                return err;

            GET_DWORD(data, e, 0x00);

            e = (e & 0xF0000000) | *entry;
    
            SET_DWORD(data, e, 0x00);

            break; 

        default:
            CYG_ASSERT(false, "Unknown FAT type");
    }
    
    for (i = 0; i < disk->fat_tbls_num; i++)
    {
        err = disk_write(disk, (void*)data, &len, pos);
        if (err != ENOERR)
            return err;

        pos += disk->fat_tbl_size;
    }
    
    return ENOERR;
}

// -------------------------------------------------------------------------
// get_tentry_type()
// Gets the type of FAT table entry.
 
static int
get_tentry_type(fatfs_disk_t *disk, cyg_uint32 entry)
{
    int type;

    switch (disk->fat_type)
    {
        case FATFS_FAT12:
            if (entry < 0x0FF0)
            {
                if (0x0000 == entry)  type = TENTRY_FREE;
                else                  type = TENTRY_REGULAR;
            }
            else if (entry >= 0x0FF8) type = TENTRY_LAST;
            else if (0x0FF7 == entry) type = TENTRY_BAD;
            else                      type = TENTRY_RESERVED;

            break;

        case FATFS_FAT16:
            if (entry < 0xFFF0)
            {
                if (0x0000 == entry)  type = TENTRY_FREE;
                else                  type = TENTRY_REGULAR;
            }
            else if (entry >= 0xFFF8) type = TENTRY_LAST;
            else if (0xFFF7 == entry) type = TENTRY_BAD;
            else                      type = TENTRY_RESERVED;

            break;

        case FATFS_FAT32:

            if (entry < 0x0FFFFFF0)
            {
                if (0x00000000 == entry)  type = TENTRY_FREE;
                else                      type = TENTRY_REGULAR;
            }
            else if (entry >= 0x0FFFFFF8) type = TENTRY_LAST;
            else if (0x0FFFFFF7 == entry) type = TENTRY_BAD;
            else                          type = TENTRY_RESERVED;

            break;

        default:
            CYG_ASSERT(false, "Unknown FAT type");
            type = TENTRY_BAD; // least likely to cause damage
    }
    return type;
}

// -------------------------------------------------------------------------
// set_tentry_type()
// Sets the type of FAT table entry.
 
static void 
set_tentry_type(fatfs_disk_t *disk, cyg_uint32 *entry, cyg_uint32 type)
{
    switch (disk->fat_type)
    {
        case FATFS_FAT12:
            switch (type)
            {
                case TENTRY_FREE:     *entry = 0x0000; return;
                case TENTRY_LAST:     *entry = 0x0FF8; return;
                case TENTRY_RESERVED: *entry = 0x0FF0; return;
                case TENTRY_BAD:      *entry = 0x0FF7; return;      
                default:
                    CYG_ASSERT(false, "Unknown tentry type");
            }
            break;
            
        case FATFS_FAT16:
            switch (type)
            {
                case TENTRY_FREE:     *entry = 0x0000; return;
                case TENTRY_LAST:     *entry = 0xFFF8; return;
                case TENTRY_RESERVED: *entry = 0xFFF0; return;
                case TENTRY_BAD:      *entry = 0xFFF7; return;
                default:
                    CYG_ASSERT(false, "Unknown tentry type");
            }
            break;
            
        case FATFS_FAT32:
            switch (type)
            {
                case TENTRY_FREE:     *entry = 0x00000000; return;
                case TENTRY_LAST:     *entry = 0x0FFFFFF8; return;
                case TENTRY_RESERVED: *entry = 0x0FFFFFF0; return;
                case TENTRY_BAD:      *entry = 0x0FFFFFF7; return;      
                default:
                    CYG_ASSERT(false, "Unknown tentry type");
            }
            break;

        default:
            CYG_ASSERT(false, "Unknown FAT type");
    }
}

// -------------------------------------------------------------------------
// get_tentry_next_cluster()
// Gets the the next file cluster number from FAT table entry.
 
static __inline__ cyg_uint32 
get_tentry_next_cluster(fatfs_disk_t *disk, cyg_uint32 entry)
{
    return entry;
}

// -------------------------------------------------------------------------
// set_tentry_next_cluster()
// Sets the the next cluster number to FAT table entry.
 
static __inline__ void 
set_tentry_next_cluster(fatfs_disk_t *disk, 
                        cyg_uint32   *entry, 
                        cyg_uint32    next_cluster)
{
    *entry = next_cluster;
}

//==========================================================================
// FAT cluster functions 

// -------------------------------------------------------------------------
// erase_cluster()
// Erases cluster (fills with 0x00).

static int
erase_cluster(fatfs_disk_t *disk, cyg_uint32 cluster)
{
    cyg_uint8  data[32];
    cyg_uint32 pos;
    int        err, len, i;
    
    pos = 0;
    len = 32;
    memset((void*)data, 0x00, len);
    
    CYG_TRACE1(TCL, "erasing cluster=%d", cluster);

    for (i = 0; i < (disk->cluster_size >> 5); i++)
    {
        err = disk_cluster_write(disk, (void*)data, &len, cluster, pos);
        if (err != ENOERR)
            return err;

        pos += len;
    }
    
    return ENOERR;
}

// -------------------------------------------------------------------------
// mark_cluster()
// Marks cluster (sets the cluster's FAT table entry to given type). 

static int
mark_cluster(fatfs_disk_t *disk, cyg_uint32 cluster, cyg_uint32 type)
{
    cyg_uint32 tentry;
 
    set_tentry_type(disk, &tentry, type);
    return write_tentry(disk, cluster, &tentry);
}

// -------------------------------------------------------------------------
// link_cluster()
// Links two clusters.

static int
link_cluster(fatfs_disk_t *disk, cyg_uint32 cluster1, cyg_uint32 cluster2)
{
    cyg_uint32 tentry;
    
    set_tentry_next_cluster(disk, &tentry, cluster2);
    return write_tentry(disk, cluster1, &tentry);
}

// -------------------------------------------------------------------------
// find_next_free_cluster()
// Finds first free cluster starting from given cluster.
// If none is available free_cluster is set to 0.
// If CO_MARK_LAST is set in opts the found cluster is marked as LAST.
// If CO_ERASE_NEW is set in opts the found cluster is erased.

static int
find_next_free_cluster(fatfs_disk_t   *disk,
                       cyg_uint32      start_cluster, 
                       cyg_uint32     *free_cluster,
                       cluster_opts_t  opts)
{
    cyg_uint32 c, tentry;
    int        err;

    if (start_cluster < 2) c = 2;
    else                   c = start_cluster + 1;

    *free_cluster = 0;

    CYG_TRACE1(TCL, "starting at cluster=%d", c);
   
    // Search from the starting cluster to the end of FAT and
    // from start of FAT to the starting cluster
    while (c != start_cluster)
    {
        // Check for end of FAT
        if (c >= disk->fat_tbl_nents)
        {
            c = 2;
            if (c >= start_cluster)
                break;
        }

        err = read_tentry(disk, c, &tentry);
        if (err != ENOERR)
            return err;

        if (TENTRY_FREE == get_tentry_type(disk, tentry))
        {
            CYG_TRACE1(TCL, "found free cluster=%d", c);
            
            *free_cluster = c;
            
            if (opts & CO_MARK_LAST)
                err = mark_cluster(disk, c, TENTRY_LAST);
            if ((err == ENOERR) && (opts & CO_ERASE_NEW))
                err = erase_cluster(disk, c);
            
            return err;
        }
        c++;
    }   

    // No free clusters found

    CYG_TRACE0(TCL, "!!! no free clusters found");
 
    return ENOSPC;
}

// -------------------------------------------------------------------------
// find_and_append_cluster()
// Finds a free cluster on disk and appends it to the given cluster. 
// New cluster is marked as LAST. 

static int
find_and_append_cluster(fatfs_disk_t   *disk, 
                        cyg_uint32      cluster, 
                        cyg_uint32     *new_cluster,
                        cluster_opts_t  opts)
{
    cyg_uint32 free_cluster;
    int        err;

    err = find_next_free_cluster(disk, cluster, 
        &free_cluster, opts | CO_MARK_LAST);
    if (err != ENOERR)
        return err;

    err = link_cluster(disk, cluster, free_cluster);
    if (err != ENOERR)
        return err;

    *new_cluster = free_cluster;

    CYG_TRACE2(TCL, "appending new cluster=%d to cluster=%d", 
                    free_cluster, cluster);
    
    return ENOERR;
}

// -------------------------------------------------------------------------
// find_nth_cluster0()
// Finds nth cluster in chain (ie nth cluster of file) searching
// from given position. The result is returned by the same position
// variable. 
 
static int
find_nth_cluster0(fatfs_disk_t     *disk,
                  fatfs_data_pos_t *pos, 
                  cyg_uint32        n)
{
    cyg_uint32 cluster, cluster_snum;
    int        err = ENOERR;
 
    if (pos->cluster_snum == n)
        return ENOERR;

    cluster      = pos->cluster;
    cluster_snum = pos->cluster_snum;
   
    CYG_TRACE4(TCL, "cluster=%d snum=%d n=%d n_to_search=%d",
                    cluster, cluster_snum, n, n-cluster_snum);
   
    // Adjust the number of clusters that should be
    // walked according to the given position
    n -= cluster_snum;

    // Walk the cluster chain for n clusters or until last cluster
    while (n > 0)
    {
        cyg_uint32 tentry;

        err = read_tentry(disk, cluster, &tentry);
        if (err != ENOERR)
            return err;

        switch (get_tentry_type(disk, tentry))
        {
            case TENTRY_REGULAR:
                break;
            case TENTRY_LAST:
                CYG_TRACE1(TCL, "chain end at n=%d", n);
                err = EEOF; // File has less clusters than given n
                            // this err should be caught by the 
                            // calling function 
                goto out;
            default:
                // Inconsistant FAT table state !!!
                CYG_TRACE2(TCL, "!!! inconsistant FAT tentry=%x n=%d", 
                                tentry, n);
                err = EIO;                 
                goto out;
        }
        cluster = get_tentry_next_cluster(disk, tentry);
        cluster_snum++;
        n--;
    }
    
out:
    pos->cluster      = cluster;
    pos->cluster_snum = cluster_snum;

    CYG_TRACE2(TCL, "nth cluster=%d snum=%d", cluster, cluster_snum);

    return err;
}

// -------------------------------------------------------------------------
// find_nth_cluster()
// Finds nth cluster in chain (ie nth cluster of file) searching 
// from given position. The result is returned by the same position
// variable. If the chain ends one cluster before the given nth cluster 
// and the CO_EXTEND is specified, than the chain is extended by one cluster.
 
static int
find_nth_cluster(fatfs_disk_t     *disk,
                 fatfs_data_pos_t *pos, 
                 cyg_uint32        n,
                 cluster_opts_t    opts)
{
    int err;
   
    // Find nth cluster 
    err = find_nth_cluster0(disk, pos, n);    

    // EEOF meens that the cluster chain ended early
    if ((err != EEOF) || !(opts & CO_EXTEND))
        return err;
    
    // Check if one cluster short
    if (pos->cluster_snum == (n - 1))
    {
        // Extend the chain for one cluster
        cyg_uint32 new_cluster;

        // Append new cluster to the end of chain
        err = find_and_append_cluster(disk, pos->cluster, &new_cluster, opts);
        if (err != ENOERR)
            return err;

        // Update position
        pos->cluster       = new_cluster;
        pos->cluster_snum += 1;
        pos->cluster_pos   = 0;

        CYG_TRACE1(TCL, "appended new cluster=%d", new_cluster);
    }
    
    return err;
}

// -------------------------------------------------------------------------
// get_next_cluster()
// Gets next cluster in chain (ie next cluster of file).
// If CO_EXTEND is specified and the current cluster is last in the 
// chain then the chain is extended by one cluster.

static int
get_next_cluster(fatfs_disk_t     *disk,
                 fatfs_data_pos_t *pos,
                 cluster_opts_t    opts)
{
    int err;

    err = find_nth_cluster(disk, pos, pos->cluster_snum + 1, opts);
    if (err != ENOERR)
        return err;

    // Reset inside cluster position
    pos->cluster_pos = 0;
 
    return ENOERR;
}

// -------------------------------------------------------------------------
// get_position_from_off()
// Gets position from given offset. The search is started from the
// given position and the result is returned by the same variable. 
// If CO_EXTEND is specified the file is extended if one cluster too short.

static int
get_position_from_off(fatfs_disk_t     *disk,
                      cyg_uint32        first_cluster,
                      cyg_uint32        offset,
                      fatfs_data_pos_t *pos,
                      cluster_opts_t    opts)
{
    fatfs_data_pos_t new_pos;
    cyg_uint32       n;
    int              err;

    // Position inside the cluster
    new_pos.cluster_pos = offset & (disk->cluster_size - 1);

    // Cluster seq number to be searched for
    n = offset >> disk->cluster_size_log2;

    if (n < pos->cluster_snum)
    { 
        // Start searching from first cluster
        new_pos.cluster      = first_cluster;
        new_pos.cluster_snum = 0;
    }
    else
    {
        // Start searching from the current position
        new_pos.cluster      = pos->cluster;
        new_pos.cluster_snum = pos->cluster_snum;
    }

    err = find_nth_cluster(disk, &new_pos, n, opts);
    
    // Err could be EEOF wich means that the given 
    // offset if out of given file (cluster chain)

    if (ENOERR == err)
        *pos = new_pos; 
    
    return err;
} 

// -------------------------------------------------------------------------
// free_cluster_chain()
// Marks all clusters FREE from given cluster to the last cluster in chain.

static int
free_cluster_chain(fatfs_disk_t *disk, cyg_uint32 start_cluster)
{
    cyg_uint32 c, next_c, tentry;
    bool       last;
    int        err;

    CYG_TRACE1(TCL, "start cluster=%d", start_cluster);

    c = next_c = start_cluster;
    last = false;
    while (!last)
    {
        err = read_tentry(disk, c, &tentry);
        if (err != ENOERR)
            return err;

        switch (get_tentry_type(disk, tentry))
        {
            case TENTRY_LAST:
                // Last cluster in chain
                last = true;
                break;
            case TENTRY_REGULAR:
                // Get next cluster in chain
                next_c = get_tentry_next_cluster(disk, tentry);
                break;
            default:
                CYG_TRACE2(TCL, "!!! inconsistant FAT tentry=%x c=%d", 
                                tentry, c);
                return EIO;
        }

        // Set the current tentry to FREE 
        set_tentry_type(disk, &tentry, TENTRY_FREE);
        err = write_tentry(disk, c, &tentry);
        if (err != ENOERR)
            return err;

        // Next cluster in chain
        c = next_c; 
    }

    CYG_TRACE1(TCL, "last cluster=%d", c);
    
    return ENOERR;
}

//==========================================================================
// FAT dir entry functions 

// -------------------------------------------------------------------------
// print_raw_dentry()
// Prints FAT directory entry. 

#if TDE
static void
print_raw_dentry(fat_raw_dir_entry_t* dentry)
{
    if (DENTRY_IS_DELETED(dentry))
        diag_printf("FAT: FDE name:    '?%.7s'\n", &dentry->name[1]);
    else    
        diag_printf("FAT: FDE name:    '%.8s'\n", dentry->name);
    diag_printf("FAT: FDE ext:     '%.3s'\n", dentry->ext);
    diag_printf("FAT: FDE attr:     %c%c%c%c%c%c\n", 
                (DENTRY_IS_RDONLY(dentry)  ? 'R' : '-'),
                (DENTRY_IS_HIDDEN(dentry)  ? 'H' : '-'),
                (DENTRY_IS_SYSTEM(dentry)  ? 'S' : '-'),
                (DENTRY_IS_VOLUME(dentry)  ? 'V' : '-'),
                (DENTRY_IS_DIR(dentry)     ? 'D' : '-'),
                (DENTRY_IS_ARCHIVE(dentry) ? 'A' : '-'));
    diag_printf("FAT: FDE crt time: %u\n", dentry->crt_time);
    diag_printf("FAT: FDE crt date: %u\n", dentry->crt_date);
    diag_printf("FAT: FDE acc date: %u\n", dentry->acc_date);
    diag_printf("FAT: FDE wrt time: %u\n", dentry->wrt_time);
    diag_printf("FAT: FDE wrt date: %u\n", dentry->wrt_date);
    diag_printf("FAT: FDE cluster:  %u\n", dentry->cluster);
    diag_printf("FAT: FDE size:     %u\n", dentry->size);
}
#endif // TDE

// -------------------------------------------------------------------------
// read_raw_dentry()
// Reads dir entry from disk.
 
static int
read_raw_dentry(fatfs_disk_t        *disk,
                fatfs_data_pos_t    *pos, 
                fat_raw_dir_entry_t *dentry)
{
    cyg_uint8  data[DENTRY_SIZE];
    int        len, err;
 
    CYG_TRACE3(TDE, "cluster=%d snum=%d pos=%d",
               pos->cluster, pos->cluster_snum, pos->cluster_pos); 
   
    len = DENTRY_SIZE;

    // Check if we are reading the FAT12/16 root directory
    if (0 == pos->cluster)
        err = disk_read(disk, (void*)data, &len,  
                        disk->fat_root_dir_pos + pos->cluster_pos);
    else
        err = disk_cluster_read(disk, (void*)data, &len, 
                                pos->cluster, pos->cluster_pos);
    if (err != ENOERR)
        return err;

    GET_BYTES(data, dentry->name,     8, 0x00);
    GET_BYTES(data, dentry->ext,      3, 0x08);
    GET_BYTE(data,  dentry->attr,        0x0B);
    GET_BYTE(data,  dentry->nt_reserved, 0x0C);
    GET_BYTE(data,  dentry->crt_sec_100, 0x0D);
    GET_WORD(data,  dentry->crt_time,    0x0E);
    GET_WORD(data,  dentry->crt_date,    0x10);
    GET_WORD(data,  dentry->acc_date,    0x12);
    GET_WORD(data,  dentry->cluster_HI,  0x14);
    GET_WORD(data,  dentry->wrt_time,    0x16);
    GET_WORD(data,  dentry->wrt_date,    0x18);
    GET_WORD(data,  dentry->cluster,     0x1A);
    GET_DWORD(data, dentry->size,        0x1C);

     // Zero terminate strings
    dentry->name[8] = '\0';    
    dentry->ext[3]  = '\0';    
  
#if TDE    
    print_raw_dentry(dentry);
#endif

    return ENOERR;
}

// -------------------------------------------------------------------------
// write_raw_dentry()
// Writes raw dir entry to disk. 
 
static int
write_raw_dentry(fatfs_disk_t        *disk,
                 fatfs_data_pos_t    *pos, 
                 fat_raw_dir_entry_t *dentry)
{
    cyg_uint8  data[DENTRY_SIZE];
    int        len, err;
 
    CYG_TRACE3(TDE, "cluster=%d snum=%d pos=%d",
               pos->cluster, pos->cluster_snum, pos->cluster_pos); 
   
    SET_BYTES(data, dentry->name,     8, 0x00);
    SET_BYTES(data, dentry->ext,      3, 0x08);
    SET_BYTE(data,  dentry->attr,        0x0B);
    SET_BYTE(data,  dentry->nt_reserved, 0x0C);
    SET_BYTE(data,  dentry->crt_sec_100, 0x0D);
    SET_WORD(data,  dentry->crt_time,    0x0E);
    SET_WORD(data,  dentry->crt_date,    0x10);
    SET_WORD(data,  dentry->acc_date,    0x12);
    SET_WORD(data,  dentry->cluster_HI,  0x14);
    SET_WORD(data,  dentry->wrt_time,    0x16);
    SET_WORD(data,  dentry->wrt_date,    0x18);
    SET_WORD(data,  dentry->cluster,     0x1A);
    SET_DWORD(data, dentry->size,        0x1C);
   
    len = DENTRY_SIZE;

    // Check if we are writting to the FAT12/16 root directory
    if (0 == pos->cluster)
        err = disk_write(disk, (void*)data, &len, 
                         disk->fat_root_dir_pos + pos->cluster_pos);
    else
        err = disk_cluster_write(disk, (void*)data, &len,
                                 pos->cluster, pos->cluster_pos); 
    if (err != ENOERR)
        return err;

#if TDE    
    print_raw_dentry(dentry);
#endif

    return ENOERR;
}

// -------------------------------------------------------------------------
// raw_dentry_set_deleted()
// Sets the dentry filename first char to 0xE5 (ie deleted). 
 
static __inline__ void 
raw_dentry_set_deleted(fatfs_disk_t *disk, fat_raw_dir_entry_t *dentry)
{
    dentry->name[0] = 0xE5;
}

// -------------------------------------------------------------------------
// get_raw_dentry_filename()
// Gets the filename from given dir entry. 
 
static void 
get_raw_dentry_filename(fat_raw_dir_entry_t *dentry, char *name)
{
    int   i     = 0;
    char *cptr  = dentry->name;
    char *cname = name;

    while (*cptr != ' ' && i < 8)
    {
        *cname++ = *cptr++; i++;
    }
    cptr = dentry->ext;

    if (*cptr != ' ')
    {
        *cname++ = '.'; i = 0;
        while (*cptr != ' ' && i < 3)
        {
            *cname++ = *cptr++; i++;
        }
    }
    *cname = '\0';

    CYG_TRACE3(TDE, "dos name='%s' dos ext='%s' filename='%s'",
                    dentry->name, dentry->ext, name);
}

// -------------------------------------------------------------------------
// set_raw_dentry_filename()
// Sets the filename of given dir entry. 
 
static void 
set_raw_dentry_filename(fat_raw_dir_entry_t *dentry, 
                        const char          *name, 
                        int                  namelen)
{
    int         i, nidx;
    const char *cname;
    char       *cptr;

    // Special case check
    if ('.' == name[0])
    {
        if ('\0' == name[1])
        {
            strcpy(dentry->name, ".       ");
            strcpy(dentry->ext,  "   ");
            return;
        }
        else if ('.' == name[1] && '\0' == name[2])
        {
            strcpy(dentry->name, "..      ");
            strcpy(dentry->ext,  "   ");
            return;
        }
    }
    
    if (0 == namelen)
        namelen = 9999;
    
    nidx  = 0;
    cname = name;
    cptr  = dentry->name;
    for (i = 0; i < 8; i++)
    {
        if (*cname != '.' && *cname != '\0' && nidx++ < namelen)
            *cptr++ = toupper(*cname++);
        else
            *cptr++ = ' ';
    }
    *cptr = '\0';
    
    while (*cname != '.' && *cname != '\0' && nidx++ < namelen)
        cname++;
   
    if ('.' == *cname && nidx++ < namelen) 
        cname++;
    
    cptr = dentry->ext;
    for (i = 0; i < 3; i++)
    {
        if (*cname != '.' && *cname != '\0' && nidx++ < namelen)
            *cptr++ = toupper(*cname++);
        else
            *cptr++ = ' ';
    }
    *cptr = '\0';

    CYG_TRACE4(TDE, "filename='%s' namelen=%d dos name='%s' dos ext='%s'", 
                    name, namelen, dentry->name, dentry->ext);
}

// -------------------------------------------------------------------------
// read_next_raw_dentry()
// Gets next dir entry searching from given position to the end.
// If EEOF is returned there are no more entries in given dir.
 
static int
read_next_raw_dentry(fatfs_disk_t        *disk,
                     fatfs_data_pos_t    *pos,
                     fat_raw_dir_entry_t *dentry)
{
    int err = ENOERR;

    // If we are reading the root dir on FAT32 we have
    // to correct the position to the root dir cluster
    if (FATFS_FAT32 == disk->fat_type && 0 == pos->cluster)
        pos->cluster = disk->fat_root_dir_cluster;
        
    while (true)
    {
        // FAT12/16 root dir check
        if (0 == pos->cluster) 
        {
            if (pos->cluster_pos >= disk->fat_root_dir_nents)
                err = EEOF;
        }
        else
        {
            // Change cluster if needed
            if (pos->cluster_pos >= disk->cluster_size)
                err = get_next_cluster(disk, pos, CO_NONE);
        }

        if (err != ENOERR)
            break;

        err = read_raw_dentry(disk, pos, dentry);
        if (err != ENOERR)
            return err;

        if (DENTRY_IS_ZERO(dentry))
        {
            // If we get a ZERO dir entry, we assume that
            // there are no more entries in current dir
            CYG_TRACE0(TDE, "end of dir"); 
            err = EEOF;
            break;
        }
        else if (!DENTRY_IS_DELETED(dentry))
        {
            // Dir entry found
            CYG_TRACE3(TDE, "found new dentry at cluster=%d snum=%d pos=%d",
                            pos->cluster, pos->cluster_snum, pos->cluster_pos);
            break;
        }

        pos->cluster_pos += DENTRY_SIZE;
    }

    // EEOF could be returned if there are no more entries in this
    // dir - this should be cought by the calling function 

    return err;
}

// -------------------------------------------------------------------------
// get_free_raw_dentry()
// Gets free dir entry slot searching from given position extending the
// directory if needed. If an deleated entry is found it is reused.

static int
get_free_raw_dentry(fatfs_disk_t     *disk, 
                    fatfs_data_pos_t *pos)
{
    fat_raw_dir_entry_t raw_dentry;
    fatfs_data_pos_t    cpos;
    int                 err = ENOERR;
    
    cpos = *pos;

    // If we are reading the root dir on FAT32 we have
    // to correct the position to the root dir cluster
    if (FATFS_FAT32 == disk->fat_type && 0 == cpos.cluster)
        cpos.cluster = disk->fat_root_dir_cluster;
 
    CYG_TRACE3(TDE, "cluster=%d snum=%d pos=%d", 
                    pos->cluster, pos->cluster_snum, pos->cluster_pos);
 
    while (true)
    {
        // FAT12/16 root dir check
        if (0 == cpos.cluster) 
        {
            if (cpos.cluster_pos >= disk->fat_root_dir_size)
                err = ENOSPC;
        }
        else
        { 
            // Change cluster if needed
            if (cpos.cluster_pos >= disk->cluster_size)
                err = get_next_cluster(disk, &cpos, CO_EXTEND | CO_ERASE_NEW);
        }

        if (err != ENOERR)
            return err;

        err = read_raw_dentry(disk, &cpos, &raw_dentry);
        if (err != ENOERR)
            return err;

        if (DENTRY_IS_DELETED(&raw_dentry))
        {
            CYG_TRACE3(TDE, "deleted dentry at cluster=%d snum=%d pos=%d",
                            cpos.cluster, cpos.cluster_snum, cpos.cluster_pos);

            *pos = cpos;
            return ENOERR;
        }
        else if (DENTRY_IS_ZERO(&raw_dentry))
        {
            CYG_TRACE3(TDE, "zero dentry at cluster=%d snum=%d pos=%d",
                            cpos.cluster, cpos.cluster_snum, cpos.cluster_pos);

            *pos = cpos;
            return ENOERR;  
        }
       
        cpos.cluster_pos += DENTRY_SIZE;
    }
}
 
// -------------------------------------------------------------------------
// raw_to_dentry()
// Converts raw FAT dir entry to dir entry. 
 
static void
raw_to_dentry(fat_raw_dir_entry_t *raw_dentry,
              fatfs_data_pos_t    *raw_dentry_pos,
              fatfs_dir_entry_t   *dentry)
{
    get_raw_dentry_filename(raw_dentry, dentry->filename);

    if (DENTRY_IS_DIR(raw_dentry))
        dentry->mode = __stat_mode_DIR;
    else
        dentry->mode = __stat_mode_REG;
    
#ifdef CYGCFG_FS_FAT_USE_ATTRIBUTES
    dentry->attrib = raw_dentry->attr;
#endif // CYGCFG_FS_FAT_USE_ATTRIBUTES

    date_dos2unix(raw_dentry->crt_time, raw_dentry->crt_date, &dentry->ctime);
    date_dos2unix(0,                    raw_dentry->acc_date, &dentry->atime);
    date_dos2unix(raw_dentry->wrt_time, raw_dentry->wrt_date, &dentry->mtime);
    
    dentry->size       = raw_dentry->size;
    dentry->priv_data  = raw_dentry->nt_reserved;
    dentry->cluster    = raw_dentry->cluster | (raw_dentry->cluster_HI << 16);
    dentry->disk_pos   = *raw_dentry_pos;
}

// -------------------------------------------------------------------------
// dentry_to_raw()
// Converts dir entry to raw FAT dir entry. 
 
static void
dentry_to_raw(fatfs_dir_entry_t *dentry, fat_raw_dir_entry_t *raw_dentry)
{
    set_raw_dentry_filename(raw_dentry, dentry->filename, 0);

    if (__stat_mode_DIR == dentry->mode)
        raw_dentry->attr = S_FATFS_DIR;
    else
        raw_dentry->attr = S_FATFS_ARCHIVE;
#ifdef CYGCFG_FS_FAT_USE_ATTRIBUTES
    raw_dentry->attr = dentry->attrib;
#endif // CYGCFG_FS_FAT_USE_ATTRIBUTES

        
    date_unix2dos(dentry->ctime, &raw_dentry->crt_time, &raw_dentry->crt_date);
    date_unix2dos(dentry->atime, NULL,                  &raw_dentry->acc_date);
    date_unix2dos(dentry->mtime, &raw_dentry->wrt_time, &raw_dentry->wrt_date);
    
    raw_dentry->crt_sec_100 = 0; //FIXME
    raw_dentry->size        = dentry->size;
    raw_dentry->nt_reserved = dentry->priv_data;
    raw_dentry->cluster     = dentry->cluster & 0xFFFF;
    raw_dentry->cluster_HI  = dentry->cluster >> 16;
}

//==========================================================================
// FAT data functions 

// -------------------------------------------------------------------------
// read_data()
// Reads data from given position. 
 
static int
read_data(fatfs_disk_t     *disk,
          void             *data,
          cyg_uint32       *len,
          fatfs_data_pos_t *pos)
{
    cyg_uint8   *buf   = (cyg_uint8 *) data;
    cyg_uint32   size  = *len;
    int          err   = ENOERR;

    CYG_TRACE4(TDO, "len=%d cluster=%d snum=%d pos=%d",
                    *len, pos->cluster, pos->cluster_snum, 
                    pos->cluster_pos);

    while (size > 0)
    {
        cyg_uint32 csize;

        // Check if we are still inside current cluster
        if (pos->cluster_pos >= disk->cluster_size)
        {
            // Get next cluster of file 
            err = get_next_cluster(disk, pos, CO_NONE);
            if (err != ENOERR)
                goto out;
        }
        
        // Adjust the data chunk size to be read to the cluster boundary
        if (size > (disk->cluster_size - pos->cluster_pos))
            csize = disk->cluster_size - pos->cluster_pos;
        else
            csize = size;

        CYG_TRACE4(TDO, "-- len=%d cluster=%d snum=%d pos=%d",
                        csize, pos->cluster, pos->cluster_snum,
                        pos->cluster_pos);

        err = disk_cluster_read(disk, (void*)buf, &csize, 
                                pos->cluster, pos->cluster_pos);
        if (err != ENOERR)
            goto out;

        // Adjust running variables

        buf              += csize;
        pos->cluster_pos += csize;
        size             -= csize;    
    }
    
out:
    *len -= size;

    CYG_TRACE1(TDO, "total len=%d", *len);

    return err;
}

// -------------------------------------------------------------------------
// write_data()
// Writes data to given position. 
 
static int
write_data(fatfs_disk_t     *disk,
           void             *data,
           cyg_uint32       *len,
           fatfs_data_pos_t *pos)
{
    cyg_uint8   *buf   = (cyg_uint8 *) data;
    cyg_uint32   size  = *len;
    int          err   = ENOERR;

    CYG_TRACE4(TDO, "len=%d cluster=%d snum=%d pos=%d",
                    *len, pos->cluster, pos->cluster_snum, 
                    pos->cluster_pos);

    while (size > 0)
    {
        cyg_uint32 csize;

        // Check if we are still inside current cluster
        if (pos->cluster_pos >= disk->cluster_size)
        {
            // Get next cluster of file, if at the last 
            // cluster try to extend the cluster chain 
            err = get_next_cluster(disk, pos, CO_EXTEND);
            if (err != ENOERR)
                goto out;
        }
        
        // Adjust the data chunk size to be read to the cluster boundary
        if (size > (disk->cluster_size - pos->cluster_pos))
            csize = disk->cluster_size - pos->cluster_pos;
        else
            csize = size;
 
        CYG_TRACE4(TDO, "-- len=%d cluster=%d snum=%d pos=%d",
                        csize, pos->cluster, pos->cluster_snum, 
                        pos->cluster_pos);

        err = disk_cluster_write(disk, (void*)buf, &csize, 
                                 pos->cluster, pos->cluster_pos);
        if (err != ENOERR)
            goto out;

        // Adjust running variables
        
        buf              += csize;
        pos->cluster_pos += csize;
        size             -= csize;    
    }
    
out:
    *len -= size;

    CYG_TRACE1(TDO, "total len=%d", *len);

    return err;
}

//==========================================================================
// Misc functions 

// -------------------------------------------------------------------------
// init_dir_entry()
// Initializes attributes of a new dir entry. 
 
static void
init_dir_entry(fatfs_dir_entry_t *dentry, 
               const char        *name, 
               int                namelen,
               mode_t             mode,
               cyg_uint32         parent_cluster, 
               cyg_uint32         first_cluster, 
               fatfs_data_pos_t  *pos)
{
    if (0 == namelen)
        namelen = 12;
    
    strncpy(dentry->filename, name, namelen);
    dentry->filename[namelen] = '\0';
    
    dentry->mode  = mode;

#ifdef CYGCFG_FS_FAT_USE_ATTRIBUTES
    if (S_ISDIR(dentry->mode))
        dentry->attrib = S_FATFS_DIR;
    else
        dentry->attrib = S_FATFS_ARCHIVE;
#endif // CYGCFG_FS_FAT_USE_ATTRIBUTES

    dentry->ctime = 
    dentry->atime =
    dentry->mtime = cyg_timestamp();

    dentry->priv_data      = 0;
    dentry->size           = 0;
    dentry->cluster        = first_cluster;
    dentry->parent_cluster = parent_cluster;
    dentry->disk_pos       = *pos;
}

// -------------------------------------------------------------------------
// is_root_dir_entry()
// Check if the given dir entry is the root dir entry. 
 
static __inline__ bool
is_root_dir_entry(fatfs_dir_entry_t *dentry)
{
    return ('\0' == dentry->filename[0] && 0 == dentry->cluster);
}

//==========================================================================
//==========================================================================
// Exported functions 

// -------------------------------------------------------------------------
// fatfs_init()
// Gets disk info. 
 
int
fatfs_init(fatfs_disk_t *disk)
{
    cyg_uint32        sec_num, sec_per_fat, root_dir_sec_num;
    cyg_uint32        data_sec_num, data_clu_num;
    fat_boot_record_t boot_rec;
    int               err;

    CYG_CHECK_DATA_PTRC(disk);
 
    err = read_boot_record(disk, &boot_rec);
    if (err != ENOERR)    
        return err;

    // Check some known boot record values
    if (0x29 != boot_rec.ext_sig       ||
        0x55 != boot_rec.exe_marker[0] ||
        0xAA != boot_rec.exe_marker[1])
        return EINVAL;

    // Sector and cluster sizes 
    disk->sector_size       = boot_rec.bytes_per_sec;
    disk->sector_size_log2  = get_val_log2(disk->sector_size);
    disk->cluster_size      = boot_rec.bytes_per_sec * boot_rec.sec_per_clu;
    disk->cluster_size_log2 = get_val_log2(disk->cluster_size);

    // Sector and cluster size should always be a power of 2
    if (0 == disk->sector_size_log2 || 0 == disk->cluster_size_log2)
        return EINVAL;

    // Determine number of sectors
    if (boot_rec.sec_num_32 != 0)
        sec_num = boot_rec.sec_num_32;
    else
        sec_num = boot_rec.sec_num;

    // Determine number of sectors per fat
    if (boot_rec.sec_per_fat != 0)
        sec_per_fat = boot_rec.sec_per_fat;
    else
        sec_per_fat = boot_rec.sec_per_fat_32;
    
    // Number of sectors used by root directory 
    root_dir_sec_num = ((boot_rec.max_root_dents * DENTRY_SIZE) +
                        (boot_rec.bytes_per_sec - 1)) / boot_rec.bytes_per_sec;
        
    // Number of data sectors
    data_sec_num = sec_num - (boot_rec.res_sec_num + 
        (boot_rec.fat_tbls_num * sec_per_fat) + root_dir_sec_num);
    
    // Number of data clusters
    data_clu_num = data_sec_num / boot_rec.sec_per_clu;

    // FAT table size and position
    disk->fat_tbl_pos   = boot_rec.bytes_per_sec * boot_rec.res_sec_num;
    disk->fat_tbl_size  = boot_rec.bytes_per_sec * sec_per_fat;   
    disk->fat_tbl_nents = data_clu_num + 2;
    disk->fat_tbls_num  = boot_rec.fat_tbls_num;

    // Determine the type of FAT based on number of data clusters
    if (data_clu_num < 4085)
        disk->fat_type  = FATFS_FAT12;
    else if (data_clu_num < 65525) 
        disk->fat_type  = FATFS_FAT16;
    else 
        disk->fat_type  = FATFS_FAT32;

    // Determine root dir and data positions
    if (FATFS_FAT32 != disk->fat_type)
    {
        disk->fat_root_dir_pos     = disk->fat_tbl_pos + 
                                     disk->fat_tbls_num * disk->fat_tbl_size;
        disk->fat_root_dir_size    = boot_rec.max_root_dents * DENTRY_SIZE;
        disk->fat_root_dir_nents   = boot_rec.max_root_dents;
        disk->fat_root_dir_cluster = 0;
        disk->fat_data_pos         = disk->fat_root_dir_pos + 
                                     disk->fat_root_dir_size;
    }
    else
    {
        disk->fat_root_dir_pos     = 0;
        disk->fat_root_dir_size    = 0;
        disk->fat_root_dir_nents   = 0;
        disk->fat_root_dir_cluster = boot_rec.root_cluster; 
        disk->fat_data_pos         = disk->fat_tbl_pos +
                                     disk->fat_tbls_num * disk->fat_tbl_size;
    }
    
    return ENOERR;
}

// -------------------------------------------------------------------------
// fatfs_get_root_dir_entry()
// Gets root dir entry. 

void
fatfs_get_root_dir_entry(fatfs_disk_t *disk, fatfs_dir_entry_t *dentry)
{
    CYG_CHECK_DATA_PTRC(disk);
    CYG_CHECK_DATA_PTRC(dentry);
    
    dentry->mode           = __stat_mode_DIR;
#ifdef CYGCFG_FS_FAT_USE_ATTRIBUTES
    dentry->attrib         = S_FATFS_DIR;
#endif // CYGCFG_FS_FAT_USE_ATTRIBUTES
    dentry->size           = disk->fat_root_dir_size;
    dentry->ctime          = 0;
    dentry->atime          = 0;
    dentry->mtime          = 0;
    dentry->filename[0]    = '\0';
    dentry->cluster        = 0;
    dentry->parent_cluster = 0;

    dentry->disk_pos.cluster      = 0;
    dentry->disk_pos.cluster_snum = 0;
    dentry->disk_pos.cluster_pos  = 0;
}

// -------------------------------------------------------------------------
// fatfs_get_disk_usage()
// Gets disk space.

int
fatfs_get_disk_usage(fatfs_disk_t *disk,
                     cyg_uint32   *total_clusters,
                     cyg_uint32   *free_clusters)
{
    cyg_uint32 c, nfree, tentry;
    int        err;

    nfree = 0;
    for (c = 2; c < disk->fat_tbl_nents; c++)
    {
        err = read_tentry(disk, c, &tentry);
        if (err != ENOERR)
            return err;

        if (TENTRY_FREE == get_tentry_type(disk, tentry))
            nfree++;
    }

    *total_clusters = disk->fat_tbl_nents - 2;
    *free_clusters  = nfree;
  
    return ENOERR;
}


// -------------------------------------------------------------------------
// fatfs_read_dir_entry()
// Reads dir entry at given position.
// If there is no dir entry at given position the next closest is returned 
// and the position is updated. If EEOF error is returned, then there are 
// no more dir entries in given dir. 

int
fatfs_read_dir_entry(fatfs_disk_t      *disk,
                     fatfs_dir_entry_t *dir,
                     fatfs_data_pos_t  *pos,
                     fatfs_dir_entry_t *dentry)
{
    fat_raw_dir_entry_t raw_dentry;
    int                 err;

    CYG_CHECK_DATA_PTRC(disk);
    CYG_CHECK_DATA_PTRC(dir);
    CYG_CHECK_DATA_PTRC(pos);
    CYG_CHECK_DATA_PTRC(dentry);
 
    err = read_next_raw_dentry(disk, pos, &raw_dentry);
    if (err != ENOERR)
       return err;

    raw_to_dentry(&raw_dentry, pos, dentry);
    dentry->parent_cluster = dir->cluster;

    // Increment position for next call
    pos->cluster_pos += DENTRY_SIZE;
    
    return ENOERR;
}

// -------------------------------------------------------------------------
// fatfs_initpos()
// Initializes position to the start of the given file.

int
fatfs_initpos(fatfs_disk_t      *disk, 
              fatfs_dir_entry_t *file,
              fatfs_data_pos_t  *pos)
{
    CYG_CHECK_DATA_PTRC(disk);
    CYG_CHECK_DATA_PTRC(file);
    CYG_CHECK_DATA_PTRC(pos);
    
    pos->cluster      = file->cluster;
    pos->cluster_snum = 0;
    pos->cluster_pos  = 0;

    return ENOERR;
}

// -------------------------------------------------------------------------
// fatfs_setpos()
// Sets the file position from offset.

int
fatfs_setpos(fatfs_disk_t      *disk, 
             fatfs_dir_entry_t *file,
             fatfs_data_pos_t  *pos,
             cyg_uint32         offset)
{
    CYG_CHECK_DATA_PTRC(disk);
    CYG_CHECK_DATA_PTRC(file);
    CYG_CHECK_DATA_PTRC(pos);
    
    return get_position_from_off(disk, file->cluster, offset, pos, CO_NONE);
}

// -------------------------------------------------------------------------
// fatfs_getpos()
// Gets the file offset from position.

cyg_uint32 
fatfs_getpos(fatfs_disk_t      *disk,
             fatfs_dir_entry_t *file,
             fatfs_data_pos_t  *pos)
{
    CYG_CHECK_DATA_PTRC(disk);
    CYG_CHECK_DATA_PTRC(file);
    CYG_CHECK_DATA_PTRC(pos);
    
    return (pos->cluster_snum << disk->cluster_size_log2) + pos->cluster_pos;
}

// -------------------------------------------------------------------------
// fatfs_write_dir_entry()
// Writes dir entry to disk.
 
int
fatfs_write_dir_entry(fatfs_disk_t *disk, fatfs_dir_entry_t *dentry)
{
    fat_raw_dir_entry_t raw_dentry;
    int                 err;

    CYG_CHECK_DATA_PTRC(disk);
    CYG_CHECK_DATA_PTRC(dentry);
 
    dentry_to_raw(dentry, &raw_dentry);
    err = write_raw_dentry(disk, &dentry->disk_pos, &raw_dentry);
    return err;
}

// -------------------------------------------------------------------------
// fatfs_delete_file()
// Marks dir entry as deleted and frees its cluster chain.
 
int
fatfs_delete_file(fatfs_disk_t *disk, fatfs_dir_entry_t *file)
{
    fat_raw_dir_entry_t raw_dentry;
    int                 err;

    CYG_CHECK_DATA_PTRC(disk);
    CYG_CHECK_DATA_PTRC(file);

    if (is_root_dir_entry(file))
        return EINVAL;
    
    CYG_TRACE1(TDE, "filename='%s'", file->filename);

    dentry_to_raw(file, &raw_dentry);

    // Check if we are about to delete a directory
    if (DENTRY_IS_DIR(&raw_dentry))
    {
        fat_raw_dir_entry_t raw_cdentry;
        fatfs_data_pos_t    pos;
        int                 i = 0;
        
        fatfs_initpos(disk, file, &pos);
        
        CYG_TRACE0(TDE, "got directory");

        // Count number of entries in this dir    

        while (true)
        {
            err = read_next_raw_dentry(disk, &pos, &raw_cdentry);

            if (EEOF == err)
                break;
            else if (err != ENOERR)
                return err;

            pos.cluster_pos += DENTRY_SIZE;
            i++; 
        }
        CYG_TRACE1(TDE, "child count=%d", i);
        
        // Check if the dir is empty (except '.' and '..')
        if (i > 2)
            return EPERM; 
    }    
    
    // Free file clusters
    free_cluster_chain(disk, raw_dentry.cluster);
    raw_dentry_set_deleted(disk, &raw_dentry);
    err = write_raw_dentry(disk, &file->disk_pos, &raw_dentry);
    return err;
} 

// -------------------------------------------------------------------------
// fatfs_create_file()
// Creates a new file.
 
int
fatfs_create_file(fatfs_disk_t      *disk, 
                  fatfs_dir_entry_t *dir, 
                  const char        *name,
                  int                namelen,
                  fatfs_dir_entry_t *dentry)
{
    fatfs_data_pos_t pos;
    int              err;

    CYG_CHECK_DATA_PTRC(disk);
    CYG_CHECK_DATA_PTRC(dir);
    CYG_CHECK_DATA_PTRC(name);
    CYG_CHECK_DATA_PTRC(dentry);
 
    CYG_TRACE2(TDE, "filename='%s' parent='%s'", name, dir->filename);

    fatfs_initpos(disk, dir, &pos);

    // Get free dir entry in parent dir
    err = get_free_raw_dentry(disk, &pos);
    if (err != ENOERR)
        return err;

    // Create new file dir entry
    
    init_dir_entry(dentry, 
                   name, 
                   namelen, 
                   __stat_mode_REG, 
                   dir->cluster, 
                   0, 
                   &pos); 

    err = fatfs_write_dir_entry(disk, dentry);
    if (err != ENOERR)
       return err;
 
    return ENOERR;     
}

// -------------------------------------------------------------------------
// fatfs_create_dir()
// Creates a new directory.
 
int
fatfs_create_dir(fatfs_disk_t      *disk, 
                 fatfs_dir_entry_t *dir, 
                 const char        *name,
                 int                namelen,
                 fatfs_dir_entry_t *dentry)
{
    fatfs_dir_entry_t cdentry;
    fatfs_data_pos_t  pos;
    cyg_uint32        free_cluster;
    int               err;

    CYG_CHECK_DATA_PTRC(disk);
    CYG_CHECK_DATA_PTRC(dir);
    CYG_CHECK_DATA_PTRC(name);
    CYG_CHECK_DATA_PTRC(dentry);
 
    CYG_TRACE2(TDE, "filename='%s' parent='%s'", name, dir->filename);

    // Get free cluster
    err = find_next_free_cluster(disk, 
                                 0, 
                                 &free_cluster, 
                                 CO_MARK_LAST | CO_ERASE_NEW);
    if (err != ENOERR)
        return err;

    fatfs_initpos(disk, dir, &pos);

    // Get free dir entry in parent dir
    err = get_free_raw_dentry(disk, &pos);
    if (err != ENOERR)
        return err;

    // Create new dir entry

    init_dir_entry(dentry, 
                   name, 
                   namelen, 
                   __stat_mode_DIR, 
                   dir->cluster, 
                   free_cluster, 
                   &pos); 

    err = fatfs_write_dir_entry(disk, dentry);
    if (err != ENOERR)
       return err;

    // Create '.' and '..' dir entries

    fatfs_initpos(disk, dentry, &pos);

    CYG_TRACE0(TDE, "Creating '.' entry");

    init_dir_entry(&cdentry, 
                   ".", 
                   0, 
                   __stat_mode_DIR, 
                   dentry->cluster, 
                   dentry->cluster, 
                   &pos);

    err = fatfs_write_dir_entry(disk, &cdentry);
    if (err != ENOERR)
        return err;
          
    pos.cluster_pos += DENTRY_SIZE;

    CYG_TRACE0(TDE, "Creating '..' entry");
    
    init_dir_entry(&cdentry, 
                   "..", 
                   0, 
                   __stat_mode_DIR,
                   dentry->cluster, 
                   dir->cluster, 
                   &pos); 
    
    err = fatfs_write_dir_entry(disk, &cdentry);
    if (err != ENOERR)
        return err;
  
    return ENOERR;     
}

// -------------------------------------------------------------------------
// fatfs_trunc_file()
// Truncates a file to zero length.

int
fatfs_trunc_file(fatfs_disk_t *disk, fatfs_dir_entry_t *file)
{
    int err;
    
    CYG_CHECK_DATA_PTRC(disk);
    CYG_CHECK_DATA_PTRC(file);
 
    CYG_TRACE1(TDE, "file='%s'", file->filename);
  
    if (S_ISDIR(file->mode))
        return EINVAL; 

    if (0 == file->size)
        return ENOERR;
   
    err = free_cluster_chain(disk, file->cluster);
    if (err != ENOERR)
        return err;
    
    // Update file attributes

    file->cluster = 0;
    file->size    = 0;
    file->mtime   =
    file->atime   = cyg_timestamp();

    return fatfs_write_dir_entry(disk, file);
}

// -------------------------------------------------------------------------
// fatfs_rename_file()
// Renames a file.
 
int
fatfs_rename_file(fatfs_disk_t      *disk, 
                  fatfs_dir_entry_t *dir1, 
                  fatfs_dir_entry_t *target,
                  fatfs_dir_entry_t *dir2, 
                  const char        *name,
                  int                namelen)
{
    fat_raw_dir_entry_t raw_dentry;
    fatfs_data_pos_t    new_pos;
    int                 err;

    CYG_CHECK_DATA_PTRC(disk);
    CYG_CHECK_DATA_PTRC(dir1);
    CYG_CHECK_DATA_PTRC(target);
    CYG_CHECK_DATA_PTRC(dir2);
    CYG_CHECK_DATA_PTRC(name);
 
    if (is_root_dir_entry(target))
        return EINVAL;
 
    strncpy(target->filename, name, namelen);
    target->filename[namelen] = '\0';
   
    // Moving around in same dir

    if (dir1 == dir2)
    {
        CYG_TRACE0(TDE, "same dir");
        return fatfs_write_dir_entry(disk, target); 
    }
    
    CYG_TRACE0(TDE, "different dirs"); 
    
    // Moveing around in different dirs

    fatfs_initpos(disk, dir2, &new_pos);

    CYG_TRACE0(TDE, "writting to new dir"); 

    // Get free dir entry in target dir

    err = get_free_raw_dentry(disk, &new_pos);
    if (err != ENOERR)
        return err;

    // Write file dentry to new location

    dentry_to_raw(target, &raw_dentry);
    err = write_raw_dentry(disk, &new_pos, &raw_dentry);
    if (err != ENOERR)
        return err;
   
    CYG_TRACE0(TDE, "deleting from old dir"); 
    
    // Deleate dentry at old location

    raw_dentry_set_deleted(disk, &raw_dentry);
    raw_dentry.size    = 0;
    raw_dentry.cluster = 0;
    err = write_raw_dentry(disk, &target->disk_pos, &raw_dentry);
    if (err != ENOERR)
        return err;
   
    // Set file new position and parent cluster

    target->disk_pos       = new_pos;
    target->parent_cluster = dir2->cluster;
 
    // If we moved a directory, we also have to correct the '..' entry  

    if ( S_ISDIR(target->mode) )
    {
        fat_raw_dir_entry_t raw_cdentry;
        fatfs_data_pos_t    pos;
       
        fatfs_initpos(disk, target, &pos);

        CYG_TRACE0(TDE, "moving directory - correcting '..' entry");

        while (true)
        {
            err = read_next_raw_dentry(disk, &pos, &raw_cdentry);

            if (EEOF == err)
                return EINVAL; // This dir doesn't have the '..' entry,
                               // that means something is very wrong
            else if (err != ENOERR)
                return err;

            if (0 == strncmp("..", raw_cdentry.name, 2))
            {
                raw_cdentry.cluster = dir2->cluster;
                err = write_raw_dentry(disk, &pos, &raw_cdentry);
                if (err != ENOERR)
                    return err;
                break;
            }

            pos.cluster_pos += DENTRY_SIZE;
        }
    }
   
    return ENOERR;     
}

// -------------------------------------------------------------------------
// fatfs_read_data()
// Reads data from disk. 
 
int
fatfs_read_data(fatfs_disk_t      *disk,
                fatfs_dir_entry_t *file,
                fatfs_data_pos_t  *pos,
                void              *data,
                cyg_uint32        *len)
{
    CYG_CHECK_DATA_PTRC(disk);
    CYG_CHECK_DATA_PTRC(file);
    CYG_CHECK_DATA_PTRC(data);
    CYG_CHECK_DATA_PTRC(len);
    CYG_CHECK_DATA_PTRC(pos);

    return read_data(disk, data, len, pos);
}

// -------------------------------------------------------------------------
// fatfs_write_data()
// Writes data to disk. 
 
int
fatfs_write_data(fatfs_disk_t      *disk,
                 fatfs_dir_entry_t *file,
                 fatfs_data_pos_t  *pos,
                 void              *data,
                 cyg_uint32        *len)
{
    int err;

    CYG_CHECK_DATA_PTRC(disk);
    CYG_CHECK_DATA_PTRC(file);
    CYG_CHECK_DATA_PTRC(data);
    CYG_CHECK_DATA_PTRC(len);
    CYG_CHECK_DATA_PTRC(pos);

    // Check if this file has a zero size and no first cluster

    if (0 == file->size && 0 == file->cluster)
    {
        cyg_uint32 free_cluster;

        CYG_TRACE0(TDO, "new cluster for zero file");

        err = find_next_free_cluster(disk, 0, &free_cluster, CO_MARK_LAST);

        if (err != ENOERR)
        {
            *len = 0;
            return err;
        }

        file->cluster = free_cluster;
        fatfs_initpos(disk, file, pos);
    }

    return write_data(disk, data, len, pos);
}

// -------------------------------------------------------------------------
// Support routines
// These enable the definition of local versions of certain routines
// if the given packages are not present.

#ifndef CYGPKG_LIBC_I18N

__externC int
toupper( int c )
{
    return (('a' <= c) && (c <= 'z')) ? c - 'a' + 'A' : c ;
}

#endif

#ifndef CYGFUN_LIBC_STRING_BSD_FUNCS

__externC int
strcasecmp( const char *s1, const char *s2 )
{
    int ret;
    CYG_REPORT_FUNCNAMETYPE( "strcasecmp", "returning %d" );
    CYG_REPORT_FUNCARG2( "s1=%08x, s2=%08x", s1, s2 );

    CYG_CHECK_DATA_PTR( s1, "s1 is not a valid pointer!" );
    CYG_CHECK_DATA_PTR( s2, "s2 is not a valid pointer!" );

    while (*s1 != '\0' && toupper(*s1) == toupper(*s2))
    {
        s1++;
        s2++;
    }

    ret = toupper(*(unsigned char *) s1) - toupper(*(unsigned char *) s2);
    CYG_REPORT_RETVAL( ret );
    return ret;
}

__externC int
strncasecmp( const char *s1, const char *s2, size_t n )
{
    int ret;
    CYG_REPORT_FUNCNAMETYPE( "strncasecmp", "returning %d" );
    CYG_REPORT_FUNCARG3( "s1=%08x, s2=%08x, n=%d", s1, s2, n );

    if (n == 0)
    {
        CYG_REPORT_RETVAL(0);
        return 0;
    }

    CYG_CHECK_DATA_PTR( s1, "s1 is not a valid pointer!" );
    CYG_CHECK_DATA_PTR( s2, "s2 is not a valid pointer!" );

    while (n-- != 0 && toupper(*s1) == toupper(*s2))
    {
        if (n == 0 || *s1 == '\0' || *s2 == '\0')
            break;
        s1++;
        s2++;
    }

    ret = toupper(*(unsigned char *) s1) - toupper(*(unsigned char *) s2);
    CYG_REPORT_RETVAL( ret );
    return ret;
}

#endif

// -------------------------------------------------------------------------
// EOF fatfs_supp.c
