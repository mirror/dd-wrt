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

gsp_updater: gsc_update.o
	$(CC) $(CFLAGS)  -ffunction-sections -fdata-sections -Wl,--gc-sections $(COPTS) $(LDFLAGS) $^ $(LDLIBS) -o $@

test_audio: test_audio.o
	$(CC) $(CFLAGS) $(COPTS) $(LDFLAGS) $^ $(LDLIBS) -o $@ -lasound

%.o: %.c
	$(CC) -c $(CFLAGS) $^ -o $@

install: gsp_updater i2cset i2cget i2cdump
	install -d $(INSTDIR)
	install -m $(INSTMODE) -o $(INSTOWNER) -g $(INSTGROUP) gsp_updater i2cset i2cget i2cdump $(INSTDIR)
	install -d $(INSTALLDIR)/etc
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_2380_v50.txt $(INSTALLDIR)/etc/gsc_2380_v50.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_2382_v50.txt $(INSTALLDIR)/etc/gsc_2382_v50.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_2383_v50.txt $(INSTALLDIR)/etc/gsc_2383_v50.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_2384_v35.txt $(INSTALLDIR)/etc/gsc_2384_v35.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_2386_v51.txt $(INSTALLDIR)/etc/gsc_2386_v51.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_2387_v46.txt $(INSTALLDIR)/etc/gsc_2387_v46.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_2388_v51.txt $(INSTALLDIR)/etc/gsc_2388_v51.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_2391_v51.txt $(INSTALLDIR)/etc/gsc_2391_v51.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_51xx_v50.txt $(INSTALLDIR)/etc/gsc_51xx_v50.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_52xx_v50.txt $(INSTALLDIR)/etc/gsc_52xx_v50.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_53xx_v50.txt $(INSTALLDIR)/etc/gsc_53xx_v50.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_54xx_v50.txt $(INSTALLDIR)/etc/gsc_54xx_v50.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_551x_v50.txt $(INSTALLDIR)/etc/gsc_551x_v50.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_552x_v50.txt $(INSTALLDIR)/etc/gsc_552x_v50.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_553x_v50.txt $(INSTALLDIR)/etc/gsc_553x_v50.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_630x_v61.txt $(INSTALLDIR)/etc/gsc_610x_v61.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_630x_v61.txt $(INSTALLDIR)/etc/gsc_620x_v61.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_630x_v61.txt $(INSTALLDIR)/etc/gsc_630x_v61.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_630x_v61.txt $(INSTALLDIR)/etc/gsc_640x_v61.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_6903_v61.txt $(INSTALLDIR)/etc/gsc_6903_v61.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_6904_v61.txt $(INSTALLDIR)/etc/gsc_6904_v61.txt
	install -m 0644 -o $(INSTOWNER) -g $(INSTGROUP) gsc_6905_v61.txt $(INSTALLDIR)/etc/gsc_6905_v61.txt

clean:
	rm -f gsp_updater i2cset *.o core

