AM_TESTS_ENVIRONMENT = XXD_FOUND=@XXD_FOUND@; export XXD_FOUND;

if AUTOMAKE_TEST_COMPAT
TESTS_ENVIRONMENT = $(AM_TESTS_ENVIRONMENT)
endif

TESTS = referenceFAT12.mkfs              \
	referenceFAT16.mkfs              \
	referenceFAT32.mkfs              \
	referenceFAT32mbr.mkfs           \
	mkfs-fat32_2_res_sects.mkfs      \
	mkfs-fat32_4K.mkfs               \
	check-bad_names.fsck             \
	check-fat12_first_cluster.fsck   \
	check-fat16_first_cluster.fsck   \
	check-fat32_first_cluster.fsck   \
	check-fat16_dos_cln_shut.fsck    \
	check-fat32_dos_cln_shut.fsck    \
	check-chain_to_free_cluster.fsck \
	check-chain_too_long.fsck        \
	check-chain_to_other_file.fsck   \
	check-circular_chain.fsck        \
	check-duplicate_names.fsck       \
	check-dot_entries.fsck           \
	check-huge.fsck                  \
	check-label-different.fsck       \
	check-label-only-boot.fsck       \
	check-label-only-root.fsck       \
	label-fat32_mkdosfs_label1_dosfslabel_empty.label          \
	label-fat32_mkdosfs_label1_dosfslabel_label2.label         \
	label-fat32_mkdosfs_label1_dosfslabel_NO_NAME.label        \
	label-fat32_mkdosfs_label1_mlabel_erase.label              \
	label-fat32_mkdosfs_label1_mlabel_NO_NAME.label            \
	label-fat32_mkdosfs_label1_xp_erase.label                  \
	label-fat32_mkdosfs_label1_xp_label2.label                 \
	label-fat32_mkdosfs_label1.label                           \
	label-fat32_mkdosfs_none_dosfslabel_label1_xp_label2.label \
	label-fat32_mkdosfs_none_dosfslabel_label1.label           \
	label-fat32_mkdosfs_none_dosfslabel_NO_NAME.label          \
	label-fat32_mkdosfs_none_xp_label1_dosfslabel_label2.label \
	label-fat32_mkdosfs_none_xp_label1.label                   \
	label-fat32_mkdosfs_none.label                             \
	label-fat32_xp_label1.label                                \
	label-fat32_xp_none_dosfslabel_label1.label                \
	label-fat32_xp_none_mlabel_label1.label                    \
	label-fat32_xp_none.label


XFAIL_TESTS = check-huge.fsck

TEST_EXTENSIONS = .mkfs .fsck .label
MKFS_LOG_COMPILER = $(srcdir)/test-mkfs
FSCK_LOG_COMPILER = $(srcdir)/test-fsck
LABEL_LOG_COMPILER = $(srcdir)/test-label

dist_check_DATA = test-mkfs test-fsck test-label   \
		  referenceFAT12.mkfs              \
		  referenceFAT12.xxd               \
		  referenceFAT16.mkfs              \
		  referenceFAT16.xxd               \
		  referenceFAT32.mkfs              \
		  referenceFAT32.xxd               \
		  referenceFAT32mbr.mkfs           \
		  referenceFAT32mbr.xxd            \
		  mkfs-fat32_2_res_sects.mkfs      \
		  mkfs-fat32_2_res_sects.xxd       \
		  mkfs-fat32_4K.mkfs               \
		  mkfs-fat32_4K.xxd                \
		  check-bad_names.fsck             \
		  check-bad_names.xxd              \
		  check-fat12_first_cluster.fsck   \
		  check-fat12_first_cluster.args   \
		  check-fat12_first_cluster.xxd    \
		  check-fat16_first_cluster.fsck   \
		  check-fat16_first_cluster.args   \
		  check-fat16_first_cluster.xxd    \
		  check-fat32_first_cluster.fsck   \
		  check-fat32_first_cluster.args   \
		  check-fat32_first_cluster.xxd    \
		  check-fat16_dos_cln_shut.fsck    \
		  check-fat16_dos_cln_shut.xxd     \
		  check-fat32_dos_cln_shut.fsck    \
		  check-fat32_dos_cln_shut.xxd     \
		  check-chain_to_free_cluster.fsck \
		  check-chain_to_free_cluster.xxd  \
		  check-chain_too_long.fsck        \
		  check-chain_too_long.xxd         \
		  check-chain_to_other_file.fsck   \
		  check-chain_to_other_file.xxd    \
		  check-circular_chain.fsck        \
		  check-circular_chain.xxd         \
		  check-duplicate_names.fsck       \
		  check-duplicate_names.xxd        \
		  check-dot_entries.fsck           \
		  check-dot_entries.xxd            \
		  check-huge.fsck                  \
		  check-label-different.fsck       \
		  check-label-different.xxd        \
		  check-label-only-boot.fsck       \
		  check-label-only-boot.xxd        \
		  check-label-only-root.fsck       \
		  check-label-only-root.xxd        \
		  label-fat32_mkdosfs_label1_dosfslabel_empty.label          \
		  label-fat32_mkdosfs_label1_dosfslabel_empty.xxd            \
		  label-fat32_mkdosfs_label1_dosfslabel_label2.label         \
		  label-fat32_mkdosfs_label1_dosfslabel_label2.xxd           \
		  label-fat32_mkdosfs_label1_dosfslabel_NO_NAME.label        \
		  label-fat32_mkdosfs_label1_dosfslabel_NO_NAME.xxd          \
		  label-fat32_mkdosfs_label1_mlabel_erase.label              \
		  label-fat32_mkdosfs_label1_mlabel_erase.xxd                \
		  label-fat32_mkdosfs_label1_mlabel_NO_NAME.label            \
		  label-fat32_mkdosfs_label1_mlabel_NO_NAME.xxd              \
		  label-fat32_mkdosfs_label1_xp_erase.label                  \
		  label-fat32_mkdosfs_label1_xp_erase.xxd                    \
		  label-fat32_mkdosfs_label1_xp_label2.label                 \
		  label-fat32_mkdosfs_label1_xp_label2.xxd                   \
		  label-fat32_mkdosfs_label1.label                           \
		  label-fat32_mkdosfs_label1.xxd                             \
		  label-fat32_mkdosfs_none_dosfslabel_label1_xp_label2.label \
		  label-fat32_mkdosfs_none_dosfslabel_label1_xp_label2.xxd   \
		  label-fat32_mkdosfs_none_dosfslabel_label1.label           \
		  label-fat32_mkdosfs_none_dosfslabel_label1.xxd             \
		  label-fat32_mkdosfs_none_dosfslabel_NO_NAME.label          \
		  label-fat32_mkdosfs_none_dosfslabel_NO_NAME.xxd            \
		  label-fat32_mkdosfs_none_xp_label1_dosfslabel_label2.label \
		  label-fat32_mkdosfs_none_xp_label1_dosfslabel_label2.xxd   \
		  label-fat32_mkdosfs_none_xp_label1.label                   \
		  label-fat32_mkdosfs_none_xp_label1.xxd                     \
		  label-fat32_mkdosfs_none.label                             \
		  label-fat32_mkdosfs_none.xxd                               \
		  label-fat32_xp_label1.label                                \
		  label-fat32_xp_label1.xxd                                  \
		  label-fat32_xp_none_dosfslabel_label1.label                \
		  label-fat32_xp_none_dosfslabel_label1.xxd                  \
		  label-fat32_xp_none_mlabel_label1.label                    \
		  label-fat32_xp_none_mlabel_label1.xxd                      \
		  label-fat32_xp_none.label                                  \
		  label-fat32_xp_none.xxd
