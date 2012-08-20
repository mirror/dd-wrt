#include <resolv.h>
#include <netdb.h>
#include <stdio.h>




void main(int argc,char *argv[])
{
int salen;
char host[128];
char serv[16];
int rc;
struct sockaddr *sa;
res_init();
fprintf(stdout,"nsaddrs\n");
sa = (struct sockaddr*)_res._u._ext.nsaddrs[0];
fprintf(stdout,"%p",sa);
if (!sa)
	sa = (struct sockaddr*)&_res.nsaddr_list[0];
	salen = sizeof(struct sockaddr);
	sa->sa_family = AF_INET;
	rc = getnameinfo(sa, salen,host, sizeof(host),
	/* can do ((flags & IGNORE_PORT) ? NULL : serv) but why bother? */
			serv, sizeof(serv),
			/* do not resolve port# into service _name_ */
			NI_NUMERICHOST | NI_NUMERICSERV
	);



fprintf(stdout,"rc:%d:%p->%d %s",rc,sa,sa->sa_family,host);

}



