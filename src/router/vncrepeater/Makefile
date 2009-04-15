CFLAGS=-Wall
OBJDIR=/tmp/uvnc
repeater: repeater.o repeaterproc.o openbsd_stringfuncs.o iniparser.o readini.o repeaterutil.o repeaterevents.o
	$(CXX) $(CFLAGS) -o repeater repeater.o repeaterproc.o openbsd_stringfuncs.o iniparser.o readini.o repeaterutil.o repeaterevents.o
	$(CC) $(CFLAGS) -o start-stop-daemon start-stop-daemon.c
repeater.o: repeater.cpp
	$(CXX) $(CFLAGS) -c repeater.cpp

repeaterproc.o: repeaterproc.cpp
	$(CXX) $(CFLAGS) -c repeaterproc.cpp

openbsd_stringfuncs.o: openbsd_stringfuncs.cpp
	$(CXX) $(CFLAGS) -c openbsd_stringfuncs.cpp

iniparser.o: iniparser.cpp
	$(CXX) $(CFLAGS) -c iniparser.cpp

readini.o: readini.cpp
	$(CXX) $(CFLAGS) -c readini.cpp

repeaterutil.o: repeaterutil.cpp
	$(CXX) $(CFLAGS) -c repeaterutil.cpp

repeaterevents.o: repeaterevents.cpp
	$(CXX) $(CFLAGS) -c repeaterevents.cpp

clean:
	rm -f *.o repeater start-stop-daemon

install:
	mkdir -p $(INSTALLDIR)/usr/sbin
	mkdir -p $(INSTALLDIR)/etc/config
	cp -R repeater $(INSTALLDIR)/usr/sbin
	cp -R config/* $(INSTALLDIR)/etc/config
#	cp -R start-stop-daemon $(INSTALLDIR)/usr/sbin/start-stop-daemon
#	cp -R uvncrepeater /etc/init.d/uvncrepeater
#	@if test ! -d ${OBJDIR}; then \
#                mkdir /etc/uvnc; \
#        else :; fi
#	cp -R uvncrepeater.ini /etc/uvnc/uvncrepeater.ini
#	cat message
