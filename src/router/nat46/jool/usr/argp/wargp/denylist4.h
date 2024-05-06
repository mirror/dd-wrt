#ifndef SRC_USR_ARGP_WARGP_DENYLIST_H_
#define SRC_USR_ARGP_WARGP_DENYLIST_H_

int handle_denylist4_display(char *iname, int argc, char **argv, void const *arg);
int handle_denylist4_add(char *iname, int argc, char **argv, void const *arg);
int handle_denylist4_remove(char *iname, int argc, char **argv, void const *arg);
int handle_denylist4_flush(char *iname, int argc, char **argv, void const *arg);

int handle_blacklist4_display(char *iname, int argc, char **argv, void const *arg);
int handle_blacklist4_add(char *iname, int argc, char **argv, void const *arg);
int handle_blacklist4_remove(char *iname, int argc, char **argv, void const *arg);
int handle_blacklist4_flush(char *iname, int argc, char **argv, void const *arg);

void autocomplete_denylist4_display(void const *args);
void autocomplete_denylist4_add(void const *args);
void autocomplete_denylist4_remove(void const *args);
void autocomplete_denylist4_flush(void const *args);

#endif /* SRC_USR_ARGP_WARGP_DENYLIST_H_ */
