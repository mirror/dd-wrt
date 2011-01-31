PGMS=delay svctor # ed encopy endump zndump znth
# Remove the # for more hardware-specific debugging programs.
# I'll need those only if you're fiddling with the guts of drivers.

MAN8=

include ../Rules.make

zndump.o:	zndump.c
		$(CC) -c $(CFLAGS) -w -O zndump.c
