all: swconfig

%.o: %.c
	$(CC) $(CFLAGS) $(COPTS) $(MIPS16_OPT) -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections  -c -o $@ $^

swconfig: cli.o swlib.o #uci.o
	$(CC) $(LDFLAGS)  -ffunction-sections -fdata-sections -Wl,--gc-sections -o $@ $^ $(LIBS)

clean:
	rm -f swconfig *.o
