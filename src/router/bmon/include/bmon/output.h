/*
 * output.h               Output API
 *
 * Copyright (c) 2001-2004 Thomas Graf <tgraf@suug.ch>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef __BMON_OUTPUT_H_
#define __BMON_OUTPUT_H_

#include <bmon/bmon.h>
#include <bmon/conf.h>

struct output_module
{
	char *                 om_name;
	void                 (*om_init)(void);
	void                 (*om_set_opts)(tv_t *);
	int                  (*om_probe)(void);
	void                 (*om_pre)(void);
	void                 (*om_draw)(void);
	void                 (*om_resize)(void);
	void                 (*om_post)(void);
	void                 (*om_shutdown)(void);
	int                    om_enable;
	struct output_module * om_next;
};

extern void register_output_module(struct output_module *);
extern void register_secondary_output_module(struct output_module *);

extern void set_output(const char *);
extern void set_sec_output(const char *);
extern void output_init(void);
extern void output_pre(void);
extern void output_draw(void);
extern void output_resize(void);
extern void output_post(void);
extern void output_shutdown(void);
extern const char * get_preferred_output_name(void);
extern int resized;
extern int got_resized(void);

#endif
