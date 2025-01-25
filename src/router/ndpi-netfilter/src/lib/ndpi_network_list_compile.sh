#!/bin/bash
#

perl nlist.pl
perl gen_proto_list.pl

gcc -O2 -I. -I../include -Ithird_party/include -o ndpi_network_list_compile \
	ndpi_network_list_compile.c
