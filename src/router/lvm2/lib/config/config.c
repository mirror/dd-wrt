/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2011 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */


#include "lib/misc/lib.h"

#include "lib/config/config.h"
#include "lib/misc/crc.h"
#include "lib/device/device.h"
#include "lib/datastruct/str_list.h"
#include "lib/commands/toolcontext.h"
#include "lib/misc/lvm-file.h"
#include "lib/mm/memlock.h"
#include "lib/label/label.h"

#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <float.h>

static const char *_config_source_names[] = {
	[CONFIG_UNDEFINED] = "undefined",
	[CONFIG_FILE] = "file",
	[CONFIG_MERGED_FILES] = "merged files",
	[CONFIG_STRING] = "string",
	[CONFIG_PROFILE_COMMAND] = "command profile",
	[CONFIG_PROFILE_METADATA] = "metadata profile",
	[CONFIG_FILE_SPECIAL] = "special purpose"
};

struct config_file {
	off_t st_size;
	char *filename;
	int exists;
	int keep_open;
	struct device *dev;
};

struct config_source {
	config_source_t type;
	struct timespec timestamp;
	union {
		struct config_file *file;
		struct config_file *profile;
	} source;
	struct cft_check_handle *check_handle;
};

/*
 * Map each ID to respective definition of the configuration item.
 */
static struct cfg_def_item _cfg_def_items[CFG_COUNT + 1] = {
#define cfg_section(id, name, parent, flags, since_version, deprecated_since_version, deprecation_comment, comment) {id, parent, name, CFG_TYPE_SECTION, {0}, (flags), since_version, {0}, deprecated_since_version, deprecation_comment, comment},
#define cfg(id, name, parent, flags, type, default_value, since_version, unconfigured_value, deprecated_since_version, deprecation_comment, comment) {id, parent, name, type, {.v_##type = (default_value)}, (flags), since_version, {.v_UNCONFIGURED = (unconfigured_value)}, deprecated_since_version, deprecation_comment, comment},
#define cfg_runtime(id, name, parent, flags, type, since_version, deprecated_since_version, deprecation_comment, comment) {id, parent, name, type, {.fn_##type = get_default_##id}, (flags) | CFG_DEFAULT_RUN_TIME, since_version, {.fn_UNCONFIGURED = get_default_unconfigured_##id}, deprecated_since_version, (deprecation_comment), comment},
#define cfg_array(id, name, parent, flags, types, default_value, since_version, unconfigured_value, deprecated_since_version, deprecation_comment, comment) {id, parent, name, CFG_TYPE_ARRAY | (types), {.v_CFG_TYPE_STRING = (default_value)}, (flags), (since_version), {.v_UNCONFIGURED = (unconfigured_value)}, deprecated_since_version, deprecation_comment, comment},
#define cfg_array_runtime(id, name, parent, flags, types, since_version, deprecated_since_version, deprecation_comment, comment) {id, parent, name, CFG_TYPE_ARRAY | (types), {.fn_CFG_TYPE_STRING = get_default_##id}, (flags) | CFG_DEFAULT_RUN_TIME, (since_version), {.fn_UNCONFIGURED = get_default_unconfigured_##id}, deprecated_since_version, deprecation_comment, comment},
#include "lib/config/config_settings.h"
#undef cfg_section
#undef cfg
#undef cfg_runtime
#undef cfg_array
#undef cfg_array_runtime
};

config_source_t config_get_source_type(struct dm_config_tree *cft)
{
	struct config_source *cs = dm_config_get_custom(cft);
	return cs ? cs->type : CONFIG_UNDEFINED;
}

static inline int _is_profile_based_config_source(config_source_t source)
{
	return (source == CONFIG_PROFILE_COMMAND) ||
	       (source == CONFIG_PROFILE_METADATA);
}

static inline int _is_file_based_config_source(config_source_t source)
{
	return (source == CONFIG_FILE) ||
	       (source == CONFIG_FILE_SPECIAL) ||
	       _is_profile_based_config_source(source);
}

/*
 * public interface
 */
struct dm_config_tree *config_open(config_source_t source,
				   const char *filename,
				   int keep_open)
{
	struct dm_config_tree *cft = dm_config_create();
	struct config_source *cs;
	struct config_file *cf;

	if (!cft)
		return NULL;

	if (!(cs = dm_pool_zalloc(cft->mem, sizeof(struct config_source)))) {
		log_error("Failed to allocate config source.");
		goto fail;
	}

	if (_is_file_based_config_source(source)) {
		if (!(cf = dm_pool_zalloc(cft->mem, sizeof(struct config_file)))) {
			log_error("Failed to allocate config file.");
			goto fail;
		}

		cf->keep_open = keep_open;
		if (filename &&
		    !(cf->filename = dm_pool_strdup(cft->mem, filename))) {
			log_error("Failed to duplicate filename.");
			goto fail;
		}

		cs->source.file = cf;
	}

	cs->type = source;
	dm_config_set_custom(cft, cs);
	return cft;
fail:
	dm_config_destroy(cft);
	return NULL;
}

/*
 * Doesn't populate filename if the file is empty.
 */
int config_file_check(struct dm_config_tree *cft, const char **filename, struct stat *info)
{
	struct config_source *cs = dm_config_get_custom(cft);
	struct config_file *cf;
	struct stat _info;

	if (!_is_file_based_config_source(cs->type)) {
		log_error(INTERNAL_ERROR "config_file_check: expected file, special file or "
					 "profile config source, found %s config source.",
					 _config_source_names[cs->type]);
		return 0;
	}

	if (!info)
		info = &_info;

	cf = cs->source.file;

	if (stat(cf->filename, info)) {
		log_sys_error("stat", cf->filename);
		cf->exists = 0;
		return 0;
	}

	if (!S_ISREG(info->st_mode)) {
		log_error("%s is not a regular file", cf->filename);
		cf->exists = 0;
		return 0;
	}

	lvm_stat_ctim(&cs->timestamp, info);
	cf->exists = 1;
	cf->st_size = info->st_size;

	if (info->st_size == 0)
		log_verbose("%s is empty", cf->filename);
	else if (filename)
		*filename = cf->filename;

	return 1;
}

/*
 * Return 1 if config files ought to be reloaded
 */
int config_file_changed(struct dm_config_tree *cft)
{
	struct config_source *cs = dm_config_get_custom(cft);
	struct config_file *cf;
	struct stat info;
	struct timespec ts;

	if (cs->type != CONFIG_FILE) {
		log_error(INTERNAL_ERROR "config_file_changed: expected file config source, "
					 "found %s config source.", _config_source_names[cs->type]);
		return 0;
	}

	cf = cs->source.file;

	if (!cf->filename)
		return 0;

	if (stat(cf->filename, &info) == -1) {
		/* Ignore a deleted config file: still use original data */
		if (errno == ENOENT) {
			if (!cf->exists)
				return 0;
			log_very_verbose("Config file %s has disappeared!",
					 cf->filename);
			goto reload;
		}
		log_sys_error("stat", cf->filename);
		log_error("Failed to reload configuration files");
		return 0;
	}

	if (!S_ISREG(info.st_mode)) {
		log_error("Configuration file %s is not a regular file",
			  cf->filename);
		goto reload;
	}

	/* Unchanged? */
	lvm_stat_ctim(&ts, &info);
	if ((timespeccmp(&cs->timestamp, &ts, ==)) &&
	    cf->st_size == info.st_size)
		return 0;

      reload:
	log_verbose("Detected config file change to %s", cf->filename);
	return 1;
}

void config_destroy(struct dm_config_tree *cft)
{
	struct config_source *cs;
	struct config_file *cf;

	if (!cft)
		return;

	cs = dm_config_get_custom(cft);

	if (_is_file_based_config_source(cs->type)) {
		cf = cs->source.file;
		if (cf && cf->dev)
			if (!dev_close(cf->dev))
				stack;
	}

	dm_config_destroy(cft);
}

struct dm_config_tree *config_file_open_and_read(const char *config_file,
						 config_source_t source,
						 struct cmd_context *cmd)
{
	struct dm_config_tree *cft;
	struct stat info;

	if (!(cft = config_open(source, config_file, 0))) {
		log_error("config_tree allocation failed");
		return NULL;
	}

	/* Is there a config file? */
	if (stat(config_file, &info) == -1) {
		/* Profile file must be present! */
		if (errno == ENOENT && (!_is_profile_based_config_source(source)))
			return cft;
		log_sys_error("stat", config_file);
		goto bad;
	}

	log_very_verbose("Loading config file: %s", config_file);
	if (!config_file_read(cft)) {
		log_error("Failed to load config file %s", config_file);
		goto bad;
	}

	return cft;
bad:
	config_destroy(cft);
	return NULL;
}

struct dm_config_tree *get_config_tree_by_source(struct cmd_context *cmd,
						 config_source_t source)
{
	struct dm_config_tree *cft = cmd->cft;
	struct config_source *cs;

	while (cft) {
		cs = dm_config_get_custom(cft);
		if (cs && cs->type == source)
			return cft;
		cft = cft->cascade;
	}

	return NULL;
}

/*
 * Returns config tree if it was removed.
 */
struct dm_config_tree *remove_config_tree_by_source(struct cmd_context *cmd,
						    config_source_t source)
{
	struct dm_config_tree *previous_cft = NULL;
	struct dm_config_tree *cft = cmd->cft;
	struct config_source *cs;

	while (cft) {
		cs = dm_config_get_custom(cft);
		if (cs && (cs->type == source)) {
			if (previous_cft) {
				previous_cft->cascade = cft->cascade;
				cmd->cft = previous_cft;
			} else
				cmd->cft = cft->cascade;
			cft->cascade = NULL;
			break;
		}
		previous_cft = cft;
		cft = cft->cascade;
	}

	return cft;
}

struct cft_check_handle *get_config_tree_check_handle(struct cmd_context *cmd,
						      struct dm_config_tree *cft)
{
	struct config_source *cs;

	if (!(cs = dm_config_get_custom(cft)))
		return NULL;

	if (cs->check_handle)
		goto out;

	/*
	 * Attach config check handle to all config types but
	 * CONFIG_FILE_SPECIAL - this one uses its own check
	 * methods and the cft_check_handle is not applicable here.
	 */
	if (cs->type != CONFIG_FILE_SPECIAL) {
		if (!(cs->check_handle = dm_pool_zalloc(cft->mem, sizeof(*cs->check_handle)))) {
			log_error("Failed to allocate configuration check handle.");
			return NULL;
		}
		cs->check_handle->cft = cft;
		cs->check_handle->cmd = cmd;
	}
out:
	return cs->check_handle;
}


int override_config_tree_from_string(struct cmd_context *cmd,
				     const char *config_settings)
{
	struct dm_config_tree *cft_new;
	struct config_source *cs = dm_config_get_custom(cmd->cft);

	/*
	 * Follow this sequence:
	 * CONFIG_STRING -> CONFIG_PROFILE_COMMAND -> CONFIG_PROFILE_METADATA -> CONFIG_FILE/CONFIG_MERGED_FILES
	 */

	if (cs->type == CONFIG_STRING) {
		log_error(INTERNAL_ERROR "override_config_tree_from_string: "
			  "config cascade already contains a string config.");
		return 0;
	}

	if (!(cft_new = dm_config_from_string(config_settings))) {
		log_error("Failed to set overridden configuration entries.");
		return 0;
	}

	if (cmd->is_interactive &&
	    !config_force_check(cmd, CONFIG_STRING, cft_new)) {
		log_error("Ignoring invalid configuration string.");
		dm_config_destroy(cft_new);
		return 0;
	}

	if (!(cs = dm_pool_zalloc(cft_new->mem, sizeof(struct config_source)))) {
		log_error("Failed to allocate config source.");
		dm_config_destroy(cft_new);
		return 0;
	}

	cs->type = CONFIG_STRING;
	dm_config_set_custom(cft_new, cs);

	cmd->cft = dm_config_insert_cascaded_tree(cft_new, cmd->cft);

	return 1;
}

static int _override_config_tree_from_command_profile(struct cmd_context *cmd,
						      struct profile *profile)
{
	struct dm_config_tree *cft = cmd->cft, *cft_previous = NULL;
	struct config_source *cs = dm_config_get_custom(cft);

	if (cs->type == CONFIG_STRING) {
		cft_previous = cft;
		cft = cft->cascade;
		cs = dm_config_get_custom(cft);
	}

	if (cs->type == CONFIG_PROFILE_COMMAND) {
		log_error(INTERNAL_ERROR "_override_config_tree_from_command_profile: "
				"config cascade already contains a command profile config.");
		return 0;
	}

	if (cft_previous)
		dm_config_insert_cascaded_tree(cft_previous, profile->cft);
	else
		cmd->cft = profile->cft;

	dm_config_insert_cascaded_tree(profile->cft, cft);

	return 1;
}

static int _override_config_tree_from_metadata_profile(struct cmd_context *cmd,
						       struct profile *profile)
{
	struct dm_config_tree *cft = cmd->cft, *cft_previous = NULL;
	struct config_source *cs = dm_config_get_custom(cft);

	if (cs->type == CONFIG_STRING) {
		cft_previous = cft;
		cft = cft->cascade;
	}

	if (cs->type == CONFIG_PROFILE_COMMAND) {
		cft_previous = cft;
		cft = cft->cascade;
	}

	cs = dm_config_get_custom(cft);

	if (cs->type == CONFIG_PROFILE_METADATA) {
		log_error(INTERNAL_ERROR "_override_config_tree_from_metadata_profile: "
				"config cascade already contains a metadata profile config.");
		return 0;
	}

	if (cft_previous)
		dm_config_insert_cascaded_tree(cft_previous, profile->cft);
	else
		cmd->cft = profile->cft;

	dm_config_insert_cascaded_tree(profile->cft, cft);

	return 1;
}

int override_config_tree_from_profile(struct cmd_context *cmd,
				      struct profile *profile)
{
	/*
	 * Follow this sequence:
	 * CONFIG_STRING -> CONFIG_PROFILE_COMMAND -> CONFIG_PROFILE_METADATA -> CONFIG_FILE/CONFIG_MERGED_FILES
	 */

	if (!profile->cft && !load_profile(cmd, profile))
		return_0;

	if (profile->source == CONFIG_PROFILE_COMMAND)
		return _override_config_tree_from_command_profile(cmd, profile);

	if (profile->source == CONFIG_PROFILE_METADATA)
		return _override_config_tree_from_metadata_profile(cmd, profile);

	log_error(INTERNAL_ERROR "override_config_tree_from_profile: incorrect profile source type");
	return 0;
}

/*
 * When checksum_only is set, the checksum of buffer is only matched
 * and function avoids parsing of mda into config tree which
 * remains unmodified and should not be used.
 */
int config_file_read_fd(struct dm_config_tree *cft, struct device *dev, dev_io_reason_t reason,
			off_t offset, size_t size, off_t offset2, size_t size2,
			checksum_fn_t checksum_fn, uint32_t checksum,
			int checksum_only, int no_dup_node_check)
{
	char *fb, *fe;
	int r = 0;
	int use_mmap = 1;
	off_t mmap_offset = 0;
	char *buf = NULL;
	struct config_source *cs = dm_config_get_custom(cft);

	if (!_is_file_based_config_source(cs->type)) {
		log_error(INTERNAL_ERROR "config_file_read_fd: expected file, special file "
					 "or profile config source, found %s config source.",
					 _config_source_names[cs->type]);
		return 0;
	}

	/* Only use mmap with regular files */
	if (!(dev->flags & DEV_REGULAR) || size2)
		use_mmap = 0;

	if (use_mmap) {
		mmap_offset = offset % lvm_getpagesize();
		/* memory map the file */
		fb = mmap((caddr_t) 0, size + mmap_offset, PROT_READ,
			  MAP_PRIVATE, dev_fd(dev), offset - mmap_offset);
		if (fb == (caddr_t) (-1)) {
			log_sys_error("mmap", dev_name(dev));
			goto out;
		}
		fb = fb + mmap_offset;
	} else {
		if (!(buf = malloc(size + size2))) {
			log_error("Failed to allocate circular buffer.");
			return 0;
		}

		if (!dev_read_bytes(dev, offset, size, buf))
			goto out;

		if (size2) {
			if (!dev_read_bytes(dev, offset2, size2, buf + size))
				goto out;
		}

		fb = buf;
	}

	/*
	 * The checksum passed in is the checksum from the mda_header
	 * preceding this metadata.  They should always match.
	 * FIXME: handle case where mda_header checksum is bad,
	 * but the checksum calculated here is correct.
	 */
	if (checksum_fn && checksum !=
	    (checksum_fn(checksum_fn(INITIAL_CRC, (const uint8_t *)fb, size),
			 (const uint8_t *)(fb + size), size2))) {
		log_error("%s: Checksum error at offset %" PRIu64, dev_name(dev), (uint64_t) offset);
		goto out;
	}

	if (!checksum_only) {
		fe = fb + size + size2;
		if (no_dup_node_check) {
			if (!dm_config_parse_without_dup_node_check(cft, fb, fe))
				goto_out;
		} else {
			if (!dm_config_parse(cft, fb, fe))
				goto_out;
		}
	}

	r = 1;

      out:
	if (!use_mmap)
		free(buf);
	else {
		/* unmap the file */
		if (munmap(fb - mmap_offset, size + mmap_offset)) {
			log_sys_error("munmap", dev_name(dev));
			r = 0;
		}
	}

	return r;
}

int config_file_read(struct dm_config_tree *cft)
{
	const char *filename = NULL;
	struct config_source *cs = dm_config_get_custom(cft);
	struct config_file *cf;
	struct stat info;
	int r;

	if (!config_file_check(cft, &filename, &info))
		return_0;

	/* Nothing to do.  E.g. empty file. */
	if (!filename)
		return 1;

	cf = cs->source.file;

	if (!cf->dev) {
		if (!(cf->dev = dev_create_file(filename, NULL, NULL, 1)))
			return_0;

		if (!dev_open_readonly_buffered(cf->dev)) {
			dev_destroy_file(cf->dev);
			cf->dev = NULL;
			return_0;
		}
	}

	r = config_file_read_fd(cft, cf->dev, DEV_IO_MDA_CONTENT, 0, (size_t) info.st_size, 0, 0,
				(checksum_fn_t) NULL, 0, 0, 0);

	if (!cf->keep_open) {
		if (!dev_close(cf->dev))
			stack;
		cf->dev = NULL;
	}

	return r;
}

struct timespec config_file_timestamp(struct dm_config_tree *cft)
{
	struct config_source *cs = dm_config_get_custom(cft);
	return cs->timestamp;
}

#define cfg_def_get_item_p(id) (&_cfg_def_items[id])
#define cfg_def_get_default_unconfigured_value_hint(cmd,item) (((item)->flags & CFG_DEFAULT_RUN_TIME) ? (item)->default_unconfigured_value.fn_UNCONFIGURED(cmd) : (item)->default_unconfigured_value.v_UNCONFIGURED)
#define cfg_def_get_default_value_hint(cmd,item,type,profile) (((item)->flags & CFG_DEFAULT_RUN_TIME) ? (item)->default_value.fn_##type(cmd,profile) : (item)->default_value.v_##type)
#define cfg_def_get_default_value(cmd,item,type,profile) ((item)->flags & CFG_DEFAULT_UNDEFINED ? 0 : cfg_def_get_default_value_hint(cmd,item,type,profile))

static int _cfg_def_make_path(char *buf, size_t buf_size, int id, cfg_def_item_t *item, int xlate)
{
	int variable = item->flags & CFG_NAME_VARIABLE;
	int parent_id = item->parent;
	int count, n;

	if (id == parent_id) {
		buf[0] = '\0';
		return 0;
	}

	count = _cfg_def_make_path(buf, buf_size, parent_id, cfg_def_get_item_p(parent_id), xlate);
	if ((n = dm_snprintf(buf + count, buf_size - count, "%s%s%s%s",
			     count ? "/" : "",
			     xlate && variable ? "<" : "",
			     !xlate && variable ? "#" : item->name,
			     xlate && variable ? ">" : "")) < 0) {
		log_error(INTERNAL_ERROR "_cfg_def_make_path: supplied buffer too small for %s/%s",
					  cfg_def_get_item_p(parent_id)->name, item->name);
		buf[0] = '\0';
		return 0;
	}

	return count + n;
}

int config_def_get_path(char *buf, size_t buf_size, int id)
{
	return _cfg_def_make_path(buf, buf_size, id, cfg_def_get_item_p(id), 0);
}

static void _get_type_name(char *buf, size_t buf_size, cfg_def_type_t type)
{
	(void) dm_snprintf(buf, buf_size, "%s%s%s%s%s%s",
			   (type & CFG_TYPE_ARRAY) ?
				((type & ~CFG_TYPE_ARRAY) ?
				 " array with values of type:" : " array") : "",
			   (type & CFG_TYPE_SECTION) ? " section" : "",
			   (type & CFG_TYPE_BOOL) ?  " boolean" : "",
			   (type & CFG_TYPE_INT) ? " integer" : "",
			   (type & CFG_TYPE_FLOAT) ?  " float" : "",
			   (type & CFG_TYPE_STRING) ? " string" : "");
}

static void _log_type_error(const char *path, cfg_def_type_t actual,
			    cfg_def_type_t expected, int suppress_messages)
{
	static char actual_type_name[128];
	static char expected_type_name[128];

	_get_type_name(actual_type_name, sizeof(actual_type_name), actual);
	_get_type_name(expected_type_name, sizeof(expected_type_name), expected);

	log_warn_suppress(suppress_messages, "WARNING: Configuration setting \"%s\" has invalid type. "
					     "Found%s but expected%s.", path,
					     actual_type_name, expected_type_name);
}

static struct dm_config_value *_get_def_array_values(struct cmd_context *cmd,
						     struct dm_config_tree *cft,
						     const cfg_def_item_t *def,
						     uint32_t format_flags)
{
	const char *def_enc_value;
	char *enc_value, *token, *p, *r;
	struct dm_config_value *array = NULL, *v = NULL, *oldv = NULL;

	def_enc_value = cfg_def_get_default_value(cmd, def, CFG_TYPE_ARRAY, NULL);

	if (!def_enc_value) {
		if (!(array = dm_config_create_value(cft))) {
			log_error("Failed to create default empty array for %s.", def->name);
			return NULL;
		}
		array->type = DM_CFG_EMPTY_ARRAY;
		dm_config_value_set_format_flags(array, format_flags);
		return array;
	}

	if (!(p = token = enc_value = strdup(def_enc_value))) {
		log_error("_get_def_array_values: strdup failed");
		return NULL;
	}
	/* Proper value always starts with '#'. */
	if (token[0] != '#')
		goto bad;

	while (token) {
		/* Move to type identifier. Error on no char. */
		token++;
		if (!token[0])
			goto bad;

		/* Move to the actual value and decode any "##" into "#". */
		p = token + 1;
		while ((p = strchr(p, '#')) && p[1] == '#') {
			memmove(p, p + 1, strlen(p));
			p++;
		}
		/* Separate the value out of the whole string. */
		if (p)
			p[0] = '\0';

		if (!(v = dm_config_create_value(cft))) {
			log_error("Failed to create default config array value for %s.", def->name);
			free(enc_value);
			return NULL;
		}

		dm_config_value_set_format_flags(v, format_flags);

		if (oldv)
			oldv->next = v;
		if (!array)
			array = v;

		switch (toupper(token[0])) {
			case 'I':
			case 'B':
				errno = 0;
				v->v.i = strtoll(token + 1, &r, 10);
				if (errno || *r)
					goto bad;
				v->type = DM_CFG_INT;
				break;
			case 'F':
				errno = 0;
				v->v.f = strtod(token + 1, &r);
				if (errno || *r)
					goto bad;
				v->type = DM_CFG_FLOAT;
				break;
			case 'S':
				if (!(r = dm_pool_strdup(cft->mem, token + 1))) {
					free(enc_value);
					log_error("Failed to duplicate token for default "
						  "array value of %s.", def->name);
					return NULL;
				}
				v->v.str = r;
				v->type = DM_CFG_STRING;
				break;
			default:
				goto bad;
		}

		oldv = v;
		token = p;
	}

	free(enc_value);
	return array;
bad:
	log_error(INTERNAL_ERROR "Default array value malformed for \"%s\", "
		  "value: \"%s\", token: \"%s\".", def->name,
		  def->default_value.v_CFG_TYPE_STRING, token);
	free(enc_value);
	return NULL;
}

static int _config_def_check_node_single_value(struct cft_check_handle *handle,
					       const char *rp, const struct dm_config_value *v,
					       const cfg_def_item_t *def)
{
	/* Check empty array first if present. */
	if (v->type == DM_CFG_EMPTY_ARRAY) {
		if (!(def->type & CFG_TYPE_ARRAY)) {
			_log_type_error(rp, CFG_TYPE_ARRAY, def->type, handle->suppress_messages);
			return 0;
		}
		if (!(def->flags & CFG_ALLOW_EMPTY)) {
			log_warn_suppress(handle->suppress_messages,
				"Configuration setting \"%s\" invalid. Empty value not allowed.", rp);
			return 0;
		}
		return 1;
	}

	switch (v->type) {
		case DM_CFG_INT:
			if (!(def->type & CFG_TYPE_INT) && !(def->type & CFG_TYPE_BOOL)) {
				_log_type_error(rp, CFG_TYPE_INT, def->type, handle->suppress_messages);
				return 0;
			}
			break;
		case DM_CFG_FLOAT:
			if (!(def->type & CFG_TYPE_FLOAT)) {
				_log_type_error(rp, CFG_TYPE_FLOAT, def->type, handle-> suppress_messages);
				return 0;
			}
			break;
		case DM_CFG_STRING:
			if (def->type & CFG_TYPE_BOOL) {
				if (!dm_config_value_is_bool(v)) {
					log_warn_suppress(handle->suppress_messages,
						"Configuration setting \"%s\" invalid. "
						"Found string value \"%s\", "
						"expected boolean value: 0/1, \"y/n\", "
						"\"yes/no\", \"on/off\", "
						"\"true/false\".", rp, v->v.str);
					return 0;
				}
			} else if  (!(def->type & CFG_TYPE_STRING)) {
				_log_type_error(rp, CFG_TYPE_STRING, def->type, handle->suppress_messages);
				return 0;
			} else if (!(def->flags & CFG_ALLOW_EMPTY) && !*v->v.str) {
				log_warn_suppress(handle->suppress_messages,
						  "Configuration setting \"%s\" invalid. "
						  "It cannot be set to an empty value.", rp);
				return 0;
			}
			break;
		default: ;
	}

	return 1;
}

static int _check_value_differs_from_default(struct cft_check_handle *handle,
					     const struct dm_config_value *v,
					     const cfg_def_item_t *def,
					     struct dm_config_value *v_def)
{
	struct dm_config_value *v_def_array, *v_def_iter;
	int diff = 0, id;
	int64_t i;
	float f;
	const char *str;

	if ((handle->ignoreunsupported && (def->flags & CFG_UNSUPPORTED)) ||
	    (handle->ignoreadvanced && (def->flags & CFG_ADVANCED))) {
		diff = 0;
		goto out;
	}

	/* if default value is undefined, the value used differs from default */
	if (def->flags & CFG_DEFAULT_UNDEFINED) {
		diff = 1;
		goto out;
	}

	if (!v_def && (def->type & CFG_TYPE_ARRAY)) {
		if (!(v_def_array = v_def_iter = _get_def_array_values(handle->cmd, handle->cft, def, 0)))
			return_0;
		do {
			/* iterate over each element of the array and check its value */
			if ((v->type != v_def_iter->type) ||
			    _check_value_differs_from_default(handle, v, def, v_def_iter))
				break;
			v_def_iter = v_def_iter->next;
			v = v->next;
		} while (v_def_iter && v);
		diff = v || v_def_iter;
		dm_pool_free(handle->cft->mem, v_def_array);
	} else {
		switch (v->type) {
			case DM_CFG_INT:
				/* int value can be a real int but it can also represent bool */
				i = v_def ? v_def->v.i
					  : def->type & CFG_TYPE_BOOL ?
						cfg_def_get_default_value(handle->cmd, def, CFG_TYPE_BOOL, NULL) :
						cfg_def_get_default_value(handle->cmd, def, CFG_TYPE_INT, NULL);
				diff = i != v->v.i;
				break;
			case DM_CFG_FLOAT:
				f = v_def ? v_def->v.f
					  : cfg_def_get_default_value(handle->cmd, def, CFG_TYPE_FLOAT, NULL);
				diff = fabs(f - v->v.f) < FLT_EPSILON;
				break;
			case DM_CFG_STRING:
				/* string value can be a real string but it can also represent bool */
				if (v_def ? v_def->type == DM_CFG_INT : def->type == CFG_TYPE_BOOL) {
					i = v_def ? v_def->v.i
						  : cfg_def_get_default_value(handle->cmd, def, CFG_TYPE_BOOL, NULL);
					diff = i != v->v.i;
				} else {
					str = v_def ? v_def->v.str
						    : cfg_def_get_default_value(handle->cmd, def, CFG_TYPE_STRING, NULL);
					diff = strcmp(str, v->v.str);
				}
				break;
			case DM_CFG_EMPTY_ARRAY:
				diff = (v_def && (v_def->type != DM_CFG_EMPTY_ARRAY));
				break;
			default:
				log_error(INTERNAL_ERROR "inconsistent state reached in _check_value_differs_from_default");
				return 0;
		}
	}
out:
	if (diff) {
		/* mark whole path from bottom to top with CFG_DIFF */
		for (id = def->id; id && !(handle->status[id] & CFG_DIFF); id = _cfg_def_items[id].parent)
			handle->status[id] |= CFG_DIFF;
	}

	return diff;
}

static int _config_def_check_node_value(struct cft_check_handle *handle,
					const char *rp, const struct dm_config_value *v,
					const cfg_def_item_t *def)
{
	const struct dm_config_value *v_iter;

	if (!v) {
		if (def->type != CFG_TYPE_SECTION) {
			_log_type_error(rp, CFG_TYPE_SECTION, def->type, handle->suppress_messages);
			return 0;
		}
		return 1;
	}

	if (v->next) {
		if (!(def->type & CFG_TYPE_ARRAY)) {
			_log_type_error(rp, CFG_TYPE_ARRAY, def->type, handle->suppress_messages);
			return 0;
		}
	}

	v_iter = v;
	do {
		if (!_config_def_check_node_single_value(handle, rp, v_iter, def))
			return 0;
		v_iter = v_iter->next;
	} while (v_iter);

	if (handle->check_diff)
		_check_value_differs_from_default(handle, v, def, NULL);

	return 1;
}

static int _config_def_check_node_is_profilable(struct cft_check_handle *handle,
						const char *rp, struct dm_config_node *cn,
						const cfg_def_item_t *def)
{
	uint16_t flags;

	if (!(def->flags & CFG_PROFILABLE)) {
		log_warn_suppress(handle->suppress_messages,
				  "Configuration %s \"%s\" is not customizable by "
				  "a profile.", cn->v ? "option" : "section", rp);
		return 0;
	}

	flags = def->flags & ~CFG_PROFILABLE;

	/*
	* Make sure there is no metadata profilable config in the command profile!
	*/
	if ((handle->source == CONFIG_PROFILE_COMMAND) && (flags & CFG_PROFILABLE_METADATA)) {
		log_warn_suppress(handle->suppress_messages,
				  "Configuration %s \"%s\" is customizable by "
				  "metadata profile only, not command profile.",
				   cn->v ? "option" : "section", rp);
		return 0;
	}

	/*
	 * Make sure there is no command profilable config in the metadata profile!
	 * (sections do not need to be flagged with CFG_PROFILABLE_METADATA, the
	 *  CFG_PROFILABLE is enough as sections may contain both types inside)
	 */
	if ((handle->source == CONFIG_PROFILE_METADATA) && cn->v && !(flags & CFG_PROFILABLE_METADATA)) {
		log_warn_suppress(handle->suppress_messages,
				  "Configuration %s \"%s\" is customizable by "
				  "command profile only, not metadata profile.",
				   cn->v ? "option" : "section", rp);
		return 0;
	}

	return 1;
}

static int _config_def_check_node_is_allowed(struct cft_check_handle *handle,
					     const char *rp, struct dm_config_node *cn,
					     const cfg_def_item_t *def)
{
	if (handle->disallowed_flags & def->flags) {
		log_warn_suppress(handle->suppress_messages,
				  "Configuration %s \"%s\" is not allowed here.",
				  cn->v ? "option" : "section", rp);
		return 0;
	}

	return 1;
}

static int _config_def_check_node(struct cft_check_handle *handle,
				  const char *vp, char *pvp, char *rp, char *prp,
				  size_t buf_size, struct dm_config_node *cn)
{
	cfg_def_item_t *def;
	int sep = vp != pvp; /* don't use '/' separator for top-level node */

	if (dm_snprintf(pvp, buf_size, "%s%s", sep ? "/" : "", cn->key) < 0 ||
	    dm_snprintf(prp, buf_size, "%s%s", sep ? "/" : "", cn->key) < 0) {
		log_error("Failed to construct path for configuration node %s.", cn->key);
		return 0;
	}


	if (!(def = (cfg_def_item_t *) dm_hash_lookup(handle->cmd->cft_def_hash, vp))) {
		/* If the node is not a section but a setting, fail now. */
		if (cn->v) {
			log_warn_suppress(handle->suppress_messages,
				"Configuration setting \"%s\" unknown.", rp);
			cn->id = -1;
			return 0;
		}

		/* If the node is a section, try if the section name is variable. */
		/* Modify virtual path vp in situ and replace the key name with a '#'. */
		/* The real path without '#' is still stored in rp variable. */
		pvp[sep] = '#', pvp[sep + 1] = '\0';
		if (!(def = (cfg_def_item_t *) dm_hash_lookup(handle->cmd->cft_def_hash, vp))) {
			log_warn_suppress(handle->suppress_messages,
				"Configuration section \"%s\" unknown.", rp);
			cn->id = -1;
			return 0;
		}
	}

	handle->status[def->id] |= CFG_USED;
	cn->id = def->id;

	if (!_config_def_check_node_value(handle, rp, cn->v, def))
		return 0;

	/*
	 * Also check whether this configuration item is allowed
	 * in certain types of configuration trees as in some
	 * the use of configuration is restricted, e.g. profiles...
	 */
	if (_is_profile_based_config_source(handle->source) &&
	    !_config_def_check_node_is_profilable(handle, rp, cn, def))
		return_0;

	if (!_config_def_check_node_is_allowed(handle, rp, cn, def))
		return_0;

	handle->status[def->id] |= CFG_VALID;
	return 1;
}

static int _config_def_check_tree(struct cft_check_handle *handle,
				  const char *vp, char *pvp, char *rp, char *prp,
				  size_t buf_size, struct dm_config_node *root)
{
	struct dm_config_node *cn;
	cfg_def_item_t *def;
	int valid, r = 1;
	size_t len;

	def = cfg_def_get_item_p(root->id);
	if (def->flags & CFG_SECTION_NO_CHECK)
		return 1;

	for (cn = root->child; cn; cn = cn->sib) {
		if ((valid = _config_def_check_node(handle, vp, pvp, rp, prp,
						    buf_size, cn)) && !cn->v) {
			len = strlen(rp);
			valid = _config_def_check_tree(handle, vp, pvp + strlen(pvp),
						       rp, prp + len, buf_size - len, cn);
		}
		if (!valid)
			r = 0;
	}

	return r;
}

int config_def_check(struct cft_check_handle *handle)
{
	cfg_def_item_t *def;
	struct dm_config_node *cn;
	char vp[CFG_PATH_MAX_LEN], rp[CFG_PATH_MAX_LEN];
	size_t rplen;
	int id, r = 1;

	/*
	 * vp = virtual path, it might contain substitutes for variable parts
	 * 	of the path, used while working with the hash
	 * rp = real path, the real path of the config element as found in the
	 *      configuration, used for message output
	 */

	/*
	 * If the check has already been done and 'skip_if_checked' is set,
	 * skip the actual check and use last result if available.
	 * If not available, we must do the check. The global status
	 * is stored in root node.
	 */
	if (handle->skip_if_checked && (handle->status[root_CFG_SECTION] & CFG_USED))
		return handle->status[root_CFG_SECTION] & CFG_VALID;

	/* Nothing to do if checks are disabled and also not forced. */
	if (!handle->force_check && !find_config_tree_bool(handle->cmd, config_checks_CFG, NULL))
		return 1;

	/* Clear 'used' and 'valid' status flags. */
	for (id = 0; id < CFG_COUNT; id++)
		handle->status[id] &= ~(CFG_USED | CFG_VALID | CFG_DIFF);

	/*
	 * Create a hash of all possible configuration
	 * sections and settings with full path as a key.
	 * If section name is variable, use '#' as a substitute.
	 */
	if (!handle->cmd->cft_def_hash) {
		if (!(handle->cmd->cft_def_hash = dm_hash_create(64))) {
			log_error("Failed to create configuration definition hash.");
			r = 0; goto out;
		}
		for (id = 1; id < CFG_COUNT; id++) {
			def = cfg_def_get_item_p(id);
			if (!_cfg_def_make_path(vp, CFG_PATH_MAX_LEN, def->id, def, 0)) {
				dm_hash_destroy(handle->cmd->cft_def_hash);
				handle->cmd->cft_def_hash = NULL;
				r = 0; goto out;
			}
			if (!dm_hash_insert(handle->cmd->cft_def_hash, vp, def)) {
				log_error("Failed to insert configuration to hash.");
				r = 0;
				goto out;
			}
		}
	}

	/*
	 * Mark this handle as used so next time we know that the check
	 * has already been done and so we can just reuse the previous
	 * status instead of running this whole check again.
	 */
	handle->status[root_CFG_SECTION] |= CFG_USED;

	/*
	 * Allow only sections as top-level elements.
	 * Iterate top-level sections and dive deeper.
	 * If any of subsequent checks fails, the whole check fails.
	 */
	for (cn = handle->cft->root; cn; cn = cn->sib) {
		if (!cn->v) {
			/* top level node: vp=vp, rp=rp */
			if (!_config_def_check_node(handle, vp, vp, rp, rp,
						    CFG_PATH_MAX_LEN, cn)) {
				r = 0; continue;
			}
			rplen = strlen(rp);
			if (!_config_def_check_tree(handle,
						    vp, vp + strlen(vp),
						    rp, rp + rplen,
						    CFG_PATH_MAX_LEN - rplen, cn))
				r = 0;
		} else {
			log_error_suppress(handle->suppress_messages,
				"Configuration setting \"%s\" invalid. "
				"It's not part of any section.", cn->key);
			r = 0;
		}
	}
out:
	if (r)
		handle->status[root_CFG_SECTION] |= CFG_VALID;
	else
		handle->status[root_CFG_SECTION] &= ~CFG_VALID;

	return r;
}

static int _apply_local_profile(struct cmd_context *cmd, struct profile *profile)
{
	if (!profile)
		return 0;

	/*
	 * Global metadata profile overrides the local one.
	 * This simply means the "--metadataprofile" arg
	 * overrides any profile attached to VG/LV.
	 */
	if ((profile->source == CONFIG_PROFILE_METADATA) &&
	     cmd->profile_params->global_metadata_profile)
		return 0;

	return override_config_tree_from_profile(cmd, profile);
}

static int _config_disabled(struct cmd_context *cmd, cfg_def_item_t *item, const char *path)
{
	if ((item->flags & CFG_DISABLED) && dm_config_tree_find_node(cmd->cft, path)) {
		log_warn("WARNING: Configuration setting %s is disabled. Using default value.", path);
		return 1;
	}

	return 0;
}

const struct dm_config_node *find_config_node(struct cmd_context *cmd, struct dm_config_tree *cft, int id)
{
	cfg_def_item_t *item = cfg_def_get_item_p(id);
	char path[CFG_PATH_MAX_LEN];
	const struct dm_config_node *cn;

	_cfg_def_make_path(path, sizeof(path), item->id, item, 0);

	cn = dm_config_tree_find_node(cft, path);

	return cn;
}

const struct dm_config_node *find_config_tree_node(struct cmd_context *cmd, int id, struct profile *profile)
{
	cfg_def_item_t *item = cfg_def_get_item_p(id);
	char path[CFG_PATH_MAX_LEN];
	int profile_applied;
	const struct dm_config_node *cn;

	profile_applied = _apply_local_profile(cmd, profile);
	_cfg_def_make_path(path, sizeof(path), item->id, item, 0);

	cn = dm_config_tree_find_node(cmd->cft, path);

	if (profile_applied && profile)
		remove_config_tree_by_source(cmd, profile->source);

	return cn;
}

const char *find_config_tree_str(struct cmd_context *cmd, int id, struct profile *profile)
{
	cfg_def_item_t *item = cfg_def_get_item_p(id);
	char path[CFG_PATH_MAX_LEN];
	int profile_applied;
	const char *str;

	profile_applied = _apply_local_profile(cmd, profile);
	_cfg_def_make_path(path, sizeof(path), item->id, item, 0);

	if (item->type != CFG_TYPE_STRING)
		log_error(INTERNAL_ERROR "%s cfg tree element not declared as string.", path);

	str = _config_disabled(cmd, item, path) ? cfg_def_get_default_value(cmd, item, CFG_TYPE_STRING, profile)
						: dm_config_tree_find_str(cmd->cft, path, cfg_def_get_default_value(cmd, item, CFG_TYPE_STRING, profile));

	if (profile_applied && profile)
		remove_config_tree_by_source(cmd, profile->source);

	return str;
}

const char *find_config_tree_str_allow_empty(struct cmd_context *cmd, int id, struct profile *profile)
{
	cfg_def_item_t *item = cfg_def_get_item_p(id);
	char path[CFG_PATH_MAX_LEN];
	int profile_applied;
	const char *str;

	profile_applied = _apply_local_profile(cmd, profile);
	_cfg_def_make_path(path, sizeof(path), item->id, item, 0);

	if (item->type != CFG_TYPE_STRING)
		log_error(INTERNAL_ERROR "%s cfg tree element not declared as string.", path);
	if (!(item->flags & CFG_ALLOW_EMPTY))
		log_error(INTERNAL_ERROR "%s cfg tree element not declared to allow empty values.", path);

	str = _config_disabled(cmd, item, path) ? cfg_def_get_default_value(cmd, item, CFG_TYPE_STRING, profile)
						: dm_config_tree_find_str_allow_empty(cmd->cft, path, cfg_def_get_default_value(cmd, item, CFG_TYPE_STRING, profile));

	if (profile_applied && profile)
		remove_config_tree_by_source(cmd, profile->source);

	return str;
}

int find_config_tree_int(struct cmd_context *cmd, int id, struct profile *profile)
{
	cfg_def_item_t *item = cfg_def_get_item_p(id);
	char path[CFG_PATH_MAX_LEN];
	int profile_applied;
	int i;

	profile_applied = _apply_local_profile(cmd, profile);
	_cfg_def_make_path(path, sizeof(path), item->id, item, 0);

	if (item->type != CFG_TYPE_INT)
		log_error(INTERNAL_ERROR "%s cfg tree element not declared as integer.", path);

	i = _config_disabled(cmd, item, path) ? cfg_def_get_default_value(cmd, item, CFG_TYPE_INT, profile)
					      : dm_config_tree_find_int(cmd->cft, path, cfg_def_get_default_value(cmd, item, CFG_TYPE_INT, profile));

	if (profile_applied && profile)
		remove_config_tree_by_source(cmd, profile->source);

	return i;
}

int64_t find_config_tree_int64(struct cmd_context *cmd, int id, struct profile *profile)
{
	cfg_def_item_t *item = cfg_def_get_item_p(id);
	char path[CFG_PATH_MAX_LEN];
	int profile_applied;
	int i64;

	profile_applied = _apply_local_profile(cmd, profile);
	_cfg_def_make_path(path, sizeof(path), item->id, item, 0);

	if (item->type != CFG_TYPE_INT)
		log_error(INTERNAL_ERROR "%s cfg tree element not declared as integer.", path);

	i64 = _config_disabled(cmd, item, path) ? cfg_def_get_default_value(cmd, item, CFG_TYPE_INT, profile)
						: dm_config_tree_find_int64(cmd->cft, path, cfg_def_get_default_value(cmd, item, CFG_TYPE_INT, profile));

	if (profile_applied && profile)
		remove_config_tree_by_source(cmd, profile->source);

	return i64;
}

float find_config_tree_float(struct cmd_context *cmd, int id, struct profile *profile)
{
	cfg_def_item_t *item = cfg_def_get_item_p(id);
	char path[CFG_PATH_MAX_LEN];
	int profile_applied;
	float f;

	profile_applied = _apply_local_profile(cmd, profile);
	_cfg_def_make_path(path, sizeof(path), item->id, item, 0);

	if (item->type != CFG_TYPE_FLOAT)
		log_error(INTERNAL_ERROR "%s cfg tree element not declared as float.", path);

	f = _config_disabled(cmd, item, path) ? cfg_def_get_default_value(cmd, item, CFG_TYPE_FLOAT, profile)
					      : dm_config_tree_find_float(cmd->cft, path, cfg_def_get_default_value(cmd, item, CFG_TYPE_FLOAT, profile));

	if (profile_applied && profile)
		remove_config_tree_by_source(cmd, profile->source);

	return f;
}

int find_config_bool(struct cmd_context *cmd, struct dm_config_tree *cft, int id)
{
	cfg_def_item_t *item = cfg_def_get_item_p(id);
	char path[CFG_PATH_MAX_LEN];
	int b;

	_cfg_def_make_path(path, sizeof(path), item->id, item, 0);

	if (item->type != CFG_TYPE_BOOL)
		log_error(INTERNAL_ERROR "%s cfg tree element not declared as boolean.", path);

	b = _config_disabled(cmd, item, path) ? cfg_def_get_default_value(cmd, item, CFG_TYPE_BOOL, NULL)
					      : dm_config_tree_find_bool(cft, path, cfg_def_get_default_value(cmd, item, CFG_TYPE_BOOL, NULL));

	return b;
}

int find_config_tree_bool(struct cmd_context *cmd, int id, struct profile *profile)
{
	cfg_def_item_t *item = cfg_def_get_item_p(id);
	char path[CFG_PATH_MAX_LEN];
	int profile_applied;
	int b;

	profile_applied = _apply_local_profile(cmd, profile);
	_cfg_def_make_path(path, sizeof(path), item->id, item, 0);

	if (item->type != CFG_TYPE_BOOL)
		log_error(INTERNAL_ERROR "%s cfg tree element not declared as boolean.", path);

	b = _config_disabled(cmd, item, path) ? cfg_def_get_default_value(cmd, item, CFG_TYPE_BOOL, profile)
					      : dm_config_tree_find_bool(cmd->cft, path, cfg_def_get_default_value(cmd, item, CFG_TYPE_BOOL, profile));

	if (profile_applied && profile)
		remove_config_tree_by_source(cmd, profile->source);

	return b;
}

static struct dm_config_node *_get_array_def_node(struct cmd_context *cmd,
						  cfg_def_item_t *def,
						  struct profile *profile)
{
	struct dm_config_node *cn;

	if (def->flags & CFG_DEFAULT_UNDEFINED)
		return NULL;

	if (!(cn = dm_config_create_node(cmd->cft, def->name))) {
		log_error("Failed to create default array node for %s.", def->name);
		return NULL;
	}

	if (!(cn->v = _get_def_array_values(cmd, cmd->cft, def, 0))) {
		dm_pool_free(cmd->cft->mem, cn);
		return_NULL;
	}

	return cn;
}

struct _config_array_out_handle {
	struct dm_pool *mem;
	char *str;
};

static int _config_array_line(const struct dm_config_node *cn, const char *line, void *baton)
{
	struct _config_array_out_handle *handle = (struct _config_array_out_handle *) baton;

	if (!(handle->str = dm_pool_strdup(handle->mem, line))) {
		log_error("_config_array_line: dm_pool_strdup failed");
		return 0;
	}

	return 1;
}

static void _log_array_value_used(struct dm_pool *mem, const struct dm_config_node *cn,
				  const char *path, int default_used)
{
	struct _config_array_out_handle out_handle = { 0 };
	struct dm_config_node_out_spec out_spec = { 0 };
	uint32_t old_format_flags;

	out_handle.mem = mem;
	out_spec.line_fn = _config_array_line;

	old_format_flags = dm_config_value_get_format_flags(cn->v);
	dm_config_value_set_format_flags(cn->v,
		DM_CONFIG_VALUE_FMT_COMMON_EXTRA_SPACES |
		DM_CONFIG_VALUE_FMT_COMMON_ARRAY);

	if (!dm_config_write_one_node_out(cn, &out_spec, &out_handle)) {
		log_error("_log_array_value_used: failed to write node value");
		out_handle.mem = NULL;
	}

	if (default_used)
		log_very_verbose("%s not found in config: defaulting to %s",
				 path, out_handle.mem ? out_handle.str : "<unknown>");
	else
		log_very_verbose("Setting %s to %s",
				 path, out_handle.mem ? out_handle.str : "<unknown>");

	if (out_handle.mem)
		dm_pool_free(out_handle.mem, out_handle.str);
	dm_config_value_set_format_flags(cn->v, old_format_flags);
}

const struct dm_config_node *find_config_tree_array(struct cmd_context *cmd, int id, struct profile *profile)
{
	cfg_def_item_t *item = cfg_def_get_item_p(id);
	char path[CFG_PATH_MAX_LEN];
	int profile_applied;
	const struct dm_config_node *cn = NULL, *cn_def = NULL;
	profile_applied = _apply_local_profile(cmd, profile);
	_cfg_def_make_path(path, sizeof(path), item->id, item, 0);

	if (!(item->type & CFG_TYPE_ARRAY))
		log_error(INTERNAL_ERROR "%s cfg tree element not declared as array.", path);

	if (_config_disabled(cmd, item, path) ||
	    !(cn = find_config_tree_node(cmd, id, profile)))
		cn_def = _get_array_def_node(cmd, item, profile);

	if (cn)
		_log_array_value_used(cmd->cft->mem, cn, path, 0);
	else if (cn_def) {
		_log_array_value_used(cmd->cft->mem, cn_def, path, 1);
		cn = cn_def;
	}

	if (profile_applied && profile)
		remove_config_tree_by_source(cmd, profile->source);

	return cn;
}

/* Insert cn2 after cn1 */
static void _insert_config_node(struct dm_config_node **cn1,
				struct dm_config_node *cn2)
{
	if (!*cn1) {
		*cn1 = cn2;
		cn2->sib = NULL;
	} else {
		cn2->sib = (*cn1)->sib;
		(*cn1)->sib = cn2;
	}
}

/*
 * Merge section cn2 into section cn1 (which has the same name)
 * overwriting any existing cn1 nodes with matching names.
 */
static void _merge_section(struct dm_config_node *cn1, struct dm_config_node *cn2,
			   config_merge_t merge_type)
{
	struct dm_config_node *cn, *nextn, *oldn;
	struct dm_config_value *cv;

	for (cn = cn2->child; cn; cn = nextn) {
		nextn = cn->sib;

		if (merge_type == CONFIG_MERGE_TYPE_TAGS) {
			/* Skip "tags" */
			if (!strcmp(cn->key, "tags"))
				continue;
		}

		/* Subsection? */
		if (!cn->v)
			/* Ignore - we don't have any of these yet */
			continue;
		/* Not already present? */
		if (!(oldn = dm_config_find_node(cn1->child, cn->key))) {
			_insert_config_node(&cn1->child, cn);
			continue;
		}
		if (merge_type == CONFIG_MERGE_TYPE_TAGS) {
			/* Merge certain value lists */
			if ((!strcmp(cn1->key, "activation") &&
			     !strcmp(cn->key, "volume_list")) ||
			    (!strcmp(cn1->key, "devices") &&
			     (!strcmp(cn->key, "filter") || !strcmp(cn->key, "types")))) {
				cv = cn->v;
				while (cv->next)
					cv = cv->next;
				cv->next = oldn->v;
			}
		}

		/* Replace values */
		oldn->v = cn->v;
	}
}

static int _match_host_tags(struct dm_list *tags, const struct dm_config_node *tn)
{
	const struct dm_config_value *tv;
	const char *str;

	for (tv = tn->v; tv; tv = tv->next) {
		if (tv->type != DM_CFG_STRING)
			continue;
		str = tv->v.str;
		if (*str == '@')
			str++;
		if (!*str)
			continue;
		if (str_list_match_item(tags, str))
			return 1;
	}

	return 0;
}

/* Destructively merge a new config tree into an existing one */
int merge_config_tree(struct cmd_context *cmd, struct dm_config_tree *cft,
		      struct dm_config_tree *newdata, config_merge_t merge_type)
{
	struct dm_config_node *root = cft->root;
	struct dm_config_node *cn, *nextn, *oldn, *cn2;
	const struct dm_config_node *tn;
	struct config_source *cs, *csn;

	for (cn = newdata->root; cn; cn = nextn) {
		nextn = cn->sib;
		if (merge_type == CONFIG_MERGE_TYPE_TAGS) {
			/* Ignore tags section */
			if (!strcmp(cn->key, "tags"))
				continue;
			/* If there's a tags node, skip if host tags don't match */
			if ((tn = dm_config_find_node(cn->child, "tags"))) {
				if (!_match_host_tags(&cmd->tags, tn))
					continue;
			}
		}
		if (!(oldn = dm_config_find_node(root, cn->key))) {
			_insert_config_node(&cft->root, cn);
			if (merge_type == CONFIG_MERGE_TYPE_TAGS) {
				/* Remove any "tags" nodes */
				for (cn2 = cn->child; cn2; cn2 = cn2->sib) {
					if (!strcmp(cn2->key, "tags")) {
						cn->child = cn2->sib;
						continue;
					}
					if (cn2->sib && !strcmp(cn2->sib->key, "tags")) {
						cn2->sib = cn2->sib->sib;
						continue;
					}
				}
			}
			continue;
		}
		_merge_section(oldn, cn, merge_type);
	}

	/*
	 * Persistent filter loading is based on timestamp,
	 * so we need to know the newest timestamp to make right decision
	 * whether the .cache isn't older then any of configs
	 */
	cs = dm_config_get_custom(cft);
	csn = dm_config_get_custom(newdata);

	if (cs && csn && timespeccmp(&cs->timestamp, &csn->timestamp, <))
		cs->timestamp = csn->timestamp;

	return 1;
}

struct out_baton {
	FILE *fp;
	struct config_def_tree_spec *tree_spec;
	struct dm_pool *mem;
};

#define MAX_COMMENT_LINE 512

static int _copy_one_line(const char *comment, char *line, int *pos, int len)
{
	int p;
	int i = 0;
	char c;

	if (*pos >= len)
		return 0;

	memset(line, 0, MAX_COMMENT_LINE+1);

	for (p = *pos; ; p++) {
		c = comment[p];

		(*pos)++;

		if (c == '\n' || c == '\0')
			break;

		line[i++] = c;

		if (i == MAX_COMMENT_LINE)
		       break;
	}

	return i;
}

static int _get_config_node_version(uint16_t version_enc, char *version)
{
	if (dm_snprintf(version, 9, "%u.%u.%u",
			(version_enc & 0xE000) >> 13,
			(version_enc & 0x1E00) >> 9,
			(version_enc & 0x1FF)) == -1) {
		log_error("_get_config_node_version: couldn't create version string");
		return 0;
	}

	return 1;
}

static int _def_node_is_deprecated(cfg_def_item_t *def, struct config_def_tree_spec *spec)
{
	return def->deprecated_since_version &&
	       (spec->version >= def->deprecated_since_version);
}

static int _out_prefix_fn(const struct dm_config_node *cn, const char *line, void *baton)
{
	struct out_baton *out = baton;
	struct cfg_def_item *cfg_def;
	char version[9]; /* 8+1 chars for max version of 7.15.511 */
	const char *node_type_name = cn->v ? "option" : "section";
	char path[CFG_PATH_MAX_LEN];
	char commentline[MAX_COMMENT_LINE+1];

	if (cn->id <= 0)
		return 1;

	if (out->tree_spec->type == CFG_DEF_TREE_LIST)
		return 1;

	if ((out->tree_spec->type == CFG_DEF_TREE_DIFF) &&
	    (!(out->tree_spec->check_status[cn->id] & CFG_DIFF)))
		return 1;

	cfg_def = cfg_def_get_item_p(cn->id);

	if (out->tree_spec->withsummary || out->tree_spec->withcomments) {
		_cfg_def_make_path(path, sizeof(path), cfg_def->id, cfg_def, 1);
		fprintf(out->fp, "\n");
		fprintf(out->fp, "%s# Configuration %s %s.\n", line, node_type_name, path);

		if (out->tree_spec->withcomments &&
		    _def_node_is_deprecated(cfg_def, out->tree_spec))
			fprintf(out->fp, "%s# %s", line, cfg_def->deprecation_comment);

		if (cfg_def->comment) {
			int pos = 0;
			while (_copy_one_line(cfg_def->comment, commentline, &pos, strlen(cfg_def->comment))) {
				if ((commentline[0] == '#') && (strlen(commentline) == 1)) {
					if (!out->tree_spec->withspaces)
						continue;
					commentline[0] = '\0';
				}
				fprintf(out->fp, "%s# %s\n", line, commentline);
				/* withsummary prints only the first comment line. */
				if (!out->tree_spec->withcomments)
					break;
			}
		}

		if (_def_node_is_deprecated(cfg_def, out->tree_spec))
			fprintf(out->fp, "%s# This configuration %s is deprecated.\n", line, node_type_name);

		if (cfg_def->flags & CFG_ADVANCED)
			fprintf(out->fp, "%s# This configuration %s is advanced.\n", line, node_type_name);

		if (cfg_def->flags & CFG_UNSUPPORTED)
			fprintf(out->fp, "%s# This configuration %s is not officially supported.\n", line, node_type_name);

		if (cfg_def->flags & CFG_NAME_VARIABLE)
			fprintf(out->fp, "%s# This configuration %s has variable name.\n", line, node_type_name);

		if (cfg_def->flags & CFG_DEFAULT_UNDEFINED)
			fprintf(out->fp, "%s# This configuration %s does not have a default value defined.\n", line, node_type_name);

		if (cfg_def->flags & CFG_DEFAULT_COMMENTED)
			fprintf(out->fp, "%s# This configuration %s has an automatic default value.\n", line, node_type_name);

		if ((out->tree_spec->type == CFG_DEF_TREE_FULL) &&
		    (out->tree_spec->check_status[cn->id] & CFG_USED))
			fprintf(out->fp, "%s# Value defined in existing configuration has been used for this setting.\n", line);
	}

	if (out->tree_spec->withversions) {
		if (!_get_config_node_version(cfg_def->since_version, version))
			return_0;
		fprintf(out->fp, "%s# Available since version %s.\n", line, version);

		if (_def_node_is_deprecated(cfg_def, out->tree_spec)) {
			if (!_get_config_node_version(cfg_def->deprecated_since_version, version))
				return_0;
			fprintf(out->fp, "%s# Deprecated since version %s.\n", line, version);
		}
	}

	return 1;
}

static int _should_print_cfg_with_undef_def_val(struct out_baton *out, cfg_def_item_t *cfg_def,
						const struct dm_config_node *cn)
{
	if (!(cfg_def->flags & CFG_DEFAULT_UNDEFINED))
		return 1;

	/* print it only if the value is directly defined in some config = it's used */
	return out->tree_spec->check_status && (out->tree_spec->check_status[cn->id] & CFG_USED);
}

static int _out_line_fn(const struct dm_config_node *cn, const char *line, void *baton)
{
	struct out_baton *out = baton;
	struct cfg_def_item *cfg_def;
	char config_path[CFG_PATH_MAX_LEN];
	char summary[MAX_COMMENT_LINE+1];
	char version[9];
	int pos = 0;
	size_t len;
	char *space_prefix;

	if ((out->tree_spec->type == CFG_DEF_TREE_DIFF) &&
	    (!(out->tree_spec->check_status[cn->id] & CFG_DIFF)))
		return 1;

	cfg_def = cfg_def_get_item_p(cn->id);

	if (out->tree_spec->type == CFG_DEF_TREE_LIST) {
		/* List view with node paths and summary. */
		if (cfg_def->type & CFG_TYPE_SECTION)
			return 1;
		if (!_cfg_def_make_path(config_path, CFG_PATH_MAX_LEN, cfg_def->id, cfg_def, 1))
			return_0;
		if (out->tree_spec->withversions && !_get_config_node_version(cfg_def->since_version, version))
			return_0;

		summary[0] = '\0';
		if (out->tree_spec->withsummary && cfg_def->comment)
			_copy_one_line(cfg_def->comment, summary, &pos, strlen(cfg_def->comment));

		fprintf(out->fp, "%s%s%s%s%s%s%s\n", config_path,
			*summary || out->tree_spec->withversions ? " - ": "",
			*summary ? summary : "",
			*summary ? " " : "",
			out->tree_spec->withversions ? "[" : "",
			out->tree_spec->withversions ? version : "",
			out->tree_spec->withversions ? "]" : "");

		return 1;
	}

	/* Usual tree view with nodes and their values. */

	if ((out->tree_spec->type != CFG_DEF_TREE_CURRENT) &&
	    (out->tree_spec->type != CFG_DEF_TREE_DIFF) &&
	    (out->tree_spec->type != CFG_DEF_TREE_FULL) &&
	    (cfg_def->flags & (CFG_DEFAULT_UNDEFINED | CFG_DEFAULT_COMMENTED))) {
		/* print with # at the front to comment out the line */
		if (_should_print_cfg_with_undef_def_val(out, cfg_def, cn)) {
			space_prefix = ((len = strspn(line, "\t "))) ? dm_pool_strndup(out->mem, line, len) : NULL;
			fprintf(out->fp, "%s%s%s\n", space_prefix ? : "", "# ", line + len);
			if (space_prefix)
				dm_pool_free(out->mem, space_prefix);
		}
		return 1;
	}

	/* print the line as it is */
	if (_should_print_cfg_with_undef_def_val(out, cfg_def, cn))
		fprintf(out->fp, "%s\n", line);

	return 1;
}

static int _out_suffix_fn(const struct dm_config_node *cn, const char *line, void *baton)
{
	return 1;
}

int config_write(struct dm_config_tree *cft,
		 struct config_def_tree_spec *tree_spec,
		 const char *file, int argc, char **argv)
{
	static const struct dm_config_node_out_spec _out_spec = {
		.prefix_fn = _out_prefix_fn,
		.line_fn = _out_line_fn,
		.suffix_fn = _out_suffix_fn
	};
	const struct dm_config_node *cn;
	struct out_baton baton = {
		.tree_spec = tree_spec,
		.mem = cft->mem
	};
	int r = 1;

	if (!file) {
		baton.fp = stdout;
		file = "stdout";
	} else if (!(baton.fp = fopen(file, "w"))) {
		log_sys_error("open", file);
		return 0;
	}

	log_verbose("Dumping configuration to %s", file);

	if (tree_spec->withgeneralpreamble)
		fprintf(baton.fp, CFG_PREAMBLE_GENERAL);
	if (tree_spec->withlocalpreamble)
		fprintf(baton.fp, CFG_PREAMBLE_LOCAL);

	if (!argc) {
		if (!dm_config_write_node_out(cft->root, &_out_spec, &baton)) {
			log_error("Failure while writing to %s", file);
			r = 0;
		}
	} else while (argc--) {
		if ((cn = dm_config_find_node(cft->root, *argv))) {
			if (!dm_config_write_one_node_out(cn, &_out_spec, &baton)) {
				log_error("Failure while writing to %s", file);
				r = 0;
			}
		} else {
			log_error("Configuration node %s not found", *argv);
			r = 0;
		}
		argv++;
	}

	if (baton.fp && baton.fp != stdout && dm_fclose(baton.fp)) {
		stack;
		r = 0;
	}

	return r;
}

static struct dm_config_node *_add_def_node(struct dm_config_tree *cft,
					    struct config_def_tree_spec *spec,
					    struct dm_config_node *parent,
					    struct dm_config_node *relay,
					    cfg_def_item_t *def)
{
	struct dm_config_node *cn;
	const char *str;
	uint32_t format_flags = 0;

	if (!(cn = dm_config_create_node(cft, def->name))) {
		log_error("Failed to create default config setting node.");
		return NULL;
	}

	if (!(def->type & CFG_TYPE_SECTION) && !(def->type & CFG_TYPE_ARRAY)) {
		if (!(cn->v = dm_config_create_value(cft))) {
			log_error("Failed to create default config setting node value.");
			return NULL;
		}
		if (spec->withspaces)
			format_flags |= DM_CONFIG_VALUE_FMT_COMMON_EXTRA_SPACES;
	}

	cn->id = def->id;

	if (spec->unconfigured && def->default_unconfigured_value.v_UNCONFIGURED) {
		cn->v->type = DM_CFG_STRING;
		cn->v->v.str = cfg_def_get_default_unconfigured_value_hint(spec->cmd, def);
		if (def->type != CFG_TYPE_STRING)
			format_flags |= DM_CONFIG_VALUE_FMT_STRING_NO_QUOTES;
		dm_config_value_set_format_flags(cn->v, format_flags);
	} else if (!(def->type & CFG_TYPE_ARRAY)) {
		switch (def->type) {
			case CFG_TYPE_SECTION:
				cn->v = NULL;
				break;
			case CFG_TYPE_BOOL:
				cn->v->type = DM_CFG_INT;
				cn->v->v.i = cfg_def_get_default_value_hint(spec->cmd, def, CFG_TYPE_BOOL, NULL);
				break;
			case CFG_TYPE_INT:
				cn->v->type = DM_CFG_INT;
				cn->v->v.i = cfg_def_get_default_value_hint(spec->cmd, def, CFG_TYPE_INT, NULL);
				if (def->flags & CFG_FORMAT_INT_OCTAL)
					format_flags |= DM_CONFIG_VALUE_FMT_INT_OCTAL;
				break;
			case CFG_TYPE_FLOAT:
				cn->v->type = DM_CFG_FLOAT;
				cn->v->v.f = cfg_def_get_default_value_hint(spec->cmd, def, CFG_TYPE_FLOAT, NULL);
				break;
			case CFG_TYPE_STRING:
				cn->v->type = DM_CFG_STRING;
				if (!(str = cfg_def_get_default_value_hint(spec->cmd, def, CFG_TYPE_STRING, NULL)))
					str = "";
				cn->v->v.str = str;
				break;
			default:
				log_error(INTERNAL_ERROR "_add_def_node: unknown type");
				return NULL;
				break;
		}
		dm_config_value_set_format_flags(cn->v, format_flags);
	} else {
		if (spec->withspaces)
			format_flags |= DM_CONFIG_VALUE_FMT_COMMON_EXTRA_SPACES;
		format_flags |= DM_CONFIG_VALUE_FMT_COMMON_ARRAY;
		cn->v = _get_def_array_values(spec->cmd, cft, def, format_flags);
	}

	cn->child = NULL;
	if (parent) {
		cn->parent = parent;
		if (!parent->child)
			parent->child = cn;
	} else
		cn->parent = cn;

	if (relay)
		relay->sib = cn;

	return cn;
}

static int _should_skip_deprecated_def_node(cfg_def_item_t *def, struct config_def_tree_spec *spec)
{
	return spec->ignoredeprecated && _def_node_is_deprecated(def, spec);
}

static int _should_skip_def_node(struct config_def_tree_spec *spec, int section_id, int id)
{
	cfg_def_item_t *def = cfg_def_get_item_p(id);
	uint16_t flags;

	if ((def->parent != section_id) ||
	    (spec->ignoreadvanced && def->flags & CFG_ADVANCED) ||
	    (spec->ignoreunsupported && def->flags & CFG_UNSUPPORTED))
		return 1;

	switch (spec->type) {
		case CFG_DEF_TREE_FULL:
			/* fall through */
		case CFG_DEF_TREE_MISSING:
			if (!spec->check_status) {
				log_error_once(INTERNAL_ERROR "couldn't determine missing "
				       "config nodes - unknown status of last config check.");
				return 1;
			}
			if ((spec->check_status[id] & CFG_USED) ||
			    (def->flags & CFG_NAME_VARIABLE))
				return 1;

			if ((spec->type == CFG_DEF_TREE_MISSING) &&
			    ((def->since_version > spec->version) ||
			     _should_skip_deprecated_def_node(def, spec)))
				return 1;
			break;
		case CFG_DEF_TREE_NEW:
			if ((def->since_version != spec->version) ||
			    _should_skip_deprecated_def_node(def, spec))
				return 1;
			break;
		case CFG_DEF_TREE_NEW_SINCE:
			if ((def->since_version < spec->version) ||
			    _should_skip_deprecated_def_node(def, spec))
				return 1;
			break;
		case CFG_DEF_TREE_PROFILABLE:
			/* fall through */
		case CFG_DEF_TREE_PROFILABLE_CMD:
			/* fall through */
		case CFG_DEF_TREE_PROFILABLE_MDA:
			if (!(def->flags & CFG_PROFILABLE) ||
			    (def->since_version > spec->version) ||
			    _should_skip_deprecated_def_node(def, spec))
				return 1;
			flags = def->flags & ~CFG_PROFILABLE;
			if (spec->type == CFG_DEF_TREE_PROFILABLE_CMD) {
				if (flags & CFG_PROFILABLE_METADATA)
					return 1;
			} else if (spec->type == CFG_DEF_TREE_PROFILABLE_MDA) {
				if (!(flags & CFG_PROFILABLE_METADATA))
					return 1;
			}
			break;
		default:
			if ((def->since_version > spec->version) ||
			    _should_skip_deprecated_def_node(def, spec))
				return 1;
			break;
	}

	return 0;
}

static struct dm_config_node *_add_def_section_subtree(struct dm_config_tree *cft,
						       struct config_def_tree_spec *spec,
						       struct dm_config_node *parent,
						       struct dm_config_node *relay,
						       int section_id)
{
	struct dm_config_node *cn = NULL, *relay_sub = NULL, *tmp;
	cfg_def_item_t *def;
	int id;

	for (id = 0; id < CFG_COUNT; id++) {
		if (_should_skip_def_node(spec, section_id, id))
			continue;

		if (!cn && !(cn = _add_def_node(cft, spec, parent, relay, cfg_def_get_item_p(section_id))))
				goto bad;

		def = cfg_def_get_item_p(id);
		if ((tmp = def->type == CFG_TYPE_SECTION ? _add_def_section_subtree(cft, spec, cn, relay_sub, id)
							 : _add_def_node(cft, spec, cn, relay_sub, def)))
			relay_sub = tmp;
	}

	return cn;
bad:
	log_error("Failed to create default config section node.");
	return NULL;
}

struct dm_config_tree *config_def_create_tree(struct config_def_tree_spec *spec)
{
	struct dm_config_tree *cft = NULL, *tmp_cft = NULL;
	struct dm_config_node *root = NULL, *relay = NULL, *tmp;
	int id;

	if (!(cft = dm_config_create())) {
		log_error("Failed to create default config tree.");
		return NULL;
	}

	for (id = root_CFG_SECTION + 1; id < CFG_COUNT; id++) {
		if (cfg_def_get_item_p(id)->parent != root_CFG_SECTION)
			continue;

		if (spec->ignorelocal && (id == local_CFG_SECTION))
			continue;

		if ((tmp = _add_def_section_subtree(cft, spec, root, relay, id))) {
			relay = tmp;
			if (!root)
				root = relay;
		}
	}

	cft->root = root;

	if (spec->type == CFG_DEF_TREE_FULL) {
		if (!(tmp_cft = dm_config_create())) {
			log_error("Failed to create temporary config tree while creating full tree.");
			goto bad;
		}

		if (!(tmp_cft->root = dm_config_clone_node_with_mem(cft->mem, spec->current_cft->root, 1))) {
			log_error("Failed to clone current config tree.");
			goto bad;
		}

		if (!merge_config_tree(spec->cmd, cft, tmp_cft, CONFIG_MERGE_TYPE_RAW)) {
			log_error("Failed to merge default and current config tree.");
			goto bad;
		}

		dm_config_destroy(tmp_cft);
	}

	return cft;
bad:
	if (cft)
		dm_config_destroy(cft);
	if (tmp_cft)
		dm_config_destroy(tmp_cft);
	return NULL;
}

int config_force_check(struct cmd_context *cmd, config_source_t source, struct dm_config_tree *cft)
{
	struct cft_check_handle *handle;
	int r;

	if (!(handle = dm_pool_zalloc(cmd->libmem, sizeof(*handle)))) {
		log_debug("_check_profile: profile check handle allocation failed");
		return 0;
	}

	handle->cmd = cmd;
	handle->cft = cft;
	handle->source = source;
	handle->force_check = 1;
	/* provide warning messages only if config/checks=1 */
	handle->suppress_messages = !find_config_tree_bool(cmd, config_checks_CFG, NULL);

	/*
	 * Some settings can't be changed if we're running commands interactively
	 * within lvm shell so check for them in case we're in this interactive mode.
	 */
	if (cmd->is_interactive)
		handle->disallowed_flags |= CFG_DISALLOW_INTERACTIVE;

	r = config_def_check(handle);

	dm_pool_free(cmd->libmem, handle);
	return r;
}

static int _get_profile_from_list(struct dm_list *list, const char *profile_name,
				  config_source_t source, struct profile **profile_found)
{
	struct profile *profile;

	dm_list_iterate_items(profile, list) {
		if (!strcmp(profile->name, profile_name)) {
			if (profile->source == source) {
				*profile_found = profile;
				return 1;
			}
			log_error(INTERNAL_ERROR "Profile %s already added as "
				  "%s type, but requested type is %s.",
				   profile_name,
				   _config_source_names[profile->source],
				   _config_source_names[source]);
			return 0;
		}
	}

	*profile_found = NULL;
	return 1;
}

struct profile *add_profile(struct cmd_context *cmd, const char *profile_name, config_source_t source)
{
	struct profile *profile;

	/* Do some sanity checks first. */
	if (!_is_profile_based_config_source(source)) {
		log_error(INTERNAL_ERROR "add_profile: incorrect configuration "
			  "source, expected %s or %s but %s requested",
			  _config_source_names[CONFIG_PROFILE_COMMAND],
			  _config_source_names[CONFIG_PROFILE_METADATA],
			  _config_source_names[source]);
		return NULL;
	}

	if (!profile_name || !*profile_name) {
		log_error("Undefined profile name.");
		return NULL;
	}

	if (strchr(profile_name, '/')) {
		log_error("%s: bad profile name, it contains '/'.", profile_name);
		return NULL;
	}

	/*
	 * Check if the profile is on the list of profiles to be loaded or if
	 * not found there, if it's on the list of already loaded profiles.
	 */
	if (!_get_profile_from_list(&cmd->profile_params->profiles_to_load,
				    profile_name, source, &profile))
		return_NULL;

	if (profile)
		profile->source = source;
	else if (!_get_profile_from_list(&cmd->profile_params->profiles,
					 profile_name, source, &profile))
		return_NULL;

	if (profile) {
		if (profile->source != source) {
			log_error(INTERNAL_ERROR "add_profile: loaded profile "
				  "has incorrect type, expected %s but %s found",
				   _config_source_names[source],
				   _config_source_names[profile->source]);
			return NULL;
		}
		return profile;
	}

	if (!(profile = dm_pool_zalloc(cmd->libmem, sizeof(*profile)))) {
		log_error("profile allocation failed");
		return NULL;
	}

	profile->source = source;
	profile->name = dm_pool_strdup(cmd->libmem, profile_name);
	dm_list_add(&cmd->profile_params->profiles_to_load, &profile->list);

	return profile;
}

int load_profile(struct cmd_context *cmd, struct profile *profile) {
	static char profile_path[PATH_MAX];

	if (critical_section()) {
		log_error(INTERNAL_ERROR "trying to load profile %s "
			  "in critical section.", profile->name);
		return 0;
	}

	if (profile->cft)
		return 1;

	if (dm_snprintf(profile_path, sizeof(profile_path), "%s/%s.profile",
		cmd->profile_params->dir, profile->name) < 0) {
		log_error("LVM_SYSTEM_DIR or profile name too long");
		return 0;
	}

	if (!(profile->cft = config_file_open_and_read(profile_path, profile->source, cmd)))
		return 0;

	/*
	 * *Profile must be valid* otherwise we'd end up with incorrect config!
	 * If there were config items present that are not supposed to be
	 * customized by a profile, we could end up with non-deterministic
	 * behaviour. Therefore, this check is *strictly forced* even if
	 * config/checks=0. The config/checks=0 will only cause the warning
	 * messages to be suppressed, but the check itself is always done
	 * for profiles!
	 */
	if (!config_force_check(cmd, profile->source, profile->cft)) {
		log_error("Ignoring invalid %s %s.",
			  _config_source_names[profile->source], profile->name);
		config_destroy(profile->cft);
		profile->cft = NULL;
		return 0;
	}

	dm_list_move(&cmd->profile_params->profiles, &profile->list);
	return 1;
}

int load_pending_profiles(struct cmd_context *cmd)
{
	struct profile *profile, *temp_profile;
	int r = 1;

	dm_list_iterate_items_safe(profile, temp_profile, &cmd->profile_params->profiles_to_load) {
		if (!load_profile(cmd, profile))
			r = 0;
	}

	return r;
}

const char *get_default_devices_cache_dir_CFG(struct cmd_context *cmd, struct profile *profile)
{
	static char buf[PATH_MAX];

	if (dm_snprintf(buf, sizeof(buf), "%s/%s", cmd->system_dir, DEFAULT_CACHE_SUBDIR) < 0) {
		log_error("Persistent cache directory name too long.");
		return NULL;
	}

	return dm_pool_strdup(cmd->mem, buf);
}

const char *get_default_unconfigured_devices_cache_dir_CFG(struct cmd_context *cmd)
{
	return "@DEFAULT_SYS_DIR@/@DEFAULT_CACHE_SUBDIR@";
}

const char *get_default_devices_cache_CFG(struct cmd_context *cmd, struct profile *profile)
{
	const char *cache_dir = NULL, *cache_file_prefix = NULL;
	static char buf[PATH_MAX];

	/*
	 * If 'cache_dir' or 'cache_file_prefix' is set, ignore 'cache'.
	*/
	if (find_config_tree_node(cmd, devices_cache_dir_CFG, profile))
		cache_dir = find_config_tree_str(cmd, devices_cache_dir_CFG, profile);
	if (find_config_tree_node(cmd, devices_cache_file_prefix_CFG, profile))
		cache_file_prefix = find_config_tree_str_allow_empty(cmd, devices_cache_file_prefix_CFG, profile);

	if (cache_dir || cache_file_prefix) {
		if (dm_snprintf(buf, sizeof(buf),
				"%s%s%s/%s.cache",
				cache_dir ? "" : cmd->system_dir,
				cache_dir ? "" : "/",
				cache_dir ? : DEFAULT_CACHE_SUBDIR,
				cache_file_prefix ? : DEFAULT_CACHE_FILE_PREFIX) < 0) {
			log_error("Persistent cache filename too long.");
			return NULL;
		}
		return dm_pool_strdup(cmd->mem, buf);
	}

	if (dm_snprintf(buf, sizeof(buf), "%s/%s/%s.cache", cmd->system_dir,
			DEFAULT_CACHE_SUBDIR, DEFAULT_CACHE_FILE_PREFIX) < 0) {
		log_error("Persistent cache filename too long.");
		return NULL;
	}
	return dm_pool_strdup(cmd->mem, buf);
}

const char *get_default_unconfigured_devices_cache_CFG(struct cmd_context *cmd)
{
	const char *cache_file_prefix = NULL;
	static char buf[PATH_MAX];

	if (find_config_tree_node(cmd, devices_cache_file_prefix_CFG, NULL))
		cache_file_prefix = find_config_tree_str_allow_empty(cmd, devices_cache_file_prefix_CFG, NULL);

	if (dm_snprintf(buf, sizeof(buf), "%s/%s.cache",
			get_default_unconfigured_devices_cache_dir_CFG(cmd),
			cache_file_prefix ? : DEFAULT_CACHE_FILE_PREFIX) < 0) {
		log_error("Persistent cache filename too long.");
		return NULL;
	}

	return dm_pool_strdup(cmd->mem, buf);
}

const char *get_default_backup_backup_dir_CFG(struct cmd_context *cmd, struct profile *profile)
{
	static char buf[PATH_MAX];

	if (dm_snprintf(buf, sizeof(buf), "%s/%s", cmd->system_dir, DEFAULT_BACKUP_SUBDIR) == -1) {
		log_error("Couldn't create default backup path '%s/%s'.",
			  cmd->system_dir, DEFAULT_BACKUP_SUBDIR);
		return NULL;
	}

	return dm_pool_strdup(cmd->mem, buf);
}

const char *get_default_unconfigured_backup_backup_dir_CFG(struct cmd_context *cmd)
{
	return "@DEFAULT_SYS_DIR@/@DEFAULT_BACKUP_SUBDIR@";
}

const char *get_default_backup_archive_dir_CFG(struct cmd_context *cmd, struct profile *profile)
{
	static char buf[PATH_MAX];

	if (dm_snprintf (buf, sizeof(buf), "%s/%s", cmd->system_dir, DEFAULT_ARCHIVE_SUBDIR) == -1) {
		log_error("Couldn't create default archive path '%s/%s'.",
			  cmd->system_dir, DEFAULT_ARCHIVE_SUBDIR);
		return NULL;
	}

	return dm_pool_strdup(cmd->mem, buf);
}

const char *get_default_unconfigured_backup_archive_dir_CFG(struct cmd_context *cmd)
{
	return "@DEFAULT_SYS_DIR@/@DEFAULT_ARCHIVE_SUBDIR@";
}

const char *get_default_config_profile_dir_CFG(struct cmd_context *cmd, struct profile *profile)
{
	static char buf[PATH_MAX];

	if (dm_snprintf(buf, sizeof(buf), "%s/%s", cmd->system_dir, DEFAULT_PROFILE_SUBDIR) == -1) {
		log_error("Couldn't create default profile path '%s/%s'.",
			  cmd->system_dir, DEFAULT_PROFILE_SUBDIR);
		return NULL;
	}

	return dm_pool_strdup(cmd->mem, buf);
}

const char *get_default_unconfigured_config_profile_dir_CFG(struct cmd_context *cmd)
{
	return "@DEFAULT_SYS_DIR@/@DEFAULT_PROFILE_SUBDIR@";
}

const char *get_default_activation_mirror_image_fault_policy_CFG(struct cmd_context *cmd, struct profile *profile)
{
	return find_config_tree_str(cmd, activation_mirror_device_fault_policy_CFG, profile);
}

int get_default_allocation_thin_pool_chunk_size_CFG(struct cmd_context *cmd, struct profile *profile)
{
	uint32_t chunk_size;
	int chunk_size_calc_method;

	if (!get_default_allocation_thin_pool_chunk_size(cmd, profile, &chunk_size,
							 &chunk_size_calc_method)) {
		stack; /* Ignore this error, never happens... */
		chunk_size = DEFAULT_THIN_POOL_CHUNK_SIZE * 2;
	}

	return (int) chunk_size;
}

int get_default_allocation_cache_pool_chunk_size_CFG(struct cmd_context *cmd, struct profile *profile)
{
	return DEFAULT_CACHE_POOL_CHUNK_SIZE * 2;
}

uint64_t get_default_allocation_cache_pool_max_chunks_CFG(struct cmd_context *cmd, struct profile *profile)
{
	static int _warn_max_chunks = 0;
	/*
	 * TODO: In future may depend on the cache target version,
	 *       newer targets may scale better.
	 */
	uint64_t default_max_chunks = DEFAULT_CACHE_POOL_MAX_CHUNKS;
	uint64_t max_chunks = find_config_tree_int(cmd, allocation_cache_pool_max_chunks_CFG, profile);

	if (!max_chunks)
		max_chunks = default_max_chunks;
	else if (max_chunks > default_max_chunks)
		/* Still warn the user when the value is tweaked above recommended level */
		/* Maybe drop to log_verbose... */
		log_warn_suppress(_warn_max_chunks++, "WARNING: Configured cache_pool_max_chunks value "
				  FMTu64 " is higher then recommended " FMTu64 ".",
				  max_chunks, default_max_chunks);

	return max_chunks;
}
