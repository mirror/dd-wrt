#ifndef SCREEN_PROCESS_H
#define SCREEN_PROCESS_H

#include "winmsgbuf.h"

void  InitKeytab (void);
void  ProcessInput (char *, size_t);
void  ProcessInput2 (char *, size_t);
void  DoProcess (Window *, char **, size_t *, struct paster *);
void  DoAction  (struct action *);
int   FindCommnr (const char *);
void  DoCommand (char **, int *);
void  Activate (int);
void  KillWindow (Window *);
void  SetForeWindow (Window *);
int   Parse (char *, int, char **, int *);
void  SetEscape (struct acluser *, int, int);
void  DoScreen (char *, char **);
int   IsNumColon (char *, char *, int);
void  ShowWindows (int);
char *AddWindows (WinMsgBufContext *, int, int, int);
char *AddWindowFlags (char *, int, Window *);
char *AddOtherUsers (char *, int, Window *);
int   WindowByNoN (char *);
Window *FindNiceWindow (Window *, char *);
int   CompileKeys (char *, int, unsigned char *);
void  RefreshXtermOSC (void);
uint64_t ParseAttrColor (char *, int);
void  ApplyAttrColor (uint64_t, struct mchar *);
void  SwitchWindow (Window *);
int   StuffKey (int);

/* global variables */

extern bool hardcopy_append;

extern char *noargs[];
extern char NullStr[];
extern char *zmodem_recvcmd;
extern char *zmodem_sendcmd;

extern int idletimo;
extern int kmap_extn;
extern int zmodem_mode;
extern int TtyMode;

extern struct action idleaction;
extern struct action dmtab[];
extern struct action ktab[];
extern struct action mmtab[];
extern struct action umtab[];

extern struct kmap_ext *kmap_exts;

extern int maptimeout;

#endif /* SCREEN_PROCESS_H */
