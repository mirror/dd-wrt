/*
 * Copyright (c) 2017 Phil Sutter <phil@nwl.cc>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 (or any
 * later) as published by the Free Software Foundation.
 */

#include <nft.h>

#include <stdarg.h>
#include <nftables.h>
#include <utils.h>

int nft_print(struct output_ctx *octx, const char *fmt, ...)
{
	int ret;
	va_list arg;

	va_start(arg, fmt);
	ret = vfprintf(octx->output_fp, fmt, arg);
	va_end(arg);
	fflush(octx->output_fp);

	return ret;
}

int nft_gmp_print(struct output_ctx *octx, const char *fmt, ...)
{
	int ret;
	va_list arg;

	va_start(arg, fmt);
	ret = gmp_vfprintf(octx->output_fp, fmt, arg);
	va_end(arg);
	fflush(octx->output_fp);

	return ret;
}
