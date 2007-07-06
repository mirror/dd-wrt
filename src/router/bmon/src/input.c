/*
 * input.c            Input API
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
#include <bmon/input.h>
#include <bmon/node.h>
#include <bmon/utils.h>

static struct input_module *reg_pri_list;
static struct input_module *reg_sec_list;
static struct input_module *preferred;

const char *
get_preferred_input_name(void)
{
	return preferred ? preferred->im_name : "none";
}

static inline void
__register_input_module(struct input_module *ops, struct input_module **list)
{
	ops->im_next = *list;
	*list = ops;
}


void
register_input_module(struct input_module *ops)
{
	__register_input_module(ops, &reg_pri_list);
}

void
register_secondary_input_module(struct input_module *ops)
{
	__register_input_module(ops, &reg_sec_list);
}

static inline struct input_module *
__get_input_module(const char *name, struct input_module *list)
{
	struct input_module *i;

	for (i = list; i; i = i->im_next)
		if (!strcmp(i->im_name, name))
			return i;

	return NULL;
}

static struct input_module *
get_input_module(const char *name)
{
	return __get_input_module(name, reg_pri_list);
}

static struct input_module *
get_sec_input_module(const char *name)
{
	return __get_input_module(name, reg_sec_list);
}

#define FOREACH_SIM(F) \
	do { \
		struct input_module *i; \
		for (i = reg_sec_list; i; i = i->im_next) \
			if (i->im_enable && i->im_##F) \
				i->im_##F (); \
	} while (0)

static void
find_preferred(void)
{
	if (NULL == preferred) {
		struct input_module *i;
		/*
		 * User has not chosen a output module
		 */

#if defined SYS_SUNOS
		preferred = get_input_module("kstat");
#elif defined SYS_BSD
		preferred = get_input_module("sysctl");
#elif defined SYS_LINUX
		preferred = get_input_module("netlink");

		if (NULL == preferred)
			preferred = get_input_module("proc");

		if (NULL == preferred)
			preferred = get_input_module("sysfs");
#endif

		if (NULL == preferred) {
			for (i = reg_pri_list; i; i = i->im_next) {
				if (i->im_no_default) {
					preferred = i;
					break;
				}
			}
		}

		if (NULL == preferred)
			quit("No input method found\n");
	}
}


void
input_read(void)
{
	find_preferred();

	reset_nodes();
	preferred->im_read();

	FOREACH_SIM(read);

	remove_unused_node_items();
}

static void
list_input(void)
{
	struct input_module *i;

	printf("Input modules:\n");
	if (NULL == reg_pri_list)
		printf("\tNo input modules found.\n");
	else
		for (i = reg_pri_list; i; i = i->im_next)
			printf("\t%s\n", i->im_name);
}

void
set_input(const char *name)
{
	static int set = 0;
	module_conf_t *ml, *m;
	
	if (set)
		return;
	set = 1;

	if (NULL == name || !strcasecmp(name, "list")) {
		list_input();
		exit(0);
	}
	
	ml = parse_module_param(name);

	for (m = ml; m; m = m->next) {
		preferred = get_input_module(ml->name);

		if (NULL == preferred)
			continue;

		if (preferred->im_set_opts)
			preferred->im_set_opts(ml->attrs);

		if (preferred->im_probe)
			if (preferred->im_probe())
				return;
	}

	quit("No (working) input module found\n");
}

static void
list_sec_input(void)
{
	struct input_module *i;

	printf("Secondary input modules:\n");
	if (NULL == reg_sec_list)
		printf("\tNo secondary input modules found.\n");
	else
		for (i = reg_sec_list; i; i = i->im_next)
			printf("\t%s\n", i->im_name);
}

void
set_sec_input(const char *name)
{
	static int set = 0;
	module_conf_t *ml, *m;

	if (set)
		return;
	set = 1;

	if (NULL == name || !strcasecmp(name, "list")) {
		list_sec_input();
		exit(0);
	}
	
	ml = parse_module_param(name);

	for (m = ml; m; m = m->next) {
		struct input_module *i = get_sec_input_module(m->name);

		if (NULL == i)
			continue;

		if (i->im_set_opts)
			i->im_set_opts(ml->attrs);

		if (i->im_probe) {
			if (i->im_probe() == 1)
				i->im_enable = 1;
		}
	}
}

void
input_init(void)
{
	find_preferred();

	if (preferred->im_init)
		preferred->im_init();

	FOREACH_SIM(init);
}

void
input_shutdown(void)
{
	find_preferred();

	if (preferred->im_shutdown)
		preferred->im_shutdown();

	FOREACH_SIM(shutdown);
}
