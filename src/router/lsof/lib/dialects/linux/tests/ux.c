#define _POSIX_SOURCE
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#define TEMPLATE "/tmp/lsof-test-ux-%ld.s"

static void do_nothing(int n) {}

int main(void) {
    int rendezvous[2];
    if (pipe(rendezvous) < 0) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid > 0) {
        close(rendezvous[0]);
        /* parent: server */
        int server = socket(AF_UNIX, SOCK_STREAM, 0);
        if (server < 0) {
            perror("socket (server)");
            return 1;
        }
        struct sockaddr_un un = {
            .sun_family = AF_UNIX,
        };
        snprintf(un.sun_path, 108, TEMPLATE, (long int)getpid());
        unlink(un.sun_path);

        if (bind(server, (void *)&un, sizeof(un)) < 0) {
            perror("bind (server)");
            return 1;
        }
        if (listen(server, 0) < 0) {
            perror("listen (server)");
            return 1;
        }

        char b = 1;
        if (write(rendezvous[1], &b, 1) < 0) {
            perror("write rendezvous pipe (server)");
            return 1;
        }
        b++;

        int client;
        if ((client = accept(server, NULL, 0)) < 0) {
            perror("listen (server)");
            return 1;
        }

        char buf[1024];
        if (read(client, buf, 1024) < 0) {
            perror("read (server)");
            return 1;
        }

        fputs(buf, stdout);
        printf("ppid %ld\nlisten %d\naccept %d\npath %s\n", (long int)getpid(),
               server, client, un.sun_path);
        fputs("end\n", stdout);
        fflush(stdout);
        signal(SIGCONT, do_nothing);
        pause();

        unlink(un.sun_path);
        (void)write(rendezvous[1], &b, 1);
        b++;

        return 0;
    } else if (pid == 0) {
        char b;
        /* child: client */
        close(rendezvous[1]);
        if (read(rendezvous[0], &b, 1) < 0) {
            perror("read rendezvous pipe (client)");
            /* TODO: kill the parent process. */
            return 1;
        }

        int server = socket(AF_UNIX, SOCK_STREAM, 0);
        if (server < 0) {
            perror("socket (client)");
            return 1;
        }

        struct sockaddr_un un = {
            .sun_family = AF_UNIX,
        };
        snprintf(un.sun_path, 108, TEMPLATE, (long int)getppid());
        if (connect(server, (void *)&un, sizeof(un)) < 0) {
            perror("connect (client)");
            return 1;
        }

        FILE *serverf = fdopen(server, "w");
        if (!serverf) {
            perror("fdopen (client)");
            return 1;
        }

        int server2 = socket(AF_UNIX, SOCK_STREAM, 0);
        if (server2 < 0) {
            perror("socket (client)");
            return 1;
        }
        if (connect(server2, (void *)&un, sizeof(un)) < 0) {
            perror("connect (client (server2))");
            return 1;
        }

        fprintf(serverf, "pid %ld\nconnect %d\nconnect2 %d\n",
                (long int)getpid(), server, server2);
        putc('\0', serverf);
        fflush(serverf);

        if (read(rendezvous[0], &b, 1) < 0) {
            perror("read rendezvous pipe (client)");
            return 1;
        }
        return 0;
    }

    perror("fork");
    return 1;
}
