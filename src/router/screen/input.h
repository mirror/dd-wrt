#ifndef SCREEN_INPUT_H
#define SCREEN_INPUT_H

#include <stdlib.h>

void  inp_setprompt (char *, char *);
void  Input (char *, size_t, int, void (*)(char *, size_t, void *), char *, int);
int   InInput (void);

#endif /* SCREEN_INPUT_H */
