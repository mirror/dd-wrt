// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2019 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "xfs.h"
#include <assert.h>
#include <sys/statvfs.h>
#include "platform_defs.h"
#include "input.h"
#include "libfrog/paths.h"
#include "libfrog/ptvar.h"
#include "xfs_scrub.h"
#include "common.h"
#include "descr.h"

/*
 * Deferred String Description Renderer
 * ====================================
 * There are many places in xfs_scrub where some event occurred and we'd like
 * to be able to print some sort of message describing what happened, and
 * where.  However, we don't know whether we're going to need the description
 * of where ahead of time and there's little point in spending any time looking
 * up gettext strings and formatting buffers until we actually need to.
 *
 * This code provides enough of a function closure that we are able to record
 * some information about the program status but defer rendering the textual
 * description until we know that we need it.  Once we've rendered the string
 * we can skip it for subsequent calls.  We use per-thread storage for the
 * message buffer to amortize the memory allocation across calls.
 *
 * On a clean filesystem this can reduce the xfs_scrub runtime by 7-10% by
 * avoiding unnecessary work.
 */

static struct ptvar *descr_ptvar;

/* Global buffer for when we aren't running in threaded mode. */
static char global_dsc_buf[DESCR_BUFSZ];

/*
 * Render a textual description string using the function and location stored
 * in the description context.
 */
const char *
__descr_render(
	struct descr		*dsc,
	const char		*file,
	int			line)
{
	char			*dsc_buf;
	int			ret;

	if (descr_ptvar) {
		dsc_buf = ptvar_get(descr_ptvar, &ret);
		if (ret)
			return _("error finding description buffer");
	} else
		dsc_buf = global_dsc_buf;

	ret = dsc->fn(dsc->ctx, dsc_buf, DESCR_BUFSZ, dsc->where);
	if (ret < 0) {
		snprintf(dsc_buf, DESCR_BUFSZ,
_("error %d while rendering description at %s line %d\n"),
				ret, file, line);
	}

	return dsc_buf;
}

/*
 * Set a new location context for this deferred-rendering string.
 * The caller is responsible for freeing the old context, if any.
 */
void
descr_set(
	struct descr		*dsc,
	void			*where)
{
	dsc->where = where;
}

/* Allocate all the description string buffers. */
int
descr_init_phase(
	struct scrub_ctx	*ctx,
	unsigned int		nr_threads)
{
	int			ret;

	assert(descr_ptvar == NULL);
	ret = -ptvar_alloc(nr_threads, DESCR_BUFSZ, &descr_ptvar);
	if (ret)
		str_liberror(ctx, ret, _("creating description buffer"));

	return ret;
}

/* Free all the description string buffers. */
void
descr_end_phase(void)
{
	if (descr_ptvar)
		ptvar_free(descr_ptvar);
	descr_ptvar = NULL;
}
