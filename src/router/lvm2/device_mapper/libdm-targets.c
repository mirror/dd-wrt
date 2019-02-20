/*
 * Copyright (C) 2005-2015 Red Hat, Inc. All rights reserved.
 *
 * This file is part of the device-mapper userspace tools.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "misc/dmlib.h"
#include "libdm-common.h"

int dm_get_status_snapshot(struct dm_pool *mem, const char *params,
			   struct dm_status_snapshot **status)
{
	struct dm_status_snapshot *s;
	int r;

	if (!params) {
		log_error("Failed to parse invalid snapshot params.");
		return 0;
	}

	if (!(s = dm_pool_zalloc(mem, sizeof(*s)))) {
		log_error("Failed to allocate snapshot status structure.");
		return 0;
	}

	r = sscanf(params, FMTu64 "/" FMTu64 " " FMTu64,
		   &s->used_sectors, &s->total_sectors,
		   &s->metadata_sectors);

	if (r == 3 || r == 2)
		s->has_metadata_sectors = (r == 3);
	else if (!strcmp(params, "Invalid"))
		s->invalid = 1;
	else if (!strcmp(params, "Merge failed"))
		s->merge_failed = 1;
	else if (!strcmp(params, "Overflow"))
		s->overflow = 1;
	else {
		dm_pool_free(mem, s);
		log_error("Failed to parse snapshot params: %s.", params);
		return 0;
	}

	*status = s;

	return 1;
}

/*
 * Skip nr fields each delimited by a single space.
 * FIXME Don't assume single space.
 */
static const char *_skip_fields(const char *p, unsigned nr)
{
	while (p && nr-- && (p = strchr(p, ' ')))
		p++;

	return p;
}

/*
 * Count number of single-space delimited fields.
 * Number of fields is number of spaces plus one.
 */
static unsigned _count_fields(const char *p)
{
	unsigned nr = 1;

	if (!p || !*p)
		return 0;

	while ((p = _skip_fields(p, 1)))
		nr++;

	return nr;
}

/*
 * Various RAID status versions include:
 * Versions < 1.5.0 (4 fields):
 *   <raid_type> <#devs> <health_str> <sync_ratio>
 * Versions 1.5.0+  (6 fields):
 *   <raid_type> <#devs> <health_str> <sync_ratio> <sync_action> <mismatch_cnt>
 * Versions 1.9.0+  (7 fields):
 *   <raid_type> <#devs> <health_str> <sync_ratio> <sync_action> <mismatch_cnt> <data_offset>
 */
int dm_get_status_raid(struct dm_pool *mem, const char *params,
		       struct dm_status_raid **status)
{
	int i;
	unsigned num_fields;
	const char *p, *pp, *msg_fields = "";
	struct dm_status_raid *s = NULL;
	unsigned a = 0;

	if ((num_fields = _count_fields(params)) < 4)
		goto_bad;

	/* Second field holds the device count */
	msg_fields = "<#devs> ";
	if (!(p = _skip_fields(params, 1)) || (sscanf(p, "%d", &i) != 1))
		goto_bad;

	msg_fields = "";
	if (!(s = dm_pool_zalloc(mem, sizeof(struct dm_status_raid))))
		goto_bad;

	if (!(s->raid_type = dm_pool_zalloc(mem, p - params)))
		goto_bad; /* memory is freed when pool is destroyed */

	if (!(s->dev_health = dm_pool_zalloc(mem, i + 1))) /* Space for health chars */
		goto_bad;

	msg_fields = "<raid_type> <#devices> <health_chars> and <sync_ratio> ";
	if (sscanf(params, "%s %u %s " FMTu64 "/" FMTu64,
		   s->raid_type,
		   &s->dev_count,
		   s->dev_health,
		   &s->insync_regions,
		   &s->total_regions) != 5)
		goto_bad;

	/*
	 * All pre-1.5.0 version parameters are read.  Now we check
	 * for additional 1.5.0+ parameters (i.e. num_fields at least 6).
	 *
	 * Note that 'sync_action' will be NULL (and mismatch_count
	 * will be 0) if the kernel returns a pre-1.5.0 status.
	 */
	if (num_fields < 6)
		goto out;

	msg_fields = "<sync_action> and <mismatch_cnt> ";

	/* Skip pre-1.5.0 params */
	if (!(p = _skip_fields(params, 4)) || !(pp = _skip_fields(p, 1)))
		goto_bad;

	if (!(s->sync_action = dm_pool_zalloc(mem, pp - p)))
		goto_bad;

	if (sscanf(p, "%s " FMTu64, s->sync_action, &s->mismatch_count) != 2)
		goto_bad;

	if (num_fields < 7)
		goto out;

	/*
	 * All pre-1.9.0 version parameters are read.  Now we check
	 * for additional 1.9.0+ parameters (i.e. nr_fields at least 7).
	 *
	 * Note that data_offset will be 0 if the
	 * kernel returns a pre-1.9.0 status.
	 */
	msg_fields = "<data_offset>";
	if (!(p = _skip_fields(params, 6))) /* skip pre-1.9.0 params */
		goto bad;
	if (sscanf(p, FMTu64, &s->data_offset) != 1)
		goto bad;

out:
	*status = s;

	if (s->insync_regions == s->total_regions) {
		/* FIXME: kernel gives misleading info here
		 * Trying to recognize a true state */
		while (i-- > 0)
			if (s->dev_health[i] == 'a')
				a++; /* Count number of 'a' */

		if (a && a < s->dev_count) {
			/* SOME legs are in 'a' */
			if (!strcasecmp(s->sync_action, "recover")
			    || !strcasecmp(s->sync_action, "idle"))
				/* Kernel may possibly start some action
				 * in near-by future, do not report 100% */
				s->insync_regions--;
		}
	}

	return 1;

bad:
	log_error("Failed to parse %sraid params: %s", msg_fields, params);

	if (s)
		dm_pool_free(mem, s);

	*status = NULL;

	return 0;
}

/*
 * <metadata block size> <#used metadata blocks>/<#total metadata blocks>
 * <cache block size> <#used cache blocks>/<#total cache blocks>
 * <#read hits> <#read misses> <#write hits> <#write misses>
 * <#demotions> <#promotions> <#dirty> <#features> <features>*
 * <#core args> <core args>* <policy name> <#policy args> <policy args>*
 *
 * metadata block size      : Fixed block size for each metadata block in
 *                            sectors
 * #used metadata blocks    : Number of metadata blocks used
 * #total metadata blocks   : Total number of metadata blocks
 * cache block size         : Configurable block size for the cache device
 *                            in sectors
 * #used cache blocks       : Number of blocks resident in the cache
 * #total cache blocks      : Total number of cache blocks
 * #read hits               : Number of times a READ bio has been mapped
 *                            to the cache
 * #read misses             : Number of times a READ bio has been mapped
 *                            to the origin
 * #write hits              : Number of times a WRITE bio has been mapped
 *                            to the cache
 * #write misses            : Number of times a WRITE bio has been
 *                            mapped to the origin
 * #demotions               : Number of times a block has been removed
 *                            from the cache
 * #promotions              : Number of times a block has been moved to
 *                            the cache
 * #dirty                   : Number of blocks in the cache that differ
 *                            from the origin
 * #feature args            : Number of feature args to follow
 * feature args             : 'writethrough' (optional)
 * #core args               : Number of core arguments (must be even)
 * core args                : Key/value pairs for tuning the core
 *                            e.g. migration_threshold
 *			     *policy name              : Name of the policy
 * #policy args             : Number of policy arguments to follow (must be even)
 * policy args              : Key/value pairs
 *                            e.g. sequential_threshold
 */
int dm_get_status_cache(struct dm_pool *mem, const char *params,
			struct dm_status_cache **status)
{
	int i, feature_argc;
	char *str;
	const char *p, *pp;
	struct dm_status_cache *s;

	if (!(s = dm_pool_zalloc(mem, sizeof(struct dm_status_cache))))
		return_0;

	if (strstr(params, "Error")) {
		s->error = 1;
		s->fail = 1; /*  This is also I/O fail state */
		goto out;
	}

	if (strstr(params, "Fail")) {
		s->fail = 1;
		goto out;
	}

	/* Read in args that have definitive placement */
	if (sscanf(params,
		   " " FMTu32
		   " " FMTu64 "/" FMTu64
		   " " FMTu32
		   " " FMTu64 "/" FMTu64
		   " " FMTu64 " " FMTu64
		   " " FMTu64 " " FMTu64
		   " " FMTu64 " " FMTu64
		   " " FMTu64
		   " %d",
		   &s->metadata_block_size,
		   &s->metadata_used_blocks, &s->metadata_total_blocks,
		   &s->block_size, /* AKA, chunk_size */
		   &s->used_blocks, &s->total_blocks,
		   &s->read_hits, &s->read_misses,
		   &s->write_hits, &s->write_misses,
		   &s->demotions, &s->promotions,
		   &s->dirty_blocks,
		   &feature_argc) != 14)
		goto bad;

	/* Now jump to "features" section */
	if (!(p = _skip_fields(params, 12)))
		goto bad;

	/* Read in features */
	for (i = 0; i < feature_argc; i++) {
		if (!strncmp(p, "writethrough ", 13))
			s->feature_flags |= DM_CACHE_FEATURE_WRITETHROUGH;
		else if (!strncmp(p, "writeback ", 10))
			s->feature_flags |= DM_CACHE_FEATURE_WRITEBACK;
		else if (!strncmp(p, "passthrough ", 12))
			s->feature_flags |= DM_CACHE_FEATURE_PASSTHROUGH;
		else if (!strncmp(p, "metadata2 ", 10))
			s->feature_flags |= DM_CACHE_FEATURE_METADATA2;
		else
			log_error("Unknown feature in status: %s", params);

		if (!(p = _skip_fields(p, 1)))
			goto bad;
	}

	/* Read in core_args. */
	if (sscanf(p, "%d ", &s->core_argc) != 1)
		goto bad;
	if ((s->core_argc > 0) &&
	    (!(s->core_argv = dm_pool_zalloc(mem, sizeof(char *) * s->core_argc)) ||
	     !(p = _skip_fields(p, 1)) ||
	     !(str = dm_pool_strdup(mem, p)) ||
	     !(p = _skip_fields(p, (unsigned) s->core_argc)) ||
	     (dm_split_words(str, s->core_argc, 0, s->core_argv) != s->core_argc)))
		goto bad;

	/* Read in policy args */
	pp = p;
	if (!(p = _skip_fields(p, 1)) ||
	    !(s->policy_name = dm_pool_zalloc(mem, (p - pp))))
		goto bad;
	if (sscanf(pp, "%s %d", s->policy_name, &s->policy_argc) != 2)
		goto bad;
	if (s->policy_argc &&
	    (!(s->policy_argv = dm_pool_zalloc(mem, sizeof(char *) * s->policy_argc)) ||
	     !(p = _skip_fields(p, 1)) ||
	     !(str = dm_pool_strdup(mem, p)) ||
	     (dm_split_words(str, s->policy_argc, 0, s->policy_argv) != s->policy_argc)))
		goto bad;

	/* TODO: improve this parser */
	if (strstr(p, " ro"))
		s->read_only = 1;

	if (strstr(p, " needs_check"))
		s->needs_check = 1;
out:
	*status = s;
	return 1;

bad:
	log_error("Failed to parse cache params: %s", params);
	dm_pool_free(mem, s);
	*status = NULL;

	return 0;
}

int parse_thin_pool_status(const char *params, struct dm_status_thin_pool *s)
{
	int pos;

	memset(s, 0, sizeof(*s));

	if (!params) {
		log_error("Failed to parse invalid thin params.");
		return 0;
	}

	if (strstr(params, "Error")) {
		s->error = 1;
		s->fail = 1; /*  This is also I/O fail state */
		return 1;
	}

	if (strstr(params, "Fail")) {
		s->fail = 1;
		return 1;
	}

	/* FIXME: add support for held metadata root */
	if (sscanf(params, FMTu64 " " FMTu64 "/" FMTu64 " " FMTu64 "/" FMTu64 "%n",
		   &s->transaction_id,
		   &s->used_metadata_blocks,
		   &s->total_metadata_blocks,
		   &s->used_data_blocks,
		   &s->total_data_blocks, &pos) < 5) {
		log_error("Failed to parse thin pool params: %s.", params);
		return 0;
	}

	/* New status flags */
	if (strstr(params + pos, "no_discard_passdown"))
		s->discards = DM_THIN_DISCARDS_NO_PASSDOWN;
	else if (strstr(params + pos, "ignore_discard"))
		s->discards = DM_THIN_DISCARDS_IGNORE;
	else /* default discard_passdown */
		s->discards = DM_THIN_DISCARDS_PASSDOWN;

	/* Default is 'writable' (rw) data */
	if (strstr(params + pos, "out_of_data_space"))
		s->out_of_data_space = 1;
	else if (strstr(params + pos, "ro "))
		s->read_only = 1;

	/* Default is 'queue_if_no_space' */
	if (strstr(params + pos, "error_if_no_space"))
		s->error_if_no_space = 1;

	if (strstr(params + pos, "needs_check"))
		s->needs_check = 1;

	return 1;
}

int dm_get_status_thin_pool(struct dm_pool *mem, const char *params,
			    struct dm_status_thin_pool **status)
{
	struct dm_status_thin_pool *s;

	if (!(s = dm_pool_alloc(mem, sizeof(struct dm_status_thin_pool)))) {
		log_error("Failed to allocate thin_pool status structure.");
		return 0;
	}

	if (!parse_thin_pool_status(params, s)) {
		dm_pool_free(mem, s);
		return_0;
	}

	*status = s;

	return 1;
}

int dm_get_status_thin(struct dm_pool *mem, const char *params,
		       struct dm_status_thin **status)
{
	struct dm_status_thin *s;

	if (!(s = dm_pool_zalloc(mem, sizeof(struct dm_status_thin)))) {
		log_error("Failed to allocate thin status structure.");
		return 0;
	}

	if (strchr(params, '-')) {
		/* nothing to parse */
	} else if (strstr(params, "Fail")) {
		s->fail = 1;
	} else if (sscanf(params, FMTu64 " " FMTu64,
		   &s->mapped_sectors,
		   &s->highest_mapped_sector) != 2) {
		dm_pool_free(mem, s);
		log_error("Failed to parse thin params: %s.", params);
		return 0;
	}

	*status = s;

	return 1;
}

/*
 * dm core parms:	     0 409600 mirror
 * Mirror core parms:	     2 253:4 253:5 400/400
 * New-style failure params: 1 AA
 * New-style log params:     3 cluster 253:3 A
 *			 or  3 disk 253:3 A
 *			 or  1 core
 */
#define DM_MIRROR_MAX_IMAGES 8 /* limited by kernel DM_KCOPYD_MAX_REGIONS */

int dm_get_status_mirror(struct dm_pool *mem, const char *params,
			 struct dm_status_mirror **status)
{
	struct dm_status_mirror *s;
	const char *p, *pos = params;
	unsigned num_devs, argc, i;
	int used;

	if (!(s = dm_pool_zalloc(mem, sizeof(*s)))) {
		log_error("Failed to alloc mem pool to parse mirror status.");
		return 0;
	}

	if (sscanf(pos, "%u %n", &num_devs, &used) != 1)
		goto_out;
	pos += used;

	if (num_devs > DM_MIRROR_MAX_IMAGES) {
		log_error(INTERNAL_ERROR "More then " DM_TO_STRING(DM_MIRROR_MAX_IMAGES)
			  " reported in mirror status.");
		goto out;
	}

	if (!(s->devs = dm_pool_alloc(mem, num_devs * sizeof(*(s->devs))))) {
		log_error("Allocation of devs failed.");
		goto out;
	}

	for (i = 0; i < num_devs; ++i, pos += used)
		if (sscanf(pos, "%u:%u %n",
			   &(s->devs[i].major), &(s->devs[i].minor), &used) != 2)
			goto_out;

	if (sscanf(pos, FMTu64 "/" FMTu64 "%n",
		   &s->insync_regions, &s->total_regions, &used) != 2)
		goto_out;
	pos += used;

	if (sscanf(pos, "%u %n", &argc, &used) != 1)
		goto_out;
	pos += used;

	for (i = 0; i < num_devs ; ++i)
		s->devs[i].health = pos[i];

	if (!(pos = _skip_fields(pos, argc)))
		goto_out;

	if (strncmp(pos, "userspace", 9) == 0) {
		pos += 9;
		/* FIXME: support status of userspace mirror implementation */
	}

	if (sscanf(pos, "%u %n", &argc, &used) != 1)
		goto_out;
	pos += used;

	if (argc == 1) {
		/* core, cluster-core */
		if (!(s->log_type = dm_pool_strdup(mem, pos))) {
			log_error("Allocation of log type string failed.");
			goto out;
		}
	} else {
		if (!(p = _skip_fields(pos, 1)))
			goto_out;

		/* disk, cluster-disk */
		if (!(s->log_type = dm_pool_strndup(mem, pos, p - pos - 1))) {
			log_error("Allocation of log type string failed.");
			goto out;
		}
		pos = p;

		if ((argc > 2) && !strcmp(s->log_type, "disk")) {
			s->log_count = argc - 2;

			if (!(s->logs = dm_pool_alloc(mem, s->log_count * sizeof(*(s->logs))))) {
				log_error("Allocation of logs failed.");
				goto out;
			}

			for (i = 0; i < s->log_count; ++i, pos += used)
				if (sscanf(pos, "%u:%u %n",
					   &s->logs[i].major, &s->logs[i].minor, &used) != 2)
					goto_out;

			for (i = 0; i < s->log_count; ++i)
				s->logs[i].health = pos[i];
		}
	}

	s->dev_count = num_devs;
	*status = s;

	return 1;
out:
	log_error("Failed to parse mirror status %s.", params);
	dm_pool_free(mem, s);
	*status = NULL;

	return 0;
}
