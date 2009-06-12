
all: safl.o sa_flash

safl.o: safl.c
	cc -DMODULE -D__KERNEL__ -Wall -Wstrict-prototypes -O6 -c safl.c \
	   `[ -f /usr/include/linux/modversions.h ] && echo -DMODVERSIONS`

sa_flash: sa_flash.c
	cc -g -O2 -Wall sa_flash.c -o sa_flash

clean:
	rm -f *.o sa_flash core

