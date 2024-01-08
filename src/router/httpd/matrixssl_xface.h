#ifndef __MATRIXSSL_XFACE_H__
#define __MATRIXSSL_XFACE_H__

#include <stdio.h>
#include <matrixSsl.h>

//#define DEBUG_MATRIXSSL 1

typedef struct {
	int fp;
	ssl_t *ssl;
	char *ssl_recv_buf;
	int ssl_recv_buflen;
	int ssl_recv_cur;
	char *ssl_send_buf;
	int ssl_send_buflen;
	int ssl_send_cur;
} matrixssl_buf;

#define msslAssert(C)
//              if (C) ; else {fprintf(stderr, "%s:%d myAssert(%s)\n",\
//                                              __FILE__, __LINE__, #C); abort(); }

void matrixssl_init(void);
void matrixssl_new_session(int fp);
char *matrixssl_gets(FILE *fp, unsigned char *buf, int len);
int matrixssl_puts(FILE *fp, unsigned char *buf);
int matrixssl_putc(FILE *fp, unsigned char buf);
int matrixssl_printf(FILE *fp, unsigned char *fmt, unsigned char *buf);
int matrixssl_write(FILE *fp, unsigned char *buf, int size);
int matrixssl_read(FILE *fp, unsigned char *buf, int size);
int matrixssl_flush(FILE *fp);
int matrixssl_free_session(FILE *fp);
int do_matrixssl_recv(FILE *fp);
int do_matrixssl_send(FILE *fp);

#endif /* __MATRIXSSL_XFACE_H__ */
