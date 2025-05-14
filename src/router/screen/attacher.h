#ifndef SCREEN_ATTACHER_H
#define SCREEN_ATTACHER_H

int   Attach (int);
void  Attacher (void) __attribute__((__noreturn__));
void  AttacherFinit (int) __attribute__((__noreturn__));
void  SendCmdMessage (char *, char *, char **, int);

#endif /* SCREEN_ATTACHER_H */
