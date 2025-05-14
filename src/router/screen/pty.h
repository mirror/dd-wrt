#ifndef SCREEN_PTY_H
#define SCREEN_PTY_H

int   OpenPTY (char **);
int  ClosePTY (int);

/* global variables */

extern int pty_preopen;

#endif /* SCREEN_PTY_H */
