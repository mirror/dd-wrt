#ifndef SCREEN_RESIZE_H
#define SCREEN_RESIZE_H

#include "window.h"

int   ChangeWindowSize (Window *, int, int, int);
void  ChangeScreenSize (int, int, int);
void  CheckScreenSize (int);
void *xrealloc (void *, size_t);
void  ResizeLayersToCanvases (void);
void  ResizeLayer (Layer *, int, int, Display *);
int   MayResizeLayer (Layer *);
void  FreeAltScreen (Window *);
void  EnterAltScreen (Window *);
void  LeaveAltScreen (Window *);

/* global variables */

extern struct winsize glwz;

#endif /* SCREEN_RESIZE_H */
