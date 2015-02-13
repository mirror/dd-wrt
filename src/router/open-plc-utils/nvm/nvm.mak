# file: nvm/nvm.mak

# ====================================================================
# programs;
# --------------------------------------------------------------------

chknvm.o: chknvm.c error.h files.h flags.h getoptv.h memory.h nvm.h sdram.h
chknvm2.o: chknvm2.c error.h files.h flags.h getoptv.h memory.h nvm.h sdram.h
nvmmerge.o: nvmmerge.c error.h files.h flags.h getoptv.h memory.h nvm.h
nvmsplit.o: nvmsplit.c error.h files.h flags.h getoptv.h memory.h nvm.h

# ====================================================================
# functions;
# --------------------------------------------------------------------

NVMSelect.o: NVMSelect.c error.h files.h plc.h
manifest.o: manifest.c endian.h error.h format.h nvm.h
manifetch.o: manifetch.c endian.h nvm.h
nvm.o: nvm.c nvm.h
nvmfile.o: nvmfile.c endian.h error.h files.h nvm.h
nvmfile1.o: nvmfile1.c error.h files.h memory.h nvm.h
nvmfile2.o: nvmfile2.c error.h files.h memory.h nvm.h
nvmpeek.o: nvmpeek.c memory.h nvm.h
nvmpeek1.o: nvmpeek1.c format.h memory.h nvm.h
nvmpeek2.o: nvmpeek2.c format.h memory.h nvm.h
nvmseek1.o: nvmseek1.c endian.h error.h flags.h memory.h nvm.h pib.h
nvmseek2.o: nvmseek2.c endian.h error.h flags.h memory.h nvm.h pib.h

# ====================================================================
# headers;
# --------------------------------------------------------------------

nvm.h: types.h

