BIN=fft_eval
OBJ=fft_eval.o
.SUFFIXES: .o .c
.c.o:
	$(CC) $(LTO) $(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -c -o $@ $<

default:	all
all:	$(BIN)

$(BIN): $(OBJ)
	$(CC) $(LDLTO) $(COPTS) -o $@ $(OBJ) -lm

clean:
	rm -rf $(BIN) $(OBJ)
	
install: 
	install -D $(BIN) $(INSTALLDIR)/usr/sbin/fft_eval

