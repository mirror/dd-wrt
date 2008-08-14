all: static shared 

static:
	cd library  && make static && cd ..

shared:
	cd library  && make shared && cd ..

progs:
	cd programs && make all && cd ..

doc:
	rm -rf docs
	cd library && doxygen && cd ..
	mv html docs

install:
	mkdir -p $(DESTDIR)/{include/xyssl,lib}
	cp -v -r include $(DESTDIR)/include
	cp -v library/libxyssl.a $(DESTDIR)/lib
	
	mkdir -p $(DESTDIR)/bin
	for i in programs/*; do test -x $i && \
	    cp -v $i $(DESTDIR)/xyssl_$i;   \
	done

clean:
	cd library  && make clean && cd ..
	cd programs && make clean && cd ..

