#ifndef SRC_USR_ARGP_WARGP_INSTANCE_H_
#define SRC_USR_ARGP_WARGP_INSTANCE_H_

int handle_instance_display(char *iname, int argc, char **argv, void const *arg);
int handle_instance_add(char *iname, int argc, char **argv, void const *arg);
int handle_instance_remove(char *iname, int argc, char **argv, void const *arg);
int handle_instance_flush(char *iname, int argc, char **argv, void const *arg);
int handle_instance_status(char *iname, int argc, char **argv, void const *arg);

void autocomplete_instance_display(void const *args);
void autocomplete_instance_add(void const *args);
void autocomplete_instance_remove(void const *args);
void autocomplete_instance_flush(void const *args);
void autocomplete_instance_status(void const *args);

#endif /* SRC_USR_ARGP_WARGP_INSTANCE_H_ */
