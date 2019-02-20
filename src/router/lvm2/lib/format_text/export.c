/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2017 Red Hat, Inc. All rights reserved.
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

#include "base/memory/zalloc.h"
#include "lib/misc/lib.h"
#include "import-export.h"
#include "lvm-version.h"
#include "lib/metadata/metadata.h"
#include "lib/display/display.h"
#include "lib/misc/lvm-string.h"
#include "lib/metadata/segtype.h"
#include "lib/format_text/text_export.h"
#include "lib/commands/toolcontext.h"
#include "libdaemon/client/config-util.h"

#include <stdarg.h>
#include <time.h>
#include <sys/utsname.h>

struct formatter;
__attribute__((format(printf, 3, 0)))
typedef int (*out_with_comment_fn) (struct formatter * f, const char *comment,
				    const char *fmt, va_list ap);
typedef int (*nl_fn) (struct formatter * f);

/*
 * Macro for formatted output.
 * out_with_comment_fn returns -1 if data didn't fit and buffer was expanded.
 * Then argument list is reset and out_with_comment_fn is called again.
 */
#define _out_with_comment(f, buffer, fmt, ap) \
	do { \
		va_start(ap, fmt); \
		r = (f)->out_with_comment((f), (buffer), (fmt), ap); \
		va_end(ap); \
	} while (r == -1)

/*
 * The first half of this file deals with
 * exporting the vg, ie. writing it to a file.
 */
struct formatter {
	struct dm_pool *mem;	/* pv names allocated from here */
	struct dm_hash_table *pv_names;	/* dev_name -> pv_name (eg, pv1) */

	union {
		FILE *fp;	/* where we're writing to */
		struct {
			char *start;
			uint32_t size;
			uint32_t used;
		} buf;
	} data;

	out_with_comment_fn out_with_comment;
	nl_fn nl;

	int indent;		/* current level of indentation */
	int error;
	int header;		/* 1 => comments at start; 0 => end */
};

static struct utsname _utsname;

static void _init(void)
{
	static int _initialised = 0;

	if (_initialised)
		return;

	if (uname(&_utsname)) {
		log_error("uname failed: %s", strerror(errno));
		memset(&_utsname, 0, sizeof(_utsname));
	}

	_initialised = 1;
}

/*
 * Formatting functions.
 */

#define MAX_INDENT 5
static void _inc_indent(struct formatter *f)
{
	if (++f->indent > MAX_INDENT)
		f->indent = MAX_INDENT;
}

static void _dec_indent(struct formatter *f)
{
	if (!f->indent--) {
		log_error(INTERNAL_ERROR "problem tracking indentation");
		f->indent = 0;
	}
}

/*
 * Newline function for prettier layout.
 */
static int _nl_file(struct formatter *f)
{
	fprintf(f->data.fp, "\n");

	return 1;
}

static int _extend_buffer(struct formatter *f)
{
	char *newbuf;

	log_debug_metadata("Doubling metadata output buffer to " FMTu32,
			   f->data.buf.size * 2);
	if (!(newbuf = realloc(f->data.buf.start,
				   f->data.buf.size * 2))) {
		log_error("Buffer reallocation failed.");
		return 0;
	}
	f->data.buf.start = newbuf;
	f->data.buf.size *= 2;

	return 1;
}

static int _nl_raw(struct formatter *f)
{
	/* If metadata doesn't fit, extend buffer */
	if ((f->data.buf.used + 2 > f->data.buf.size) &&
	    (!_extend_buffer(f)))
		return_0;

	*(f->data.buf.start + f->data.buf.used) = '\n';
	f->data.buf.used += 1;

	*(f->data.buf.start + f->data.buf.used) = '\0';

	return 1;
}

#define COMMENT_TAB 6
__attribute__((format(printf, 3, 0)))
static int _out_with_comment_file(struct formatter *f, const char *comment,
				  const char *fmt, va_list ap)
{
	int i;
	char white_space[MAX_INDENT + 1];

	if (ferror(f->data.fp))
		return 0;

	for (i = 0; i < f->indent; i++)
		white_space[i] = '\t';
	white_space[i] = '\0';
	fputs(white_space, f->data.fp);
	i = vfprintf(f->data.fp, fmt, ap);

	if (comment) {
		/*
		 * line comments up if possible.
		 */
		i += 8 * f->indent;
		i /= 8;
		i++;

		do
			fputc('\t', f->data.fp);

		while (++i < COMMENT_TAB);

		fputs(comment, f->data.fp);
	}
	fputc('\n', f->data.fp);

	return 1;
}

__attribute__((format(printf, 3, 0)))
static int _out_with_comment_raw(struct formatter *f,
				 const char *comment __attribute__((unused)),
				 const char *fmt, va_list ap)
{
	int n;
	va_list apc;

	va_copy(apc, ap);
	n = vsnprintf(f->data.buf.start + f->data.buf.used,
		      f->data.buf.size - f->data.buf.used, fmt, apc);
	va_end(apc);

	/* If metadata doesn't fit, extend buffer */
	if (n < 0 || (n + f->data.buf.used + 2 > f->data.buf.size)) {
		if (!_extend_buffer(f))
			return_0;
		return -1; /* Retry */
	}

	f->data.buf.used += n;

	outnl(f);

	return 1;
}

/*
 * Formats a string, converting a size specified
 * in 512-byte sectors to a more human readable
 * form (eg, megabytes).  We may want to lift this
 * for other code to use.
 */
static int _sectors_to_units(uint64_t sectors, char *buffer, size_t s)
{
	static const char *_units[] = {
		"Kilobytes",
		"Megabytes",
		"Gigabytes",
		"Terabytes",
		"Petabytes",
		"Exabytes",
		NULL
	};

	int i;
	double d = (double) sectors;

	/* to convert to K */
	d /= 2.0;

	for (i = 0; (d > 1024.0) && _units[i]; i++)
		d /= 1024.0;

	return dm_snprintf(buffer, s, "# %g %s", d, _units[i]) > 0;
}

/* increment indention level */
void out_inc_indent(struct formatter *f)
{
	_inc_indent(f);
}

/* decrement indention level */
void out_dec_indent(struct formatter *f)
{
	_dec_indent(f);
}

/* insert new line */
int out_newline(struct formatter *f)
{
	return f->nl(f);
}

/*
 * Appends a comment giving a size in more easily
 * readable form (eg, 4M instead of 8096).
 */
int out_size(struct formatter *f, uint64_t size, const char *fmt, ...)
{
	char buffer[64];
	va_list ap;
	int r;

	if (!_sectors_to_units(size, buffer, sizeof(buffer)))
		return 0;

	_out_with_comment(f, buffer, fmt, ap);

	return r;
}

/*
 * Appends a comment indicating that the line is
 * only a hint.
 */
int out_hint(struct formatter *f, const char *fmt, ...)
{
	va_list ap;
	int r;

	_out_with_comment(f, "# Hint only", fmt, ap);

	return r;
}

/*
 * The normal output function with comment
 */
int out_text_with_comment(struct formatter *f, const char *comment, const char *fmt, ...)
{
	va_list ap;
	int r;

	_out_with_comment(f, comment, fmt, ap);

	return r;
}

/*
 * The normal output function.
 */
int out_text(struct formatter *f, const char *fmt, ...)
{
	va_list ap;
	int r;

	_out_with_comment(f, NULL, fmt, ap);

	return r;
}

static int _out_line(const char *line, void *_f) {
	struct formatter *f = (struct formatter *) _f;
	return out_text(f, "%s", line);
}

int out_config_node(struct formatter *f, const struct dm_config_node *cn)
{
	return dm_config_write_node(cn, _out_line, f);
}

static int _print_header(struct cmd_context *cmd, struct formatter *f,
			 const char *desc)
{
	char *buf;
	time_t t;

	t = time(NULL);

	outf(f, "# Generated by LVM2 version %s: %s", LVM_VERSION, ctime(&t));
	outf(f, CONTENTS_FIELD " = \"" CONTENTS_VALUE "\"");
	outf(f, FORMAT_VERSION_FIELD " = %d", FORMAT_VERSION_VALUE);
	outnl(f);

	buf = alloca(dm_escaped_len(desc));
	outf(f, "description = \"%s\"", dm_escape_double_quotes(buf, desc));
	outnl(f);
	outf(f, "creation_host = \"%s\"\t# %s %s %s %s %s", _utsname.nodename,
	     _utsname.sysname, _utsname.nodename, _utsname.release,
	     _utsname.version, _utsname.machine);
	if (cmd->system_id && *cmd->system_id)
		outf(f, "creation_host_system_id = \"%s\"", cmd->system_id);
	outf(f, "creation_time = " FMTu64 "\t# %s", (uint64_t)t, ctime(&t));

	return 1;
}

static int _print_flag_config(struct formatter *f, uint64_t status, int type)
{
	char buffer[4096];

	if (!print_flags(buffer, sizeof(buffer), type, STATUS_FLAG, status))
		return_0;
	outf(f, "status = %s", buffer);

	if (!print_flags(buffer, sizeof(buffer), type, COMPATIBLE_FLAG, status))
		return_0;
	outf(f, "flags = %s", buffer);

	return 1;
}

static char *_alloc_printed_str_list(struct dm_list *list)
{
	struct dm_str_list *sl;
	int first = 1;
	size_t size = 0;
	char *buffer, *buf;

	dm_list_iterate_items(sl, list)
		/* '"' + item + '"' + ',' + ' ' */
		size += strlen(sl->str) + 4;
	/* '[' + ']' + '\0' */
	size += 3;

	if (!(buffer = buf = malloc(size))) {
		log_error("Could not allocate memory for string list buffer.");
		return NULL;
	}

	if (!emit_to_buffer(&buf, &size, "["))
		goto_bad;

	dm_list_iterate_items(sl, list) {
		if (!first) {
			if (!emit_to_buffer(&buf, &size, ", "))
				goto_bad;
		} else
			first = 0;

		if (!emit_to_buffer(&buf, &size, "\"%s\"", sl->str))
			goto_bad;
	}

	if (!emit_to_buffer(&buf, &size, "]"))
		goto_bad;

	return buffer;

bad:
	free(buffer);
	return_NULL;
}

static int _out_list(struct formatter *f, struct dm_list *list,
		     const char *list_name)
{
	char *buffer;

	if (!dm_list_empty(list)) {
		if (!(buffer = _alloc_printed_str_list(list)))
			return_0;
		if (!out_text(f, "%s = %s", list_name, buffer)) {
			free(buffer);
			return_0;
		}
		free(buffer);
	}

	return 1;
}

static int _print_vg(struct formatter *f, struct volume_group *vg)
{
	char buffer[4096];
	const struct format_type *fmt = NULL;
	uint64_t status = vg->status;

	if (!id_write_format(&vg->id, buffer, sizeof(buffer)))
		return_0;

	outf(f, "id = \"%s\"", buffer);

	outf(f, "seqno = %u", vg->seqno);

	if (vg->original_fmt)
		fmt = vg->original_fmt;
	else if (vg->fid)
		fmt = vg->fid->fmt;
	if (fmt)
		outfc(f, "# informational", "format = \"%s\"", fmt->name);

	/*
	 * Removing WRITE and adding LVM_WRITE_LOCKED makes it read-only
	 * to old versions of lvm that only look for LVM_WRITE.
	 */
	if ((status & LVM_WRITE) && vg_flag_write_locked(vg)) {
		status &= ~LVM_WRITE;
		status |= LVM_WRITE_LOCKED;
	}

	if (!_print_flag_config(f, status, VG_FLAGS))
		return_0;

	if (!_out_list(f, &vg->tags, "tags"))
		return_0;
 
	if (vg->system_id && *vg->system_id)
		outf(f, "system_id = \"%s\"", vg->system_id);

	if (vg->lock_type) {
		outf(f, "lock_type = \"%s\"", vg->lock_type);
		if (vg->lock_args)
			outf(f, "lock_args = \"%s\"", vg->lock_args);
	}

	outsize(f, (uint64_t) vg->extent_size, "extent_size = %u",
		vg->extent_size);
	outf(f, "max_lv = %u", vg->max_lv);
	outf(f, "max_pv = %u", vg->max_pv);

	/* Default policy is NORMAL; INHERIT is meaningless */
	if (vg->alloc != ALLOC_NORMAL && vg->alloc != ALLOC_INHERIT) {
		outnl(f);
		outf(f, "allocation_policy = \"%s\"",
		     get_alloc_string(vg->alloc));
	}

	if (vg->profile)
		outf(f, "profile = \"%s\"", vg->profile->name);

	outf(f, "metadata_copies = %u", vg->mda_copies);

	return 1;
}

/*
 * Get the pv%d name from the formatters hash
 * table.
 */
static const char *_get_pv_name_from_uuid(struct formatter *f, char *uuid)
{
	const char *pv_name = dm_hash_lookup(f->pv_names, uuid);

	if (!pv_name)
		log_error(INTERNAL_ERROR "PV name for uuid %s missing from text metadata export hash table.",
			  uuid);

	return pv_name;
}

static const char *_get_pv_name(struct formatter *f, struct physical_volume *pv)
{
	char uuid[64] __attribute__((aligned(8)));

	if (!pv || !id_write_format(&pv->id, uuid, sizeof(uuid)))
		return_NULL;

	return _get_pv_name_from_uuid(f, uuid);
}

static int _print_pvs(struct formatter *f, struct volume_group *vg)
{
	struct pv_list *pvl;
	struct physical_volume *pv;
	char buffer[PATH_MAX * 2];
	const char *name;

	outf(f, "physical_volumes {");
	_inc_indent(f);

	dm_list_iterate_items(pvl, &vg->pvs) {
		pv = pvl->pv;

		if (!id_write_format(&pv->id, buffer, sizeof(buffer)))
			return_0;

		if (!(name = _get_pv_name_from_uuid(f, buffer)))
			return_0;

		outnl(f);
		outf(f, "%s {", name);
		_inc_indent(f);

		outf(f, "id = \"%s\"", buffer);

		if (strlen(pv_dev_name(pv)) >= PATH_MAX) {
			log_error("pv device name size is out of bounds.");
			return 0;
		}

		outhint(f, "device = \"%s\"",
			dm_escape_double_quotes(buffer, pv_dev_name(pv)));
		outnl(f);

		if (!_print_flag_config(f, pv->status, PV_FLAGS))
			return_0;

		if (!_out_list(f, &pv->tags, "tags"))
			return_0;

		outsize(f, pv->size, "dev_size = " FMTu64, pv->size);

		outf(f, "pe_start = " FMTu64, pv->pe_start);
		outsize(f, vg->extent_size * (uint64_t) pv->pe_count,
			"pe_count = %u", pv->pe_count);

		if (pv->ba_start && pv->ba_size) {
			outf(f, "ba_start = " FMTu64, pv->ba_start);
			outsize(f, pv->ba_size, "ba_size = " FMTu64, pv->ba_size);
		}

		_dec_indent(f);
		outf(f, "}");
	}

	_dec_indent(f);
	outf(f, "}");
	return 1;
}

static int _print_segment(struct formatter *f, struct volume_group *vg,
			  int count, struct lv_segment *seg)
{
	char buffer[2048];

	if (!print_segtype_lvflags(buffer, sizeof(buffer), seg->lv->status))
		return_0;

	outf(f, "segment%u {", count);
	_inc_indent(f);

	outf(f, "start_extent = %u", seg->le);
	outsize(f, (uint64_t) seg->len * vg->extent_size,
		"extent_count = %u", seg->len);
	outnl(f);
	if (seg->reshape_len)
		outsize(f, (uint64_t) seg->reshape_len * vg->extent_size,
			"reshape_count = %u", seg->reshape_len);

	outf(f, "type = \"%s%s\"", seg->segtype->name, buffer);

	if (!_out_list(f, &seg->tags, "tags"))
		return_0;

	if (seg->segtype->ops->text_export &&
	    !seg->segtype->ops->text_export(seg, f))
		return_0;

	_dec_indent(f);
	outf(f, "}");

	return 1;
}

int out_areas(struct formatter *f, const struct lv_segment *seg,
	      const char *type)
{
	const char *name;
	unsigned int s;
	struct physical_volume *pv;

	outnl(f);

	outf(f, "%ss = [", type);
	_inc_indent(f);

	for (s = 0; s < seg->area_count; s++) {
		switch (seg_type(seg, s)) {
		case AREA_PV:
			if (!(pv = seg_pv(seg, s))) {
				log_error(INTERNAL_ERROR "Missing PV for area " FMTu32 " of %s segment of LV %s.",
					  s, type, display_lvname(seg->lv));
				return 0;
			}
				
			if (!(name = _get_pv_name(f, pv)))
				return_0;

			outf(f, "\"%s\", %u%s", name,
			     seg_pe(seg, s),
			     (s == seg->area_count - 1) ? "" : ",");
			break;
		case AREA_LV:
			/* FIXME This helper code should be target-independent! Check for metadata LV property. */
			if (!seg_is_raid(seg)) {
				outf(f, "\"%s\", %u%s",
				     seg_lv(seg, s)->name,
				     seg_le(seg, s),
				     (s == seg->area_count - 1) ? "" : ",");
				continue;
			}

			/* RAID devices are laid-out in metadata/data pairs */
			/* FIXME Validation should be elsewhere, not here! */
			if (!lv_is_raid_image(seg_lv(seg, s)) ||
			    (seg->meta_areas && seg_metalv(seg, s) && !lv_is_raid_metadata(seg_metalv(seg, s)))) {
				log_error("RAID segment has non-RAID areas");
				return 0;
			}

			if (seg->meta_areas && seg_metalv(seg,s))
				outf(f, "\"%s\", \"%s\"%s",
				     (seg->meta_areas && seg_metalv(seg, s)) ? seg_metalv(seg, s)->name : "",
				     seg_lv(seg, s)->name, (s == seg->area_count - 1) ? "" : ",");
			else
				outf(f, "\"%s\"%s", seg_lv(seg, s)->name, (s == seg->area_count - 1) ? "" : ",");

			break;
		case AREA_UNASSIGNED:
			log_error(INTERNAL_ERROR "Invalid type for area " FMTu32 " of %s segment of LV %s.",
				  s, type, display_lvname(seg->lv));
			return 0;
		}
	}

	_dec_indent(f);
	outf(f, "]");
	return 1;
}

static int _print_timestamp(struct formatter *f,
			     const char *name, time_t ts,
			     char *buf, size_t buf_size)
{
	struct tm *local_tm;

	if (ts) {
		strncpy(buf, "# ", buf_size);
		if (!(local_tm = localtime(&ts)) ||
		    !strftime(buf + 2, buf_size - 2,
			      "%Y-%m-%d %T %z", local_tm))
			buf[0] = 0;

		outfc(f, buf, "%s = " FMTu64, name, (uint64_t) ts);
	}

	return 1;
}

static int _print_lv(struct formatter *f, struct logical_volume *lv)
{
	struct lv_segment *seg;
	char buffer[4096];
	int seg_count;
	uint64_t status = lv->status;

	outnl(f);
	outf(f, "%s {", lv->name);
	_inc_indent(f);

	/* FIXME: Write full lvid */
	if (!id_write_format(&lv->lvid.id[1], buffer, sizeof(buffer)))
		return_0;

	outf(f, "id = \"%s\"", buffer);

	/*
	 * Removing WRITE and adding LVM_WRITE_LOCKED makes it read-only
	 * to old versions of lvm that only look for LVM_WRITE.
	 */
	if ((status & LVM_WRITE) && vg_flag_write_locked(lv->vg)) {
		status &= ~LVM_WRITE;
		status |= LVM_WRITE_LOCKED;
	}

	if (!_print_flag_config(f, status, LV_FLAGS))
		return_0;

	if (!_out_list(f, &lv->tags, "tags"))
		return_0;

	if (lv->timestamp) {
		if (!_print_timestamp(f, "creation_time", lv->timestamp,
				      buffer, sizeof(buffer)))
			return_0;
		outf(f, "creation_host = \"%s\"", lv->hostname);
	}

	if (lv->lock_args)
		outf(f, "lock_args = \"%s\"", lv->lock_args);

	if (lv->alloc != ALLOC_INHERIT)
		outf(f, "allocation_policy = \"%s\"",
		     get_alloc_string(lv->alloc));

	if (lv->profile)
		outf(f, "profile = \"%s\"", lv->profile->name);

	switch (lv->read_ahead) {
	case DM_READ_AHEAD_NONE:
		outfc(f, "# None", "read_ahead = -1");
		break;
	case DM_READ_AHEAD_AUTO:
		/* No output - use default */
		break;
	default:
		outf(f, "read_ahead = %u", lv->read_ahead);
	}

	if (lv->major >= 0)
		outf(f, "major = %d", lv->major);
	if (lv->minor >= 0)
		outf(f, "minor = %d", lv->minor);
	outf(f, "segment_count = %u", dm_list_size(&lv->segments));
	outnl(f);

	seg_count = 1;
	dm_list_iterate_items(seg, &lv->segments) {
		if (!_print_segment(f, lv->vg, seg_count++, seg))
			return_0;
	}

	_dec_indent(f);
	outf(f, "}");

	return 1;
}

static int _print_lvs(struct formatter *f, struct volume_group *vg)
{
	struct lv_list *lvl;

	/*
	 * Don't bother with an lv section if there are no lvs.
	 */
	if (dm_list_empty(&vg->lvs))
		return 1;

	outf(f, "logical_volumes {");
	_inc_indent(f);

	/*
	 * Write visible LVs first
	 */
	dm_list_iterate_items(lvl, &vg->lvs) {
		if (!(lv_is_visible(lvl->lv)))
			continue;
		if (!_print_lv(f, lvl->lv))
			return_0;
	}

	dm_list_iterate_items(lvl, &vg->lvs) {
		if ((lv_is_visible(lvl->lv)))
			continue;
		if (!_print_lv(f, lvl->lv))
			return_0;
	}

	_dec_indent(f);
	outf(f, "}");

	return 1;
}

static int _alloc_printed_indirect_descendants(struct dm_list *indirect_glvs, char **buffer)
{
	struct glv_list *user_glvl;
	size_t buf_size = 0;
	int first = 1;
	char *buf;

	*buffer = NULL;

	dm_list_iterate_items(user_glvl, indirect_glvs) {
		if (user_glvl->glv->is_historical)
			continue;
		/* '"' + name + '"' + ',' + ' ' */
		buf_size += strlen(user_glvl->glv->live->name) + 4;
	}

	if (!buf_size)
		return 1;

	/* '[' + ']' + '\0' */
	buf_size += 3;

	if (!(*buffer = malloc(buf_size))) {
		log_error("Could not allocate memory for ancestor list buffer.");
		return 0;
	}
	buf = *buffer;

	if (!emit_to_buffer(&buf, &buf_size, "["))
		goto_bad;

	dm_list_iterate_items(user_glvl, indirect_glvs) {
		if (user_glvl->glv->is_historical)
			continue;
		if (!first) {
			if (!emit_to_buffer(&buf, &buf_size, ", "))
				goto_bad;
		} else
			first = 0;

		if (!emit_to_buffer(&buf, &buf_size, "\"%s\"", user_glvl->glv->live->name))
			goto_bad;
	}

	if (!emit_to_buffer(&buf, &buf_size, "]"))
		goto_bad;

	return 1;
bad:
	if (*buffer) {
		free(*buffer);
		*buffer = NULL;
	}
	return 0;
}

static int _print_historical_lv(struct formatter *f, struct historical_logical_volume *hlv)
{
	char buffer[40];
	char *descendants_buffer = NULL;
	int r = 0;

	if (!id_write_format(&hlv->lvid.id[1], buffer, sizeof(buffer)))
		goto_out;

	if (!_alloc_printed_indirect_descendants(&hlv->indirect_glvs, &descendants_buffer))
		goto_out;

	outnlgo(f);
	outfgo(f, "%s {", hlv->name);
	_inc_indent(f);

	outfgo(f, "id = \"%s\"", buffer);

	if (!_print_timestamp(f, "creation_time", hlv->timestamp, buffer, sizeof(buffer)))
		goto_out;

	if (!_print_timestamp(f, "removal_time", hlv->timestamp_removed, buffer, sizeof(buffer)))
		goto_out;

	if (hlv->indirect_origin) {
		if (hlv->indirect_origin->is_historical)
			outfgo(f, "origin = \"%s%s\"", HISTORICAL_LV_PREFIX, hlv->indirect_origin->historical->name);
		else
			outfgo(f, "origin = \"%s\"", hlv->indirect_origin->live->name);
	}

	if (descendants_buffer)
		outfgo(f, "descendants = %s", descendants_buffer);

	_dec_indent(f);
	outfgo(f, "}");

	r = 1;
out:
	free(descendants_buffer);

	return r;
}

static int _print_historical_lvs(struct formatter *f, struct volume_group *vg)
{
	struct glv_list *glvl;

	if (dm_list_empty(&vg->historical_lvs))
		return 1;

	outf(f, "historical_logical_volumes {");
	_inc_indent(f);

	dm_list_iterate_items(glvl, &vg->historical_lvs) {
		if (!_print_historical_lv(f, glvl->glv->historical))
			return_0;
	}

	_dec_indent(f);
	outf(f, "}");

	return 1;
}

/*
 * In the text format we refer to pv's as 'pv1',
 * 'pv2' etc.  This function builds a hash table
 * to enable a quick lookup from device -> name.
 */
static int _build_pv_names(struct formatter *f, struct volume_group *vg)
{
	int count = 0;
	struct pv_list *pvl;
	struct physical_volume *pv;
	char buffer[32], *uuid, *name;

	if (!(f->mem = dm_pool_create("text pv_names", 512)))
		return_0;

	if (!(f->pv_names = dm_hash_create(128)))
		return_0;

	dm_list_iterate_items(pvl, &vg->pvs) {
		pv = pvl->pv;

		/* FIXME But skip if there's already an LV called pv%d ! */
		if (dm_snprintf(buffer, sizeof(buffer), "pv%d", count++) < 0)
			return_0;

		if (!(name = dm_pool_strdup(f->mem, buffer)))
			return_0;

		if (!(uuid = dm_pool_zalloc(f->mem, 64)) ||
		   !id_write_format(&pv->id, uuid, 64))
			return_0;

		if (!dm_hash_insert(f->pv_names, uuid, name))
			return_0;
	}

	return 1;
}

static int _text_vg_export(struct formatter *f,
			   struct volume_group *vg, const char *desc)
{
	int r = 0;

	if (!_build_pv_names(f, vg))
		goto_out;

	if (f->header && !_print_header(vg->cmd, f, desc))
		goto_out;

	if (!out_text(f, "%s {", vg->name))
		goto_out;

	_inc_indent(f);

	if (!_print_vg(f, vg))
		goto_out;

	outnl(f);
	if (!_print_pvs(f, vg))
		goto_out;

	outnl(f);
	if (!_print_lvs(f, vg))
		goto_out;

	outnl(f);
	if (!_print_historical_lvs(f, vg))
		goto_out;

	_dec_indent(f);
	if (!out_text(f, "}"))
		goto_out;

	if (!f->header && !_print_header(vg->cmd, f, desc))
		goto_out;

	r = 1;

      out:
	if (f->mem) {
		dm_pool_destroy(f->mem);
		f->mem = NULL;
	}

	if (f->pv_names) {
		dm_hash_destroy(f->pv_names);
		f->pv_names = NULL;
	}

	return r;
}

int text_vg_export_file(struct volume_group *vg, const char *desc, FILE *fp)
{
	struct formatter *f;
	int r;

	_init();

	if (!(f = zalloc(sizeof(*f))))
		return_0;

	f->data.fp = fp;
	f->indent = 0;
	f->header = 1;
	f->out_with_comment = &_out_with_comment_file;
	f->nl = &_nl_file;

	r = _text_vg_export(f, vg, desc);
	if (r)
		r = !ferror(f->data.fp);
	free(f);
	return r;
}

/* Returns amount of buffer used incl. terminating NUL */
size_t text_vg_export_raw(struct volume_group *vg, const char *desc, char **buf)
{
	struct formatter *f;
	size_t r = 0;

	_init();

	if (!(f = zalloc(sizeof(*f))))
		return_0;

	f->data.buf.size = 65536;	/* Initial metadata limit */
	if (!(f->data.buf.start = malloc(f->data.buf.size))) {
		log_error("text_export buffer allocation failed");
		goto out;
	}

	f->indent = 0;
	f->header = 0;
	f->out_with_comment = &_out_with_comment_raw;
	f->nl = &_nl_raw;

	if (!_text_vg_export(f, vg, desc)) {
		free(f->data.buf.start);
		goto_out;
	}

	r = f->data.buf.used + 1;
	*buf = f->data.buf.start;

      out:
	free(f);
	return r;
}

size_t export_vg_to_buffer(struct volume_group *vg, char **buf)
{
	return text_vg_export_raw(vg, "", buf);
}

struct dm_config_tree *export_vg_to_config_tree(struct volume_group *vg)
{
	char *buf = NULL;
	struct dm_config_tree *vg_cft;

	if (!export_vg_to_buffer(vg, &buf)) {
		log_error("Could not format metadata for VG %s.", vg->name);
		return_NULL;
	}

	if (!(vg_cft = config_tree_from_string_without_dup_node_check(buf))) {
		log_error("Error parsing metadata for VG %s.", vg->name);
		free(buf);
		return_NULL;
	}

	free(buf);
	return vg_cft;
}

#undef outf
#undef outnl
