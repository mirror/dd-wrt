#ifndef SRC_USR_ARGP_WARGP_EAMT_H_
#define SRC_USR_ARGP_WARGP_EAMT_H_

int handle_eamt_display(char *iname, int argc, char **argv, void const *arg);
int handle_eamt_add(char *iname, int argc, char **argv, void const *arg);
int handle_eamt_remove(char *iname, int argc, char **argv, void const *arg);
int handle_eamt_flush(char *iname, int argc, char **argv, void const *arg);
int handle_eamt_query(char *iname, int argc, char **argv, void const *arg);

void autocomplete_eamt_display(void const *args);
void autocomplete_eamt_add(void const *args);
void autocomplete_eamt_remove(void const *args);
void autocomplete_eamt_flush(void const *args);
void autocomplete_eamt_query(void const *args);

#endif /* SRC_USR_ARGP_WARGP_EAMT_H_ */
