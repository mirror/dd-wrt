/*
  +----------------------------------------------------------------------+
  | phar php single-file executable PHP extension                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 2006-2014 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt.                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Gregory Beaver <cellog@php.net>                             |
  |          Marcus Boerger <helly@php.net>                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>
#include "php.h"
#include "tar.h"
#include "php_ini.h"
#include "zend_constants.h"
#include "zend_execute.h"
#include "zend_exceptions.h"
#include "zend_hash.h"
#include "zend_interfaces.h"
#include "zend_operators.h"
#include "zend_qsort.h"
#include "zend_vm.h"
#include "main/php_streams.h"
#include "main/streams/php_stream_plain_wrapper.h"
#include "main/SAPI.h"
#include "main/php_main.h"
#include "main/php_open_temporary_file.h"
#include "ext/standard/info.h"
#include "ext/standard/basic_functions.h"
#include "ext/standard/file.h"
#include "ext/standard/php_string.h"
#include "ext/standard/url.h"
#include "ext/standard/crc32.h"
#include "ext/standard/md5.h"
#include "ext/standard/sha1.h"
#include "ext/standard/php_var.h"
#include "ext/standard/php_smart_str.h"
#include "ext/standard/php_versioning.h"
#ifndef PHP_WIN32
#include "TSRM/tsrm_strtok_r.h"
#endif
#include "TSRM/tsrm_virtual_cwd.h"
#if HAVE_SPL
#include "ext/spl/spl_array.h"
#include "ext/spl/spl_directory.h"
#include "ext/spl/spl_engine.h"
#include "ext/spl/spl_exceptions.h"
#include "ext/spl/spl_iterators.h"
#endif
#include "php_phar.h"
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef PHAR_HASH_OK
#include "ext/hash/php_hash.h"
#include "ext/hash/php_hash_sha.h"
#endif

#ifndef E_RECOVERABLE_ERROR
# define E_RECOVERABLE_ERROR E_ERROR
#endif

#ifndef pestrndup
# define pestrndup(s, length, persistent) ((persistent)?zend_strndup((s),(length)):estrndup((s),(length)))
#endif

#ifndef ALLOC_PERMANENT_ZVAL
# define ALLOC_PERMANENT_ZVAL(z) \
	(z) = (zval*)malloc(sizeof(zval))
#endif

/* PHP_ because this is public information via MINFO */
#define PHP_PHAR_API_VERSION      "1.1.1"
/* x.y.z maps to 0xyz0 */
#define PHAR_API_VERSION          0x1110
/* if we bump PHAR_API_VERSION, change this from 0x1100 to PHAR_API_VERSION */
#define PHAR_API_VERSION_NODIR    0x1100
#define PHAR_API_MIN_DIR          0x1110
#define PHAR_API_MIN_READ         0x1000
#define PHAR_API_MAJORVERSION     0x1000
#define PHAR_API_MAJORVER_MASK    0xF000
#define PHAR_API_VER_MASK         0xFFF0

#define PHAR_HDR_COMPRESSION_MASK 0x0000F000
#define PHAR_HDR_COMPRESSED_NONE  0x00000000
#define PHAR_HDR_COMPRESSED_GZ    0x00001000
#define PHAR_HDR_COMPRESSED_BZ2   0x00002000
#define PHAR_HDR_SIGNATURE        0x00010000

/* flags for defining that the entire file should be compressed */
#define PHAR_FILE_COMPRESSION_MASK 0x00F00000
#define PHAR_FILE_COMPRESSED_NONE  0x00000000
#define PHAR_FILE_COMPRESSED_GZ    0x00100000
#define PHAR_FILE_COMPRESSED_BZ2   0x00200000

#define PHAR_SIG_MD5              0x0001
#define PHAR_SIG_SHA1             0x0002
#define PHAR_SIG_SHA256           0x0003
#define PHAR_SIG_SHA512           0x0004
#define PHAR_SIG_OPENSSL          0x0010

/* flags byte for each file adheres to these bitmasks.
   All unused values are reserved */
#define PHAR_ENT_COMPRESSION_MASK 0x0000F000
#define PHAR_ENT_COMPRESSED_NONE  0x00000000
#define PHAR_ENT_COMPRESSED_GZ    0x00001000
#define PHAR_ENT_COMPRESSED_BZ2   0x00002000

#define PHAR_ENT_PERM_MASK        0x000001FF
#define PHAR_ENT_PERM_MASK_USR    0x000001C0
#define PHAR_ENT_PERM_SHIFT_USR   6
#define PHAR_ENT_PERM_MASK_GRP    0x00000038
#define PHAR_ENT_PERM_SHIFT_GRP   3
#define PHAR_ENT_PERM_MASK_OTH    0x00000007
#define PHAR_ENT_PERM_DEF_FILE    0x000001B6
#define PHAR_ENT_PERM_DEF_DIR     0x000001FF

#define PHAR_FORMAT_SAME    0
#define PHAR_FORMAT_PHAR    1
#define PHAR_FORMAT_TAR     2
#define PHAR_FORMAT_ZIP     3

#define TAR_FILE    '0'
#define TAR_LINK    '1'
#define TAR_SYMLINK '2'
#define TAR_DIR     '5'
#define TAR_NEW     '8'

#define PHAR_MUNG_PHP_SELF			(1<<0)
#define PHAR_MUNG_REQUEST_URI		(1<<1)
#define PHAR_MUNG_SCRIPT_NAME		(1<<2)
#define PHAR_MUNG_SCRIPT_FILENAME	(1<<3)

typedef struct _phar_entry_fp phar_entry_fp;
typedef struct _phar_archive_data phar_archive_data;

ZEND_BEGIN_MODULE_GLOBALS(phar)
	/* a list of phar_archive_data objects that reference a cached phar, so
	   that if copy-on-write is performed, we can swap them out for the new value */
	HashTable   phar_persist_map;
	HashTable   phar_fname_map;
	/* for cached phars, this is a per-process store of fp/ufp */
	phar_entry_fp *cached_fp;
	HashTable   phar_alias_map;
	int         phar_SERVER_mung_list;
	int         readonly;
	char*       cache_list;
	int         manifest_cached;
	int         persist;
	int         has_zlib;
	int         has_bz2;
	zend_bool   readonly_orig;
	zend_bool   require_hash_orig;
	zend_bool   intercepted;
	int         request_init;
	int         require_hash;
	int         request_done;
	int         request_ends;
	void        (*orig_fopen)(INTERNAL_FUNCTION_PARAMETERS);
	void        (*orig_file_get_contents)(INTERNAL_FUNCTION_PARAMETERS);
	void        (*orig_is_file)(INTERNAL_FUNCTION_PARAMETERS);
	void        (*orig_is_link)(INTERNAL_FUNCTION_PARAMETERS);
	void        (*orig_is_dir)(INTERNAL_FUNCTION_PARAMETERS);
	void        (*orig_opendir)(INTERNAL_FUNCTION_PARAMETERS);
	void        (*orig_file_exists)(INTERNAL_FUNCTION_PARAMETERS);
	void        (*orig_fileperms)(INTERNAL_FUNCTION_PARAMETERS);
	void        (*orig_fileinode)(INTERNAL_FUNCTION_PARAMETERS);
	void        (*orig_filesize)(INTERNAL_FUNCTION_PARAMETERS);
	void        (*orig_fileowner)(INTERNAL_FUNCTION_PARAMETERS);
	void        (*orig_filegroup)(INTERNAL_FUNCTION_PARAMETERS);
	void        (*orig_fileatime)(INTERNAL_FUNCTION_PARAMETERS);
	void        (*orig_filemtime)(INTERNAL_FUNCTION_PARAMETERS);
	void        (*orig_filectime)(INTERNAL_FUNCTION_PARAMETERS);
	void        (*orig_filetype)(INTERNAL_FUNCTION_PARAMETERS);
	void        (*orig_is_writable)(INTERNAL_FUNCTION_PARAMETERS);
	void        (*orig_is_readable)(INTERNAL_FUNCTION_PARAMETERS);
	void        (*orig_is_executable)(INTERNAL_FUNCTION_PARAMETERS);
	void        (*orig_lstat)(INTERNAL_FUNCTION_PARAMETERS);
	void        (*orig_readfile)(INTERNAL_FUNCTION_PARAMETERS);
	void        (*orig_stat)(INTERNAL_FUNCTION_PARAMETERS);
	/* used for includes with . in them inside front controller */
	char*       cwd;
	int         cwd_len;
	int         cwd_init;
	char        *openssl_privatekey;
	int         openssl_privatekey_len;
	/* phar_get_archive cache */
	char*       last_phar_name;
	int         last_phar_name_len;
	char*       last_alias;
	int         last_alias_len;
	phar_archive_data* last_phar;
	HashTable mime_types;
ZEND_END_MODULE_GLOBALS(phar)

ZEND_EXTERN_MODULE_GLOBALS(phar)

#ifdef ZTS
#	include "TSRM.h"
#	define PHAR_G(v) TSRMG(phar_globals_id, zend_phar_globals *, v)
#	define PHAR_GLOBALS ((zend_phar_globals *) (*((void ***) tsrm_ls))[TSRM_UNSHUFFLE_RSRC_ID(phar_globals_id)])
#else
#	define PHAR_G(v) (phar_globals.v)
#	define PHAR_GLOBALS (&phar_globals)
#endif

#ifndef php_uint16
# if SIZEOF_SHORT == 2
#  define php_uint16 unsigned short
# else
#  define php_uint16 uint16_t
# endif
#endif
#include "pharzip.h"

#if HAVE_SPL
typedef union _phar_archive_object  phar_archive_object;
typedef union _phar_entry_object    phar_entry_object;
#endif

/*
 * used in phar_entry_info->fp_type to
 */
enum phar_fp_type {
	/* regular file pointer phar_archive_data->fp */
	PHAR_FP,
	/* uncompressed file pointer phar_archive_data->uncompressed_fp */
	PHAR_UFP,
	/* modified file pointer phar_entry_info->fp */
	PHAR_MOD,
	/* temporary manifest entry (file outside of the phar mapped to a location inside the phar)
	   this entry stores the stream to open in link (normally used for tars, but we steal it here) */
	PHAR_TMP
};

/* entry for one file in a phar file */
typedef struct _phar_entry_info {
	/* first bytes are exactly as in file */
	php_uint32               uncompressed_filesize;
	php_uint32               timestamp;
	php_uint32               compressed_filesize;
	php_uint32               crc32;
	php_uint32               flags;
	/* remainder */
	/* when changing compression, save old flags in case fp is NULL */
	php_uint32               old_flags;
	zval                     *metadata;
	int                      metadata_len; /* only used for cached manifests */
	php_uint32               filename_len;
	char                     *filename;
	enum phar_fp_type        fp_type;
	/* offset within original phar file of the file contents */
	long                     offset_abs;
	/* offset within fp of the file contents */
	long                     offset;
	/* offset within original phar file of the file header (for zip-based/tar-based) */
	long                     header_offset;
	php_stream               *fp;
	php_stream               *cfp;
	int                      fp_refcount;
	char                     *tmp;
	phar_archive_data        *phar;
	smart_str                metadata_str;
	char                     *link; /* symbolic link to another file */
	char                     tar_type;
	/* position in the manifest */
	uint                     manifest_pos;
	/* for stat */
	unsigned short           inode;

	unsigned int             is_crc_checked:1;
	unsigned int             is_modified:1;
	unsigned int             is_deleted:1;
	unsigned int             is_dir:1;
	/* this flag is used for mounted entries (external files mapped to location
	   inside a phar */
	unsigned int             is_mounted:1;
	/* used when iterating */
	unsigned int             is_temp_dir:1;
	/* tar-based phar file stuff */
	unsigned int             is_tar:1;
	/* zip-based phar file stuff */
	unsigned int             is_zip:1;
	/* for cached phar entries */
	unsigned int             is_persistent:1;
} phar_entry_info;

/* information about a phar file (the archive itself) */
struct _phar_archive_data {
	char                     *fname;
	int                      fname_len;
	/* for phar_detect_fname_ext, this stores the location of the file extension within fname */
	char                     *ext;
	int                      ext_len;
	char                     *alias;
	int                      alias_len;
	char                     version[12];
	size_t                   internal_file_start;
	size_t                   halt_offset;
	HashTable                manifest;
	/* hash of virtual directories, as in path/to/file.txt has path/to and path as virtual directories */
	HashTable                virtual_dirs;
	/* hash of mounted directory paths */
	HashTable                mounted_dirs;
	php_uint32               flags;
	php_uint32               min_timestamp;
	php_uint32               max_timestamp;
	php_stream               *fp;
	/* decompressed file contents are stored here */
	php_stream               *ufp;
	int                      refcount;
	php_uint32               sig_flags;
	int                      sig_len;
	char                     *signature;
	zval                     *metadata;
	int                      metadata_len; /* only used for cached manifests */
	uint                     phar_pos;
	/* if 1, then this alias was manually specified by the user and is not a permanent alias */
	unsigned int             is_temporary_alias:1;
	unsigned int             is_modified:1;
	unsigned int             is_writeable:1;
	unsigned int             is_brandnew:1;
	/* defer phar creation */
	unsigned int             donotflush:1;
	/* zip-based phar variables */
	unsigned int             is_zip:1;
	/* tar-based phar variables */
	unsigned int             is_tar:1;
	/* PharData variables       */
	unsigned int             is_data:1;
	/* for cached phar manifests */
	unsigned int             is_persistent:1;
};

typedef struct _phar_entry_fp_info {
	enum phar_fp_type        fp_type;
	/* offset within fp of the file contents */
	long                     offset;
} phar_entry_fp_info;

struct _phar_entry_fp {
	php_stream *fp;
	php_stream *ufp;
	phar_entry_fp_info *manifest;
};

static inline php_stream *phar_get_entrypfp(phar_entry_info *entry TSRMLS_DC)
{
	if (!entry->is_persistent) {
		return entry->phar->fp;
	}
	return PHAR_GLOBALS->cached_fp[entry->phar->phar_pos].fp;
}

static inline php_stream *phar_get_entrypufp(phar_entry_info *entry TSRMLS_DC)
{
	if (!entry->is_persistent) {
		return entry->phar->ufp;
	}
	return PHAR_GLOBALS->cached_fp[entry->phar->phar_pos].ufp;
}

static inline void phar_set_entrypfp(phar_entry_info *entry, php_stream *fp TSRMLS_DC)
{
	if (!entry->phar->is_persistent) {
		entry->phar->fp =  fp;
		return;
	}

	PHAR_GLOBALS->cached_fp[entry->phar->phar_pos].fp = fp;
}

static inline void phar_set_entrypufp(phar_entry_info *entry, php_stream *fp TSRMLS_DC)
{
	if (!entry->phar->is_persistent) {
		entry->phar->ufp =  fp;
		return;
	}

	PHAR_GLOBALS->cached_fp[entry->phar->phar_pos].ufp = fp;
}

static inline php_stream *phar_get_pharfp(phar_archive_data *phar TSRMLS_DC)
{
	if (!phar->is_persistent) {
		return phar->fp;
	}
	return PHAR_GLOBALS->cached_fp[phar->phar_pos].fp;
}

static inline php_stream *phar_get_pharufp(phar_archive_data *phar TSRMLS_DC)
{
	if (!phar->is_persistent) {
		return phar->ufp;
	}
	return PHAR_GLOBALS->cached_fp[phar->phar_pos].ufp;
}

static inline void phar_set_pharfp(phar_archive_data *phar, php_stream *fp TSRMLS_DC)
{
	if (!phar->is_persistent) {
		phar->fp =  fp;
		return;
	}

	PHAR_GLOBALS->cached_fp[phar->phar_pos].fp = fp;
}

static inline void phar_set_pharufp(phar_archive_data *phar, php_stream *fp TSRMLS_DC)
{
	if (!phar->is_persistent) {
		phar->ufp =  fp;
		return;
	}

	PHAR_GLOBALS->cached_fp[phar->phar_pos].ufp = fp;
}

static inline void phar_set_fp_type(phar_entry_info *entry, enum phar_fp_type type, off_t offset TSRMLS_DC)
{
	phar_entry_fp_info *data;

	if (!entry->is_persistent) {
		entry->fp_type = type;
		entry->offset = offset;
		return;
	}
	data = &(PHAR_GLOBALS->cached_fp[entry->phar->phar_pos].manifest[entry->manifest_pos]);
	data->fp_type = type;
	data->offset = offset;
}

static inline enum phar_fp_type phar_get_fp_type(phar_entry_info *entry TSRMLS_DC)
{
	if (!entry->is_persistent) {
		return entry->fp_type;
	}
	return PHAR_GLOBALS->cached_fp[entry->phar->phar_pos].manifest[entry->manifest_pos].fp_type;
}

static inline off_t phar_get_fp_offset(phar_entry_info *entry TSRMLS_DC)
{
	if (!entry->is_persistent) {
		return entry->offset;
	}
	if (PHAR_GLOBALS->cached_fp[entry->phar->phar_pos].manifest[entry->manifest_pos].fp_type == PHAR_FP) {
		if (!PHAR_GLOBALS->cached_fp[entry->phar->phar_pos].manifest[entry->manifest_pos].offset) {
			PHAR_GLOBALS->cached_fp[entry->phar->phar_pos].manifest[entry->manifest_pos].offset = entry->offset;
		}
	}
	return PHAR_GLOBALS->cached_fp[entry->phar->phar_pos].manifest[entry->manifest_pos].offset;
}

#define PHAR_MIME_PHP '\0'
#define PHAR_MIME_PHPS '\1'
#define PHAR_MIME_OTHER '\2'

typedef struct _phar_mime_type {
	char *mime;
	int len;
	/* one of PHAR_MIME_* */
	char type;
} phar_mime_type;

/* stream access data for one file entry in a phar file */
typedef struct _phar_entry_data {
	phar_archive_data        *phar;
	php_stream               *fp;
	/* stream position proxy, allows multiple open streams referring to the same fp */
	off_t                    position;
	/* for copies of the phar fp, defines where 0 is */
	off_t                    zero;
	unsigned int             for_write:1;
	unsigned int             is_zip:1;
	unsigned int             is_tar:1;
	phar_entry_info          *internal_file;
} phar_entry_data;

#if HAVE_SPL
/* archive php object */
union _phar_archive_object {
	zend_object              std;
	spl_filesystem_object    spl;
	struct {
		zend_object          std;
		phar_archive_data    *archive;
	} arc;
};
#endif

#if HAVE_SPL
/* entry php object */
union _phar_entry_object {
	zend_object              std;
	spl_filesystem_object    spl;
	struct {
		zend_object          std;
		phar_entry_info      *entry;
	} ent;
};
#endif

#ifndef PHAR_MAIN
extern char *(*phar_save_resolve_path)(const char *filename, int filename_len TSRMLS_DC);
#endif

# define phar_stream_copy_to_stream(src, dest, maxlen, len)	_php_stream_copy_to_stream_ex((src), (dest), (maxlen), (len) STREAMS_CC TSRMLS_CC)

typedef char *phar_zstr;
#define PHAR_STR(a, b)	\
	b = a;
#define PHAR_ZSTR(a, b)	\
	b = a;
#define PHAR_STR_FREE(a)

BEGIN_EXTERN_C()

#ifdef PHP_WIN32
char *tsrm_strtok_r(char *s, const char *delim, char **last);

static inline void phar_unixify_path_separators(char *path, int path_len)
{
	char *s;

	/* unixify win paths */
	for (s = path; s - path < path_len; ++s) {
		if (*s == '\\') {
			*s = '/';
		}
	}
}
#endif
/**
 * validate an alias, returns 1 for success, 0 for failure
 */
static inline int phar_validate_alias(const char *alias, int alias_len) /* {{{ */
{
	return !(memchr(alias, '/', alias_len) || memchr(alias, '\\', alias_len) || memchr(alias, ':', alias_len) ||
		memchr(alias, ';', alias_len) || memchr(alias, '\n', alias_len) || memchr(alias, '\r', alias_len));
}
/* }}} */

static inline void phar_set_inode(phar_entry_info *entry TSRMLS_DC) /* {{{ */
{
	char tmp[MAXPATHLEN];
	int tmp_len;

	tmp_len = entry->filename_len + entry->phar->fname_len;
	memcpy(tmp, entry->phar->fname, entry->phar->fname_len);
	memcpy(tmp + entry->phar->fname_len, entry->filename, entry->filename_len);
	entry->inode = (unsigned short)zend_get_hash_value(tmp, tmp_len);
}
/* }}} */

void phar_request_initialize(TSRMLS_D);

void phar_object_init(TSRMLS_D);
void phar_destroy_phar_data(phar_archive_data *phar TSRMLS_DC);

int phar_open_entry_file(phar_archive_data *phar, phar_entry_info *entry, char **error TSRMLS_DC);
int phar_postprocess_file(phar_entry_data *idata, php_uint32 crc32, char **error, int process_zip TSRMLS_DC);
int phar_open_from_filename(char *fname, int fname_len, char *alias, int alias_len, int options, phar_archive_data** pphar, char **error TSRMLS_DC);
int phar_open_or_create_filename(char *fname, int fname_len, char *alias, int alias_len, int is_data, int options, phar_archive_data** pphar, char **error TSRMLS_DC);
int phar_create_or_parse_filename(char *fname, int fname_len, char *alias, int alias_len, int is_data, int options, phar_archive_data** pphar, char **error TSRMLS_DC);
int phar_open_executed_filename(char *alias, int alias_len, char **error TSRMLS_DC);
int phar_free_alias(phar_archive_data *phar, char *alias, int alias_len TSRMLS_DC);
int phar_get_archive(phar_archive_data **archive, char *fname, int fname_len, char *alias, int alias_len, char **error TSRMLS_DC);
int phar_open_parsed_phar(char *fname, int fname_len, char *alias, int alias_len, int is_data, int options, phar_archive_data** pphar, char **error TSRMLS_DC);
int phar_verify_signature(php_stream *fp, size_t end_of_phar, php_uint32 sig_type, char *sig, int sig_len, char *fname, char **signature, int *signature_len, char **error TSRMLS_DC);
int phar_create_signature(phar_archive_data *phar, php_stream *fp, char **signature, int *signature_length, char **error TSRMLS_DC);

/* utility functions */
char *phar_create_default_stub(const char *index_php, const char *web_index, size_t *len, char **error TSRMLS_DC);
char *phar_decompress_filter(phar_entry_info * entry, int return_unknown);
char *phar_compress_filter(phar_entry_info * entry, int return_unknown);

void phar_remove_virtual_dirs(phar_archive_data *phar, char *filename, int filename_len TSRMLS_DC);
void phar_add_virtual_dirs(phar_archive_data *phar, char *filename, int filename_len TSRMLS_DC);
int phar_mount_entry(phar_archive_data *phar, char *filename, int filename_len, char *path, int path_len TSRMLS_DC);
char *phar_find_in_include_path(char *file, int file_len, phar_archive_data **pphar TSRMLS_DC);
char *phar_fix_filepath(char *path, int *new_len, int use_cwd TSRMLS_DC);
phar_entry_info * phar_open_jit(phar_archive_data *phar, phar_entry_info *entry, char **error TSRMLS_DC);
int phar_parse_metadata(char **buffer, zval **metadata, int zip_metadata_len TSRMLS_DC);
void destroy_phar_manifest_entry(void *pDest);
int phar_seek_efp(phar_entry_info *entry, off_t offset, int whence, off_t position, int follow_links TSRMLS_DC);
php_stream *phar_get_efp(phar_entry_info *entry, int follow_links TSRMLS_DC);
int phar_copy_entry_fp(phar_entry_info *source, phar_entry_info *dest, char **error TSRMLS_DC);
int phar_open_entry_fp(phar_entry_info *entry, char **error, int follow_links TSRMLS_DC);
phar_entry_info *phar_get_link_source(phar_entry_info *entry TSRMLS_DC);
int phar_create_writeable_entry(phar_archive_data *phar, phar_entry_info *entry, char **error TSRMLS_DC);
int phar_separate_entry_fp(phar_entry_info *entry, char **error TSRMLS_DC);
int phar_open_archive_fp(phar_archive_data *phar TSRMLS_DC);
int phar_copy_on_write(phar_archive_data **pphar TSRMLS_DC);

/* tar functions in tar.c */
int phar_is_tar(char *buf, char *fname);
int phar_parse_tarfile(php_stream* fp, char *fname, int fname_len, char *alias, int alias_len, phar_archive_data** pphar, int is_data, php_uint32 compression, char **error TSRMLS_DC);
int phar_open_or_create_tar(char *fname, int fname_len, char *alias, int alias_len, int is_data, int options, phar_archive_data** pphar, char **error TSRMLS_DC);
int phar_tar_flush(phar_archive_data *phar, char *user_stub, long len, int defaultstub, char **error TSRMLS_DC);

/* zip functions in zip.c */
int phar_parse_zipfile(php_stream *fp, char *fname, int fname_len, char *alias, int alias_len, phar_archive_data** pphar, char **error TSRMLS_DC);
int phar_open_or_create_zip(char *fname, int fname_len, char *alias, int alias_len, int is_data, int options, phar_archive_data** pphar, char **error TSRMLS_DC);
int phar_zip_flush(phar_archive_data *archive, char *user_stub, long len, int defaultstub, char **error TSRMLS_DC);

#ifdef PHAR_MAIN
static int phar_open_from_fp(php_stream* fp, char *fname, int fname_len, char *alias, int alias_len, int options, phar_archive_data** pphar, int is_data, char **error TSRMLS_DC);
extern php_stream_wrapper php_stream_phar_wrapper;
#else
extern HashTable cached_phars;
extern HashTable cached_alias;
#endif

int phar_archive_delref(phar_archive_data *phar TSRMLS_DC);
int phar_entry_delref(phar_entry_data *idata TSRMLS_DC);

phar_entry_info *phar_get_entry_info(phar_archive_data *phar, char *path, int path_len, char **error, int security TSRMLS_DC);
phar_entry_info *phar_get_entry_info_dir(phar_archive_data *phar, char *path, int path_len, char dir, char **error, int security TSRMLS_DC);
phar_entry_data *phar_get_or_create_entry_data(char *fname, int fname_len, char *path, int path_len, char *mode, char allow_dir, char **error, int security TSRMLS_DC);
int phar_get_entry_data(phar_entry_data **ret, char *fname, int fname_len, char *path, int path_len, char *mode, char allow_dir, char **error, int security TSRMLS_DC);
int phar_flush(phar_archive_data *archive, char *user_stub, long len, int convert, char **error TSRMLS_DC);
int phar_detect_phar_fname_ext(const char *filename, int filename_len, const char **ext_str, int *ext_len, int executable, int for_create, int is_complete TSRMLS_DC);
int phar_split_fname(char *filename, int filename_len, char **arch, int *arch_len, char **entry, int *entry_len, int executable, int for_create TSRMLS_DC);

typedef enum {
	pcr_use_query,
	pcr_is_ok,
	pcr_err_double_slash,
	pcr_err_up_dir,
	pcr_err_curr_dir,
	pcr_err_back_slash,
	pcr_err_star,
	pcr_err_illegal_char,
	pcr_err_empty_entry
} phar_path_check_result;

phar_path_check_result phar_path_check(char **p, int *len, const char **error);

END_EXTERN_C()

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
