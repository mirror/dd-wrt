/*
 * Copyright (C) 2016 Red Hat, Inc. All rights reserved.
 *
 * _stats_get_extents_for_file() based in part on filefrag_fiemap() from
 * e2fsprogs/misc/filefrag.c. Copyright 2003 by Theodore Ts'o.
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

#include "libdm/misc/dmlib.h"
#include "libdm/misc/kdev_t.h"

#include "math.h" /* log10() */

#include <sys/sysmacros.h>
#include <sys/ioctl.h>
#include <sys/vfs.h> /* fstatfs */

#ifdef __linux__
  #include <linux/fs.h> /* FS_IOC_FIEMAP */
#endif

#ifdef HAVE_LINUX_FIEMAP_H
  #include <linux/fiemap.h> /* fiemap */
#endif

#ifdef HAVE_LINUX_MAGIC_H
  #include <linux/magic.h> /* BTRFS_SUPER_MAGIC */
#endif

#define DM_STATS_REGION_NOT_PRESENT UINT64_MAX
#define DM_STATS_GROUP_NOT_PRESENT DM_STATS_GROUP_NONE

#define NSEC_PER_USEC   1000L
#define NSEC_PER_MSEC   1000000L
#define NSEC_PER_SEC    1000000000L

#define PRECISE_ARG "precise_timestamps"
#define HISTOGRAM_ARG "histogram:"

#define STATS_ROW_BUF_LEN 4096
#define STATS_MSG_BUF_LEN 1024
#define STATS_FIE_BUF_LEN 2048

#define SECTOR_SHIFT 9L

/* Histogram bin */
struct dm_histogram_bin {
	uint64_t upper; /* Upper bound on this bin. */
	uint64_t count; /* Count value for this bin. */
};

struct dm_histogram {
	/* The stats handle this histogram belongs to. */
	const struct dm_stats *dms;
	/* The region this histogram belongs to. */
	const struct dm_stats_region *region;
	uint64_t sum; /* Sum of histogram bin counts. */
	int nr_bins; /* Number of histogram bins assigned. */
	struct dm_histogram_bin bins[0];
};

/*
 * See Documentation/device-mapper/statistics.txt for full descriptions
 * of the device-mapper statistics counter fields.
 */
struct dm_stats_counters {
	uint64_t reads;		    /* Num reads completed */
	uint64_t reads_merged;	    /* Num reads merged */
	uint64_t read_sectors;	    /* Num sectors read */
	uint64_t read_nsecs;	    /* Num milliseconds spent reading */
	uint64_t writes;	    /* Num writes completed */
	uint64_t writes_merged;	    /* Num writes merged */
	uint64_t write_sectors;	    /* Num sectors written */
	uint64_t write_nsecs;	    /* Num milliseconds spent writing */
	uint64_t io_in_progress;    /* Num I/Os currently in progress */
	uint64_t io_nsecs;	    /* Num milliseconds spent doing I/Os */
	uint64_t weighted_io_nsecs; /* Weighted num milliseconds doing I/Os */
	uint64_t total_read_nsecs;  /* Total time spent reading in milliseconds */
	uint64_t total_write_nsecs; /* Total time spent writing in milliseconds */
	struct dm_histogram *histogram; /* Histogram. */
};

struct dm_stats_region {
	uint64_t region_id; /* as returned by @stats_list */
	uint64_t group_id;
	uint64_t start;
	uint64_t len;
	uint64_t step;
	char *program_id;
	char *aux_data;
	uint64_t timescale; /* precise_timestamps is per-region */
	struct dm_histogram *bounds; /* histogram configuration */
	struct dm_histogram *histogram; /* aggregate cache */
	struct dm_stats_counters *counters;
};

struct dm_stats_group {
	uint64_t group_id;
	const char *alias;
	dm_bitset_t regions;
	struct dm_histogram *histogram;
};

struct dm_stats {
	/* device binding */
	int bind_major;  /* device major that this dm_stats object is bound to */
	int bind_minor;  /* device minor that this dm_stats object is bound to */
	char *bind_name; /* device-mapper device name */
	char *bind_uuid; /* device-mapper UUID */
	char *program_id; /* default program_id for this handle */
	const char *name; /* cached device_name used for reporting */
	struct dm_pool *mem; /* memory pool for region and counter tables */
	struct dm_pool *hist_mem; /* separate pool for histogram tables */
	struct dm_pool *group_mem; /* separate pool for group tables */
	uint64_t nr_regions; /* total number of present regions */
	uint64_t max_region; /* size of the regions table */
	uint64_t interval_ns;  /* sampling interval in nanoseconds */
	uint64_t timescale; /* default sample value multiplier */
	int precise; /* use precise_timestamps when creating regions */
	struct dm_stats_region *regions;
	struct dm_stats_group *groups;
	/* statistics cursor */
	uint64_t walk_flags; /* walk control flags */
	uint64_t cur_flags;
	uint64_t cur_group;
	uint64_t cur_region;
	uint64_t cur_area;
};

#define PROC_SELF_COMM "/proc/self/comm"
static char *_program_id_from_proc(void)
{
	FILE *comm = NULL;
	char buf[STATS_ROW_BUF_LEN];

	if (!(comm = fopen(PROC_SELF_COMM, "r")))
		return_NULL;

	if (!fgets(buf, sizeof(buf), comm)) {
		log_error("Could not read from %s", PROC_SELF_COMM);
		if (fclose(comm))
			stack;
		return NULL;
	}

	if (fclose(comm))
		stack;

	return dm_strdup(buf);
}

static uint64_t _nr_areas(uint64_t len, uint64_t step)
{
	/* Default is one area. */
	if (!len || !step)
		return 1;
	/*
	 * drivers/md/dm-stats.c::message_stats_create()
	 * A region may be sub-divided into areas with their own counters.
	 * Any partial area at the end of the region is treated as an
	 * additional complete area.
	 */
	return (len + step - 1) / step;
}

static uint64_t _nr_areas_region(struct dm_stats_region *region)
{
	return _nr_areas(region->len, region->step);
}

struct dm_stats *dm_stats_create(const char *program_id)
{
	size_t hist_hint = sizeof(struct dm_histogram_bin);
	size_t group_hint = sizeof(struct dm_stats_group);
	struct dm_stats *dms = NULL;

	if (!(dms = dm_zalloc(sizeof(*dms))))
		return_NULL;

	/* FIXME: better hint. */
	if (!(dms->mem = dm_pool_create("stats_pool", 4096))) {
		dm_free(dms);
		return_NULL;
	}

	if (!(dms->hist_mem = dm_pool_create("histogram_pool", hist_hint)))
		goto_bad;

	if (!(dms->group_mem = dm_pool_create("group_pool", group_hint)))
		goto_bad;

	if (!program_id || !strlen(program_id))
		dms->program_id = _program_id_from_proc();
	else
		dms->program_id = dm_strdup(program_id);

	if (!dms->program_id) {
		log_error("Could not allocate memory for program_id");
		goto bad;
	}

	dms->bind_major = -1;
	dms->bind_minor = -1;
	dms->bind_name = NULL;
	dms->bind_uuid = NULL;

	dms->name = NULL;

	/* by default all regions use msec precision */
	dms->timescale = NSEC_PER_MSEC;
	dms->precise = 0;

	dms->nr_regions = DM_STATS_REGION_NOT_PRESENT;
	dms->max_region = DM_STATS_REGION_NOT_PRESENT;
	dms->regions = NULL;

	/* maintain compatibility with earlier walk version */
	dms->walk_flags = dms->cur_flags = DM_STATS_WALK_DEFAULT;

	return dms;

bad:
	dm_pool_destroy(dms->mem);
	if (dms->hist_mem)
		dm_pool_destroy(dms->hist_mem);
	if (dms->group_mem)
		dm_pool_destroy(dms->group_mem);
	dm_free(dms);
	return NULL;
}

/*
 * Test whether the stats region pointed to by region is present.
 */
static int _stats_region_present(const struct dm_stats_region *region)
{
	return !(region->region_id == DM_STATS_REGION_NOT_PRESENT);
}

/*
 * Test whether the stats group pointed to by group is present.
 */
static int _stats_group_present(const struct dm_stats_group *group)
{
	return !(group->group_id == DM_STATS_GROUP_NOT_PRESENT);
}

/*
 * Test whether a stats group id is present.
 */
static int _stats_group_id_present(const struct dm_stats *dms, uint64_t id)
{
	struct dm_stats_group *group = NULL;

	if (id == DM_STATS_GROUP_NOT_PRESENT)
		return 0;

	if (!dms)
		return_0;

	if (!dms->regions)
		return 0;

	if (id > dms->max_region)
		return 0;

	group = &dms->groups[id];

	return _stats_group_present(group);
}

/*
 * Test whether the given region_id is a member of any group.
 */
static uint64_t _stats_region_is_grouped(const struct dm_stats* dms,
					 uint64_t region_id)
{
	uint64_t group_id;

	if (region_id == DM_STATS_GROUP_NOT_PRESENT)
		return 0;

	if (!_stats_region_present(&dms->regions[region_id]))
		return 0;

	group_id = dms->regions[region_id].group_id;

	return group_id != DM_STATS_GROUP_NOT_PRESENT;
}

static void _stats_histograms_destroy(struct dm_pool *mem,
				      struct dm_stats_region *region)
{
	/* Unpopulated handle. */
	if (!region->counters)
		return;

	/*
	 * Free everything in the pool back to the first histogram.
	 */
	if (region->counters[0].histogram)
		dm_pool_free(mem, region->counters[0].histogram);
}

static void _stats_region_destroy(struct dm_stats_region *region)
{
	if (!_stats_region_present(region))
		return;

	region->start = region->len = region->step = 0;
	region->timescale = 0;

	/*
	 * Don't free counters and histogram bounds here: they are
	 * dropped from the pool along with the corresponding
	 * regions table.
	 *
	 * The following objects are all allocated with dm_malloc.
	 */

	region->counters = NULL;
	region->bounds = NULL;

	dm_free(region->program_id);
	region->program_id = NULL;
	dm_free(region->aux_data);
	region->aux_data = NULL;
	region->region_id = DM_STATS_REGION_NOT_PRESENT;
}

static void _stats_regions_destroy(struct dm_stats *dms)
{
	struct dm_pool *mem = dms->mem;
	uint64_t i;

	if (!dms->regions)
		return;

	/* walk backwards to obey pool order */
	for (i = dms->max_region; (i != DM_STATS_REGION_NOT_PRESENT); i--) {
		_stats_histograms_destroy(dms->hist_mem, &dms->regions[i]);
		_stats_region_destroy(&dms->regions[i]);
	}

	dm_pool_free(mem, dms->regions);
	dms->regions = NULL;
}

static void _stats_group_destroy(struct dm_stats_group *group)
{
	if (!_stats_group_present(group))
		return;

	group->histogram = NULL;

	if (group->alias) {
		dm_free((char *) group->alias);
		group->alias = NULL;
	}
	if (group->regions) {
		dm_bitset_destroy(group->regions);
		group->regions = NULL;
	}
	group->group_id = DM_STATS_GROUP_NOT_PRESENT;
}

static void _stats_groups_destroy(struct dm_stats *dms)
{
	uint64_t i;

	if (!dms->groups)
		return;

	for (i = dms->max_region; (i != DM_STATS_REGION_NOT_PRESENT); i--)
		_stats_group_destroy(&dms->groups[i]);
	dm_pool_free(dms->group_mem, dms->groups);
	dms->groups = NULL;
}

static int _set_stats_device(struct dm_stats *dms, struct dm_task *dmt)
{
	if (dms->bind_name)
		return dm_task_set_name(dmt, dms->bind_name);
	if (dms->bind_uuid)
		return dm_task_set_uuid(dmt, dms->bind_uuid);
	if (dms->bind_major > 0)
		return dm_task_set_major(dmt, dms->bind_major)
			&& dm_task_set_minor(dmt, dms->bind_minor);
	return_0;
}

static int _stats_bound(const struct dm_stats *dms)
{
	if (dms->bind_major > 0 || dms->bind_name || dms->bind_uuid)
		return 1;
	/* %p format specifier expects a void pointer. */
	log_error("Stats handle at %p is not bound.", dms);
	return 0;
}

static void _stats_clear_binding(struct dm_stats *dms)
{
	if (dms->bind_name)
		dm_pool_free(dms->mem, dms->bind_name);
	if (dms->bind_uuid)
		dm_pool_free(dms->mem, dms->bind_uuid);
	dm_free((char *) dms->name);

	dms->bind_name = dms->bind_uuid = NULL;
	dms->bind_major = dms->bind_minor = -1;
	dms->name = NULL;
}

int dm_stats_bind_devno(struct dm_stats *dms, int major, int minor)
{
	_stats_clear_binding(dms);
	_stats_regions_destroy(dms);
	_stats_groups_destroy(dms);

	dms->bind_major = major;
	dms->bind_minor = minor;

	return 1;
}

int dm_stats_bind_name(struct dm_stats *dms, const char *name)
{
	_stats_clear_binding(dms);
	_stats_regions_destroy(dms);
	_stats_groups_destroy(dms);

	if (!(dms->bind_name = dm_pool_strdup(dms->mem, name)))
		return_0;

	return 1;
}

int dm_stats_bind_uuid(struct dm_stats *dms, const char *uuid)
{
	_stats_clear_binding(dms);
	_stats_regions_destroy(dms);
	_stats_groups_destroy(dms);

	if (!(dms->bind_uuid = dm_pool_strdup(dms->mem, uuid)))
		return_0;

	return 1;
}

int dm_stats_bind_from_fd(struct dm_stats *dms, int fd)
{
        int major, minor;
        struct stat buf;

        if (fstat(fd, &buf)) {
                log_error("fstat failed for fd %d.", fd);
                return 0;
        }

        major = (int) MAJOR(buf.st_dev);
        minor = (int) MINOR(buf.st_dev);

        if (!dm_stats_bind_devno(dms, major, minor))
                return_0;
        return 1;
}

static int _stats_check_precise_timestamps(const struct dm_stats *dms)
{
	/* Already checked? */
	if (dms && dms->precise)
		return 1;

	return dm_message_supports_precise_timestamps();
}

int dm_stats_driver_supports_precise(void)
{
	return _stats_check_precise_timestamps(NULL);
}

int dm_stats_driver_supports_histogram(void)
{
	return _stats_check_precise_timestamps(NULL);
}

static int _fill_hist_arg(char *hist_arg, size_t hist_len, uint64_t scale,
			  struct dm_histogram *bounds)
{
	int i, l, len = 0, nr_bins;
	char *arg = hist_arg;
	uint64_t value;

	nr_bins = bounds->nr_bins;

	for (i = 0; i < nr_bins; i++) {
		value = bounds->bins[i].upper / scale;
		if ((l = dm_snprintf(arg, hist_len - len, FMTu64"%s", value,
				     (i == (nr_bins - 1)) ? "" : ",")) < 0)
			return_0;
		len += l;
		arg += l;
	}
	return 1;
}

static void *_get_hist_arg(struct dm_histogram *bounds, uint64_t scale,
			   size_t *len)
{
	struct dm_histogram_bin *entry, *bins;
	size_t hist_len = 1; /* terminating '\0' */
	double value;

	entry = bins = bounds->bins;

	entry += bounds->nr_bins - 1;
	while(entry >= bins) {
		value = (double) (entry--)->upper;
		/* Use lround to avoid size_t -> double cast warning. */
		hist_len += 1 + (size_t) lround(log10(value / scale));
		if (entry != bins)
			hist_len++; /* ',' */
	}

	*len = hist_len;

	return dm_zalloc(hist_len);
}

static char *_build_histogram_arg(struct dm_histogram *bounds, int *precise)
{
	struct dm_histogram_bin *entry, *bins;
	size_t hist_len;
	char *hist_arg;
	uint64_t scale;

	entry = bins = bounds->bins;

	/* Empty histogram is invalid. */
	if (!bounds->nr_bins) {
		log_error("Cannot format empty histogram description.");
		return NULL;
	}

	/* Validate entries and set *precise if precision < 1ms. */
	entry += bounds->nr_bins - 1;
	while (entry >= bins) {
		if (entry != bins) {
			if (entry->upper < (entry - 1)->upper) {
				log_error("Histogram boundaries must be in "
					  "order of increasing magnitude.");
				return 0;
			}
		}

		/*
		 * Only enable precise_timestamps automatically if any
		 * value in the histogram bounds uses precision < 1ms.
		 */
		if (((entry--)->upper % NSEC_PER_MSEC) && !*precise)
			*precise = 1;
	}

	scale = (*precise) ? 1 : NSEC_PER_MSEC;

	/* Calculate hist_len and allocate a character buffer. */
	if (!(hist_arg = _get_hist_arg(bounds, scale, &hist_len))) {
		log_error("Could not allocate memory for histogram argument.");
		return 0;
	}

	/* Fill hist_arg with boundary strings. */
	if (!_fill_hist_arg(hist_arg, hist_len, scale, bounds))
		goto_bad;

	return hist_arg;

bad:
	log_error("Could not build histogram arguments.");
	dm_free(hist_arg);

	return NULL;
}

static struct dm_task *_stats_send_message(struct dm_stats *dms, char *msg)
{
	struct dm_task *dmt;

	if (!(dmt = dm_task_create(DM_DEVICE_TARGET_MSG)))
		return_0;

	if (!_set_stats_device(dms, dmt))
		goto_bad;

	if (!dm_task_set_message(dmt, msg))
		goto_bad;

	if (!dm_task_run(dmt))
		goto_bad;

	return dmt;

bad:
	dm_task_destroy(dmt);
	return NULL;
}

/*
 * Cache the dm device_name for the device bound to dms.
 */
static int _stats_set_name_cache(struct dm_stats *dms)
{
	struct dm_task *dmt;

	if (dms->name)
		return 1;

	if (!(dmt = dm_task_create(DM_DEVICE_INFO)))
		return_0;

	if (!_set_stats_device(dms, dmt))
		goto_bad;

	if (!dm_task_run(dmt))
		goto_bad;

	if (!(dms->name = dm_strdup(dm_task_get_name(dmt))))
		goto_bad;

	dm_task_destroy(dmt);

	return 1;

bad:
	log_error("Could not retrieve device-mapper name for device.");
	dm_task_destroy(dmt);
	return 0;
}

/*
 * update region group_id values
 */
static void _stats_update_groups(struct dm_stats *dms)
{
	struct dm_stats_group *group;
	uint64_t group_id, i;

	for (group_id = 0; group_id < dms->max_region + 1; group_id++) {
		if (!_stats_group_id_present(dms, group_id))
			continue;

		group = &dms->groups[group_id];

		for (i = dm_bit_get_first(group->regions);
		     i != DM_STATS_GROUP_NOT_PRESENT;
		     i = dm_bit_get_next(group->regions, i))
			dms->regions[i].group_id = group_id;
	}
}

static void _check_group_regions_present(struct dm_stats *dms,
					 struct dm_stats_group *group)
{
	dm_bitset_t regions = group->regions;
	int64_t i, group_id;

	group_id = i = dm_bit_get_first(regions);

	for (; i > 0; i = dm_bit_get_next(regions, i))
		if (!_stats_region_present(&dms->regions[i])) {
			log_warn("Group descriptor " FMTd64 " contains "
				 "non-existent region_id " FMTd64 ".",
				 group_id, i);
			dm_bit_clear(regions, i);
		}
}

/*
 * Parse a DMS_GROUP group descriptor embedded in a region's aux_data.
 *
 * DMS_GROUP="ALIAS:MEMBERS"
 *
 * ALIAS: group alias
 * MEMBERS: list of group member region ids.
 *
 */
#define DMS_GROUP_TAG "DMS_GROUP="
#define DMS_GROUP_TAG_LEN (sizeof(DMS_GROUP_TAG) - 1)
#define DMS_GROUP_SEP ':'
#define DMS_AUX_SEP "#"

static int _parse_aux_data_group(struct dm_stats *dms,
				 struct dm_stats_region *region,
				 struct dm_stats_group *group)
{
	char *alias, *c, *end;
	dm_bitset_t regions;

	memset(group, 0, sizeof(*group));
	group->group_id = DM_STATS_GROUP_NOT_PRESENT;

	/* find start of group tag */
	c = strstr(region->aux_data, DMS_GROUP_TAG);
	if (!c)
		return 1; /* no group is not an error */

	alias = c + strlen(DMS_GROUP_TAG);

	c = strchr(c, DMS_GROUP_SEP);

	if (!c) {
		log_error("Found malformed group tag while reading aux_data");
		return 0;
	}

	/* terminate alias and advance to members */
	*(c++) = '\0';

	log_debug("Read alias '%s' from aux_data", alias);

	if (!c) {
		log_error("Found malformed group descriptor while "
			  "reading aux_data, expected '%c'", DMS_GROUP_SEP);
		return 0;
	}

	/* if user aux_data follows make sure we have a terminated
	 * string to pass to dm_bitset_parse_list().
	 */
	end = strstr(c, DMS_AUX_SEP);
	if (!end)
		end = c + strlen(c);
	*(end++) = '\0';

	if (!(regions = dm_bitset_parse_list(c, NULL, 0))) {
		log_error("Could not parse member list while "
			  "reading group aux_data");
		return 0;
	}

	group->group_id = dm_bit_get_first(regions);
	if (group->group_id != region->region_id) {
		log_error("Found invalid group descriptor in region " FMTu64
			  " aux_data.", region->region_id);
		group->group_id = DM_STATS_GROUP_NOT_PRESENT;
		goto bad;
	}

	group->regions = regions;
	group->alias = NULL;
	if (strlen(alias)) {
		group->alias = dm_strdup(alias);
		if (!group->alias) {
			log_error("Could not allocate memory for group alias");
			goto bad;
		}
	}

	/* separate group tag from user aux_data */
	if ((strlen(end) > 1) || strncmp(end, "-", 1))
		c = dm_strdup(end);
	else
		c = dm_strdup("");

	if (!c) {
		log_error("Could not allocate memory for user aux_data");
		goto bad_alias;
	}

	dm_free(region->aux_data);
	region->aux_data = c;

	log_debug("Found group_id " FMTu64 ": alias=\"%s\"", group->group_id,
		  (group->alias) ? group->alias : "");

	return 1;

bad_alias:
	dm_free((char *) group->alias);
bad:
	dm_bitset_destroy(regions);
	return 0;
}

/*
 * Parse a histogram specification returned by the kernel in a
 * @stats_list response.
 */
static int _stats_parse_histogram_spec(struct dm_stats *dms,
				       struct dm_stats_region *region,
				       const char *histogram)
{
	static const char _valid_chars[] = "0123456789,";
	uint64_t scale = region->timescale, this_val = 0;
	struct dm_pool *mem = dms->hist_mem;
	struct dm_histogram_bin cur;
	struct dm_histogram hist;
	int nr_bins = 1;
	const char *c, *v, *val_start;
	char *p, *endptr = NULL;

	/* Advance past "histogram:". */
	histogram = strchr(histogram, ':');
	if (!histogram) {
		log_error("Could not parse histogram description.");
		return 0;
	}
	histogram++;

	/* @stats_list rows are newline terminated. */
	if ((p = strchr(histogram, '\n')))
		*p = '\0';

	if (!dm_pool_begin_object(mem, sizeof(cur)))
		return_0;

	memset(&hist, 0, sizeof(hist));

	hist.nr_bins = 0; /* fix later */
	hist.region = region;
	hist.dms = dms;

	if (!dm_pool_grow_object(mem, &hist, sizeof(hist)))
		goto_bad;

	c = histogram;
	do {
		for (v = _valid_chars; *v; v++)
			if (*c == *v)
				break;
		if (!*v) {
			stack;
			goto badchar;
		}

		if (*c == ',') {
			log_error("Invalid histogram description: %s",
				  histogram);
			goto bad;
		} else {
			val_start = c;
			endptr = NULL;

			errno = 0;
			this_val = strtoull(val_start, &endptr, 10);
			if (errno || !endptr) {
				log_error("Could not parse histogram boundary.");
				goto bad;
			}

			c = endptr; /* Advance to units, comma, or end. */

			if (*c == ',')
				c++;
			else if (*c || (*c == ' ')) { /* Expected ',' or NULL. */
				stack;
				goto badchar;
			}

			if (*c == ',')
				c++;

			cur.upper = scale * this_val;
			cur.count = 0;

			if (!dm_pool_grow_object(mem, &cur, sizeof(cur)))
				goto_bad;

			nr_bins++;
		}
	} while (*c && (*c != ' '));

	/* final upper bound. */
	cur.upper = UINT64_MAX;
	if (!dm_pool_grow_object(mem, &cur, sizeof(cur)))
		goto_bad;

	region->bounds = dm_pool_end_object(mem);

	if (!region->bounds)
		return_0;

	region->bounds->nr_bins = nr_bins;

	log_debug("Added region histogram spec with %d entries.", nr_bins);
	return 1;

badchar:
	log_error("Invalid character in histogram: '%c' (0x%x)", *c, *c);
bad:
	dm_pool_abandon_object(mem);
	return 0;
}

static int _stats_parse_list_region(struct dm_stats *dms,
				    struct dm_stats_region *region, char *line)
{
	char *p = NULL, string_data[STATS_ROW_BUF_LEN];
	char *program_id, *aux_data, *stats_args;
	char *empty_string = (char *) "";
	int r;

	memset(string_data, 0, sizeof(string_data));

	/*
	 * Parse fixed fields, line format:
	 *
	 * <region_id>: <start_sector>+<length> <step> <string data>
	 *
	 * Maximum string data size is 4096 - 1 bytes.
	 */
	r = sscanf(line, FMTu64 ": " FMTu64 "+" FMTu64 " " FMTu64 " %4095c",
		   &region->region_id, &region->start, &region->len,
		   &region->step, string_data);

	if (r != 5)
		return_0;

	/* program_id is guaranteed to be first. */
	program_id = string_data;

	/*
	 * FIXME: support embedded '\ ' in string data:
	 *   s/strchr/_find_unescaped_space()/
	 */
	if ((p = strchr(string_data, ' '))) {
		/* terminate program_id string. */
		*p = '\0';
		if (!strncmp(program_id, "-", 1))
			program_id = empty_string;
		aux_data = p + 1;
		if ((p = strchr(aux_data, ' '))) {
			/* terminate aux_data string. */
			*p = '\0';
			stats_args = p + 1;
		} else
			stats_args = empty_string;

		/* no aux_data? */
		if (!strncmp(aux_data, "-", 1))
			aux_data = empty_string;
		else
			/* remove trailing newline */
			aux_data[strlen(aux_data) - 1] = '\0';
	} else
		aux_data = stats_args = empty_string;

	if (strstr(stats_args, PRECISE_ARG))
		region->timescale = 1;
	else
		region->timescale = NSEC_PER_MSEC;

	if ((p = strstr(stats_args, HISTOGRAM_ARG))) {
		if (!_stats_parse_histogram_spec(dms, region, p))
			return_0;
	} else
		region->bounds = NULL;

	/* clear aggregate cache */
	region->histogram = NULL;

	region->group_id = DM_STATS_GROUP_NOT_PRESENT;

	if (!(region->program_id = dm_strdup(program_id)))
		return_0;
	if (!(region->aux_data = dm_strdup(aux_data))) {
		dm_free(region->program_id);
		return_0;
	}

	region->counters = NULL;
	return 1;
}

static int _stats_parse_list(struct dm_stats *dms, const char *resp)
{
	uint64_t max_region = 0, nr_regions = 0;
	struct dm_stats_region cur, fill;
	struct dm_stats_group cur_group;
	struct dm_pool *mem = dms->mem, *group_mem = dms->group_mem;
	char line[STATS_ROW_BUF_LEN];
	FILE *list_rows;

	if (!resp) {
		log_error("Could not parse NULL @stats_list response.");
		return 0;
	}

	_stats_regions_destroy(dms);
	_stats_groups_destroy(dms);

	/* no regions */
	if (!strlen(resp)) {
		dms->nr_regions = dms->max_region = 0;
		dms->regions = NULL;
		return 1;
	}

	/*
	 * dm_task_get_message_response() returns a 'const char *' but
	 * since fmemopen also permits "w" it expects a 'char *'.
	 */
	/* coverity[alloc_strlen] intentional */
	if (!(list_rows = fmemopen((char *)resp, strlen(resp), "r")))
		return_0;

	/* begin region table */
	if (!dm_pool_begin_object(mem, 1024))
		goto_bad;

	/* begin group table */
	if (!dm_pool_begin_object(group_mem, 32))
		goto_bad;

	while(fgets(line, sizeof(line), list_rows)) {

		cur_group.group_id = DM_STATS_GROUP_NOT_PRESENT;
		cur_group.regions = NULL;
		cur_group.alias = NULL;

		if (!_stats_parse_list_region(dms, &cur, line))
			goto_bad;

		/* handle holes in the list of region_ids */
		if (cur.region_id > max_region) {
			memset(&fill, 0, sizeof(fill));
			memset(&cur_group, 0, sizeof(cur_group));
			fill.region_id = DM_STATS_REGION_NOT_PRESENT;
			cur_group.group_id = DM_STATS_GROUP_NOT_PRESENT;
			do {
				if (!dm_pool_grow_object(mem, &fill, sizeof(fill)))
					goto_bad;
				if (!dm_pool_grow_object(group_mem, &cur_group,
							 sizeof(cur_group)))
					goto_bad;
			} while (max_region++ < (cur.region_id - 1));
		}

		if (cur.aux_data)
			if (!_parse_aux_data_group(dms, &cur, &cur_group))
				log_error("Failed to parse group descriptor "
					  "from region_id " FMTu64 " aux_data:"
					  "'%s'", cur.region_id, cur.aux_data);
				/* continue */

		if (!dm_pool_grow_object(mem, &cur, sizeof(cur)))
			goto_bad;

		if (!dm_pool_grow_object(group_mem, &cur_group,
					 sizeof(cur_group)))
			goto_bad;

		max_region++;
		nr_regions++;
	}

	if (!nr_regions)
		/* no region data read from @stats_list */
		goto bad;

	dms->nr_regions = nr_regions;
	dms->max_region = max_region - 1;
	dms->regions = dm_pool_end_object(mem);
	dms->groups = dm_pool_end_object(group_mem);

	dm_stats_foreach_group(dms)
		_check_group_regions_present(dms, &dms->groups[dms->cur_group]);

	_stats_update_groups(dms);

	if (fclose(list_rows))
		stack;

	return 1;

bad:
	if (fclose(list_rows))
		stack;
	dm_pool_abandon_object(mem);
	dm_pool_abandon_object(group_mem);

	return 0;
}

int dm_stats_list(struct dm_stats *dms, const char *program_id)
{
	char msg[STATS_MSG_BUF_LEN];
	struct dm_task *dmt;
	int r;

	if (!_stats_bound(dms))
		return_0;

	/* allow zero-length program_id for list */
	if (!program_id)
		program_id = dms->program_id;

	if (!_stats_set_name_cache(dms))
		return_0;

	if (dms->regions)
		_stats_regions_destroy(dms);

	r = dm_snprintf(msg, sizeof(msg), "@stats_list %s", program_id);

	if (r < 0) {
		log_error("Failed to prepare stats message.");
		return 0;
	}

	if (!(dmt = _stats_send_message(dms, msg)))
		return_0;

	if (!_stats_parse_list(dms, dm_task_get_message_response(dmt))) {
		log_error("Could not parse @stats_list response.");
		goto bad;
	}

	dm_task_destroy(dmt);
	return 1;

bad:
	dm_task_destroy(dmt);
	return 0;
}

/*
 * Parse histogram data returned from a @stats_print operation.
 */
static int _stats_parse_histogram(struct dm_pool *mem, char *hist_str,
				  struct dm_histogram **histogram,
				  struct dm_stats_region *region)
{
	static const char _valid_chars[] = "0123456789:";
	struct dm_histogram *bounds = region->bounds;
	struct dm_histogram hist = {
		.nr_bins = region->bounds->nr_bins
	};
	const char *c, *v, *val_start;
	struct dm_histogram_bin cur;
	uint64_t sum = 0, this_val;
	char *endptr = NULL;
	int bin = 0;

	c = hist_str;

	if (!dm_pool_begin_object(mem, sizeof(cur)))
		return_0;

	if (!dm_pool_grow_object(mem, &hist, sizeof(hist)))
		goto_bad;

	do {
		memset(&cur, 0, sizeof(cur));
		for (v = _valid_chars; *v; v++)
			if (*c == *v)
				break;
		if (!*v)
			goto badchar;

		if (*c == ',')
			goto badchar;
		else {
			val_start = c;
			endptr = NULL;

			errno = 0;
			this_val = strtoull(val_start, &endptr, 10);
			if (errno || !endptr) {
				log_error("Could not parse histogram value.");
				goto bad;
			}
			c = endptr; /* Advance to colon, or end. */

			if (*c == ':')
				c++;
			else if (*c & (*c != '\n'))
				/* Expected ':', '\n', or NULL. */
				goto badchar;

			if (*c == ':')
				c++;

			cur.upper = bounds->bins[bin].upper;
			cur.count = this_val;
			sum += this_val;

			if (!dm_pool_grow_object(mem, &cur, sizeof(cur)))
				goto_bad;

			bin++;
		}
	} while (*c && (*c != '\n'));

	log_debug("Added region histogram data with %d entries.", hist.nr_bins);

	*histogram = dm_pool_end_object(mem);
	(*histogram)->sum = sum;

	return 1;

badchar:
	log_error("Invalid character in histogram data: '%c' (0x%x)", *c, *c);
bad:
	dm_pool_abandon_object(mem);
	return 0;
}

static int _stats_parse_region(struct dm_stats *dms, const char *resp,
			       struct dm_stats_region *region,
			       uint64_t timescale)
{
	struct dm_histogram *hist = NULL;
	struct dm_pool *mem = dms->mem;
	struct dm_stats_counters cur;
	FILE *stats_rows = NULL;
	uint64_t start = 0, len = 0;
	char row[STATS_ROW_BUF_LEN];
	int r;

	if (!resp) {
		log_error("Could not parse empty @stats_print response.");
		return 0;
	}

	region->start = UINT64_MAX;

	if (!dm_pool_begin_object(mem, 512))
		goto_bad;

	/*
	 * dm_task_get_message_response() returns a 'const char *' but
	 * since fmemopen also permits "w" it expects a 'char *'.
	 */
	/* coverity[alloc_strlen] intentional */
	stats_rows = fmemopen((char *)resp, strlen(resp), "r");
	if (!stats_rows)
		goto_bad;

	/*
	 * Output format for each step-sized area of a region:
	 *
	 * <start_sector>+<length> counters
	 *
	 * The first 11 counters have the same meaning as
	 * /sys/block/ * /stat or /proc/diskstats.
	 *
	 * Please refer to Documentation/iostats.txt for details.
	 *
	 * 1. the number of reads completed
	 * 2. the number of reads merged
	 * 3. the number of sectors read
	 * 4. the number of milliseconds spent reading
	 * 5. the number of writes completed
	 * 6. the number of writes merged
	 * 7. the number of sectors written
	 * 8. the number of milliseconds spent writing
	 * 9. the number of I/Os currently in progress
	 * 10. the number of milliseconds spent doing I/Os
	 * 11. the weighted number of milliseconds spent doing I/Os
	 *
	 * Additional counters:
	 * 12. the total time spent reading in milliseconds
	 * 13. the total time spent writing in milliseconds
	 *
	*/
	while (fgets(row, sizeof(row), stats_rows)) {
		r = sscanf(row, FMTu64 "+" FMTu64 /* start+len */
			   /* reads */
			   FMTu64 " " FMTu64 " " FMTu64 " " FMTu64 " "
			   /* writes */
			   FMTu64 " " FMTu64 " " FMTu64 " " FMTu64 " "
			   /* in flight & io nsecs */
			   FMTu64 " " FMTu64 " " FMTu64 " "
			   /* tot read/write nsecs */
			   FMTu64 " " FMTu64, &start, &len,
			   &cur.reads, &cur.reads_merged, &cur.read_sectors,
			   &cur.read_nsecs,
			   &cur.writes, &cur.writes_merged, &cur.write_sectors,
			   &cur.write_nsecs,
			   &cur.io_in_progress,
			   &cur.io_nsecs, &cur.weighted_io_nsecs,
			   &cur.total_read_nsecs, &cur.total_write_nsecs);
		if (r != 15) {
			log_error("Could not parse @stats_print row.");
			goto bad;
		}

		/* scale time values up if needed */
		if (timescale != 1) {
			cur.read_nsecs *= timescale;
			cur.write_nsecs *= timescale;
			cur.io_nsecs *= timescale;
			cur.weighted_io_nsecs *= timescale;
			cur.total_read_nsecs *= timescale;
			cur.total_write_nsecs *= timescale;
		}

		if (region->bounds) {
			/* Find first histogram separator. */
			char *hist_str = strchr(row, ':');
			if (!hist_str) {
				log_error("Could not parse histogram value.");
				goto bad;
			}
			/* Find space preceding histogram. */
			while (hist_str && *(hist_str - 1) != ' ')
				hist_str--;

			/* Use a separate pool for histogram objects since we
			 * are growing the area table and each area's histogram
			 * table simultaneously.
			 */
			if (!_stats_parse_histogram(dms->hist_mem, hist_str,
						    &hist, region))
				goto_bad;
			hist->dms = dms;
			hist->region = region;
		}

		cur.histogram = hist;

		if (!dm_pool_grow_object(mem, &cur, sizeof(cur)))
			goto_bad;

		if (region->start == UINT64_MAX) {
			region->start = start;
			region->step = len; /* area size is always uniform. */
		}
	}

	if (region->start == UINT64_MAX)
		/* no area data read from @stats_print */
		goto bad;

	region->len = (start + len) - region->start;
	region->timescale = timescale;
	region->counters = dm_pool_end_object(mem);

	if (fclose(stats_rows))
		stack;

	return 1;

bad:
	if (stats_rows)
		if (fclose(stats_rows))
			stack;
	dm_pool_abandon_object(mem);

	return 0;
}

static void _stats_walk_next_present(const struct dm_stats *dms,
				     uint64_t *flags,
				     uint64_t *cur_r, uint64_t *cur_a,
				     uint64_t *cur_g)
{
	struct dm_stats_region *cur = NULL;

	/* start of walk: region loop advances *cur_r to 0. */
	if (*cur_r != DM_STATS_REGION_NOT_PRESENT)
		cur = &dms->regions[*cur_r];

	/* within current region? */
	if (cur && (*flags & DM_STATS_WALK_AREA)) {
		if (++(*cur_a) < _nr_areas_region(cur))
			return;
		else
			*cur_a = 0;
	}

	/* advance to next present, non-skipped region or end */
	while (++(*cur_r) <= dms->max_region) {
		cur = &dms->regions[*cur_r];
		if (!_stats_region_present(cur))
			continue;
		if ((*flags & DM_STATS_WALK_SKIP_SINGLE_AREA))
			if (!(*flags & DM_STATS_WALK_AREA))
				if (_nr_areas_region(cur) < 2)
					continue;
		/* matching region found */
		break;
	}
	return;
}

static void _stats_walk_next(const struct dm_stats *dms, uint64_t *flags,
			     uint64_t *cur_r, uint64_t *cur_a, uint64_t *cur_g)
{
	if (!dms || !dms->regions)
		return;

	if (*flags & DM_STATS_WALK_AREA) {
		/* advance to next area, region, or end */
		_stats_walk_next_present(dms, flags, cur_r, cur_a, cur_g);
		return;
	}

	if (*flags & DM_STATS_WALK_REGION) {
		/* enable region aggregation */
		*cur_a = DM_STATS_WALK_REGION;
		_stats_walk_next_present(dms, flags, cur_r, cur_a, cur_g);
		return;
	}

	if (*flags & DM_STATS_WALK_GROUP) {
		/* enable group aggregation */
		*cur_r = *cur_a = DM_STATS_WALK_GROUP;
		while (!_stats_group_id_present(dms, ++(*cur_g))
		       && (*cur_g) < dms->max_region + 1)
			; /* advance to next present group or end */
		return;
	}

	log_error("stats_walk_next called with empty walk flags");
}

static void _group_walk_start(const struct dm_stats *dms, uint64_t *flags,
			      uint64_t *cur_r, uint64_t *cur_a, uint64_t *cur_g)
{
	if (!(*flags & DM_STATS_WALK_GROUP))
		return;

	*cur_a = *cur_r = DM_STATS_WALK_GROUP;
	*cur_g = 0;

	/* advance to next present group or end */
	while ((*cur_g) <= dms->max_region) {
	        if (_stats_region_is_grouped(dms, *cur_g))
			break;
		(*cur_g)++;
	}

	if (*cur_g > dms->max_region)
		/* no groups to walk */
		*flags &= ~DM_STATS_WALK_GROUP;
}

static void _stats_walk_start(const struct dm_stats *dms, uint64_t *flags,
			      uint64_t *cur_r, uint64_t *cur_a,
			      uint64_t *cur_g)
{
	log_debug("starting stats walk with %s %s %s %s",
		  (*flags & DM_STATS_WALK_AREA) ? "AREA" : "",
		  (*flags & DM_STATS_WALK_REGION) ? "REGION" : "",
		  (*flags & DM_STATS_WALK_GROUP) ? "GROUP" : "",
		  (*flags & DM_STATS_WALK_SKIP_SINGLE_AREA) ? "SKIP" : "");

	if (!dms->regions)
		return;

	if (!(*flags & (DM_STATS_WALK_AREA | DM_STATS_WALK_REGION)))
		return _group_walk_start(dms, flags, cur_r, cur_a, cur_g);

	/* initialise cursor state */
	*cur_a = 0;
	*cur_r = DM_STATS_REGION_NOT_PRESENT;
	*cur_g = DM_STATS_GROUP_NOT_PRESENT;

	if (!(*flags & DM_STATS_WALK_AREA))
		*cur_a = DM_STATS_WALK_REGION;

	/* advance to first present, non-skipped region */
	_stats_walk_next_present(dms, flags, cur_r, cur_a, cur_g);
}

#define DM_STATS_WALK_MASK (DM_STATS_WALK_AREA			\
			    | DM_STATS_WALK_REGION		\
			    | DM_STATS_WALK_GROUP		\
			    | DM_STATS_WALK_SKIP_SINGLE_AREA)

int dm_stats_walk_init(struct dm_stats *dms, uint64_t flags)
{
	if (!dms)
		return_0;

	if (flags & ~DM_STATS_WALK_MASK) {
		log_error("Unknown value in walk flags: 0x" FMTx64,
			  (uint64_t) (flags & ~DM_STATS_WALK_MASK));
		return 0;
	}
	dms->walk_flags = flags;
	log_debug("dm_stats_walk_init: initialised flags to " FMTx64, flags);
	return 1;
}

void dm_stats_walk_start(struct dm_stats *dms)
{
	if (!dms || !dms->regions)
		return;

	dms->cur_flags = dms->walk_flags;

	_stats_walk_start(dms, &dms->cur_flags,
			  &dms->cur_region, &dms->cur_area,
			  &dms->cur_group);
}

void dm_stats_walk_next(struct dm_stats *dms)
{
	_stats_walk_next(dms, &dms->cur_flags,
			 &dms->cur_region, &dms->cur_area,
			 &dms->cur_group);
}

void dm_stats_walk_next_region(struct dm_stats *dms)
{
	dms->cur_flags &= ~DM_STATS_WALK_AREA;
	_stats_walk_next(dms, &dms->cur_flags,
			 &dms->cur_region, &dms->cur_area,
			 &dms->cur_group);
}

/*
 * Return 1 if any regions remain that are present and not skipped
 * by the current walk flags or 0 otherwise.
 */
static uint64_t _stats_walk_any_unskipped(const struct dm_stats *dms,
					  uint64_t *flags,
					  uint64_t *cur_r, uint64_t *cur_a)
{
	struct dm_stats_region *region;
	uint64_t i;

	if (*cur_r > dms->max_region)
		return 0;

	for (i = *cur_r; i <= dms->max_region; i++) {
		region = &dms->regions[i];
		if (!_stats_region_present(region))
			continue;
		if ((*flags & DM_STATS_WALK_SKIP_SINGLE_AREA)
		    && !(*flags & DM_STATS_WALK_AREA))
			if (_nr_areas_region(region) < 2)
				continue;
		return 1;
	}
	return 0;
}

static void _stats_walk_end_areas(const struct dm_stats *dms, uint64_t *flags,
				  uint64_t *cur_r, uint64_t *cur_a,
				  uint64_t *cur_g)
{
	int end = !_stats_walk_any_unskipped(dms, flags, cur_r, cur_a);

	if (!(*flags & DM_STATS_WALK_AREA))
		return;

	if (!end)
		return;

	*flags &= ~DM_STATS_WALK_AREA;
	if (*flags & DM_STATS_WALK_REGION) {
		/* start region walk */
		*cur_a = DM_STATS_WALK_REGION;
		*cur_r = DM_STATS_REGION_NOT_PRESENT;
		_stats_walk_next_present(dms, flags, cur_r, cur_a, cur_g);
		if (!_stats_walk_any_unskipped(dms, flags, cur_r, cur_a)) {
			/* no more regions */
			*flags &= ~DM_STATS_WALK_REGION;
			if (!(*flags & DM_STATS_WALK_GROUP))
				*cur_r = dms->max_region;
		}
	}

	if (*flags & DM_STATS_WALK_REGION)
		return;

	if (*flags & DM_STATS_WALK_GROUP)
		_group_walk_start(dms, flags, cur_r, cur_a, cur_g);
}

static int _stats_walk_end(const struct dm_stats *dms, uint64_t *flags,
			   uint64_t *cur_r, uint64_t *cur_a, uint64_t *cur_g)
{
	if (*flags & DM_STATS_WALK_AREA) {
		_stats_walk_end_areas(dms, flags, cur_r, cur_a, cur_g);
		goto out;
	}

	if (*flags & DM_STATS_WALK_REGION) {
		if (!_stats_walk_any_unskipped(dms, flags, cur_r, cur_a)) {
			*flags &= ~DM_STATS_WALK_REGION;
			_group_walk_start(dms, flags, cur_r, cur_a, cur_g);
		}
		goto out;
	}

	if (*flags & DM_STATS_WALK_GROUP) {
		if (*cur_g <= dms->max_region)
			goto out;
		*flags &= ~DM_STATS_WALK_GROUP;
	}
out:
	return !(*flags & ~DM_STATS_WALK_SKIP_SINGLE_AREA);
}

int dm_stats_walk_end(struct dm_stats *dms)
{
	if (!dms)
		return 1;

	if (_stats_walk_end(dms, &dms->cur_flags,
			    &dms->cur_region, &dms->cur_area,
			    &dms->cur_group)) {
		dms->cur_flags = dms->walk_flags;
		return 1;
	}
	return 0;
}

dm_stats_obj_type_t dm_stats_object_type(const struct dm_stats *dms,
					 uint64_t region_id,
					 uint64_t area_id)
{
	uint64_t group_id;

	region_id = (region_id == DM_STATS_REGION_CURRENT)
		     ? dms->cur_region : region_id ;
	area_id = (area_id == DM_STATS_AREA_CURRENT)
		   ? dms->cur_area : area_id ;

	if (region_id == DM_STATS_REGION_NOT_PRESENT)
		/* no region */
		return DM_STATS_OBJECT_TYPE_NONE;

	if (region_id & DM_STATS_WALK_GROUP) {
		if (region_id == DM_STATS_WALK_GROUP)
			/* indirect group_id from cursor */
			group_id = dms->cur_group;
		else
			/* immediate group_id encoded in region_id */
			group_id = region_id & ~DM_STATS_WALK_GROUP;
		if (!_stats_group_id_present(dms, group_id))
			return DM_STATS_OBJECT_TYPE_NONE;
		return DM_STATS_OBJECT_TYPE_GROUP;
	}

	if (region_id > dms->max_region)
		/* end of table */
		return DM_STATS_OBJECT_TYPE_NONE;

	if (area_id & DM_STATS_WALK_REGION)
		/* aggregate region */
		return DM_STATS_OBJECT_TYPE_REGION;

	/* plain region_id and area_id */
	return DM_STATS_OBJECT_TYPE_AREA;
}

dm_stats_obj_type_t dm_stats_current_object_type(const struct dm_stats *dms)
{
	/* dm_stats_object_type will decode region/area */
	return dm_stats_object_type(dms,
				    DM_STATS_REGION_CURRENT,
				    DM_STATS_AREA_CURRENT);
}

uint64_t dm_stats_get_region_nr_areas(const struct dm_stats *dms,
				      uint64_t region_id)
{
	struct dm_stats_region *region = NULL;

	/* groups or aggregate regions cannot be subdivided */
	if (region_id & DM_STATS_WALK_GROUP)
		return 1;

	region = &dms->regions[region_id];
	return _nr_areas_region(region);
}

uint64_t dm_stats_get_current_nr_areas(const struct dm_stats *dms)
{
	/* groups or aggregate regions cannot be subdivided */
	if (dms->cur_region & DM_STATS_WALK_GROUP)
		return 1;

	return dm_stats_get_region_nr_areas(dms, dms->cur_region);
}

uint64_t dm_stats_get_nr_areas(const struct dm_stats *dms)
{
	uint64_t nr_areas = 0, flags = DM_STATS_WALK_AREA;
	/* use a separate cursor */
	uint64_t cur_region = 0, cur_area = 0, cur_group = 0;

	/* no regions to visit? */
	if (!dms->regions)
		return 0;

	flags = DM_STATS_WALK_AREA;
	_stats_walk_start(dms, &flags, &cur_region, &cur_area, &cur_group);
	do {
		nr_areas += dm_stats_get_current_nr_areas(dms);
		_stats_walk_next(dms, &flags,
				 &cur_region, &cur_area,
				 &cur_group);
	} while (!_stats_walk_end(dms, &flags,
				  &cur_region, &cur_area,
				  &cur_group));
	return nr_areas;
}

int dm_stats_group_present(const struct dm_stats *dms, uint64_t group_id)
{
	return _stats_group_id_present(dms, group_id);
}

int dm_stats_get_region_nr_histogram_bins(const struct dm_stats *dms,
					  uint64_t region_id)
{
	region_id = (region_id == DM_STATS_REGION_CURRENT)
		     ? dms->cur_region : region_id ;

	/* FIXME: support group histograms if all region bounds match */
	if (region_id & DM_STATS_WALK_GROUP)
		return 0;

	if (!dms->regions[region_id].bounds)
		return 0;

	return dms->regions[region_id].bounds->nr_bins;
}

/*
 * Fill buf with a list of set regions in the regions bitmap. Consecutive
 * ranges of set region IDs are output using "M-N" range notation.
 *
 * The number of bytes consumed is returned or zero on error.
 */
static size_t _stats_group_tag_fill(const struct dm_stats *dms,
				    dm_bitset_t regions,
				    char *buf, size_t buflen)
{
	int i, j, r, next, last = 0;
	size_t used = 0;

	last = dm_bit_get_last(regions);

	i = dm_bit_get_first(regions);
	for(; i >= 0; i = dm_bit_get_next(regions, i)) {
		/* find range end */
		j = i;
		do
			next = j + 1;
		while ((j = dm_bit_get_next(regions, j)) == next);

		/* set to last set bit */
		j = next - 1;

		/* handle range vs. single region */
		if (i != j)
			r = dm_snprintf(buf, buflen, FMTu64 "-" FMTu64 "%s",
					(uint64_t) i, (uint64_t) j,
					(j == last) ? "" : ",");
		else
			r = dm_snprintf(buf, buflen, FMTu64 "%s", (uint64_t) i,
					(i == last) ? "" : ",");
		if (r < 0)
			goto_bad;

		i = next; /* skip handled bits if in range */

		buf += r;
		used += r;
	}

	return used;
bad:
	log_error("Could not format group list.");
	return 0;
}

/*
 * Calculate the space required to hold a string description of the group
 * described by the regions bitset using comma separated list in range
 * notation ("A,B,C,M-N").
 */
static size_t _stats_group_tag_len(const struct dm_stats *dms,
				   dm_bitset_t regions)
{
	int64_t i, j, next, nr_regions = 0;
	size_t buflen = 0, id_len = 0;

	/* check region ids and find last set bit */
	i = dm_bit_get_first(regions);
	for (; i >= 0; i = dm_bit_get_next(regions, i)) {
		/* length of region_id or range start in characters */
		id_len = (i) ? 1 + (size_t) log10(i) : 1;
		buflen += id_len;
		j = i;
		do
			next = j + 1;
		while ((j = dm_bit_get_next(regions, j)) == next);

		/* set to last set bit */
		j = next - 1;

		nr_regions += j - i + 1;

		/* handle range */
		if (i != j) {
			/* j is always > i, which is always >= 0 */
			id_len = 1 + (size_t) log10(j);
			buflen += id_len + 1; /* range end plus "-" */
		}
		buflen++;
		i = next; /* skip bits if handling range */
	}
	return buflen;
}

/*
 * Build a DMS_GROUP="..." tag for the group specified by group_id,
 * to be stored in the corresponding region's aux_data field.
 */
static char *_build_group_tag(struct dm_stats *dms, uint64_t group_id)
{
	char *aux_string, *buf;
	dm_bitset_t regions;
	const char *alias;
	size_t buflen = 0;
	int r;

	regions = dms->groups[group_id].regions;
	alias = dms->groups[group_id].alias;

	buflen = _stats_group_tag_len(dms, regions);

	if (!buflen)
		return_0;

	buflen += DMS_GROUP_TAG_LEN;
	buflen += 1 + (alias ? strlen(alias) : 0); /* 'alias:' */

	buf = aux_string = dm_malloc(buflen);
	if (!buf) {
		log_error("Could not allocate memory for aux_data string.");
		return NULL;
	}

	if (!dm_strncpy(buf, DMS_GROUP_TAG, DMS_GROUP_TAG_LEN + 1))
		goto_bad;

	buf += DMS_GROUP_TAG_LEN;
	buflen -= DMS_GROUP_TAG_LEN;

	r = dm_snprintf(buf, buflen, "%s%c", alias ? alias : "", DMS_GROUP_SEP);
	if (r < 0)
		goto_bad;

	buf += r;
	buflen -= r;

	r = _stats_group_tag_fill(dms, regions, buf, buflen);
	if (!r)
		goto_bad;

	return aux_string;
bad:
	log_error("Could not format group aux_data.");
	dm_free(aux_string);
	return NULL;
}

/*
 * Store updated aux_data for a region. The aux_data is passed to the
 * kernel using the @stats_set_aux message. Any required group tag is
 * generated from the current group table and included in the message.
 */
static int _stats_set_aux(struct dm_stats *dms,
			  uint64_t region_id, const char *aux_data)
{
	const char *group_tag = NULL;
	struct dm_task *dmt = NULL;
	char msg[STATS_MSG_BUF_LEN];

	/* group data required? */
	if (_stats_group_id_present(dms, region_id)) {
		group_tag = _build_group_tag(dms, region_id);
		if (!group_tag) {
			log_error("Could not build group descriptor for "
				  "region ID " FMTu64, region_id);
			goto bad;
		}
	}

	if (dm_snprintf(msg, sizeof(msg), "@stats_set_aux " FMTu64 " %s%s%s ",
			region_id, (group_tag) ? group_tag : "",
			(group_tag) ? DMS_AUX_SEP : "",
			(strlen(aux_data)) ? aux_data : "-") < 0) {
		log_error("Could not prepare @stats_set_aux message");
		goto bad;
	}

	if (!(dmt = _stats_send_message(dms, msg)))
		goto_bad;

	dm_free((char *) group_tag);

	/* no response to a @stats_set_aux message */
	dm_task_destroy(dmt);

	return 1;
bad:
	dm_free((char *) group_tag);
	return 0;
}

/*
 * Maximum length of a "start+end" range string:
 * Two 20 digit uint64_t, '+', and NULL.
 */
#define RANGE_LEN 42
static int _stats_create_region(struct dm_stats *dms, uint64_t *region_id,
				uint64_t start, uint64_t len, int64_t step,
				int precise, const char *hist_arg,
				const char *program_id,	const char *aux_data)
{
	char msg[STATS_MSG_BUF_LEN], range[RANGE_LEN], *endptr = NULL;
	const char *err_fmt = "Could not prepare @stats_create %s.";
	const char *precise_str = PRECISE_ARG;
	const char *resp, *opt_args = NULL;
	struct dm_task *dmt = NULL;
	int r = 0, nr_opt = 0;

	if (!_stats_bound(dms))
		return_0;

	if (!program_id || !strlen(program_id))
		program_id = dms->program_id;

	if (start || len) {
		if (dm_snprintf(range, sizeof(range), FMTu64 "+" FMTu64,
				start, len) < 0) {
			log_error(err_fmt, "range");
			return 0;
		}
	}

	if (precise < 0)
		precise = dms->precise;

	if (precise)
		nr_opt++;
	else
		precise_str = "";

	if (hist_arg)
		nr_opt++;
	else
		hist_arg = "";

	if (nr_opt) {
		if ((dm_asprintf((char **)&opt_args, "%d %s %s%s", nr_opt,
				 precise_str,
				 (strlen(hist_arg)) ? HISTOGRAM_ARG : "",
				 hist_arg)) < 0) {
			log_error(err_fmt, PRECISE_ARG " option.");
			return 0;
		}
	} else
		opt_args = dm_strdup("");

	if (dm_snprintf(msg, sizeof(msg), "@stats_create %s %s" FMTu64
			" %s %s %s", (start || len) ? range : "-",
			(step < 0) ? "/" : "",
			(uint64_t)llabs(step),
			opt_args, program_id, aux_data) < 0) {
		log_error(err_fmt, "message");
		dm_free((void *) opt_args);
		return 0;
	}

	if (!(dmt = _stats_send_message(dms, msg)))
		goto_out;

	resp = dm_task_get_message_response(dmt);
	if (!resp) {
		log_error("Could not parse empty @stats_create response.");
		goto out;
	}

	if (region_id) {
		errno = 0;
		*region_id = strtoull(resp, &endptr, 10);
		if (errno || resp == endptr)
			goto_out;
	}

	r = 1;

out:
	if (dmt)
		dm_task_destroy(dmt);
	dm_free((void *) opt_args);

	return r;
}

int dm_stats_create_region(struct dm_stats *dms, uint64_t *region_id,
			   uint64_t start, uint64_t len, int64_t step,
			   int precise, struct dm_histogram *bounds,
			   const char *program_id, const char *user_data)
{
	char *hist_arg = NULL;
	int r = 0;

	/* Nanosecond counters and histograms both need precise_timestamps. */
	if ((precise || bounds) && !_stats_check_precise_timestamps(dms))
		return_0;

	if (bounds) {
		/* _build_histogram_arg enables precise if vals < 1ms. */
		if (!(hist_arg = _build_histogram_arg(bounds, &precise)))
			goto_out;
	}

	r = _stats_create_region(dms, region_id, start, len, step,
				 precise, hist_arg, program_id, user_data);
	dm_free(hist_arg);

out:
	return r;
}

static void _stats_clear_group_regions(struct dm_stats *dms, uint64_t group_id)
{
	struct dm_stats_group *group;
	uint64_t i;

	group = &dms->groups[group_id];
	for (i = dm_bit_get_first(group->regions);
	     i != DM_STATS_GROUP_NOT_PRESENT;
	     i = dm_bit_get_next(group->regions, i))
		dms->regions[i].group_id = DM_STATS_GROUP_NOT_PRESENT;
}

static int _stats_remove_region_id_from_group(struct dm_stats *dms,
					      uint64_t region_id)
{
	struct dm_stats_region *region = &dms->regions[region_id];
	uint64_t group_id = region->group_id;
	dm_bitset_t regions = dms->groups[group_id].regions;

	if (!_stats_region_is_grouped(dms, region_id))
		return_0;

	dm_bit_clear(regions, region_id);

	/* removing group leader? */
	if (region_id == group_id) {
		_stats_clear_group_regions(dms, group_id);
		_stats_group_destroy(&dms->groups[group_id]);
	}

	return _stats_set_aux(dms, group_id, dms->regions[group_id].aux_data);
}

static int _stats_delete_region(struct dm_stats *dms, uint64_t region_id)
{
	char msg[STATS_MSG_BUF_LEN];
	struct dm_task *dmt;

	if (_stats_region_is_grouped(dms, region_id))
		if (!_stats_remove_region_id_from_group(dms, region_id)) {
			log_error("Could not remove region ID " FMTu64 " from "
				  "group ID " FMTu64,
				  region_id, dms->regions[region_id].group_id);
			return 0;
		}

	if (dm_snprintf(msg, sizeof(msg), "@stats_delete " FMTu64, region_id) < 0) {
		log_error("Could not prepare @stats_delete message.");
		return 0;
	}

	dmt = _stats_send_message(dms, msg);
	if (!dmt)
		return_0;
	dm_task_destroy(dmt);

	return 1;
}

int dm_stats_delete_region(struct dm_stats *dms, uint64_t region_id)
{
	int listed = 0;

	if (!_stats_bound(dms))
		return_0;

	/*
	 * To correctly delete a region, that may be part of a group, a
	 * listed handle is required, since the region may need to be
	 * removed from another region's group descriptor; earlier
	 * versions of the region deletion interface do not have this
	 * requirement since there are no dependencies between regions.
	 *
	 * Listing a previously unlisted handle has numerous
	 * side-effects on other calls and operations (e.g. stats
	 * walks), especially when returning to a function that depends
	 * on the state of the region table, or statistics cursor.
	 *
	 * To avoid changing the semantics of the API, and the need for
	 * a versioned symbol, maintain a flag indicating when a listing
	 * has been carried out, and drop the region table before
	 * returning.
	 *
	 * This ensures compatibility with programs compiled against
	 * earlier versions of libdm.
	 */
	if (!dms->regions && !(listed = dm_stats_list(dms, dms->program_id))) {
		log_error("Could not obtain region list while deleting "
			  "region ID " FMTu64, region_id);
		goto bad;
	}

	if (!dm_stats_get_nr_regions(dms)) {
		log_error("Could not delete region ID " FMTu64 ": "
			  "no regions found", region_id);
		goto bad;
	}

	/* includes invalid and special region_id values */
	if (!dm_stats_region_present(dms, region_id)) {
		log_error("Region ID " FMTu64 " does not exist", region_id);
		goto bad;
	}

	if (!_stats_delete_region(dms, region_id))
		goto bad;

	if (!listed)
		/* wipe region and mark as not present */
		_stats_region_destroy(&dms->regions[region_id]);
	else
		/* return handle to prior state */
		_stats_regions_destroy(dms);

	return 1;
bad:
	if (listed)
		_stats_regions_destroy(dms);

	return 0;
}

int dm_stats_clear_region(struct dm_stats *dms, uint64_t region_id)
{
	char msg[STATS_MSG_BUF_LEN];
	struct dm_task *dmt;

	if (!_stats_bound(dms))
		return_0;

	if (dm_snprintf(msg, sizeof(msg), "@stats_clear " FMTu64, region_id) < 0) {
		log_error("Could not prepare @stats_clear message.");
		return 0;
	}

	dmt = _stats_send_message(dms, msg);

	if (!dmt)
		return_0;

	dm_task_destroy(dmt);

	return 1;
}

static struct dm_task *_stats_print_region(struct dm_stats *dms,
				    uint64_t region_id, unsigned start_line,
				    unsigned num_lines, unsigned clear)
{
	/* @stats_print[_clear] <region_id> [<start_line> <num_lines>] */
	const char *err_fmt = "Could not prepare @stats_print %s.";
	char msg[STATS_MSG_BUF_LEN], lines[RANGE_LEN];
	struct dm_task *dmt = NULL;

	if (start_line || num_lines)
		if (dm_snprintf(lines, sizeof(lines),
				"%u %u", start_line, num_lines) < 0) {
			log_error(err_fmt, "row specification");
			return NULL;
		}

	if (dm_snprintf(msg, sizeof(msg), "@stats_print%s " FMTu64 " %s",
			(clear) ? "_clear" : "",
			region_id, (start_line || num_lines) ? lines : "") < 0) {
		log_error(err_fmt, "message");
		return NULL;
	}

	if (!(dmt = _stats_send_message(dms, msg)))
		return_NULL;

	return dmt;
}

char *dm_stats_print_region(struct dm_stats *dms, uint64_t region_id,
			    unsigned start_line, unsigned num_lines,
			    unsigned clear)
{
	char *resp = NULL;
	struct dm_task *dmt = NULL;
	const char *response;

	if (!_stats_bound(dms))
		return_0;

	/*
	 * FIXME: 'print' can be emulated for groups or aggregate regions
	 * by populating the handle and emitting aggregate counter data
	 * in the kernel print format.
	 */
	if (region_id == DM_STATS_WALK_GROUP)
		return_0;

	dmt = _stats_print_region(dms, region_id,
				  start_line, num_lines, clear);

	if (!dmt)
		return_0;

	if (!(response = dm_task_get_message_response(dmt)))
		goto_out;

	if (!(resp = dm_pool_strdup(dms->mem, response)))
		log_error("Could not allocate memory for response buffer.");
out:
	dm_task_destroy(dmt);

	return resp;
}

void dm_stats_buffer_destroy(struct dm_stats *dms, char *buffer)
{
	dm_pool_free(dms->mem, buffer);
}

uint64_t dm_stats_get_nr_regions(const struct dm_stats *dms)
{
	if (!dms)
		return_0;

	if (!dms->regions)
		return 0;

	return dms->nr_regions;
}

uint64_t dm_stats_get_nr_groups(const struct dm_stats *dms)
{
	uint64_t group_id, nr_groups = 0;

	if (!dms)
		return_0;

	/* no regions or groups? */
	if (!dms->regions || !dms->groups)
		return 0;

	for (group_id = 0; group_id <= dms->max_region; group_id++)
		if (dms->groups[group_id].group_id
		    != DM_STATS_GROUP_NOT_PRESENT)
			nr_groups++;

	return nr_groups;
}

/**
 * Test whether region_id is present in this set of stats data.
 */
int dm_stats_region_present(const struct dm_stats *dms, uint64_t region_id)
{
	if (!dms->regions)
		return_0;

	if (region_id > dms->max_region)
		return 0;

	return _stats_region_present(&dms->regions[region_id]);
}

static int _dm_stats_populate_region(struct dm_stats *dms, uint64_t region_id,
				     const char *resp)
{
	struct dm_stats_region *region = &dms->regions[region_id];

	if (!_stats_bound(dms))
		return_0;

	if (!region) {
		log_error("Cannot populate empty handle before dm_stats_list().");
		return 0;
	}
	if (!_stats_parse_region(dms, resp, region, region->timescale)) {
		log_error("Could not parse @stats_print message response.");
		return 0;
	}
	region->region_id = region_id;
	return 1;
}

int dm_stats_populate(struct dm_stats *dms, const char *program_id,
		      uint64_t region_id)
{
	int all_regions = (region_id == DM_STATS_REGIONS_ALL);
	struct dm_task *dmt = NULL; /* @stats_print task */
	uint64_t saved_flags; /* saved walk flags */
	const char *resp;

	/*
	 * We are about do destroy and re-create the region table, so it
	 * is safe to use the cursor embedded in the stats handle: just
	 * save a copy of the current walk_flags to restore later.
	 */
	saved_flags = dms->walk_flags;

	if (!_stats_bound(dms))
		return_0;

	if ((!all_regions) && (region_id & DM_STATS_WALK_GROUP)) {
		log_error("Invalid region_id for dm_stats_populate: "
			  "DM_STATS_WALK_GROUP");
		return 0;
	}

	/* allow zero-length program_id for populate */
	if (!program_id)
		program_id = dms->program_id;

	if (all_regions && !dm_stats_list(dms, program_id)) {
		log_error("Could not parse @stats_list response.");
		goto bad;
	} else if (!_stats_set_name_cache(dms)) {
		goto_bad;
	}

	if (!dms->nr_regions) {
		log_verbose("No stats regions registered: %s", dms->name);
		return 0;
	}

	dms->walk_flags = DM_STATS_WALK_REGION;
	dm_stats_walk_start(dms);
	do {
		region_id = (all_regions)
			     ? dm_stats_get_current_region(dms) : region_id;

		/* obtain all lines and clear counter values */
		if (!(dmt = _stats_print_region(dms, region_id, 0, 0, 1)))
			goto_bad;

		resp = dm_task_get_message_response(dmt);
		if (!_dm_stats_populate_region(dms, region_id, resp)) {
			dm_task_destroy(dmt);
			goto_bad;
		}

		dm_task_destroy(dmt);
		dm_stats_walk_next(dms);

	} while (all_regions && !dm_stats_walk_end(dms));

	dms->walk_flags = saved_flags;
	return 1;

bad:
	dms->walk_flags = saved_flags;
	_stats_regions_destroy(dms);
	dms->regions = NULL;
	return 0;
}

/**
 * destroy a dm_stats object and all associated regions and counter sets.
 */
void dm_stats_destroy(struct dm_stats *dms)
{
	if (!dms)
		return;

	_stats_regions_destroy(dms);
	_stats_groups_destroy(dms);
	_stats_clear_binding(dms);
	dm_pool_destroy(dms->mem);
	dm_pool_destroy(dms->hist_mem);
	dm_pool_destroy(dms->group_mem);
	dm_free(dms->program_id);
	dm_free((char *) dms->name);
	dm_free(dms);
}

/*
 * Walk each area that is a member of region_id rid.
 * i is a variable of type int that holds the current area_id.
 */
#define _foreach_region_area(dms, rid, i)				\
for ((i) = 0; (i) < _nr_areas_region(&dms->regions[(rid)]); (i)++)	\

/*
 * Walk each region that is a member of group_id gid.
 * i is a variable of type int that holds the current region_id.
 */
#define _foreach_group_region(dms, gid, i)			\
for ((i) = dm_bit_get_first((dms)->groups[(gid)].regions);	\
     (i) != DM_STATS_GROUP_NOT_PRESENT;				\
     (i) = dm_bit_get_next((dms)->groups[(gid)].regions, (i)))	\

/*
 * Walk each region that is a member of group_id gid visiting each
 * area within the region.
 * i is a variable of type int that holds the current region_id.
 * j is a variable of type int variable that holds the current area_id.
 */
#define _foreach_group_area(dms, gid, i, j)			\
_foreach_group_region(dms, gid, i)				\
	_foreach_region_area(dms, i, j)

static uint64_t _stats_get_counter(const struct dm_stats *dms,
				   const struct dm_stats_counters *area,
				   dm_stats_counter_t counter)
{
	switch(counter) {
	case DM_STATS_READS_COUNT:
		return area->reads;
	case DM_STATS_READS_MERGED_COUNT:
		return area->reads_merged;
	case DM_STATS_READ_SECTORS_COUNT:
		return area->read_sectors;
	case DM_STATS_READ_NSECS:
		return area->read_nsecs;
	case DM_STATS_WRITES_COUNT:
		return area->writes;
	case DM_STATS_WRITES_MERGED_COUNT:
		return area->writes_merged;
	case DM_STATS_WRITE_SECTORS_COUNT:
		return area->write_sectors;
	case DM_STATS_WRITE_NSECS:
		return area->write_nsecs;
	case DM_STATS_IO_IN_PROGRESS_COUNT:
		return area->io_in_progress;
	case DM_STATS_IO_NSECS:
		return area->io_nsecs;
	case DM_STATS_WEIGHTED_IO_NSECS:
		return area->weighted_io_nsecs;
	case DM_STATS_TOTAL_READ_NSECS:
		return area->total_read_nsecs;
	case DM_STATS_TOTAL_WRITE_NSECS:
		return area->total_write_nsecs;
	case DM_STATS_NR_COUNTERS:
	default:
		log_error("Attempt to read invalid counter: %d", counter);
	}
	return 0;
}

uint64_t dm_stats_get_counter(const struct dm_stats *dms,
			      dm_stats_counter_t counter,
			      uint64_t region_id, uint64_t area_id)
{
	uint64_t i, j, sum = 0; /* aggregation */
	int sum_regions = 0;
	struct dm_stats_region *region;
	struct dm_stats_counters *area;

	region_id = (region_id == DM_STATS_REGION_CURRENT)
		     ? dms->cur_region : region_id ;
	area_id = (area_id == DM_STATS_REGION_CURRENT)
		   ? dms->cur_area : area_id ;

	sum_regions = !!(region_id & DM_STATS_WALK_GROUP);

	if (region_id == DM_STATS_WALK_GROUP)
		/* group walk using the cursor */
		region_id = dms->cur_group;
	else if (region_id & DM_STATS_WALK_GROUP)
		/* group walk using immediate group_id */
		region_id &= ~DM_STATS_WALK_GROUP;
	region = &dms->regions[region_id];

	/*
	 * All statistics aggregation takes place here: aggregate metrics
	 * are calculated as normal using the aggregated counter values
	 * returned for the region or group specified.
	 */

	if (_stats_region_is_grouped(dms, region_id) && (sum_regions)) {
		/* group */
		if (area_id & DM_STATS_WALK_GROUP)
			_foreach_group_area(dms, region->group_id, i, j) {
				area = &dms->regions[i].counters[j];
				sum += _stats_get_counter(dms, area, counter);
			}
		else
			_foreach_group_region(dms, region->group_id, i) {
				area = &dms->regions[i].counters[area_id];
				sum += _stats_get_counter(dms, area, counter);
			}
	} else if (area_id == DM_STATS_WALK_REGION) {
		/* aggregate region */
		_foreach_region_area(dms, region_id, j) {
			area = &dms->regions[region_id].counters[j];
			sum += _stats_get_counter(dms, area, counter);
		}
	} else {
		/* plain region / area */
		area = &region->counters[area_id];
		sum = _stats_get_counter(dms, area, counter);
	}

	return sum;
}

/*
 * Methods for accessing named counter fields. All methods share the
 * following naming scheme and prototype:
 *
 * uint64_t dm_stats_get_COUNTER(const struct dm_stats *, uint64_t, uint64_t)
 *
 * Where the two integer arguments are the region_id and area_id
 * respectively.
 *
 * name is the name of the counter (lower case)
 * counter is the part of the enum name following DM_STATS_ (upper case)
 */
#define MK_STATS_GET_COUNTER_FN(name, counter)				\
uint64_t dm_stats_get_ ## name(const struct dm_stats *dms,		\
			       uint64_t region_id, uint64_t area_id)	\
{									\
	return dm_stats_get_counter(dms, DM_STATS_ ## counter,		\
				    region_id, area_id);		\
}

MK_STATS_GET_COUNTER_FN(reads, READS_COUNT)
MK_STATS_GET_COUNTER_FN(reads_merged, READS_MERGED_COUNT)
MK_STATS_GET_COUNTER_FN(read_sectors, READ_SECTORS_COUNT)
MK_STATS_GET_COUNTER_FN(read_nsecs, READ_NSECS)
MK_STATS_GET_COUNTER_FN(writes, WRITES_COUNT)
MK_STATS_GET_COUNTER_FN(writes_merged, WRITES_MERGED_COUNT)
MK_STATS_GET_COUNTER_FN(write_sectors, WRITE_SECTORS_COUNT)
MK_STATS_GET_COUNTER_FN(write_nsecs, WRITE_NSECS)
MK_STATS_GET_COUNTER_FN(io_in_progress, IO_IN_PROGRESS_COUNT)
MK_STATS_GET_COUNTER_FN(io_nsecs, IO_NSECS)
MK_STATS_GET_COUNTER_FN(weighted_io_nsecs, WEIGHTED_IO_NSECS)
MK_STATS_GET_COUNTER_FN(total_read_nsecs, TOTAL_READ_NSECS)
MK_STATS_GET_COUNTER_FN(total_write_nsecs, TOTAL_WRITE_NSECS)
#undef MK_STATS_GET_COUNTER_FN

/*
 * Floating point stats metric functions
 *
 * Called from dm_stats_get_metric() to calculate the value of
 * the requested metric.
 *
 * int _metric_name(const struct dm_stats *dms,
 * 		    struct dm_stats_counters *c,
 * 		    double *value);
 *
 * Calculate a metric value from the counter data for the given
 * identifiers and store it in the memory pointed to by value,
 * applying group or region aggregation if enabled.
 *
 * Return one on success or zero on failure.
 *
 * To add a new metric:
 *
 * o Add a new name to the dm_stats_metric_t enum.
 * o Create a _metric_fn() to calculate the new metric.
 * o Add _metric_fn to the _metrics function table
 *   (entries in enum order).
 * o Do not add a new named public function for the metric -
 *   users of new metrics are encouraged to convert to the enum
 *   based metric interface.
 *
 */

static int _rd_merges_per_sec(const struct dm_stats *dms, double *rrqm,
			      uint64_t region_id, uint64_t area_id)
{
	double mrgs;
	mrgs = (double) dm_stats_get_counter(dms, DM_STATS_READS_MERGED_COUNT,
					     region_id, area_id);

	*rrqm = mrgs / (double) dms->interval_ns;

	return 1;
}

static int _wr_merges_per_sec(const struct dm_stats *dms, double *wrqm,
			      uint64_t region_id, uint64_t area_id)
{
	double mrgs;
	mrgs = (double) dm_stats_get_counter(dms, DM_STATS_WRITES_MERGED_COUNT,
					     region_id, area_id);

	*wrqm = mrgs / (double) dms->interval_ns;

	return 1;
}

static int _reads_per_sec(const struct dm_stats *dms, double *rd_s,
			  uint64_t region_id, uint64_t area_id)
{
	double reads;
	reads = (double) dm_stats_get_counter(dms, DM_STATS_READS_COUNT,
					      region_id, area_id);

	*rd_s = (reads * NSEC_PER_SEC) / (double) dms->interval_ns;

	return 1;
}

static int _writes_per_sec(const struct dm_stats *dms, double *wr_s,
			   uint64_t region_id, uint64_t area_id)
{
	double writes;
	writes = (double) dm_stats_get_counter(dms, DM_STATS_WRITES_COUNT,
					       region_id, area_id);

	*wr_s = (writes * NSEC_PER_SEC) / (double) dms->interval_ns;

	return 1;
}

static int _read_sectors_per_sec(const struct dm_stats *dms, double *rsec_s,
				 uint64_t region_id, uint64_t area_id)
{
	double sect;
	sect = (double) dm_stats_get_counter(dms, DM_STATS_READ_SECTORS_COUNT,
					     region_id, area_id);

	*rsec_s = (sect * (double) NSEC_PER_SEC) / (double) dms->interval_ns;

	return 1;
}

static int _write_sectors_per_sec(const struct dm_stats *dms, double *wsec_s,
				  uint64_t region_id, uint64_t area_id)
{
	double sect;
	sect = (double) dm_stats_get_counter(dms, DM_STATS_WRITE_SECTORS_COUNT,
					     region_id, area_id);

	*wsec_s = (sect * (double) NSEC_PER_SEC) / (double) dms->interval_ns;

	return 1;
}

static int _average_request_size(const struct dm_stats *dms, double *arqsz,
				 uint64_t region_id, uint64_t area_id)
{
	double ios, sectors;

	ios = (double) (dm_stats_get_counter(dms, DM_STATS_READS_COUNT,
					     region_id, area_id)
			+ dm_stats_get_counter(dms, DM_STATS_WRITES_COUNT,
					       region_id, area_id));
	sectors = (double) (dm_stats_get_counter(dms, DM_STATS_READ_SECTORS_COUNT,
						 region_id, area_id)
			    + dm_stats_get_counter(dms, DM_STATS_WRITE_SECTORS_COUNT,
						   region_id, area_id));

	if (ios > 0.0)
		*arqsz = sectors / ios;
	else
		*arqsz = 0.0;

	return 1;
}

static int _average_queue_size(const struct dm_stats *dms, double *qusz,
			       uint64_t region_id, uint64_t area_id)
{
	double io_ticks;
	io_ticks = (double) dm_stats_get_counter(dms, DM_STATS_WEIGHTED_IO_NSECS,
						 region_id, area_id);

	if (io_ticks > 0.0)
		*qusz = io_ticks / (double) dms->interval_ns;
	else
		*qusz = 0.0;

	return 1;
}

static int _average_wait_time(const struct dm_stats *dms, double *await,
			      uint64_t region_id, uint64_t area_id)
{
	uint64_t io_ticks, nr_ios;

	io_ticks = dm_stats_get_counter(dms, DM_STATS_READ_NSECS,
					region_id, area_id);
	io_ticks += dm_stats_get_counter(dms, DM_STATS_WRITE_NSECS,
					 region_id, area_id);

	nr_ios = dm_stats_get_counter(dms, DM_STATS_READS_COUNT,
				      region_id, area_id);
	nr_ios += dm_stats_get_counter(dms, DM_STATS_WRITES_COUNT,
				       region_id, area_id);

	if (nr_ios > 0)
		*await = (double) io_ticks / (double) nr_ios;
	else
		*await = 0.0;

	return 1;
}

static int _average_rd_wait_time(const struct dm_stats *dms, double *await,
				 uint64_t region_id, uint64_t area_id)
{
	uint64_t rd_io_ticks, nr_rd_ios;

	rd_io_ticks = dm_stats_get_counter(dms, DM_STATS_READ_NSECS,
					   region_id, area_id);
	nr_rd_ios = dm_stats_get_counter(dms, DM_STATS_READS_COUNT,
					 region_id, area_id);

	/*
	 * If rd_io_ticks is > 0 this should imply that nr_rd_ios is
	 * also > 0 (unless a kernel bug exists). Test for both here
	 * before using the IO count as a divisor (Coverity).
	 */
	if (rd_io_ticks > 0 && nr_rd_ios > 0)
		*await = (double) rd_io_ticks / (double) nr_rd_ios;
	else
		*await = 0.0;

	return 1;
}

static int _average_wr_wait_time(const struct dm_stats *dms, double *await,
				 uint64_t region_id, uint64_t area_id)
{
	uint64_t wr_io_ticks, nr_wr_ios;

	wr_io_ticks = dm_stats_get_counter(dms, DM_STATS_WRITE_NSECS,
					   region_id, area_id);
	nr_wr_ios = dm_stats_get_counter(dms, DM_STATS_WRITES_COUNT,
					 region_id, area_id);

	/*
	 * If wr_io_ticks is > 0 this should imply that nr_wr_ios is
	 * also > 0 (unless a kernel bug exists). Test for both here
	 * before using the IO count as a divisor (Coverity).
	 */
	if (wr_io_ticks > 0 && nr_wr_ios > 0)
		*await = (double) wr_io_ticks / (double) nr_wr_ios;
	else
		*await = 0.0;

	return 1;
}

static int _throughput(const struct dm_stats *dms, double *tput,
		       uint64_t region_id, uint64_t area_id)
{
	uint64_t nr_ios;

	nr_ios = dm_stats_get_counter(dms, DM_STATS_READS_COUNT,
				      region_id, area_id);
	nr_ios += dm_stats_get_counter(dms, DM_STATS_WRITES_COUNT,
				       region_id, area_id);

	*tput = ((double) NSEC_PER_SEC * (double) nr_ios)
		/ (double) (dms->interval_ns);

	return 1;
}

static int _utilization(const struct dm_stats *dms, double *util,
			uint64_t region_id, uint64_t area_id)
{
	uint64_t io_nsecs, interval_ns = dms->interval_ns;

	/**
	 * If io_nsec > interval_ns there is something wrong with the clock
	 * for the last interval; do not allow a value > 100% utilization
	 * to be passed to a dm_make_percent() call. We expect to see these
	 * at startup if counters have not been cleared before the first read.
	 *
	 * A zero interval_ns is also an error since metrics cannot be
	 * calculated without a defined interval - return zero and emit a
	 * backtrace in this case.
	 */
	io_nsecs = dm_stats_get_counter(dms, DM_STATS_IO_NSECS,
					region_id, area_id);

	if (!interval_ns) {
		*util = 0.0;
		return_0;
	}

	io_nsecs = ((io_nsecs < interval_ns) ? io_nsecs : interval_ns);

	*util = (double) io_nsecs / (double) interval_ns;

	return 1;
}

static int _service_time(const struct dm_stats *dms, double *svctm,
			 uint64_t region_id, uint64_t area_id)
{
	double tput, util;

	if (!_throughput(dms, &tput, region_id, area_id))
		return 0;

	if (!_utilization(dms, &util, region_id, area_id))
		return 0;

	util *= 100;

	/* avoid NAN with zero counter values */
	if ( (uint64_t) tput == 0 || (uint64_t) util == 0) {
		*svctm = 0.0;
		return 1;
	}

	*svctm = ((double) NSEC_PER_SEC * dm_percent_to_float(util))
		  / (100.0 * tput);

	return 1;
}

/*
 * Table in enum order:
 *      DM_STATS_RD_MERGES_PER_SEC,
 *      DM_STATS_WR_MERGES_PER_SEC,
 *      DM_STATS_READS_PER_SEC,
 *      DM_STATS_WRITES_PER_SEC,
 *      DM_STATS_READ_SECTORS_PER_SEC,
 *      DM_STATS_WRITE_SECTORS_PER_SEC,
 *      DM_STATS_AVERAGE_REQUEST_SIZE,
 *      DM_STATS_AVERAGE_QUEUE_SIZE,
 *      DM_STATS_AVERAGE_WAIT_TIME,
 *      DM_STATS_AVERAGE_RD_WAIT_TIME,
 *      DM_STATS_AVERAGE_WR_WAIT_TIME
 *      DM_STATS_SERVICE_TIME,
 *      DM_STATS_THROUGHPUT,
 *      DM_STATS_UTILIZATION
 *
*/

typedef int (*_metric_fn_t)(const struct dm_stats *, double *,
			    uint64_t, uint64_t);

_metric_fn_t _metrics[DM_STATS_NR_METRICS] = {
	_rd_merges_per_sec,
	_wr_merges_per_sec,
	_reads_per_sec,
	_writes_per_sec,
	_read_sectors_per_sec,
	_write_sectors_per_sec,
	_average_request_size,
	_average_queue_size,
	_average_wait_time,
	_average_rd_wait_time,
	_average_wr_wait_time,
	_service_time,
	_throughput,
	_utilization
};

int dm_stats_get_metric(const struct dm_stats *dms, int metric,
			uint64_t region_id, uint64_t area_id, double *value)
{
	if (!dms->interval_ns)
		return_0;

	/*
	 * Decode DM_STATS_{REGION,AREA}_CURRENT here; counters will then
	 * be returned for the actual current region and area.
	 *
	 * DM_STATS_WALK_GROUP is passed through to the counter methods -
	 * aggregates for the group are returned and used to calculate
	 * the metric for the group totals.
	 */
	region_id = (region_id == DM_STATS_REGION_CURRENT)
		     ? dms->cur_region : region_id ;
	area_id = (area_id == DM_STATS_REGION_CURRENT)
		   ? dms->cur_area : area_id ;

	if (metric < 0 || metric >= DM_STATS_NR_METRICS) {
		log_error("Attempt to read invalid metric: %d", metric);
		return 0;
	}

	return _metrics[metric](dms, value, region_id, area_id);
}

/**
 * Methods for accessing stats metrics. All methods share the
 * following naming scheme and prototype:
 *
 * uint64_t dm_stats_get_metric(struct dm_stats *,
 * 				int, int,
 * 				uint64_t, uint64_t,
 * 				double *v)
 *
 * Where the two integer arguments are the region_id and area_id
 * respectively.
 *
 * name is the name of the metric (lower case)
 * metric is the part of the enum name following DM_STATS_ (upper case)
 */
#define MK_STATS_GET_METRIC_FN(name, metric, meta)			\
int dm_stats_get_ ## name(const struct dm_stats *dms, double *meta,	\
			  uint64_t region_id, uint64_t area_id)		\
{									\
	return dm_stats_get_metric(dms, DM_STATS_ ## metric, 		\
				   region_id, area_id, meta);		\
}

MK_STATS_GET_METRIC_FN(rd_merges_per_sec, RD_MERGES_PER_SEC, rrqm)
MK_STATS_GET_METRIC_FN(wr_merges_per_sec, WR_MERGES_PER_SEC, wrqm)
MK_STATS_GET_METRIC_FN(reads_per_sec, READS_PER_SEC, rd_s)
MK_STATS_GET_METRIC_FN(writes_per_sec, WRITES_PER_SEC, wr_s)
MK_STATS_GET_METRIC_FN(read_sectors_per_sec, READ_SECTORS_PER_SEC, rsec_s)
MK_STATS_GET_METRIC_FN(write_sectors_per_sec, WRITE_SECTORS_PER_SEC, wsec_s)
MK_STATS_GET_METRIC_FN(average_request_size, AVERAGE_REQUEST_SIZE, arqsz)
MK_STATS_GET_METRIC_FN(average_queue_size, AVERAGE_QUEUE_SIZE, qusz)
MK_STATS_GET_METRIC_FN(average_wait_time, AVERAGE_WAIT_TIME, await)
MK_STATS_GET_METRIC_FN(average_rd_wait_time, AVERAGE_RD_WAIT_TIME, await)
MK_STATS_GET_METRIC_FN(average_wr_wait_time, AVERAGE_WR_WAIT_TIME, await)
MK_STATS_GET_METRIC_FN(service_time, SERVICE_TIME, svctm)
MK_STATS_GET_METRIC_FN(throughput, THROUGHPUT, tput)

/*
 * Utilization is an exception since it used the dm_percent_t type in the
 * original named function based interface: preserve this behaviour for
 * backwards compatibility with existing users.
 *
 * The same metric may be accessed as a double via the enum based metric
 * interface.
 */
int dm_stats_get_utilization(const struct dm_stats *dms, dm_percent_t *util,
			     uint64_t region_id, uint64_t area_id)
{
	double _util;

	if (!dm_stats_get_metric(dms, DM_STATS_UTILIZATION,
				 region_id, area_id, &_util))
		return_0;
	/* scale up utilization value in the range [0.00..1.00] */
	*util = dm_make_percent(DM_PERCENT_1 * _util, DM_PERCENT_1);
	return 1;
}

void dm_stats_set_sampling_interval_ms(struct dm_stats *dms, uint64_t interval_ms)
{
	/* All times use nsecs internally. */
	dms->interval_ns = interval_ms * NSEC_PER_MSEC;
}

void dm_stats_set_sampling_interval_ns(struct dm_stats *dms, uint64_t interval_ns)
{
	dms->interval_ns = interval_ns;
}

uint64_t dm_stats_get_sampling_interval_ms(const struct dm_stats *dms)
{
	/* All times use nsecs internally. */
	return (dms->interval_ns / NSEC_PER_MSEC);
}

uint64_t dm_stats_get_sampling_interval_ns(const struct dm_stats *dms)
{
	/* All times use nsecs internally. */
	return (dms->interval_ns);
}

int dm_stats_set_program_id(struct dm_stats *dms, int allow_empty,
			    const char *program_id)
{
	if (!allow_empty && (!program_id || !strlen(program_id))) {
		log_error("Empty program_id not permitted without "
			  "allow_empty=1");
		return 0;
	}

	if (!program_id)
		program_id = "";

	dm_free(dms->program_id);

	if (!(dms->program_id = dm_strdup(program_id)))
		return_0;

	return 1;
}

uint64_t dm_stats_get_current_region(const struct dm_stats *dms)
{
	return dms->cur_region;
}

uint64_t dm_stats_get_current_area(const struct dm_stats *dms)
{
	return dms->cur_area & ~DM_STATS_WALK_ALL;
}

int dm_stats_get_region_start(const struct dm_stats *dms, uint64_t *start,
			      uint64_t region_id)
{
	if (!dms || !dms->regions)
		return_0;

	/* start is unchanged when aggregating areas */
	if (region_id & DM_STATS_WALK_REGION)
		region_id &= ~DM_STATS_WALK_REGION;

	/* use start of first region as group start */
	if (region_id & DM_STATS_WALK_GROUP) {
		if (region_id == DM_STATS_WALK_GROUP)
			region_id = dms->cur_group;
		else
			region_id &= ~DM_STATS_WALK_GROUP;
	}

	*start = dms->regions[region_id].start;
	return 1;
}

int dm_stats_get_region_len(const struct dm_stats *dms, uint64_t *len,
			    uint64_t region_id)
{
	uint64_t i;
	if (!dms || !dms->regions)
		return_0;

	*len = 0;

	/* length is unchanged when aggregating areas */
	if (region_id & DM_STATS_WALK_REGION)
		region_id &= ~DM_STATS_WALK_REGION;

	if (region_id & DM_STATS_WALK_GROUP) {
		/* decode region / group ID */
		if (region_id == DM_STATS_WALK_GROUP)
			region_id = dms->cur_group;
		else
			region_id &= ~DM_STATS_WALK_GROUP;

		/* use sum of region sizes as group size */
		if (_stats_region_is_grouped(dms, region_id))
			_foreach_group_region(dms, dms->cur_group, i)
				*len += dms->regions[i].len;
		else {
			log_error("Group ID " FMTu64 " does not exist",
				  region_id);
			return 0;
		}
	} else
		*len = dms->regions[region_id].len;

	return 1;
}

int dm_stats_get_region_area_len(const struct dm_stats *dms, uint64_t *len,
				 uint64_t region_id)
{
	if (!dms || !dms->regions)
		return_0;

	/* groups are not subdivided - area size equals group size */
	if (region_id & (DM_STATS_WALK_GROUP | DM_STATS_WALK_REGION))
		/* get_region_len will decode region_id */
		return dm_stats_get_region_len(dms, len, region_id);

	*len = dms->regions[region_id].step;
	return 1;
}

int dm_stats_get_current_region_start(const struct dm_stats *dms,
				      uint64_t *start)
{
	return dm_stats_get_region_start(dms, start, dms->cur_region);
}

int dm_stats_get_current_region_len(const struct dm_stats *dms,
				    uint64_t *len)
{
	return dm_stats_get_region_len(dms, len, dms->cur_region);
}

int dm_stats_get_current_region_area_len(const struct dm_stats *dms,
					 uint64_t *step)
{
	return dm_stats_get_region_area_len(dms, step, dms->cur_region);
}

int dm_stats_get_area_start(const struct dm_stats *dms, uint64_t *start,
			    uint64_t region_id, uint64_t area_id)
{
	struct dm_stats_region *region;
	if (!dms || !dms->regions)
		return_0;

	/* group or region area start equals region start */
	if (region_id & (DM_STATS_WALK_GROUP | DM_STATS_WALK_REGION))
		return dm_stats_get_region_start(dms, start, region_id);

	region = &dms->regions[region_id];
	*start = region->start + region->step * area_id;
	return 1;
}

int dm_stats_get_area_offset(const struct dm_stats *dms, uint64_t *offset,
			     uint64_t region_id, uint64_t area_id)
{
	if (!dms || !dms->regions)
		return_0;

	/* no areas for groups or aggregate regions */
	if (region_id & (DM_STATS_WALK_GROUP | DM_STATS_WALK_REGION))
		*offset = 0;
	else
		*offset = dms->regions[region_id].step * area_id;

	return 1;
}

int dm_stats_get_current_area_start(const struct dm_stats *dms,
				    uint64_t *start)
{
	return dm_stats_get_area_start(dms, start,
				       dms->cur_region, dms->cur_area);
}

int dm_stats_get_current_area_offset(const struct dm_stats *dms,
					  uint64_t *offset)
{
	return dm_stats_get_area_offset(dms, offset,
				       dms->cur_region, dms->cur_area);
}

int dm_stats_get_current_area_len(const struct dm_stats *dms,
				  uint64_t *len)
{
	return dm_stats_get_region_area_len(dms, len, dms->cur_region);
}

const char *dm_stats_get_region_program_id(const struct dm_stats *dms,
					   uint64_t region_id)
{
	const char *program_id = NULL;

	if (region_id & DM_STATS_WALK_GROUP)
		return dms->program_id;

	if (region_id & DM_STATS_WALK_REGION)
		region_id &= ~DM_STATS_WALK_REGION;

	program_id = dms->regions[region_id].program_id;
	return (program_id) ? program_id : "";
}

const char *dm_stats_get_region_aux_data(const struct dm_stats *dms,
					 uint64_t region_id)
{
	const char *aux_data = NULL;

	if (region_id & DM_STATS_WALK_GROUP)
		return "";

	if (region_id & DM_STATS_WALK_REGION)
		region_id &= ~DM_STATS_WALK_REGION;

	aux_data = dms->regions[region_id].aux_data;
	return (aux_data) ? aux_data : "" ;
}

int dm_stats_set_alias(struct dm_stats *dms, uint64_t group_id, const char *alias)
{
	struct dm_stats_group *group = NULL;
	const char *old_alias = NULL;

	if (!dms->regions || !dms->groups || !alias)
		return_0;

	if (!_stats_region_is_grouped(dms, group_id)) {
		log_error("Cannot set alias for ungrouped region ID "
			  FMTu64, group_id);
		return 0;
	}

	if (group_id & DM_STATS_WALK_GROUP) {
		if (group_id == DM_STATS_WALK_GROUP)
			group_id = dms->cur_group;
		else
			group_id &= ~DM_STATS_WALK_GROUP;
	}

	if (group_id != dms->regions[group_id].group_id) {
		/* dm_stats_set_alias() must be called on the group ID. */
		log_error("Cannot set alias for group member " FMTu64 ".",
			  group_id);
		return 0;
	}

	group = &dms->groups[group_id];
	old_alias = group->alias;

	group->alias = dm_strdup(alias);
	if (!group->alias) {
		log_error("Could not allocate memory for alias.");
		goto bad;
	}

	if (!_stats_set_aux(dms, group_id, dms->regions[group_id].aux_data)) {
		log_error("Could not set new aux_data");
		goto bad;
	}

	dm_free((char *) old_alias);

	return 1;

bad:
	dm_free((char *) group->alias);
	group->alias = old_alias;
	return 0;
}

const char *dm_stats_get_alias(const struct dm_stats *dms, uint64_t id)
{
	const struct dm_stats_region *region;

	id = (id == DM_STATS_REGION_CURRENT) ? dms->cur_region : id;

	if (id & DM_STATS_WALK_GROUP) {
		if (id == DM_STATS_WALK_GROUP)
			id = dms->cur_group;
		else
			id &= ~DM_STATS_WALK_GROUP;
	}

	region = &dms->regions[id];
	if (!_stats_region_is_grouped(dms, id)
	    || !dms->groups[region->group_id].alias)
		return dms->name;

	return dms->groups[region->group_id].alias;
}

const char *dm_stats_get_current_region_program_id(const struct dm_stats *dms)
{
	return dm_stats_get_region_program_id(dms, dms->cur_region);
}

const char *dm_stats_get_current_region_aux_data(const struct dm_stats *dms)
{
	return dm_stats_get_region_aux_data(dms, dms->cur_region);
}

int dm_stats_get_region_precise_timestamps(const struct dm_stats *dms,
					   uint64_t region_id)
{
	struct dm_stats_region *region;

	if (region_id == DM_STATS_REGION_CURRENT)
		region_id = dms->cur_region;

	if (region_id == DM_STATS_WALK_GROUP)
		region_id = dms->cur_group;
	else if (region_id & DM_STATS_WALK_GROUP)
		region_id &= ~DM_STATS_WALK_GROUP;

	region = &dms->regions[region_id];
	return region->timescale == 1;
}

int dm_stats_get_current_region_precise_timestamps(const struct dm_stats *dms)
{
	return dm_stats_get_region_precise_timestamps(dms,
						      DM_STATS_REGION_CURRENT);
}

/*
 * Histogram access methods.
 */

static void _sum_histogram_bins(const struct dm_stats *dms,
				struct dm_histogram *dmh_aggr,
				uint64_t region_id, uint64_t area_id)
{
	struct dm_stats_region *region;
	struct dm_histogram_bin *bins;
	struct dm_histogram *dmh_cur;
	int bin;

	region = &dms->regions[region_id];
	dmh_cur = region->counters[area_id].histogram;
	bins = dmh_aggr->bins;

	for (bin = 0; bin < dmh_aggr->nr_bins; bin++)
		bins[bin].count += dmh_cur->bins[bin].count;
}

/*
 * Create an aggregate histogram for a sub-divided region or a group.
 */
static struct dm_histogram *_aggregate_histogram(const struct dm_stats *dms,
						 uint64_t region_id,
						 uint64_t area_id)
{
	struct dm_histogram *dmh_aggr, *dmh_cur, **dmh_cachep;
	uint64_t group_id = DM_STATS_GROUP_NOT_PRESENT;
	int bin, nr_bins, group = 1;
	size_t hist_size;

	if (area_id == DM_STATS_WALK_REGION) {
		/* region aggregation */
		group = 0;
		if (!_stats_region_present(&dms->regions[region_id]))
			return_NULL;

		if (!dms->regions[region_id].bounds)
			return_NULL;

		if (!dms->regions[region_id].counters)
			return dms->regions[region_id].bounds;

		if (dms->regions[region_id].histogram)
			return dms->regions[region_id].histogram;

		dmh_cur = dms->regions[region_id].counters[0].histogram;
		dmh_cachep = &dms->regions[region_id].histogram;
		nr_bins = dms->regions[region_id].bounds->nr_bins;
	} else {
		/* group aggregation */
		group_id = region_id;
		area_id = DM_STATS_WALK_GROUP;
		if (!_stats_group_id_present(dms, group_id))
			return_NULL;

		if (!dms->regions[group_id].bounds)
			return_NULL;

		if (!dms->regions[group_id].counters)
			return dms->regions[group_id].bounds;

		if (dms->groups[group_id].histogram)
			return dms->groups[group_id].histogram;

		dmh_cur = dms->regions[group_id].counters[0].histogram;
		dmh_cachep = &dms->groups[group_id].histogram;
		nr_bins = dms->regions[group_id].bounds->nr_bins;
	}

	hist_size = sizeof(*dmh_aggr)
		     + nr_bins * sizeof(struct dm_histogram_bin);

	if (!(dmh_aggr = dm_pool_zalloc(dms->hist_mem, hist_size))) {
		log_error("Could not allocate group histogram");
		return 0;
	}

	dmh_aggr->nr_bins = dmh_cur->nr_bins;
	dmh_aggr->dms = dms;

	if (!group)
		_foreach_region_area(dms, region_id, area_id) {
			_sum_histogram_bins(dms, dmh_aggr, region_id, area_id);
		}
	else {
		_foreach_group_area(dms, group_id, region_id, area_id) {
			_sum_histogram_bins(dms, dmh_aggr, region_id, area_id);
		}
	}

	for (bin = 0; bin < nr_bins; bin++) {
		dmh_aggr->sum += dmh_aggr->bins[bin].count;
		dmh_aggr->bins[bin].upper = dmh_cur->bins[bin].upper;
	}

	/* cache aggregate histogram for subsequent access */
	*dmh_cachep = dmh_aggr;

	return dmh_aggr;
}

struct dm_histogram *dm_stats_get_histogram(const struct dm_stats *dms,
					    uint64_t region_id,
					    uint64_t area_id)
{
	int aggr = 0;

	if (region_id == DM_STATS_REGION_CURRENT) {
		region_id = dms->cur_region;
		if (region_id & DM_STATS_WALK_GROUP) {
			region_id = dms->cur_group;
			aggr = 1;
		}
	} else if (region_id & DM_STATS_WALK_GROUP) {
		region_id &= ~DM_STATS_WALK_GROUP;
		aggr = 1;
	}

	area_id = (area_id == DM_STATS_AREA_CURRENT)
		   ? dms->cur_area : area_id ;

	if (area_id == DM_STATS_WALK_REGION)
		aggr = 1;

	if (aggr)
		return _aggregate_histogram(dms, region_id, area_id);

	if (region_id & DM_STATS_WALK_REGION)
		region_id &= ~DM_STATS_WALK_REGION;

	if (!dms->regions[region_id].counters)
		return dms->regions[region_id].bounds;

	return dms->regions[region_id].counters[area_id].histogram;
}

int dm_histogram_get_nr_bins(const struct dm_histogram *dmh)
{
	return dmh->nr_bins;
}

uint64_t dm_histogram_get_bin_lower(const struct dm_histogram *dmh, int bin)
{
	return (!bin) ? 0 : dmh->bins[bin - 1].upper;
}

uint64_t dm_histogram_get_bin_upper(const struct dm_histogram *dmh, int bin)
{
	return dmh->bins[bin].upper;
}

uint64_t dm_histogram_get_bin_width(const struct dm_histogram *dmh, int bin)
{
	uint64_t upper, lower;
	upper = dm_histogram_get_bin_upper(dmh, bin);
	lower = dm_histogram_get_bin_lower(dmh, bin);
	return (upper - lower);
}

uint64_t dm_histogram_get_bin_count(const struct dm_histogram *dmh, int bin)
{
	return dmh->bins[bin].count;
}

uint64_t dm_histogram_get_sum(const struct dm_histogram *dmh)
{
	return dmh->sum;
}

dm_percent_t dm_histogram_get_bin_percent(const struct dm_histogram *dmh,
					  int bin)
{
	uint64_t value = dm_histogram_get_bin_count(dmh, bin);
	uint64_t width = dm_histogram_get_bin_width(dmh, bin);
	uint64_t total = dm_histogram_get_sum(dmh);

	double val = (double) value;

	if (!total || !value || !width)
		return DM_PERCENT_0;

	return dm_make_percent((uint64_t) val, total);
}

/*
 * Histogram string helper functions: used to construct histogram and
 * bin boundary strings from numeric data.
 */

/*
 * Allocate an unbound histogram object with nr_bins bins. Only used
 * for histograms used to hold bounds values as arguments for calls to
 * dm_stats_create_region().
 */
static struct dm_histogram *_alloc_dm_histogram(int nr_bins)
{
	/* Allocate space for dm_histogram + nr_entries. */
	size_t size = sizeof(struct dm_histogram) +
		(unsigned) nr_bins * sizeof(struct dm_histogram_bin);
	return dm_zalloc(size);
}

/*
 * Parse a histogram bounds string supplied by the user. The string
 * consists of a list of numbers, "n1,n2,n3,..." with optional 'ns',
 * 'us', 'ms', or 's' unit suffixes.
 *
 * The scale parameter indicates the timescale used for this region: one
 * for nanoscale resolution and NSEC_PER_MSEC for miliseconds.
 *
 * On return bounds contains a pointer to an array of uint64_t
 * histogram bounds values expressed in units of nanoseconds.
 */
struct dm_histogram *dm_histogram_bounds_from_string(const char *bounds_str)
{
	static const char _valid_chars[] = "0123456789,muns";
	uint64_t this_val = 0, mult = 1;
	const char *c, *v, *val_start;
	struct dm_histogram_bin *cur;
	struct dm_histogram *dmh;
	int nr_entries = 1;
	char *endptr;

	c = bounds_str;

	/* Count number of bounds entries. */
	while(*c)
		if (*(c++) == ',')
			nr_entries++;

	c = bounds_str;

	if (!(dmh = _alloc_dm_histogram(nr_entries)))
		return_0;

	dmh->nr_bins = nr_entries;

	cur = dmh->bins;

	do {
		for (v = _valid_chars; *v; v++)
			if (*c == *v)
				break;

		if (!*v) {
			stack;
			goto badchar;
		}

		if (*c == ',') {
			log_error("Empty histogram bin not allowed: %s",
				  bounds_str);
			goto bad;
		} else {
			val_start = c;
			endptr = NULL;

			this_val = strtoull(val_start, &endptr, 10);
			if (!endptr) {
				log_error("Could not parse histogram bound.");
				goto bad;
			}
			c = endptr; /* Advance to units, comma, or end. */

			if (*c == 's') {
				mult = NSEC_PER_SEC;
				c++; /* Advance over 's'. */
			} else if (*(c + 1) == 's') {
				if (*c == 'm')
					mult = NSEC_PER_MSEC;
				else if (*c == 'u')
					mult = NSEC_PER_USEC;
				else if (*c == 'n')
					mult = 1;
				else {
					stack;
					goto badchar;
				}
				c += 2; /* Advance over 'ms', 'us', or 'ns'. */
			} else if (*c == ',')
				c++;
			else if (*c) { /* Expected ',' or NULL. */
				stack;
				goto badchar;
			}

			if (*c == ',')
				c++;
			this_val *= mult;
			(cur++)->upper = this_val;
		}
	} while (*c);

	/* Bounds histograms have no owner. */
	dmh->dms = NULL;
	dmh->region = NULL;

	return dmh;

badchar:
	log_error("Invalid character in histogram: %c", *c);
bad:
	dm_free(dmh);
	return NULL;
}

struct dm_histogram *dm_histogram_bounds_from_uint64(const uint64_t *bounds)
{
	const uint64_t *entry = bounds;
	struct dm_histogram_bin *cur;
	struct dm_histogram *dmh;
	int nr_entries = 1;

	if (!bounds || !bounds[0]) {
		log_error("Could not parse empty histogram bounds array");
		return 0;
	}

	/* Count number of bounds entries. */
	while(*entry)
		if (*(++entry))
			nr_entries++;

	entry = bounds;

	if (!(dmh = _alloc_dm_histogram(nr_entries)))
		return_0;

	dmh->nr_bins = nr_entries;

	cur = dmh->bins;

	while (*entry)
		(cur++)->upper = *(entry++);

	/* Bounds histograms have no owner. */
	dmh->dms = NULL;
	dmh->region = NULL;

	return dmh;
}

void dm_histogram_bounds_destroy(struct dm_histogram *bounds)
{
	if (!bounds)
		return;

	/* Bounds histograms are not bound to any handle or region. */
	if (bounds->dms || bounds->region) {
		log_error("Freeing invalid histogram bounds pointer %p.",
			  (void *) bounds);
		stack;
	}
	/* dm_free() expects a (void *). */
	dm_free((void *) bounds);
}

/*
 * Scale a bounds value down from nanoseconds to the largest possible
 * whole unit suffix.
 */
static void _scale_bound_value_to_suffix(uint64_t *bound, const char **suffix)
{
	*suffix = "ns";
	if (!(*bound % NSEC_PER_SEC)) {
		*bound /= NSEC_PER_SEC;
		*suffix = "s";
	} else if (!(*bound % NSEC_PER_MSEC)) {
		*bound /= NSEC_PER_MSEC;
		*suffix = "ms";
	} else if (!(*bound % NSEC_PER_USEC)) {
		*bound /= NSEC_PER_USEC;
		*suffix = "us";
	}
}

#define DM_HISTOGRAM_BOUNDS_MASK 0x30
#define BOUNDS_LEN 64

static int _make_bounds_string(char *buf, size_t size, uint64_t lower,
			       uint64_t upper, int flags, int width)
{
	char bound_buf[BOUNDS_LEN];
	const char *l_suff = NULL;
	const char *u_suff = NULL;
	const char *sep = "";
	int bounds = flags & DM_HISTOGRAM_BOUNDS_MASK;

	if (!bounds)
		return_0;

	*buf = '\0';

	if (flags & DM_HISTOGRAM_SUFFIX) {
		_scale_bound_value_to_suffix(&lower, &l_suff);
		_scale_bound_value_to_suffix(&upper, &u_suff);
	} else
		l_suff = u_suff = "";

	if (flags & DM_HISTOGRAM_VALUES)
		sep = ":";

	if (bounds > DM_HISTOGRAM_BOUNDS_LOWER) {
		/* Handle infinite uppermost bound. */
		if (upper == UINT64_MAX) {
			if (dm_snprintf(bound_buf, sizeof(bound_buf),
					 ">" FMTu64 "%s", lower, l_suff) < 0)
				goto_out;
			/* Only display an 'upper' string for final bin. */
			bounds = DM_HISTOGRAM_BOUNDS_UPPER;
		} else {
			if (dm_snprintf(bound_buf, sizeof(bound_buf),
					 FMTu64 "%s", upper, u_suff) < 0)
				goto_out;
		}
	} else if (bounds == DM_HISTOGRAM_BOUNDS_LOWER) {
		if ((dm_snprintf(bound_buf, sizeof(bound_buf), FMTu64 "%s",
				 lower, l_suff)) < 0)
			goto_out;
	}

	switch (bounds) {
	case DM_HISTOGRAM_BOUNDS_LOWER:
	case DM_HISTOGRAM_BOUNDS_UPPER:
		return dm_snprintf(buf, size, "%*s%s", width, bound_buf, sep);
	case DM_HISTOGRAM_BOUNDS_RANGE:
		return dm_snprintf(buf, size,  FMTu64 "%s-%s%s",
				   lower, l_suff, bound_buf, sep);
	}
out:
	return 0;
}

#define BOUND_WIDTH_NOSUFFIX 10 /* 999999999 nsecs */
#define BOUND_WIDTH 6 /* bounds string up to 9999xs */
#define COUNT_WIDTH 6 /* count string: up to 9999 */
#define PERCENT_WIDTH 6 /* percent string : 0.00-100.00% */
#define DM_HISTOGRAM_VALUES_MASK 0x06

const char *dm_histogram_to_string(const struct dm_histogram *dmh, int bin,
				   int width, int flags)
{
	char buf[BOUNDS_LEN], bounds_buf[BOUNDS_LEN];
	int minwidth, bounds, values, start, last;
	uint64_t lower, upper, val_u64; /* bounds of the current bin. */
	/* Use the histogram pool for string building. */
	struct dm_pool *mem = dmh->dms->hist_mem;
	const char *sep = "";
	int bounds_width;
	ssize_t len = 0;
	float val_flt;

	bounds = flags & DM_HISTOGRAM_BOUNDS_MASK;
	values = flags & DM_HISTOGRAM_VALUES;

	if (bin < 0) {
		start = 0;
		last = dmh->nr_bins - 1;
	} else
		start = last = bin;

	minwidth = width;

	if (width < 0 || !values)
		width = minwidth = 0; /* no padding */
	else if (flags & DM_HISTOGRAM_PERCENT)
		width = minwidth = (width) ? : PERCENT_WIDTH;
	else if (flags & DM_HISTOGRAM_VALUES)
		width = minwidth = (width) ? : COUNT_WIDTH;

	if (values && !width)
		sep = ":";

	/* Set bounds string to the empty string. */
	bounds_buf[0] = '\0';

	if (!dm_pool_begin_object(mem, 64))
		return_0;

	for (bin = start; bin <= last; bin++) {
		if (bounds) {
			/* Default bounds width depends on time suffixes. */
			bounds_width = (!(flags & DM_HISTOGRAM_SUFFIX))
					? BOUND_WIDTH_NOSUFFIX
					: BOUND_WIDTH ;

			bounds_width = (!width) ? width : bounds_width;

			lower = dm_histogram_get_bin_lower(dmh, bin);
			upper = dm_histogram_get_bin_upper(dmh, bin);

			len = sizeof(bounds_buf);
			len = _make_bounds_string(bounds_buf, len,
						  lower, upper, flags,
						  bounds_width);
			/*
			 * Comma separates "bounds: value" pairs unless
			 * --noheadings is used.
			 */
			sep = (width || !values) ? "," : ":";

			/* Adjust width by real bounds length if set. */
			width -= (width) ? (len - (bounds_width + 1)) : 0;

			/* -ve width indicates specified width was overrun. */
			width = (width > 0) ? width : 0;
		}

		if (bin == last)
			sep = "";

		if (flags & DM_HISTOGRAM_PERCENT) {
			dm_percent_t pr;
			pr = dm_histogram_get_bin_percent(dmh, bin);
			val_flt = dm_percent_to_float(pr);
			len = dm_snprintf(buf, sizeof(buf), "%s%*.2f%%%s",
					  bounds_buf, width, val_flt, sep);
		} else if (values) {
			val_u64 = dmh->bins[bin].count;
			len = dm_snprintf(buf, sizeof(buf), "%s%*"PRIu64"%s",
					  bounds_buf, width, val_u64, sep);
		} else if (bounds)
			len = dm_snprintf(buf, sizeof(buf), "%s%s", bounds_buf,
					  sep);
		else {
			*buf = '\0';
			len = 0;
		}

		if (len < 0)
			goto_bad;

		width = minwidth; /* re-set histogram column width. */
		if (!dm_pool_grow_object(mem, buf, (size_t) len))
			goto_bad;
	}

	if (!dm_pool_grow_object(mem, "\0", 1))
		goto_bad;

	return (const char *) dm_pool_end_object(mem);

bad:
	dm_pool_abandon_object(mem);
	return NULL;
}

/*
 * A lightweight representation of an extent (region, area, file
 * system block or extent etc.). A table of extents can be used
 * to sort and to efficiently find holes or overlaps among a set
 * of tuples of the form (id, start, len).
 */
struct _extent {
	struct dm_list list;
	uint64_t id;
	uint64_t start;
	uint64_t len;
};

/* last address in an extent */
#define _extent_end(a) ((a)->start + (a)->len - 1)

/* a and b must be sorted by increasing start sector */
#define _extents_overlap(a, b) (_extent_end(a) > (b)->start)

/*
 * Comparison function to sort extents in ascending start order.
 */
static int _extent_start_compare(const void *p1, const void *p2)
{
	const struct _extent *r1, *r2;
	r1 = (const struct _extent *) p1;
	r2 = (const struct _extent *) p2;

	if (r1->start < r2->start)
		return -1;
	else if (r1->start == r2->start)
		return 0;
	return 1;
}

static int _stats_create_group(struct dm_stats *dms, dm_bitset_t regions,
			       const char *alias, uint64_t *group_id)
{
	struct dm_stats_group *group;
	*group_id = dm_bit_get_first(regions);

	/* group has no regions? */
	if (*group_id == DM_STATS_GROUP_NOT_PRESENT)
		return_0;

	group = &dms->groups[*group_id];

	if (group->regions) {
		log_error(INTERNAL_ERROR "Unexpected group state while"
			  "creating group ID bitmap" FMTu64, *group_id);
		return 0;
	}

	group->group_id = *group_id;
	group->regions = regions;

	if (alias)
		group->alias = dm_strdup(alias);
	else
		group->alias = NULL;

	/* force an update of the group tag stored in aux_data */
	if (!_stats_set_aux(dms, *group_id, dms->regions[*group_id].aux_data))
		return 0;

	return 1;
}

static int _stats_group_check_overlap(const struct dm_stats *dms,
				      dm_bitset_t regions, int count)
{
	struct dm_list ext_list = DM_LIST_HEAD_INIT(ext_list);
	struct _extent *ext, *tmp, *next, *map = NULL;
	size_t map_size = (dms->max_region + 1) * sizeof(*map);
	int i = 0, id, overlap, merged;

	map = dm_pool_alloc(dms->mem, map_size);
	if (!map) {
		log_error("Could not allocate memory for region map");
		return 0;
	}

	/* build a table of extents in order of region_id */
	for (id = dm_bit_get_first(regions); id >= 0;
	     id = dm_bit_get_next(regions, id)) {
		dm_list_init(&map[i].list);
		map[i].id = id;
		map[i].start = dms->regions[id].start;
		map[i].len = dms->regions[id].len;
		i++;
	}

	/* A single region cannot overlap itself. */
	if (i == 1) {
		dm_pool_free(dms->mem, map);
		return 1;
	}

	/* sort by extent.start */
	qsort(map, count, sizeof(*map), _extent_start_compare);

	for (i = 0; i < count; i++)
		dm_list_add(&ext_list, &map[i].list);

	overlap = 0;
merge:
	merged = 0;
	dm_list_iterate_items_safe(ext, tmp, &ext_list) {
		next = dm_list_item(dm_list_next(&ext_list, &ext->list),
				    struct _extent);
		if (!next)
			continue;

		if (_extents_overlap(ext, next)) {
			log_warn("WARNING: region IDs " FMTu64 " and "
				 FMTu64 " overlap. Some events will be "
				 "counted twice.", ext->id, next->id);
			/* merge larger extent into smaller */
			if (_extent_end(ext) > _extent_end(next)) {
				next->id = ext->id;
				next->len = ext->len;
			}
			if (ext->start < next->start)
				next->start = ext->start;
			dm_list_del(&ext->list);
			overlap = merged = 1;
		}
	}
	/* continue until no merge candidates remain */
	if (merged)
		goto merge;

	dm_pool_free(dms->mem, map);
	return (overlap == 0);
}

static void _stats_copy_histogram_bounds(struct dm_histogram *to,
					 struct dm_histogram *from)
{
	int i;

	to->nr_bins = from->nr_bins;

	for (i = 0; i < to->nr_bins; i++)
		to->bins[i].upper = from->bins[i].upper;
}

/*
 * Compare histogram bounds h1 and h2, and return 1 if they match (i.e.
 * have the same number of bins and identical bin boundary values), or 0
 * otherwise.
 */
static int _stats_check_histogram_bounds(struct dm_histogram *h1,
					 struct dm_histogram *h2)
{
	int i;

	if (!h1 || !h2)
		return 0;

	if (h1->nr_bins != h2->nr_bins)
		return 0;

	for (i = 0; i < h1->nr_bins; i++)
		if (h1->bins[i].upper != h2->bins[i].upper)
			return 0;
	return 1;
}

/*
 * Create a new group in stats handle dms from the group description
 * passed in group.
 */
int dm_stats_create_group(struct dm_stats *dms, const char *members,
			  const char *alias, uint64_t *group_id)
{
	struct dm_histogram *check = NULL, *bounds;
	int i, count = 0, precise = 0;
	dm_bitset_t regions;

	if (!dms->regions || !dms->groups) {
		log_error("Could not create group: no regions found.");
		return 0;
	};

	if (!(regions = dm_bitset_parse_list(members, NULL, 0))) {
		log_error("Could not parse list: '%s'", members);
		return 0;
	}

	if (!(check = dm_pool_zalloc(dms->hist_mem, sizeof(*check)))) {
		log_error("Could not allocate memory for bounds check");
		goto bad;
	}

	/* too many bits? */
	if ((*regions - 1) > dms->max_region) {
		log_error("Invalid region ID: %d", *regions - 1);
		goto bad;
	}

	/*
	 * Check that each region_id in the bitmap meets the group
	 * constraints: present, not already grouped, and if any
	 * histogram is present that they all have the same bounds.
	 */
	for (i = dm_bit_get_first(regions); i >= 0;
	     i = dm_bit_get_next(regions, i)) {
		if (!dm_stats_region_present(dms, i)) {
			log_error("Region ID %d does not exist", i);
			goto bad;
		}
		if (_stats_region_is_grouped(dms, i)) {
			log_error("Region ID %d already a member of group ID "
				  FMTu64, i, dms->regions[i].group_id);
			goto bad;
		}
		if (dms->regions[i].timescale == 1)
			precise++;

		/* check for matching histogram bounds */
		bounds = dms->regions[i].bounds;
		if (bounds && !check->nr_bins)
			_stats_copy_histogram_bounds(check, bounds);
		else if (bounds) {
			if (!_stats_check_histogram_bounds(check, bounds)) {
				log_error("All region histogram bounds "
					  "must match exactly");
				goto bad;
			}
		}
		count++;
	}

	if (precise && (precise != count))
		log_warn("WARNING: Grouping regions with different clock resolution: "
			 "precision may be lost.");

	if (!_stats_group_check_overlap(dms, regions, count))
		log_very_verbose("Creating group with overlapping regions.");

	if (!_stats_create_group(dms, regions, alias, group_id))
		goto bad;

	dm_pool_free(dms->hist_mem, check);
	return 1;

bad:
	dm_pool_free(dms->hist_mem, check);
	dm_bitset_destroy(regions);
	return 0;
}

/*
 * Remove the specified group_id.
 */
int dm_stats_delete_group(struct dm_stats *dms, uint64_t group_id,
			  int remove_regions)
{
	struct dm_stats_region *leader;
	dm_bitset_t regions;
	uint64_t i;

	if (group_id > dms->max_region) {
		log_error("Invalid group ID: " FMTu64, group_id);
		return 0;
	}

	if (!_stats_group_id_present(dms, group_id)) {
		log_error("Group ID " FMTu64 " does not exist", group_id);
		return 0;
	}

	regions = dms->groups[group_id].regions;
	leader = &dms->regions[group_id];

	/* delete all but the group leader */
	for (i = (*regions - 1); i > leader->region_id; i--) {
		if (dm_bit(regions, i)) {
			dm_bit_clear(regions, i);
			if (remove_regions && !dm_stats_delete_region(dms, i))
				log_warn("WARNING: Failed to delete region "
					 FMTu64 " on %s.", i, dms->name);
		}
	}

	/* clear group and mark as not present */
	_stats_clear_group_regions(dms, group_id);
	_stats_group_destroy(&dms->groups[group_id]);

	/* delete leader or clear aux_data */
	if (remove_regions)
		return dm_stats_delete_region(dms, group_id);
	else if (!_stats_set_aux(dms, group_id, leader->aux_data))
		return 0;

	return 1;
}

uint64_t dm_stats_get_group_id(const struct dm_stats *dms, uint64_t region_id)
{
	region_id = (region_id == DM_STATS_REGION_CURRENT)
		     ? dms->cur_region : region_id;

	if (region_id & DM_STATS_WALK_GROUP) {
		if (region_id == DM_STATS_WALK_GROUP)
			return dms->cur_group;
		else
			return region_id & ~DM_STATS_WALK_GROUP;
	}

	if (region_id & DM_STATS_WALK_REGION)
		region_id &= ~DM_STATS_WALK_REGION;

	return dms->regions[region_id].group_id;
}

int dm_stats_get_group_descriptor(const struct dm_stats *dms,
				  uint64_t group_id, char **buf)
{
	dm_bitset_t regions = dms->groups[group_id].regions;
	size_t buflen;

	buflen = _stats_group_tag_len(dms, regions);

	*buf = dm_pool_alloc(dms->mem, buflen);
	if (!*buf) {
		log_error("Could not allocate memory for regions string");
		return 0;
	}

	if (!_stats_group_tag_fill(dms, regions, *buf, buflen))
		return 0;

	return 1;
}

#ifdef HAVE_LINUX_FIEMAP_H
/*
 * Resize the group bitmap corresponding to group_id so that it can
 * contain at least num_regions members.
 */
static int _stats_resize_group(struct dm_stats_group *group,
			       uint64_t num_regions)
{
	uint64_t last_bit = dm_bit_get_last(group->regions);
	dm_bitset_t new, old;

	if (last_bit >= num_regions) {
		log_error("Cannot resize group bitmap to " FMTu64
			  " with bit " FMTu64 " set.", num_regions, last_bit);
		return 0;
	}

	log_very_verbose("Resizing group bitmap from " FMTu32 " to " FMTu64
			 " (last_bit: " FMTu64 ").", group->regions[0],
			 num_regions, last_bit);

	new = dm_bitset_create(NULL, (unsigned) num_regions);
	if (!new) {
		log_error("Could not allocate memory for new group bitmap.");
		return 0;
	}

	old = group->regions;
	dm_bit_copy(new, old);
	group->regions = new;
	dm_bitset_destroy(old);
	return 1;
}

/*
 * Group a table of region_ids corresponding to the extents of a file.
 */
static int _stats_group_file_regions(struct dm_stats *dms, uint64_t *region_ids,
				     uint64_t count, const char *alias)
{
	dm_bitset_t regions = dm_bitset_create(NULL, dms->nr_regions);
	uint64_t i, group_id = DM_STATS_GROUP_NOT_PRESENT;
	char *members = NULL;
	size_t buflen;

	if (!regions) {
		log_error("Cannot map file: failed to allocate group bitmap.");
		return 0;
	}

	for (i = 0; i < count; i++)
		dm_bit_set(regions, region_ids[i]);

	buflen = _stats_group_tag_len(dms, regions);
	members = dm_malloc(buflen);

	if (!members) {
		log_error("Cannot map file: failed to allocate group "
			  "descriptor.");
		dm_bitset_destroy(regions);
		return 0;
	}

	if (!_stats_group_tag_fill(dms, regions, members, buflen))
		goto bad;

	/*
	 * overlaps should not be possible: overlapping file extents
	 * returned by FIEMAP imply a kernel bug or a corrupt fs.
	 */
	if (!_stats_group_check_overlap(dms, regions, count))
		log_very_verbose("Creating group with overlapping regions.");

	if (!_stats_create_group(dms, regions, alias, &group_id))
		goto bad;

	dm_free(members);
	return 1;
bad:
	dm_bitset_destroy(regions);
	dm_free(members);
	return 0;
}

static int _stats_add_file_extent(int fd, struct dm_pool *mem, uint64_t id,
				  struct fiemap_extent *fm_ext)
{
	struct _extent extent;

	/* final address of list is unknown */
	memset(&extent.list, 0, sizeof(extent.list));

	/* convert bytes to dm (512b) sectors */
	extent.start = fm_ext->fe_physical >> SECTOR_SHIFT;
	extent.len = fm_ext->fe_length >> SECTOR_SHIFT;
	extent.id = id;

	log_very_verbose("Extent " FMTu64 " on fd %d at " FMTu64 "+"
			 FMTu64, extent.id, fd, extent.start, extent.len);

	if (!dm_pool_grow_object(mem, &extent,
				 sizeof(extent))) {
		log_error("Cannot map file: failed to grow extent map.");
		return 0;
	}
	return 1;
}

/* test for the boundary of an extent */
#define ext_boundary(ext, exp)		\
((ext).fe_logical != 0) &&		\
((ext).fe_physical != (exp))

/*
 * Copy fields from fiemap_extent 'from' to the fiemap_extent
 * pointed to by 'to'.
 */
#define ext_copy(to, from)	\
do {				\
	*(to) = *(from);	\
} while (0)

static uint64_t _stats_map_extents(int fd, struct dm_pool *mem,
				   struct fiemap *fiemap,
				   struct fiemap_extent *fm_ext,
				   struct fiemap_extent *fm_last,
				   struct fiemap_extent *fm_pending,
				   uint64_t next_extent,
				   int *eof)
{
	uint64_t expected = 0, nr_extents = next_extent;
	unsigned int i;

	/*
	 * Loop over the returned extents adding the fm_pending extent
	 * to the table of extents each time a discontinuity (or eof)
	 * is detected.
	 *
	 * We use a pointer to fm_pending in the caller since it is
	 * possible that logical extents comprising a single physical
	 * extent are returned by successive FIEMAP calls.
	 */
	for (i = 0; i < fiemap->fm_mapped_extents; i++) {
		expected = fm_last->fe_physical + fm_last->fe_length;

		if (fm_ext[i].fe_flags & FIEMAP_EXTENT_LAST)
			*eof = 1;

		/* cannot map extents that are not yet allocated. */
		if (fm_ext[i].fe_flags
		    & (FIEMAP_EXTENT_UNKNOWN | FIEMAP_EXTENT_DELALLOC))
			continue;

		/*
		 * Begin a new extent if the current physical address differs
		 * from the expected address yielded by fm_last.fe_physical +
		 * fm_last.fe_length.
		 *
		 * A logical discontinuity is seen at the start of the file if
		 * unwritten space exists before the first extent: do not add
		 * any extent record until we have accumulated a non-zero length
		 * in fm_pending.
		 */
		if (fm_pending->fe_length &&
		    ext_boundary(fm_ext[i], expected)) {
			if (!_stats_add_file_extent(fd, mem, nr_extents,
						    fm_pending))
				goto_bad;
			nr_extents++;
			/* Begin a new pending extent. */
			ext_copy(fm_pending, fm_ext + i);
		} else {
			expected = 0;
			/* Begin a new pending extent for extent 0. If there is
			 * a hole at the start of the file, the first allocated
			 * extent will have a non-zero fe_logical. Detect this
			 * case by testing fm_pending->fe_length: if no length
			 * has been accumulated we are handling the first
			 * physical extent of the file.
			 */
			if (!fm_pending->fe_length || fm_ext[i].fe_logical == 0)
				ext_copy(fm_pending, fm_ext + i);
			else
				/* accumulate this logical extent's length */
				fm_pending->fe_length += fm_ext[i].fe_length;
		}
		*fm_last = fm_ext[i];
	}

	/*
	 * If the file only has a single extent, no boundary is ever
	 * detected to trigger addition of the first extent.
	 */
	if (*eof || (fm_ext[i - 1].fe_logical == 0)) {
		_stats_add_file_extent(fd, mem, nr_extents, fm_pending);
		nr_extents++;
	}

	fiemap->fm_start = (fm_ext[i - 1].fe_logical +
			    fm_ext[i - 1].fe_length);

	/* return the number of extents found in this call. */
	return nr_extents - next_extent;
bad:
	/* signal mapping error to caller */
	*eof = -1;
	return 0;
}

/*
 * Read the extents of an open file descriptor into a table of struct _extent.
 *
 * Based on e2fsprogs/misc/filefrag.c::filefrag_fiemap().
 *
 * Copyright 2003 by Theodore Ts'o.
 *
 */
static struct _extent *_stats_get_extents_for_file(struct dm_pool *mem, int fd,
						   uint64_t *count)
{
	struct fiemap_extent fm_last = {0}, fm_pending = {0}, *fm_ext = NULL;
	struct fiemap *fiemap = NULL;
	int eof = 0, nr_extents = 0;
	struct _extent *extents;
	unsigned long flags = 0;
	uint64_t *buf;

	/* grow temporary extent table in the pool */
	if (!dm_pool_begin_object(mem, sizeof(*extents)))
		return NULL;

	buf = dm_zalloc(STATS_FIE_BUF_LEN);
	if (!buf) {
		log_error("Could not allocate memory for FIEMAP buffer.");
		goto bad;
	}

	/* initialise pointers into the ioctl buffer. */
	fiemap = (struct fiemap *) buf;
	fm_ext = &fiemap->fm_extents[0];

	/* space available per ioctl */
	*count = (STATS_FIE_BUF_LEN - sizeof(*fiemap))
		  / sizeof(struct fiemap_extent);

	flags = FIEMAP_FLAG_SYNC;

	do {
		/* start of ioctl loop - zero size and set count to bufsize */
		fiemap->fm_length = ~0ULL;
		fiemap->fm_flags = flags;
		fiemap->fm_extent_count = *count;

		/* get count-sized chunk of extents */
		if (ioctl(fd, FS_IOC_FIEMAP, (unsigned long) fiemap) < 0) {
			if (errno == EBADR)
				log_err_once("FIEMAP failed with unknown "
					     "flags %x.", fiemap->fm_flags);
			goto bad;
		}

		/* If 0 extents are returned, more ioctls are not needed */
		if (fiemap->fm_mapped_extents == 0)
			break;

		nr_extents += _stats_map_extents(fd, mem, fiemap, fm_ext,
						 &fm_last, &fm_pending,
						 nr_extents, &eof);

		/* check for extent mapping error */
		if (eof < 0)
			goto bad;

	} while (eof == 0);

	if (!nr_extents) {
		log_error("Cannot map file: no allocated extents.");
		goto bad;
	}

	/* return total number of extents */
	*count = nr_extents;
	extents = dm_pool_end_object(mem);

	/* free FIEMAP buffer. */
	dm_free(buf);

	return extents;

bad:
	*count = 0;
	dm_pool_abandon_object(mem);
	dm_free(buf);
	return NULL;
}

#define MATCH_EXTENT(e, s, l) \
(((e).start == (s)) && ((e).len == (l)))

static struct _extent *_find_extent(uint64_t nr_extents, struct _extent *extents,
				    uint64_t start, uint64_t len)
{
	size_t i;
	for (i = 0; i < nr_extents; i++)
		if (MATCH_EXTENT(extents[i], start, len))
			return extents + i;
	return NULL;
}

/*
 * Clean up a table of region_id values that were created during a
 * failed dm_stats_create_regions_from_fd, or dm_stats_update_regions_from_fd
 * operation.
 */
static void _stats_cleanup_region_ids(struct dm_stats *dms, uint64_t *regions,
				      uint64_t nr_regions)
{
	uint64_t i;

	for (i = 0; i < nr_regions; i++)
		if (!_stats_delete_region(dms, regions[i]))
			log_error("Could not delete region " FMTu64 ".", i);
}

/*
 * First update pass: prune no-longer-allocated extents from the group
 * and build a table of the remaining extents so that their creation
 * can be skipped in the second pass.
 */
static int _stats_unmap_regions(struct dm_stats *dms, uint64_t group_id,
				struct dm_pool *mem, struct _extent *extents,
				struct _extent **old_extents, uint64_t *count,
				int *regroup)
{
	struct dm_stats_region *region = NULL;
	struct dm_stats_group *group = NULL;
	uint64_t nr_kept, nr_old;
	struct _extent ext;
	int64_t i;

	group = &dms->groups[group_id];

	log_very_verbose("Checking for changed file extents in group ID "
			 FMTu64, group_id);

	if (!dm_pool_begin_object(mem, sizeof(**old_extents))) {
		log_error("Could not allocate extent table.");
		return 0;
	}

	nr_kept = nr_old = 0; /* counts of old and retained extents */

	/*
	 * First pass: delete de-allocated extents and set regroup=1 if
	 * deleting the current group leader.
	 */
	i = dm_bit_get_last(group->regions);
	for (; i >= 0; i = dm_bit_get_prev(group->regions, i)) {
		region = &dms->regions[i];
		nr_old++;

		if (extents && _find_extent(*count, extents,
				  region->start, region->len)) {
			ext.start = region->start;
			ext.len = region->len;
			ext.id = i;
			nr_kept++;

			if (!dm_pool_grow_object(mem, &ext, sizeof(ext)))
				goto out;

			log_very_verbose("Kept region " FMTu64, i);
		} else {

			if (i == group_id)
				*regroup = 1;

			if (!_stats_delete_region(dms, i)) {
				log_error("Could not remove region ID " FMTu64,
					  i);
				goto out;
			}

			log_very_verbose("Deleted region " FMTu64, i);
		}
	}

	*old_extents = dm_pool_end_object(mem);
	if (!*old_extents) {
		log_error("Could not finalize region extent table.");
		goto out;
	}
	log_very_verbose("Kept " FMTd64 " of " FMTd64 " old extents",
			 nr_kept, nr_old);
	log_very_verbose("Found " FMTu64 " new extents",
			 *count - nr_kept);

	return (int) nr_kept;
out:
	dm_pool_abandon_object(mem);
	return -1;
}

/*
 * Create or update a set of regions representing the extents of a file
 * and return a table of uint64_t region_id values. The number of regions
 * created is returned in the memory pointed to by count (which must be
 * non-NULL).
 *
 * If group_id is not equal to DM_STATS_GROUP_NOT_PRESENT, it is assumed
 * that group_id corresponds to a group containing existing regions that
 * were mapped to this file at an earlier time: regions will be added or
 * removed to reflect the current status of the file.
 */
static uint64_t *_stats_map_file_regions(struct dm_stats *dms, int fd,
					 struct dm_histogram *bounds,
					 int precise, uint64_t group_id,
					 uint64_t *count, int *regroup)
{
	struct _extent *extents = NULL, *old_extents = NULL;
	uint64_t *regions = NULL, fail_region, i, num_bits;
	struct dm_stats_group *group = NULL;
	struct dm_pool *extent_mem = NULL;
	struct _extent *old_ext;
	char *hist_arg = NULL;
	struct statfs fsbuf;
	int64_t nr_kept = 0;
	struct stat buf;
	int update;

	*count = 0;
	update = _stats_group_id_present(dms, group_id);

#ifdef BTRFS_SUPER_MAGIC
	if (fstatfs(fd, &fsbuf)) {
		log_error("fstatfs failed for fd %d", fd);
		return 0;
	}

	if (fsbuf.f_type == BTRFS_SUPER_MAGIC) {
		log_error("Cannot map file: btrfs does not provide "
			  "physical FIEMAP extent data.");
		return 0;
	}
#endif

	if (fstat(fd, &buf)) {
		log_error("fstat failed for fd %d", fd);
		return 0;
	}

	if (!(buf.st_mode & S_IFREG)) {
		log_error("Not a regular file");
		return 0;
	}

	if (!dm_is_dm_major(major(buf.st_dev))) {
		log_error("Cannot map file: not a device-mapper device.");
		return 0;
	}

	/*
	 * If regroup is set here, we are creating a new filemap: otherwise
	 * we are updating a group with a valid group identifier in group_id.
	 */
	if (update)
		log_very_verbose("Updating extents from fd %d with group ID "
				 FMTu64 " on (%d:%d)", fd, group_id,
				 major(buf.st_dev), minor(buf.st_dev));
	else
		log_very_verbose("Mapping extents from fd %d on (%d:%d)",
				 fd, major(buf.st_dev), minor(buf.st_dev));

	/* Use a temporary, private pool for the extent table. This avoids
         * hijacking the dms->mem (region table) pool which would lead to
         * interleaving temporary allocations with dm_stats_list() data,
         * causing complications in the error path.
         */
	if (!(extent_mem = dm_pool_create("extents", sizeof(*extents))))
		return_NULL;

	if (!(extents = _stats_get_extents_for_file(extent_mem, fd, count))) {
		log_very_verbose("No extents found in fd %d", fd);
		if (!update)
			goto out;
	}

	if (update) {
		group = &dms->groups[group_id];
		if ((nr_kept = _stats_unmap_regions(dms, group_id, extent_mem,
						     extents, &old_extents,
						     count, regroup)) < 0)
			goto_out;
	}

        if (bounds)
                if (!(hist_arg = _build_histogram_arg(bounds, &precise)))
                        goto_out;

	/* make space for end-of-table marker */
	if (!(regions = dm_malloc((1 + *count) * sizeof(*regions)))) {
		log_error("Could not allocate memory for region IDs.");
		goto_out;
	}

	/*
	 * Second pass (first for non-update case): create regions for
	 * all extents not retained from the prior mapping, and insert
	 * retained regions into the table of region_id values.
	 *
	 * If a regroup is not scheduled, set group bits for newly
	 * created regions in the group leader bitmap.
	 */
	for (i = 0; i < *count; i++) {
		if (update) {
			if ((old_ext = _find_extent((uint64_t) nr_kept,
						    old_extents,
						    extents[i].start,
						    extents[i].len))) {
				regions[i] = old_ext->id;
				continue;
			}
		}
		if (!_stats_create_region(dms, regions + i, extents[i].start,
					  extents[i].len, -1, precise, hist_arg,
					  dms->program_id, "")) {
			log_error("Failed to create region " FMTu64 " of "
				  FMTu64 " at " FMTu64 ".", i, *count,
				  extents[i].start);
			goto out_remove;
		}

		log_very_verbose("Created new region mapping " FMTu64 "+" FMTu64
				 " with region ID " FMTu64, extents[i].start,
				 extents[i].len, regions[i]);

		if (!*regroup && update) {
			/* expand group bitmap */
			if (regions[i] > (group->regions[0] - 1)) {
				num_bits = regions[i] + *count;
				if (!_stats_resize_group(group, num_bits)) {
					log_error("Failed to resize group "
						  "bitmap.");
					goto out_remove;
				}
			}
			dm_bit_set(group->regions, regions[i]);
		}

	}
	regions[*count] = DM_STATS_REGION_NOT_PRESENT;

	/* Update group leader aux_data for new group members. */
	if (!*regroup && update)
		if (!_stats_set_aux(dms, group_id,
				    dms->regions[group_id].aux_data))
			log_error("Failed to update group aux_data.");

	if (bounds)
		dm_free(hist_arg);

	/* the extent table will be empty if the file has been truncated. */
	if (extents)
		dm_pool_free(extent_mem, extents);

	dm_pool_destroy(extent_mem);

	return regions;

out_remove:
	/* New region creation may begin to fail part-way through creating
	 * a set of file mapped regions: in this case we need to roll back
	 * the regions that were already created and return the handle to
	 * a consistent state. A listed handle is required for this: use a
	 * single list operation and call _stats_delete_region() directly
	 * to avoid a @stats_list ioctl and list parsing for each region.
	 */
	if (!dm_stats_list(dms, NULL))
		goto out;

	fail_region = i;
	_stats_cleanup_region_ids(dms, regions, fail_region);
	*count = 0;

out:
	dm_pool_destroy(extent_mem);
	dm_free(hist_arg);
	dm_free(regions);
	return NULL;
}

uint64_t *dm_stats_create_regions_from_fd(struct dm_stats *dms, int fd,
					  int group, int precise,
					  struct dm_histogram *bounds,
					  const char *alias)
{
	uint64_t *regions, count;
	int regroup = 1;

	if (alias && !group) {
		log_error("Cannot set alias without grouping regions.");
		return NULL;
	}

	if (!(regions = _stats_map_file_regions(dms, fd, bounds, precise,
						DM_STATS_GROUP_NOT_PRESENT,
						&count, &regroup)))
		return NULL;

	if (!group)
		return regions;

	/* refresh handle */
	if (!dm_stats_list(dms, NULL))
		goto_out;

	if (!_stats_group_file_regions(dms, regions, count, alias))
		goto_out;

	return regions;
out:
	_stats_cleanup_region_ids(dms, regions, count);
	dm_free(regions);
	return NULL;
}

uint64_t *dm_stats_update_regions_from_fd(struct dm_stats *dms, int fd,
					  uint64_t group_id)
{
	struct dm_histogram *bounds = NULL;
	int nr_bins, precise, regroup;
	uint64_t *regions, count = 0;
	const char *alias = NULL;

	if (!dms->regions || !dm_stats_group_present(dms, group_id)) {
		if (!dm_stats_list(dms, dms->program_id)) {
			log_error("Could not obtain region list while "
				  "updating group " FMTu64 ".", group_id);
			return NULL;
		}
	}

	if (!dm_stats_group_present(dms, group_id)) {
		log_error("Group ID " FMTu64 " does not exist.", group_id);
		return NULL;
	}

	/*
	 * If the extent corresponding to the group leader's region has been
	 * deallocated, _stats_map_file_regions() will remove the region and
	 * the group. In this case, regroup will be set by the call and the
	 * group will be re-created using saved values.
	 */
	regroup = 0;

	/*
	 * A copy of the alias is needed to re-create the group when regroup=1.
	 */
	if (dms->groups[group_id].alias) {
		alias = dm_strdup(dms->groups[group_id].alias);
		if (!alias) {
			log_error("Failed to allocate group alias string.");
			return NULL;
		}
	}

	if (dms->regions[group_id].bounds) {
		/*
		 * A copy of the histogram bounds must be passed to
		 * _stats_map_file_regions() to be used when creating new
		 * regions: it is not safe to use the copy in the current group
		 * leader since it may be destroyed during the first group
		 * update pass.
		 */
		nr_bins = dms->regions[group_id].bounds->nr_bins;
		bounds = _alloc_dm_histogram(nr_bins);
		if (!bounds) {
			log_error("Could not allocate memory for group "
				  "histogram bounds.");
			goto out;
		}
		_stats_copy_histogram_bounds(bounds,
					     dms->regions[group_id].bounds);
	}

	precise = (dms->regions[group_id].timescale == 1);

	regions = _stats_map_file_regions(dms, fd, bounds, precise,
					  group_id, &count, &regroup);

	if (!regions)
		goto bad;

	if (!dm_stats_list(dms, NULL))
		goto bad;

	/* regroup if there are regions to group */
	if (regroup && (*regions != DM_STATS_REGION_NOT_PRESENT))
		if (!_stats_group_file_regions(dms, regions, count, alias))
			goto bad;

	dm_free(bounds);
	dm_free((char *) alias);
	return regions;
bad:
	_stats_cleanup_region_ids(dms, regions, count);
	dm_free(bounds);
	dm_free(regions);
out:
	dm_free((char *) alias);
	return NULL;
}
#else /* !HAVE_LINUX_FIEMAP */
uint64_t *dm_stats_create_regions_from_fd(struct dm_stats *dms, int fd,
					  int group, int precise,
					  struct dm_histogram *bounds,
					  const char *alias)
{
	log_error("File mapping requires FIEMAP ioctl support.");
	return 0;
}

uint64_t *dm_stats_update_regions_from_fd(struct dm_stats *dms, int fd,
					  uint64_t group_id)
{
	log_error("File mapping requires FIEMAP ioctl support.");
	return 0;
}
#endif /* HAVE_LINUX_FIEMAP */

#ifdef DMFILEMAPD
static const char *_filemapd_mode_names[] = {
	"inode",
	"path",
	NULL
};

dm_filemapd_mode_t dm_filemapd_mode_from_string(const char *mode_str)
{
	dm_filemapd_mode_t mode = DM_FILEMAPD_FOLLOW_INODE;
	const char **mode_name;

	if (mode_str) {
		for (mode_name = _filemapd_mode_names; *mode_name; mode_name++)
			if (!strcmp(*mode_name, mode_str))
				break;
		if (*mode_name)
			mode = DM_FILEMAPD_FOLLOW_INODE
				+ (mode_name - _filemapd_mode_names);
		else {
			log_error("Could not parse dmfilemapd mode: %s",
				  mode_str);
			return DM_FILEMAPD_FOLLOW_NONE;
		}
	}
	return mode;
}

#define DM_FILEMAPD "dmfilemapd"
#define NR_FILEMAPD_ARGS 7 /* includes argv[0] */
/*
 * Start dmfilemapd to monitor the specified file descriptor, and to
 * update the group given by 'group_id' when the file's allocation
 * changes.
 *
 * usage: dmfilemapd <fd> <group_id> <mode> [<foreground>[<log_level>]]
 */
int dm_stats_start_filemapd(int fd, uint64_t group_id, const char *path,
			    dm_filemapd_mode_t mode, unsigned foreground,
			    unsigned verbose)
{
	char fd_str[8], group_str[8], fg_str[2], verb_str[2];
	const char *mode_str = _filemapd_mode_names[mode];
	char *args[NR_FILEMAPD_ARGS + 1];
	pid_t pid = 0;
	int argc = 0;

	if (fd < 0) {
		log_error("dmfilemapd file descriptor must be "
			  "non-negative: %d", fd);
		return 0;
	}

	if (path[0] != '/') {
		log_error("Path argument must specify an absolute path.");
		return 0;
	}

	if (mode > DM_FILEMAPD_FOLLOW_PATH) {
		log_error("Invalid dmfilemapd mode argument: "
			  "Must be DM_FILEMAPD_FOLLOW_INODE or "
			  "DM_FILEMAPD_FOLLOW_PATH");
		return 0;
	}

	if (foreground > 1) {
		log_error("Invalid dmfilemapd foreground argument. "
			  "Must be 0 or 1: %d.", foreground);
		return 0;
	}

	if (verbose > 3) {
		log_error("Invalid dmfilemapd verbose argument. "
			  "Must be 0..3: %d.", verbose);
		return 0;
	}

	/* set argv[0] */
	args[argc++] = (char *) DM_FILEMAPD;

	/* set <fd> */
	if ((dm_snprintf(fd_str, sizeof(fd_str), "%d", fd)) < 0) {
		log_error("Could not format fd argument.");
		return 0;
	}
	args[argc++] = fd_str;

	/* set <group_id> */
	if ((dm_snprintf(group_str, sizeof(group_str), FMTu64, group_id)) < 0) {
		log_error("Could not format group_id argument.");
		return 0;
	}
	args[argc++] = group_str;

	/* set <path> */
	args[argc++] = (char *) path;

	/* set <mode> */
	args[argc++] = (char *) mode_str;

	/* set <foreground> */
	if ((dm_snprintf(fg_str, sizeof(fg_str), "%u", foreground)) < 0) {
		log_error("Could not format foreground argument.");
		return 0;
	}
	args[argc++] = fg_str;

	/* set <verbose> */
	if ((dm_snprintf(verb_str, sizeof(verb_str), "%u", verbose)) < 0) {
		log_error("Could not format verbose argument.");
		return 0;
	}
	args[argc++] = verb_str;

	/* terminate args[argc] */
	args[argc] = NULL;

	log_very_verbose("Spawning daemon as '%s %d " FMTu64 " %s %s %u %u'",
			 *args, fd, group_id, path, mode_str,
			 foreground, verbose);

	if (!foreground && ((pid = fork()) < 0)) {
		log_error("Failed to fork dmfilemapd process.");
		return 0;
	}

	if (pid > 0) {
		log_very_verbose("Forked dmfilemapd process as pid %d", pid);
		return 1;
	}

	execvp(args[0], args);
	log_sys_error("execvp", args[0]);
	if (!foreground)
		_exit(127);
	return 0;
}
# else /* !DMFILEMAPD */
dm_filemapd_mode_t dm_filemapd_mode_from_string(const char *mode_str)
{
	return 0;
};

int dm_stats_start_filemapd(int fd, uint64_t group_id, const char *path,
			    dm_filemapd_mode_t mode, unsigned foreground,
			    unsigned verbose)
{
	log_error("dmfilemapd support disabled.");
	return 0;
}
#endif /* DMFILEMAPD */

/*
 * Backward compatible dm_stats_create_region() implementations.
 *
 * Keep these at the end of the file to avoid adding clutter around the
 * current dm_stats_create_region() version.
 */

#if defined(__GNUC__)
int dm_stats_create_region_v1_02_106(struct dm_stats *dms, uint64_t *region_id,
				     uint64_t start, uint64_t len, int64_t step,
				     int precise, const char *program_id,
				     const char *aux_data);
int dm_stats_create_region_v1_02_106(struct dm_stats *dms, uint64_t *region_id,
				     uint64_t start, uint64_t len, int64_t step,
				     int precise, const char *program_id,
				     const char *aux_data)
{
	/* 1.02.106 lacks histogram argument. */
	return _stats_create_region(dms, region_id, start, len, step, precise,
				    NULL, program_id, aux_data);
}
DM_EXPORT_SYMBOL(dm_stats_create_region, 1_02_106);

int dm_stats_create_region_v1_02_104(struct dm_stats *dms, uint64_t *region_id,
				     uint64_t start, uint64_t len, int64_t step,
				     const char *program_id, const char *aux_data);
int dm_stats_create_region_v1_02_104(struct dm_stats *dms, uint64_t *region_id,
				     uint64_t start, uint64_t len, int64_t step,
				     const char *program_id, const char *aux_data)
{
	/* 1.02.104 lacks histogram and precise arguments. */
	return _stats_create_region(dms, region_id, start, len, step, 0, NULL,
				    program_id, aux_data);
}
DM_EXPORT_SYMBOL(dm_stats_create_region, 1_02_104);
#endif
