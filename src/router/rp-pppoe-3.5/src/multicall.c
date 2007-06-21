#include <string.h>


#ifdef HAVE_PPPOERELAY
extern int pppoerelay_main(int argc, char *argv[]);
#endif
#ifdef HAVE_PPPOESERVER
extern int pppoeserver_main(int argc, char *argv[]);
#endif


int main(int argc,char *argv[])
{
  char *base = strrchr (argv[0], '/');      
  base = base ? base + 1 : argv[0];
#ifdef HAVE_PPPOERELAY
  if (strstr (base, "pppoe-relay"))
    return pppoerelay_main(argc,argv);
#endif
#ifdef HAVE_PPPOESERVER
  if (strstr (base, "pppoe-server"))
    return pppoeserver_main(argc,argv);
#endif
return -1;
}

