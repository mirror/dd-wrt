/**
 * \file net.h
 */
#ifndef _NET_H
#define _NET_H

#ifdef __cplusplus
extern "C" {
#endif

#define ERR_NET_UNKNOWN_HOST                    0xFF10
#define ERR_NET_SOCKET_FAILED                   0xFF20
#define ERR_NET_CONNECT_FAILED                  0xFF20
#define ERR_NET_BIND_FAILED                     0xFF30
#define ERR_NET_LISTEN_FAILED                   0xFF40
#define ERR_NET_ACCEPT_FAILED                   0xFF50
#define ERR_NET_RECV_FAILED                     0xFF60
#define ERR_NET_SEND_FAILED                     0xFF70
#define ERR_NET_CONN_RESET                      0xFF80
#define ERR_NET_WOULD_BLOCK                     0xFF90

/**
 * \brief          Initiate a TCP connection with host:port
 *
 * \return         0 if successful, or one of:
 *                      ERR_NET_SOCKET_FAILED,
 *                      ERR_NET_UNKNOWN_HOST,
 *                      ERR_NET_CONNECT_FAILED
 */
int net_connect( int *fd, char *host, unsigned int port );

/**
 * \brief          Create a listening socket on bind_ip:port.
 *                 If bind_ip == NULL, all interfaces are binded.
 *
 * \return         0 if successful, or one of:
 *                      ERR_NET_SOCKET_FAILED,
 *                      ERR_NET_BIND_FAILED,
 *                      ERR_NET_LISTEN_FAILED
 */
int net_bind( int *fd, char *bind_ip, unsigned int port );

/**
 * \brief          Accept a connection from a remote client
 *
 * \return         0 if successful, ERR_NET_ACCEPT_FAILED, or
 *                 ERR_NET_WOULD_BLOCK is bind_fd was set to
 *                 non-blocking and accept() is blocking.
 */
int net_accept( int bind_fd, int *client_fd,
                unsigned char client_ip[4] );

/**
 * \brief          Set the socket non-blocking
 *
 * \return         0 if successful, or 1 if the operation failed
 */
int net_set_nonblock( int fd );

/**
 * \brief          Portable usleep helper
 *
 * \note           Real amount of time slept will not be less than
 *                 select()'s timeout granularity (typically, 10ms).
 */
void net_usleep( unsigned long usec );

/**
 * \brief          Read at most 'len' characters. len is updated to
 *                 reflect the actual number of characters read.
 *
 * \return         0 if successful, ERR_NET_CONN_RESET if the
 *                 connection was closed from the other side, or
 *                 ERR_NET_WOULD_BLOCK if read() is blocking.
 */
int net_recv( int fd, unsigned char *buf, int *len );

/**
 * \brief          Write at most 'len' characters. len is updated to
 *                 reflect the number of characters _not_ written.
 *
 * \return         0 if successful, ERR_NET_CONN_RESET if the
 *                 connection was closed from the other side, or
 *                 ERR_NET_WOULD_BLOCK if write() is blocking.
 */
int net_send( int fd, unsigned char *buf, int *len );

/**
 * \brief          Gracefully shutdown the connection
 */
void net_close( int fd );

#ifdef __cplusplus
}
#endif

#endif /* net.h */
