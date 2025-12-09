/*
 * Copyright (c) Red Hat GmbH.	Author: Florian Westphal <fw@strlen.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 (or any
 * later) as published by the Free Software Foundation.
 */

#include <nft.h>
#include <stdio.h>
#include <stdbool.h>

#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <time.h>

#include <sys/random.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <afl++.h>
#include <nftables.h>

static const char self_fault_inject_file[] = "/proc/self/make-it-fail";

#ifdef __AFL_FUZZ_TESTCASE_LEN
/* the below macro gets passed via afl-cc, declares prototypes
 * depending on the afl-cc flavor.
 */
__AFL_FUZZ_INIT();
#else
/* this lets the source compile without afl-clang-fast/lto */
static unsigned char fuzz_buf[4096];
static ssize_t fuzz_len;

#define __AFL_INIT() do { } while (0)
#define __AFL_FUZZ_TESTCASE_LEN fuzz_len
#define __AFL_FUZZ_TESTCASE_BUF fuzz_buf
#define __AFL_FUZZ_INIT() do { } while (0)
#define __AFL_LOOP(x) \
   ((fuzz_len = read(0, fuzz_buf, sizeof(fuzz_buf))) > 0 ? 1 : 0)
#endif

struct nft_afl_state {
	FILE *make_it_fail_fp;
};

static struct nft_afl_state state;

enum nft_fuzzer_opts {
	OPT_HELP = 'h',
	OPT_CHECK = 'c',
	OPT_JSON = 'j',
	OPT_INVALID = '?',

	/* --long only */
	OPT_FUZZER = 1,
	OPT_RANDOUTFLAGS = 2,
};

static const char optstring[] = "hcj";

static struct option options[] = {
	{
		.name = "help",
		.val = OPT_HELP,
	}, {
		.name = "check",
		.val = OPT_CHECK,
	}, {
		.name = "json",
		.val = OPT_JSON,
	}, {
		.name = "fuzzer",
		.val = OPT_FUZZER,
		.has_arg = 1,
	}, {
		.name = "random-outflags",
		.val = OPT_RANDOUTFLAGS,
	}, {
	}
};

static const struct {
	const char			*name;
	enum nft_afl_fuzzer_stage	stage;
} fuzzer_stage_param[] = {
	{
		.name		= "parser",
		.stage		= NFT_AFL_FUZZER_PARSER,
	},
	{
		.name		= "eval",
		.stage		= NFT_AFL_FUZZER_EVALUATION,
	},
	{
		.name		= "netlink-ro",
		.stage		= NFT_AFL_FUZZER_NETLINK_RO,
	},
	{
		.name		= "netlink-rw",
		.stage		= NFT_AFL_FUZZER_NETLINK_RW,
	},
};

static void nft_afl_print_build_info(FILE *fp)
{
#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
	fprintf(fp, "\nWARNING: BUILT WITH FUZZER SUPPORT AND AFL INSTRUMENTATION\n");
#else
	fprintf(fp, "\nWARNING: BUILT WITH FUZZER SUPPORT BUT NO AFL INSTRUMENTATION\n");
#endif
}

static void nft_afl_exit(const char *err)
{
	fprintf(stderr, "Error: fuzzer: %s\n", err);
	sleep(60);	/* assume we're running under afl-fuzz and would be restarted right away */
	exit(EXIT_FAILURE);
}

static char *preprocess(unsigned char *input, ssize_t len)
{
	ssize_t real_len = strnlen((char *)input, len);

	if (real_len == 0)
		return NULL;

	if (real_len >= len)
		input[len - 1] = 0;

	return (char *)input;
}

static bool kernel_is_tainted(void)
{
	FILE *fp = fopen("/proc/sys/kernel/tainted", "r");
	unsigned int taint;
	bool ret = false;

	if (fp) {
		if (fscanf(fp, "%u", &taint) == 1 && taint) {
			fprintf(stderr, "Kernel is tainted: 0x%x\n", taint);
			sleep(3);	/* in case we run under fuzzer, don't restart right away */
			ret = true;
		}

		fclose(fp);
	}

	return ret;
}

static void fault_inject_write(FILE *fp, unsigned int v)
{
	rewind(fp);
	fprintf(fp, "%u\n", v);
	fflush(fp);
}

static void fault_inject_enable(const struct nft_afl_state *state)
{
	if (state->make_it_fail_fp)
		fault_inject_write(state->make_it_fail_fp, 1);
}

static void fault_inject_disable(const struct nft_afl_state *state)
{
	if (state->make_it_fail_fp)
		fault_inject_write(state->make_it_fail_fp, 0);
}

static void nft_afl_run_cmd(struct nft_ctx *ctx, const char *input_cmd)
{
	if (kernel_is_tainted())
		return;

	switch (ctx->afl_ctx_stage) {
	case NFT_AFL_FUZZER_PARSER:
	case NFT_AFL_FUZZER_EVALUATION:
	case NFT_AFL_FUZZER_NETLINK_RO:
		nft_run_cmd_from_buffer(ctx, input_cmd);
		return;
	case NFT_AFL_FUZZER_NETLINK_RW:
		break;
	}

	fault_inject_enable(&state);
	nft_run_cmd_from_buffer(ctx, input_cmd);
	fault_inject_disable(&state);

	kernel_is_tainted();
}

static FILE *fault_inject_open(void)
{
	return fopen(self_fault_inject_file, "r+");
}

static bool nft_afl_state_init(struct nft_afl_state *state)
{
	state->make_it_fail_fp = fault_inject_open();
	return true;
}

static int nft_afl_init(struct nft_ctx *ctx, enum nft_afl_fuzzer_stage stage)
{
#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
	const char instrumented[] = "afl instrumented";
#else
	const char instrumented[] = "no afl instrumentation";
#endif
	unsigned int input_flags;

	nft_afl_print_build_info(stderr);

	if (!nft_afl_state_init(&state))
		return -1;

	ctx->afl_ctx_stage = stage;

	if (state.make_it_fail_fp) {
		unsigned int value;
		int ret;

		rewind(state.make_it_fail_fp);
		ret = fscanf(state.make_it_fail_fp, "%u", &value);
		if (ret != 1 || value != 1) {
			fclose(state.make_it_fail_fp);
			state.make_it_fail_fp = NULL;
		}

		/* if its enabled, disable and then re-enable ONLY
		 * when submitting data to the kernel.
		 *
		 * Otherwise even libnftables memory allocations could fail
		 * which is not what we want.
		 */
		fault_inject_disable(&state);
	}

	input_flags = nft_ctx_input_get_flags(ctx);
	input_flags |= NFT_CTX_INPUT_NO_DNS;
	nft_ctx_input_set_flags(ctx, input_flags);

	if (stage < NFT_AFL_FUZZER_NETLINK_RW)
		nft_ctx_set_dry_run(ctx, true);

	fprintf(stderr, "starting (%s, %s fault injection)", instrumented, state.make_it_fail_fp ? "with" : "no");
	return 0;
}

static uint32_t random_u32(void)
{
	uint32_t v;

	if (getrandom(&v, sizeof(v), GRND_NONBLOCK) == (ssize_t)sizeof(v))
		return v;

	v = (uint32_t)time(NULL) + (uint32_t)getpid();
	srandom(v + random());

	v = random();
	v += random();

	return v;
}

static uint32_t random_outflags(void)
{
	uint32_t random_value;

	random_value = random_u32();

	/* never enable json automatically, rely on command line for this */
	return random_value & ~NFT_CTX_OUTPUT_JSON;
}

static void show_help(const char *name)
{
	int i;

	printf("Usage: %s [ options ]\n\nOptions\n", name);

	for (i = 0; i < (int)(sizeof(options) / sizeof(options[0])) - 1; i++) {
		printf("--%s", options[i].name);

		if (options[i].has_arg)
			fputs(" <arg>", stdout);

		puts("");
	}

	puts("");
	puts("Also see \"nft --help\" for more information on common command line options.");
}

static void show_help_fuzzer(const char *name)
{
	int i;

	show_help(name);
	puts("");

	for (i = 0; i < (int)(sizeof(fuzzer_stage_param) / sizeof(fuzzer_stage_param[0])); i++)
		printf("--fuzzer %s\n", fuzzer_stage_param[i].name);

	puts("Hint: combine \"--fuzzer netlink-rw\" with \"--check\" to not apply changes\n");
}

static int nft_afl_main(struct nft_ctx *ctx)
{
	unsigned char *buf;
	ssize_t len;

	if (kernel_is_tainted())
		return -1;

	if (state.make_it_fail_fp) {
		FILE *fp = fault_inject_open();

		/* reopen is needed because /proc/self is a symlink, i.e.
		 * fp refers to parent process, not "us".
		 */
		if (!fp) {
			fprintf(stderr, "Could not reopen %s: %s", self_fault_inject_file, strerror(errno));
			return -1;
		}

		fclose(state.make_it_fail_fp);
		state.make_it_fail_fp = fp;
	}

	buf = __AFL_FUZZ_TESTCASE_BUF;

	while (__AFL_LOOP(UINT_MAX)) {
		char *input;

		len = __AFL_FUZZ_TESTCASE_LEN;  // do not use the macro directly in a call!

		input = preprocess(buf, len);
		if (!input)
			continue;

		/* buf is null terminated at this point */
		nft_afl_run_cmd(ctx, input);
	}

	/* afl-fuzz will restart us. */
	return 0;
}

int main(int argc, char *argv[])
{
	enum nft_afl_fuzzer_stage fuzzer_stage = NFT_AFL_FUZZER_NETLINK_RO;
	unsigned int json_output_flag = 0;
	bool random_output_flags = false;
	int ret = EXIT_SUCCESS;
	struct nft_ctx *nft;
	unsigned int i;

	nft = nft_ctx_new(NFT_CTX_DEFAULT);

	while (1) {
		int val = getopt_long(argc, argv, optstring, options, NULL);
		if (val == -1)
			break;

		switch (val) {
		case OPT_HELP:
			show_help(argv[0]);
			goto out;
		case OPT_CHECK:
			nft_ctx_set_dry_run(nft, true);
			break;
		case OPT_FUZZER:
			for (i = 0; i < array_size(fuzzer_stage_param); i++) {
				if (strcmp(fuzzer_stage_param[i].name, optarg))
					continue;
				fuzzer_stage = fuzzer_stage_param[i].stage;
				break;
			}

			if (!strcmp(optarg, "help")) {
				show_help_fuzzer(argv[0]);
				goto out;
			}

			if (i == array_size(fuzzer_stage_param)) {
				fprintf(stderr, "invalid fuzzer stage `%s'\n",
					optarg);
				show_help_fuzzer(argv[0]);
				goto out_fail;
			}
			break;
		case OPT_RANDOUTFLAGS:
			random_output_flags = true;
			break;
		case OPT_JSON:
#ifdef HAVE_LIBJANSSON
			json_output_flag = NFT_CTX_OUTPUT_JSON;
#else
			fprintf(stderr, "Error: JSON support not compiled-in\n");
			goto out_fail;
#endif
		case OPT_INVALID:
			nft_afl_exit("Unknown option");
			goto out_fail;
		}
	}

	ret = nft_afl_init(nft, fuzzer_stage);
	if (ret != 0)
		nft_afl_exit("cannot initialize");

	__AFL_INIT();

	if (random_output_flags) {
		unsigned int output_flags = random_outflags();

		nft_ctx_output_set_flags(nft, output_flags | json_output_flag);
	}

	ret = nft_afl_main(nft);
	if (ret != 0)
		nft_afl_exit("fatal error");
out:
	nft_ctx_free(nft);
	return ret;
out_fail:
	nft_ctx_free(nft);
	return EXIT_FAILURE;
}
