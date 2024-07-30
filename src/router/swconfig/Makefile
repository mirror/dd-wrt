all: libswitch.so swconfig

%.o: %.c
	$(CC) $(CFLAGS) $(COPTS) $(MIPS16_OPT) -fPIC -I$(TOP)/shared -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections  -c -o $@ $^

libswitch.so: swlib.o
	$(CC) $(LDFLAGS) $(COPTS) -shared  -o $@ $^ $(LIBS)

swconfig: cli.o
	$(CC) $(LDFLAGS) $(COPTS)  -ffunction-sections -fdata-sections -Wl,--gc-sections -o $@ $^ $(LIBS) -L. -lswitch

clean:
	rm -f swconfig *.o *.so
