/*
 * These are exported solely for the purpose of mtd_blkdevs.c and mtdchar.c.
 * You should not use them for _anything_ else.
 */


struct squashfs_super_block {
	__le32			s_magic;
	__le32			inodes;
	__le32			mkfs_time;
	__le32			block_size;
	__le32			fragments;
	__le16			compression;
	__le16			block_log;
	__le16			flags;
	__le16			no_ids;
	__le16			s_major;
	__le16			s_minor;
	__le64			root_inode;
	__le64			bytes_used;
	__le64			id_table_start;
	__le64			xattr_id_table_start;
	__le64			inode_table_start;
	__le64			directory_table_start;
	__le64			fragment_table_start;
	__le64			lookup_table_start;
};


extern struct mutex mtd_table_mutex;

struct mtd_info *__mtd_next_device(int i);
int __must_check add_mtd_device(struct mtd_info *mtd);
int del_mtd_device(struct mtd_info *mtd);
int add_mtd_partitions(struct mtd_info *, const struct mtd_partition *, int);
int del_mtd_partitions(struct mtd_info *);

struct mtd_partitions;

int parse_mtd_partitions(struct mtd_info *master, const char * const *types,
			 struct mtd_partitions *pparts,
			 struct mtd_part_parser_data *data);

void mtd_part_parser_cleanup(struct mtd_partitions *parts);

int __init init_mtdchar(void);
void __exit cleanup_mtdchar(void);

#define mtd_for_each_device(mtd)			\
	for ((mtd) = __mtd_next_device(0);		\
	     (mtd) != NULL;				\
	     (mtd) = __mtd_next_device(mtd->index + 1))
