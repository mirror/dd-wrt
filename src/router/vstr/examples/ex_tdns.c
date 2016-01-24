/* gcc -O2 -Wall -W -pthread -o bind_pthread -c bind_pthread.c */
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <err.h>
#include <errno.h>

volatile int stop = 0;

#define IP2INT(a, b, c, d) (((a) << 24) | ((b) << 16) | ((c) <<  8) | (d))
#define INT2IP(x)        \
    ((x) >> 24) & 0xFF,  \
    ((x) >> 16) & 0xFF,  \
    ((x) >>  8) & 0xFF,  \
    ((x) >>  0) & 0xFF
#define INT2IP_REV(x)    \
    ((x) >>  0) & 0xFF,  \
    ((x) >>  8) & 0xFF,  \
    ((x) >> 16) & 0xFF,  \
    ((x) >> 24) & 0xFF

static void in_ptr(unsigned int ip)
{
 int ern;
 char rbuf[1024];
 struct hostent rres, *hent;

 ip = htonl(ip);
 if (gethostbyaddr_r((char *)&ip, 4, AF_INET, &rres, rbuf, sizeof(rbuf),
                     &hent, &ern) != 0)
   hent = NULL;
 ip = ntohl(ip);
 
 flockfile(stdout);
 if (!hent)
   printf("%u.%u.%u.%u.in-addr.arpa. IN.PTR:ret=3#NAME\n", INT2IP_REV(ip));
 else
   printf("%u.%u.%u.%u.in-addr.arpa. IN.PTR %s\n",
          INT2IP_REV(ip), hent->h_name);
 fflush(stdout);
 funlockfile(stdout);
} 

static void in_a(const char *buf)
{
 int ern;
 char rbuf[1024];
 struct hostent rres, *hent;

 if (gethostbyname_r(buf, &rres, rbuf, sizeof(rbuf), &hent, &ern) != 0)
   hent = NULL;

 flockfile(stdout);
 if (!hent)
   printf("%s. IN.A:ret=3#NAME\n", buf);
 else
 {
   unsigned int ip = *(unsigned int *)hent->h_addr_list[0];
   
   ip = ntohl(ip);
   printf("%s. IN.A %u.%u.%u.%u\n", hent->h_name, INT2IP_REV(ip));
 }
 
 fflush(stdout);
 funlockfile(stdout);
} 

static void *threadfunc(void *ctx)
{
  char buf[1024];
  
  flockfile(stdin);
  while (!feof(stdin) && fgets(buf, sizeof(buf), stdin))
  {
    unsigned int a, b, c, d;

    funlockfile(stdin);

    if (buf[0] && (buf[strlen(buf) - 1] == '\n'))
      buf[strlen(buf) - 1] = 0;
    
    if (0) { }
    else if (sscanf(buf, "in.ptr %u.%u.%u.%u", &a, &b, &c, &d) == 4)
      in_ptr(IP2INT(a, b, c, d));
    else if ((strlen(buf) > strlen("in.a ")) &&
             !memcmp(buf, "in.a ", strlen("in.a ")))
    {
      char *ptr = buf;
      
      ptr += strlen("in.a ");
      ptr += strspn(ptr, " ");
      
      in_a(ptr);
    }
    else  
      return (ctx);
    
    flockfile(stdin);
  }
  funlockfile(stdin);

  return (NULL);
}

#define EQ(x, y) (strcmp(x, y) == 0)

int main(int argc, char *argv[])
{
  unsigned int nr_thrd = 1;
  pthread_t *a;
  unsigned int i;
  void *ret = NULL;

  if ((argc == 3) &&
      EQ(argv[1], "-c"))
    nr_thrd = atoi(argv[2]);

  a = calloc(nr_thrd, sizeof(pthread_t));
  if (!a)
    errno = ENOMEM, err(EXIT_FAILURE, "init");
  
  for (i = 0; i < nr_thrd; i++)
    pthread_create(a + i, NULL, threadfunc, a + i);
  
  for (i = 0; i < nr_thrd; i++)
    pthread_join(a[i], &ret);
  
  exit (EXIT_SUCCESS);
}
