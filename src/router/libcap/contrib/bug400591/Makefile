all: bug

bug: bug.c ../../libcap Makefile
	make -C ../../libcap
	cc -g -I../../libcap/include --static -o $@ $< -L../../libcap -lcap
	./bug

clean:
	rm -f bug.o bug
