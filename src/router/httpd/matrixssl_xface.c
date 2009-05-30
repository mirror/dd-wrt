
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <matrixssl_xface.h>

#define MAX_MATRIXSSL_SESSIONS 32

int no_matrixssl_sessions = 0;
matrixssl_buf *bufs[MAX_MATRIXSSL_SESSIONS];
sslKeys_t *keys = NULL;

int MATRIXSSL_ADDSBUF(size_t x, matrixssl_buf * y, unsigned char *z)
{
	if (y && x > 0 && y->ssl_send_buflen - y->ssl_send_cur < x) {
		y->ssl_send_buf =
		    (char *)realloc(y->ssl_send_buf, y->ssl_send_cur + x);
		if (y->ssl_send_buf)
			y->ssl_send_buflen = y->ssl_send_cur + x;
		else
			return -1;
	}
	if (y && x > 0) {
		memcpy(y->ssl_send_buf + y->ssl_send_cur, z, x);
		y->ssl_send_cur += x;
	} else
		return -1;
	return 0;
}

int MATRIXSSL_ADDRBUF(size_t x, matrixssl_buf * y, unsigned char *z)
{
	if (y && x > 0 && y->ssl_recv_buflen - y->ssl_recv_cur < x) {
		y->ssl_recv_buf =
		    (char *)realloc(y->ssl_recv_buf, y->ssl_recv_cur + x);
		if (y->ssl_recv_buf)
			y->ssl_recv_buflen = y->ssl_recv_cur + x;
		else
			return -1;
	}
	if (y && x > 0) {
		memcpy(y->ssl_recv_buf + y->ssl_recv_cur, z, x);
		y->ssl_recv_cur += x;
	} else
		return -1;
	return 0;
}

#define MATRIXSSL_RSTBUF(x)	if(x && x->ssl_recv_buflen == x->ssl_recv_cur && x->ssl_recv_buflen>0){\
					x->ssl_recv_buflen = x->ssl_recv_cur = 0;\
					if(x->ssl_recv_buf)\
						free(x->ssl_recv_buf);\
					x->ssl_recv_buf = NULL;\
				}\
				if(x && x->ssl_send_buflen == x->ssl_send_cur && x->ssl_send_buflen>0){\
					x->ssl_send_buflen = x->ssl_send_cur = 0;\
					if(x->ssl_send_buf)\
						free(x->ssl_send_buf);\
					x->ssl_send_buf = NULL;\
				}

matrixssl_buf *matrixssl_newbuf(int fp)
{
	int i = 0;

	for (i = 0; i < MAX_MATRIXSSL_SESSIONS; i++) {
		if (bufs[i] == NULL) {
			bufs[i] =
			    (matrixssl_buf *) malloc(sizeof(matrixssl_buf));
			if (bufs[i]) {
				memset(bufs[i], 0, sizeof(matrixssl_buf));
				bufs[i]->fp = fp;
				no_matrixssl_sessions++;
			}
			return bufs[i];
		}
	}
	return NULL;
}

matrixssl_buf *matrixssl_findbuf(int fp)
{
	int i = 0;

	for (i = 0; i < MAX_MATRIXSSL_SESSIONS; i++) {
		if (bufs[i]->fp == fp) {

			return bufs[i];
		}
	}
	return NULL;
}

int matrixssl_freebuf(int fp)
{
	int i = 0;

	for (i = 0; i < MAX_MATRIXSSL_SESSIONS; i++) {
		if (bufs[i]->fp == fp) {
			matrixSslDeleteSession(bufs[i]->ssl);
			free(bufs[i]->ssl_recv_buf);
			free(bufs[i]->ssl_send_buf);
			free(bufs[i]);
			bufs[i] = NULL;
			no_matrixssl_sessions--;
			return 0;
		}
	}

	return -1;
}

void matrixssl_init(void)
{
	int i = 0;

	matrixSslOpen();

	for (i = 0; i < MAX_MATRIXSSL_SESSIONS; i++)
		bufs[i] = NULL;

}

void matrixssl_new_session(int fp)
{
	matrixssl_buf *pbuf = matrixssl_newbuf(fp);

	if (NULL == keys || NULL == pbuf
	    || no_matrixssl_sessions >= MAX_MATRIXSSL_SESSIONS)
		return;

	matrixSslNewSession(&pbuf->ssl, keys, NULL, SSL_FLAGS_SERVER);
}

char *matrixssl_gets(FILE * fp, unsigned char *buf, int len)
{
	matrixssl_buf *pbuf = matrixssl_findbuf((int)fp);
	unsigned char *p, *s;

	if (pbuf && pbuf->ssl_recv_buflen == 0 && do_matrixssl_recv(fp) <= 0)
		return NULL;

	if (pbuf->ssl_recv_buflen - pbuf->ssl_recv_cur <= 0)
		return NULL;

	s = pbuf->ssl_recv_buf + pbuf->ssl_recv_cur;
	p = buf;
	while (len > 1 && *s != '\n'
	       && pbuf->ssl_recv_cur < pbuf->ssl_recv_buflen) {

		*p++ = *s++;
		len--;
		pbuf->ssl_recv_cur++;
	}

	if (p == buf)
		return NULL;

	if (*s == EOF || *s == '\n') {
		pbuf->ssl_recv_cur++;
		*p++ = *s++;
	}

	*p = 0;

	if (pbuf->ssl_recv_cur >= pbuf->ssl_recv_buflen)
		MATRIXSSL_RSTBUF(pbuf);

#ifdef DEBUG_MATRIXSSL
	printf("matrixssl_gets - returning %d chars %s, %d left in buffer\n",
	       strlen(buf), buf, pbuf->ssl_recv_buflen - pbuf->ssl_recv_cur);
#endif

	return buf;
}

int matrixssl_putc(FILE * fp, unsigned char c)
{
	matrixssl_buf *pbuf = matrixssl_findbuf((int)fp);

	if (NULL == pbuf)
		return -1;
	if (pbuf->ssl_send_cur > (SSL_MAX_RECORD_LEN - 2048))
		matrixssl_flush(fp);

	MATRIXSSL_ADDSBUF(1, pbuf, &c);

	return 1;
}

int matrixssl_puts(FILE * fp, unsigned char *buf)
{
	matrixssl_buf *pbuf = matrixssl_findbuf((int)fp);
	if (NULL == pbuf || NULL == buf || strlen(buf) <= 0)
		return -1;

	if (pbuf->ssl_send_cur > (SSL_MAX_RECORD_LEN - 2048))
		matrixssl_flush(fp);

	MATRIXSSL_ADDSBUF(strlen(buf), pbuf, buf);

	return strlen(buf);
}

int matrixssl_printf(FILE * fp, unsigned char *fmt, unsigned char *buf)
{
	matrixssl_buf *pbuf = matrixssl_findbuf((int)fp);
	unsigned char out_buf[1024] = { 0 };
#ifdef DEBUG_MATRIXSSL
	printf("matrixssl_printf\n");
#endif
	if (NULL == pbuf || NULL == buf || strlen(buf) <= 0)
		return -1;

	snprintf(out_buf, 1023, fmt, buf);

	if (pbuf->ssl_send_cur > (SSL_MAX_RECORD_LEN - 2048))
		matrixssl_flush(fp);

	MATRIXSSL_ADDSBUF(strlen(out_buf), pbuf, out_buf);

	return do_matrixssl_send(fp);
}

int matrixssl_write(FILE * fp, unsigned char *buf, int size)
{
	matrixssl_buf *pbuf = matrixssl_findbuf((int)fp);
#ifdef DEBUG_MATRIXSSL
	printf("matrixssl_write\n");
#endif
	if (pbuf->ssl_send_cur > (SSL_MAX_RECORD_LEN - 2048))
		matrixssl_flush(fp);

	MATRIXSSL_ADDSBUF(size, pbuf, buf);

	return do_matrixssl_send(fp);
}

int matrixssl_read(FILE * fp, unsigned char *buf, int len)
{
	matrixssl_buf *pbuf = matrixssl_findbuf((int)fp);
#ifdef DEBUG_MATRIXSSL
	printf("matrixssl_read\n");
#endif
	if (NULL == pbuf)
		return -1;
	else if (len == 0)	// || pbuf->ssl_recv_buflen - pbuf->ssl_recv_cur < 0)
		return 0;
	else if (pbuf->ssl_recv_buflen == 0 && do_matrixssl_recv(fp) <= 0)
		return 0;

	len = MIN(pbuf->ssl_recv_buflen - pbuf->ssl_recv_cur, len);

	memcpy(buf, pbuf->ssl_recv_buf + pbuf->ssl_recv_cur, len);

	pbuf->ssl_recv_cur += len;

	if (pbuf->ssl_recv_cur >= pbuf->ssl_recv_buflen) {
#ifdef DEBUG_MATRIXSSL
		printf("matrixssl_read - resetting buffer\n");
#endif
		MATRIXSSL_RSTBUF(pbuf);
	}
#ifdef DEBUG_MATRIXSSL
	printf("matrixssl_read - returning %d bytes, %d left in buffer\n", len,
	       pbuf->ssl_recv_buflen - pbuf->ssl_recv_cur);
#endif
	return len;
/*

   matrixssl_buf* pbuf = matrixssl_findbuf((int)fp);
   char *p, *s;

   if (pbuf &&
       pbuf->ssl_recv_buflen == 0 &&
       do_matrixssl_recv(fp) <= 0)
	return 0;

   if(pbuf->ssl_recv_buflen - pbuf->ssl_recv_cur <= 0)
	return 0;

   s = pbuf->ssl_recv_buf+pbuf->ssl_recv_cur;
   p = buf;
*/
//   do{
/*     while (len > 0)
     {

	*p++ = *s++;
	len--;
	pbuf->ssl_recv_cur++;
	
	if(pbuf->ssl_recv_cur >= pbuf->ssl_recv_buflen){
		MATRIXSSL_RSTBUF(pbuf);
		if(do_matrixssl_recv(fp) < 0)
			break;
		s = pbuf->ssl_recv_buf+pbuf->ssl_recv_cur;
	}
	
     }
*/
//     if(pbuf->ssl_recv_cur >= pbuf->ssl_recv_buflen)
//       MATRIXSSL_RSTBUF(pbuf);

//   } while(len > 0 && do_matrixssl_recv(fp) > 0); 
/*
   if(pbuf->ssl_recv_cur >= pbuf->ssl_recv_buflen)
     MATRIXSSL_RSTBUF(pbuf);

#ifdef DEBUG_MATRIXSSL
printf("matrixssl_read - returning %d chars\n",p-buf);
#endif   
   
   return p-buf;
*/
}

int matrixssl_flush(FILE * fp)
{
#ifdef DEBUG_MATRIXSSL
	printf("matrixssl_flush\n");
#endif
	return do_matrixssl_send(fp);
}

int matrixssl_free_session(FILE * fp)
{
	matrixssl_freebuf((int)fp);

	close((int)fp);

	return 0;
}

int do_matrixssl_recv(FILE * fp)
{
	int rc = -1, ret = -1, in_buf_size = 0, out_buf_size = 0;
	sslBuf_t in, out;
	unsigned char error = 0, alertLevel = 0, alertDescription = 0;
	unsigned char *in_buf = NULL, *out_buf = NULL;;
	long more = 0;

	matrixssl_buf *pbuf = matrixssl_findbuf((int)fp);

	if (NULL == pbuf)
		return -1;

	in_buf_size = out_buf_size = SSL_MAX_PLAINTEXT_LEN;
	in_buf = (char *)malloc(SSL_MAX_PLAINTEXT_LEN);
	out_buf = (char *)malloc(SSL_MAX_PLAINTEXT_LEN);

	MATRIXSSL_RSTBUF(pbuf);

	while (1) {

		in.start = in.end = in.buf = in_buf;
		in.size = in_buf_size;
		memset(in.buf, 0, in_buf_size);

		out.buf = out.start = out.end = out_buf;
		out.size = out_buf_size;
		memset(out.buf, 0, out_buf_size);

		ret =
		    recv((int)fp, in.end, (int)((in.buf + in.size) - in.end),
			 0);
#ifdef DEBUG_MATRIXSSL
		printf("do_matrixssl_recv - %d bytes read from socket\n", ret);
#endif
		if (ret <= 0)
			break;

		in.end += ret;
		in.size = ret;

	      decodeMore:
		error = 0;
		alertLevel = 0;
		alertDescription = 0;
		rc = matrixSslDecode(pbuf->ssl, &in, &out, &error, &alertLevel,
				     &alertDescription);
#ifdef DEBUG_MATRIXSSL
		printf
		    ("do_matrixssl_recv - %d bytes in buffer, %d bytes out buffer\n",
		     in.end - in.start, out.end - out.start);
#endif

		switch (rc) {
/*
	Successfully decoded an application data record, and placed in out buf
*/
		case SSL_SUCCESS:
#ifdef DEBUG_MATRIXSSL
			printf("SSL_SUCCESS\n");
#endif
			if (matrixSslHandshakeIsComplete(pbuf->ssl) == 0
			    && in.end <= in.start)
				continue;

			if (in.end > in.start)
				goto decodeMore;
			else
				continue;

		case SSL_PROCESS_DATA:
#ifdef DEBUG_MATRIXSSL
			printf("SSL_PROCESS_DATA\n");
#endif
			MATRIXSSL_ADDRBUF(out.end - out.start, pbuf, out.start);
			out.start = out.end;

			ioctl((int)fp, FIONREAD, (unsigned long *)&more);

#ifdef DEBUG_MATRIXSSL
			fprintf(stderr,
				"SSL_PROCESS_DATA - received %d bytes, %d more to get\n",
				(int)pbuf->ssl_recv_cur, (int)more);
#endif

/*      if (more > 0){
        memmove(in.buf, in.end, in.size - (in.end - in.start));
	in.start = in.buf;
	in.end = in.buf + in.size - (in.end - in.start);
	in.size = in_buf_size;
      	ret = recv((int)fp, in.end, (int)(in.size - (in.end - in.start)), 0);   
	in.end += ret;
        in.size += ret; 
      }
*/
			if (in.end - in.start > 0) {
#ifdef DEBUG_MATRIXSSL
				fprintf(stderr,
					"SSL_PROCESS_DATA - in.size %d bytes, in.end - in.start %d bytes\n",
					in.size, in.end - in.start);
#endif
				goto decodeMore;
			} else
				ret = pbuf->ssl_recv_cur;

			goto matrixssl_recv_done;
/*
	We've decoded a record that requires a response
	The out buffer contains the encoded response that we must send back
	before decoding any more data
*/
		case SSL_SEND_RESPONSE:
#ifdef DEBUG_MATRIXSSL
			printf("SSL_SEND_RESPONSE\n");
#endif
			while (out.start < out.end) {
				rc = send((int)fp, out.start,
					  (int)(out.end - out.start), 0);

				if (rc <= 0)
					goto matrixssl_recv_done;
				out.start += rc;
			}

			out.buf = out.start = out.end = out_buf;
			out.size = out_buf_size;
			memset(out.buf, 0, out_buf_size);
			goto decodeMore;

/*
	We have a partial record, need to read more data from socket and
	try decoding again.
*/
		case SSL_PARTIAL:
#ifdef DEBUG_MATRIXSSL
			printf("SSL_PARTIAL\n");
#endif
			ioctl((int)fp, FIONREAD, (unsigned long *)&more);

			while (more > 0) {

				if (in_buf_size - (in.end - in.buf) <= more) {
					in.start = in.buf = in_buf =
					    (char *)realloc(in_buf,
							    in_buf_size + more);
					in.end = in.buf + in.size;
					in_buf_size += more;
				}
				ret =
				    recv((int)fp, in.end,
					 in_buf_size - (in.end - in.buf), 0);

#ifdef DEBUG_MATRIXSSL
				printf("SSL_PARTIAL - recv %d bytes\n", ret);
#endif

				if (ret <= 0)
					break;

				in.end += ret;
				//in.size += ret;
				ioctl((int)fp, FIONREAD,
				      (unsigned long *)&more);
			}

			goto decodeMore;
/*
	The out buffer is too small to fit the decoded or response
	data.  Increase the size of the buffer and call decode again
*/
		case SSL_FULL:
#ifdef DEBUG_MATRIXSSL
			printf("SSL_FULL\n");
#endif

			out_buf_size *= 2;
			out.buf = out.start = out.end = out_buf =
			    (char *)realloc(out_buf, out_buf_size);
			out.size = out_buf_size;

#ifdef DEBUG_MATRIXSSL
			printf("SSL_FULL - out_buf is %d bytes\n",
			       out_buf_size);
#endif

			goto decodeMore;
/*
	There was an error decoding the data, or encoding the out buffer.
	There may be a response data in the out buffer, so try to send.
	We try a single hail-mary send of the data, and then close the socket.
	Since we're closing on error, we don't worry too much about a clean flush.
*/
		case SSL_ERROR:
#ifdef DEBUG_MATRIXSSL
			printf("SSL_ERROR - error code is %d\n", error);
#endif
/*
	We've decoded an alert.  The level and description passed into
	matrixSslDecode are filled in with the specifics.
*/
		case SSL_ALERT:
#ifdef DEBUG_MATRIXSSL
			printf("SSL_ALERT\n");
#endif
			if (alertDescription != SSL_ALERT_CLOSE_NOTIFY) {
				fprintf(stderr,
					"Closing connection on alert level %d, description %d.\n",
					alertLevel, alertDescription);
			}

		default:
#ifdef DEBUG_MATRIXSSL
			printf("do_matrixssl_recv - return\n");
#endif
			MATRIXSSL_RSTBUF(pbuf);
			ret = -1;
			goto matrixssl_recv_done;
		}
	}

matrixssl_recv_done:
#ifdef DEBUG_MATRIXSSL
	printf("matrixssl_recv - returning %d\n", ret);
#endif
	pbuf->ssl_recv_cur = 0;
	free(out_buf);
	free(in_buf);
	return ret;
}

int do_matrixssl_send(FILE * fp)
{
	sslBuf_t out;
	int bytes = -1, len = 0;
	matrixssl_buf *pbuf = matrixssl_findbuf((int)fp);
	char out_buf[SSL_MAX_RECORD_LEN];

	if (NULL == pbuf || pbuf->ssl_send_cur <= 0)
		return -1;

	do {
		out.buf = out.start = out.end = out_buf;
		out.size = SSL_MAX_RECORD_LEN;

		bytes = MIN(pbuf->ssl_send_cur - len, SSL_MAX_RECORD_LEN);

		if (matrixSslEncode
		    (pbuf->ssl, pbuf->ssl_send_buf + len, bytes, &out) < 0)
			return -1;

		while (out.start < out.end) {

			bytes =
			    send((int)fp, out.start, (int)(out.end - out.start),
				 0);

			if (bytes <= 0)
				return -1;

			out.start += bytes;
		}

		len += out.end - out.buf;

	}
	while (len < pbuf->ssl_send_cur);

	pbuf->ssl_send_cur = 0;

	return bytes;
}
