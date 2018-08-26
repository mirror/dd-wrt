INCS = -I..
LIBS = -L../$(ARCH)objs -lslang -lm

bcdump: bcdump.c ../$(ARCH)objs/libslang.a
	$(CC) $(CFLAGS) $(INCS) bcdump.c -o bcdump $(LIBS)
clean:
	/bin/rm -f *.o bcdump

