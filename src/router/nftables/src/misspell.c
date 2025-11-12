/*
 * Copyright (c) 2018 Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 (or any
 * later) as published by the Free Software Foundation.
 */

#include <nft.h>

#include <limits.h>
#include <utils.h>
#include <misspell.h>

enum string_distance_function {
	DELETION		= 0,	/* m1 */
	INSERTION,			/* m2 */
	TRANSFORMATION,			/* m3 */
};
#define DISTANCE_MAX	(TRANSFORMATION + 1)

static unsigned int min_distance(unsigned int *cost)
{
	unsigned int min = UINT_MAX;
	int k;

	for (k = 0; k < DISTANCE_MAX; k++) {
		if (cost[k] < min)
			min = cost[k];
	}

	return min;
}

/* A simple implementation of "The string-to-string correction problem (1974)"
 * by Robert A. Wagner.
 */
static unsigned int string_distance(const char *a, const char *b)
{
	unsigned int len_a = strlen(a);
	unsigned int len_b = strlen(b);
	unsigned int *distance;
	unsigned int i, j, ret;

	distance = xzalloc((len_a + 1) * (len_b + 1) * sizeof(unsigned int));

#define DISTANCE(__i, __j)	distance[(__i) * len_b + (__j)]

	for (i = 0; i <= len_a; i++)
		DISTANCE(i, 0) = i;
	for (j = 0; j <= len_b; j++)
		DISTANCE(0, j) = j;

	for (i = 1; i <= len_a; i++) {
		for (j = 1; j <= len_b; j++) {
			unsigned int subcost = (a[i] == b[j]) ? 0 : 1;
			unsigned int cost[3];

			cost[DELETION] = DISTANCE(i - 1, j) + 1;
			cost[INSERTION] = DISTANCE(i, j - 1) + 1;
			cost[TRANSFORMATION] = DISTANCE(i - 1, j - 1) + subcost;
			DISTANCE(i, j) = min_distance(cost);

			if (i > 1 && j > 1 &&
			    a[i] == b[j - 1] &&
			    a[i - 1] == b[j])
				DISTANCE(i, j) =
					min(DISTANCE(i, j),
					    DISTANCE(i - 2, j - 2) + subcost);
		}
	}

	ret = DISTANCE(len_a, len_b);

	free(distance);

	return ret;
}

void string_misspell_init(struct string_misspell_state *st)
{
	st->obj = NULL;
	st->min_distance = UINT_MAX;
}

int string_misspell_update(const char *a, const char *b,
			   void *obj, struct string_misspell_state *st)
{
	unsigned int len_a, len_b, max_len, min_len, distance, threshold;

	len_a = strlen(a);
	len_b = strlen(b);

	max_len = max(len_a, len_b);
	min_len = min(len_a, len_b);

	if (max_len <= 1)
		return 0;

	if (max_len - min_len <= 1)
		threshold = max(div_round_up(max_len, 3), 1);
	else
		threshold = div_round_up(max_len + 2, 3);

	distance = string_distance(a, b);
	if (distance > threshold)
		return 0;
	else if (distance < st->min_distance) {
		st->min_distance = distance;
		st->obj = obj;
		return 1;
	}
	return 0;
}
