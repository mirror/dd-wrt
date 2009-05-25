#!/bin/sh
#rm -f nprobe-nprobe.o
gcc -static *.o -o nprobes  -lz -lpcap -lpthread -ldl    #-lsocket -lnsl  -lresolv -lrt
#strip nprobes
