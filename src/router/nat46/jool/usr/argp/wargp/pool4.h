#ifndef SRC_USR_ARGP_WARGP_POOL4_H_
#define SRC_USR_ARGP_WARGP_POOL4_H_

int handle_pool4_display(char *iname, int argc, char **argv, void const *arg);
int handle_pool4_add(char *iname, int argc, char **argv, void const *arg);
int handle_pool4_remove(char *iname, int argc, char **argv, void const *arg);
int handle_pool4_flush(char *iname, int argc, char **argv, void const *arg);

void autocomplete_pool4_display(void const *args);
void autocomplete_pool4_add(void const *args);
void autocomplete_pool4_remove(void const *args);
void autocomplete_pool4_flush(void const *args);

#endif /* SRC_USR_ARGP_WARGP_POOL4_H_ */
