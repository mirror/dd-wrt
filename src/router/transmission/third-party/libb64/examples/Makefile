BINARIES = c-example1 c-example2

CFLAGS += -g

SOURCES = c-example1.c c-example2.c

TARGETS = $(BINARIES)

DEBRIS = encoded.txt decoded.txt

# only because we use ANSI C, not required to use libb64!
CFLAGS += -Werror -pedantic
CFLAGS += -I../include

CXXFLAGS += $(CFLAGS)

vpath %.h ../include/b64
vpath %.a ../src

.PHONY : clean

all: $(TARGETS) test

c-example1 c-example2: libb64.a

clean: 
	rm -f *.exe* *.o $(TARGETS) $(DEBRIS) *.bak *~

distclean: clean
	rm -f depend

depend: $(SOURCES)
	makedepend -f- $(CFLAGS) $(SOURCES) 2> /dev/null 1> depend

test-c-example1: c-example1
	./c-example1

test-c-example2: c-example2
	./c-example2 loremgibson.txt encoded.txt decoded.txt
	diff -q loremgibson.txt decoded.txt

test: test-c-example1 test-c-example2

-include depend

