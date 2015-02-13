# file: serial/serial.mak

# ====================================================================
# source files;
# --------------------------------------------------------------------

baudrate.o: baudrate.c error.h
int6kbaud.o: int6kbaud.c endian.h error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h serial.h symbol.h types.h
int6kdetect.o: int6kdetect.c error.h files.h flags.h getoptv.h putoptv.h serial.h
int6kuart.o: int6kuart.c endian.h error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h serial.h types.h
ptsctl.o: ptsctl.c error.h files.h flags.h getoptv.h number.h putoptv.h symbol.h timer.h version.h
ttycat.o: ttycat.c error.h files.h getoptv.h number.h putoptv.h serial.h
ttyrecv.o: ttyrecv.c error.h flags.h getoptv.h number.h putoptv.h serial.h types.h
ttysend.o: ttysend.c error.h files.h getoptv.h number.h putoptv.h serial.h
ttysig.o: ttysig.c error.h flags.h getoptv.h number.h putoptv.h types.h version.h
weeder.o: weeder.c error.h files.h flags.h getoptv.h number.h putoptv.h symbol.h timer.h version.h

# ====================================================================
# files;
# --------------------------------------------------------------------

baudrate.o: baudrate.c error.h
closeport.o: closeport.c serial.h types.h
openport.o: openport.c error.h files.h flags.h serial.h types.h
serial.o: serial.c error.h flags.h number.h serial.h types.h

serial.h: types.h

