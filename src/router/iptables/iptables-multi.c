#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

int iptables_main(int argc, char **argv);
int iptables_save_main(int argc, char **argv);
int iptables_restore_main(int argc, char **argv);

int main(int argc, char **argv) {
  char *progname;

int i=0;
for (i=0;i<argc;i++)
    fprintf(stderr,"%s ",argv[i]);

fprintf(stderr,"\n");

  if (argc == 0) {
    fprintf(stderr, "no argv[0]?");
    exit(1);
  } else {
// for debugging only (SeG,DD-WRT)
    int i;
    progname = basename(argv[0]);

    if (!strcmp(progname, "iptables"))
      return iptables_main(argc, argv);
    
//    if (!strcmp(progname, "iptables-save"))
//      return iptables_save_main(argc, argv);
    
    if (!strcmp(progname, "iptables-restore"))
      return iptables_restore_main(argc, argv);
    
    fprintf(stderr, "iptables multi-purpose version: unknown applet name %s\n", progname);
    exit(1);
  }
}
