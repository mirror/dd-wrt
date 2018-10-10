AM_TESTS_ENVIRONMENT = XXD_FOUND=@XXD_FOUND@; export XXD_FOUND;

if AUTOMAKE_TEST_COMPAT
TESTS_ENVIRONMENT = $(AM_TESTS_ENVIRONMENT)
endif

TESTS = referenceFAT12.mkfs              \
	referenceFAT16.mkfs              \
	referenceFAT32.mkfs              \
	check-bad_names.fsck             \
	check-chain_to_free_cluster.fsck \
	check-chain_too_long.fsck        \
	check-chain_to_other_file.fsck   \
	check-circular_chain.fsck        \
	check-duplicate_names.fsck       \
	check-dot_entries.fsck

XFAIL_TESTS = check-dot_entries.fsck

TEST_EXTENSIONS = .mkfs .fsck
MKFS_LOG_COMPILER = $(srcdir)/test-mkfs
FSCK_LOG_COMPILER = $(srcdir)/test-fsck

dist_check_DATA = test-mkfs test-fsck              \
		  referenceFAT12.mkfs              \
		  referenceFAT12.xxd               \
		  referenceFAT16.mkfs              \
		  referenceFAT16.xxd               \
		  referenceFAT32.mkfs              \
		  referenceFAT32.xxd               \
		  check-bad_names.fsck             \
		  check-chain_to_free_cluster.fsck \
		  check-chain_too_long.fsck        \
		  check-chain_to_other_file.fsck   \
		  check-circular_chain.fsck        \
		  check-duplicate_names.fsck       \
		  check-dot_entries.fsck
