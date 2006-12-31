/* unix.c - Unix domain socket communication */

/* Written 1998 by Werner Almesberger, EPFL ICA */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "atmd.h"


static int mkaddr(const char *path,struct sockaddr_un *addr)
{
    addr->sun_family = AF_UNIX;
    strcpy(addr->sun_path,path);
    return (char *) &addr->sun_path[strlen(path)]-(char *) addr;
}


int un_create(const char *path,mode_t mode)
{
    struct sockaddr_un addr;
    mode_t old_umask;
    int size;
    int s;

    s = socket(PF_UNIX,SOCK_DGRAM,0);
    if (s < 0) return s;
    (void) unlink(path);
    size = mkaddr(path,&addr);
    old_umask = umask(~mode);
    if (bind(s,(struct sockaddr *) &addr,size) < 0) return -1;
    (void) umask(old_umask);
    return s;
}


int un_attach(const char *path)
{
    struct sockaddr_un addr;
    int size;
    int s;

    s = socket(PF_UNIX,SOCK_DGRAM,0);
    if (s < 0) return s;
    size = mkaddr("",&addr);
    if (bind(s,(struct sockaddr *) &addr,size) < 0) return -1;
    size = mkaddr(path,&addr);
    if (connect(s,(struct sockaddr *) &addr,size) < 0) return -1;
    return s;
}


int un_recv_connect(int s,void *buf,int size)
{
    struct sockaddr_un addr;
    int addr_size;
    int len;

    addr_size = sizeof(addr);
    len = recvfrom(s,buf,size,0,(struct sockaddr *) &addr,&addr_size);
    if (len < 0) return len;
    if (connect(s,(struct sockaddr *) &addr,addr_size) < 0) return -1;
    return len;
}


int un_recv(UN_CTX *ctx,int s,void *buf,int size)
{
    ctx->s = s;
    ctx->size = sizeof(ctx->addr);
    return recvfrom(s,buf,size,0,(struct sockaddr *) &ctx->addr,&ctx->size);
}


int un_send(const UN_CTX *ctx,void *buf,int len)
{
    int sent;

    sent = sendto(ctx->s,buf,len,0,(struct sockaddr *) &ctx->addr,ctx->size);
    if (sent < 0 || sent == len) return sent;
    errno = EMSGSIZE; /* ugly */
    return -1;
}


int un_reply(int s,void *buf,int size,
  int (*handler)(void *buf,int len,void *user),void *user)
{
    UN_CTX ctx;
    int len;

    len = un_recv(&ctx,s,buf,size);
    if (len < 0) return len;
    len = handler(buf,len,user);
    if (len <= 0) return len;
    return un_send(&ctx,buf,len);
}
