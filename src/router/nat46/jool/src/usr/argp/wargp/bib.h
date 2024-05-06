#ifndef SRC_USR_ARGP_WARGP_BIB_H_
#define SRC_USR_ARGP_WARGP_BIB_H_

int handle_bib_display(char *iname, int argc, char **argv, void const *arg);
int handle_bib_add(char *iname, int argc, char **argv, void const *arg);
int handle_bib_remove(char *iname, int argc, char **argv, void const *arg);

void autocomplete_bib_display(void const *args);
void autocomplete_bib_add(void const *args);
void autocomplete_bib_remove(void const *args);

#endif /* SRC_USR_ARGP_WARGP_BIB_H_ */
