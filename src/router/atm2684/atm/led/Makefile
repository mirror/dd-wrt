LIBS=-latm -latmd
OBJS=join.o conn.o main.o address.o frames.o display.o kernel.o
BOOTPGMS=zeppelin
MAN8=zeppelin.8
CFLAGS_PRIVATE=

include ../Rules.make

zeppelin:$(OBJS)
	$(CC) $(LDFLAGS) -o $(BOOTPGMS) $(OBJS) $(LIBD) $(LDLIBS) $(LIBS)
