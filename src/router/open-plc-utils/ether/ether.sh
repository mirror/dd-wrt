#!/bin/sh
# file: ether/ether.sh

# ====================================================================
# programs;
# --------------------------------------------------------------------

gcc -Wall -Wextra -Wno-unused-parameter -o edru edru.c 
gcc -Wall -Wextra -Wno-unused-parameter -o edsu edsu.c
gcc -Wall -Wextra -Wno-unused-parameter -o efbu efbu.c 
gcc -Wall -Wextra -Wno-unused-parameter -o efru efru.c 
gcc -Wall -Wextra -Wno-unused-parameter -o efsu efsu.c 
gcc -Wall -Wextra -Wno-unused-parameter -o nics nics.c 

# ====================================================================
# functions;
# --------------------------------------------------------------------

gcc -Wall -Wextra -Wno-unused-parameter -c fcs.c
gcc -Wall -Wextra -Wno-unused-parameter -c channel.c
gcc -Wall -Wextra -Wno-unused-parameter -c closechannel.c
gcc -Wall -Wextra -Wno-unused-parameter -c gethwaddr.c
gcc -Wall -Wextra -Wno-unused-parameter -c getifname.c
gcc -Wall -Wextra -Wno-unused-parameter -c hostnics.c
gcc -Wall -Wextra -Wno-unused-parameter -c openchannel.c
gcc -Wall -Wextra -Wno-unused-parameter -c readpacket.c
gcc -Wall -Wextra -Wno-unused-parameter -c sendpacket.c

# ====================================================================
# programs that require winpcap or libpcap;
# --------------------------------------------------------------------

# gcc -Wall -Wextra -Wno-unused-parameter -o pcapdevs pcapdevs.c -lpcap

# ====================================================================
# functions that require winpcap or libpcap;
# --------------------------------------------------------------------

# gcc -Wall -Wextra -Wno-unused-parameter -c pcap_nametoindex.c
# gcc -Wall -Wextra -Wno-unused-parameter -c pcap_indextoname.c
# gcc -Wall -Wextra -Wno-unused-parameter -c pcap_nameindex.c
# gcc -Wall -Wextra -Wno-unused-parameter -c pcap_freenameindex.c
 
# ====================================================================
# cleanse;
# --------------------------------------------------------------------

rm -f *.o

