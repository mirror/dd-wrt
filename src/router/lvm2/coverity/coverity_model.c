/*
 * Copyright (C) 2015 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU General Public License v.2.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Coverity usage:
 *
 * translate model into xml
 * cov-make-library -of coverity_model.xml coverity_model.c
 *
 * compile (using outdir 'cov'):
 * cov-build --dir=cov make CC=gcc
 *
 * analyze (agressively, using 'cov')
 * cov-analyze --dir cov --wait-for-license --hfa --concurrency --enable-fnptr --enable-constraint-fpp --security --all --aggressiveness-level=high --field-offset-escape --user-model-file=coverity/coverity_model.xml
 *
 * generate html output (to 'html' from 'cov'):
 * cov-format-errors --dir cov  --html-output html
 */

struct lv_segment;
struct logical_volume;

struct lv_segment *first_seg(const struct logical_volume *lv)
{
	return ((struct lv_segment **)lv)[0];
}

struct lv_segment *last_seg(const struct logical_volume *lv)
{
	return ((struct lv_segment **)lv)[0];
}

const char *find_config_tree_str(struct cmd_context *cmd, int id, struct profile *profile)
{
	return "STRING";
}

struct logical_volume *origin_from_cow(const struct logical_volume *lv)
{
	if (lv)
		return lv;

	__coverity_panic__();
}

/* simple_memccpy() from glibc */
void *memccpy(void *dest, const void *src, int c, size_t n)
{
	const char *s = src;
	char *d = dest;

	while (n-- > 0)
		if ((*d++ = *s++) == (char) c)
			return d;

	return 0;
}

/*
 * 2 lines bellow needs to be placed in coverity/config/user_nodefs.h
 * Not sure about any other way.
 * Without them, coverity shows warning since x86 system header files
 * are using inline assembly to reset fdset
 */
//#nodef FD_ZERO model_FD_ZERO
//void model_FD_ZERO(void *fdset);

void model_FD_ZERO(void *fdset)
{
	unsigned i;

	for (i = 0; i < 1024 / 8 / sizeof(long); ++i)
		((long*)fdset)[i] = 0;
}


/* Resent Coverity reports quite weird errors... */
int *__errno_location(void)
{
}
const unsigned short **__ctype_b_loc (void)
{
}



/*
 * Added extra pointer check to not need these models,
 * for now just keep then in file
 */

/*
struct cmd_context;
struct profile;

const char *find_config_tree_str(struct cmd_context *cmd, int id, struct profile *profile)
{
        return "text";
}

const char *find_config_tree_str_allow_empty(struct cmd_context *cmd, int id, struct profile *profile)
{
        return "text";
}
*/

/*
 * Until fixed coverity case# 00531860:
 *   A FORWARD_NULL false positive on a recursive function call
 *
 * model also these functions:
 */
/*
const struct dm_config_node;
const struct dm_config_node *find_config_tree_array(struct cmd_context *cmd, int id, struct profile *profile)
{
	const struct dm_config_node *cn;

	return cn;
}

const struct dm_config_node *find_config_tree_node(struct cmd_context *cmd, int id, struct profile *profile)
{
	const struct dm_config_node *cn;

	return cn;
}

int find_config_tree_bool(struct cmd_context *cmd, int id, struct profile *profile)
{
	int b;

	return b;
}
*/
