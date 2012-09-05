@echo off
set CFLAGS=-Wall -g -O2 -fno-common
rm -f config.h Makefile config.cache
bash ./configure
make clean
make
