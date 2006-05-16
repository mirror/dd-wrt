shatd.o: shatd.c do.h device.h pool.h util.h lookup.h arp.h ip2ether.h \
  cleanup.h ctrl.h comhandler.h
util.o: util.c util.h
do.o: do.c util.h do.h
pool.o: pool.c util.h pool.h
device.o: device.c util.h device.h pool.h
lookup.o: lookup.c util.h pool.h device.h lookup.h
arp.o: arp.c util.h arp.h device.h pool.h lookup.h
ip2ether.o: ip2ether.c ip2ether.h device.h pool.h util.h lookup.h
cleanup.o: cleanup.c util.h cleanup.h lookup.h pool.h device.h
ctrl.o: ctrl.c util.h ctrl.h
comhandler.o: comhandler.c util.h ctrl.h device.h pool.h comhandler.h \
  arp.h lookup.h ip2ether.h
