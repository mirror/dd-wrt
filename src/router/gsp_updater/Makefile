INSTDIR   = $(INSTALLDIR)/usr/bin
INSTMODE  = 0755
INSTOWNER = root
INSTGROUP = root

all: gsp_updater i2cset i2cget i2cdump
CFLAGS += -DNEED_PRINTF  -ffunction-sections -fdata-sections -Wl,--gc-sections 

i2cset: i2cset.o util.o i2cbusses.o
	$(CC) $(CFLAGS) -ffunction-sections -fdata-sections -Wl,--gc-sections  $(COPTS) $(LDFLAGS) $^ $(LDLIBS) -o $@

i2cget: i2cget.o util.o i2cbusses.o
	$(CC) $(CFLAGS) -ffunction-sections -fdata-sections -Wl,--gc-sections  $(COPTS) $(LDFLAGS) $^ $(LDLIBS) -o $@

i2cdump: i2cdump.o util.o i2cbusses.o
	$(CC) $(CFLAGS) -ffunction-sections -fdata-sections -Wl,--gc-sections  $(COPTS) $(LDFLAGS) $^ $(LDLIBS) -o $@

gsp_updater: gsp_updater.o
	$(CC) $(CFLAGS)  -ffunction-sections -fdata-sections -Wl,--gc-sections $(COPTS) $(LDFLAGS) $^ $(LDLIBS) -o $@

test_audio: test_audio.o
	$(CC) $(CFLAGS) $(COPTS) $(LDFLAGS) $^ $(LDLIBS) -o $@ -lasound

%.o: %.c
	$(CC) -c $(CFLAGS) $^ -o $@

install: gsp_updater i2cset i2cget i2cdump
	install -d $(INSTDIR)
	install -m $(INSTMODE) -o $(INSTOWNER) -g $(INSTGROUP) gsp_updater i2cset i2cget i2cdump $(INSTDIR)
	install -d $(INSTALLDIR)/etc
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_2380_v35.txt $(INSTALLDIR)/etc/gsc_2380_v35.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_2382_v35.txt $(INSTALLDIR)/etc/gsc_2382_v35.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_2383_v35.txt $(INSTALLDIR)/etc/gsc_2383_v35.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_2384_v35.txt $(INSTALLDIR)/etc/gsc_2384_v35.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_2386_v35.txt $(INSTALLDIR)/etc/gsc_2386_v35.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_2387_v35.txt $(INSTALLDIR)/etc/gsc_2387_v35.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_2388_v35.txt $(INSTALLDIR)/etc/gsc_2388_v35.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_2391_v35.txt $(INSTALLDIR)/etc/gsc_2391_v35.txt

clean:
	rm -f gsp_updater i2cset *.o core

