/*
 *  Host Resources MIB - storage group interface - hr_system.h
 *
 */
#ifndef _MIBGROUP_HRSTORAGE_H
#define _MIBGROUP_HRSTORAGE_H

extern void     init_hr_storage(void);
extern FindVarMethod var_hrstore;

#define	HRS_TYPE_FS_MAX		100     /* Maximum # of filesystems supported */

#define	HRS_TYPE_MEM		101     /* incrementally from FS_MAX */
#define	HRS_TYPE_SWAP		102
#define	HRS_TYPE_MBUF		103
                                        /*
                                         * etc, etc, etc 
                                         */
#define	HRS_TYPE_MAX		104     /* one greater than largest type */

#endif                          /* _MIBGROUP_HRSTORAGE_H */
