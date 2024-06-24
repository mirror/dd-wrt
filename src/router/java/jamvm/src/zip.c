/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2009
 * Robert Lougher <rob@jamvm.org.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "jam.h"
#include "hash.h"
#include "zip.h"

#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <sys/mman.h>

/* Required on OpenSolaris. */
#ifndef MAP_FILE
#define MAP_FILE 0
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Common definitions needed for hashtable */
#define HASHTABSZE 1<<8
#define PREPARE(ptr) ptr
#define SCAVENGE(ptr) FALSE
#define FOUND(ptr1, ptr2) ptr2

/* zlib window size */
#define MAX_WINDOW_BITS 15

/* Macros for reading non-aligned little-endian values from zip file */
#define READ_LE_INT(p) ((p)[0]|((p)[1]<<8)|((p)[2]<<16)|((p)[3]<<24))
#define READ_LE_SHORT(p) ((p)[0]|((p)[1]<<8))

/* Offsets and lengths of fields within the zip file format */

#define SIG_LEN                    4

/* End of central directory record */
#define END_CEN_SIG                0x06054b50
#define END_CEN_LEN                22
#define END_CEN_ENTRIES_OFFSET     8
#define END_CEN_DIR_START_OFFSET   16

/* Central directory file header */
#define CEN_FILE_HEADER_SIG        0x02014b50
#define CEN_FILE_HEADER_LEN        46
#define CEN_FILE_COMPMETH_OFFSET   10
#define CEN_FILE_COMPLEN_OFFSET    20
#define CEN_FILE_UNCOMPLEN_OFFSET  24
#define CEN_FILE_PATHLEN_OFFSET    28
#define CEN_FILE_EXTRALEN_OFFSET   30
#define CEN_FILE_COMMENTLEN_OFFSET 32
#define CEN_FILE_LOCALHDR_OFFSET   42

/* Local file header */
#define LOC_FILE_HEADER_SIG        0x04034b50
#define LOC_FILE_HEADER_LEN        30
#define LOC_FILE_EXTRA_OFFSET      28

/* Supported compression methods */
#define COMP_STORED                0
#define COMP_DEFLATED              8

#define HASH(ptr) zipHash(ptr)
#define COMPARE(ptr1, ptr2, hash1, hash2) ((hash1 == hash2) && \
                                           zipComp(ptr1, ptr2)) 

/* The filenames within the zip file are added to a hash-table for faster
   access.  To save memory, the entries point directly into the zip file's
   central directory.  However, filenames within the directory are not null-
   terminated.  This means the path length must be read each time.  As the
   utf8 routines operate on null-terminated strings, the filename must also
   be copied into a temporary buffer.  This is NOT performance critical,
   as the hash is calculated only once per entry (on insertion)
*/
int zipHash(char *path) {
    int path_len = READ_LE_SHORT((unsigned char*)path +
                                        (CEN_FILE_PATHLEN_OFFSET -
                                         CEN_FILE_HEADER_LEN));
    char buff[path_len + 1];

    memcpy(buff, path, path_len);
    buff[path_len] = '\0';

    return utf8Hash(buff);
}

/* When adding entries to the hash-table, the paths being compared will
   both be within the zip file.  As there are no concerns about differing
   encoding of null, we can do a memcmp to compare them
*/
int zipComp(char *path1, char *path2) {
    int path1_len = READ_LE_SHORT((unsigned char*)path1 +
                                          (CEN_FILE_PATHLEN_OFFSET -
                                           CEN_FILE_HEADER_LEN));
    int path2_len = READ_LE_SHORT((unsigned char*)path2 +
                                          (CEN_FILE_PATHLEN_OFFSET -
                                           CEN_FILE_HEADER_LEN));

    return path1_len == path2_len && !memcmp(path1, path2, path1_len);
}

ZipFile *processArchive(char *path) {
    unsigned char magic[SIG_LEN];
    unsigned char *data, *pntr;
    int entries, fd, len;

    HashTable *hash_table;
    ZipFile *zip;

    if((fd = open(path, O_RDONLY)) == -1)
        return NULL;

    /* First 4 bytes must be the signature for the first local file header */
    if(read(fd, &magic[0], SIG_LEN) != SIG_LEN ||
                    READ_LE_INT(magic) != LOC_FILE_HEADER_SIG)
        goto error;

    /* Get the length */
    len = lseek(fd, 0, SEEK_END);

    /* Mmap the file into memory */
    if((data = (unsigned char*)mmap(0, len, PROT_READ, MAP_FILE |
                                            MAP_PRIVATE, fd, 0)) == MAP_FAILED)
        goto error;

    /* Locate the end of central directory record by searching backwards for
       the record signature. */

    if(len < END_CEN_LEN)
        goto error2;
        
    for(pntr = data + len - END_CEN_LEN; pntr >= data; )
        if(*pntr == (END_CEN_SIG & 0xff))
            if(READ_LE_INT(pntr) == END_CEN_SIG)
                break;
            else
                pntr -= SIG_LEN;
        else
            pntr--;

    /* Check that we found it */
    if(pntr < data)
        goto error2;

    /* Get the number of entries in the central directory */
    entries = READ_LE_SHORT(pntr + END_CEN_ENTRIES_OFFSET);

    /* Create and initialise hash table to hold the directory.
       Do not create lock -- we're single threaded (bootstrapping)
       and once entries are added, table is read-only */

    hash_table = sysMalloc(sizeof(HashTable));
    initHashTable((*hash_table), HASHTABSZE, FALSE);

    /* Get the offset from the start of the file of the first directory entry */
    pntr = data + READ_LE_INT(pntr + END_CEN_DIR_START_OFFSET);

    /* Scan the directory list and add the entries to the hash table */

    while(entries--) {
        char *found, *pathname;
        int path_len, comment_len, extra_len;

        /* Make sure we're not reading outside the file */
        if((pntr + CEN_FILE_HEADER_LEN) > (data + len))
            goto error2;

        /* Check directory entry signature is present */
        if(READ_LE_INT(pntr) != CEN_FILE_HEADER_SIG)
            goto error2;

        /* Get the length of the pathname */
        path_len = READ_LE_SHORT(pntr + CEN_FILE_PATHLEN_OFFSET);

        /* Not interested in these fields but need length to skip */
        extra_len = READ_LE_SHORT(pntr + CEN_FILE_EXTRALEN_OFFSET);
        comment_len = READ_LE_SHORT(pntr + CEN_FILE_COMMENTLEN_OFFSET);

        /* The pathname starts after the fixed part of the dir entry */
        pathname = (char*)(pntr += CEN_FILE_HEADER_LEN);

        /* Skip variable fields, to point to next sig */
        pntr += path_len + extra_len + comment_len;

        /* Add if absent, no scavenge, not locked */
        findHashEntry((*hash_table), pathname, found, TRUE, FALSE, FALSE);
    }

    zip = sysMalloc(sizeof(ZipFile));

    zip->data = data;
    zip->length = len;
    zip->dir_hash = hash_table;

    return zip;

error2:
    munmap(data, len);

error:
    close(fd);
    return NULL;
}

#undef HASH
#undef COMPARE

#define HASH(ptr) utf8Hash(ptr)
#define COMPARE(ptr1, ptr2, hash1, hash2) ((hash1 == hash2) && \
                                           utf8ZipComp(ptr1, ptr2)) 

/* When searching for an entry, the comparison will be between a null-
   terminated utf8 path and a non null-terminated path within the zip.
   Again, the memcpy is not performance critical, as comparisons are
   performed using the hash value; a filename comparison is only
   done when hash values clash (which is rare)
*/
int utf8ZipComp(char *path1, char *path2) {
    int path2_len = READ_LE_SHORT((unsigned char*)path2 +
                                          (CEN_FILE_PATHLEN_OFFSET -
                                           CEN_FILE_HEADER_LEN));
    char buff[path2_len + 1];

    memcpy(buff, path2, path2_len);
    buff[path2_len] = '\0';

    return utf8Comp(path1, buff);
}

char *findArchiveDirEntry(char *pathname, ZipFile *zip) {
    char *found;

    /* Do not add if absent, no scavenge, not locked */
    findHashEntry((*zip->dir_hash), pathname, found, FALSE, FALSE, FALSE);

    return found;
}

char *findArchiveEntry(char *pathname, ZipFile *zip, int *uncomp_len) {
    int offset, path_len, comp_len, extra_len, comp_method;
    unsigned char *decomp_buff, *comp_data;
    unsigned char *dir_entry;

    dir_entry = (unsigned char *)findArchiveDirEntry(pathname, zip);
    if(dir_entry == NULL)
        return NULL;

    /* Found the file -- the pathname points directly into the
       directory entry.  Read the values relative to it */

    /* Offset of the file entry relative to the start of the file */
    offset = READ_LE_INT(dir_entry + (CEN_FILE_LOCALHDR_OFFSET -
                                      CEN_FILE_HEADER_LEN));

    if((offset + LOC_FILE_HEADER_LEN) > zip->length)
        return NULL;

    /* Get the variable length part of the local file header */
    extra_len = READ_LE_SHORT(zip->data + offset + LOC_FILE_EXTRA_OFFSET);

    /* Get the path_len again */
    path_len = READ_LE_SHORT(dir_entry + (CEN_FILE_PATHLEN_OFFSET -
                                          CEN_FILE_HEADER_LEN));

    /* The file's length when uncompressed -- this is passed out */
    *uncomp_len = READ_LE_INT(dir_entry + (CEN_FILE_UNCOMPLEN_OFFSET -
                                           CEN_FILE_HEADER_LEN));

    /* The compressed file's length, i.e. the data size in the file */
    comp_len = READ_LE_INT(dir_entry + (CEN_FILE_COMPLEN_OFFSET -
                                        CEN_FILE_HEADER_LEN));

    /* How the file is compressed */
    comp_method = READ_LE_SHORT(dir_entry + (CEN_FILE_COMPMETH_OFFSET -
                                             CEN_FILE_HEADER_LEN));

    /* Calculate the data start */
    offset += LOC_FILE_HEADER_LEN + path_len + extra_len;

    /* Make sure we're not reading outside the file */
    if((offset + comp_len) > zip->length)
        return NULL;

    comp_data = zip->data + offset;
    decomp_buff = sysMalloc(*uncomp_len);

    switch(comp_method) {
        case COMP_STORED:
            /* Data isn't compressed, so just return it "as is" */
            memcpy(decomp_buff, comp_data, comp_len);
            return (char*)decomp_buff;

        case COMP_DEFLATED: {
            z_stream stream;
            int err;

            stream.next_in = comp_data;
            stream.avail_in = comp_len;
            stream.next_out = decomp_buff;
            stream.avail_out = *uncomp_len;

            stream.zalloc = Z_NULL;
            stream.zfree = Z_NULL;

            /* Use a negative windowBits value to stop the inflator looking
               for a header */
            if(inflateInit2(&stream, -MAX_WINDOW_BITS) != Z_OK)
                goto error;

            err = inflate(&stream, Z_SYNC_FLUSH);
            inflateEnd(&stream);

            if(err == Z_STREAM_END || (err == Z_OK && stream.avail_in == 0))
                return (char*)decomp_buff;
            break;
        }

        default:
            break;
    }

error:
    sysFree(decomp_buff);
    return NULL;
}
