/*
 * Copyright (c) 1997, 1998, 1999, 2001, 2002, 2004, 2008, 2009, 2010, 2011,
 *               2012, 2013, 2014, 2020
 *      Inferno Nettverk A/S, Norway.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. The above copyright notice, this list of conditions and the following
 *    disclaimer must appear in all copies of the software, derivative works
 *    or modified versions, and any portions thereof, aswell as in all
 *    supporting documentation.
 * 2. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by
 *      Inferno Nettverk A/S, Norway.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Inferno Nettverk A/S requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  sdc@inet.no
 *  Inferno Nettverk A/S
 *  Oslo Research Park
 *  Gaustadalléen 21
 *  NO-0349 Oslo
 *  Norway
 *
 * any improvements or extensions that they make and grant Inferno Nettverk A/S
 * the rights to redistribute these changes.
 *
 */

#include "common.h"

static const char rcsid[] =
"$Id: Rcompat.c,v 1.89.4.2.6.2.4.1 2024/11/21 10:22:42 michaels Exp $";

#if !HAVE_DECL_GETS
char *gets(char *buf);
#endif /* !HAVE_DECL_GETS */

int
Rselect(nfds, readfds, writefds, exceptfds, timeout)
   int nfds;
   fd_set *readfds;
   fd_set *writefds;
   fd_set *exceptfds;
   struct timeval *timeout;
{
   return select(nfds, readfds, writefds, exceptfds, timeout);
}

ssize_t
Rwrite(d, buf, nbytes)
   int d;
   const void *buf;
   size_t nbytes;
{
   const char *function = "Rwrite()";

   clientinit();

   slog(LOG_DEBUG, "%s: fd %d, bytes %lu", function, d, (unsigned long)nbytes);

   return Rsend(d, buf, nbytes, 0);
}

ssize_t
Rwritev(d, iov, iovcnt)
   int d;
   const struct iovec *iov;
   int iovcnt;
{
   struct msghdr msg;
   const char *function = "Rwritev()";

   clientinit();

   slog(LOG_DEBUG, "%s: fd %d, iovcnt %d", function, d, iovcnt);

   bzero(&msg, sizeof(msg));
   msg.msg_iov    = iov;
   msg.msg_iovlen = iovcnt;

   return Rsendmsg(d, &msg, 0);
}

ssize_t
Rsend(s, msg, len, flags)
   int s;
   const void *msg;
   size_t len;
   int flags;
{
   const char *function = "Rsend()";
   struct msghdr msghdr;
   /* any way to get rid of warning about losing const except make a copy? */
   struct iovec iov = { msg, len };

   clientinit();

   slog(LOG_DEBUG, "%s: fd %d, bytes %lu, flags %d",
        function, s, (unsigned long)len, flags);

   bzero(&msghdr, sizeof(msghdr));
   msghdr.msg_iov    = &iov;
   msghdr.msg_iovlen = 1;

   return Rsendmsg(s, &msghdr, flags);
}

ssize_t
Rsendmsg(s, msg, flags)
   int s;
   const struct msghdr *msg;
   int flags;
{
   const int errno_s = errno;
   size_t sent, ioc;
   ssize_t rc;
   struct sockaddr_storage name;
   socklen_t namelen;
   const char *function = "Rsendmsg()";

   clientinit();

   slog(LOG_DEBUG, "%s: fd %d, msg %p, flags %d",
        function, s, msg, flags);

   if (msg == NULL)
      return write(s, NULL, 0);

   namelen = sizeof(name);
   if (getsockname(s, TOSA(&name), &namelen) == -1) {
      errno = errno_s;
      return writev(s, msg->msg_iov, (int)msg->msg_iovlen);
   }

   switch (TOSA(&name)->sa_family) {
      case AF_INET:
         break;

      case AF_INET6:
         break;

      default:
         return sendmsg(s, msg, flags);
   }

   for (sent = ioc = rc = 0; ioc < (size_t)msg->msg_iovlen; ++ioc) {
      if ((rc = Rsendto(s,
                        msg->msg_iov[ioc].iov_base,
                        msg->msg_iov[ioc].iov_len,
                        flags,
                        TOSA(msg->msg_name),
                        msg->msg_namelen)) == -1)
         break;

      sent += rc;

      if (rc != (ssize_t)msg->msg_iov[ioc].iov_len)
         break;
   }

   if (sent <= 0)
      return rc;

   return sent;
}

ssize_t
Rread(d, buf, nbytes)
   int d;
   void *buf;
   size_t nbytes;
{
   const char *function = "Rread()";

   clientinit();

   slog(LOG_DEBUG, "%s: fd %d, bytes %lu",
        function, d, (unsigned long)nbytes);

   return Rrecv(d, buf, nbytes, 0);
}

ssize_t
Rreadv(d, _iov, iovcnt)
   int d;
   const struct iovec *_iov;
   int iovcnt;
{
   const char *function = "Rreadv()";
   struct iovec iov[1] = { { _iov->iov_base, _iov->iov_len } };
   struct msghdr msg;

   clientinit();

   slog(LOG_DEBUG, "%s: fd %d, iovcnt %d", function, d, iovcnt);

   bzero(&msg, sizeof(msg));
   /* LINTED operands have incompatible pointer types */
   msg.msg_iov      = iov;
   msg.msg_iovlen   = iovcnt;

   return Rrecvmsg(d, &msg, 0);
}

ssize_t
Rrecv(s, msg, len, flags)
   int s;
   void *msg;
   size_t len;
   int flags;
{
   struct msghdr msghdr;
   struct iovec iov;
   const char *function = "Rrecv()";

   clientinit();

   slog(LOG_DEBUG, "%s: fd %d, len %lu, flags %d",
        function, s, (unsigned long)len, flags);

   /* LINTED cast discards 'const' from pointer target type */
   bzero(&iov, sizeof(iov));
   iov.iov_base = (void *)msg;
   iov.iov_len  = len;

   bzero(&msghdr, sizeof(msghdr));
   msghdr.msg_iov    = &iov;
   msghdr.msg_iovlen = 1;

   return Rrecvmsg(s, &msghdr, flags);
}

ssize_t
Rrecvmsg(s, msg, flags)
   int s;
   struct msghdr *msg;
   int flags;
{
   const int errno_s = errno;
   size_t received, ioc;
   ssize_t rc;
   struct sockaddr_storage name;
   socklen_t namelen;
   const char *function = "Rrecvmsg()";

   clientinit();

   slog(LOG_DEBUG, "%s: fd %d, msg %p, flags %d",
        function, s, msg, flags);

   if (msg == NULL) {
      rc = recvmsg(s, msg, flags);
      slog(LOG_DEBUG, "%s: rc = %ld", function, (long)rc);

      return rc;
   }

   namelen = sizeof(name);
   if (getsockname(s, TOSA(&name), &namelen) == -1) {
      errno = errno_s;

      /* readv(2).  recvmsg(2) is only for sockets. */
      rc = readv(s, msg->msg_iov, (int)msg->msg_iovlen);
      slog(LOG_DEBUG, "%s: rc = %ld", function, (long)rc);

      return rc;
   }

   switch (TOSA(&name)->sa_family) {
      case AF_INET:
         break;

#ifdef AF_INET6
      case AF_INET6:
         break;
#endif /* AF_INET6 */

      default:
         return recvmsg(s, msg, flags);
   }

   /* no cmsg on proxied sockets. */
   CMSG_TOTLEN(*msg)      = 0;
   CMSG_CONTROLDATA(*msg) = NULL;

   for (received = ioc = rc = 0; ioc < (size_t)msg->msg_iovlen; ++ioc) {
      if ((rc = Rrecvfrom(s,
                          msg->msg_iov[ioc].iov_base,
                          msg->msg_iov[ioc].iov_len,
                          flags,
                          TOSA(msg->msg_name),
                          &msg->msg_namelen)) == -1)
         break;

      received += rc;

      if (rc != (ssize_t)msg->msg_iov[ioc].iov_len)
         break;
   }

   slog(LOG_DEBUG, "%s: bytes received on fd %d: %ld (%s)",
        function, s, (long)rc, strerror(errno));

   if (received <= 0)
      return rc;

   return received;
}

#if HAVE_GSSAPI && HAVE_LINUX_GLIBC_WORKAROUND
/*
 * This code was contributed by
 * Markus Moeller (markus_moeller at compuserve.com).
 */

int
Rfputc(c, stream)
   int c;
   FILE *stream;
{
   const char *function = "Rfputc()";
   const int d = fileno(stream);

   clientinit();

   slog(LOG_DEBUG, "%s: fd %d", function, d);

   if (!gssapi_isencrypted(d))
      return fputc(c, stream);

   socks_setbufferfd(d, _IOFBF, -1);

   return (int)Rsend(d, &c, sizeof(char), 0);
}

int
Rfputs(buf, stream)
   const char *buf;
   FILE *stream;
{
   const char *function = "Rfputs()";
   const int d = fileno(stream);

   clientinit();

   slog(LOG_DEBUG, "%s: fd %d", function, d);

   if (!gssapi_isencrypted(d))
      return fputs(buf,stream);

   socks_setbufferfd(d, _IOFBF, -1);

   return (int)Rsend(d, buf, strlen(buf), 0);
}

size_t
Rfwrite(ptr, size, nmb, stream)
   const void *ptr;
   size_t size; size_t nmb;
   FILE *stream;
{
   const char *function = "Rfwrite()";
   const unsigned char *buf=ptr;
   const int d = fileno(stream);
   size_t i;

   slog(LOG_DEBUG, "%s: fd %d", function, d);

   if (!gssapi_isencrypted(d))
      return fwrite(ptr, size, nmb, stream);

   socks_setbufferfd(d, _IOFBF, -1);

   for (i = 0; i < nmb; ++i)
       if (Rwrite(d,buf+i*size,size) <= 0)
          return i;

   return nmb;
}

int
Rfprintf(FILE *stream, const char *format, ...)
{
   const int d = fileno(stream);
   const char *function = "Rfprintf()";
   va_list ap;
   int rc;

   slog(LOG_DEBUG, "%s: fd %d", function, d);

   va_start(ap, format);

   socks_setbufferfd(d, _IOFBF, -1);

   rc = Rvfprintf(stream, format, ap);

   va_end(ap);

   return rc;
}

int
Rvfprintf(stream,  format, ap)
   FILE *stream;
   const char *format;
   va_list ap;
{
   const char *function = "Rvfprintf()";
   const int d = fileno(stream);
   ssize_t len;
   char buf[8 * BUFSIZ];

   slog(LOG_DEBUG, "%s: fd %d", function, d);

   if (!gssapi_isencrypted(d))
      return vfprintf(stream, format, ap);

   len = vsnprintf(buf, sizeof(buf), format, ap);
   if (len > (ssize_t)sizeof(buf))
      len = (ssize_t)sizeof(buf);

   socks_setbufferfd(d, _IOFBF, -1);

   return (int)Rwrite(d, buf, len);
}

int
Rfflush(s)
   FILE *s;
{
   const char *function = "Rfflush()";
   int d;

   if (s == NULL) {
      (void)socks_flushbuffer(-1, -1, NULL);
      return fflush(s);
   }

   d = fileno(s);

   slog(LOG_DEBUG, "%s: fd %d", function, d);

   if (!gssapi_isencrypted(d))
      return fflush(s);

   (void)socks_flushbuffer(d, -1, NULL);
   return 0;
}

int
Rfclose(s)
   FILE *s;
{
   const char *function = "Rfclose()";
   const int d = fileno(s);

   clientinit();

   slog(LOG_DEBUG, "%s: fd %d", function, d);

   if (gssapi_isencrypted(d))
      (void)socks_flushbuffer(d, -1, NULL);

   return fclose(s);
}

int Rfgetc(stream)
   FILE *stream;
{
   const char *function = "Rfgetc()";
   unsigned char c;
   const int d = fileno(stream);

   clientinit();

   slog(LOG_DEBUG, "%s: fd %d", function, d);

   if (!gssapi_isencrypted(d))
      return fgetc(stream);

   if (Rread(d, &c, 1) != 1)
      return EOF;

   return (int)c;
}

char *
Rgets(buf)
   char *buf;
{
   const char *function = "Rgets()";
   const int d = fileno(stdin);
   size_t i;

   clientinit();

   slog(LOG_DEBUG, "%s: fd %d", function, d);

   if (!gssapi_isencrypted(d))
      return gets(buf);

   i = 0;
   while (Rread(d, buf + i, 1) == 1 && buf[i] != '\n')
      ++i;
   buf[i] = NUL;

   return buf;
}

char *
Rfgets(buf, size, stream)
   char *buf;
   int size;
   FILE *stream;
{
   const char *function = "Rfgets()";
   const int d = fileno(stream);
   int i;

   clientinit();

   slog(LOG_DEBUG, "%s: fd %d", function, d);

   if (!gssapi_isencrypted(d))
      return fgets(buf, size, stream);

   i = 0;
   while (Rread(d, buf + i, 1) == 1 && i < size - 1 && buf[i] != '\n')
      ++i;

   if (size >= 1)
      buf[i != 0 ? i + 1 : i] = NUL;

   return buf;
}

size_t
Rfread(void *ptr, size_t size, size_t nmb, FILE *stream)
{
   const char *function = "Rfread()";
   unsigned char *buf=(unsigned char *)ptr;
   size_t i;
   const int d = fileno(stream);

   slog(LOG_DEBUG, "%s: fd %d", function, d);

   if (!gssapi_isencrypted(d))
      return fread(ptr, size, nmb, stream);

   for (i = 0; i < nmb; ++i)
       if (Rread(d, buf + (i * size), size) <= 0)
          return i;

   return nmb;
}
#endif /* HAVE_GSSAPI && HAVE_LINUX_GLIBC_WORKAROUND */
