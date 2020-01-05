#include <pcap.h>
#include <pcap-bpf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "pppd.h"

int pcap_pre_compiled (char * fname, struct bpf_program *p)
{
    char buf[128];
    int line = 0, size = 0, index=0, ret=1;
    FILE *f = fopen (fname, "r");
    if (!f)
    {
       option_error("error opening precompiled active-filter '%s': %s",
                    fname, strerror (errno));
       return 0;
    }
    while (fgets (buf, 127, f))
    {
       line++;
       if (*buf == '#')
           continue;
       if (size)
       {
           /*
             struct bpf_insn {
             u_short   code;
             u_char    jt;
             u_char    jf;
             bpf_int32 k;
             }
           */
           struct bpf_insn * insn = & p->bf_insns[index];
           unsigned code, jt, jf, k;
           if (sscanf (buf, "%u %u %u %u", &code, &jt, &jf, &k) != 4)
           {
               goto err;
           }
           insn->code = code;
           insn->jt = jt;
           insn->jf = jf;
           insn->k  = k;
           index++;
       }
       else
       {
           if (sscanf (buf, "%u", &size) != 1)
           {
               goto err;
           }
           p->bf_len = size;
           p->bf_insns = (struct bpf_insn *) 
               malloc (size * sizeof (struct bpf_insn));
       }
    } 
    if (size != index)
    {
       option_error("error in precompiled active-filter,"
                    " expected %d expressions, got %dn",
                    size, index);
       ret = 0;
    }
    fclose(f);
    return ret;

err:
  option_error("error in precompiled active-filter"
              " expression line %s:%d (wrong size)\n", 
              fname, line);
  fclose (f);
  return 0;
}
