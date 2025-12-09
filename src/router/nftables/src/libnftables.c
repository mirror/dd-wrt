/*
 * Copyright (c) 2017 Eric Leblond <eric@regit.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 (or any
 * later) as published by the Free Software Foundation.
 */

#include <nft.h>

#include <nftables/libnftables.h>
#include <afl++.h>
#include <erec.h>
#include <mnl.h>
#include <parser.h>
#include <utils.h>
#include <iface.h>
#include <cmd.h>
#include <errno.h>
#include <sys/stat.h>
#include <libgen.h>

static int do_mnl_batch_talk(struct netlink_ctx *ctx, struct list_head *err_list,
			     uint32_t num_cmds)
{
#if HAVE_FUZZER_BUILD
	if (ctx->nft->afl_ctx_stage &&
	    ctx->nft->afl_ctx_stage < NFT_AFL_FUZZER_NETLINK_RW)
		return 0;
#endif
	return mnl_batch_talk(ctx, err_list, num_cmds);
}

static int nft_netlink(struct nft_ctx *nft,
		       struct list_head *cmds, struct list_head *msgs)
{
	uint32_t batch_seqnum, seqnum = 0, last_seqnum = UINT32_MAX, num_cmds = 0;
	struct netlink_ctx ctx = {
		.nft  = nft,
		.msgs = msgs,
		.list = LIST_HEAD_INIT(ctx.list),
		.batch = mnl_batch_init(),
	};
	struct cmd *cmd;
	struct mnl_err *err, *tmp;
	LIST_HEAD(err_list);
	int ret = 0;

	if (list_empty(cmds))
		goto out;

#if HAVE_FUZZER_BUILD
	if (nft->afl_ctx_stage &&
	    nft->afl_ctx_stage <= NFT_AFL_FUZZER_EVALUATION)
		goto out;
#endif
	batch_seqnum = mnl_batch_begin(ctx.batch, mnl_seqnum_inc(&seqnum));

	list_for_each_entry(cmd, cmds, list) {
		ctx.seqnum = cmd->seqnum_from = mnl_seqnum_inc(&seqnum);
		ret = do_command(&ctx, cmd);
		if (ret < 0) {
			netlink_io_error(&ctx, &cmd->location,
					 "Could not process rule: %s",
					 strerror(errno));
			goto out;
		}
		seqnum = cmd->seqnum_to = ctx.seqnum;
		mnl_seqnum_inc(&seqnum);
		num_cmds++;
	}
	if (!nft->check)
		mnl_batch_end(ctx.batch, mnl_seqnum_inc(&seqnum));

	if (!mnl_batch_ready(ctx.batch))
		goto out;

	ret = do_mnl_batch_talk(&ctx, &err_list, num_cmds);
	if (ret < 0) {
		if (ctx.maybe_emsgsize && errno == EMSGSIZE) {
			netlink_io_error(&ctx, NULL,
					 "Could not process rule: %s\n"
					 "Please, rise /proc/sys/net/core/wmem_max on the host namespace. Hint: %d bytes",
					 strerror(errno), round_pow_2(ctx.maybe_emsgsize));
			goto out;
		}
		netlink_io_error(&ctx, NULL,
				 "Could not process rule: %s", strerror(errno));
		goto out;
	}

	if (!list_empty(&err_list))
		ret = -1;

	list_for_each_entry_safe(err, tmp, &err_list, head) {
		/* cmd seqnums are monotonic: only reset the starting position
		 * if the error seqnum is lower than the previous one.
		 */
		if (err->seqnum < last_seqnum)
			cmd = list_first_entry(cmds, struct cmd, list);

		list_for_each_entry_from(cmd, cmds, list) {
			last_seqnum = cmd->seqnum_to;
			if ((err->seqnum >= cmd->seqnum_from &&
			     err->seqnum <= cmd->seqnum_to) ||
			    err->seqnum == batch_seqnum) {
				nft_cmd_error(&ctx, cmd, err);
				errno = err->err;
				if (err->seqnum >= cmd->seqnum_from ||
				    err->seqnum <= cmd->seqnum_to) {
					mnl_err_list_free(err);
					break;
				}
			}
		}

		if (&cmd->list == cmds) {
			/* not found, rewind */
			last_seqnum = UINT32_MAX;
		}
	}
	/* nfnetlink uses the first netlink message header in the batch whose
	 * sequence number is zero to report for EOPNOTSUPP and EPERM errors in
	 * some scenarios. Now it is safe to release pending errors here.
	 */
	list_for_each_entry_safe(err, tmp, &err_list, head)
		mnl_err_list_free(err);
out:
	mnl_batch_reset(ctx.batch);
	return ret;
}

static void nft_init(struct nft_ctx *ctx)
{
	mark_table_init(ctx);
	realm_table_rt_init(ctx);
	devgroup_table_init(ctx);
	ct_label_table_init(ctx);
}

static void nft_exit(struct nft_ctx *ctx)
{
	cache_free(&ctx->cache.table_cache);
	ct_label_table_exit(ctx);
	realm_table_rt_exit(ctx);
	devgroup_table_exit(ctx);
	mark_table_exit(ctx);
}

EXPORT_SYMBOL(nft_ctx_add_var);
int nft_ctx_add_var(struct nft_ctx *ctx, const char *var)
{
	char *separator = strchr(var, '=');
	int pcount = ctx->num_vars;
	struct nft_vars *tmp;
	const char *value;

	if (!separator)
		return -1;

	tmp = xrealloc(ctx->vars, (pcount + 1) * sizeof(struct nft_vars));

	*separator = '\0';
	value = separator + 1;

	ctx->vars = tmp;
	ctx->vars[pcount].key = xstrdup(var);
	ctx->vars[pcount].value = xstrdup(value);
	ctx->num_vars++;

	return 0;
}

EXPORT_SYMBOL(nft_ctx_clear_vars);
void nft_ctx_clear_vars(struct nft_ctx *ctx)
{
	unsigned int i;

	for (i = 0; i < ctx->num_vars; i++) {
		free_const(ctx->vars[i].key);
		free_const(ctx->vars[i].value);
	}
	ctx->num_vars = 0;
	free(ctx->vars);
	ctx->vars = NULL;
}

static bool nft_ctx_find_include_path(struct nft_ctx *ctx, const char *path)
{
	unsigned int i;

	for (i = 0; i < ctx->num_include_paths; i++) {
		if (!strcmp(ctx->include_paths[i], path))
			return true;
	}

	if (!strcmp(path, DEFAULT_INCLUDE_PATH))
		return true;

	return false;
}

static int __nft_ctx_add_include_path(struct nft_ctx *ctx, const char *path)
{
	char **tmp;
	int pcount = ctx->num_include_paths;

	tmp = xrealloc(ctx->include_paths, (pcount + 1) * sizeof(char *));

	ctx->include_paths = tmp;

	if (asprintf(&ctx->include_paths[pcount], "%s", path) < 0)
		return -1;

	ctx->num_include_paths++;
	return 0;
}

EXPORT_SYMBOL(nft_ctx_add_include_path);
int nft_ctx_add_include_path(struct nft_ctx *ctx, const char *path)
{
	char canonical_path[PATH_MAX];

	if (!realpath(path, canonical_path))
		return -1;

	if (nft_ctx_find_include_path(ctx, canonical_path))
		return 0;

	return __nft_ctx_add_include_path(ctx, canonical_path);
}

EXPORT_SYMBOL(nft_ctx_clear_include_paths);
void nft_ctx_clear_include_paths(struct nft_ctx *ctx)
{
	while (ctx->num_include_paths)
		free(ctx->include_paths[--ctx->num_include_paths]);

	free(ctx->include_paths);
	ctx->include_paths = NULL;
}

EXPORT_SYMBOL(nft_ctx_new);
struct nft_ctx *nft_ctx_new(uint32_t flags)
{
	struct nft_ctx *ctx;

#ifdef HAVE_LIBXTABLES
	xt_init();
#endif

	ctx = xzalloc(sizeof(struct nft_ctx));
	nft_init(ctx);

	ctx->state = xzalloc(sizeof(struct parser_state));
	ctx->parser_max_errors	= 10;
	cache_init(&ctx->cache.table_cache);
	ctx->top_scope = scope_alloc();
	ctx->flags = flags;
	ctx->output.output_fp = stdout;
	ctx->output.error_fp = stderr;
	init_list_head(&ctx->vars_ctx.indesc_list);

	ctx->nf_sock = nft_mnl_socket_open();

	return ctx;
}

static ssize_t cookie_write(void *cptr, const char *buf, size_t buflen)
{
	struct cookie *cookie = cptr;

	if (!cookie->buflen) {
		cookie->buflen = buflen + 1;
		cookie->buf = xmalloc(cookie->buflen);
	} else if (cookie->pos + buflen >= cookie->buflen) {
		size_t newlen = cookie->buflen * 2;

		while (newlen <= cookie->pos + buflen)
			newlen *= 2;

		cookie->buf = xrealloc(cookie->buf, newlen);
		cookie->buflen = newlen;
	}
	memcpy(cookie->buf + cookie->pos, buf, buflen);
	cookie->pos += buflen;
	cookie->buf[cookie->pos] = '\0';

	return buflen;
}

static int init_cookie(struct cookie *cookie)
{
	cookie_io_functions_t cookie_fops = {
		.write = cookie_write,
	};

	if (cookie->orig_fp) { /* just rewind buffer */
		if (cookie->buflen) {
			cookie->pos = 0;
			cookie->buf[0] = '\0';
		}
		return 0;
	}

	cookie->orig_fp = cookie->fp;

	cookie->fp = fopencookie(cookie, "w", cookie_fops);
	if (!cookie->fp) {
		cookie->fp = cookie->orig_fp;
		cookie->orig_fp = NULL;
		return 1;
	}

	return 0;
}

static int exit_cookie(struct cookie *cookie)
{
	if (!cookie->orig_fp)
		return 1;

	fclose(cookie->fp);
	cookie->fp = cookie->orig_fp;
	cookie->orig_fp = NULL;
	free(cookie->buf);
	cookie->buf = NULL;
	cookie->buflen = 0;
	cookie->pos = 0;
	return 0;
}

EXPORT_SYMBOL(nft_ctx_buffer_output);
int nft_ctx_buffer_output(struct nft_ctx *ctx)
{
	return init_cookie(&ctx->output.output_cookie);
}

EXPORT_SYMBOL(nft_ctx_unbuffer_output);
int nft_ctx_unbuffer_output(struct nft_ctx *ctx)
{
	return exit_cookie(&ctx->output.output_cookie);
}

EXPORT_SYMBOL(nft_ctx_buffer_error);
int nft_ctx_buffer_error(struct nft_ctx *ctx)
{
	return init_cookie(&ctx->output.error_cookie);
}

EXPORT_SYMBOL(nft_ctx_unbuffer_error);
int nft_ctx_unbuffer_error(struct nft_ctx *ctx)
{
	return exit_cookie(&ctx->output.error_cookie);
}

static const char *get_cookie_buffer(struct cookie *cookie)
{
	fflush(cookie->fp);

	/* This is a bit tricky: Rewind the buffer for future use and return
	 * the old content at the same time. Therefore return an empty string
	 * if buffer position is zero, otherwise just rewind buffer position
	 * and return the unmodified buffer. */

	if (!cookie->pos)
		return "";

	cookie->pos = 0;
	return cookie->buf;
}

EXPORT_SYMBOL(nft_ctx_get_output_buffer);
const char *nft_ctx_get_output_buffer(struct nft_ctx *ctx)
{
	return get_cookie_buffer(&ctx->output.output_cookie);
}

EXPORT_SYMBOL(nft_ctx_get_error_buffer);
const char *nft_ctx_get_error_buffer(struct nft_ctx *ctx)
{
	return get_cookie_buffer(&ctx->output.error_cookie);
}

EXPORT_SYMBOL(nft_ctx_free);
void nft_ctx_free(struct nft_ctx *ctx)
{
	mnl_socket_close(ctx->nf_sock);

	exit_cookie(&ctx->output.output_cookie);
	exit_cookie(&ctx->output.error_cookie);
	iface_cache_release();
	nft_cache_release(&ctx->cache);
	nft_ctx_clear_vars(ctx);
	nft_ctx_clear_include_paths(ctx);
	scope_free(ctx->top_scope);
	free(ctx->state);
	nft_exit(ctx);
	free(ctx);
}

EXPORT_SYMBOL(nft_ctx_set_output);
FILE *nft_ctx_set_output(struct nft_ctx *ctx, FILE *fp)
{
	FILE *old = ctx->output.output_fp;

	if (!fp || ferror(fp))
		return NULL;

	ctx->output.output_fp = fp;

	return old;
}

EXPORT_SYMBOL(nft_ctx_set_error);
FILE *nft_ctx_set_error(struct nft_ctx *ctx, FILE *fp)
{
	FILE *old = ctx->output.error_fp;

	if (!fp || ferror(fp))
		return NULL;

	ctx->output.error_fp = fp;

	return old;
}

EXPORT_SYMBOL(nft_ctx_get_dry_run);
bool nft_ctx_get_dry_run(struct nft_ctx *ctx)
{
	return ctx->check;
}

EXPORT_SYMBOL(nft_ctx_set_dry_run);
void nft_ctx_set_dry_run(struct nft_ctx *ctx, bool dry)
{
	ctx->check = dry;
}

EXPORT_SYMBOL(nft_ctx_get_optimize);
uint32_t nft_ctx_get_optimize(struct nft_ctx *ctx)
{
	return ctx->optimize_flags;
}

EXPORT_SYMBOL(nft_ctx_set_optimize);
void nft_ctx_set_optimize(struct nft_ctx *ctx, uint32_t flags)
{
	ctx->optimize_flags = flags;
}

EXPORT_SYMBOL(nft_ctx_input_get_flags);
unsigned int nft_ctx_input_get_flags(struct nft_ctx *ctx)
{
	return ctx->input.flags;
}

EXPORT_SYMBOL(nft_ctx_input_set_flags);
unsigned int nft_ctx_input_set_flags(struct nft_ctx *ctx, unsigned int flags)
{
	unsigned int old_flags;

	old_flags = ctx->input.flags;
	ctx->input.flags = flags;
	return old_flags;
}

EXPORT_SYMBOL(nft_ctx_output_get_flags);
unsigned int nft_ctx_output_get_flags(struct nft_ctx *ctx)
{
	return ctx->output.flags;
}

EXPORT_SYMBOL(nft_ctx_output_set_flags);
void nft_ctx_output_set_flags(struct nft_ctx *ctx, unsigned int flags)
{
	ctx->output.flags = flags;
}

EXPORT_SYMBOL(nft_ctx_output_get_debug);
unsigned int nft_ctx_output_get_debug(struct nft_ctx *ctx)
{
	return ctx->debug_mask;
}
EXPORT_SYMBOL(nft_ctx_output_set_debug);
void nft_ctx_output_set_debug(struct nft_ctx *ctx, unsigned int mask)
{
	ctx->debug_mask = mask;
}

static const struct input_descriptor indesc_cmdline = {
	.type	= INDESC_BUFFER,
	.name	= "<cmdline>",
};

static int nft_parse_bison_buffer(struct nft_ctx *nft, const char *buf,
				  struct list_head *msgs, struct list_head *cmds,
				  const struct input_descriptor *indesc)
{
	int ret;

	parser_init(nft, nft->state, msgs, cmds, nft->top_scope);
	nft->scanner = scanner_init(nft->state);
	scanner_push_buffer(nft->scanner, indesc, buf);

	ret = nft_parse(nft, nft->scanner, nft->state);
	if (ret != 0 || nft->state->nerrs > 0)
		return -1;

	return 0;
}

static char *stdin_to_buffer(void)
{
	unsigned int bufsiz = 16384, consumed = 0;
	int numbytes;
	char *buf;

	buf = xmalloc(bufsiz);

	numbytes = read(STDIN_FILENO, buf, bufsiz);
	while (numbytes > 0) {
		consumed += numbytes;
		if (consumed == bufsiz) {
			bufsiz *= 2;
			buf = xrealloc(buf, bufsiz);
		}
		numbytes = read(STDIN_FILENO, buf + consumed, bufsiz - consumed);
	}
	buf[consumed] = '\0';

	return buf;
}

static const struct input_descriptor indesc_stdin = {
	.type	= INDESC_STDIN,
	.name	= "/dev/stdin",
};

static int nft_parse_bison_filename(struct nft_ctx *nft, const char *filename,
				    struct list_head *msgs, struct list_head *cmds)
{
	int ret;

	if (nft->stdin_buf)
		return nft_parse_bison_buffer(nft, nft->stdin_buf, msgs, cmds,
					      &indesc_stdin);

	parser_init(nft, nft->state, msgs, cmds, nft->top_scope);
	nft->scanner = scanner_init(nft->state);
	if (scanner_read_file(nft, filename, &internal_location) < 0)
		return -1;

	ret = nft_parse(nft, nft->scanner, nft->state);
	if (ret != 0 || nft->state->nerrs > 0)
		return -1;

	return 0;
}

static int nft_evaluate(struct nft_ctx *nft, struct list_head *msgs,
			struct list_head *cmds)
{
	struct nft_cache_filter *filter;
	struct cmd *cmd, *next;
	unsigned int flags;
	int err = 0;

	filter = nft_cache_filter_init();
	if (nft_cache_evaluate(nft, cmds, msgs, filter, &flags) < 0) {
		nft_cache_filter_fini(filter);
		return -1;
	}
	if (nft_cache_update(nft, flags, msgs, filter) < 0) {
		nft_cache_filter_fini(filter);
		return -1;
	}

	nft_cache_filter_fini(filter);

	list_for_each_entry(cmd, cmds, list) {
		if (cmd->op != CMD_ADD &&
		    cmd->op != CMD_CREATE)
			continue;

		nft_cmd_expand(cmd);
	}

	list_for_each_entry_safe(cmd, next, cmds, list) {
		struct eval_ctx ectx = {
			.nft	= nft,
			.msgs	= msgs,
		};

		if (cmd_evaluate(&ectx, cmd) < 0 &&
		    ++nft->state->nerrs == nft->parser_max_errors) {
			err = -1;
			break;
		}
	}

	if (err < 0 || nft->state->nerrs)
		return -1;

	return 0;
}

EXPORT_SYMBOL(nft_run_cmd_from_buffer);
int nft_run_cmd_from_buffer(struct nft_ctx *nft, const char *buf)
{
	int rc = -EINVAL, parser_rc;
	struct cmd *cmd, *next;
	LIST_HEAD(msgs);
	LIST_HEAD(cmds);
	char *nlbuf;

	nlbuf = xzalloc(strlen(buf) + 2);
	sprintf(nlbuf, "%s\n", buf);

	if (nft_output_json(&nft->output) || nft_input_json(&nft->input))
		rc = nft_parse_json_buffer(nft, nlbuf, &msgs, &cmds);
	if (rc == -EINVAL)
		rc = nft_parse_bison_buffer(nft, nlbuf, &msgs, &cmds,
					    &indesc_cmdline);

#if HAVE_FUZZER_BUILD
	if (nft->afl_ctx_stage == NFT_AFL_FUZZER_PARSER)
		goto err;
#endif
	parser_rc = rc;

	rc = nft_evaluate(nft, &msgs, &cmds);
	if (rc < 0) {
		if (errno == EPERM) {
			fprintf(stderr, "%s (you must be root)\n",
				strerror(errno));
		}
		goto err;
	}

	if (parser_rc) {
		rc = parser_rc;
		goto err;
	}

	if (nft_netlink(nft, &cmds, &msgs) != 0)
		rc = -1;
err:
	erec_print_list(&nft->output, &msgs, nft->debug_mask);
	list_for_each_entry_safe(cmd, next, &cmds, list) {
		list_del(&cmd->list);
		cmd_free(cmd);
	}
	iface_cache_release();
	if (nft->scanner) {
		scanner_destroy(nft);
		nft->scanner = NULL;
	}
	free(nlbuf);

	if (!rc &&
	    nft_output_json(&nft->output) &&
	    nft_output_echo(&nft->output))
		json_print_echo(nft);

	if (rc || nft->check)
		nft_cache_release(&nft->cache);

	return rc;
}

static int load_cmdline_vars(struct nft_ctx *ctx, struct list_head *msgs)
{
	unsigned int bufsize, ret, i, offset = 0;
	LIST_HEAD(cmds);
	char *buf;
	int rc;

	if (ctx->num_vars == 0)
		return 0;

	bufsize = 1024;
	buf = xzalloc(bufsize + 1);
	for (i = 0; i < ctx->num_vars; i++) {
retry:
		ret = snprintf(buf + offset, bufsize - offset,
			       "define %s=%s; ",
			       ctx->vars[i].key, ctx->vars[i].value);
		if (ret >= bufsize - offset) {
			bufsize *= 2;
			buf = xrealloc(buf, bufsize + 1);
			goto retry;
		}
		offset += ret;
	}
	snprintf(buf + offset, bufsize - offset, "\n");

	rc = nft_parse_bison_buffer(ctx, buf, msgs, &cmds, &indesc_cmdline);

	assert(list_empty(&cmds));
	/* Stash the buffer that contains the variable definitions and zap the
	 * list of input descriptors before releasing the scanner state,
	 * otherwise error reporting path walks over released objects.
	 */
	ctx->vars_ctx.buf = buf;
	list_splice_init(&ctx->state->indesc_list, &ctx->vars_ctx.indesc_list);
	scanner_destroy(ctx);
	ctx->scanner = NULL;

	return rc;
}

/* need to use stat() to, fopen() will block for named fifos and
 * libjansson makes no checks before or after open either.
 * /dev/stdin is *never* used, read() from STDIN_FILENO is used instead.
 */
static struct error_record *filename_is_useable(struct nft_ctx *nft, const char *name)
{
	unsigned int type;
	struct stat sb;
	int err;

	if (!strcmp(name, "/dev/stdin"))
		return NULL;

	err = stat(name, &sb);
	if (err)
		return error(&internal_location, "Could not open file \"%s\": %s\n",
			     name, strerror(errno));

	type = sb.st_mode & S_IFMT;

	if (type == S_IFREG || type == S_IFIFO)
		return NULL;

	return error(&internal_location, "Not a regular file: \"%s\"\n", name);
}

static int __nft_run_cmd_from_filename(struct nft_ctx *nft, const char *filename)
{
	struct error_record *erec;
	struct cmd *cmd, *next;
	int rc, parser_rc;
	LIST_HEAD(msgs);
	LIST_HEAD(cmds);

	erec = filename_is_useable(nft, filename);
	if (erec) {
		erec_print(&nft->output, erec, nft->debug_mask);
		erec_destroy(erec);
		return -1;
	}

	rc = load_cmdline_vars(nft, &msgs);
	if (rc < 0)
		goto err;

	rc = -EINVAL;
	if (nft_output_json(&nft->output) || nft_input_json(&nft->input))
		rc = nft_parse_json_filename(nft, filename, &msgs, &cmds);
	if (rc == -EINVAL)
		rc = nft_parse_bison_filename(nft, filename, &msgs, &cmds);

	parser_rc = rc;

	if (nft->optimize_flags)
		nft_optimize(nft, &cmds);

	rc = nft_evaluate(nft, &msgs, &cmds);
	if (rc < 0)
		goto err;

	if (parser_rc) {
		rc = parser_rc;
		goto err;
	}

	if (nft_netlink(nft, &cmds, &msgs) != 0)
		rc = -1;
err:
	erec_print_list(&nft->output, &msgs, nft->debug_mask);
	list_for_each_entry_safe(cmd, next, &cmds, list) {
		list_del(&cmd->list);
		cmd_free(cmd);
	}
	iface_cache_release();
	if (nft->scanner) {
		scanner_destroy(nft);
		nft->scanner = NULL;
	}
	if (!list_empty(&nft->vars_ctx.indesc_list)) {
		struct input_descriptor *indesc, *next;

		list_for_each_entry_safe(indesc, next, &nft->vars_ctx.indesc_list, list) {
			if (indesc->name)
				free_const(indesc->name);

			free(indesc);
		}
	}
	free_const(nft->vars_ctx.buf);

	if (!rc &&
	    nft_output_json(&nft->output) &&
	    nft_output_echo(&nft->output))
		json_print_echo(nft);

	if (rc || nft->check)
		nft_cache_release(&nft->cache);

	scope_release(nft->state->scopes[0]);

	return rc;
}

static int nft_run_optimized_file(struct nft_ctx *nft, const char *filename)
{
	uint32_t optimize_flags;
	bool check;
	int ret;

	check = nft->check;
	nft->check = true;
	optimize_flags = nft->optimize_flags;
	nft->optimize_flags = 0;

	/* First check the original ruleset loads fine as is. */
	ret = __nft_run_cmd_from_filename(nft, filename);
	if (ret < 0)
		return ret;

	nft->check = check;
	nft->optimize_flags = optimize_flags;

	return __nft_run_cmd_from_filename(nft, filename);
}

static int nft_ctx_add_basedir_include_path(struct nft_ctx *nft,
					    const char *filename)
{
	char *basedir = xstrdup(filename);
	int ret;

	ret = nft_ctx_add_include_path(nft, dirname(basedir));

	free(basedir);

	return ret;
}

EXPORT_SYMBOL(nft_run_cmd_from_filename);
int nft_run_cmd_from_filename(struct nft_ctx *nft, const char *filename)
{
	int ret;

	if (!strcmp(filename, "-"))
		filename = "/dev/stdin";

	if (!strcmp(filename, "/dev/stdin"))
		nft->stdin_buf = stdin_to_buffer();

	if (!nft->stdin_buf &&
	    nft_ctx_add_basedir_include_path(nft, filename) < 0)
		return -1;

	if (nft->optimize_flags) {
		ret = nft_run_optimized_file(nft, filename);
		free_const(nft->stdin_buf);
		return ret;
	}

	ret = __nft_run_cmd_from_filename(nft, filename);
	free_const(nft->stdin_buf);

	return ret;
}
