#ifndef SCREEN_SOCKET_H
#define SCREEN_SOCKET_H

#include "window.h"

int   FindSocket (int *, int *, int *, char *);
int   MakeClientSocket (int);
int   MakeServerSocket (void);
int   RecoverSocket (void);
int   chsock (void);
void  ReceiveMsg (void);
void  SendCreateMsg (char *, struct NewWindow *);
int   SendErrorMsg (char *, char *);
int   SendAttachMsg (int, Message *, int);
void  ReceiveRaw (int);

#endif /* SCREEN_SOCKET_H */
