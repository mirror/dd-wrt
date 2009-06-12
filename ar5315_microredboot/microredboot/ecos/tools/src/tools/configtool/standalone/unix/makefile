COMPILE := ../common/ecosconfig.cxx ../common/cdl_exec.cxx ../../common/common/build.cxx ../../common/common/flags.cxx
OBJECTS := $(COMPILE:.cxx=.o)
INCLUDE_PATH := -I../../common/common -I$(ECOS)/include -I$(TCL)/include
CXX := g++

ecosconfig: $(OBJECTS)
	$(CXX) $^ -L$(ECOS)/lib -L$(TCL)/lib -lcdl -lcyginfra -ltcl8.2 -ldl -o $@

%.o: %.cxx
	$(CXX) -c -g -O2 -I$(dir $<) $(INCLUDE_PATH) -o $@ $<

clean:
	rm ecosconfig $(OBJECTS)

