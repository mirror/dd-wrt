#ifndef SRC_USR_ARGP_WARGP_GLOBAL_H_
#define SRC_USR_ARGP_WARGP_GLOBAL_H_

int handle_global_display(char *iname, int argc, char **argv, void const *arg);
void autocomplete_global_display(void const *args);

struct cmd_option *build_global_update_children(void);

#endif /* SRC_USR_ARGP_WARGP_GLOBAL_H_ */
