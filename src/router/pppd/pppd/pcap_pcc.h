#ifndef PCAP_PCC_H
#define PCAP_PCC_H

#include <pcap.h>

int pcap_pre_compiled (char * fname, struct bpf_program *p);
#endif /* PCAP_PCC_H */
