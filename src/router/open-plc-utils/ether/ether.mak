# ====================================================================
# programs;
# --------------------------------------------------------------------

edru.o: edru.c channel.h error.h flags.h getoptv.h memory.h number.h putoptv.h types.h
edsu.o: edsu.c channel.h error.h ether.h files.h flags.h getoptv.h memory.h number.h putoptv.h
efbu.o: efbu.c channel.h error.h flags.h getoptv.h memory.h number.h putoptv.h symbol.h
efeu.o: efeu.c channel.h error.h flags.h getoptv.h memory.h number.h putoptv.h types.h
efru.o: efru.c channel.h error.h flags.h getoptv.h memory.h mme.h number.h plc.h putoptv.h types.h
efsu.o: efsu.c channel.h error.h flags.h getoptv.h memory.h number.h putoptv.h symbol.h
nics.o: nics.c error.h ether.h getoptv.h memory.h putoptv.h version.h
pcapdevs.o: pcapdevs.c error.h ether.h flags.h getoptv.h memory.h putoptv.h types.h version.h

# ====================================================================
# source files;
# --------------------------------------------------------------------

channel.o: channel.c channel.h
closechannel.o: closechannel.c channel.h
fcs.o: fcs.c
gethwaddr.o: gethwaddr.c error.h ether.h
getifname.o: getifname.c error.h ether.h
openchannel.o: openchannel.c channel.h error.h flags.h gethwaddr.c memory.h
pcap_freenameindex.o: pcap_freenameindex.c ether.h
pcap_indextoname.o: pcap_indextoname.c ether.h
pcap_nameindex.o: pcap_nameindex.c ether.h
pcap_nametoindex.o: pcap_nametoindex.c ether.h
pcapdevs.o: pcapdevs.c error.c error.h ether.h flags.h gethwaddr.c getoptv.c getoptv.h hexdecode.c memory.h putoptv.c putoptv.h types.h version.c version.h
readchannel.o: readchannel.c channel.h error.h 
readpacket.o: readpacket.c channel.h error.h flags.h hexload.c memory.h
sendpacket.o: sendpacket.c channel.h flags.h memory.h

# ====================================================================
# header files;
# --------------------------------------------------------------------

channel.h: ether.h types.h
ether.h: types.h

