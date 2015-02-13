# ====================================================================
# files;
# --------------------------------------------------------------------

config2cfg.o: config2cfg.c getoptv.h putoptv.h version.h flags.h files.h error.h sdram.h 
sdram.o: sdram.c sdram.h checksum32.c types.h flags.h memory.h
sdramfile.o: sdramfile.c sdram.h memory.h flags.h types.h
sdrampeek.o: sdrampeek.c sdram.h types.h flags.h
nvrampeek.o: nvrampeek.c nvram.h symbol.h
nvram.o: nvram.c nvram.h

# ====================================================================
# files;
# --------------------------------------------------------------------

sdram.h: types.h flags.h
nvram.h: types.h
