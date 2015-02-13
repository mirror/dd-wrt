# ====================================================================
# programs; 
# --------------------------------------------------------------------

chkpib.o: chkpib.c HPAVKey.h error.h files.h flags.h getoptv.h nvm.h pib.h
chkpib2.o: chkpib2.c HPAVKey.h error.h files.h flags.h getoptv.h nvm.h pib.h
fetchpib.o: fetchpib.c error.h files.h flags.h getoptv.h nvm.h pib.h
getpib.o: getpib.c error.h files.h flags.h getoptv.h memory.h number.h pib.h types.h
gpioinfo.o: gpioinfo.c error.h files.h getoptv.h number.h putoptv.h types.h
modpib.o: modpib.c HPAVKey.h chars.h error.h files.h flags.h getoptv.h keys.h number.h pib.h plc.h
pib2xml.o: pib2xml.c chars.h error.h files.h format.h getoptv.h node.h pib.h putoptv.h version.h
pibcomp.o: pibcomp.c chars.h error.h files.h flags.h getoptv.h memory.h number.h pib.h sizes.h
pibdump.o: pibdump.c chars.h error.h files.h flags.h getoptv.h memory.h number.h nvm.h pib.h putoptv.h sizes.h version.h
pibruin.o: pibruin.c error.h files.h flags.h getoptv.h memory.h number.h pib.h rules.h symbol.h types.h
pibrump.o: pibrump.c error.h files.h flags.h getoptv.h memory.h pib.h rules.h symbol.h types.h
pibscalers.o: pibscalers.c error.h pib.h plc.h
psgraph.o: psgraph.c error.h files.h getoptv.h number.h pib.h plc.h putoptv.h
psin.o: psin.c HPAVKey.h chars.h endian.h error.h files.h getoptv.h number.h pib.h plc.h types.h
pskey.o: pskey.c HPAVKey.h SHA256.h error.h files.h flags.h getoptv.h number.h pib.h types.h
psnotch.o: psnotch.c chars.h error.h flags.h getoptv.h number.h putoptv.h
psout.o: psout.c chars.h endian.h error.h files.h getoptv.h number.h pib.h plc.h types.h
psread.o: psread.c chars.h error.h number.h pib.h
qosinfo.o: qosinfo.c error.h files.h getoptv.h number.h putoptv.h
setpib.o: setpib.c error.h files.h flags.h getoptv.h memory.h number.h nvm.h pib.h putoptv.h types.h
throwpib.o: throwpib.c error.h files.h flags.h getoptv.h nvm.h pib.h
xml2pib.o: xml2pib.c error.h files.h flags.h getoptv.h node.h nvm.h pib.h

#====================================================================
# functions;
# --------------------------------------------------------------------

pibfile.o: pibfile.c error.h files.h memory.h pib.h
pibfile1.o: pibfile1.c error.h files.h pib.h
pibfile2.o: pibfile2.c error.h files.h memory.h nvm.h pib.h
piblock.o: piblock.c error.h files.h pib.h
pibscalers.o: pibscalers.c error.h pib.h plc.h
pibseek.o: pibseek.c endian.h error.h flags.h memory.h nvm.h pib.h
pibpeek1.o: pibpeek1.c HPAVKey.h keys.h memory.h number.h pib.h
pibpeek2.o: pibpeek2.c HPAVKey.h keys.h memory.h number.h pib.h
pibtype.o: pibtype.c endian.h error.h files.h flags.h pib.h
ruledump.o: ruledump.c memory.h rules.h

# ====================================================================
# pib header files;
# --------------------------------------------------------------------

pib.h: HPAVKey.h memory.h types.h


