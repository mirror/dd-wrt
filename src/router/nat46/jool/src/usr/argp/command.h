#ifndef SRC_USR_ARGP_COMMAND_H_
#define SRC_USR_ARGP_COMMAND_H_

#include <stdbool.h>
#include "common/config.h"

/**
 * BTW: "cmd" (command) refers to the "jool" command. Eg.
 * `jool jool0 pool4 add 192.0.2.1`.
 */
struct cmd_option {
	/**
	 * Name this node is known by the userspace application interface.
	 * This being NULL signals the end of the array.
	 */
	char const *label;
	xlator_type xt;
	/** Hide this option from the user? */
	bool hidden;

	/**
	 * Array of cmd_options available after this one.
	 * If this exists, then @child_builder and @handler.cb must be NULL.
	 */
	struct cmd_option *children;

	/**
	 * A function that returns cmd_options available after this one.
	 * If this exists, then @children and @handler.cb must be NULL.
	 */
	struct cmd_option *(*child_builder)(void);

	/**
	 * A function that will handle any arguments after this one.
	 * If this exists, then @children and @child_builder must be NULL.
	 */
	int (*handler)(char *iname, int argc, char **argv, void const *args);
	/*
	 * Prints the autocompletion candidates for the last element in @argv.
	 * Used by bash autocomplete.
	 * For example, if the user issued `jool bib display --t<tab>`,
	 * then this function will receive `display --t`, and is expected to
	 * print "--tcp" in standard output.
	 *
	 * Actually, current implementations will always print all the known
	 * candidates given this cmd_option's context. This works well enough.
	 */
	void (*handle_autocomplete)(void const *args);
	/*
	 * Will be passed as the last argument to @handler and
	 * @handle_autocomplete.
	 */
	void const *args;

	/** Used by the code to chain temporarily correlated nodes at times. */
	struct cmd_option *next;
};

bool cmdopt_is_hidden(struct cmd_option *option);

#endif /* SRC_USR_ARGP_COMMAND_H_ */
