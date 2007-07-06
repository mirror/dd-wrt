/*
 * output.c               Output API
 *
 * Copyright (c) 2001-2005 Thomas Graf <tgraf@suug.ch>
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

#include <bmon/bmon.h>
#include <bmon/output.h>
#include <bmon/conf.h>
#include <bmon/node.h>
#include <bmon/signal.h>
#include <bmon/utils.h>

static struct output_module *reg_pri_list;
static struct output_module *reg_sec_list;
static struct output_module *preferred;

static inline void
__regiter_output_module(struct output_module *ops, struct output_module **list)
{
	ops->om_next = *list;
	*list = ops;
}

void register_output_module(struct output_module *ops)
{
	__regiter_output_module(ops, &reg_pri_list);
}

void register_secondary_output_module(struct output_module *ops)
{
	__regiter_output_module(ops, &reg_sec_list);
}

static inline struct output_module *
__get_output_module(const char *name, struct output_module *list)
{
	struct output_module *i;

	for (i = list; i; i = i->om_next)
		if (!strcmp(i->om_name, name))
			return i;

	return NULL;
}

static struct output_module * get_output_module(const char *name)
{
	return __get_output_module(name, reg_pri_list);
}

static struct output_module * get_sec_output_module(const char *name)
{
	return __get_output_module(name, reg_sec_list);
}

#define FOREACH_SOM(F) \
	do { \
		struct output_module *i; \
		for (i = reg_sec_list; i; i = i->om_next) \
			if (i->om_enable && i->om_##F) \
				i->om_##F (); \
	} while (0)

const char * get_preferred_output_name(void)
{
	return preferred ? preferred->om_name : "none";
}

static void find_preferred(int quiet)
{
	if (NULL == preferred)
		preferred = get_output_module("curses");

	if (NULL == preferred)
		preferred = get_output_module("ascii");

	if (NULL == preferred && !quiet)
		quit("No output module found.\n");
}

void output_init(void)
{
	find_preferred(0);

	if (preferred->om_init)
		preferred->om_init();

	FOREACH_SOM(init);
}

void output_pre(void)
{
	find_preferred(0);

	if (preferred->om_pre)
		preferred->om_pre();

	FOREACH_SOM(pre);
}
					
void output_draw(void)
{
	if (get_signal_output())
		if (!is_signal_recvd())
			return;

	find_preferred(0);

	calc_node_rates();

	if (preferred->om_draw)
		preferred->om_draw();

	FOREACH_SOM(draw);
}

void output_post(void)
{
	find_preferred(0);

	if (preferred->om_post)
		preferred->om_post();

	FOREACH_SOM(post);
}

void output_shutdown(void)
{
	find_preferred(1);

	if (preferred && preferred->om_shutdown)
		preferred->om_shutdown();

	FOREACH_SOM(shutdown);
}

static void list_output(void)
{
	struct output_module *o;

	printf("Output modules:\n");
	if (NULL == reg_pri_list)
		printf("\tNo output modules found.\n");
	else
		for (o = reg_pri_list; o; o = o->om_next)
			printf("\t%s\n", o->om_name);
}

void set_output(const char *name)
{
	static int set = 0;
	module_conf_t *ml, *m;

	if (set)
		return;
	set = 1;

	if (NULL == name || !strcasecmp(name, "list")) {
		list_output();
		exit(0);
	}
	
	ml = parse_module_param(name);

	for (m = ml; m; m = m->next) {
		preferred = get_output_module(ml->name);

		if (NULL == preferred)
			continue;

		if (preferred->om_set_opts)
			preferred->om_set_opts(ml->attrs);

		if (preferred->om_probe)
			if (preferred->om_probe())
				return;
	}
	
	quit("No (working) output module found\n");
}

static void list_sec_output(void)
{
	struct output_module *o;

	printf("Secondary output modules:\n");
	if (NULL == reg_sec_list)
		printf("\tNo secondary output modules found.\n");
	else
		for (o = reg_sec_list; o; o = o->om_next)
			printf("\t%s\n", o->om_name);
}

void set_sec_output(const char *name)
{
	module_conf_t *ml, *m;

	if (NULL == name || !strcasecmp(name, "list")) {
		list_sec_output();
		exit(0);
	}
	
	ml = parse_module_param(name);

	for (m = ml; m; m = m->next) {
		struct output_module *o = get_sec_output_module(m->name);

		if (NULL == o)
			continue;

		if (o->om_set_opts)
			o->om_set_opts(ml->attrs);

		if (o->om_probe) {
			if (o->om_probe() == 1)
				o->om_enable = 1;
		}
	}
}

void output_resize(void)
{
	find_preferred(0);

	if (preferred && preferred->om_resize)
		preferred->om_resize();

	FOREACH_SOM(resize);
}

int resized;

int got_resized(void)
{
	int ret = resized;
	resized = 0;
	return ret;
}
