/*
 *	DDNS v3 Client
 *
 *		By:	Alan Yates <alany@ay.com.au>
 *		Date:	27-08-2000
 */
#ifdef WIN32
#include <windows.h>
#else
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#endif
#include <stdio.h>
#include "sockio.h"

int
ddns3_sockio_resolve(char *str, struct sockaddr_in *sai) {
	struct hostent *he;
	char buf[500];
	char err[100];
	short port;
	long addr;
	int ret, i;

	for(i = 0; str[i]; i++) if(str[i] == ':') str[i] = ' ';
	ret = sscanf(str, "%s%hd", buf, &port);
	if(ret < 2) return -1;

	if((addr = inet_addr(buf)) == -1) {
		if(!(he = gethostbyname(buf))) {
			//perror("gethostbyname()");
			ddns3_sockio_error(err,100);
			fprintf(stderr,"gethostbyname(): %s\n", err);
			return -1;
		}
		memcpy(&sai->sin_addr.s_addr, he->h_addr, he->h_length);
	} else {
		sai->sin_addr.s_addr = addr;
	}
	sai->sin_port = htons(port);
	sai->sin_family = AF_INET;

	return 0;
}

int
ddns3_sockio_connect(char *host) {
	int ret;
	int sock;
	struct sockaddr_in sai;
	char err[100];

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock < 0) {
		//perror("socket(PF_INET, SOCK_STREAM, 0)");
		ddns3_sockio_error(err,100);
		fprintf(stderr, "socket(): %s\n",err);
		return -1;
	}

	ret = ddns3_sockio_resolve(host, &sai);
	if(ret < 0) {
		return -1;
	}

	ret = connect(sock, (struct sockaddr *)&sai, sizeof(struct sockaddr_in));
	if(ret < 0) {
		//perror("connect()");
		ddns3_sockio_error(err,100);
		fprintf(stderr, "connect(): %s\n",err);
		return -1;
	}
	
	return sock;
}

int
ddns3_sockio_write(int sock, char *buf, int len) {
	int ret;
	char err[100];

	/* FIXME: we pretend short writes never happen! */
	ret = send(sock, buf, len, 0);
	if(ret < 0) {
		//perror("write()");
		ddns3_sockio_error(err,100);
		fprintf(stderr, "send(): %s\n", err);
	}
	
	return ret;
}

int
ddns3_sockio_read(int sock, char *buf, int len) {
	int ret;
	char err[100];

	ret = recv(sock, buf, len, 0);
	if(ret < 0){
		//perror("read()");
		ddns3_sockio_error(err,100);
		fprintf(stderr, "recv(): %s\n", err);
	}

	return ret;
}

int
ddns3_sockio_close(int sock) {
#ifdef WIN32
	closesocket(sock);
#else
	close(sock);
#endif
	return 0;
}

int
ddns3_sockio_init(void) {
#ifdef WIN32
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
 
	wVersionRequested = MAKEWORD( 1, 1 );
 
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
	    /* Tell the user that we could not find a usable */
	    /* WinSock DLL.                                  */
	    return -1;
	}
#endif

	return 0;
}

int
ddns3_sockio_cleanup(void) {
#ifdef WIN32
	WSACleanup();
#endif
	return 0;
}

int
ddns3_sockio_error(char *buf, int len) {
	int code;
#ifdef WIN32
	code = WSAGetLastError();
	sprintf(buf,"%d",code);
#else
	code = errno;
	sprintf(buf,"%s",strerror(code));
#endif
	return code;
}
