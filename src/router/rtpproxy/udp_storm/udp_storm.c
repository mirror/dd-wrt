#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <err.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if !defined(SA_LEN)
#define SA_LEN(sa) \
  (((sa)->sa_family == AF_INET) ? \
  sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6))
#endif

struct sender_arg {
    char *host;
    int port;
    char *sendbuf;
    int sendlen;
};

int
resolve(struct sockaddr *ia, int pf, const char *host,
  const char *servname, int flags)
{
    int n;
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = flags;          /* We create listening sockets */
    hints.ai_family = pf;              /* Protocol family */
    hints.ai_socktype = SOCK_DGRAM;     /* UDP */

    n = getaddrinfo(host, servname, &hints, &res);
    if (n == 0) {
        /* Use the first socket address returned */
        memcpy(ia, res->ai_addr, res->ai_addrlen);
        freeaddrinfo(res);
    }

    return n;
}

void sender(void *prt)
{
    int s, n;
    struct sockaddr ia;
    struct sender_arg *sender_arg;
    char *port;

    {int b=0; while (b);}
    sender_arg = (struct sender_arg *)prt;

    asprintf(&port, "%d", sender_arg->port);
    n = resolve(&ia, AF_INET, sender_arg->host, port, AI_PASSIVE);

    s = socket(AF_INET, SOCK_DGRAM, 0);
    connect(s, &ia, SA_LEN(&ia));

    for (;;) {
        n = send(s, sender_arg->sendbuf, sender_arg->sendlen, 0);
        /*printf("send: %d\n", n);*/
        usleep(10000);
    }
}

int main(int argc, char **argv)
{
    int min_port, max_port, nthreads;
    pthread_t thread;
    struct sender_arg sender_arg, *sa;
    void *thread_ret;
    char ch, *datafile;
    char sendbuf[88], databuf[1024 * 8];
    FILE *f;

    min_port = 6000;
    max_port = 7000;
    sender_arg.host = "1.2.3.4";
    datafile = NULL;
    nthreads = -1;
    while ((ch = getopt(argc, argv, "p:P:h:f:t:")) != -1)
        switch (ch) {
        case 'p':
            min_port = atoi(optarg);
            break;

        case 'P':
            max_port = atoi(optarg);
            break;

        case 'h':
            sender_arg.host = optarg;
            break;

        case 'f':
            datafile = optarg;
            break;

        case 't':
            nthreads = atoi(optarg);
            break;
    }

    if (nthreads <= 0) {
        nthreads = max_port - min_port + 1;
    } else if (nthreads < (max_port - min_port + 1)) {
        errx(1, "number of threads should be greater than or equial to port range");
        /* Not reached */
    }

    if (datafile == NULL) {
        sender_arg.sendbuf = sendbuf;
        sender_arg.sendlen = sizeof(sendbuf);
    } else {
        f = fopen(datafile, "r");
        if (f == NULL) {
            err(1, "%s", datafile);
            /* Not reached */
        }
        sender_arg.sendlen = fread(databuf, 1, sizeof(databuf), f);
        sender_arg.sendbuf = databuf;
        fclose(f);
    }

    for (sender_arg.port = min_port; nthreads > 0; nthreads--) {
        sa = malloc(sizeof(*sa));
        sa->host = sender_arg.host;
        sa->port = sender_arg.port;
        sa->sendbuf = sender_arg.sendbuf;
        sa->sendlen = sender_arg.sendlen;
        pthread_create(&thread, NULL, (void *(*)(void *))&sender, (void *)sa);
        sender_arg.port++;
        if (sender_arg.port > max_port)
            sender_arg.port = min_port;
    }
    pthread_join(thread, &thread_ret);
    for(;;);
    return (0);
}
