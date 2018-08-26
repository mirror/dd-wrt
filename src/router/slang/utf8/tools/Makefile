TABLES = slupper.h sllower.h slischar.h slcombin.h slwcwidth.h
UNICODEDATA = UnicodeData.txt EastAsianWidth.txt

SLANG_SRC_DIR = ../../src
# This works with GNU make.  It uses a pattern rule (%.h) to deal with the
# fact that mktables creates all the .h files

all: $(TABLES)

$(TABLES) : mktables $(UNICODEDATA)
	./mktables $(UNICODEDATA)

install: $(TABLES)
	cp $(TABLES) $(SLANG_SRC_DIR)

clean:
	/bin/rm -f *~ \#* $(TABLES)
