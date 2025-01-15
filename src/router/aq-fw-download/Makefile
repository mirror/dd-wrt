CFLAGS += $(LTO) -DNEED_PRINTF

INCLUDES=-Iinclude -Iinclude/registerMap \
	 -Iinclude/registerMap/APPIA \
	 -Iinclude/registerMap/HHD

all: aq-fw-download

%.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) -c -o $@ $^

aq-fw-download: mdioBootLoadCLD.o src/AQ_PhyInterface.o src/AQ_API.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f aq-fw-download mdioBootLoadCLD.o src/AQ_PhyInterface.o src/AQ_API.o
	
install:
	install -D aq-fw-download $(INSTALLDIR)/usr/sbin/aq-fw-download