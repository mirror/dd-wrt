cmd_sysklogd/lib.a := rm -f sysklogd/lib.a; arm-linux-uclibc-ar  rcs sysklogd/lib.a sysklogd/klogd.o sysklogd/logread.o sysklogd/syslogd_and_logger.o
