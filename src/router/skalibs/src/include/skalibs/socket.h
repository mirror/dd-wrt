/* ISC license. */

#ifndef SKALIBS_SOCKET_H
#define SKALIBS_SOCKET_H

#include <stddef.h>
#include <stdint.h>

#include <skalibs/gccattributes.h>
#include <skalibs/posixplz.h>
#include <skalibs/tai.h>
#include <skalibs/fcntl.h>

typedef ssize_t socket_io_func (int, char *, size_t, char *, uint16_t *) ;
typedef socket_io_func *socket_io_func_ref ;

typedef ssize_t socket_iow_func (int, char const *, size_t, char const *, uint16_t) ;
typedef socket_iow_func *socket_iow_func_ref ;

extern int socket_internal (int, int, int, unsigned int) ;
extern int socketpair_internal (int, int, int, unsigned int, int *) ;


 /* UNIX domain sockets */

#define IPCPATH_MAX 107

#define ipc_stream() ipc_stream_nb()
#define ipc_stream_b() ipc_stream_internal(0)
#define ipc_stream_nb() ipc_stream_internal(O_NONBLOCK)
#define ipc_stream_coe() ipc_stream_internal(O_CLOEXEC)
#define ipc_stream_nbcoe() ipc_stream_internal(O_NONBLOCK|O_CLOEXEC)
extern int ipc_stream_internal (unsigned int) ;

#define ipc_datagram() ipc_datagram_nb()
#define ipc_datagram_b() ipc_datagram_internal(0)
#define ipc_datagram_nb() ipc_datagram_internal(O_NONBLOCK)
#define ipc_datagram_coe() ipc_datagram_internal(O_CLOEXEC)
#define ipc_datagram_nbcoe() ipc_datagram_internal(O_NONBLOCK|O_CLOEXEC)
extern int ipc_datagram_internal (unsigned int) ;

#define ipc_pair(sv) ipc_pair_nb(sv)
#define ipc_pair_b(sv) ipc_pair_internal((sv), 0)
#define ipc_pair_nb(sv) ipc_pair_internal((sv), O_NONBLOCK)
#define ipc_pair_coe(sv) ipc_pair_internal((sv), O_CLOEXEC)
#define ipc_pair_nbcoe(sv) ipc_pair_internal((sv), O_NONBLOCK|O_CLOEXEC)
extern int ipc_pair_internal (int *, unsigned int) ;

extern int ipc_bind (int, char const *) ;
extern int ipc_bind_reuse (int, char const *) ;
#define ipc_bind_reuse_lock(fd, path, fdlock) ipc_bind_reuse_lock_perms(fd, path, (fdlock), 0)
extern int ipc_bind_reuse_perms (int, char const *, unsigned int) ;
extern int ipc_bind_reuse_lock_perms (int, char const *, int *, unsigned int) ;
extern int ipc_listen (int, int) ;

#define ipc_accept(s, path, len, trunc) ipc_accept_internal(s, path, len, (trunc), 0)
#define ipc_accept_nb(s, path, len, trunc) ipc_accept_internal(s, path, len, (trunc), O_NONBLOCK)
#define ipc_accept_coe(s, path, len, trunc) ipc_accept_internal(s, path, len, (trunc), O_CLOEXEC)
#define ipc_accept_nbcoe(s, path, len, trunc) ipc_accept_internal(s, path, len, (trunc), O_NONBLOCK|O_CLOEXEC)
extern int ipc_accept_internal (int, char *, size_t, int *, unsigned int) ;

extern int ipc_local (int, char *, size_t, int *) ;

extern int ipc_connect (int, char const *) ;
extern int ipc_connected (int) ;
extern int ipc_timed_connect (int, char const *, tain const *, tain *) ;
#define ipc_timed_connect_g(fd, path, deadline) ipc_timed_connect(fd, path, (deadline), &STAMP)

extern ssize_t ipc_send (int, char const *, size_t, char const *) ;
extern ssize_t ipc_recv (int, char *, size_t, char *) ;


 /* INET and INET6 domain sockets */

#define socket_tcp() socket_tcp4()
#define socket_tcp_b() socket_tcp4_b()
#define socket_tcp_nb() socket_tcp4_nb()
#define socket_tcp_coe() socket_tcp4_coe()
#define socket_tcp_nbcoe() socket_tcp4_nbcoe()

#define socket_tcp4() socket_tcp4_nb()
#define socket_tcp4_b() socket_tcp4_internal(0)
#define socket_tcp4_nb() socket_tcp4_internal(O_NONBLOCK)
#define socket_tcp4_coe() socket_tcp4_internal(O_CLOEXEC)
#define socket_tcp4_nbcoe() socket_tcp4_internal(O_NONBLOCK|O_CLOEXEC)
extern int socket_tcp4_internal (unsigned int) ;

#define socket_tcp6() socket_tcp6_nb()
#define socket_tcp6_b() socket_tcp6_internal(0)
#define socket_tcp6_nb() socket_tcp6_internal(O_NONBLOCK)
#define socket_tcp6_coe() socket_tcp6_internal(O_CLOEXEC)
#define socket_tcp6_nbcoe() socket_tcp6_internal(O_NONBLOCK|O_CLOEXEC)
extern int socket_tcp6_internal (unsigned int) ;

#define socket_udp() socket_udp4()
#define socket_udp_b() socket_udp4_b()
#define socket_udp_nb() socket_udp4_nb()
#define socket_udp_coe() socket_udp4_coe()
#define socket_udp_nbcoe() socket_udp4_nbcoe()

#define socket_udp4() socket_udp4_nb()
#define socket_udp4_b() socket_udp4_internal(0)
#define socket_udp4_nb() socket_udp4_internal(O_NONBLOCK)
#define socket_udp4_coe() socket_udp4_internal(O_CLOEXEC)
#define socket_udp4_nbcoe() socket_udp4_internal(O_NONBLOCK|O_CLOEXEC)
extern int socket_udp4_internal (unsigned int) ;

#define socket_udp6() socket_udp6_nb()
#define socket_udp6_b() socket_udp6_internal(0)
#define socket_udp6_nb() socket_udp6_internal(O_NONBLOCK)
#define socket_udp6_coe() socket_udp6_internal(O_CLOEXEC)
#define socket_udp6_nbcoe() socket_udp6_internal(O_NONBLOCK|O_CLOEXEC)
extern int socket_udp6_internal (unsigned int) ;

extern int socket_waitconn (int, tain const *, tain *) ;
#define socket_waitconn_g(fd, deadline) socket_waitconn(fd, (deadline), &STAMP)
extern int socket_deadlineconnstamp4 (int, char const *, uint16_t, tain const *, tain *) ;
#define socket_deadlineconnstamp(s, ip, port, deadline, stamp) socket_deadlineconnstamp4(s, ip, port, deadline, stamp)
#define socket_deadlineconnstamp4_g(fd, ip, port, deadline) socket_deadlineconnstamp4(fd, ip, port, (deadline), &STAMP)
extern int socket_deadlineconnstamp4_u32 (int, uint32_t, uint16_t, tain const *, tain *) ;
#define socket_deadlineconnstamp4_u32_g(fd, ip, port, deadline) socket_deadlineconnstamp4_u32(fd, ip, port, (deadline), &STAMP)

extern int socket_timeoutconn (int, char const *, uint16_t, unsigned int) ;
extern int socket_connect4 (int, char const *, uint16_t) ;
extern int socket_connect4_u32 (int, uint32_t, uint16_t) ;
extern int socket_connected (int) gccattr_const ;
extern int socket_bind4 (int, char const *, uint16_t) ;
extern int socket_bind4_reuse (int, char const *, uint16_t) ;
#define socket_listen(fd, b) ipc_listen(fd, b)

extern int socket_connect6 (int, char const *, uint16_t) ;
extern int socket_deadlineconnstamp6 (int, char const *, uint16_t, tain const *, tain *) ;
#define socket_deadlineconnstamp6_g(fd, ip6, port, deadline) socket_deadlineconnstamp6(fd, ip6, port, (deadline), &STAMP)
extern int socket_bind6 (int, char const *, uint16_t) ;
extern int socket_bind6_reuse (int, char const *, uint16_t) ;

#define socket_accept4(s, ip, port) socket_accept4_internal(s, ip, (port), 0)
#define socket_accept4_nb(s, ip, port) socket_accept4_internal(s, ip, (port), O_NONBLOCK)
#define socket_accept4_coe(s, ip, port) socket_accept4_internal(s, ip, (port), O_CLOEXEC)
#define socket_accept4_nbcoe(s, ip, port) socket_accept4_internal(s, ip, (port), O_NONBLOCK|O_CLOEXEC)
extern int socket_accept4_internal (int, char *, uint16_t *, unsigned int) ;
extern socket_io_func socket_recv4 ;
extern socket_iow_func socket_send4 ;
extern int socket_local4 (int, char *, uint16_t *) ;
extern int socket_remote4 (int, char *, uint16_t *) ;

#define socket_accept6(s, ip6, port) socket_accept6_internal(s, ip6, (port), 0)
#define socket_accept6_nb(s, ip6, port) socket_accept6_internal(s, ip6, (port), O_NONBLOCK)
#define socket_accept6_coe(s, ip6, port) socket_accept6_internal(s, ip6, (port), O_CLOEXEC)
#define socket_accept6_nbcoe(s, ip6, port) socket_accept6_internal(s, ip6, (port), O_NONBLOCK|O_CLOEXEC)
extern int socket_accept6_internal (int, char *, uint16_t *, unsigned int) ;
extern socket_io_func socket_recv6 ;
extern socket_iow_func socket_send6 ;
extern int socket_local6 (int, char *, uint16_t *) ;
extern int socket_remote6 (int, char *, uint16_t *) ;

extern int socket_ipoptionskill (int) ;
extern int socket_tcpnodelay (int) ;
extern int socket_tcpdelay (int) ;
extern void socket_tryreservein (int, unsigned int) ;


 /* Timed send and recv operations (for dgram sockets) */

extern ssize_t socket_ioloop (int, char *, size_t, char *, uint16_t *, socket_io_func_ref, int, tain const *, tain *) ;
extern socket_io_func socket_ioloop_send4 ;
extern socket_io_func socket_ioloop_send6 ;

#define socket_sendnb4(fd, buf, len, ip4, port, deadline, stamp) socket_ioloop(fd, buf, len, (char *)ip4, &(port), &socket_ioloop_send4, 1, deadline, stamp)
#define socket_sendnb4_g(fd, buf, len, ip4, port, deadline) socket_sendnb4(fd, buf, len, ip4, port, (deadline), &STAMP)
#define socket_recvnb4(fd, buf, len, ip4, port, deadline, stamp) socket_ioloop(fd, buf, len, ip4, port, &socket_recv4, 0, deadline, stamp)
#define socket_recvnb4_g(fd, buf, len, ip4, port, deadline) socket_recvnb4(fd, buf, len, ip4, port, (deadline), &STAMP)

#define socket_sendnb6(fd, buf, len, ip6, port, deadline, stamp) socket_ioloop(fd, buf, len, (char *)ip6, &(port), &socket_ioloop_send6, 1, deadline, stamp)
#define socket_sendnb6_g(fd, buf, len, ip6, port, deadline) socket_sendnb6(fd, buf, len, ip6, port, (deadline), &STAMP)
#define socket_recvnb6(fd, buf, len, ip6, port, deadline, stamp) socket_ioloop(fd, buf, len, ip6, port, &socket_recv6, 0, deadline, stamp)
#define socket_recvnb6_g(fd, buf, len, ip6, port, deadline) socket_recvnb6(fd, buf, len, ip6, port, (deadline), &STAMP)

#endif
