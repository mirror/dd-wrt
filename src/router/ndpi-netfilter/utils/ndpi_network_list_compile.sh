#!/bin/bash
#

perl ndpi_network_list_compile_gen.pl

gcc -O2 -I. -I../src/include -I../src/lib/third_party/include -o ndpi_network_list_compile \
	ndpi_network_list_compile.c
