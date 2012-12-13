INSTDIR   = $(INSTALLDIR)/usr/bin
INSTMODE  = 0755
INSTOWNER = root
INSTGROUP = root

all: gsp_updater
CFLAGS += -DNEED_PRINTF

gsp_updater: gsp_updater.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

test_audio: test_audio.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@ -lasound

%.o: %.c
	$(CC) -c $(CFLAGS) $^ -o $@ -lasound

install: gsp_updater
	install -d $(INSTDIR)
	install -m $(INSTMODE) -o $(INSTOWNER) -g $(INSTGROUP) gsp_updater $(INSTDIR)
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
	rm -f gsp_updater *.o core

