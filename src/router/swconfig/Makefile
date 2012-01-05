all: swconfig

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

swconfig: cli.o swlib.o #uci.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f swconfig *.o
