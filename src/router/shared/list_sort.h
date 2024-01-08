/* GPL v2, lend from the linux kernel */

#ifndef __DD_LIST_SORT_H
#define __DD_LIST_SORT_H

/**
 * list_sort - sort a list
 * @priv: private data, opaque to list_sort(), passed to @cmp
 * @head: the list to sort
 * @cmp: the elements comparison function
 *
 * This function implements "merge sort", which has O(nlog(n))
 * complexity.
 *
 * The comparison function @cmp must return a negative value if @a
 * should sort before @b, and a positive value if @a should sort after
 * @b. If @a and @b are equivalent, and their original relative
 * ordering is to be preserved, @cmp must return 0.
 */
void dd_list_sort(void *priv, struct dd_list_head *head, int (*cmp)(void *priv, struct dd_list_head *a, struct dd_list_head *b));

#endif
