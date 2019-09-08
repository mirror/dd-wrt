CFLAGS=-g -O2 -Wall -std=c99

all: ndpi_flow_dump

clean::
	rm -f ndpi_flow_dump

ndpi_flow_dump: ndpi_flow_dump.c ../src/ndpi_flow_info.h
	$(CC) -o $@ $(CFLAGS) $<

