/*
 *	DDNS v3 Client
 *
 *		By:	Alan Yates <alany@ay.com.au>
 *		Date:	27-08-2000
 */

#ifndef _SOCKIO_H
#define _SOCKIO_H

int ddns3_sockio_connect(char *host);
int ddns3_sockio_write(int sock, char *buf, int len);
int ddns3_sockio_read(int sock, char *buf, int len);
int ddns3_sockio_close(int sock);
int ddns3_sockio_init(void);
int ddns3_sockio_cleanup(void);
int ddns3_sockio_error(char *buf, int len);
#endif /* _SOCKIO_H */
