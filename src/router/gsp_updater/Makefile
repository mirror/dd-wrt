INSTDIR   = $(INSTALLDIR)/usr/bin
INSTMODE  = 0755
INSTOWNER = root
INSTGROUP = root

all: gsp_updater

gsp_updater: gsp_updater.o
	$(CC) $(CFLAGS) -DNEED_PRINTF  $(LDFLAGS) $^ $(LDLIBS) -o $@

test_audio: test_audio.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@ -lasound

%.o: %.c
	$(CC) -c $(CFLAGS) $^ -o $@ -lasound

install: gsp_updater
	install -d $(INSTDIR)
	install -m $(INSTMODE) -o $(INSTOWNER) -g $(INSTGROUP) gsp_updater $(INSTDIR)
	install -m $(INSTMODE) -o $(INSTOWNER) -g $(INSTGROUP) gsp_update.txt $(INSTALLDIR)/etc

clean:
	rm -f gsp_updater *.o core

