#ifndef LOCAL_H
#define LOCAL_H

#define LISTENPORT 2638
extern int listenport;
int *socket_init(char *bindaddr);
void l_listen(int *sock, struct usb_device *, int devnum);

#endif /* ! LOCAL_H */
