#!/bin/sh
sed -ne 's/^.*_syscall[0-9].*([^,]*,\([^,)]*\).*/\1.o/gp' syscalls.c


